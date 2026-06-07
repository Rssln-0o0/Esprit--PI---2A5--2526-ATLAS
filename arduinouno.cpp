#include "arduinouno.h"

#include <QDebug>
#include <QInputDialog>
#include <QMessageBox>
#include <QRegularExpression>
#include <QSettings>
#include <QThread>
#include <QWidget>

namespace {

bool portExcluded(const QString &portName, const QString &excludePortName)
{
    if (excludePortName.isEmpty())
        return false;
    const QStringList parts = excludePortName.split(
        QRegularExpression(QStringLiteral("[|;,\r\n\t]+")), Qt::SkipEmptyParts);
    if (parts.isEmpty())
        return QString::compare(portName, excludePortName.trimmed(), Qt::CaseInsensitive) == 0;
    for (const QString &p : parts) {
        if (QString::compare(portName, p.trimmed(), Qt::CaseInsensitive) == 0)
            return true;
    }
    return false;
}

} // namespace

bool ArduinoUno::isUsbSerialBridge(const QSerialPortInfo &info)
{
    if (!info.hasVendorIdentifier() || !info.hasProductIdentifier())
        return false;
    const quint16 vid = info.vendorIdentifier();
    const quint16 pid = info.productIdentifier();
    if (vid == 0x2341 && pid == 0x0043)
        return true;
    if (vid == 0x1A86 && (pid == 0x7523 || pid == 0x5523))
        return true;
    if (vid == 0x10C4 && (pid == 0xEA60 || pid == 0xEA61))
        return true;
    if (vid == 0x0403 && pid == 0x6001)
        return true;
    return false;
}

ArduinoUno::ArduinoUno(const QString &settingsGroup, const QString &displayName, QObject *parent)
    : QObject(parent)
{
    serial = new QSerialPort(this);
    m_settingsGroup = settingsGroup.trimmed().isEmpty() ? QStringLiteral("arduinorfid") : settingsGroup.trimmed();
    m_displayName = displayName.trimmed().isEmpty() ? QStringLiteral("RFID Arduino") : displayName.trimmed();
}

ArduinoUno::~ArduinoUno()
{
    if (serial && serial->isOpen())
        serial->close();
}

void ArduinoUno::applySerialParams()
{
    if (!serial)
        return;
    serial->setBaudRate(QSerialPort::Baud9600);
    serial->setDataBits(QSerialPort::Data8);
    serial->setParity(QSerialPort::NoParity);
    serial->setStopBits(QSerialPort::OneStop);
    serial->setFlowControl(QSerialPort::NoFlowControl);
}

void ArduinoUno::flushInput()
{
    if (!serial || !serial->isOpen())
        return;
    serial->clear(QSerialPort::Input);
}

bool ArduinoUno::tryOpenPort(const QString &portName)
{
    if (!serial || portName.isEmpty())
        return false;
    m_lastTriedPortName = portName;

    if (serial->isOpen())
        serial->close();
    
    serial->setPortName(portName);
    applySerialParams();
    
    if (serial->open(QIODevice::ReadWrite)) {
        m_lastOpenError.clear();
        flushInput();
        return true;
    }
    
    m_lastOpenError = serial->errorString();
    return false;
}

int ArduinoUno::connect_arduino(const QString &excludePortName)
{
    arduino_is_available = false;
    arduino_port_name.clear();

    if (!serial)
        return -1;

    if (serial->isOpen())
        return 0;

    QSettings settings(QStringLiteral("ATLAS"), QStringLiteral("ATLAS"));
    const QString saved = settings.value(QStringLiteral("%1/portName").arg(m_settingsGroup)).toString().trimmed();

    // 1. D'abord, on essaie uniquement le port sauvegardé dans les paramètres (s'il existe).
    if (!saved.isEmpty() && !portExcluded(saved, excludePortName)) {
        if (tryOpenPort(saved)) {
            arduino_is_available = true;
            arduino_port_name = saved;
            return 0;
        }
    }

    // 2. S'il n'y a pas de port sauvegardé ou s'il échoue, on retourne une erreur.
    // L'interface graphique devra appeler prompt_configure_port() pour demander à l'utilisateur.
    return -1;
}

int ArduinoUno::close_arduino()
{
    if (!serial)
        return 1;
    if (serial->isOpen()) {
        serial->close();
        return 0;
    }
    return 1;
}

void ArduinoUno::write_to_arduino(QByteArray d)
{
    if (serial && serial->isOpen())
        serial->write(d);
}

QByteArray ArduinoUno::read_from_arduino()
{
    return (serial && serial->isOpen()) ? serial->readAll() : QByteArray();
}

QSerialPort *ArduinoUno::getserial()
{
    return serial;
}

QString ArduinoUno::getarduino_port_name()
{
    return arduino_port_name;
}

QString ArduinoUno::lastOpenError() const
{
    return m_lastOpenError;
}

QString ArduinoUno::lastTriedPortName() const
{
    return m_lastTriedPortName;
}

bool ArduinoUno::prompt_configure_port(QWidget *parent, const QString &excludePortName)
{
    const QList<QSerialPortInfo> ports = QSerialPortInfo::availablePorts();
    if (ports.isEmpty()) {
        QMessageBox::warning(parent, m_displayName,
                             QObject::tr("Aucun port série détecté. Branchez l’Arduino Uno en USB."));
        return false;
    }
    QStringList names;
    for (const QSerialPortInfo &p : ports) {
        if (portExcluded(p.portName(), excludePortName))
            continue;
        names << (p.portName() + QStringLiteral(" — ") + p.description());
    }
    if (names.isEmpty()) {
        QMessageBox::warning(parent, m_displayName,
                             QObject::tr("Aucun port libre. Débranchez l’ESP32-CAM ou changez de câble."));
        return false;
    }
    bool ok = false;
    const QString selected = QInputDialog::getItem(
        parent, m_displayName,
        QObject::tr("Port COM de l’Arduino (lecteur RFID) :"), names, 0, false, &ok);
    if (!ok || selected.isEmpty())
        return false;
    const int sep = selected.indexOf(QStringLiteral(" — "));
    QString portName = sep >= 0 ? selected.left(sep).trimmed() : selected.trimmed();

    if (serial && serial->isOpen())
        serial->close();
    if (!tryOpenPort(portName)) {
        QMessageBox::warning(parent, m_displayName,
                             QObject::tr("Impossible d’ouvrir %1.\n%2")
                                 .arg(portName)
                                 .arg(serial ? serial->errorString() : QString()));
        return false;
    }
    arduino_is_available = true;
    arduino_port_name = portName;
    QSettings(QStringLiteral("ATLAS"), QStringLiteral("ATLAS"))
        .setValue(QStringLiteral("%1/portName").arg(m_settingsGroup), arduino_port_name);
    return true;
}
