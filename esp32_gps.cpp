#include "esp32_gps.h"

#include <QFont>
#include <QMouseEvent>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QPaintEvent>
#include <QPainter>
#include <QPainterPath>
#include <QPen>
#include <QPolygonF>
#include <QResizeEvent>
#include <QSizePolicy>
#include <QTimer>
#include <QUrl>
#include <QUrlQuery>
#include <QWheelEvent>
#include <QImage>
#include <QtMath>

#include <QAbstractSocket>
#include <QDateTime>
#include <QIODevice>
#include <QLocale>
#include <QTcpSocket>
#include <QSettings>

#include <cmath>

#include <functional>

namespace {

static QString normalizedLineForAtlas(const QString &s)
{
    QString t = s.trimmed();
    if (t.startsWith(QChar(0xFEFF)))
        t.remove(0, 1);
    const int ix = t.indexOf(QStringLiteral("ATLAS_GPS"));
    if (ix >= 0)
        t = t.mid(ix).trimmed();
    return t;
}

static bool parseAtlasGpsFields(const QString &atlasLine, double *outLat, double *outLon, double *outHeading)
{
    if (!outLat || !outLon || !outHeading)
        return false;
    if (!atlasLine.startsWith(QStringLiteral("ATLAS_GPS")))
        return false;

    const QStringList p = atlasLine.split(QLatin1Char(','));
    if (p.size() < 3)
        return false;

    const QLocale c(QLocale::C);
    bool okLat = false;
    bool okLon = false;
    const double lat = c.toDouble(p.at(1).trimmed(), &okLat);
    const double lon = c.toDouble(p.at(2).trimmed(), &okLon);
    if (!okLat || !okLon || lat < -90.0 || lat > 90.0 || lon < -180.0 || lon > 180.0)
        return false;

    double heading = 0;
    if (p.size() >= 4) {
        bool okH = false;
        heading = c.toDouble(p.at(3).trimmed(), &okH);
        if (!okH)
            heading = 0;
    }
    *outLat = lat;
    *outLon = lon;
    *outHeading = heading;
    return true;
}

static QString stripNmeaChecksum(const QString &s)
{
    QString t = s.trimmed();
    const int ix = t.indexOf(QLatin1Char('*'));
    if (ix >= 0)
        t = t.left(ix);
    return t;
}

static double nmeaLatitudeToDeg(const QString &dm, const QString &hem)
{
    const QLocale c(QLocale::C);
    bool ok = false;
    const double v = c.toDouble(dm.trimmed(), &ok);
    if (!ok || dm.trimmed().length() < 2)
        return 999.0;
    const int deg = int(v / 100.0);
    const double minutes = v - double(deg) * 100.0;
    double dec = double(deg) + minutes / 60.0;
    if (hem.compare(QLatin1Char('S'), Qt::CaseInsensitive) == 0)
        dec = -dec;
    return dec;
}

static double nmeaLongitudeToDeg(const QString &dm, const QString &hem)
{
    const QLocale c(QLocale::C);
    bool ok = false;
    const double v = c.toDouble(dm.trimmed(), &ok);
    if (!ok || dm.trimmed().length() < 3)
        return 999.0;
    const int deg = int(v / 100.0);
    const double minutes = v - double(deg) * 100.0;
    double dec = double(deg) + minutes / 60.0;
    if (hem.compare(QLatin1Char('W'), Qt::CaseInsensitive) == 0)
        dec = -dec;
    return dec;
}

static bool parseRmcLine(const QString &line, double *lat, double *lon, double *course)
{
    const QString s = stripNmeaChecksum(line);
    if (!s.startsWith(QLatin1String("$GPRMC"), Qt::CaseInsensitive)
        && !s.startsWith(QLatin1String("$GNRMC"), Qt::CaseInsensitive))
        return false;

    const QStringList p = s.split(QLatin1Char(','));
    if (p.size() < 10)
        return false;
    if (p.at(2) != QLatin1String("A"))
        return false;

    const double la = nmeaLatitudeToDeg(p.at(3), p.at(4));
    const double lo = nmeaLongitudeToDeg(p.at(5), p.at(6));
    if (la < -90.0 || la > 90.0 || lo < -180.0 || lo > 180.0)
        return false;

    bool okC = false;
    double co = 0;
    if (p.size() > 8 && !p.at(8).trimmed().isEmpty())
        co = QLocale(QLocale::C).toDouble(p.at(8).trimmed(), &okC);
    if (!okC)
        co = 0;

    *lat = la;
    *lon = lo;
    *course = co;
    return true;
}

static bool parseGgaLine(const QString &line, double *lat, double *lon, double *course)
{
    *course = 0;
    const QString s = stripNmeaChecksum(line);
    if (!s.startsWith(QLatin1String("$GPGGA"), Qt::CaseInsensitive)
        && !s.startsWith(QLatin1String("$GNGGA"), Qt::CaseInsensitive))
        return false;

    const QStringList p = s.split(QLatin1Char(','));
    if (p.size() < 10)
        return false;
    bool okQ = false;
    const int q = p.at(6).trimmed().toInt(&okQ);
    if (!okQ || q == 0)
        return false;

    const double la = nmeaLatitudeToDeg(p.at(2), p.at(3));
    const double lo = nmeaLongitudeToDeg(p.at(4), p.at(5));
    if (la < -90.0 || la > 90.0 || lo < -180.0 || lo > 180.0)
        return false;
    *lat = la;
    *lon = lo;
    return true;
}

static bool parseAnyGpsDataLine(const QString &s, double *lat, double *lon, double *heading)
{
    if (parseAtlasGpsFields(normalizedLineForAtlas(s), lat, lon, heading))
        return true;
    if (parseRmcLine(s, lat, lon, heading))
        return true;
    if (parseGgaLine(s, lat, lon, heading))
        return true;
    return false;
}

static void feedGpsByteStream(QByteArray &rxBuffer, qint64 *lastGpsFixMs, const QByteArray &chunk,
                              const std::function<void(const QString &)> &onLog,
                              const std::function<void(double, double, double)> &onPosition)
{
    rxBuffer.append(chunk);
    if (rxBuffer.startsWith("\xEF\xBB\xBF"))
        rxBuffer.remove(0, 3);

    while (true) {
        const int nl = rxBuffer.indexOf('\n');
        if (nl < 0)
            break;
        QByteArray line = rxBuffer.left(nl).trimmed();
        rxBuffer.remove(0, nl + 1);
        if (line.isEmpty())
            continue;

        const QString s = QString::fromUtf8(line);
        onLog(s);

        double lat = 0;
        double lon = 0;
        double heading = 0;
        if (!parseAnyGpsDataLine(s, &lat, &lon, &heading))
            continue;

        if (lastGpsFixMs)
            *lastGpsFixMs = QDateTime::currentMSecsSinceEpoch();
        onPosition(lat, lon, heading);
    }
}

double haversineMeters(double lat1, double lon1, double lat2, double lon2)
{
    constexpr double R = 6371000.0;
    const double p1 = qDegreesToRadians(lat1);
    const double p2 = qDegreesToRadians(lat2);
    const double dp = qDegreesToRadians(lat2 - lat1);
    const double dl = qDegreesToRadians(lon2 - lon1);
    const double a = std::sin(dp / 2) * std::sin(dp / 2)
        + std::cos(p1) * std::cos(p2) * std::sin(dl / 2) * std::sin(dl / 2);
    return 2 * R * std::asin(std::sqrt(std::min(1.0, a)));
}

} // namespace

