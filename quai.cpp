#include "quai.h"
#include "connection.h"
#include <QDateTime>
#include <QDebug>
#include <QHash>
#include <QLocale>
#include <QMarginsF>
#include <QMetaType>
#include <QPageLayout>
#include <QPageSize>
#include <QPrinter>
#include <QSet>
#include <QSqlError>
#include <QSqlRecord>
#include <QTextDocument>
#include <QtGlobal>

QString Quai::s_lastError;

namespace {

struct ResolvedQuaiSchema {
    bool ok = false;
    QString quaiId;
    QString quaiNom;
    QString quaiType;
    QString longueur;
    QString profondeur;
    QString capacite;
    QString statut;
    QString currentUsage;
    QString occupancyRate;
    QString priorityLevel;
    QString safetyLevel;
    QString lightingStatus;
    QString incidentLog;
};

static QHash<QString, ResolvedQuaiSchema> s_schemaByConnection;

static QSet<QString> loadQuaiColumnNamesUpper(QSqlDatabase &db)
{
    QSet<QString> set;
    if (!db.isValid() || !db.isOpen())
        return set;

    QSqlQuery q(db);
    if (q.exec(QStringLiteral("SELECT * FROM QUAI WHERE 1=0"))) {
        const QSqlRecord rec = q.record();
        for (int i = 0; i < rec.count(); ++i)
            set.insert(rec.fieldName(i).toUpper());
    }
    if (!set.isEmpty())
        return set;

    if (q.exec(QStringLiteral("SELECT * FROM QUAI WHERE ROWNUM < 1"))) {
        const QSqlRecord rec = q.record();
        for (int i = 0; i < rec.count(); ++i)
            set.insert(rec.fieldName(i).toUpper());
    }
    if (!set.isEmpty())
        return set;

    if (q.exec(QStringLiteral("SELECT COLUMN_NAME FROM USER_TAB_COLUMNS WHERE UPPER(TABLE_NAME)=UPPER('QUAI')"))) {
        while (q.next())
            set.insert(q.value(0).toString().toUpper());
    }
    return set;
}

static QString pickCol(const QSet<QString> &avail, std::initializer_list<const char *> candidates)
{
    for (const char *c : candidates) {
        const QString u = QString::fromLatin1(c).toUpper();
        if (avail.contains(u))
            return u;
    }
    return QString();
}

static ResolvedQuaiSchema resolveQuaiTable(QSqlDatabase &db)
{
    ResolvedQuaiSchema r;
    const QSet<QString> avail = loadQuaiColumnNamesUpper(db);
    if (avail.isEmpty())
        return r;

    r.quaiId = pickCol(avail, {"QUAI_ID"});
    r.quaiNom = pickCol(avail, {"QUAI_NOM"});
    r.quaiType = pickCol(avail, {"QUAI_TYPE"});
    r.longueur = pickCol(avail, {"LONGUEUR"});
    r.profondeur = pickCol(avail, {"PROFONDEUR"});
    r.capacite = pickCol(avail, {"CAPACITE"});
    r.statut = pickCol(avail, {"STATUT"});
    r.currentUsage = pickCol(avail, {"CURRENT_USAGE", "CURRENTUSAGE"});
    r.occupancyRate = pickCol(avail, {"OCCUPANCY_RATE", "OCCUPANCYRATE"});
    r.priorityLevel = pickCol(avail, {"PRIORITY_LEVEL", "PRIORITYLEVEL"});
    r.safetyLevel = pickCol(avail, {"SAFETY_LEVEL", "SAFETYLEVEL"});
    r.lightingStatus = pickCol(avail, {"LIGHTING_STATUS", "LIGHTINGSTATUS"});
    r.incidentLog = pickCol(avail, {"INCIDENT_LOG", "INCIDENTLOG"});

    r.ok = !r.quaiId.isEmpty() && !r.quaiNom.isEmpty();
    return r;
}

static const ResolvedQuaiSchema &schemaRef()
{
    const QString conn = QString::fromLatin1(Connection::connectionName());
    auto it = s_schemaByConnection.find(conn);
    if (it != s_schemaByConnection.end())
        return it.value();

    QSqlDatabase db = QSqlDatabase::database(conn);
    ResolvedQuaiSchema s = resolveQuaiTable(db);
    s_schemaByConnection.insert(conn, s);
    it = s_schemaByConnection.find(conn);
    return it.value();
}

static QString numOrNull(const QString &col)
{
    return col.isEmpty() ? QStringLiteral("CAST(NULL AS NUMBER)") : col;
}

static QString strOrNull(const QString &col)
{
    return col.isEmpty() ? QStringLiteral("CAST(NULL AS VARCHAR2(100))") : col;
}

} // namespace

