#ifndef BATEAUDB_H
#define BATEAUDB_H

#include <QDate>
#include <QPointF>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QString>
#include <QVector>

class BateauDb
{
public:
    BateauDb();

    // CRUD — table BATEAU (Oracle)
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

    /** Dernière position GPS en base (colonnes LATITUDE / LONGITUDE / CAP_DERNIER). */
    bool loadBateauGps(int idBateau, double *outLat, double *outLon, double *outCapDeg);

    /** Met à jour BATEAU + horodatage (SYSTIMESTAMP). */
    bool updateBateauGps(int idBateau, double lat, double lon, double capDeg);

    /** Compatibilité: conserve la signature, sans persistance d'historique séparé. */
    bool appendGpsTracePoint(int idBateau, double lat, double lon);

    /** Retourne au plus le dernier point (lon, lat) stocké dans BATEAU. */
    QVector<QPointF> loadGpsTrace(int idBateau, int maxPoints);

    /** Compatibilité: plus d’historique séparé, ne fait rien et retourne true. */
    bool clearGpsTrace(int idBateau);

    QString lastError() const { return m_lastError; }

private:
    QSqlDatabase db;
    QString m_lastError;
};

#endif
