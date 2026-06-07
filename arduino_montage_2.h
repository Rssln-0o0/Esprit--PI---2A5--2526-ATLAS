#ifndef ARDUINO_MONTAGE_2_H
#define ARDUINO_MONTAGE_2_H

#include <QObject>
#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>
#include <QByteArray>
#include <QString>
#include <limits>

class QWidget;

struct SmartPortRealtimeState
{
    bool securityMode = false;
    bool gas = false;
    bool humidity = false;
    bool flame = false;
    bool rain = false;
    bool motion = false;
    bool magnetic = false;

    bool fan = false;
    bool pump = false;
    bool buzzer = false;
    bool light = false;

    bool ledGas = false;
    bool ledHumidity = false;
    bool ledFlame = false;
    bool ledMotion = false;
    double waterCm = std::numeric_limits<double>::quiet_NaN();
    bool tideLow = false; // true => maree basse, false => maree haute
};
Q_DECLARE_METATYPE(SmartPortRealtimeState)

class ArduinoMontage2 : public QObject
{
    Q_OBJECT
public:
    explicit ArduinoMontage2(QObject *parent = nullptr);
    ~ArduinoMontage2() override;

    int connect_arduino(const QString &excludePortName = QString());
    int close_arduino();
    bool isConnected() const;
    QString portName() const;
    void set_exclude_serial_port(const QString &portName);
    bool prompt_configure_port(QWidget *parent);

    void setSecurityMode(bool enabled);
    void setLighting(bool on);
    void setFan(bool on);
    void setPump(bool on);
    void setBuzzer(bool on);
    void setFanAuto();
    void setPumpAuto();
    void setBuzzerAuto();
    void setAllAuto();
    void requestStatus();
    void ping();

signals:
    void connectedChanged(bool connected, const QString &portName);
    void logLine(const QString &line);
    void parseError(const QString &line);
    void stateUpdated(const SmartPortRealtimeState &state);

private slots:
    void onReadyRead();

private:
    bool probeSmartPortFirmware(QByteArray *probePayload = nullptr);
    bool tryOpenPort(const QString &name, bool verifyFirmware = false);
    bool isUsbSerialBridge(const QSerialPortInfo &info) const;
    void applySerialParams();
    void sendLine(const QString &line);
    void parseLine(const QString &line);
    void parseStateLine(const QString &line);
    static bool parseBoolToken(const QString &value, bool *ok);

    QSerialPort *m_serial = nullptr;
    QByteArray m_readBuffer;
    QString m_portName;
    QString m_excludeSerialPort;
    SmartPortRealtimeState m_lastState;
};

#endif // ARDUINO_MONTAGE_2_H