QString Quai::sqlQuaiSelectList()
{
    const ResolvedQuaiSchema &s = schemaRef();
    QStringList parts;
    parts << numOrNull(s.quaiId);
    parts << strOrNull(s.quaiNom);
    parts << strOrNull(s.quaiType);
    parts << numOrNull(s.longueur);
    parts << numOrNull(s.profondeur);
    parts << numOrNull(s.capacite);
    parts << strOrNull(s.statut);
    parts << numOrNull(s.currentUsage);
    parts << numOrNull(s.occupancyRate);
    parts << strOrNull(s.priorityLevel);
    parts << strOrNull(s.safetyLevel);
    parts << strOrNull(s.lightingStatus);
    return parts.join(QStringLiteral(", "));
}

QString Quai::sqlQuaiColQuaiId() { return schemaRef().quaiId; }
QString Quai::sqlQuaiColQuaiNom() { return schemaRef().quaiNom; }
QString Quai::sqlQuaiColQuaiType() { return schemaRef().quaiType; }
QString Quai::sqlQuaiColStatut() { return schemaRef().statut; }
QString Quai::sqlQuaiColLongueur() { return schemaRef().longueur; }
QString Quai::sqlQuaiColProfondeur() { return schemaRef().profondeur; }
QString Quai::sqlQuaiColCapacite() { return schemaRef().capacite; }
QString Quai::sqlQuaiColCurrentUsage() { return schemaRef().currentUsage; }
QString Quai::sqlQuaiColOccupancyRate() { return schemaRef().occupancyRate; }
QString Quai::sqlQuaiColPriorityLevel() { return schemaRef().priorityLevel; }
QString Quai::sqlQuaiColSafetyLevel() { return schemaRef().safetyLevel; }
QString Quai::sqlQuaiColLightingStatus() { return schemaRef().lightingStatus; }
QString Quai::sqlQuaiColIncidentLog() { return schemaRef().incidentLog; }

bool Quai::appendIncidentLog(int quaiId, const QString &line)
{
    s_lastError.clear();
    const ResolvedQuaiSchema &s = schemaRef();
    if (!s.ok || s.quaiId.isEmpty()) {
        s_lastError = QStringLiteral("Schéma QUAI illisible.");
        return false;
    }
    if (s.incidentLog.isEmpty()) {
        s_lastError = QStringLiteral("Colonne INCIDENT_LOG absente sur la table QUAI.");
        return false;
    }
    if (quaiId <= 0 || line.trimmed().isEmpty()) {
        s_lastError = QStringLiteral("ID ou texte d'incident invalide.");
        return false;
    }
    QSqlDatabase db = QSqlDatabase::database(Connection::connectionName());
    if (!db.isOpen()) {
        s_lastError = QStringLiteral("Base fermée.");
        return false;
    }
    const QString stamp = QDateTime::currentDateTime().toString(Qt::ISODate);
    const QString addition = stamp + QStringLiteral(" — ") + line.trimmed();
    const QString sql = QStringLiteral(
                          "UPDATE QUAI SET %1 = SUBSTR(RTRIM(NVL(%1, '') || CHR(10) || ?), 1, 4000) WHERE %2 = ?")
                          .arg(s.incidentLog, s.quaiId);
    QSqlQuery q(db);
    q.prepare(sql);
    q.addBindValue(addition);
    q.addBindValue(quaiId);
    if (!q.exec()) {
        s_lastError = q.lastError().text();
        return false;
    }
    db.commit();
    return true;
}

