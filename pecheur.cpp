#include "pecheur.h"
#include <QDebug>

static void appendPecheurSearchWhere(QString &sql, const QString &termUpper)
{
    if (termUpper.isEmpty())
        return;

    sql += QStringLiteral(
        " WHERE ("
        "UPPER(NVL(ID_PECHEUR, '')) LIKE :term OR "
        "UPPER(NVL(NOMPRENOM, '')) LIKE :term OR "
        "UPPER(NVL(ROLE, '')) LIKE :term OR "
        "UPPER(TO_CHAR(NVL(ANNEES_EXPERIENCE, 0))) LIKE :term OR "
        "UPPER(NVL(STATUT, '')) LIKE :term OR "
        "UPPER(NVL(CONTACT, '')) LIKE :term OR "
        "UPPER(NVL(RFID, '')) LIKE :term OR "
        "UPPER(TO_CHAR(DATE_LIMITE, 'DD/MM/YYYY')) LIKE :term"
        ")");
}

Pecheur::Pecheur()
{
    db = QSqlDatabase::database(Connection::connectionName());
    m_lastError.clear();
}

bool Pecheur::ajouter(QString id, QString nom, QString prenom, QString role,
                      QString statut, int experience, QDate dateLimite, QString telephone,
                      QString rfid, QString cin)
{
    // CRUD (DB): INSERT INTO PECHEUR ...
    QSqlQuery query(db);

    QString nomprenom = (nom + " " + prenom).trimmed();

    if (id.isEmpty()) {
        if (query.exec("SELECT seq_pecheur_id.NEXTVAL FROM DUAL") && query.next())
            id = query.value(0).toString();
        if (id.isEmpty()) {
            m_lastError = QString("Impossible d'obtenir un ID (séquence seq_pecheur_id).");
            return false;
        }
    }

    if (cin.isEmpty()) {
        QSqlQuery qcin(db);
        if (qcin.exec("SELECT CIN FROM employe WHERE ROWNUM = 1")) {
            if (qcin.next())
                cin = qcin.value(0).toString();
        }
        if (cin.isEmpty()) {
            m_lastError = QString("Aucun employé dans la base. Ajoutez d'abord au moins un employé (table employe).");
            qDebug() << m_lastError;
            return false;
        }
    }

    query.prepare("INSERT INTO PECHEUR (ID_PECHEUR, NOMPRENOM, ROLE, CONTACT, ANNEES_EXPERIENCE, STATUT, DATE_LIMITE, RFID, CIN) "
                  "VALUES (:id, :nomprenom, :role, :contact, :annees_experience, :statut, :date_limite, :rfid, :cin)");

    query.bindValue(":id", id);
    query.bindValue(":nomprenom", nomprenom);
    query.bindValue(":role", role);
    query.bindValue(":contact", telephone);
    query.bindValue(":annees_experience", experience);
    query.bindValue(":statut", statut);
    query.bindValue(":date_limite", dateLimite);
    query.bindValue(":rfid", rfid.trimmed().toUpper().remove(' '));
    query.bindValue(":cin", cin);

    if (query.exec()) {
        db.commit();
        m_lastError.clear();
        qDebug() << "Pêcheur ajouté avec succès";
        return true;
    } else {
        m_lastError = query.lastError().text();
        qDebug() << "Erreur lors de l'ajout:" << m_lastError;
        return false;
    }
}

bool Pecheur::supprimer(QString id)
{
    // CRUD (DB): DELETE FROM PECHEUR WHERE ID_PECHEUR = :id
    QSqlQuery query(db);

    query.prepare("DELETE FROM PECHEUR WHERE ID_PECHEUR = :id");
    query.bindValue(":id", id);

    if (query.exec()) {
        db.commit();
        qDebug() << "Pêcheur supprimé avec succès";
        return true;
    } else {
        qDebug() << "Erreur lors de la suppression:" << query.lastError().text();
        return false;
    }
}

bool Pecheur::modifier(QString id, QString nom, QString prenom, QString role,
                       QString statut, int experience, QDate dateLimite, QString telephone,
                       QString rfid, QString cin)
{
    // CRUD (DB): UPDATE PECHEUR SET ... WHERE ID_PECHEUR = :id
    QSqlQuery query(db);

    QString nomprenom = (nom + " " + prenom).trimmed();

    if (cin.isEmpty()) {
        query.prepare("UPDATE PECHEUR SET NOMPRENOM = :nomprenom, ROLE = :role, "
                      "CONTACT = :contact, ANNEES_EXPERIENCE = :annees_experience, STATUT = :statut, "
                      "DATE_LIMITE = :date_limite, RFID = :rfid WHERE ID_PECHEUR = :id");
    } else {
        query.prepare("UPDATE PECHEUR SET NOMPRENOM = :nomprenom, ROLE = :role, "
                      "CONTACT = :contact, ANNEES_EXPERIENCE = :annees_experience, STATUT = :statut, "
                      "DATE_LIMITE = :date_limite, RFID = :rfid, CIN = :cin WHERE ID_PECHEUR = :id");
    }

    query.bindValue(":id", id);
    query.bindValue(":nomprenom", nomprenom);
    query.bindValue(":role", role);
    query.bindValue(":contact", telephone);
    query.bindValue(":annees_experience", experience);
    query.bindValue(":statut", statut);
    query.bindValue(":date_limite", dateLimite);
    query.bindValue(":rfid", rfid.trimmed().toUpper().remove(' '));
    if (!cin.isEmpty())
        query.bindValue(":cin", cin);

    if (query.exec()) {
        db.commit();
        qDebug() << "Pêcheur modifié avec succès";
        return true;
    } else {
        qDebug() << "Erreur lors de la modification:" << query.lastError().text();
        return false;
    }
}

