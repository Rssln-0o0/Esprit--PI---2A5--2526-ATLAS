#include "libraries.h"
#include "mainwindow.h"
#include <QElapsedTimer>
#include <QLineEdit>
#include <QThread>
#include <QWidget>
#include <QInputDialog>
#include <QSettings>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QMessageBox>

static const int kStockWeightBaudRate = 9600;

namespace
{

struct WeightReadResult {
	bool ok = false;
	double kilograms = 0.0;
	QString rawLine;
	QString errorMessage;
};

static void setWeightPanelMessage(MainWindow *window, const QString &text, const QString &color)
{
	if (!window)
		return;

	if (QWidget *label = window->findChild<QWidget *>(QStringLiteral("StockWeighMsg"))) {
		label->setProperty("text", text);
		label->setStyleSheet(QStringLiteral("color: %1; font-weight: 700;").arg(color));
	}
}

static WeightReadResult parseWeightPayload(const QByteArray &payload)
{
	WeightReadResult result;
	const QString text = QString::fromUtf8(payload).trimmed();

	if (text.isEmpty()) {
		result.errorMessage = QObject::tr("Aucune donnée reçue de l'Arduino Uno.");
		return result;
	}

	const QStringList lines = text.split(QRegularExpression(QStringLiteral("[\\r\\n]+")), Qt::SkipEmptyParts);
	const QRegularExpression noWeightTokenRx(QStringLiteral("(?:^|\\W)(?:NO_WEIGHT|NO LOAD|NO-LOAD|EMPTY)(?:$|\\W)"),
											QRegularExpression::CaseInsensitiveOption);
	const QRegularExpression noResponseRx(QStringLiteral("(?:^|\\W)(?:ERROR: HX711_NOT_READY|ERROR: INVALID_READING|ERROR: HX711_NO_STABLE_READING)(?:$|\\W)"),
											QRegularExpression::CaseInsensitiveOption);
	const QRegularExpression numberRx(QStringLiteral("([+-]?\\d+(?:[.,]\\d+)?)"));
	const QRegularExpression noWeightRx(
		QStringLiteral("\\b(?:no\\s*weight|empty|no\\s*load|aucun(?:e)?\\s+poids?|pas\\s+de\\s+poids|rien)\\b"),
		QRegularExpression::CaseInsensitiveOption);
	const QRegularExpression gramsRx(QStringLiteral("(?:^|\\W)(?:g|grammes?|grams?)\\b"),
									 QRegularExpression::CaseInsensitiveOption);
	const QRegularExpression kilogramsRx(QStringLiteral("\\bkg\\b"),
										 QRegularExpression::CaseInsensitiveOption);

	for (int i = lines.size() - 1; i >= 0; --i) {
		const QString line = lines.at(i).trimmed();
		if (line.isEmpty())
			continue;

		if (noWeightTokenRx.match(line).hasMatch()) {
			result.errorMessage = QObject::tr("Aucun poids détecté sur la balance.");
			return result;
		}

		if (noResponseRx.match(line).hasMatch()) {
			result.errorMessage = QObject::tr("Le capteur n'a pas renvoyé de mesure exploitable.");
			return result;
		}

		if (noWeightRx.match(line).hasMatch()) {
			result.errorMessage = QObject::tr("Aucun poids détecté sur la balance.");
			return result;
		}

		const QRegularExpressionMatch match = numberRx.match(line);
		if (!match.hasMatch())
			continue;

		QString numberText = match.captured(1);
		numberText.replace(',', '.');

		bool ok = false;
		double value = numberText.toDouble(&ok);
		if (!ok)
			continue;

		const QString lowerLine = line.toLower();
		if (!kilogramsRx.match(lowerLine).hasMatch() && gramsRx.match(lowerLine).hasMatch())
			value /= 1000.0;

		if (value <= 0.0) {
			result.errorMessage = QObject::tr("Le poids mesuré est nul ou invalide.");
			return result;
		}

		result.ok = true;
		result.kilograms = value;
		result.rawLine = line;
		return result;
	}

	result.errorMessage = QObject::tr("Réponse invalide de l'Arduino Uno: %1").arg(text.left(140));
	return result;
}

static bool ensureStockPortOpen(MainWindow *window, QSerialPort *port)
{
	if (port->isOpen())
		return true;

	QSettings settings(QStringLiteral("ATLAS"), QStringLiteral("ATLAS"));
	QString savedPort = settings.value(QStringLiteral("StockBalancePort/portName")).toString();

	if (savedPort.isEmpty()) {
		QStringList names;
		for (const QSerialPortInfo &p : QSerialPortInfo::availablePorts()) {
			names << p.portName();
		}
		if (names.isEmpty()) {
			QMessageBox::warning(window, QObject::tr("Balance"), QObject::tr("Aucun port série trouvé."));
			return false;
		}
		bool ok;
		savedPort = QInputDialog::getItem(window, QObject::tr("Balance"), QObject::tr("Port COM de la balance:"), names, 0, false, &ok);
		if (!ok || savedPort.isEmpty())
			return false;
		settings.setValue(QStringLiteral("StockBalancePort/portName"), savedPort);
	}

	port->setPortName(savedPort);
	port->setBaudRate(kStockWeightBaudRate);

	if (!port->open(QIODevice::ReadWrite)) {
		QString errDetail = port->errorString();
		if (port->error() == QSerialPort::PermissionError) {
			errDetail += QObject::tr("\n\nLe module RFID ou une autre carte a probablement bloqué ce port à cause des anciens paramètres sauvegardés. Réinitialisation des ports...");
			
			// Clear all potentially conflicting saved ports in the registry
			settings.remove(QStringLiteral("arduinorfid_employe/portName"));
			settings.remove(QStringLiteral("arduinorfid_pecheur/portName"));
			settings.remove(QStringLiteral("arduino_quai_gate/portName"));
			settings.remove(QStringLiteral("arduinorfid/portName"));
			settings.remove(QStringLiteral("smartport/portName"));
			settings.remove(QStringLiteral("esp32cam/portName"));
		}

		QMessageBox::warning(window, QObject::tr("Erreur"), QObject::tr("Impossible d'ouvrir %1:\n%2").arg(savedPort).arg(errDetail));
		settings.remove(QStringLiteral("StockBalancePort/portName"));
		return false;
	}
	return true;
}

static QSerialPort* getSharedStockSerial(MainWindow *window)
{
	QSerialPort *port = window->findChild<QSerialPort *>(QStringLiteral("SharedStockSerialPort"));
	if (!port) {
		port = new QSerialPort(window);
		port->setObjectName(QStringLiteral("SharedStockSerialPort"));
		QObject::connect(port, &QSerialPort::readyRead, window, [window, port]() {
			QByteArray current = port->property("payloadBuf").toByteArray();
			current.append(port->readAll());
			if (current.contains('\n')) {
				port->setProperty("payloadBuf", QByteArray());
				WeightReadResult result = parseWeightPayload(current);
				if (result.ok) {
					const QString displayWeight = QString::number(result.kilograms, 'f', result.kilograms < 10.0 ? 2 : 1);
					const int intWeight = static_cast<int>(result.kilograms);
					const QString stringIntWeight = QString::number(intWeight);
					
					QWidget *quantityEdit = window->findChild<QWidget *>(QStringLiteral("stockQuantiteEdit"));
					QWidget *saveBtn = window->findChild<QWidget *>(QStringLiteral("btnStockSave"));
					
					bool inEditMode = (saveBtn && saveBtn->isVisible());

					if (quantityEdit) quantityEdit->setProperty("text", stringIntWeight);
					setWeightPanelMessage(window, QObject::tr("Poids: %1 kg -> %2 enregistré").arg(displayWeight, stringIntWeight), QStringLiteral("#2ecc71"));

					if (inEditMode) {
						QMetaObject::invokeMethod(window, "on_btnStockSave_clicked");
					}
				} else {
					const QString msg = result.errorMessage.isEmpty() ? QObject::tr("Aucune mesure valide.") : result.errorMessage;
					setWeightPanelMessage(window, msg, QStringLiteral("#e74c3c"));
				}
			} else {
				port->setProperty("payloadBuf", current);
			}
		});
	}
	return port;
}

} // namespace

