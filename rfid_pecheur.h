#ifndef RFID_PECHEUR_H
#define RFID_PECHEUR_H

#include <QDate>
#include <QSqlDatabase>
#include <QString>

struct PecheurRfidRecord
{
    QString id;
    QString nom;
    QString prenom;
    QString role;
    QString statut;
    int experience = 0;
    QDate dateLimite;
    QString telephone;
    QString rfid;
};

class RfidPecheur
{
public:
    static QString normalizeUid(const QString &uidRaw);
    static bool lookupByRfid(QSqlDatabase &db, const QString &uidRaw, PecheurRfidRecord *out, QString *errorMessage = nullptr);
};

#endif // RFID_PECHEUR_H
