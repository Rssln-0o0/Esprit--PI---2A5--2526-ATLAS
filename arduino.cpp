#include "arduino.h"

#include <QInputDialog>
#include <QMessageBox>
#include <QSettings>
#include <QWidget>

namespace {

struct UsbId {
    quint16 vendor;
    quint16 product;
};

// Ponts USB–série fréquents sur modules ESP32-CAM
const UsbId kEsp32BridgeIds[] = {
    {0x1A86, 0x7523}, // CH340
    {0x1A86, 0x5523}, // CH341
    {0x10C4, 0xEA60}, // CP210x
    {0x10C4, 0xEA61},
    {0x0403, 0x6001}, // FTDI
};

} // namespace

static bool serialPortExcluded(const QString &portName, const QString &excludePortName)
{
    if (excludePortName.isEmpty())
        return false;
    return QString::compare(portName, excludePortName, Qt::CaseInsensitive) == 0;
}

void Arduino::set_exclude_serial_port(const QString &portName)
{
    m_excludeSerialPort = portName.trimmed();
}

Arduino::Arduino()
{
    serial = new QSerialPort;
}

Arduino::~Arduino()
{
    if (serial && serial->isOpen())
        serial->close();
    delete serial;
    serial = nullptr;
}

bool Arduino::port_matches_esp32_bridge(const QSerialPortInfo &info)
{
    const quint16 vid = info.vendorIdentifier();
    const quint16 pid = info.productIdentifier();
    for (const UsbId &u : kEsp32BridgeIds) {
        if (vid == u.vendor && pid == u.product)
            return true;
    }
    const QString desc = info.description().toLower();
    const QString mfg = info.manufacturer().toLower();
    if (desc.contains(QStringLiteral("ch340")) || desc.contains(QStringLiteral("ch341")))
        return true;
    if (desc.contains(QStringLiteral("cp210")) || mfg.contains(QStringLiteral("silicon labs")))
        return true;
    if (desc.contains(QStringLiteral("usb serial")) || desc.contains(QStringLiteral("usb-uart")))
        return true;
    return false;
}

void Arduino::apply_serial_params()
{
    if (!serial)
        return;
    QSettings settings(QStringLiteral("ATLAS"), QStringLiteral("ATLAS"));
    const int baudRate = settings.value(QStringLiteral("esp32cam/baudRate"), 921600).toInt();
    serial->setBaudRate(baudRate);
    serial->setDataBits(QSerialPort::Data8);
    serial->setParity(QSerialPort::NoParity);
    serial->setStopBits(QSerialPort::OneStop);
    serial->setFlowControl(QSerialPort::NoFlowControl);
    serial->setReadBufferSize(512 * 1024);
}

int Arduino::connect_arduino()
{
    arduino_is_available = false;
    arduino_port_name.clear();

    if (!serial)
        return -1;

    if (serial->isOpen())
        return 0;

    QSettings settings(QStringLiteral("ATLAS"), QStringLiteral("ATLAS"));
    const QString saved = settings.value(QStringLiteral("esp32cam/portName")).toString().trimmed();
    
    if (!saved.isEmpty() && !serialPortExcluded(saved, m_excludeSerialPort)) {
        arduino_is_available = true;
        arduino_port_name = saved;
    }

    if (!arduino_is_available)
        return -1;

    serial->setPortName(arduino_port_name);
    apply_serial_params();

    if (!serial->open(QIODevice::ReadWrite))
        return 1;

    flush_input();
    return 0;
}

int Arduino::close_arduino()
{
    if (!serial)
        return 1;
    if (serial->isOpen()) {
        serial->close();
        return 0;
    }
    return 1;
}

void Arduino::write_to_arduino(QByteArray d)
{
    if (serial && serial->isOpen())
        serial->write(d);
}

QByteArray Arduino::read_from_arduino()
{
    data = (serial && serial->isOpen()) ? serial->readAll() : QByteArray();
    return data;
}

QSerialPort *Arduino::getserial()
{
    return serial;
}

QString Arduino::getarduino_port_name()
{
    return arduino_port_name;
}

void Arduino::write_flash_led(quint8 value)
{
    if (!serial || !serial->isOpen())
        return;
    char cmd[2] = {'L', static_cast<char>(value & 0xFF)};
    serial->write(cmd, 2);
}

void Arduino::flush_input()
{
    if (!serial || !serial->isOpen())
        return;
    serial->clear(QSerialPort::Input);
    while (serial->waitForReadyRead(30))
        serial->readAll();
}

bool Arduino::ensure_open(QString *errorMessage)
{
    if (serial && serial->isOpen())
        return true;
    const int ret = connect_arduino();
    if (ret == 0)
        return true;
    if (errorMessage) {
        if (ret == -1)
            *errorMessage = QStringLiteral("ESP32-CAM non détectée. Branchez le câble USB.");
        else
            *errorMessage = QStringLiteral("Impossible d’ouvrir le port série.%1")
                                .arg(arduino_port_name.isEmpty() ? QString() : QStringLiteral(" (%1)").arg(arduino_port_name));
    }
    return false;
}