// --- Esp32GpsTcpClient (Wi‑Fi TCP : trames ATLAS_GPS / NMEA) ---

Esp32GpsTcpClient::Esp32GpsTcpClient(QObject *parent)
    : QObject(parent)
{
}

Esp32GpsTcpClient::~Esp32GpsTcpClient()
{
    disconnectDevice();
}

void Esp32GpsTcpClient::disconnectDevice()
{
    if (m_socket) {
        m_socket->abort();
        m_socket->deleteLater();
        m_socket = nullptr;
    }
    m_rxBuffer.clear();
    m_lastGpsFixMs = 0;
}

bool Esp32GpsTcpClient::isConnected() const
{
    return m_socket && m_socket->state() == QAbstractSocket::ConnectedState;
}

bool Esp32GpsTcpClient::hasRecentPosition(qint64 maxAgeMs) const
{
    const qint64 now = QDateTime::currentMSecsSinceEpoch();
    return m_lastGpsFixMs > 0 && (now - m_lastGpsFixMs) <= maxAgeMs;
}

void Esp32GpsTcpClient::connectToTcpHost(const QString &host, quint16 port)
{
    disconnectDevice();

    const QString h = host.trimmed();
    if (h.isEmpty()) {
        emit statusMessage(tr("Indiquez l’adresse IP de l’ESP32 (ex. 192.168.4.1)."));
        return;
    }

    m_socket = new QTcpSocket(this);
    connect(m_socket, &QTcpSocket::readyRead, this, &Esp32GpsTcpClient::onTcpReadyRead);
    connect(m_socket, &QTcpSocket::connected, this, &Esp32GpsTcpClient::onTcpConnected);
#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
    connect(m_socket, &QAbstractSocket::errorOccurred, this, [this](QAbstractSocket::SocketError) { onTcpSocketError(); });
#else
    connect(m_socket,
            static_cast<void (QAbstractSocket::*)(QAbstractSocket::SocketError)>(&QAbstractSocket::error),
            this, [this](QAbstractSocket::SocketError) { onTcpSocketError(); });
#endif

    m_rxBuffer.clear();
    emit statusMessage(tr("Connexion Wi‑Fi TCP vers %1:%2…").arg(h).arg(port));
    m_socket->connectToHost(h, port);
}