bool Quai::clearIncidentLog(int quaiId)
{
    s_lastError.clear();
    const ResolvedQuaiSchema &s = schemaRef();
    if (!s.ok || s.quaiId.isEmpty()) {
        s_lastError = QStringLiteral("Schéma QUAI illisible.");
        return false;
    }
    if (s.incidentLog.isEmpty()) {
        s_lastError = QStringLiteral("Colonne INCIDENT_LOG absente sur la table QUAI.");
        return false;
    }
    if (quaiId <= 0) {
        s_lastError = QStringLiteral("ID quai invalide.");
        return false;
    }
    QSqlDatabase db = QSqlDatabase::database(Connection::connectionName());
    if (!db.isOpen()) {
        s_lastError = QStringLiteral("Base fermée.");
        return false;
    }
    QSqlQuery q(db);
    q.prepare(QStringLiteral("UPDATE QUAI SET %1 = NULL WHERE %2 = ?").arg(s.incidentLog, s.quaiId));
    q.addBindValue(quaiId);
    if (!q.exec()) {
        s_lastError = q.lastError().text();
        return false;
    }
    db.commit();
    return true;
}

bool Quai::endMaintenanceAlert(int quaiId)
{
    s_lastError.clear();
    const ResolvedQuaiSchema &s = schemaRef();
    if (!s.ok || s.quaiId.isEmpty()) {
        s_lastError = QStringLiteral("Schéma QUAI illisible.");
        return false;
    }
    if (quaiId <= 0) {
        s_lastError = QStringLiteral("ID quai invalide.");
        return false;
    }
    QSqlDatabase db = QSqlDatabase::database(Connection::connectionName());
    if (!db.isOpen()) {
        s_lastError = QStringLiteral("Base fermée.");
        return false;
    }
    QStringList sets;
    if (!s.quaiType.isEmpty()) {
        sets << QStringLiteral(
                    "%1 = CASE WHEN LOWER(TRIM(NVL(%1, ''))) = 'maintenance' THEN 'Pêche' ELSE %1 END")
                    .arg(s.quaiType);
    }
    if (!s.statut.isEmpty()) {
        sets << QStringLiteral(
                    "%1 = CASE WHEN LOWER(NVL(%1, '')) LIKE '%%maintenance%%' THEN 'Actif' ELSE %1 END")
                    .arg(s.statut);
    }
    if (sets.isEmpty()) {
        s_lastError = QStringLiteral("Colonnes TYPE/STATUT introuvables.");
        return false;
    }
    const QString sql = QStringLiteral("UPDATE QUAI SET %1 WHERE %2 = ?")
                            .arg(sets.join(QStringLiteral(", ")), s.quaiId);
    QSqlQuery q(db);
    q.prepare(sql);
    q.addBindValue(quaiId);
    if (!q.exec()) {
        s_lastError = q.lastError().text();
        return false;
    }
    db.commit();
    return true;
}

int Quai::countActiveQuais()
{
    s_lastError.clear();
    QSqlDatabase db = QSqlDatabase::database(Connection::connectionName());
    if (!db.isOpen()) {
        s_lastError = QStringLiteral("Base fermée.");
        return 0;
    }

    const QString stCol = sqlQuaiColStatut().isEmpty() ? QStringLiteral("STATUT") : sqlQuaiColStatut();
    const QString tyCol = sqlQuaiColQuaiType().isEmpty() ? QStringLiteral("QUAI_TYPE") : sqlQuaiColQuaiType();

    QSqlQuery q(db);
    const QString sql = QStringLiteral(
                            "SELECT COUNT(*) FROM QUAI "
                            "WHERE LOWER(NVL(%1,'')) NOT LIKE '%%inactif%%' "
                            "  AND LOWER(NVL(%1,'')) NOT LIKE '%%maintenance%%' "
                            "  AND LOWER(NVL(%2,'')) <> 'maintenance'")
                            .arg(stCol, tyCol);
    if (!q.exec(sql) || !q.next()) {
        s_lastError = q.lastError().text();
        return 0;
    }
    return q.value(0).toInt();
}

