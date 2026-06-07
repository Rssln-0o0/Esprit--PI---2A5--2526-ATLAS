#include "bateaudb.h"
#include "connection.h"

#include <QDebug>
#include <QVariant>

BateauDb::BateauDb()
{
    db = QSqlDatabase::database(Connection::connectionName());
    m_lastError.clear();
}

bool BateauDb::ajouter(int idBateau, const QString &nom, const QString &type, const QString &immatriculation,
                       const QString &puissanceMoteur, const QString &proprietaire, const QString &statut,
                       const QString &numeroLicense, const QDate &dateExpirationLicence,
                       int quaiId, int idPecheur, const QDate &assurance)
{
    QSqlQuery query(db);

    if (idBateau <= 0) {
        if (!query.exec("SELECT SEQ_BATEAU_ID.NEXTVAL FROM DUAL") || !query.next()) {
            m_lastError = QString("Impossible d'obtenir un ID (séquence SEQ_BATEAU_ID).");
            return false;
        }
        idBateau = query.value(0).toInt();
    }

    QString quaiVal = quaiId > 0 ? ":quai_id" : "NULL";
    QString pecheurVal = idPecheur > 0 ? ":id_pecheur" : "NULL";
    query.prepare(
        QString("INSERT INTO BATEAU (ID_BATEAU, NOM, TYPE, IMMATRICULATION, PUISSANCE_MOTEUR, PROPRIETAIRE, "
                "STATUT, NUMERO_LICENSE, DATE_EXPIRATION_LICENCE, QUAI_ID, ID_PECHEUR, ASSURANCE) "
                "VALUES (:id, :nom, :type, :immat, :puissance, :proprio, :statut, :num_lic, :date_exp_lic, %1, %2, :assurance)")
            .arg(quaiVal, pecheurVal));
    query.bindValue(":id", idBateau);
    query.bindValue(":nom", nom.trimmed());
    query.bindValue(":type", type.trimmed());
    query.bindValue(":immat", immatriculation.trimmed());
    query.bindValue(":puissance", puissanceMoteur.trimmed());
    query.bindValue(":proprio", proprietaire.trimmed());
    query.bindValue(":statut", statut.trimmed());
    query.bindValue(":num_lic", numeroLicense.trimmed());
    query.bindValue(":date_exp_lic", dateExpirationLicence.isValid() ? dateExpirationLicence : QVariant());
    if (quaiId > 0)
        query.bindValue(":quai_id", quaiId);
    if (idPecheur > 0)
        query.bindValue(":id_pecheur", idPecheur);
    query.bindValue(":assurance", assurance.isValid() ? assurance : QVariant());

    if (query.exec()) {
        m_lastError.clear();
        return true;
    }
    m_lastError = query.lastError().text();
    qDebug() << "BateauDb::ajouter erreur:" << m_lastError;
    return false;
}

bool BateauDb::supprimer(int idBateau)
{
    QSqlQuery query(db);
    query.prepare("DELETE FROM BATEAU WHERE ID_BATEAU = :id");
    query.bindValue(":id", idBateau);
    if (query.exec()) {
        m_lastError.clear();
        return true;
    }
    m_lastError = query.lastError().text();
    qDebug() << "BateauDb::supprimer erreur:" << m_lastError;
    return false;
}

bool BateauDb::modifier(int idBateau, const QString &nom, const QString &type, const QString &immatriculation,
                        const QString &puissanceMoteur, const QString &proprietaire, const QString &statut,
                        const QString &numeroLicense, const QDate &dateExpirationLicence,
                        int quaiId, int idPecheur, const QDate &assurance)
{
    QSqlQuery query(db);
    QString quaiVal = quaiId > 0 ? ":quai_id" : "NULL";
    QString pecheurVal = idPecheur > 0 ? ":id_pecheur" : "NULL";
    query.prepare(
        QString("UPDATE BATEAU SET NOM = :nom, TYPE = :type, IMMATRICULATION = :immat, PUISSANCE_MOTEUR = :puissance, "
                "PROPRIETAIRE = :proprio, STATUT = :statut, NUMERO_LICENSE = :num_lic, "
                "DATE_EXPIRATION_LICENCE = :date_exp_lic, QUAI_ID = %1, ID_PECHEUR = %2, ASSURANCE = :assurance "
                "WHERE ID_BATEAU = :id")
            .arg(quaiVal, pecheurVal));
    query.bindValue(":id", idBateau);
    query.bindValue(":nom", nom.trimmed());
    query.bindValue(":type", type.trimmed());
    query.bindValue(":immat", immatriculation.trimmed());
    query.bindValue(":puissance", puissanceMoteur.trimmed());
    query.bindValue(":proprio", proprietaire.trimmed());
    query.bindValue(":statut", statut.trimmed());
    query.bindValue(":num_lic", numeroLicense.trimmed());
    query.bindValue(":date_exp_lic", dateExpirationLicence.isValid() ? dateExpirationLicence : QVariant());
    if (quaiId > 0)
        query.bindValue(":quai_id", quaiId);
    if (idPecheur > 0)
        query.bindValue(":id_pecheur", idPecheur);
    query.bindValue(":assurance", assurance.isValid() ? assurance : QVariant());

    if (query.exec()) {
        m_lastError.clear();
        return true;
    }
    m_lastError = query.lastError().text();
    qDebug() << "BateauDb::modifier erreur:" << m_lastError;
    return false;
}