void Esp32GpsTcpClient::onTcpConnected()
{
    {
        QSettings settings;
        settings.beginGroup(QStringLiteral("Esp32Gps"));
        settings.setValue(QStringLiteral("wifiHost"), m_socket->peerName().isEmpty() ? m_socket->peerAddress().toString()
                                                                                       : m_socket->peerName());
        settings.setValue(QStringLiteral("wifiPort"), int(m_socket->peerPort()));
        settings.endGroup();
    }
    emit statusMessage(
        tr("Wi‑Fi TCP connecté — %1:%2. Réception : ATLAS_GPS ou NMEA $GPRMC/$GNRMC/$GPGGA.")
            .arg(m_socket->peerAddress().toString())
            .arg(m_socket->peerPort()));
    emit tcpLinkOpened();
}

void Esp32GpsTcpClient::onTcpReadyRead()
{
    if (!m_socket)
        return;
    feedGpsByteStream(m_rxBuffer, &m_lastGpsFixMs, m_socket->readAll(),
                      [this](const QString &s) { emit logLine(s); },
                      [this](double lat, double lon, double h) { emit positionUpdated(lat, lon, h); });
}

void Esp32GpsTcpClient::onTcpSocketError()
{
    if (!m_socket)
        return;
    emit statusMessage(tr("Erreur TCP : %1").arg(m_socket->errorString()));
}

// --- GpsWorldMapWidget ---

