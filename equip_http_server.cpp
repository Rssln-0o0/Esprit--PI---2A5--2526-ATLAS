#include "equip_http_server.h"
#include "connection.h"

#include <QTcpSocket>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDate>
#include <QDateTime>
#include <QNetworkInterface>
#include <QHostAddress>
#include <QTimer>
#include <QDebug>
#include <QUrl>
#include <QUrlQuery>

EquipHttpServer::EquipHttpServer(QObject *parent) : QObject(parent) { m_lanIp = detectLanIp(); }
EquipHttpServer::~EquipHttpServer() { stop(); }

bool EquipHttpServer::start(int port) {
    if (m_server.isListening()) return true;
    if (!m_server.listen(QHostAddress::Any, port)) return false;
    connect(&m_server, &QTcpServer::newConnection, this, &EquipHttpServer::onNewConnection);
    return true;
}

void EquipHttpServer::stop() { if (m_server.isListening()) m_server.close(); }

QString EquipHttpServer::equipUrl(int equipId) const {
    return QString("http://%1:%2/equip/%3").arg(m_lanIp).arg(m_server.serverPort()).arg(equipId);
}

void EquipHttpServer::onNewConnection() {
    while (m_server.hasPendingConnections()) {
        QTcpSocket *socket = m_server.nextPendingConnection();
        connect(socket, &QTcpSocket::readyRead, this, &EquipHttpServer::onReadyRead);
        connect(socket, &QTcpSocket::disconnected, socket, &QTcpSocket::deleteLater);
    }
}

void EquipHttpServer::onReadyRead() {
    QTcpSocket *socket = qobject_cast<QTcpSocket *>(sender());
    if (!socket) return;

    QByteArray data = socket->readAll();
    QString request = QString::fromUtf8(data);
    QString firstLine = request.split('\n').first().trimmed();
    QStringList parts = firstLine.split(' ');
    if (parts.size() < 2) return;

    QString path = QUrl::fromPercentEncoding(parts.at(1).toUtf8());
    QByteArray response;

    if (path.startsWith("/action/")) {
        QString cleanPath = path.split('?').first();
        QStringList p = cleanPath.split('/', Qt::SkipEmptyParts);
        if (p.size() >= 3) {
            QString action = p.at(1);
            int id = p.at(2).toInt();

            if (action == "Memo") {
                QUrl url(parts.at(1));
                QUrlQuery query(url.query());
                if (query.hasQueryItem("msg")) {
                    QString msg = query.queryItemValue("msg");
                    QSqlDatabase db = QSqlDatabase::database(Connection::connectionName());
                    if (db.isOpen()) {
                        QSqlQuery q(db);
                        // Tentative d'insertion (on suppose que la table existe ou sera créée par le DBA)
                        q.prepare("INSERT INTO EQUIPEMENT_MEMO (ID_EQUIPEMENT, MESSAGE, DATE_MEMO) VALUES (:id, :m, CURRENT_TIMESTAMP)");
                        q.bindValue(":id", id);
                        q.bindValue(":m", msg);
                        if (!q.exec()) {
                            // Si la table n'existe pas, on tente de la créer (Oracle syntax)
                            q.exec("CREATE TABLE EQUIPEMENT_MEMO (ID_EQUIPEMENT NUMBER, MESSAGE VARCHAR2(4000), DATE_MEMO TIMESTAMP)");
                            q.prepare("INSERT INTO EQUIPEMENT_MEMO (ID_EQUIPEMENT, MESSAGE, DATE_MEMO) VALUES (:id, :m, CURRENT_TIMESTAMP)");
                            q.bindValue(":id", id);
                            q.bindValue(":m", msg);
                            q.exec();
                        }
                    }
                }
            } else {
                QSqlDatabase db = QSqlDatabase::database(Connection::connectionName());
                if (db.isOpen()) {
                    QSqlQuery q(db);
                    // S'aligner sur ton tableau : État="Disponible" -> Disponibilité="Oui"
                    QString dispoVal = (action == "Disponible") ? "Oui" : "Indisponible";
                    q.prepare("UPDATE EQUIPEMENT SET ETAT = :e, DISPONIBILITE = :d WHERE ID_EQUIPEMENT = :id");
                    q.bindValue(":e", action); q.bindValue(":d", dispoVal); q.bindValue(":id", id);
                    q.exec();
                }
            }
            response = "HTTP/1.1 303 See Other\r\nLocation: /equip/" + QByteArray::number(id) + "\r\nConnection: close\r\n\r\n";
        }
    } else if (path.startsWith("/equip/")) {
        int id = path.mid(7).toInt();
        QByteArray body = buildFicheTechniqueHtml(id).toUtf8();
        response = "HTTP/1.1 200 OK\r\nContent-Type: text/html; charset=utf-8\r\nContent-Length: " + QByteArray::number(body.size()) + "\r\n\r\n" + body;
    } else {
        QByteArray body = "<html><body style='background:#05070a;color:white;text-align:center;padding:50px;'><h1>EQUIPLOG SERVER</h1></body></html>";
        response = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: " + QByteArray::number(body.size()) + "\r\n\r\n" + body;
    }

    socket->write(response);
    connect(socket, &QTcpSocket::bytesWritten, socket, [socket](qint64){ if(socket->bytesToWrite()==0) socket->disconnectFromHost(); });
}

