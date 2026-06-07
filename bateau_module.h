#ifndef BATEAU_MODULE_H
#define BATEAU_MODULE_H

#include <QColor>
#include <QObject>
#include <QList>
#include <QString>
#include <QDate>
#include <QHash>
#include <QVector>

class QLabel;
class QComboBox;
class QPushButton;
class QTimer;
class QStandardItemModel;
class QChartView;
class QDateEdit;
class QWidget;
class QPlainTextEdit;
class QLineEdit;
class QSpinBox;

class GpsWorldMapWidget;
class Esp32GpsTcpClient;
class BateauDb;

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

struct BateauData {
    QString id;
    QString name;
    QString type;
    QString owner;
    QString immatriculation;
    QString motor;
    QString licenseNumber;
    QDate licenseExpiry;
    QDate insuranceExpiry;
    bool isAtSea;
};

struct BateauTrackSnapshot {
    QString id;
    QString name;
    QString type;
    QString immat;
    double lat = 0;
    double lon = 0;
    double headingDeg = 0;
    double sogKnots = 0;
    bool atSea = false;
    qint64 lastAisEpochMs = 0;
};

struct BateauSimTrack {
    double lat = 0;
    double lon = 0;
    double headingDeg = 0;
    double sogKnots = 0;
    qint64 lastAisEpochMs = 0;
    bool initialized = false;
    bool lastDbAtSea = false;
};

class BateauModule : public QObject
{
    Q_OBJECT

public:
    explicit BateauModule(Ui::MainWindow *ui, BateauDb *dbAccess, QWidget *dialogParent, QObject *parent = nullptr);
    ~BateauModule() override;

    void initAfterSetupUi();
    void reloadFromDatabase();

    void onAjouterClicked();
    void onModifierClicked();
    void onSupprimerClicked();
    void onExportPdfClicked();
    void onRechercheTextChanged(const QString &text);
    void onTriIndexChanged(int index);
    void onBoatRapportClicked();
    void onIASendClicked();
    void onIAReportClicked();

private slots:
    void applyGpsPositionUi(double lat, double lon, double heading);
    void onGpsLinkOpened();

private:
    void initializeModels();
    void initializeTimers();
    void loadSampleDataIfEmpty();
    void refreshFromDb();
    void updateTable();
    void updateStatistics();
    void updateBlinkEffect();
    QLabel *addErrorLabel(QWidget *field, const QString &errorText);
    int activeLicenses() const;
    int expiredLicenses() const;
    int atSeaCount() const;
    void clearForm();
    QString generateAiReport(const QString &boatId);

    void setupGeolocationTab();
    void persistEsp32GpsForBoat1(double lat, double lon, double capDeg);
    void loadGpsFromDbOntoMap();
    void ensureFleetTracksForBoats();
    void advanceFleetSimulation();
    void syncGeolocationViews();
    QString evaluateFleetMetierAlerts() const;
    QColor colorForBoatType(const QString &type) const;
    QString sqlBateauTable() const;

    Ui::MainWindow *m_ui;
    BateauDb *m_bateauDb;
    QWidget *m_dialogParent;

    QList<BateauData> m_boats;
    QStandardItemModel *m_model;
    QTimer *m_statsTimer;
    QDateEdit *m_dateTechVisit;
    QDateEdit *m_dateInsurance;
    QChartView *m_chartType;
    QChartView *m_chartStatus;
    QChartView *m_chartLicense;
    QTimer *m_blinkTimer;
    bool m_blinkState;
    QLabel *m_errName;
    QLabel *m_errImmat;
    QLabel *m_errLicense;
    QLabel *m_errOwner;

    QWidget *m_geoPage;
    GpsWorldMapWidget *m_gpsWorldMap;
    Esp32GpsTcpClient *m_gpsTcp;
    QLabel *m_lblGeoWgs84;
    QLabel *m_lblGeoHover;
    QLineEdit *m_editGpsWifiHost = nullptr;
    QSpinBox *m_spinGpsWifiPort = nullptr;
    QPushButton *m_btnGpsWifiConnect = nullptr;
    QPushButton *m_btnGpsWifiDisconnect = nullptr;
    QPushButton *m_btnGpsClearTrail = nullptr;
    QPlainTextEdit *m_txtAisLog;
    /** Texte d’aide sous la carte (réutilisé avec les alertes flotte quand le TCP est inactif). */
    QString m_geoHoverHelpHtml;
    QTimer *m_fleetTimer;
    QVector<BateauTrackSnapshot> m_trackSnapshots;
    QHash<QString, BateauSimTrack> m_simByBoatId;
    bool m_geoTabReady;

    /** ESP32 → enregistrement sous ID_BATEAU = 1. */
    static constexpr int kEsp32GpsBoatId = 1;
    double m_lastTracePersistLat = 0.0;
    double m_lastTracePersistLon = 0.0;
    bool m_gpsTracePrimed = false;
};

#endif