QSqlQuery BateauDb::afficher()
{
    QSqlQuery query(db);
    query.prepare(
        "SELECT ID_BATEAU, NOM, TYPE, IMMATRICULATION, PUISSANCE_MOTEUR, PROPRIETAIRE, STATUT, "
        "NUMERO_LICENSE, DATE_EXPIRATION_LICENCE, QUAI_ID, ID_PECHEUR, ASSURANCE "
        "FROM BATEAU ORDER BY ID_BATEAU");
    if (!query.exec())
        qDebug() << "BateauDb::afficher erreur:" << query.lastError().text();
    return query;
}

bool BateauDb::loadBateauGps(int idBateau, double *outLat, double *outLon, double *outCapDeg)
{
    if (!outLat || !outLon || !outCapDeg || idBateau <= 0)
        return false;
    QSqlQuery query(db);
    query.prepare(
        "SELECT LATITUDE, LONGITUDE, CAP_DERNIER FROM BATEAU WHERE ID_BATEAU = :id");
    query.bindValue(":id", idBateau);
    if (!query.exec()) {
        m_lastError = query.lastError().text();
        qDebug() << "BateauDb::loadBateauGps erreur:" << m_lastError;
        return false;
    }
    if (!query.next())
        return false;
    if (query.value(0).isNull() || query.value(1).isNull())
        return false;
    *outLat = query.value(0).toDouble();
    *outLon = query.value(1).toDouble();
    *outCapDeg = query.value(2).isNull() ? 0.0 : query.value(2).toDouble();
    m_lastError.clear();
    return true;
}

bool BateauDb::updateBateauGps(int idBateau, double lat, double lon, double capDeg)
{
    if (idBateau <= 0)
        return false;
    QSqlQuery query(db);
    query.prepare(
        "UPDATE BATEAU SET LATITUDE = :lat, LONGITUDE = :lon, CAP_DERNIER = :cap, "
        "DERNIER_FIX_GPS = SYSTIMESTAMP WHERE ID_BATEAU = :id");
    query.bindValue(":lat", lat);
    query.bindValue(":lon", lon);
    query.bindValue(":cap", capDeg);
    query.bindValue(":id", idBateau);
    if (query.exec()) {
        m_lastError.clear();
        return true;
    }
    m_lastError = query.lastError().text();
    qDebug() << "BateauDb::updateBateauGps erreur:" << m_lastError;
    return false;
}

bool BateauDb::appendGpsTracePoint(int idBateau, double lat, double lon)
{
    if (idBateau <= 0)
        return false;
    Q_UNUSED(lat);
    Q_UNUSED(lon);
    // Table BATEAU_GPS_TRACE supprimée: cette API devient un no-op de compatibilité.
    m_lastError.clear();
    return true;
}

QVector<QPointF> BateauDb::loadGpsTrace(int idBateau, int maxPoints)
{
    QVector<QPointF> out;
    if (idBateau <= 0 || maxPoints <= 0)
        return out;

    QSqlQuery query(db);
    query.prepare("SELECT LONGITUDE, LATITUDE FROM BATEAU WHERE ID_BATEAU = :id");
    query.bindValue(":id", idBateau);
    if (!query.exec()) {
        m_lastError = query.lastError().text();
        qDebug() << "BateauDb::loadGpsTrace erreur:" << m_lastError;
        return out;
    }
    if (query.next() && !query.value(0).isNull() && !query.value(1).isNull())
        out.append(QPointF(query.value(0).toDouble(), query.value(1).toDouble()));
    m_lastError.clear();
    return out;
}

bool BateauDb::clearGpsTrace(int idBateau)
{
    if (idBateau <= 0)
        return false;
    Q_UNUSED(idBateau);
    m_lastError.clear();
    return true;
}
