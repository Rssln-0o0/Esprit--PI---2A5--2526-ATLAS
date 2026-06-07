#include "arduino_montage_2.h"

#include <QDebug>
#include <QElapsedTimer>
#include <QInputDialog>
#include <QMessageBox>
#include <QSettings>

namespace {
constexpr qint32 kDefaultBaudRate = 115200;
constexpr int kMaxLineLen = 256;
const int kProbeBauds[] = {115200, 9600, 57600, 38400, 74880, 230400};

bool serialPortExcluded(const QString &portName, const QString &excludePortName)
{
    if (excludePortName.trimmed().isEmpty())
        return false;
    return QString::compare(portName.trimmed(), excludePortName.trimmed(), Qt::CaseInsensitive) == 0;
}
}

ArduinoMontage2::ArduinoMontage2(QObject *parent)
    : QObject(parent), m_serial(new QSerialPort(this))
{
    applySerialParams();
    connect(m_serial, &QSerialPort::readyRead, this, &ArduinoMontage2::onReadyRead);
}

ArduinoMontage2::~ArduinoMontage2()
{
    close_arduino();
}

void ArduinoMontage2::applySerialParams()
{
    if (!m_serial)
        return;
    QSettings settings(QStringLiteral("ATLAS"), QStringLiteral("ATLAS"));
    const int baudRate = settings.value(QStringLiteral("smartport/baudRate"), kDefaultBaudRate).toInt();
    m_serial->setBaudRate(baudRate);
    m_serial->setDataBits(QSerialPort::Data8);
    m_serial->setParity(QSerialPort::NoParity);
    m_serial->setStopBits(QSerialPort::OneStop);
    m_serial->setFlowControl(QSerialPort::NoFlowControl);
}