void MainWindow::on_btnStockWeight_clicked()
{
	QWidget *quantityEdit = findChild<QWidget *>(QStringLiteral("stockQuantiteEdit"));
	if (!quantityEdit) {
		QMessageBox::warning(this, tr("Balance stock"), tr("L'interface de stock n'est pas disponible."));
		return;
	}

	QSerialPort *serial = getSharedStockSerial(this);
	if (!ensureStockPortOpen(this, serial)) {
		setWeightPanelMessage(this, tr("Pas de port configuré ou erreur d'ouverture."), QStringLiteral("#e74c3c"));
		return;
	}

	setWeightPanelMessage(this, tr("Mesure en cours (non-bloquant)..."), QStringLiteral("#f1c40f"));
	serial->setProperty("payloadBuf", QByteArray());
	serial->write(QByteArrayLiteral("REQUEST_WEIGHT\n"));
}

void MainWindow::on_btnStockWeightAnnuler_clicked()
{
	if (QWidget *label = findChild<QWidget *>(QStringLiteral("StockWeighMsg"))) {
		label->setProperty("text", tr("Placez le stock sur la balance et cliquez sur le bouton"));
		label->setStyleSheet(QStringLiteral("color: #ffffff; font-weight: 700;"));
	}

	weight_window_invisible();
}