int Quai::countTotalQuais()
{
    s_lastError.clear();
    QSqlDatabase db = QSqlDatabase::database(Connection::connectionName());
    if (!db.isOpen()) {
        s_lastError = QStringLiteral("Base fermée.");
        return 0;
    }
    QSqlQuery q(db);
    if (!q.exec(QStringLiteral("SELECT COUNT(*) FROM QUAI")) || !q.next()) {
        s_lastError = q.lastError().text();
        return 0;
    }
    return q.value(0).toInt();
}

bool Quai::hasAnyActiveQuai()
{
    return countActiveQuais() > 0;
}

bool Quai::setStatutForQuaiId(int quaiId, const QString &statut)
{
    s_lastError.clear();
    if (quaiId <= 0 || statut.trimmed().isEmpty()) {
        s_lastError = QStringLiteral("ID ou statut invalide.");
        return false;
    }
    const ResolvedQuaiSchema &s = schemaRef();
    if (!s.ok || s.quaiId.isEmpty() || s.statut.isEmpty()) {
        s_lastError = QStringLiteral("Schéma QUAI illisible ou colonne STATUT absente.");
        return false;
    }
    QSqlDatabase db = QSqlDatabase::database(Connection::connectionName());
    if (!db.isOpen()) {
        s_lastError = QStringLiteral("Base fermée.");
        return false;
    }
    QSqlQuery q(db);
    q.prepare(QStringLiteral("UPDATE QUAI SET %1 = ? WHERE %2 = ?").arg(s.statut, s.quaiId));
    q.addBindValue(statut.trimmed());
    q.addBindValue(quaiId);
    if (!q.exec()) {
        s_lastError = q.lastError().text();
        return false;
    }
    if (q.numRowsAffected() == 0) {
        s_lastError = QStringLiteral("Aucune ligne mise à jour (ID %1).").arg(quaiId);
        return false;
    }
    db.commit();
    return true;
}

QVector<QPair<int, QString>> Quai::activeQuaisIdAndLabel()
{
    QVector<QPair<int, QString>> out;
    QSqlDatabase db = QSqlDatabase::database(Connection::connectionName());
    if (!db.isOpen())
        return out;

    const QString stCol = sqlQuaiColStatut().isEmpty() ? QStringLiteral("STATUT") : sqlQuaiColStatut();
    const QString tyCol = sqlQuaiColQuaiType().isEmpty() ? QStringLiteral("QUAI_TYPE") : sqlQuaiColQuaiType();
    const QString idCol = sqlQuaiColQuaiId().isEmpty() ? QStringLiteral("QUAI_ID") : sqlQuaiColQuaiId();
    const QString nomCol = sqlQuaiColQuaiNom().isEmpty() ? QStringLiteral("QUAI_NOM") : sqlQuaiColQuaiNom();

    const QString sql = QStringLiteral(
                            "SELECT %3, %4 FROM QUAI "
                            "WHERE LOWER(NVL(%1,'')) NOT LIKE '%%inactif%%' "
                            "  AND LOWER(NVL(%1,'')) NOT LIKE '%%maintenance%%' "
                            "  AND LOWER(NVL(%2,'')) <> 'maintenance' "
                            "ORDER BY %3")
                            .arg(stCol, tyCol, idCol, nomCol);
    QSqlQuery q(db);
    if (!q.exec(sql))
        return out;
    while (q.next()) {
        const int id = q.value(0).toInt();
        const QString nom = q.value(1).toString().trimmed();
        out.append(qMakePair(id, QStringLiteral("%1 — %2").arg(id).arg(nom.isEmpty() ? QStringLiteral("—") : nom)));
    }
    return out;
}

