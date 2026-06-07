#ifndef BATEAU_H
#define BATEAU_H

#include <QString>
#include <QSqlQuery>
#include <QSqlDatabase>
#include <QDate>
#include "connection.h"

class Bateau
{
public:
    Bateau();

    // CRUD — table BATEAU (ID_BATEAU, NOM, TYPE, IMMATRICULATION, PUISSANCE_MOTEUR, PROPRIETAIRE,
    // STATUT, NUMERO_LICENSE, DATE_EXPIRATION_LICENCE, QUAI_ID, ID_PECHEUR, ASSURANCE)
    // idBateau: 0 = use SEQ_BATEAU_ID.NEXTVAL for insert
    bool ajouter(int idBateau, const QString &nom, const QString &type, const QString &immatriculation,
                 const QString &puissanceMoteur, const QString &proprietaire, const QString &statut,
                 const QString &numeroLicense, const QDate &dateExpirationLicence,
                 int quaiId, int idPecheur, const QDate &assurance);

    bool supprimer(int idBateau);

    bool modifier(int idBateau, const QString &nom, const QString &type, const QString &immatriculation,
                  const QString &puissanceMoteur, const QString &proprietaire, const QString &statut,
                  const QString &numeroLicense, const QDate &dateExpirationLicence,
                  int quaiId, int idPecheur, const QDate &assurance);

    QSqlQuery afficher();

    QString lastError() const { return m_lastError; }

private:
    QSqlDatabase db;
    QString m_lastError;
};

#endif // BATEAU_H
