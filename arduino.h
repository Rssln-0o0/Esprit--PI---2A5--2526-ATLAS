#ifndef ARDUINO_H
#define ARDUINO_H

#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>
#include <QByteArray>
#include <QString>
#include <QImage>

class QWidget;

/**
 * Liaison série PC ↔ ESP32-CAM (câble USB), sur le modèle du cours (QSerialPort).
 * Détection des convertisseurs USB–série courants (CH340, CP210x, FTDI).
 */
class Arduino
{
public:
    Arduino();
    ~Arduino();

    /** Ne pas ouvrir ce COM (ex. Arduino Uno RFID déjà connecté sur le même PC). */
    void set_exclude_serial_port(const QString &portName);

    int connect_arduino();
    int close_arduino();
    void write_to_arduino(QByteArray d);
    QByteArray read_from_arduino();
    QSerialPort *getserial();
    QString getarduino_port_name();

    /** Envoie la commande flash LED (protocole firmware : 'L' + octet 0–255). */
    void write_flash_led(quint8 value);

    /**
     * Demande une image JPEG à l'ESP32-CAM (commande 'C'), lit l'en-tête 4 octets + données.
     * @param out image chargée (miroir vertical comme l’aperçu caméra)
     * @param errorMessage message d’erreur pour affichage UI
     */
    bool capture_esp32_cam_jpeg(QImage *out, QString *errorMessage = nullptr);

    /**
     * Si aucun port auto-détecté : dialogue pour choisir COM + débit, enregistré dans QSettings (ATLAS).
     * @return true si le port est ouvert après configuration     */
    bool prompt_configure_port(QWidget *parent);

private:
    static bool port_matches_esp32_bridge(const QSerialPortInfo &info);
    void apply_serial_params();
    void flush_input();
    bool ensure_open(QString *errorMessage);

    QSerialPort *serial = nullptr;
    QString m_excludeSerialPort;
    QString arduino_port_name;
    bool arduino_is_available = false;
    QByteArray data;
};

#endif // ARDUINO_H