GpsWorldMapWidget::GpsWorldMapWidget(QWidget *parent)
    : QWidget(parent)
{
    setMouseTracking(true);
    setMinimumHeight(360);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setAttribute(Qt::WA_OpaquePaintEvent, false);

    QSettings s;
    s.beginGroup(QStringLiteral("Esp32Gps"));
    m_googleApiKey = s.value(QStringLiteral("googleMapsApiKey")).toString().trimmed();
    s.endGroup();
    if (m_googleApiKey.isEmpty()) {
        const QByteArray env = qgetenv("GOOGLE_MAPS_API_KEY");
        if (!env.isEmpty())
            m_googleApiKey = QString::fromUtf8(env).trimmed();
    }
    m_googleMode = !m_googleApiKey.isEmpty();
    if (m_googleMode)
        m_zoom = 14;

    m_nam = new QNetworkAccessManager(this);
    connect(m_nam, &QNetworkAccessManager::finished, this, &GpsWorldMapWidget::onNetworkFinished);

    m_refreshTimer = new QTimer(this);
    m_refreshTimer->setSingleShot(true);
    connect(m_refreshTimer, &QTimer::timeout, this, &GpsWorldMapWidget::requestGoogleStaticMap);

    const WorldPx c = geoToWorldPx(m_centerLat, m_centerLon, m_zoom);
    m_centerWorldX = c.x;
    m_centerWorldY = c.y;

    if (m_googleMode)
        scheduleGoogleRefresh();
}

GpsWorldMapWidget::WorldPx GpsWorldMapWidget::geoToWorldPx(double lat, double lon, int zoom)
{
    const double scale = kTileSize * std::pow(2.0, zoom);
    const double sinLat = std::sin(lat * M_PI / 180.0);
    const double x = (lon + 180.0) / 360.0 * scale;
    const double y = (0.5 - std::log((1.0 + sinLat) / (1.0 - sinLat)) / (4.0 * M_PI)) * scale;
    return {x, y};
}

void GpsWorldMapWidget::worldPxToGeo(double wx, double wy, int zoom, double &lat, double &lon)
{
    const double scale = kTileSize * std::pow(2.0, zoom);
    lon = wx / scale * 360.0 - 180.0;
    const double n = M_PI - (2.0 * M_PI * wy) / scale;
    lat = 180.0 / M_PI * std::atan(std::sinh(n));
}

QPointF GpsWorldMapWidget::worldToWidget(double wx, double wy) const
{
    return QPointF(width() / 2.0 + (wx - m_centerWorldX), height() / 2.0 + (wy - m_centerWorldY));
}

GpsWorldMapWidget::WorldPx GpsWorldMapWidget::widgetToWorld(const QPointF &widgetPt) const
{
    return {m_centerWorldX + (widgetPt.x() - width() / 2.0),
            m_centerWorldY + (widgetPt.y() - height() / 2.0)};
}

void GpsWorldMapWidget::scheduleGoogleRefresh()
{
    if (!m_googleMode || !m_refreshTimer)
        return;
    m_refreshTimer->start(200);
}

void GpsWorldMapWidget::requestGoogleStaticMap()
{
    if (!m_googleMode || m_googleApiKey.isEmpty() || width() < 32 || height() < 32)
        return;

    qreal dpr = 1.0;
#if QT_VERSION >= QT_VERSION_CHECK(5, 6, 0)
    dpr = devicePixelRatioF();
#endif
    int w = qBound(120, int(width() * dpr), 640);
    int h = qBound(120, int(height() * dpr), 640);

    double cLat = m_centerLat;
    double cLon = m_centerLon;
    if (m_followGps && m_hasGps) {
        cLat = m_boatLat;
        cLon = m_boatLon;
    }

    QUrl url(QStringLiteral("https://maps.googleapis.com/maps/api/staticmap"));
    QUrlQuery q;
    q.addQueryItem(QStringLiteral("center"),
                   QString::number(cLat, 'f', 6) + QLatin1Char(',') + QString::number(cLon, 'f', 6));
    q.addQueryItem(QStringLiteral("zoom"), QString::number(qBound(1, m_zoom, 20)));
    q.addQueryItem(QStringLiteral("size"), QString::number(w) + QLatin1Char('x') + QString::number(h));
    q.addQueryItem(QStringLiteral("scale"), QStringLiteral("2"));
    q.addQueryItem(QStringLiteral("maptype"), QStringLiteral("roadmap"));
    if (m_hasGps) {
        q.addQueryItem(QStringLiteral("markers"),
                       QStringLiteral("color:red|size:mid|%1,%2")
                           .arg(m_boatLat, 0, 'f', 6)
                           .arg(m_boatLon, 0, 'f', 6));
    }

    const int nPts = qMin(45, m_trail.size());
    if (nPts >= 2) {
        QString pathSpec = QStringLiteral("color:0x1E90FF|weight:4");
        for (int i = m_trail.size() - nPts; i < m_trail.size(); ++i) {
            const QPointF &ll = m_trail.at(i);
            pathSpec += QLatin1Char('|') + QString::number(ll.y(), 'f', 5) + QLatin1Char(',') + QString::number(ll.x(), 'f', 5);
        }
        q.addQueryItem(QStringLiteral("path"), pathSpec);
    }
    q.addQueryItem(QStringLiteral("key"), m_googleApiKey);
    url.setQuery(q);

    if (m_staticReply) {
        disconnect(m_staticReply, nullptr, this, nullptr);
        m_staticReply->abort();
        m_staticReply->deleteLater();
        m_staticReply = nullptr;
    }

    const QUrl u = url;
    QNetworkRequest req(u);
    req.setRawHeader("User-Agent", "SmartFishingPortATLAS/1.0 (Qt desktop; university project)");
    m_staticReply = m_nam->get(req);
    m_staticReply->setProperty("atlasStatic", true);
}

