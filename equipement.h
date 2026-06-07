#ifndef EQUIPEMENT_H
#define EQUIPEMENT_H

#include <QString>
#include <QSqlQuery>
#include <QSqlDatabase>
#include <QDate>
#include <QMap>
#include "connection.h"

class Equipement
{
public:
    Equipement();

    // CRUD — table EQUIPEMENT (ID_EQUIPEMENT, NOM, TYPE, ETAT, DATE_ACQUISITION,
    // LOCALISATION, COUT_ESTIME, FOURNISSEUR, DISPONIBILITE, ID_BATEAU)
    // idEquip: 0 = use SEQ_EQUIPEMENT_ID.NEXTVAL for insert
    bool ajouter(int idEquip, const QString &nom, const QString &type, const QString &etat,
                 const QDate &dateAcquisition, const QString &localisation, double coutEstime,
                 const QString &fournisseur, const QString &disponibilite, int idBateau);

    bool supprimer(int idEquip);

    bool modifier(int idEquip, const QString &nom, const QString &type, const QString &etat,
                  const QDate &dateAcquisition, const QString &localisation, double coutEstime,
                  const QString &fournisseur, const QString &disponibilite, int idBateau);

    QSqlQuery afficher(const QString &orderBy = "ID_EQUIPEMENT");

    // Search: nom, type, fournisseur, localisation (LIKE)
    QSqlQuery rechercher(const QString &motCle, const QString &orderBy = "ID_EQUIPEMENT");

    /**
     * Export PDF depuis la table EQUIPEMENT (requête SQL, pas le QTableWidget).
     * @param searchMotCle vide = tous les enregistrements ; sinon même filtre que rechercher()
     * @param orderBy colonne de tri (validée comme afficher/rechercher)
     */
    bool exportListePdf(const QString &pdfFilePath,
                        const QString &searchMotCle = QString(),
                        const QString &orderBy = QStringLiteral("ID_EQUIPEMENT"),
                        int *rowCountOut = nullptr);

    // Statistics aggregate data
    QMap<QString, int> getCountByType();
    QMap<QString, int> getCountByStatus();
    double getTotalValue();
    int getTotalCount();

    // Smart Maintenance Logic
    QSqlQuery getMaintenanceAlerts();
    bool updateStatus(int idEquip, const QString &newStatus);
    int getAlertCount();

    QString lastError() const { return m_lastError; }

private:
    QSqlDatabase db;
    QString m_lastError;
};

#endif // EQUIPEMENT_H
