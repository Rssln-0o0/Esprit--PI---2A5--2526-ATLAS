#include "bateau.h"
#include <QDebug>
#include <QVariant>

Bateau::Bateau()
{
    db = QSqlDatabase::database(Connection::connectionName());
    m_lastError.clear();
}

bool Bateau::ajouter(int idBateau, const QString &nom, const QString &type, const QString &immatriculation,
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

    // Oracle ODBC: do not bind QVariant() for NUMBER columns (causes ORA-00932 BINARY). Use literal NULL in SQL.
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
    if (quaiId > 0) query.bindValue(":quai_id", quaiId);
    if (idPecheur > 0) query.bindValue(":id_pecheur", idPecheur);
    query.bindValue(":assurance", assurance.isValid() ? assurance : QVariant());

    if (query.exec()) {
        m_lastError.clear();
        return true;
    }
    m_lastError = query.lastError().text();
    qDebug() << "Bateau::ajouter erreur:" << m_lastError;
    return false;
}

bool Bateau::supprimer(int idBateau)
{
    QSqlQuery query(db);
    query.prepare("DELETE FROM BATEAU WHERE ID_BATEAU = :id");
    query.bindValue(":id", idBateau);
    if (query.exec()) {
        m_lastError.clear();
        return true;
    }
    m_lastError = query.lastError().text();
    qDebug() << "Bateau::supprimer erreur:" << m_lastError;
    return false;
}

bool Bateau::modifier(int idBateau, const QString &nom, const QString &type, const QString &immatriculation,
                     const QString &puissanceMoteur, const QString &proprietaire, const QString &statut,
                     const QString &numeroLicense, const QDate &dateExpirationLicence,
                     int quaiId, int idPecheur, const QDate &assurance)
{
    QSqlQuery query(db);
    // Oracle ODBC: use literal NULL for optional NUMBER columns to avoid ORA-00932 (expected NUMBER got BINARY).
    QString quaiVal = quaiId > 0 ? ":quai_id" : "NULL";
    QString pecheurVal = idPecheur > 0 ? ":id_pecheur" : "NULL";
    query.prepare(
        QString("UPDATE BATEAU SET NOM = :nom, TYPE = :type, IMMATRICULATION = :immat, PUISSANCE_MOTEUR = :puissance, "
                "PROPRIETAIRE = :proprio, STATUT = :statut, NUMERO_LICENSE = :num_lic, "
                "DATE_EXPIRATION_LICENCE = :date_exp_lic, QUAI_ID = %1, ID_PECHEUR = %2, ASSURANCE = :assurance "
                "WHERE ID_BATEAU = :id").arg(quaiVal, pecheurVal));
    query.bindValue(":id", idBateau);
    query.bindValue(":nom", nom.trimmed());
    query.bindValue(":type", type.trimmed());
    query.bindValue(":immat", immatriculation.trimmed());
    query.bindValue(":puissance", puissanceMoteur.trimmed());
    query.bindValue(":proprio", proprietaire.trimmed());
    query.bindValue(":statut", statut.trimmed());
    query.bindValue(":num_lic", numeroLicense.trimmed());
    query.bindValue(":date_exp_lic", dateExpirationLicence.isValid() ? dateExpirationLicence : QVariant());
    if (quaiId > 0) query.bindValue(":quai_id", quaiId);
    if (idPecheur > 0) query.bindValue(":id_pecheur", idPecheur);
    query.bindValue(":assurance", assurance.isValid() ? assurance : QVariant());

    if (query.exec()) {
        m_lastError.clear();
        return true;
    }
    m_lastError = query.lastError().text();
    qDebug() << "Bateau::modifier erreur:" << m_lastError;
    return false;
}

QSqlQuery Bateau::afficher()
{
    QSqlQuery query(db);
    query.prepare(
        "SELECT ID_BATEAU, NOM, TYPE, IMMATRICULATION, PUISSANCE_MOTEUR, PROPRIETAIRE, STATUT, "
        "NUMERO_LICENSE, DATE_EXPIRATION_LICENCE, QUAI_ID, ID_PECHEUR, ASSURANCE "
        "FROM BATEAU ORDER BY ID_BATEAU");
    if (!query.exec())
        qDebug() << "Bateau::afficher erreur:" << query.lastError().text();
    return query;
}