void GpsWorldMapWidget::onNetworkFinished(QNetworkReply *reply)
{
    if (!reply)
        return;

    if (reply->property("atlasStatic").toBool()) {
        if (reply == m_staticReply)
            m_staticReply = nullptr;

        if (reply->error() == QNetworkReply::NoError) {
            QPixmap pm;
            if (pm.loadFromData(reply->readAll()))
                m_googlePixmap = pm;
        }
        reply->deleteLater();
        update();
        return;
    }

    const QString key = m_replyToKey.take(reply);
    m_pendingKeys.remove(key);

    const QByteArray body = reply->readAll();
    const QNetworkReply::NetworkError err = reply->error();
    reply->deleteLater();

    if (key.isEmpty()) {
        update();
        return;
    }

    const qint64 now = QDateTime::currentMSecsSinceEpoch();
    if (err != QNetworkReply::NoError || body.isEmpty()) {
        m_tileCooldownUntilMs.insert(key, now + 4000);
        update();
        return;
    }

    QPixmap pm;
    /* Ne pas forcer "PNG" : laisser Qt détecter ; repli QImage pour JPEG / formats alternatifs. */
    if (!pm.loadFromData(body)) {
        const QImage img = QImage::fromData(body);
        if (!img.isNull())
            pm = QPixmap::fromImage(img);
    }
    if (!pm.isNull()) {
        m_tileCache.insert(key, pm);
        m_tileCooldownUntilMs.remove(key);
    } else {
        m_tileCooldownUntilMs.insert(key, now + 6000);
    }
    update();
}

void GpsWorldMapWidget::setFollowGps(bool on)
{
    m_followGps = on;
    if (m_googleMode)
        scheduleGoogleRefresh();
}

void GpsWorldMapWidget::centerOnBoat()
{
    if (!m_hasGps)
        return;
    m_centerLat = m_boatLat;
    m_centerLon = m_boatLon;
    const WorldPx w = geoToWorldPx(m_boatLat, m_boatLon, m_zoom);
    m_centerWorldX = w.x;
    m_centerWorldY = w.y;
    if (m_googleMode)
        scheduleGoogleRefresh();
    else
        update();
}

void GpsWorldMapWidget::clearTrail()
{
    m_trail.clear();
    if (m_googleMode)
        scheduleGoogleRefresh();
    else
        update();
}

void GpsWorldMapWidget::clearLiveGps()
{
    m_hasGps = false;
    m_trail.clear();
    if (m_googleMode)
        scheduleGoogleRefresh();
    else
        update();
}