QVector<QPair<int, QString>> Quai::occupiedQuaisIdAndLabel()
{
    QVector<QPair<int, QString>> out;
    QSqlDatabase db = QSqlDatabase::database(Connection::connectionName());
    if (!db.isOpen())
        return out;

    const QString stCol = sqlQuaiColStatut().isEmpty() ? QStringLiteral("STATUT") : sqlQuaiColStatut();
    const QString tyCol = sqlQuaiColQuaiType().isEmpty() ? QStringLiteral("QUAI_TYPE") : sqlQuaiColQuaiType();
    const QString idCol = sqlQuaiColQuaiId().isEmpty() ? QStringLiteral("QUAI_ID") : sqlQuaiColQuaiId();
    const QString nomCol = sqlQuaiColQuaiNom().isEmpty() ? QStringLiteral("QUAI_NOM") : sqlQuaiColQuaiNom();

    const QString sql = QStringLiteral(
                            "SELECT %3, %4 FROM QUAI "
                            "WHERE LOWER(NVL(%1,'')) LIKE '%%inactif%%' "
                            "  AND LOWER(NVL(%1,'')) NOT LIKE '%%maintenance%%' "
                            "  AND LOWER(NVL(%2,'')) <> 'maintenance' "
                            "ORDER BY %3")
                            .arg(stCol, tyCol, idCol, nomCol);
    QSqlQuery q(db);
    if (!q.exec(sql))
        return out;
    while (q.next()) {
        const int id = q.value(0).toInt();
        const QString nom = q.value(1).toString().trimmed();
        out.append(qMakePair(id, QStringLiteral("%1 — %2").arg(id).arg(nom.isEmpty() ? QStringLiteral("—") : nom)));
    }
    return out;
}

Quai::Quai() = default;

QString Quai::lastErrorMessage()
{
    QString msg = s_lastError;
    s_lastError.clear();
    return msg;
}

int Quai::nextId()
{
    QSqlDatabase db = QSqlDatabase::database(Connection::connectionName());
    if (!db.isOpen())
        return 1;
    QSqlQuery q(db);
    if (q.exec(QStringLiteral("SELECT SEQ_QUAI_ID.NEXTVAL FROM DUAL")) && q.next()) {
        const int v = q.value(0).toInt();
        if (v > 0)
            return v;
    }
    const QString idCol = schemaRef().quaiId;
    if (!idCol.isEmpty() && q.exec(QStringLiteral("SELECT NVL(MAX(%1), 0) + 1 FROM QUAI").arg(idCol)) && q.next())
        return qMax(1, q.value(0).toInt());
    if (q.exec(QStringLiteral("SELECT NVL(MAX(QUAI_ID), 0) + 1 FROM QUAI")) && q.next())
        return qMax(1, q.value(0).toInt());
    return 1;
}

bool Quai::ajouter()
{
    s_lastError.clear();
    QSqlDatabase db = QSqlDatabase::database(Connection::connectionName());
    if (!db.isOpen()) {
        s_lastError = QStringLiteral("Base fermée.");
        return false;
    }
    const ResolvedQuaiSchema &s = schemaRef();
    if (!s.ok) {
        s_lastError = QStringLiteral("Table QUAI introuvable ou schéma illisible.");
        return false;
    }

    QStringList cols;
    QList<QVariant> vals;
    auto add = [&](const QString &c, const QVariant &v) {
        if (!c.isEmpty()) {
            cols << c;
            vals << v;
        }
    };
    add(s.quaiId, m_quaiId);
    add(s.quaiNom, m_quaiNom);
    add(s.quaiType, m_quaiType);
    add(s.longueur, m_longueur);
    add(s.profondeur, m_profondeur);
    add(s.capacite, m_capacite);
    add(s.statut, m_statut);
    add(s.currentUsage, m_currentUsage);
    add(s.occupancyRate, m_occupancyRate);
    add(s.priorityLevel, m_priorityLevel);
    add(s.safetyLevel, m_safetyLevel);
    add(s.lightingStatus, m_lightingStatus);

    QString ph;
    for (int i = 0; i < cols.size(); ++i) {
        if (i) ph += QLatin1Char(',');
        ph += QLatin1Char('?');
    }
    const QString sql = QStringLiteral("INSERT INTO QUAI (%1) VALUES (%2)").arg(cols.join(QLatin1Char(',')), ph);

    QSqlQuery query(db);
    query.prepare(sql);
    for (const QVariant &v : vals)
        query.addBindValue(v);

    if (!query.exec()) {
        s_lastError = query.lastError().text();
        qDebug() << "Quai::ajouter error:" << s_lastError;
        return false;
    }
    db.commit();
    return true;
}

