#include "rfid_pecheur.h"

#include <QSqlError>
#include <QSqlQuery>
#include <QSqlRecord>

namespace {

QString splitNom(const QString &nomPrenom)
{
    const QString full = nomPrenom.trimmed();
    const int idx = full.indexOf(' ');
    return idx > 0 ? full.left(idx).trimmed() : full;
}

QString splitPrenom(const QString &nomPrenom)
{
    const QString full = nomPrenom.trimmed();
    const int idx = full.indexOf(' ');
    return idx > 0 ? full.mid(idx + 1).trimmed() : QString();
}

QString oneRowSuffix(const QSqlDatabase &db)
{
    const QString drv = db.driverName();
    if (drv.contains(QStringLiteral("OCI"), Qt::CaseInsensitive)
        || drv.contains(QStringLiteral("ODBC"), Qt::CaseInsensitive)) {
        return QStringLiteral(" AND ROWNUM <= 1");
    }
    return QStringLiteral(" LIMIT 1");
}

} // namespace

QString RfidPecheur::normalizeUid(const QString &uidRaw)
{
    QString uid = uidRaw.trimmed().toUpper();
    uid.remove(QLatin1Char(' '));
    uid.remove(QLatin1Char('-'));
    return uid;
}

bool RfidPecheur::lookupByRfid(QSqlDatabase &db, const QString &uidRaw, PecheurRfidRecord *out, QString *errorMessage)
{
    if (!out) {
        if (errorMessage)
            *errorMessage = QStringLiteral("Destination invalide.");
        return false;
    }
    *out = PecheurRfidRecord{};

    if (!db.isValid() || !db.isOpen()) {
        if (errorMessage)
            *errorMessage = QStringLiteral("Connexion base de données indisponible.");
        return false;
    }

    const QString uid = normalizeUid(uidRaw);
    if (uid.isEmpty()) {
        if (errorMessage)
            *errorMessage = QStringLiteral("UID RFID vide.");
        return false;
    }

    QSqlQuery probe(db);
    if (!probe.exec(QStringLiteral("SELECT * FROM PECHEUR WHERE 1=0"))) {
        if (errorMessage)
            *errorMessage = probe.lastError().text();
        return false;
    }
    const QSqlRecord rec = probe.record();
    const int rfidIx = rec.indexOf(QStringLiteral("RFID"));
    if (rfidIx < 0) {
        if (errorMessage)
            *errorMessage = QStringLiteral("La colonne PECHEUR.RFID est absente.");
        return false;
    }

    QString sql = QStringLiteral(
        "SELECT ID_PECHEUR, NOMPRENOM, ROLE, ANNEES_EXPERIENCE, STATUT, DATE_LIMITE, CONTACT, RFID "
        "FROM PECHEUR "
        "WHERE UPPER(REPLACE(NVL(RFID, ''), ' ', '')) = :rfid");
    sql += oneRowSuffix(db);

    QSqlQuery q(db);
    q.prepare(sql);
    q.bindValue(QStringLiteral(":rfid"), uid);
    if (!q.exec()) {
        if (errorMessage)
            *errorMessage = q.lastError().text();
        return false;
    }
    if (!q.next())
        return false;

    const QString nomPrenom = q.value(1).toString();
    out->id = q.value(0).toString();
    out->nom = splitNom(nomPrenom);
    out->prenom = splitPrenom(nomPrenom);
    out->role = q.value(2).toString().trimmed();
    out->experience = q.value(3).toInt();
    out->statut = q.value(4).toString().trimmed();
    out->dateLimite = q.value(5).toDate();
    out->telephone = q.value(6).toString().trimmed();
    out->rfid = normalizeUid(q.value(7).toString());
    if (out->rfid.isEmpty())
        out->rfid = uid;

    if (errorMessage)
        errorMessage->clear();
    return true;
}