void GpsWorldMapWidget::loadPersistedGpsState(const QVector<QPointF> &trailLonLat, bool hasBoatPosition,
                                              double boatLat, double boatLon, double headingDeg)
{
    m_trail = trailLonLat;
    while (m_trail.size() > kMaxTrailPoints)
        m_trail.remove(0, m_trail.size() - kMaxTrailPoints);

    m_hasGps = hasBoatPosition;
    if (m_hasGps) {
        m_boatLat = boatLat;
        m_boatLon = boatLon;
        m_boatHeadingDeg = headingDeg;
    }

    if (m_followGps && m_hasGps) {
        const WorldPx w = geoToWorldPx(m_boatLat, m_boatLon, m_zoom);
        m_centerWorldX = w.x;
        m_centerWorldY = w.y;
        worldPxToGeo(m_centerWorldX, m_centerWorldY, m_zoom, m_centerLat, m_centerLon);
    }

    if (m_googleMode)
        scheduleGoogleRefresh();
    else
        update();
}

void GpsWorldMapWidget::setGpsPosition(double lat, double lon, double headingDeg)
{
    if (m_hasGps) {
        const double d = haversineMeters(m_boatLat, m_boatLon, lat, lon);
        if (d >= kTrailMinMeters)
            m_trail.append(QPointF(lon, lat));
    } else {
        m_hasGps = true;
        m_trail.append(QPointF(lon, lat));
    }

    while (m_trail.size() > kMaxTrailPoints)
        m_trail.remove(0, 1);

    m_boatLat = lat;
    m_boatLon = lon;
    m_boatHeadingDeg = headingDeg;

    if (m_followGps) {
        const WorldPx w = geoToWorldPx(m_boatLat, m_boatLon, m_zoom);
        m_centerWorldX = w.x;
        m_centerWorldY = w.y;
        worldPxToGeo(m_centerWorldX, m_centerWorldY, m_zoom, m_centerLat, m_centerLon);
    }

    if (m_googleMode)
        scheduleGoogleRefresh();
    else
        update();
}

void GpsWorldMapWidget::requestTile(int z, int tx, int ty)
{
    const int n = 1 << z;
    if (tx < 0 || ty < 0 || tx >= n || ty >= n)
        return;

    const QString key = tileKey(z, tx, ty);
    if (m_tileCache.contains(key) || m_pendingKeys.contains(key))
        return;

    const qint64 now = QDateTime::currentMSecsSinceEpoch();
    if (m_tileCooldownUntilMs.value(key, 0) > now)
        return;

    /* Limite raisonnable des requêtes en vol (le NAM en met déjà plusieurs en file). */
    constexpr int kMaxConcurrentTileFetches = 40;
    if (m_pendingKeys.size() >= kMaxConcurrentTileFetches)
        return;

    m_pendingKeys.insert(key);
    static const char kSub[] = "abcd";
    const QChar sub = QLatin1Char(kSub[(tx + ty + z) & 3]);
    const QString urlStr = QStringLiteral("https://%1.basemaps.cartocdn.com/rastertiles/voyager/%2/%3/%4.png")
                               .arg(sub)
                               .arg(z)
                               .arg(tx)
                               .arg(ty);

    const QUrl tileUrl(urlStr);
    QNetworkRequest req(tileUrl);
    req.setRawHeader("User-Agent", "SmartFishingPortATLAS/1.0 (Qt desktop; university project)");
    req.setRawHeader("Accept", "image/png,*/*;q=0.5");
    QNetworkReply *rep = m_nam->get(req);
    rep->setProperty("atlasStatic", false);
    m_replyToKey.insert(rep, key);
}

QString GpsWorldMapWidget::tileKey(int z, int tx, int ty) const
{
    return QString::number(z) + QLatin1Char('/') + QString::number(tx) + QLatin1Char('/') + QString::number(ty);
}