bool Quai::modifier()
{
    s_lastError.clear();
    if (m_quaiId <= 0) {
        s_lastError = QStringLiteral("ID quai invalide.");
        return false;
    }
    QSqlDatabase db = QSqlDatabase::database(Connection::connectionName());
    if (!db.isOpen()) {
        s_lastError = QStringLiteral("Base fermée.");
        return false;
    }
    const ResolvedQuaiSchema &s = schemaRef();
    if (!s.ok || s.quaiId.isEmpty()) {
        s_lastError = QStringLiteral("Table QUAI introuvable ou schéma illisible.");
        return false;
    }

    QStringList sets;
    QList<QVariant> vals;
    auto addSet = [&](const QString &c, const QVariant &v) {
        if (!c.isEmpty()) {
            sets << QStringLiteral("%1=?").arg(c);
            vals << v;
        }
    };
    addSet(s.quaiNom, m_quaiNom);
    addSet(s.quaiType, m_quaiType);
    addSet(s.longueur, m_longueur);
    addSet(s.profondeur, m_profondeur);
    addSet(s.capacite, m_capacite);
    addSet(s.statut, m_statut);
    addSet(s.currentUsage, m_currentUsage);
    addSet(s.occupancyRate, m_occupancyRate);
    addSet(s.priorityLevel, m_priorityLevel);
    addSet(s.safetyLevel, m_safetyLevel);
    addSet(s.lightingStatus, m_lightingStatus);

    if (sets.isEmpty()) {
        s_lastError = QStringLiteral("Aucune colonne à mettre à jour.");
        return false;
    }
    vals.append(m_quaiId);

    const QString sql = QStringLiteral("UPDATE QUAI SET %1 WHERE %2=?")
                            .arg(sets.join(QLatin1String(", ")), s.quaiId);

    QSqlQuery query(db);
    query.prepare(sql);
    for (const QVariant &v : vals)
        query.addBindValue(v);

    if (!query.exec()) {
        s_lastError = query.lastError().text();
        qDebug() << "Quai::modifier error:" << s_lastError;
        return false;
    }
    if (query.numRowsAffected() == 0) {
        s_lastError = QStringLiteral("Aucune ligne modifiée (ID %1 introuvable).").arg(m_quaiId);
        qDebug() << "Quai::modifier:" << s_lastError;
        return false;
    }
    db.commit();
    return true;
}

QSqlQuery Quai::afficher(const QString &searchText)
{
    QSqlDatabase db = QSqlDatabase::database(Connection::connectionName());
    QSqlQuery q(db);
    QString sql = QStringLiteral("SELECT %1 FROM QUAI WHERE 1=1").arg(sqlQuaiSelectList());
    if (!searchText.isEmpty()) {
        QString term = searchText;
        term.replace(QLatin1Char('\''), QLatin1String("''"));
        const QString likeLit = QStringLiteral("'%") + term + QStringLiteral("%'");
        const ResolvedQuaiSchema &s = schemaRef();
        QStringList ors;
        auto addLike = [&](const QString &col) {
            if (!col.isEmpty())
                ors << QStringLiteral("UPPER(") + col + QStringLiteral(") LIKE UPPER(") + likeLit + QLatin1Char(')');
        };
        addLike(s.quaiNom);
        addLike(s.quaiType);
        addLike(s.statut);
        addLike(s.safetyLevel);
        if (!ors.isEmpty())
            sql += QStringLiteral(" AND (") + ors.join(QStringLiteral(" OR ")) + QLatin1Char(')');
    }
    const QString orderCol = schemaRef().quaiId.isEmpty() ? QStringLiteral("QUAI_ID") : schemaRef().quaiId;
    sql += QStringLiteral(" ORDER BY ") + orderCol;
    q.exec(sql);
    return q;
}

bool Quai::supprimer(int id)
{
    s_lastError.clear();
    QSqlDatabase db = QSqlDatabase::database(Connection::connectionName());
    if (!db.isOpen())
        return false;
    const QString idCol = schemaRef().quaiId.isEmpty() ? QStringLiteral("QUAI_ID") : schemaRef().quaiId;
    QSqlQuery query(db);
    QString idStr = QString::number(id);
    query.exec(QStringLiteral("UPDATE BATEAU SET QUAI_ID = NULL WHERE %1 = %2").arg(idCol, idStr));
    if (!query.exec(QStringLiteral("DELETE FROM QUAI WHERE %1 = %2").arg(idCol, idStr))) {
        s_lastError = query.lastError().text();
        qDebug() << "Quai::supprimer error:" << s_lastError;
        return false;
    }
    if (query.numRowsAffected() == 0) {
        s_lastError = QStringLiteral("Aucune ligne supprimée (ID %1 non trouvé).").arg(id);
        qDebug() << "Quai::supprimer:" << s_lastError;
        return false;
    }
    db.commit();
    return true;
}

