#ifndef ESP32_GPS_H
#define ESP32_GPS_H

#include <QByteArray>
#include <QHash>
#include <QNetworkAccessManager>
#include <QObject>
#include <QPixmap>
#include <QPointF>
#include <QSet>
#include <QString>
#include <QVector>
#include <QWidget>

class QNetworkReply;
class QPaintEvent;
class QResizeEvent;
class QWheelEvent;
class QMouseEvent;
class QTimer;
class QTcpSocket;

/**
 * Réception GPS via **Wi‑Fi TCP** (ESP32 sur le même réseau que le PC).
 * Trames : ATLAS_GPS ou NMEA $GPRMC / $GNRMC / $GPGGA (point décimal, locale C côté Qt).
 */
class Esp32GpsTcpClient : public QObject
{
    Q_OBJECT

public:
    explicit Esp32GpsTcpClient(QObject *parent = nullptr);
    ~Esp32GpsTcpClient() override;

    void connectToTcpHost(const QString &host, quint16 port);
    void disconnectDevice();
    bool isConnected() const;

    bool hasRecentPosition(qint64 maxAgeMs = 8000) const;

signals:
    void positionUpdated(double lat, double lon, double headingDeg);
    void statusMessage(const QString &text);
    void logLine(const QString &line);
    void tcpLinkOpened();

private slots:
    void onTcpReadyRead();
    void onTcpConnected();
    void onTcpSocketError();

private:
    QTcpSocket *m_socket = nullptr;
    QByteArray m_rxBuffer;
    qint64 m_lastGpsFixMs = 0;
};

/**
 * Carte : image **Google Static Maps** si une clé API est enregistrée (QSettings
 * `Esp32Gps/googleMapsApiKey` ou variable `GOOGLE_MAPS_API_KEY`), sinon tuiles CARTO/OSM.
 * Activez « Maps Static API » sur Google Cloud. Zoom molette, déplacement clic gauche.
 */
class GpsWorldMapWidget : public QWidget
{
    Q_OBJECT

public:
    explicit GpsWorldMapWidget(QWidget *parent = nullptr);

    void setGpsPosition(double lat, double lon, double headingDeg);
    /** Piste (lon, lat) + position courante ; utilisé au chargement depuis Oracle. */
    void loadPersistedGpsState(const QVector<QPointF> &trailLonLat, bool hasBoatPosition,
                                double boatLat, double boatLon, double headingDeg);
    void clearTrail();
    /** Efface position live et piste (ex. à la connexion USB avant réception ATLAS_GPS). */
    void clearLiveGps();
    void centerOnBoat();
    void setFollowGps(bool on);

    bool followGps() const { return m_followGps; }

protected:
    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

private slots:
    void onNetworkFinished(QNetworkReply *reply);
    void requestGoogleStaticMap();

private:
    struct WorldPx {
        double x = 0;
        double y = 0;
    };

    static WorldPx geoToWorldPx(double lat, double lon, int zoom);
    static void worldPxToGeo(double wx, double wy, int zoom, double &lat, double &lon);
    QPointF worldToWidget(double wx, double wy) const;
    WorldPx widgetToWorld(const QPointF &widgetPt) const;

    void requestTile(int z, int tx, int ty);
    QString tileKey(int z, int tx, int ty) const;
    void pruneTileCache(const QSet<QString> &keepNearby);
    void scheduleGoogleRefresh();
    void paintTrailAndBoat(QPainter &p, int zoomLevel);

    QNetworkAccessManager *m_nam = nullptr;
    QHash<QString, QPixmap> m_tileCache;
    QSet<QString> m_pendingKeys;
    QHash<QNetworkReply *, QString> m_replyToKey;
    /** Après échec réseau / décodage : ne pas relancer la même tuile à chaque paint. */
    QHash<QString, qint64> m_tileCooldownUntilMs;

    QString m_googleApiKey;
    bool m_googleMode = false;
    QPixmap m_googlePixmap;
    QNetworkReply *m_staticReply = nullptr;
    QTimer *m_refreshTimer = nullptr;

    double m_centerLat = 36.805;
    double m_centerLon = 10.175;
    double m_centerWorldX = 0;
    double m_centerWorldY = 0;
    int m_zoom = 6;

    bool m_dragging = false;
    QPoint m_lastPanPos;

    bool m_hasGps = false;
    double m_boatLat = 0;
    double m_boatLon = 0;
    double m_boatHeadingDeg = 0;

    QVector<QPointF> m_trail;
    bool m_followGps = true;

    static constexpr int kTileSize = 256;
    static constexpr int kMaxTrailPoints = 4000;
    static constexpr double kTrailMinMeters = 8.0;
};

#endif