bool Arduino::prompt_configure_port(QWidget *parent)
{
    const QList<QSerialPortInfo> ports = QSerialPortInfo::availablePorts();
    if (ports.isEmpty()) {
        QMessageBox::warning(parent, QStringLiteral("ESP32-CAM"),
                             QStringLiteral("Aucun port série détecté. Branchez l’ESP32-CAM en USB."));
        return false;
    }
    QStringList names;
    for (const QSerialPortInfo &p : ports) {
        if (serialPortExcluded(p.portName(), m_excludeSerialPort))
            continue;
        names << p.portName() + QStringLiteral(" - ") + p.description();
    }
    if (names.isEmpty()) {
        QMessageBox::warning(parent, QStringLiteral("ESP32-CAM"),
                             QStringLiteral("Aucun port libre (le port RFID Arduino est réservé). "
 "Branchez l’ESP32-CAM sur un autre câble USB ou débranchez le RFID."));
        return false;
    }
    bool ok = false;
    const QString selected = QInputDialog::getItem(parent, QStringLiteral("ESP32-CAM"),
                                                   QStringLiteral("Sélectionnez le port de l’ESP32-CAM :"),
                                                   names, 0, false, &ok);
    if (!ok || selected.isEmpty())
        return false;
    QString portName = selected.left(selected.indexOf(QStringLiteral(" - "))).trimmed();
    if (portName.isEmpty())
        portName = selected.split(QStringLiteral(" - ")).value(0).trimmed();

    QSettings settings(QStringLiteral("ATLAS"), QStringLiteral("ATLAS"));
    const int prevBaud = settings.value(QStringLiteral("esp32cam/baudRate"), 921600).toInt();
    const QStringList baudList = {QStringLiteral("921600"), QStringLiteral("115200"),
                                  QStringLiteral("256000"), QStringLiteral("576000")};
    int baudIdx = baudList.indexOf(QString::number(prevBaud));
    if (baudIdx < 0)
        baudIdx = 0;
    const QString baudStr = QInputDialog::getItem(parent, QStringLiteral("ESP32-CAM"),
                                                    QStringLiteral("Vitesse (baud) :"), baudList, baudIdx, false, &ok);
    if (!ok || baudStr.isEmpty())
        return false;
    const int baudRate = baudStr.toInt();

    settings.setValue(QStringLiteral("esp32cam/portName"), portName);
    settings.setValue(QStringLiteral("esp32cam/baudRate"), baudRate);

    if (serial->isOpen())
        serial->close();
    arduino_is_available = true;
    arduino_port_name = portName;
    serial->setPortName(portName);
    apply_serial_params();
    if (!serial->open(QIODevice::ReadWrite)) {
        QMessageBox::warning(parent, QStringLiteral("ESP32-CAM"),
                             QStringLiteral("Impossible d’ouvrir le port %1.\n%2")
                                 .arg(portName, serial->errorString()));
        return false;
    }
    flush_input();
    return true;
}

bool Arduino::capture_esp32_cam_jpeg(QImage *out, QString *errorMessage)
{
    if (!out)
        return false;
    *out = QImage();
    if (!ensure_open(errorMessage))
        return false;

    flush_input();

    serial->write("C", 1);
    if (!serial->waitForBytesWritten(1000)) {
        if (errorMessage)
            *errorMessage = QStringLiteral("Écriture série interrompue.");
        return false;
    }
    serial->flush();

    if (!serial->waitForReadyRead(4000) && serial->bytesAvailable() < 4) {
        if (errorMessage)
            *errorMessage = QStringLiteral("Pas de réponse de l’ESP32-CAM.");
        return false;
    }
    while (serial->bytesAvailable() < 4 && serial->waitForReadyRead(500)) {}

    const QByteArray lenBa = serial->read(4);
    if (lenBa.size() != 4) {
        if (errorMessage)
            *errorMessage = QStringLiteral("En-tête JPEG invalide.");
        return false;
    }
    const quint32 jpegLen = (quint32)(unsigned char)lenBa[0] | ((quint32)(unsigned char)lenBa[1] << 8)
        | ((quint32)(unsigned char)lenBa[2] << 16) | ((quint32)(unsigned char)lenBa[3] << 24);
    if (jpegLen == 0 || jpegLen > 500000) {
        if (errorMessage)
            *errorMessage = QStringLiteral("Taille d’image invalide.");
        return false;
    }

    QByteArray jpegData;
    while ((quint32)jpegData.size() < jpegLen) {
        serial->waitForReadyRead(500);
        jpegData.append(serial->read(jpegLen - jpegData.size()));
        if (jpegData.size() == 0 && !serial->waitForReadyRead(500))
            break;
    }
    if ((quint32)jpegData.size() != jpegLen) {
        if (errorMessage)
            *errorMessage = QStringLiteral("Image tronquée.");
        return false;
    }
    if (!out->loadFromData(jpegData, "JPEG") || out->isNull()) {
        if (errorMessage)
            *errorMessage = QStringLiteral("Image JPEG invalide.");
        return false;
    }
    *out = out->mirrored(false, true);
    return true;
}