bool ArduinoMontage2::isUsbSerialBridge(const QSerialPortInfo &info) const
{
    const quint16 vid = info.vendorIdentifier();
    const quint16 pid = info.productIdentifier();
    if (vid == 0x1A86 && (pid == 0x7523 || pid == 0x5523))
        return true; // CH340 / CH341
    if (vid == 0x10C4 && (pid == 0xEA60 || pid == 0xEA61))
        return true; // CP210x
    if (vid == 0x0403 && pid == 0x6001)
        return true; // FTDI
    if (vid == 0x2341 || vid == 0x2A03)
        return true; // Arduino
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

bool ArduinoMontage2::tryOpenPort(const QString &name, bool verifyFirmware)
{
    if (!m_serial || name.trimmed().isEmpty())
        return false;
    if (m_serial->isOpen())
        m_serial->close();
    m_serial->setPortName(name.trimmed());
    applySerialParams();
    if (!m_serial->open(QIODevice::ReadWrite))
        return false;

    // Certains Arduino reset a l'ouverture du port serie.
    // On laisse le sketch redemarrer avant de le sonder.
    if (!m_serial->waitForReadyRead(1200)) {
        // Rien recu: normal possible, on continue vers le probe actif.
    } else {
        m_serial->readAll();
    }

    m_readBuffer.clear();
    QByteArray probePayload;
    if (verifyFirmware) {
        bool okFirmware = probeSmartPortFirmware(&probePayload);
        if (!okFirmware) {
            // Essai multi-baud pour cartes clone / sketches avec baud different.
            for (int b : kProbeBauds) {
                m_serial->setBaudRate(b);
                m_serial->clear(QSerialPort::AllDirections);
                probePayload.clear();
                if (probeSmartPortFirmware(&probePayload)) {
                    QSettings(QStringLiteral("ATLAS"), QStringLiteral("ATLAS"))
                        .setValue(QStringLiteral("smartport/baudRate"), b);
                    okFirmware = true;
                    break;
                }
            }
        }
        if (!okFirmware) {
            m_serial->close();
            return false;
        }
    }

    m_portName = m_serial->portName();
    emit connectedChanged(true, m_portName);
    emit logLine(QStringLiteral("SMART_PORT_COM=%1").arg(m_portName));

    if (!probePayload.isEmpty()) {
        const QList<QByteArray> probeLines = probePayload.split('\n');
        for (const QByteArray &rawLine : probeLines) {
            const QString line = QString::fromUtf8(rawLine).trimmed();
            if (!line.isEmpty())
                parseLine(line);
        }
    }
    return true;
}

bool ArduinoMontage2::probeSmartPortFirmware(QByteArray *probePayload)
{
    if (!m_serial || !m_serial->isOpen())
        return false;

    // Sondage progressif pour accepter les cartes qui demarrent lentement.
    // Ligne vide d'abord pour nettoyer une commande partielle.
    m_serial->write("\nPING\n");
    m_serial->flush();

    QByteArray payload;
    QElapsedTimer timer;
    timer.start();
    qint64 lastStatusAskMs = -500;
    while (timer.elapsed() < 3200) {
        const qint64 elapsed = timer.elapsed();
        if (elapsed - lastStatusAskMs >= 450) {
            m_serial->write("\nSTATUS?\n");
            m_serial->flush();
            lastStatusAskMs = elapsed;
        }
        if (m_serial->waitForReadyRead(120))
            payload.append(m_serial->readAll());
        if (payload.contains("SMART_PORT_READY")
            || payload.contains("STATE;")
            || payload.contains("ACK;MODE=")
            || payload.contains("PONG")) {
            if (probePayload)
                *probePayload = payload;
            return true;
        }
    }
    if (probePayload)
        *probePayload = payload;
    return false;
}

int ArduinoMontage2::connect_arduino(const QString &excludePortName)
{
    if (!m_serial)
        return -1;
    if (m_serial->isOpen())
        return 0;

    const QString excluded = excludePortName.trimmed().isEmpty() ? m_excludeSerialPort : excludePortName.trimmed();
    QSettings settings(QStringLiteral("ATLAS"), QStringLiteral("ATLAS"));

    // 1) Port sauvegardé en priorité (et uniquement ça pour empecher de voler les autres COM)
    const QString savedPort = settings.value(QStringLiteral("smartport/portName")).toString().trimmed();
    if (!savedPort.isEmpty() && !serialPortExcluded(savedPort, excluded) && tryOpenPort(savedPort, false))
        return 0;

    return -1;
}

void ArduinoMontage2::set_exclude_serial_port(const QString &portName)
{
    m_excludeSerialPort = portName.trimmed();
}

bool ArduinoMontage2::prompt_configure_port(QWidget *parent)
{
    const QList<QSerialPortInfo> ports = QSerialPortInfo::availablePorts();
    if (ports.isEmpty()) {
        QMessageBox::warning(parent, QStringLiteral("Smart Port Arduino"),
                             QStringLiteral("Aucun port série détecté. Branchez l'Arduino Smart Port en USB."));
        return false;
    }

    QStringList names;
    for (const QSerialPortInfo &p : ports) {
        QString label = p.portName() + QStringLiteral(" - ") + p.description();
        if (serialPortExcluded(p.portName(), m_excludeSerialPort))
            label += QStringLiteral(" [port RFID]");
        names << label;
    }
    if (names.isEmpty()) {
        QMessageBox::warning(parent, QStringLiteral("Smart Port Arduino"),
                             QStringLiteral("Aucun port série disponible."));
        return false;
    }

    bool ok = false;
    const QString selected = QInputDialog::getItem(parent, QStringLiteral("Smart Port Arduino"),
                                                   QStringLiteral("Sélectionnez le port Smart Port :"),
                                                   names, 0, false, &ok);
    if (!ok || selected.isEmpty())
        return false;

    QString portName = selected.left(selected.indexOf(QStringLiteral(" - "))).trimmed();
    if (portName.isEmpty())
        portName = selected.split(QStringLiteral(" - ")).value(0).trimmed();

    QSettings settings(QStringLiteral("ATLAS"), QStringLiteral("ATLAS"));
    const int prevBaud = settings.value(QStringLiteral("smartport/baudRate"), kDefaultBaudRate).toInt();
    const QStringList baudList = {QStringLiteral("115200"), QStringLiteral("9600"),
                                  QStringLiteral("57600"), QStringLiteral("38400"),
                                  QStringLiteral("74880"), QStringLiteral("230400")};
    int baudIdx = baudList.indexOf(QString::number(prevBaud));
    if (baudIdx < 0)
        baudIdx = 0;
    const QString baudStr = QInputDialog::getItem(parent, QStringLiteral("Smart Port Arduino"),
                                                  QStringLiteral("Vitesse (baud) :"),
                                                  baudList, baudIdx, false, &ok);
    if (!ok || baudStr.isEmpty())
        return false;

    settings.setValue(QStringLiteral("smartport/portName"), portName);
    settings.setValue(QStringLiteral("smartport/baudRate"), baudStr.toInt());
    applySerialParams();

    if (!tryOpenPort(portName, false)) {
        QMessageBox::warning(parent, QStringLiteral("Smart Port Arduino"),
                             QStringLiteral("Impossible de connecter Smart Port sur %1.\n"
                                            "Verifiez le baud et le sketch.\n%2")
                                 .arg(portName, m_serial ? m_serial->errorString() : QString()));
        return false;
    }
    return true;
}

int ArduinoMontage2::close_arduino()
{
    if (!m_serial)
        return 1;
    if (!m_serial->isOpen())
        return 1;
    m_serial->close();
    const QString old = m_portName;
    m_portName.clear();
    emit connectedChanged(false, old);
    return 0;
}

bool ArduinoMontage2::isConnected() const
{
    return m_serial && m_serial->isOpen();
}

QString ArduinoMontage2::portName() const
{
    return m_portName;
}

void ArduinoMontage2::sendLine(const QString &line)
{
    if (!m_serial || !m_serial->isOpen())
        return;
    const QByteArray frame = line.toUtf8() + '\n';
    m_serial->write(frame);
}

void ArduinoMontage2::setSecurityMode(bool enabled)
{
    sendLine(enabled ? QStringLiteral("MODE:1") : QStringLiteral("MODE:0"));
}

void ArduinoMontage2::setLighting(bool on)
{
    sendLine(on ? QStringLiteral("LIGHT:1") : QStringLiteral("LIGHT:0"));
}

void ArduinoMontage2::setFan(bool on)
{
    sendLine(on ? QStringLiteral("ACT:FAN:ON") : QStringLiteral("ACT:FAN:OFF"));
}

void ArduinoMontage2::setPump(bool on)
{
    sendLine(on ? QStringLiteral("ACT:PUMP:ON") : QStringLiteral("ACT:PUMP:OFF"));
}

void ArduinoMontage2::setBuzzer(bool on)
{
    sendLine(on ? QStringLiteral("ACT:BUZZER:ON") : QStringLiteral("ACT:BUZZER:OFF"));
}

void ArduinoMontage2::setFanAuto()
{
    sendLine(QStringLiteral("ACT:FAN:AUTO"));
}

void ArduinoMontage2::setPumpAuto()
{
    sendLine(QStringLiteral("ACT:PUMP:AUTO"));
}

void ArduinoMontage2::setBuzzerAuto()
{
    sendLine(QStringLiteral("ACT:BUZZER:AUTO"));
}

void ArduinoMontage2::setAllAuto()
{
    sendLine(QStringLiteral("ACT:ALL:AUTO"));
}

void ArduinoMontage2::requestStatus()
{
    sendLine(QStringLiteral("STATUS?"));
}

void ArduinoMontage2::ping()
{
    sendLine(QStringLiteral("PING"));
}

void ArduinoMontage2::onReadyRead()
{
    if (!m_serial)
        return;
    m_readBuffer.append(m_serial->readAll());

    while (true) {
        int cut = m_readBuffer.indexOf('\n');
        if (cut < 0)
            cut = m_readBuffer.indexOf('\r');
        if (cut < 0)
            break;

        const QByteArray raw = m_readBuffer.left(cut);
        m_readBuffer.remove(0, cut + 1);
        const QString line = QString::fromUtf8(raw).trimmed();
        if (!line.isEmpty())
            parseLine(line);
    }
    if (m_readBuffer.size() > kMaxLineLen)
        m_readBuffer.clear();
}

void ArduinoMontage2::parseLine(const QString &line)
{
    emit logLine(line);
    QString work = line.trimmed();
    const int statePos = work.indexOf(QStringLiteral("STATE;"), 0, Qt::CaseInsensitive);
    if (statePos >= 0)
        work = work.mid(statePos);

    const QString upper = work.toUpper();
    const bool hasKnownStateKey =
        upper.contains(QStringLiteral("SEC=")) || upper.contains(QStringLiteral("SEC:")) ||
        upper.contains(QStringLiteral("GAS=")) || upper.contains(QStringLiteral("GAS:")) ||
        upper.contains(QStringLiteral("HUM=")) || upper.contains(QStringLiteral("HUM:")) ||
        upper.contains(QStringLiteral("FLAME=")) || upper.contains(QStringLiteral("FLAME:")) ||
        upper.contains(QStringLiteral("RAIN=")) || upper.contains(QStringLiteral("RAIN:")) ||
        upper.contains(QStringLiteral("MOTION=")) || upper.contains(QStringLiteral("MOTION:")) ||
        upper.contains(QStringLiteral("MAG=")) || upper.contains(QStringLiteral("MAG:"));

    if (upper.startsWith(QStringLiteral("STATE;")) || hasKnownStateKey) {
        parseStateLine(work);
        return;
    }
}

bool ArduinoMontage2::parseBoolToken(const QString &value, bool *ok)
{
    const QString v = value.trimmed().toUpper();
    if (v == QStringLiteral("1") || v == QStringLiteral("ON") || v == QStringLiteral("TRUE")) {
        if (ok) *ok = true;
        return true;
    }
    if (v == QStringLiteral("0") || v == QStringLiteral("OFF") || v == QStringLiteral("FALSE")) {
        if (ok) *ok = true;
        return false;
    }
    if (ok) *ok = false;
    return false;
}

void ArduinoMontage2::parseStateLine(const QString &line)
{
    SmartPortRealtimeState s = m_lastState;
    const QStringList chunks = line.split(';', Qt::SkipEmptyParts);
    for (int i = 0; i < chunks.size(); ++i) {
        const QString p = chunks.at(i).trimmed();
        if (p.isEmpty())
            continue;
        int sep = p.indexOf('=');
        if (sep <= 0)
            sep = p.indexOf(':');
        if (sep <= 0)
            continue;
        QString key = p.left(sep).trimmed().toUpper();
        // Nettoyage des caracteres parasites (bruit serie) autour de la cle.
        while (!key.isEmpty() && !(key.front().isLetter() || key.front() == QChar('_')))
            key.remove(0, 1);
        while (!key.isEmpty() && !(key.back().isLetter() || key.back() == QChar('_')))
            key.chop(1);
        const QString val = p.mid(sep + 1).trimmed();

        // Champs non booleens.
        if (key.endsWith(QStringLiteral("WATER_CM"))) {
            bool okNum = false;
            const double cm = val.toDouble(&okNum);
            if (okNum)
                s.waterCm = cm;
            continue;
        } else if (key.endsWith(QStringLiteral("TIDE"))) {
            const QString t = val.trimmed().toUpper();
            if (t == QStringLiteral("LOW")) s.tideLow = true;
            else if (t == QStringLiteral("HIGH")) s.tideLow = false;
            continue;
        }

        bool ok = false;
        const bool b = parseBoolToken(val, &ok);
        if (!ok)
            continue;

        if (key.endsWith(QStringLiteral("SEC"))) s.securityMode = b;
        else if (key.endsWith(QStringLiteral("GAS"))) s.gas = b;
        else if (key.endsWith(QStringLiteral("HUM"))) s.humidity = b;
        else if (key.endsWith(QStringLiteral("FLAME"))) s.flame = b;
        else if (key.endsWith(QStringLiteral("RAIN"))) s.rain = b;
        else if (key.endsWith(QStringLiteral("MOTION"))) s.motion = b;
        else if (key.endsWith(QStringLiteral("MAG"))) s.magnetic = b;
        else if (key.endsWith(QStringLiteral("FAN"))) s.fan = b;
        else if (key.endsWith(QStringLiteral("PUMP"))) s.pump = b;
        else if (key.endsWith(QStringLiteral("BUZZER"))) s.buzzer = b;
        else if (key.endsWith(QStringLiteral("LIGHT"))) s.light = b;
        else if (key.endsWith(QStringLiteral("LED_GAS"))) s.ledGas = b;
        else if (key.endsWith(QStringLiteral("LED_HUM"))) s.ledHumidity = b;
        else if (key.endsWith(QStringLiteral("LED_FLAME"))) s.ledFlame = b;
        else if (key.endsWith(QStringLiteral("LED_MOTION"))) s.ledMotion = b;
    }

    m_lastState = s;
    emit stateUpdated(m_lastState);
}