bool Quai::exportListePdf(const QString &pdfFilePath,
                          const QString &whereAndSuffix,
                          const QString &orderBySuffix,
                          int *rowCountOut)
{
    s_lastError.clear();
    if (rowCountOut)
        *rowCountOut = 0;
    if (pdfFilePath.trimmed().isEmpty()) {
        s_lastError = QStringLiteral("Chemin PDF vide.");
        return false;
    }

    QSqlDatabase db = QSqlDatabase::database(Connection::connectionName());
    if (!db.isOpen()) {
        s_lastError = QStringLiteral("Base fermée.");
        return false;
    }

    const QString sql = QStringLiteral("SELECT %1 FROM QUAI WHERE 1=1")
                            .arg(sqlQuaiSelectList())
                        + whereAndSuffix + orderBySuffix;

    QSqlQuery q(db);
    if (!q.exec(sql)) {
        s_lastError = q.lastError().text();
        return false;
    }

    constexpr int kQuaiExportCols = 12;
    const QStringList headers = {
        QStringLiteral("ID"),
        QStringLiteral("Nom"),
        QStringLiteral("Type"),
        QStringLiteral("Long. (m)"),
        QStringLiteral("Prof. (m)"),
        QStringLiteral("Capacité"),
        QStringLiteral("Statut"),
        QStringLiteral("Usage"),
        QStringLiteral("Taux occup.%"),
        QStringLiteral("Priorité"),
        QStringLiteral("Niveau séc."),
        QStringLiteral("Éclairage"),
    };

    auto cellText = [](const QVariant &v) -> QString {
        if (v.isNull())
            return QString();
        if (v.typeId() == QMetaType::Double)
            return QString::number(v.toDouble(), 'f', 2);
        return v.toString();
    };

    QString html;
    html += QStringLiteral("<h2 style='text-align:center;color:#2c3e50;'>%1</h2>")
                .arg(QStringLiteral("Liste des quais").toHtmlEscaped());
    html += QStringLiteral("<p style='text-align:center;color:#7f8c8d;font-size:10pt;'>%1</p>")
                .arg(QStringLiteral("Données issues de la table QUAI (filtres et tri appliqués). %1")
                         .arg(QLocale::system().toString(QDateTime::currentDateTime(), QLocale::ShortFormat))
                         .toHtmlEscaped());

    html += QStringLiteral(
        "<table border='1' cellpadding='6' cellspacing='0' "
        "style='width:100%;border-collapse:collapse;font-size:9pt;'>");

    html += QStringLiteral("<tr style='background-color:#3498db;color:white;'>");
    for (const QString &h : headers)
        html += QStringLiteral("<th>%1</th>").arg(h.toHtmlEscaped());
    html += QStringLiteral("</tr>");

    int rowCount = 0;
    while (q.next()) {
        ++rowCount;
        html += QStringLiteral("<tr>");
        for (int c = 0; c < kQuaiExportCols; ++c)
            html += QStringLiteral("<td>%1</td>").arg(cellText(q.value(c)).toHtmlEscaped());
        html += QStringLiteral("</tr>");
    }
    html += QStringLiteral("</table>");

    QPrinter printer(QPrinter::HighResolution);
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setOutputFileName(pdfFilePath);
    printer.setPageSize(QPageSize(QPageSize::A4));
    printer.setPageOrientation(QPageLayout::Landscape);
    printer.setPageMargins(QMarginsF(9, 9, 10, 10), QPageLayout::Millimeter);
    printer.setResolution(150);

    QTextDocument doc;
    doc.setHtml(html);
    doc.print(&printer);

    if (rowCountOut)
        *rowCountOut = rowCount;
    return true;
}
