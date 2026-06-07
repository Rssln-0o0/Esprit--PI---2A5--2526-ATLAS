#ifndef ARDUINOUNO_H
#define ARDUINOUNO_H

#include <QObject>
#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>
#include <QByteArray>
#include <QString>

class QWidget;

/** Liaison série PC ↔ Arduino Uno (sketch RFID9600 baud, lignes UID:...). */
class ArduinoUno : public QObject
{
    Q_OBJECT
public:
    explicit ArduinoUno(const QString &settingsGroup = QStringLiteral("arduinorfid"),
                        const QString &displayName = QStringLiteral("RFID Arduino"),
                        QObject *parent = nullptr);
    ~ArduinoUno();

    int connect_arduino(const QString &excludePortName = QString());
    int close_arduino();
    void write_to_arduino(QByteArray d);
    QByteArray read_from_arduino();
    QSerialPort *getserial();
    QString getarduino_port_name();

    bool prompt_configure_port(QWidget *parent, const QString &excludePortName = QString());
    QString lastOpenError() const;
    QString lastTriedPortName() const;

private:
    static bool isUsbSerialBridge(const QSerialPortInfo &info);
    void applySerialParams();
    void flushInput();
    bool tryOpenPort(const QString &portName);

    QSerialPort *serial = nullptr;
    static const quint16 arduino_uno_vendor_id = 9025;
    static const quint16 arduino_uno_product_id = 67;
    QString arduino_port_name;
    bool arduino_is_available = false;
    QString m_settingsGroup;
    QString m_displayName;
    QString m_lastOpenError;
    QString m_lastTriedPortName;
};

#endif // ARDUINOUNO_H