void GpsWorldMapWidget::pruneTileCache(const QSet<QString> &keepNearby)
{
    constexpr int kMaxTiles = 480;
    if (m_tileCache.size() <= kMaxTiles)
        return;
    const QList<QString> keys = m_tileCache.keys();
    for (const QString &k : keys) {
        if (m_tileCache.size() <= kMaxTiles)
            break;
        if (!keepNearby.contains(k))
            m_tileCache.remove(k);
    }
}

void GpsWorldMapWidget::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    if (m_googleMode)
        scheduleGoogleRefresh();
}

void GpsWorldMapWidget::wheelEvent(QWheelEvent *event)
{
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
    const QPointF pos = event->position();
#else
    const QPointF pos = event->posF();
#endif
    const int steps = event->angleDelta().y() > 0 ? 1 : -1;

    if (m_googleMode) {
        const int newZ = qBound(1, m_zoom + steps, 20);
        if (newZ != m_zoom) {
            m_zoom = newZ;
            scheduleGoogleRefresh();
        }
        event->accept();
        return;
    }

    const int oldZ = m_zoom;
    const int newZ = qBound(2, oldZ + steps, 18);
    if (newZ == oldZ) {
        event->accept();
        return;
    }

    const WorldPx wUnder = widgetToWorld(pos);
    double lat0 = 0;
    double lon0 = 0;
    worldPxToGeo(wUnder.x, wUnder.y, oldZ, lat0, lon0);

    m_zoom = newZ;
    const WorldPx wNew = geoToWorldPx(lat0, lon0, newZ);
    m_centerWorldX = wNew.x - (pos.x() - width() / 2.0);
    m_centerWorldY = wNew.y - (pos.y() - height() / 2.0);

    worldPxToGeo(m_centerWorldX, m_centerWorldY, m_zoom, m_centerLat, m_centerLon);

    update();
    event->accept();
}

void GpsWorldMapWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        m_dragging = true;
        m_lastPanPos = event->pos();
    }
    event->accept();
}

void GpsWorldMapWidget::mouseMoveEvent(QMouseEvent *event)
{
    if (m_dragging) {
        const QPoint d = event->pos() - m_lastPanPos;
        m_lastPanPos = event->pos();
        m_centerWorldX -= d.x();
        m_centerWorldY -= d.y();
        worldPxToGeo(m_centerWorldX, m_centerWorldY, m_zoom, m_centerLat, m_centerLon);
        if (m_googleMode)
            scheduleGoogleRefresh();
        else
            update();
    }
    QWidget::mouseMoveEvent(event);
}

void GpsWorldMapWidget::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
        m_dragging = false;
    QWidget::mouseReleaseEvent(event);
}

