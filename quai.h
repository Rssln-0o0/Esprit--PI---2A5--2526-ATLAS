#ifndef QUAI_H
#define QUAI_H

#include <QtGlobal>
#include <QSqlQuery>
#include <QSqlDatabase>
#include <QString>
#include <QVector>
#include <QPair>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

/**
 * Classe métier QUAI — Module Quai (Rayen).
 * Les noms de colonnes sont résolus au runtime (métadonnées ODBC / dictionnaire Oracle)
 * pour supporter à la fois l’ancien schéma (CURRENTUSAGE, SAFETYLEVEL, …) et le schéma
 * avec underscores (CURRENT_USAGE, SAFETY_LEVEL, LIGHTING_STATUS, …).
 */
class Quai
{
public:
    Quai();

    /** 12 expressions pour SELECT … FROM QUAI (ordre fixe pour le tableau Qt). */
    static QString sqlQuaiSelectList();

    /** Identifiants SQL réels (majuscules Oracle) — chaîne vide si la colonne n’existe pas. */
    static QString sqlQuaiColQuaiId();
    static QString sqlQuaiColQuaiNom();
    static QString sqlQuaiColQuaiType();
    static QString sqlQuaiColStatut();
    static QString sqlQuaiColLongueur();
    static QString sqlQuaiColProfondeur();
    static QString sqlQuaiColCapacite();
    static QString sqlQuaiColCurrentUsage();
    static QString sqlQuaiColOccupancyRate();
    static QString sqlQuaiColPriorityLevel();
    static QString sqlQuaiColSafetyLevel();
    static QString sqlQuaiColLightingStatus();
    static QString sqlQuaiColIncidentLog();

    /** Ajoute une ligne horodatée dans INCIDENT_LOG (si la colonne existe). */
    static bool appendIncidentLog(int quaiId, const QString &line);
    /** Vide le journal d'incidents (INCIDENT_LOG) pour un quai. */
    static bool clearIncidentLog(int quaiId);
    /** Retire l'état maintenance en base : type Maintenance → Pêche, statut contenant maintenance → Actif. */
    static bool endMaintenanceAlert(int quaiId);
    /** Nombre de quais considérés actifs (hors inactif / maintenance). */
    static int countActiveQuais();
    /** Nombre total de lignes QUAI (pour affichage actifs vs hors pool). */
    static int countTotalQuais();
    /** True s'il existe au moins un quai actif. */
    static bool hasAnyActiveQuai();
    /** Met à jour uniquement le statut (colonne résolue) pour un quai. */
    static bool setStatutForQuaiId(int quaiId, const QString &statut);
    /** Quais actifs (même règle que countActiveQuais) : paires (id, nom affichable). */
    static QVector<QPair<int, QString>> activeQuaisIdAndLabel();
    /** Quais occupés (statut inactif, hors maintenance) : pour capteur « sortie » → repasser en Actif. */
    static QVector<QPair<int, QString>> occupiedQuaisIdAndLabel();

    // CRUD
    bool ajouter();
    bool modifier();
    bool supprimer(int id);
    /** Lecture : retourne la requête SELECT pour remplir le tableau. */
    QSqlQuery afficher(const QString &searchText = QString());

    /** Prochain ID (séquence SEQ_QUAI_ID). */
    static int nextId();
    /** Dernier message d'erreur après une opération échouée. */
    static QString lastErrorMessage();

    /**
     * Exporte la table QUAI en PDF (requête SQL côté base, pas depuis un QTableWidget).
     * @param pdfFilePath chemin du fichier .pdf à créer
     * @param whereAndSuffix suffixe après WHERE 1=1 (ex. chaîne renvoyée par MainWindow::quaiWhereClause())
     * @param orderBySuffix suffixe ORDER BY (ex. MainWindow::quaiOrderClause())
     * @param rowCountOut si non nul, reçoit le nombre de lignes données exportées
     */
    static bool exportListePdf(const QString &pdfFilePath,
                             const QString &whereAndSuffix,
                             const QString &orderBySuffix,
                             int *rowCountOut = nullptr);

    // Setters (liaison formulaire)
    void setQuaiId(int id) { m_quaiId = id; }
    void setQuaiNom(const QString &s) { m_quaiNom = s; }
    void setQuaiType(const QString &s) { m_quaiType = s; }
    void setLongueur(double v) { m_longueur = v; }
    void setProfondeur(double v) { m_profondeur = v; }
    void setCapacite(int v) { m_capacite = v; }
    void setStatut(const QString &s) { m_statut = s; }
    void setCurrentUsage(int v) { m_currentUsage = v; }
    void setOccupancyRate(double v) { m_occupancyRate = v; }
    void setPriorityLevel(const QString &s) { m_priorityLevel = s; }
    void setSafetyLevel(const QString &s) { m_safetyLevel = s; }
    void setLightingStatus(const QString &s) { m_lightingStatus = s; }

private:
    int m_quaiId = 0;
    QString m_quaiNom;
    QString m_quaiType;
    double m_longueur = 0;
    double m_profondeur = 0;
    int m_capacite = 0;
    QString m_statut;
    int m_currentUsage = 0;
    double m_occupancyRate = 0;
    QString m_priorityLevel;
    QString m_safetyLevel;
    QString m_lightingStatus;

    static QString s_lastError;
};

#endif // QUAI_H
