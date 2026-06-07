#include "quai_arduino_window.h"
#include "ui_quai_arduino_window.h"
#include "quai.h"
#include "connection.h"

#include <QComboBox>
#include <QDateTime>
#include <QLabel>
#include <QMessageBox>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QScrollBar>
#include <QSqlDatabase>
#include <QTextCursor>
#include <QTextDocument>

static void trimLogToMaxBlocks(QPlainTextEdit *edit, int maxBlocks)
{
    if (!edit)
        return;
    QTextDocument *doc = edit->document();
    while (doc->blockCount() > maxBlocks) {
        QTextCursor c(doc);
        c.movePosition(QTextCursor::Start);
        c.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
        c.removeSelectedText();
        if (!c.atEnd())
            c.deleteChar();
        else
            break;
    }
}

QuaiArduinoWindow::QuaiArduinoWindow(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::QuaiArduinoWindow)
{
    ui->setupUi(this);
    setWindowTitle(tr("Arduino — Porte capteurs (Quai_sys)"));

    connect(ui->btnRefresh, &QPushButton::clicked, this, &QuaiArduinoWindow::onRefreshClicked);
    connect(ui->btnValidate, &QPushButton::clicked, this, &QuaiArduinoWindow::onValidateClicked);
    connect(ui->btnForceOpen, &QPushButton::clicked, this, &QuaiArduinoWindow::onForceOpenClicked);
    connect(ui->btnForceClose, &QPushButton::clicked, this, &QuaiArduinoWindow::onForceCloseClicked);
    connect(ui->btnClearLog, &QPushButton::clicked, this, &QuaiArduinoWindow::onClearLogClicked);

    refreshActiveQuaisCombo();
    appendQuaiGateLog(tr("Panneau Arduino prêt."));
}

QuaiArduinoWindow::~QuaiArduinoWindow()
{
    delete ui;
}

void QuaiArduinoWindow::appendQuaiGateLog(const QString &message)
{
    if (!ui->plainTextLog)
        return;
    const QString line = QDateTime::currentDateTime().toString(QStringLiteral("HH:mm:ss")) + QLatin1Char(' ')
                         + message;
    ui->plainTextLog->appendPlainText(line);
    trimLogToMaxBlocks(ui->plainTextLog, 500);
    if (QScrollBar *sb = ui->plainTextLog->verticalScrollBar())
        sb->setValue(sb->maximum());
}

void QuaiArduinoWindow::refreshStatBoxes()
{
    QSqlDatabase db = QSqlDatabase::database(Connection::connectionName());
    if (!db.isOpen()) {
        ui->labelActiveValue->setText(QStringLiteral("—"));
        ui->labelInactiveValue->setText(QStringLiteral("—"));
        return;
    }
    const int active = Quai::countActiveQuais();
    const int total = Quai::countTotalQuais();
    ui->labelActiveValue->setText(QString::number(active));
    ui->labelInactiveValue->setText(QString::number(qMax(0, total - active)));
}

void QuaiArduinoWindow::refreshActiveQuaisCombo()
{
    ui->comboQuais->clear();
    refreshStatBoxes();

    QSqlDatabase db = QSqlDatabase::database(Connection::connectionName());
    if (!db.isOpen()) {
        ui->labelStatus->setText(tr("Base de données non disponible."));
        return;
    }
    const QVector<QPair<int, QString>> rows = Quai::activeQuaisIdAndLabel();
    for (const auto &p : rows)
        ui->comboQuais->addItem(p.second, p.first);
    if (rows.isEmpty())
        ui->labelStatus->setText(tr("Aucun quai actif — rien à affecter pour l’instant."));
    else
        ui->labelStatus->setText(tr("%1 quai(s) actif(s) dans la liste — sélectionnez celui concerné.")
                                     .arg(rows.size()));
}

void QuaiArduinoWindow::onRefreshClicked()
{
    refreshActiveQuaisCombo();
    appendQuaiGateLog(tr("Liste des quais actifs actualisée."));
}

void QuaiArduinoWindow::onClearLogClicked()
{
    if (ui->plainTextLog)
        ui->plainTextLog->clear();
    appendQuaiGateLog(tr("Journal effacé."));
}

void QuaiArduinoWindow::onForceOpenClicked()
{
    appendQuaiGateLog(tr("Action locale : forcer ouverture moteur (PORTE_OPEN)."));
    emit forceOpenQuaiGateRequested();
}

void QuaiArduinoWindow::onForceCloseClicked()
{
    appendQuaiGateLog(tr("Action locale : forcer fermeture moteur (PORTE_CLOSE)."));
    emit forceCloseQuaiGateRequested();
}

void QuaiArduinoWindow::onValidateClicked()
{
    const int quaiId = ui->comboQuais->currentData().toInt();
    if (quaiId <= 0) {
        QMessageBox::warning(this, windowTitle(),
                             tr("Choisissez un quai dans la liste (ou actualisez si elle est vide)."));
        return;
    }
    if (!Quai::setStatutForQuaiId(quaiId, QStringLiteral("Inactif"))) {
        QMessageBox::warning(this, windowTitle(),
                             tr("Échec de la mise à jour : %1").arg(Quai::lastErrorMessage()));
        return;
    }
    appendQuaiGateLog(tr("Quai %1 passé en Inactif (validation manuelle).").arg(quaiId));
    emit assignmentCompleted(quaiId);
    QMessageBox::information(this, windowTitle(),
                             tr("Le quai %1 est maintenant inactif.").arg(quaiId));
    refreshActiveQuaisCombo();
}