void GpsWorldMapWidget::paintTrailAndBoat(QPainter &p, int zoomLevel)
{
    p.setRenderHint(QPainter::Antialiasing, true);

    const int z = zoomLevel;

    if (m_trail.size() >= 2) {
        QPainterPath path;
        bool first = true;
        for (const QPointF &ll : m_trail) {
            const WorldPx w = geoToWorldPx(ll.y(), ll.x(), z);
            const QPointF pt = worldToWidget(w.x, w.y);
            if (first) {
                path.moveTo(pt);
                first = false;
            } else {
                path.lineTo(pt);
            }
        }
        p.setPen(QPen(QColor(24, 119, 242), 4.0, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        p.setBrush(Qt::NoBrush);
        p.drawPath(path);
    } else if (m_trail.size() == 1) {
        const QPointF &ll = m_trail.constFirst();
        const WorldPx w = geoToWorldPx(ll.y(), ll.x(), z);
        const QPointF c = worldToWidget(w.x, w.y);
        p.setPen(Qt::NoPen);
        p.setBrush(QColor(24, 119, 242, 200));
        p.drawEllipse(c, 4.0, 4.0);
    }

    if (m_hasGps) {
        const WorldPx bw = geoToWorldPx(m_boatLat, m_boatLon, z);
        const QPointF c = worldToWidget(bw.x, bw.y);

        p.save();
        p.translate(c);
        const double rad = qDegreesToRadians(m_boatHeadingDeg);
        const double ang = qRadiansToDegrees(qAtan2(-std::cos(rad), std::sin(rad)));
        p.rotate(ang);

        QPolygonF tri;
        tri << QPointF(22, 0) << QPointF(-14, -11) << QPointF(-14, 11);
        p.setBrush(QColor(220, 38, 38));
        p.setPen(QPen(Qt::white, 2.2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        p.drawPolygon(tri);
        p.restore();
    }
}

void GpsWorldMapWidget::paintEvent(QPaintEvent *)
{
    QPainter p(this);

    if (m_googleMode) {
        p.fillRect(rect(), QColor(30, 45, 70));
        if (!m_googlePixmap.isNull()) {
            p.drawPixmap(rect(), m_googlePixmap, m_googlePixmap.rect());
        } else {
            p.setPen(Qt::white);
            p.drawText(rect(), Qt::AlignCenter,
                       tr("Chargement de la carte Google…\n"
                          "(clé : QSettings Esp32Gps/googleMapsApiKey ou GOOGLE_MAPS_API_KEY — Maps Static API)"));
        }
        p.setPen(QPen(QColor(255, 255, 255, 210), 1));
        p.setFont(QFont(QStringLiteral("Segoe UI"), 9));
        p.drawText(8, height() - 8, QStringLiteral("Map data © Google"));
        paintTrailAndBoat(p, qBound(1, m_zoom, 20));
        return;
    }

    p.fillRect(rect(), QColor(25, 40, 60));

    const int z = m_zoom;

    const double topLeftWx = m_centerWorldX - width() / 2.0;
    const double topLeftWy = m_centerWorldY - height() / 2.0;

    const int tx0 = int(std::floor(topLeftWx / kTileSize));
    const int ty0 = int(std::floor(topLeftWy / kTileSize));
    const int tx1 = int(std::floor((topLeftWx + width()) / kTileSize));
    const int ty1 = int(std::floor((topLeftWy + height()) / kTileSize));
    const int n = 1 << z;

    /* Ne jamais évincer du cache les tuiles visibles (±1) : sinon « cadres » vides au moindre pan / suivi GPS. */
    QSet<QString> keepNearby;
    for (int ty = ty0 - 1; ty <= ty1 + 1; ++ty) {
        for (int tx = tx0 - 1; tx <= tx1 + 1; ++tx) {
            if (tx < 0 || ty < 0 || tx >= n || ty >= n)
                continue;
            keepNearby.insert(tileKey(z, tx, ty));
        }
    }

    for (int ty = ty0; ty <= ty1; ++ty) {
        for (int tx = tx0; tx <= tx1; ++tx) {
            if (tx < 0 || ty < 0 || tx >= n || ty >= n)
                continue;
            const QString key = tileKey(z, tx, ty);
            requestTile(z, tx, ty);

            const double wx0 = tx * kTileSize;
            const double wy0 = ty * kTileSize;
            const QPointF dstTopLeft = worldToWidget(wx0, wy0);
            const QRectF tileRect(dstTopLeft, QSizeF(kTileSize, kTileSize));

            if (m_tileCache.contains(key)) {
                const QPixmap &pm = m_tileCache.value(key);
                p.drawPixmap(QPoint(qRound(dstTopLeft.x()), qRound(dstTopLeft.y())), pm);
            } else {
                /* Fond discret pendant chargement (évite gros rectangles « cadre » trop visibles). */
                p.fillRect(tileRect, QColor(32, 52, 78));
            }
        }
    }

    p.setPen(QPen(QColor(255, 255, 255, 200), 1));
    p.setFont(QFont(QStringLiteral("Segoe UI"), 9));
    p.drawText(8, height() - 10,
               QStringLiteral("© OpenStreetMap © CARTO — zoom molette, déplacer clic gauche"));

    paintTrailAndBoat(p, z);

    pruneTileCache(keepNearby);
}