QString EquipHttpServer::buildFicheTechniqueHtml(int id) {
    QSqlDatabase db = QSqlDatabase::database(Connection::connectionName());
    QSqlQuery q(db);
    q.prepare("SELECT NOM, TYPE, ETAT, LOCALISATION FROM EQUIPEMENT WHERE ID_EQUIPEMENT = :id");
    q.bindValue(":id", id);
    if (!q.exec() || !q.next()) return "<html><body>Erreur SQL</body></html>";

    QString nom = q.value(0).toString().trimmed();
    QString type = q.value(1).toString().trimmed();
    QString etat = q.value(2).toString().trimmed();
    QString loc = q.value(3).toString().trimmed();
    
    QString memoHtml = "";
    q.prepare("SELECT MESSAGE, TO_CHAR(DATE_MEMO, 'DD/MM HH24:MI') FROM EQUIPEMENT_MEMO WHERE ID_EQUIPEMENT = :id ORDER BY DATE_MEMO DESC");
    q.bindValue(":id", id);
    if (q.exec()) {
        int count = 0;
        while (q.next() && count < 5) {
            QString m = q.value(0).toString();
            QString d = q.value(1).toString();
            memoHtml += QString(R"HTML(<div class="memo-card %1"><div style="font-size:10px;color:var(--muted);margin-bottom:5px;font-weight:800;">%2 • AGENT</div><div style="font-size:14px;line-height:1.4;">%3</div></div>)HTML")
                        .arg(count == 0 ? "mine" : "")
                        .arg(d)
                        .arg(esc(m));
            count++;
        }
    }
    
    if (memoHtml.isEmpty()) {
        memoHtml = "<div style='color:var(--muted);font-size:12px;text-align:center;padding:20px;'>Aucun message pour le moment.</div>";
    }

    return QStringLiteral(R"HTML(<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0, maximum-scale=1.0, user-scalable=no">
    <title>EquipLog</title>
    <style>
        @import url('https://fonts.googleapis.com/css2?family=Outfit:wght@300;400;600;800&display=swap');
        :root { --bg: #05070a; --card: rgba(22, 27, 34, 0.8); --accent: #00d2ff; --accent2: #3a7bd5; --text: #e6edf3; --muted: #8b949e; --success: #238636; --danger: #f85149; }
        body { background: var(--bg); color: var(--text); font-family: 'Outfit', sans-serif; margin: 0; padding: 0; }
        .header { position: sticky; top: 0; background: rgba(13, 17, 23, 0.8); backdrop-filter: blur(15px); padding: 15px 20px; border-bottom: 1px solid rgba(255,255,255,0.1); display: flex; justify-content: space-between; align-items: center; z-index: 100; }
        .hero { padding: 30px 20px; text-align: center; background: linear-gradient(180deg, rgba(0, 210, 255, 0.05) 0%, transparent 100%); }
        .status-badge { display: inline-block; padding: 4px 12px; border-radius: 20px; font-size: 10px; font-weight: 800; border: 1px solid var(--success); color: var(--success); margin-bottom: 10px; text-transform: uppercase; }
        .container { padding: 0 20px 100px; }
        .memo-card { background: var(--card); border-radius: 16px; padding: 16px; border-left: 4px solid var(--accent2); margin-bottom: 12px; }
        .memo-card.mine { border-left-color: var(--accent); background: rgba(0, 210, 255, 0.05); }
        .action-item { background: rgba(58, 123, 213, 0.1); padding: 16px; border-radius: 12px; display: flex; justify-content: space-between; text-decoration: none; color: white; border: 1px solid rgba(58, 123, 213, 0.2); margin-bottom: 10px; font-weight: 600; }
        .floating-btn { position: fixed; bottom: 25px; right: 25px; width: 60px; height: 60px; background: var(--accent); border-radius: 50%; display: flex; align-items: center; justify-content: center; font-size: 30px; color: #000; box-shadow: 0 10px 25px rgba(0,210,255,0.4); font-weight: bold; cursor: pointer; z-index: 1000; }
    </style>
    <script>
        let etatActuel = ")HTML") + etat + QStringLiteral(R"HTML(";
        function laisseConsigne() {
            let msg = prompt("Quelle consigne voulez-vous laisser ?");
            if (msg && msg.trim() !== "") window.location.href = "/action/Memo/)HTML") + QString::number(id) + QStringLiteral(R"HTML(?msg=" + encodeURIComponent(msg);
        }
        function verifierDemarrage() {
            if (etatActuel === "Indisponible" || etatActuel === "En Panne" || etatActuel === "En Service") {
                alert("⛔ ACTION BLOQUÉE : Cet équipement est déjà " + etatActuel);
            } else {
                window.location.href = "/action/En Service/)HTML") + QString::number(id) + QStringLiteral(R"HTML(";
            }
        }
    </script>
</head>
<body>
    <div class="header"><div style="font-weight:800;color:var(--accent);">EQUIPLOG PRO</div><div style="font-size:11px;color:var(--muted);">QUAI SUD</div></div>
    <div class="hero">
        <div class="status-badge" style=")HTML") + (etat == "Disponible" ? "" : "border-color:var(--danger);color:var(--danger);") + QStringLiteral(R"HTML("> )HTML") + etat + QStringLiteral(R"HTML( </div>
        <h1 style="font-size:28px;margin:0;">)HTML") + nom + QStringLiteral(R"HTML(</h1>
        <p style="color:var(--muted);font-size:13px;margin-top:5px;">)HTML") + type + QStringLiteral(R"HTML( • Réf #)HTML") + QString::number(id) + QStringLiteral(R"HTML(</p>
    </div>
    <div class="container">
        <h3 style="font-size:13px;color:var(--accent);text-transform:uppercase;margin-bottom:15px;letter-spacing:1px;">Journal Opérationnel</h3>
        )HTML") + memoHtml + QStringLiteral(R"HTML(
        <h3 style="font-size:13px;color:var(--accent);text-transform:uppercase;margin:30px 0 15px;letter-spacing:1px;">Actions de terrain</h3>
        <a href="javascript:void(0)" onclick="verifierDemarrage()" class="action-item"><span>🚀 Démarrer Utilisation</span><span>→</span></a>
        <a href="/action/Disponible/)HTML") + QString::number(id) + QStringLiteral(R"HTML(" class="action-item" style="border-color:rgba(35,134,54,0.3);"><span>📍 Terminer & Ranger</span><span>→</span></a>
    </div>
    <div class="floating-btn" onclick="laisseConsigne()">+</div>
</body>
</html>)HTML");
}

QString EquipHttpServer::detectLanIp() {
    QList<QString> ips;
    for (const QNetworkInterface &i : QNetworkInterface::allInterfaces()) {
        if (!i.flags().testFlag(QNetworkInterface::IsUp) || i.flags().testFlag(QNetworkInterface::IsLoopBack)) continue;
        for (const QNetworkAddressEntry &e : i.addressEntries()) if (e.ip().protocol() == QAbstractSocket::IPv4Protocol) ips << e.ip().toString();
    }
    for (const QString &ip : ips) if (ip.startsWith("192.168.0.") || ip.startsWith("192.168.1.")) return ip;
    return !ips.isEmpty() ? ips.first() : "127.0.0.1";
}

QString EquipHttpServer::esc(const QString &s) { return s.toHtmlEscaped(); }