QSqlQuery Pecheur::afficher()
{
    // Reuse the same dynamic SQL path (search + sort) with default filters.
    return afficherAvecFiltres(QString(), -1);
}

QSqlQuery Pecheur::afficherAvecFiltres(const QString &searchText, int sortIndex)
{
    // Dynamic SQL entry-point used by Pecheur table display, search, sort, and export.
    QSqlQuery query(db);

    QString orderBy = QStringLiteral(" ORDER BY ID_PECHEUR");
    if (sortIndex == 0) {
        // Trier par nom/prenom (champ NOMPRENOM)
        orderBy = QStringLiteral(" ORDER BY UPPER(NOMPRENOM) ASC");
    } else if (sortIndex == 1) {
        // Trier par prenom: 2e mot du NOMPRENOM quand disponible
        orderBy = QStringLiteral(" ORDER BY UPPER(NVL(REGEXP_SUBSTR(TRIM(NOMPRENOM), '[^ ]+', 1, 2), NOMPRENOM)) ASC");
    } else if (sortIndex == 2) {
        orderBy = QStringLiteral(" ORDER BY ANNEES_EXPERIENCE ASC");
    } else if (sortIndex == 3) {
        orderBy = QStringLiteral(" ORDER BY DATE_LIMITE ASC NULLS LAST");
    }

    // Base SELECT from PECHEUR (database-side data source).
    QString sql = QStringLiteral(
        "SELECT ID_PECHEUR, NOMPRENOM, ROLE, ANNEES_EXPERIENCE, STATUT, DATE_LIMITE, CONTACT, RFID, CIN FROM PECHEUR");

    const QString term = searchText.trimmed().toUpper();
    appendPecheurSearchWhere(sql, term);

    // Dynamic sort (DB-side ORDER BY) selected from UI sort index.
    sql += orderBy;

    query.prepare(sql);
    if (!term.isEmpty()) {
        query.bindValue(QStringLiteral(":term"), QStringLiteral("%") + term + QStringLiteral("%"));
    }

    if (!query.exec()) {
        m_lastError = query.lastError().text();
        qDebug() << "Erreur lors de l'affichage:" << m_lastError;
    } else {
        m_lastError.clear();
    }

    return query;
}

QList<QPair<QString, int>> Pecheur::statistiquesParRole(const QString &searchText)
{
    QList<QPair<QString, int>> counts;
    QSqlQuery query(db);

    QString sql = QStringLiteral(
        "SELECT UPPER(TRIM(NVL(ROLE, ''))) AS ROLE_KEY, COUNT(*) "
        "FROM PECHEUR");

    const QString term = searchText.trimmed().toUpper();
    appendPecheurSearchWhere(sql, term);
    sql += QStringLiteral(" GROUP BY UPPER(TRIM(NVL(ROLE, ''))) ORDER BY ROLE_KEY");

    query.prepare(sql);
    if (!term.isEmpty())
        query.bindValue(QStringLiteral(":term"), QStringLiteral("%") + term + QStringLiteral("%"));

    if (!query.exec()) {
        m_lastError = query.lastError().text();
        qDebug() << "Erreur statistiques role:" << m_lastError;
        return counts;
    }

    m_lastError.clear();
    while (query.next()) {
        QString role = query.value(0).toString().trimmed();
        const int total = query.value(1).toInt();
        if (role.isEmpty())
            continue;
        role = role.toLower();
        if (role.length() > 1)
            role = role.at(0).toUpper() + role.mid(1);
        else
            role = role.toUpper();
        counts.append({role, total});
    }

    return counts;
}

QList<QPair<QString, int>> Pecheur::statistiquesParStatut(const QString &searchText)
{
    QList<QPair<QString, int>> counts;
    QSqlQuery query(db);

    QString sql = QStringLiteral(
        "SELECT UPPER(TRIM(NVL(STATUT, ''))) AS STATUT_KEY, COUNT(*) "
        "FROM PECHEUR");

    const QString term = searchText.trimmed().toUpper();
    appendPecheurSearchWhere(sql, term);
    sql += QStringLiteral(" GROUP BY UPPER(TRIM(NVL(STATUT, ''))) ORDER BY STATUT_KEY");

    query.prepare(sql);
    if (!term.isEmpty())
        query.bindValue(QStringLiteral(":term"), QStringLiteral("%") + term + QStringLiteral("%"));

    if (!query.exec()) {
        m_lastError = query.lastError().text();
        qDebug() << "Erreur statistiques statut:" << m_lastError;
        return counts;
    }

    m_lastError.clear();
    while (query.next()) {
        QString statut = query.value(0).toString().trimmed();
        const int total = query.value(1).toInt();
        if (statut.isEmpty())
            continue;

        if (statut.compare(QStringLiteral("ACTIF"), Qt::CaseInsensitive) == 0)
            statut = QStringLiteral("Actif");
        else if (statut.compare(QStringLiteral("INACTIF"), Qt::CaseInsensitive) == 0)
            statut = QStringLiteral("Inactif");
        else {
            statut = statut.toLower();
            if (statut.length() > 1)
                statut = statut.at(0).toUpper() + statut.mid(1);
            else
                statut = statut.toUpper();
        }
        counts.append({statut, total});
    }

    return counts;
}
