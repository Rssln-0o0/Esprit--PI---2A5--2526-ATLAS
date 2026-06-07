#ifndef EQUIP_HTTP_SERVER_H
#define EQUIP_HTTP_SERVER_H

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDate>
#include <QDateTime>
#include <QHostAddress>
#include <QNetworkInterface>
#include <QTimer>
#include <QMap>

class EquipHttpServer : public QObject
{
    Q_OBJECT
public:
    explicit EquipHttpServer(QObject *parent = nullptr);
    ~EquipHttpServer();

    bool start(int port = 8765);
    void stop();
    
    QString equipUrl(int equipId) const;
    QString lanIp() const { return m_lanIp; }
    int port() const { return m_server.serverPort(); }
    bool isRunning() const { return m_server.isListening(); }

private slots:
    void onNewConnection();
    void onReadyRead();

private:
    QTcpServer m_server;
    QString m_lanIp;

    QByteArray buildEquipResponse(int id);
    QByteArray buildRootResponse();
    QByteArray build404Response(const QString &msg);
    QString buildFicheTechniqueHtml(int id);

    QString detectLanIp();
    QString esc(const QString &s);


};

#endif // EQUIP_HTTP_SERVER_H
