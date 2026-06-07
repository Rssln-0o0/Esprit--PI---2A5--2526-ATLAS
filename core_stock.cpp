#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "stock.h"
#include "connection.h"
#include "libraries.h"
#include <QThread>
#include <QSharedPointer>


namespace
{
    QString escapeSqlLiteral(QString value)
    {
        value.replace(QLatin1Char('\''), QLatin1String("''"));
        return value;
    }

    QString stockOrderBySql(const QString &critere)
    {
        if (critere == QStringLiteral("État") || critere == QStringLiteral("Etat") || critere.compare(QStringLiteral("etat"), Qt::CaseInsensitive) == 0)
        {
            return QStringLiteral(
                "CASE "
                "WHEN UPPER(ETAT) = 'VENDUE' THEN 1 "
                "WHEN UPPER(ETAT) = 'FRAIS' THEN 2 "
                "WHEN UPPER(ETAT) IN ('STOCKE', 'STOCKÉ') THEN 3 "
                "ELSE 4 END");
        }

        if (critere == QStringLiteral("Quantité") || critere == QStringLiteral("Quantite") || critere.compare(QStringLiteral("quantite"), Qt::CaseInsensitive) == 0)
            return QStringLiteral("QUANTITE");

        return QStringLiteral("ESPECE");
    }

    QString escapeCsvField(QString value)
    {
        value.replace('"', "\"\"");
        const bool needsQuotes = value.contains(',') || value.contains('"') || value.contains('\n') || value.contains('\r');
        if (needsQuotes)
            return '"' + value + '"';
        return value;
    }

    QStringList parseCsvLine(const QString &line)
    {
        QStringList result;
        QString field;
        bool inQuotes = false;

        for (int i = 0; i < line.size(); ++i)
        {
            const QChar c = line.at(i);
            if (c == '"')
            {
                if (inQuotes && i + 1 < line.size() && line.at(i + 1) == '"')
                {
                    field += '"';
                    ++i;
                }
                else
                {
                    inQuotes = !inQuotes;
                }
                continue;
            }

            if (c == ',' && !inQuotes)
            {
                result << field;
                field.clear();
                continue;
            }

            field += c;
        }

        result << field;
        return result;
    }

    bool stockIdExistsInDatabase(int stockId)
    {
        QSqlDatabase db = QSqlDatabase::database(Connection::connectionName());
        if (!db.isValid() || !db.isOpen())
            db = QSqlDatabase::database();

        if (!db.isValid() || !db.isOpen())
            return false;

        QSqlQuery query(db);
        query.prepare(QStringLiteral("SELECT COUNT(*) FROM STOCK WHERE STOCK_ID = :id"));
        query.bindValue(QStringLiteral(":id"), QString::number(stockId));
        if (!query.exec() || !query.next())
            return false;

        return query.value(0).toInt() > 0;
    }

    QString dateOnlyText(const QVariant &value)
    {
        if (value.metaType().id() == QMetaType::QDate)
            return value.toDate().toString(QStringLiteral("yyyy-MM-dd"));

        if (value.metaType().id() == QMetaType::QDateTime)
            return value.toDateTime().date().toString(QStringLiteral("yyyy-MM-dd"));

        const QString raw = value.toString().trimmed();
        if (raw.isEmpty())
            return QString();

        const QDate isoDate = QDate::fromString(raw, QStringLiteral("yyyy-MM-dd"));
        if (isoDate.isValid())
            return isoDate.toString(QStringLiteral("yyyy-MM-dd"));

        const QDateTime isoDateTime = QDateTime::fromString(raw, Qt::ISODate);
        if (isoDateTime.isValid())
            return isoDateTime.date().toString(QStringLiteral("yyyy-MM-dd"));

        const int splitIndex = qMax(raw.indexOf(' '), raw.indexOf('T'));
        if (splitIndex > 0) {
            const QString onlyDatePart = raw.left(splitIndex).trimmed();
            const QDate parsed = QDate::fromString(onlyDatePart, QStringLiteral("yyyy-MM-dd"));
            if (parsed.isValid())
                return parsed.toString(QStringLiteral("yyyy-MM-dd"));
        }

        return raw;
    }
}

namespace
{
    QString findProjectRootPath()
    {
        auto findFrom = [](const QString &startPath)
        {
            QDir dir(startPath);
            for (int i = 0; i < 8; ++i)
            {
                if (dir.exists(QStringLiteral("mainwindow.ui")))
                    return dir.absolutePath();
                if (!dir.cdUp())
                    break;
            }
            return QString();
        };

        const QString rootFromCwd = findFrom(QDir::currentPath());
        if (!rootFromCwd.isEmpty())
            return rootFromCwd;

        const QString rootFromApp = findFrom(QCoreApplication::applicationDirPath());
        if (!rootFromApp.isEmpty())
            return rootFromApp;

        return QDir::currentPath();
    }

    QString runStockChatbotScript(const QString &userInput, QString *errorOut = nullptr)
    {
        const QString scriptPath = QDir(findProjectRootPath()).filePath(QStringLiteral("atlas_ai_stock.py"));
        if (!QFileInfo::exists(scriptPath))
        {
            if (errorOut)
                *errorOut = QObject::tr("Script introuvable: %1").arg(QDir::toNativeSeparators(scriptPath));
            return QString();
        }

        struct PythonCandidate
        {
            QString program;
            QStringList prefixArgs;
        };

        const QList<PythonCandidate> candidates = {
            {QStringLiteral("py"), {QStringLiteral("-3"), QStringLiteral("-X"), QStringLiteral("utf8")}},
            {QStringLiteral("python"), {QStringLiteral("-X"), QStringLiteral("utf8")}},
            {QStringLiteral("python3"), {QStringLiteral("-X"), QStringLiteral("utf8")}}};

        QString lastError;
        for (const PythonCandidate &candidate : candidates)
        {
            QProcess process;
            QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
            env.remove(QStringLiteral("PYTHONHOME"));
            env.remove(QStringLiteral("PYTHONPATH"));
            env.insert(QStringLiteral("PYTHONUTF8"), QStringLiteral("1"));
            env.insert(QStringLiteral("PYTHONIOENCODING"), QStringLiteral("utf-8"));
            process.setProcessEnvironment(env);

            QStringList args = candidate.prefixArgs;
            args << scriptPath << userInput;

            process.start(candidate.program, args);
            if (!process.waitForStarted(2500))
            {
                lastError = QObject::tr("Impossible de lancer %1").arg(candidate.program);
                continue;
            }

            if (!process.waitForFinished(45000))
            {
                process.kill();
                process.waitForFinished(1000);
                lastError = QObject::tr("Le chatbot a depasse le delai d'attente.");
                continue;
            }

            const QString stdOut = QString::fromUtf8(process.readAllStandardOutput()).trimmed();
            const QString stdErr = QString::fromUtf8(process.readAllStandardError()).trimmed();

            if (process.exitStatus() == QProcess::NormalExit && process.exitCode() == 0 && !stdOut.isEmpty())
                return stdOut;

            lastError = !stdErr.isEmpty() ? stdErr : QObject::tr("Aucune reponse exploitable recue.");
        }

        if (errorOut)
            *errorOut = lastError;
        return QString();
    }
} // namespace

void MainWindow::chatbot_invisible()
{
    if (ui->chatbot_frame)
        ui->chatbot_frame->setVisible(false);
}

void MainWindow::weight_window_invisible()
{
    if (ui->weight_frame)
        ui->weight_frame->setVisible(false);
}

void MainWindow::on_btnStockExtWeight_clicked()
 {
        if (ui->weight_frame)
            ui->weight_frame->setVisible(true);
        if (ui->chatbot_frame)
            ui->chatbot_frame->setVisible(false);
 }

void MainWindow::on_btnExpDataPdf_clicked()
{
    const QString defaultName = QDir::toNativeSeparators(
        QDir::currentPath() + QDir::separator() +
        QStringLiteral("stock_export_%1.pdf").arg(QDateTime::currentDateTime().toString(QStringLiteral("yyyyMMdd_HHmmss"))));

    const QString pdfFilePath = QFileDialog::getSaveFileName(
        this,
        tr("Exporter les donnees stock en PDF"),
        defaultName,
        tr("PDF Files (*.pdf)"));

    if (pdfFilePath.trimmed().isEmpty())
        return;

    QString whereAndSuffix;
    const QString searchText = ui->stockSearchEdit ? ui->stockSearchEdit->text().trimmed() : QString();
    if (!searchText.isEmpty())
    {
        const QString escapedTerm = escapeSqlLiteral(searchText);
        const QString likeLiteral = QStringLiteral("'%") + escapedTerm + QStringLiteral("%'");
        whereAndSuffix = QStringLiteral(" AND (UPPER(ESPECE) LIKE UPPER(%1) OR TO_CHAR(CIN_VENT) LIKE %1)").arg(likeLiteral);
    }

    const QString sortLabel = ui->stockSortCombo ? ui->stockSortCombo->currentText() : QString();
    const QString orderBySuffix = QStringLiteral(" ORDER BY ") + stockOrderBySql(sortLabel);

    int rowCount = 0;
    Stock stock;
    if (!stock.exportStock_Data_Pdf(pdfFilePath, whereAndSuffix, orderBySuffix, &rowCount))
    {
        const QString errorText = Stock::lastErrorMessage().trimmed();
        QMessageBox::warning(this,
                             tr("Export PDF"),
                             errorText.isEmpty()
                                 ? tr("Echec de l'export PDF des donnees stock.")
                                 : tr("Echec de l'export PDF des donnees stock:\n%1").arg(errorText));
        return;
    }

    QMessageBox::information(this,
                             tr("Export PDF"),
                             tr("Export termine avec succes (%1 ligne(s)) vers:\n%2")
                                 .arg(rowCount)
                                 .arg(QDir::toNativeSeparators(pdfFilePath)));
}


void MainWindow::on_chatbot_button_clicked()
{
    if (!ui->chatbot_frame)
        return;

    const bool willShow = !ui->chatbot_frame->isVisible();
    ui->chatbot_frame->setVisible(willShow);

    if (!willShow || m_stockChatbotUiReady)
        return;

    setupStockChatbotUi();
    m_stockChatbotUiReady = true;

    QString greeting;
    const int hour = QTime::currentTime().hour();
    if (hour >= 5 && hour < 12)
        greeting = QStringLiteral("Bonjour, ATLAS est a votre service.");
    else if (hour >= 12 && hour < 18)
        greeting = QStringLiteral("Bon apres-midi, ATLAS est a votre service.");
    else if (hour >= 18 && hour < 22)
        greeting = QStringLiteral("Bonsoir, ATLAS est a votre service.");
    else
        greeting = QStringLiteral("Bonne nuit, ATLAS est a votre service.");

    if (QTextEdit *out = findChild<QTextEdit *>(QStringLiteral("textCB_Stock")))
    {
        if (out->toPlainText().trimmed().isEmpty())
            out->setPlainText(greeting + QStringLiteral("\n\nPosez une question sur le module Stock."));
        out->setProperty("chatbotIntroShown", true);
    }
    else if (QPlainTextEdit *plainOut = findChild<QPlainTextEdit *>(QStringLiteral("textCB_Stock")))
    {
        if (plainOut->toPlainText().trimmed().isEmpty())
            plainOut->setPlainText(greeting + QStringLiteral("\n\nPosez une question sur le module Stock."));
        plainOut->setProperty("chatbotIntroShown", true);
    }
}

void MainWindow::setupStockChatbotUi()
{
    if (ui->tabWidgetStockVentes)
    {
        QWidget *host = ui->tabWidgetStockVentes;

        if (ui->chatbot_button && ui->chatbot_button->parentWidget() != host)
            ui->chatbot_button->setParent(host);

        if (ui->chatbot_frame && ui->chatbot_frame->parentWidget() != host)
            ui->chatbot_frame->setParent(host);

        host->installEventFilter(this);
    }

    if (QTextEdit *out = findChild<QTextEdit *>(QStringLiteral("textCB_Stock")))
    {
        out->setReadOnly(true);
        out->setUndoRedoEnabled(false);
        out->document()->setDocumentMargin(14);
        out->document()->setMaximumBlockCount(220);
        out->setStyleSheet(QStringLiteral(
            "QTextEdit#textCB_Stock {"
            "background-color:#141d24;"
            "border:1px solid #2d4d5f;"
            "border-radius:12px;"
            "padding:10px;"
            "color:#eaf4fb;"
            "font: 11pt 'Segoe UI';"
            "}"));
    }

    if (QPlainTextEdit *plainOut = findChild<QPlainTextEdit *>(QStringLiteral("textCB_Stock")))
    {
        plainOut->setReadOnly(true);
        plainOut->setUndoRedoEnabled(false);
        plainOut->document()->setDocumentMargin(14);
        plainOut->document()->setMaximumBlockCount(220);
        plainOut->setStyleSheet(QStringLiteral(
            "QPlainTextEdit#textCB_Stock {"
            "background-color:#141d24;"
            "border:1px solid #2d4d5f;"
            "border-radius:12px;"
            "padding:10px;"
            "color:#eaf4fb;"
            "font: 11pt 'Segoe UI';"
            "}"));
    }

    if (ui->chatbot_button)
        ui->chatbot_button->setVisible(true);
    positionStockChatbotOverlay();
}

void MainWindow::positionStockChatbotOverlay()
{
    if (!ui || !ui->tabWidgetStockVentes)
        return;

    QWidget *host = ui->tabWidgetStockVentes;
    const int margin = 14;

    if (ui->chatbot_button)
    {
        const QSize buttonSize = ui->chatbot_button->size().isValid()
                                     ? ui->chatbot_button->size()
                                     : ui->chatbot_button->sizeHint();
        const int bx = qMax(margin, host->width() - buttonSize.width() - margin);
        const int by = qMax(margin, host->height() - buttonSize.height() - margin);
        ui->chatbot_button->move(bx, by);
        ui->chatbot_button->raise();
    }

    if (ui->chatbot_frame)
    {
        int frameWidth = ui->chatbot_frame->width();
        int frameHeight = ui->chatbot_frame->height();

        frameWidth = qMin(frameWidth, qMax(260, host->width() - 2 * margin));
        frameHeight = qMin(frameHeight, qMax(180, host->height() - 70));

        const int buttonTop = ui->chatbot_button
                                  ? ui->chatbot_button->y()
                                  : host->height() - margin;

        const int fx = qMax(margin, host->width() - frameWidth - margin);
        const int fy = qMax(36, buttonTop - frameHeight - 10);

        ui->chatbot_frame->setGeometry(fx, fy, frameWidth, frameHeight);
        ui->chatbot_frame->raise();
    }
}

void MainWindow::submitStockChatbotQuery()
{
    if (m_stockChatbotBusy)
        return;

    QLineEdit *input = findChild<QLineEdit *>(QStringLiteral("inputCB_Stock"));
    if (!input)
        return;

    const QString userText = input->text().trimmed();
    if (userText.isEmpty())
        return;

    if (QTextEdit *out = findChild<QTextEdit *>(QStringLiteral("textCB_Stock")))
    {
        if (out->property("chatbotIntroShown").toBool())
        {
            out->clear();
            out->setProperty("chatbotIntroShown", false);
        }
    }
    else if (QPlainTextEdit *plainOut = findChild<QPlainTextEdit *>(QStringLiteral("textCB_Stock")))
    {
        if (plainOut->property("chatbotIntroShown").toBool())
        {
            plainOut->clear();
            plainOut->setProperty("chatbotIntroShown", false);
        }
    }

    setStockChatbotBusyState(true);
    appendStockChatbotConversation(userText, false);
    appendStockChatbotConversation(QStringLiteral("ATLAS est en train de preparer une reponse..."), true, true);
    input->clear();

    auto result = QSharedPointer<QPair<QString, QString>>::create();
    QThread *worker = QThread::create([userText, result]() {
        QString scriptError;
        const QString botReply = runStockChatbotScript(userText, &scriptError);
        *result = qMakePair(botReply, scriptError);
    });
    worker->setParent(this);

    QObject::connect(worker, &QThread::finished, this, [this, worker, result]() {
        if (!result->first.isEmpty()) {
            appendStockChatbotConversation(result->first, true);
        } else {
            appendStockChatbotConversation(
                QStringLiteral("ATLAS n'a pas pu traiter la demande. %1")
                    .arg(result->second.isEmpty() ? QStringLiteral("") : result->second),
                true);
        }

        setStockChatbotBusyState(false);
        if (QLineEdit *inputField = findChild<QLineEdit *>(QStringLiteral("inputCB_Stock")))
            inputField->setFocus();
        worker->deleteLater();
    });

    worker->start();
}

void MainWindow::appendStockChatbotConversation(const QString &line, bool fromAtlas, bool isStatus)
{
    if (QTextEdit *out = findChild<QTextEdit *>(QStringLiteral("textCB_Stock")))
    {
        const QString escaped = QString(line).toHtmlEscaped().replace(QStringLiteral("\n"), QStringLiteral("<br/>"));
        const QString sender = fromAtlas ? QStringLiteral("ATLAS") : QStringLiteral("Vous");
        const QString bubbleColor = isStatus
                                        ? QStringLiteral("#1a2a34")
                                        : (fromAtlas ? QStringLiteral("#12303f") : QStringLiteral("#1c3444"));
        const QString textStyle = isStatus
                                      ? QStringLiteral("font-style:italic; opacity:0.9;")
                                      : QStringLiteral("");
        const QString html = QStringLiteral(
                                 "<div style='margin:8px 0 12px 0;'>"
                                 "<div style='color:#87b7d3; font-size:9pt; margin-bottom:4px;'>%1</div>"
                                 "<div style='background:%2; border:1px solid #2d4d5f; border-radius:10px; padding:8px 10px; color:#edf7ff; line-height:1.6; font-size:11pt; %3'>%4</div>"
                                 "</div>")
                                 .arg(sender)
                                 .arg(bubbleColor)
                                 .arg(textStyle)
                                 .arg(escaped);
        out->append(html);
        out->moveCursor(QTextCursor::End);
        return;
    }

    if (QPlainTextEdit *plainOut = findChild<QPlainTextEdit *>(QStringLiteral("textCB_Stock")))
    {
        const QString sender = fromAtlas ? QStringLiteral("ATLAS") : QStringLiteral("Vous");
        plainOut->appendPlainText(sender + QStringLiteral(":\n") + line + QStringLiteral("\n"));
        plainOut->moveCursor(QTextCursor::End);
    }
}

void MainWindow::setStockChatbotBusyState(bool busy)
{
    m_stockChatbotBusy = busy;

    if (QLineEdit *input = findChild<QLineEdit *>(QStringLiteral("inputCB_Stock")))
    {
        input->setEnabled(!busy);
        input->setPlaceholderText(busy
                                      ? QStringLiteral("ATLAS traite votre question...")
                                      : QStringLiteral("Posez votre question Stock..."));
    }

    if (ui && ui->btnCB_Stock)
        ui->btnCB_Stock->setEnabled(!busy);
}

void MainWindow::on_btnCB_Stock_clicked()
{
    submitStockChatbotQuery();
}

void MainWindow::on_inputCB_Stock_returnPressed()
{
    submitStockChatbotQuery();
}

QString MainWindow::stockHistoryFilePath() const
{
    auto findProjectRoot = [](const QString &startPath)
    {
        QDir dir(startPath);
        for (int i = 0; i < 8; ++i)
        {
            if (dir.exists(QStringLiteral("mainwindow.ui")))
                return dir.absolutePath();
            if (!dir.cdUp())
                break;
        }
        return QString();
    };

    const QString rootFromCwd = findProjectRoot(QDir::currentPath());
    if (!rootFromCwd.isEmpty())
        return QDir(rootFromCwd).filePath(QStringLiteral("history.csv"));

    const QString rootFromApp = findProjectRoot(QCoreApplication::applicationDirPath());
    if (!rootFromApp.isEmpty())
        return QDir(rootFromApp).filePath(QStringLiteral("history.csv"));

    return QDir(QDir::currentPath()).filePath(QStringLiteral("history.csv"));
}

void MainWindow::clearStockHistoryTable()
{
    if (QTableWidget *historyWidget = findChild<QTableWidget *>(QStringLiteral("historyStockTable")))
    {
        historyWidget->clearContents();
        historyWidget->setRowCount(0);
    }
}

void MainWindow::setupStockHistory()
{
    if (QTableWidget *historyWidget = findChild<QTableWidget *>(QStringLiteral("historyStockTable")))
    {
        historyWidget->setColumnCount(6);
        historyWidget->setHorizontalHeaderLabels(
            {QStringLiteral("Date/Heure"), QStringLiteral("Action"), QStringLiteral("Stock ID"),
             QStringLiteral("Espèce"), QStringLiteral("Quantité"), QStringLiteral("État")});
        historyWidget->setAlternatingRowColors(true);
        historyWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
        historyWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
        historyWidget->setSelectionMode(QAbstractItemView::SingleSelection);
        if (historyWidget->horizontalHeader())
        {
            historyWidget->horizontalHeader()->setVisible(true);
            historyWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
            historyWidget->horizontalHeader()->setStretchLastSection(true);
        }
        if (historyWidget->verticalHeader())
            historyWidget->verticalHeader()->setVisible(false);
    }

    loadStockHistoryFromFile();
}

void MainWindow::loadStockHistoryFromFile()
{
    clearStockHistoryTable();

    QFile historyFile(stockHistoryFilePath());
    if (!historyFile.exists())
        return;

    if (!historyFile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QMessageBox::warning(this, tr("Historique"), tr("Impossible de lire le fichier d'historique."));
        return;
    }

    QTextStream stream(&historyFile);
    stream.setEncoding(QStringConverter::Utf8);

    bool firstLine = true;
    while (!stream.atEnd())
    {
        const QString line = stream.readLine().trimmed();
        if (line.isEmpty())
            continue;

        if (firstLine && line.startsWith(QStringLiteral("Date/Heure,Action,")))
        {
            firstLine = false;
            continue;
        }
        firstLine = false;

        QStringList fields = parseCsvLine(line);
        while (fields.size() < 6)
            fields << QString();

        if (QTableWidget *historyWidget = findChild<QTableWidget *>(QStringLiteral("historyStockTable")))
        {
            const int row = historyWidget->rowCount();
            historyWidget->insertRow(row);
            for (int c = 0; c < 6; ++c)
                historyWidget->setItem(row, c, new QTableWidgetItem(fields.at(c)));
        }
    }
}

void MainWindow::appendStockHistoryEntry(const QString &action,
                                         int stockId,
                                         const QString &espece,
                                         int quantite,
                                         const QString &etat)
{
    const QString historyPath = stockHistoryFilePath();
    QFile historyFile(historyPath);
    const bool needHeader = !historyFile.exists() || historyFile.size() == 0;

    if (!historyFile.open(QIODevice::Append | QIODevice::Text))
    {
        QMessageBox::warning(this, tr("Historique"), tr("Impossible d'écrire dans le fichier d'historique."));
        return;
    }

    QTextStream stream(&historyFile);
    stream.setEncoding(QStringConverter::Utf8);

    if (needHeader)
        stream << "Date/Heure,Action,Stock ID,Espece,Quantite,Etat\n";

    const QString timestamp = QDateTime::currentDateTime().toString(QStringLiteral("yyyy-MM-dd HH:mm:ss"));
    const QStringList fields = {
        timestamp,
        action,
        QString::number(stockId),
        espece,
        QString::number(quantite),
        etat};

    QStringList escaped;
    escaped.reserve(fields.size());
    for (const QString &field : fields)
        escaped << escapeCsvField(field);
    stream << escaped.join(',') << '\n';

    stream.flush();
    historyFile.flush();
    historyFile.close();

    loadStockHistoryFromFile();
}

void MainWindow::on_btnNetHistoire_clicked()
{
    const QString historyPath = stockHistoryFilePath();
    QFile historyFile(historyPath);

    if (!historyFile.exists())
    {
        QMessageBox::information(this, tr("Historique"), tr("Aucun fichier history.csv à supprimer."));
        clearStockHistoryTable();
        return;
    }

    const QMessageBox::StandardButton choice = QMessageBox::question(
        this,
        tr("Nettoyer l'historique"),
        tr("Voulez-vous vraiment supprimer l'historique des actions stock ?"),
        QMessageBox::Yes | QMessageBox::No,
        QMessageBox::No);

    if (choice != QMessageBox::Yes)
        return;

    if (!historyFile.remove())
    {
        QMessageBox::warning(this, tr("Historique"), tr("Suppression impossible. Vérifiez les permissions du fichier."));
        return;
    }

    clearStockHistoryTable();
    QMessageBox::information(this, tr("Historique"), tr("Le fichier history.csv a été supprimé."));
}

void MainWindow::on_btnExpHistPdf_clicked()
{
    const QString csvPath = stockHistoryFilePath();
    QFile csvFile(csvPath);

    if (!csvFile.exists())
    {
        QMessageBox::information(this, tr("Export PDF"), tr("Aucun fichier history.csv trouvé à exporter."));
        return;
    }

    if (!csvFile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QMessageBox::warning(this, tr("Export PDF"), tr("Impossible de lire history.csv."));
        return;
    }

    QTextStream stream(&csvFile);
    stream.setEncoding(QStringConverter::Utf8);

    QStringList headerLabels = {
        QStringLiteral("Date/Heure"), QStringLiteral("Action"), QStringLiteral("Stock ID"),
        QStringLiteral("Espèce"), QStringLiteral("Quantité"), QStringLiteral("État")};
    QList<QStringList> rows;

    bool firstLine = true;
    while (!stream.atEnd())
    {
        const QString line = stream.readLine().trimmed();
        if (line.isEmpty())
            continue;

        if (firstLine)
        {
            QStringList csvHeader = parseCsvLine(line);
            if (csvHeader.size() >= 6)
                headerLabels = csvHeader.mid(0, 6);
            firstLine = false;
            continue;
        }

        QStringList fields = parseCsvLine(line);
        while (fields.size() < 6)
            fields << QString();
        rows.append(fields.mid(0, 6));
    }
    csvFile.close();

    if (rows.isEmpty())
    {
        QMessageBox::information(this, tr("Export PDF"), tr("history.csv ne contient aucune ligne d'historique à exporter."));
        return;
    }

    const QString defaultName = QFileInfo(csvPath).dir().filePath(
        QStringLiteral("history_export_%1.pdf").arg(QDateTime::currentDateTime().toString(QStringLiteral("yyyyMMdd_HHmmss"))));

    const QString outputPath = QFileDialog::getSaveFileName(
        this,
        tr("Exporter l'historique en PDF"),
        defaultName,
        tr("PDF Files (*.pdf)"));

    if (outputPath.isEmpty())
        return;

    QString html;
    html += "<html><head><meta charset='utf-8'>";
    html += "<style>";
    html += "body{font-family:'Segoe UI',sans-serif;color:#1f2933;font-size:10pt;}";
    html += "h1{font-size:18pt;margin:0 0 8px 0;color:#0f4c81;}";
    html += ".meta{margin:0 0 2px 0;color:#334e68;}";
    html += "table{width:100%;border-collapse:collapse;margin-top:12px;}";
    html += "th,td{border:1px solid #cbd5e1;padding:6px 8px;text-align:left;}";
    html += "th{background:#e6eef5;color:#102a43;font-weight:700;}";
    html += "tr:nth-child(even){background:#f8fafc;}";
    html += "</style></head><body>";

    html += QStringLiteral("<h1>Historique des operations Stock</h1>");
    html += QStringLiteral("<p class='meta'><b>Export effectue le:</b> %1</p>")
                .arg(QDateTime::currentDateTime().toString(QStringLiteral("yyyy-MM-dd HH:mm:ss")));
    html += QStringLiteral("<p class='meta'><b>Source:</b> %1</p>")
                .arg(QDir::toNativeSeparators(csvPath).toHtmlEscaped());
    html += QStringLiteral("<p class='meta'><b>Nombre de lignes:</b> %1</p>").arg(rows.size());

    html += "<table><thead><tr>";
    for (const QString &h : headerLabels)
        html += QStringLiteral("<th>%1</th>").arg(h.toHtmlEscaped());
    html += "</tr></thead><tbody>";

    for (const QStringList &row : rows)
    {
        html += "<tr>";
        for (int i = 0; i < 6; ++i)
            html += QStringLiteral("<td>%1</td>").arg(row.value(i).toHtmlEscaped());
        html += "</tr>";
    }
    html += "</tbody></table></body></html>";

    QPrinter printer(QPrinter::HighResolution);
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setOutputFileName(outputPath);
    printer.setPageSize(QPageSize(QPageSize::A4));
    printer.setPageMargins(QMarginsF(12, 12, 12, 12), QPageLayout::Millimeter);

    QTextDocument doc;
    doc.setHtml(html);
    doc.print(&printer);

    QMessageBox::information(this,
                             tr("Export PDF"),
                             tr("Historique exporte avec succes vers:\n%1")
                                 .arg(QDir::toNativeSeparators(outputPath)));
}

void MainWindow::on_btnStockAjouter_clicked()
{
    if (!validateStockInputs())
    {
        QMessageBox::warning(this, "Ajout", "Veuillez remplir tous les champs obligatoires.");
        return;
    }

    const int newStockId = ui->stockIdEdit->text().trimmed().toInt();
    if (stockIdExistsInDatabase(newStockId))
    {
        QMessageBox::warning(this,
                             "Ajout",
                             QString("Un stock avec l'ID %1 existe deja. Choisissez un autre ID.").arg(newStockId));
        return;
    }

    Stock s(
        newStockId,
        ui->stockEspeceEdit->text(),
        ui->stockQuantiteEdit->text().toInt(),
        ui->stockEtatCombo->currentText(),
        ui->stockSeuilMinEdit->text().toInt(),
        ui->stockSeuilMaxEdit->text().toInt(),
        ui->stockDateAjoutEdit->text(),
        ui->stockCIN->text().toInt(),
        ui->stockCMD->text().toInt(),
        ui->stockDateVente->text());

    if (s.ajouter())
    {
        appendStockHistoryEntry(QStringLiteral("ADD"),
                                ui->stockIdEdit->text().toInt(),
                                ui->stockEspeceEdit->text().trimmed(),
                                ui->stockQuantiteEdit->text().toInt(),
                                ui->stockEtatCombo->currentText().trimmed());
        refreshTable();
        QMessageBox::information(this, "Ajout", "Stock ajoute avec succes.");
    }
}

void MainWindow::on_btnSupprimer_Stock_clicked()
{
    QModelIndex index = ui->tableStockView->currentIndex();
    if (!index.isValid())
    {
        QMessageBox::warning(this, "Suppression", "Selectionnez une ligne a supprimer.");
        return;
    }

    int id = ui->tableStockView->model()->data(
                                            ui->tableStockView->model()->index(index.row(), 0))
                 .toInt();
    const QString espece = ui->tableStockView->model()->data(ui->tableStockView->model()->index(index.row(), 1)).toString();
    const int quantite = ui->tableStockView->model()->data(ui->tableStockView->model()->index(index.row(), 2)).toInt();
    const QString etat = ui->tableStockView->model()->data(ui->tableStockView->model()->index(index.row(), 3)).toString();

    QMessageBox::StandardButton confirmation = QMessageBox::question(
        this,
        "Confirmation de suppression",
        QString("Voulez-vous vraiment supprimer le stock ID %1 ?").arg(id),
        QMessageBox::Yes | QMessageBox::No,
        QMessageBox::No);

    if (confirmation != QMessageBox::Yes)
        return;

    Stock s;
    if (s.supprimer(id))
    {
        appendStockHistoryEntry(QStringLiteral("DELETE"), id, espece.trimmed(), quantite, etat.trimmed());
    }
    else
    {
        QMessageBox::critical(this, "Suppression", "Echec de la suppression du stock.");
    }

    refreshTable();
}

void MainWindow::on_btnStockClean_clicked()
{
    clearStockValidationErrors();

    ui->stockIdEdit->clear();
    ui->stockEspeceEdit->clear();
    ui->stockQuantiteEdit->clear();
    ui->stockSeuilMinEdit->clear();
    ui->stockSeuilMaxEdit->clear();
    ui->stockDateAjoutEdit->clear();
    ui->stockCIN->clear();
    ui->stockCMD->clear();
    ui->stockDateVente->clear();

    ui->stockEtatCombo->setCurrentIndex(-1);
    ui->stockIdEdit->setFocus();

    QMessageBox::information(this, "Nettoyage", "Champs nettoyés avec succes.");
}

void MainWindow::on_save_clean()
{
    clearStockValidationErrors();

    ui->stockIdEdit->clear();
    ui->stockEspeceEdit->clear();
    ui->stockQuantiteEdit->clear();
    ui->stockSeuilMinEdit->clear();
    ui->stockSeuilMaxEdit->clear();
    ui->stockDateAjoutEdit->clear();
    ui->stockCIN->clear();
    ui->stockCMD->clear();
    ui->stockDateVente->clear();

    ui->stockEtatCombo->setCurrentIndex(-1);
    ui->stockIdEdit->setFocus();
}

void MainWindow::on_btnRefresh_clicked()
{
    refreshTable();
}

void MainWindow::refreshTable()
{
    QString texteRecherche = ui->stockSearchEdit->text().trimmed();
    QString critereTri = ui->stockSortCombo->currentText();

    if (texteRecherche.isEmpty())
    {
        if (critereTri.isEmpty())
            ui->tableStockView->setModel(Stock::afficher());
        else
            ui->tableStockView->setModel(Stock::afficherTrie(critereTri));
    }
    else
    {
        ui->tableStockView->setModel(Stock::rechercherParEspece(texteRecherche, critereTri));
    }

    applyStockHeaders();
    updateStockGlobalChart();
    updateStockDistributionCharts();
    updateStockSpotlightCard();
    updateStockTrendChart();
}

void MainWindow::applyStockHeaders()
{
    QAbstractItemModel *tableModel = ui->tableStockView->model();
    if (!tableModel)
        return;

    tableModel->setHeaderData(0, Qt::Horizontal, "ID");
    tableModel->setHeaderData(1, Qt::Horizontal, "Espèce");
    tableModel->setHeaderData(2, Qt::Horizontal, "Quantité");
    tableModel->setHeaderData(3, Qt::Horizontal, "État");
    tableModel->setHeaderData(4, Qt::Horizontal, "Seuil Minimum");
    tableModel->setHeaderData(5, Qt::Horizontal, "Seuil Maximum");
    tableModel->setHeaderData(6, Qt::Horizontal, "Date d'ajout");
    tableModel->setHeaderData(7, Qt::Horizontal, "CIN");
    tableModel->setHeaderData(8, Qt::Horizontal, "N° Commande");
    tableModel->setHeaderData(9, Qt::Horizontal, "Date de vente");
}

void MainWindow::updateStockGlobalChart()
{
    QWidget *globalHost = findChild<QWidget *>(QStringLiteral("chartStockGlobal"));
    if (!globalHost)
        return;

    if (!m_stockGlobalChartView)
    {
        auto *layout = new QVBoxLayout(globalHost);
        layout->setContentsMargins(0, 0, 0, 0);
        layout->setSpacing(0);

        m_stockGlobalChartView = new QChartView(new QChart(), globalHost);
        m_stockGlobalChartView->setRenderHint(QPainter::Antialiasing);
        m_stockGlobalChartView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        layout->addWidget(m_stockGlobalChartView);
    }

    QLabel *centerLabel = globalHost->findChild<QLabel *>(QStringLiteral("stockGlobalCenterLabel"));
    if (!centerLabel)
    {
        centerLabel = new QLabel(globalHost);
        centerLabel->setObjectName(QStringLiteral("stockGlobalCenterLabel"));
        centerLabel->setAlignment(Qt::AlignCenter);
        centerLabel->setAttribute(Qt::WA_TransparentForMouseEvents);
        centerLabel->setStyleSheet("color: #7bed9f; font-size: 36px; font-weight: 700; background: transparent;");
    }
    centerLabel->setGeometry(globalHost->rect());
    centerLabel->raise();

    QSqlDatabase db = QSqlDatabase::database(Connection::connectionName());
    if (!db.isValid() || !db.isOpen())
        db = QSqlDatabase::database();

    int totalStock = 0;
    QSqlQuery q(db);
    if (q.exec("SELECT COUNT(STOCK_ID) FROM STOCK") && q.next())
        totalStock = q.value(0).toInt();

    // Baseline capacity for global stock progress; auto-expands if IDs exceed it.
    int totalCapacity = 50;
    QSqlQuery qMaxId(db);
    if (qMaxId.exec("SELECT NVL(MAX(STOCK_ID), 0) FROM STOCK") && qMaxId.next())
        totalCapacity = qMax(totalCapacity, qMaxId.value(0).toInt());

    QChart *chart = m_stockGlobalChartView->chart();
    chart->setTitle(tr("Stock Global"));
    chart->setTitleBrush(QBrush(Qt::white));
    chart->setBackgroundVisible(false);
    chart->setPlotAreaBackgroundVisible(false);
    chart->legend()->setVisible(false);
    chart->setAnimationOptions(QChart::SeriesAnimations);

    centerLabel->setText(QStringLiteral("%1/%2").arg(totalStock).arg(totalCapacity));

    const double targetProgress = totalCapacity > 0
                                      ? qBound(0.0, static_cast<double>(totalStock) / static_cast<double>(totalCapacity), 1.0)
                                      : 0.0;

    const double startProgress = m_stockGlobalDisplayedProgress;
    m_stockGlobalAnimToken += 1;
    const int token = m_stockGlobalAnimToken;

    auto renderProgress = [chart](double progress)
    {
        chart->removeAllSeries();

        auto *series = new QPieSeries(chart);
        series->setHoleSize(0.70);
        series->setPieSize(0.92);
        series->setPieStartAngle(90);
        series->setPieEndAngle(450);

        const double safeProgress = qBound(0.0, progress, 1.0);
        const double filled = safeProgress * 1000.0;
        const double remaining = 1000.0 - filled;

        QPieSlice *filledSlice = series->append(QString(), qMax(0.0, filled));
        filledSlice->setBrush(QColor(245, 245, 245));
        filledSlice->setPen(Qt::NoPen);

        QPieSlice *remainingSlice = series->append(QString(), qMax(0.0, remaining));
        remainingSlice->setBrush(QColor(55, 63, 79));
        remainingSlice->setPen(Qt::NoPen);

        chart->addSeries(series);
    };

    const int steps = 28;
    const int intervalMs = 35;
    for (int i = 0; i <= steps; ++i)
    {
        QTimer::singleShot(i * intervalMs, [this, token, i, steps, startProgress, targetProgress, renderProgress]()
                           {
            if (token != m_stockGlobalAnimToken)
                return;

            const double t = static_cast<double>(i) / static_cast<double>(steps);
            const double eased = 1.0 - std::pow(1.0 - t, 3.0);
            const double progress = startProgress + (targetProgress - startProgress) * eased;
            renderProgress(progress);

            if (i == steps)
                m_stockGlobalDisplayedProgress = targetProgress; });
    }
}

void MainWindow::updateStockDistributionCharts()
{
    QWidget *distributionHost = findChild<QWidget *>(QStringLiteral("chartStock"));
    if (!distributionHost)
        return;

    if (!m_stockEtatChartView || !m_stockEspeceChartView)
    {
        auto *layout = new QHBoxLayout(distributionHost);
        layout->setContentsMargins(0, 0, 0, 0);
        layout->setSpacing(12);

        m_stockEtatChartView = new QChartView(new QChart(), distributionHost);
        m_stockEspeceChartView = new QChartView(new QChart(), distributionHost);

        for (QChartView *view : {m_stockEtatChartView, m_stockEspeceChartView})
        {
            view->setRenderHint(QPainter::Antialiasing);
            view->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
            layout->addWidget(view, 1);
        }
    }

    QSqlDatabase db = QSqlDatabase::database(Connection::connectionName());
    if (!db.isValid() || !db.isOpen())
        db = QSqlDatabase::database();

    QMap<QString, int> etatCounts;
    QSqlQuery qEtat(db);
    if (qEtat.exec("SELECT NVL(ETAT, 'Non défini') AS ETAT_LIB, COUNT(*) FROM STOCK GROUP BY NVL(ETAT, 'Non défini')"))
    {
        while (qEtat.next())
            etatCounts[qEtat.value(0).toString().trimmed()] = qEtat.value(1).toInt();
    }

    QMap<QString, int> especeQuantites;
    QSqlQuery qEspece(db);
    if (qEspece.exec("SELECT NVL(ESPECE, 'Inconnue') AS ESPECE_LIB, NVL(SUM(QUANTITE), 0) FROM STOCK GROUP BY NVL(ESPECE, 'Inconnue')"))
    {
        while (qEspece.next())
            especeQuantites[qEspece.value(0).toString().trimmed()] = qEspece.value(1).toInt();
    }

    auto buildDonut = [](QChartView *view,
                         const QString &title,
                         const QMap<QString, int> &values,
                         const QList<QColor> &palette)
    {
        if (!view || !view->chart())
            return;

        QChart *chart = view->chart();
        chart->removeAllSeries();

        auto *series = new QPieSeries(chart);
        series->setHoleSize(0.38);

        int colorIndex = 0;
        int total = 0;
        for (auto it = values.cbegin(); it != values.cend(); ++it)
            total += it.value();

        if (values.isEmpty() || total == 0)
        {
            QPieSlice *slice = series->append(QObject::tr("Aucune donnée"), 1);
            slice->setBrush(QColor(120, 120, 120));
            slice->setLabelVisible(true);
            slice->setLabelColor(Qt::white);
        }
        else
        {
            for (auto it = values.cbegin(); it != values.cend(); ++it)
            {
                const int value = it.value();
                if (value <= 0)
                    continue;

                const int pct = qRound((100.0 * value) / total);
                QPieSlice *slice = series->append(QString("%1 - %2 (%3%)").arg(it.key()).arg(value).arg(pct), value);
                slice->setBrush(palette.at(colorIndex % palette.size()));
                slice->setLabelVisible(true);
                slice->setLabelColor(Qt::white);
                ++colorIndex;
            }
        }

        chart->addSeries(series);
        chart->setTitle(title);
        chart->setTitleBrush(QBrush(Qt::white));
        chart->setBackgroundVisible(false);
        chart->setPlotAreaBackgroundVisible(false);
        chart->legend()->setVisible(true);
        chart->legend()->setAlignment(Qt::AlignBottom);
        chart->legend()->setLabelColor(Qt::white);
        chart->setAnimationOptions(QChart::SeriesAnimations);
    };

    const QList<QColor> etatPalette = {
        QColor(46, 204, 113), QColor(52, 152, 219), QColor(241, 196, 15), QColor(231, 76, 60), QColor(155, 89, 182)};
    const QList<QColor> especePalette = {
        QColor(26, 188, 156), QColor(41, 128, 185), QColor(243, 156, 18), QColor(192, 57, 43), QColor(127, 140, 141), QColor(142, 68, 173)};

    buildDonut(m_stockEtatChartView, tr("Stock par etat"), etatCounts, etatPalette);
    buildDonut(m_stockEspeceChartView, tr("Quantite par espece"), especeQuantites, especePalette);
}

void MainWindow::updateStockSpotlightCard()
{
    QWidget *spotlightHost = findChild<QWidget *>(QStringLiteral("holoStock"));
    if (!spotlightHost)
        return;

    if (!m_stockSpotlightChartView)
    {
        if (QLayout *oldLayout = spotlightHost->layout())
        {
            QLayoutItem *item = nullptr;
            while ((item = oldLayout->takeAt(0)) != nullptr)
            {
                if (item->widget())
                    item->widget()->deleteLater();
                delete item;
            }
            delete oldLayout;
        }

        auto *hostLayout = new QVBoxLayout(spotlightHost);
        hostLayout->setContentsMargins(6, 6, 6, 6);
        hostLayout->setSpacing(0);

        m_stockSpotlightChartView = new QChartView(new QChart(), spotlightHost);
        m_stockSpotlightChartView->setRenderHint(QPainter::Antialiasing);
        m_stockSpotlightChartView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        m_stockSpotlightChartView->setStyleSheet("background: transparent;");
        hostLayout->addWidget(m_stockSpotlightChartView);
    }

    QSqlDatabase db = QSqlDatabase::database(Connection::connectionName());
    if (!db.isValid() || !db.isOpen())
        db = QSqlDatabase::database();

    QStringList categories;
    QList<int> values;

    QSqlQuery q(db);
    if (q.exec("SELECT NVL(ESPECE, 'Inconnue') AS ESPECE_LIB, COUNT(*) AS NB_ITEMS "
               "FROM STOCK "
               "GROUP BY NVL(ESPECE, 'Inconnue') "
               "ORDER BY NB_ITEMS DESC, ESPECE_LIB"))
    {
        while (q.next())
        {
            categories << q.value(0).toString().trimmed();
            values << q.value(1).toInt();
        }
    }

    QChart *chart = m_stockSpotlightChartView->chart();
    chart->removeAllSeries();
    const auto axes = chart->axes();
    for (QAbstractAxis *axis : axes)
        chart->removeAxis(axis);

    auto *series = new QBarSeries(chart);
    auto *set = new QBarSet(tr("Nombre d'items"), series);

    if (categories.isEmpty())
    {
        categories << tr("Aucune donnée");
        *set << 0;
    }
    else
    {
        for (int v : values)
            *set << v;
    }

    set->setColor(QColor(84, 196, 255));
    set->setBorderColor(QColor(132, 232, 255));

    series->append(set);
    series->setLabelsVisible(true);
    series->setLabelsFormat("@value");
    series->setLabelsPosition(QAbstractBarSeries::LabelsOutsideEnd);

    chart->addSeries(series);
    chart->setTitle(tr("Nombre d'espèces en stocks"));
    chart->setTitleBrush(QBrush(QColor(190, 245, 255)));
    chart->setBackgroundVisible(false);
    chart->setPlotAreaBackgroundVisible(false);
    chart->legend()->setVisible(false);
    chart->setAnimationOptions(QChart::SeriesAnimations);

    auto *axisX = new QBarCategoryAxis(chart);
    axisX->append(categories);
    axisX->setLabelsColor(QColor(220, 245, 255));
    axisX->setGridLineVisible(false);
    axisX->setLabelsAngle(-25);

    auto *axisY = new QValueAxis(chart);
    int maxValue = 0;
    for (int v : values)
        maxValue = qMax(maxValue, v);
    axisY->setRange(0, qMax(5, static_cast<int>(std::ceil(maxValue * 1.15))));
    axisY->setTickCount(6);
    axisY->setLabelFormat("%d");
    axisY->setLabelsColor(QColor(220, 245, 255));
    axisY->setTitleText(tr("Nombre d'items"));
    axisY->setTitleBrush(QBrush(QColor(220, 245, 255)));
    axisY->setGridLineColor(QColor(85, 130, 150));

    chart->addAxis(axisX, Qt::AlignBottom);
    chart->addAxis(axisY, Qt::AlignLeft);
    series->attachAxis(axisX);
    series->attachAxis(axisY);

    const QString subtitle = categories.size() >= 2
                                 ? tr("Top: %1 (%2 items) | Min: %3 (%4 items)")
                                       .arg(categories.first())
                                       .arg(values.first())
                                       .arg(categories.last())
                                       .arg(values.last())
                                 : tr("Top: %1 (%2 items)").arg(categories.first()).arg(values.isEmpty() ? 0 : values.first());
    chart->setTitle(chart->title() + "\n" + subtitle);
}

void MainWindow::updateStockTrendChart()
{
    QWidget *trendHost = findChild<QWidget *>(QStringLiteral("curveStock"));
    if (!trendHost)
        return;

    if (!m_stockTrendChartView)
    {
        if (QLayout *oldLayout = trendHost->layout())
        {
            QLayoutItem *item = nullptr;
            while ((item = oldLayout->takeAt(0)) != nullptr)
            {
                if (item->widget())
                    item->widget()->deleteLater();
                delete item;
            }
            delete oldLayout;
        }

        auto *layout = new QVBoxLayout(trendHost);
        layout->setContentsMargins(6, 6, 6, 6);
        layout->setSpacing(0);

        m_stockTrendChartView = new QChartView(new QChart(), trendHost);
        m_stockTrendChartView->setRenderHint(QPainter::Antialiasing);
        m_stockTrendChartView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        m_stockTrendChartView->setStyleSheet("background: transparent;");
        layout->addWidget(m_stockTrendChartView);
    }

    QSqlDatabase db = QSqlDatabase::database(Connection::connectionName());
    if (!db.isValid() || !db.isOpen())
        db = QSqlDatabase::database();

    QVector<QPair<QDate, int>> points;
    QSqlQuery q(db);
    if (q.exec("SELECT TRUNC(DATE_AJOUT) AS JOUR, COUNT(*) AS NB_AJOUTES "
               "FROM STOCK "
               "GROUP BY TRUNC(DATE_AJOUT) "
               "ORDER BY TRUNC(DATE_AJOUT)"))
    {
        while (q.next())
        {
            QDate day;
            const QVariant v = q.value(0);
            if (v.metaType().id() == QMetaType::QDate)
                day = v.toDate();
            else
                day = v.toDateTime().date();

            if (day.isValid())
                points.append(qMakePair(day, q.value(1).toInt()));
        }
    }

    if (points.isEmpty())
        points.append(qMakePair(QDate::currentDate(), 0));

    QChart *chart = m_stockTrendChartView->chart();
    chart->removeAllSeries();
    const auto axes = chart->axes();
    for (QAbstractAxis *axis : axes)
        chart->removeAxis(axis);

    auto *series = new QLineSeries(chart);
    series->setName(tr("Stock ajoutés"));
    QPen trendPen(QColor(111, 236, 255));
    trendPen.setWidth(3);
    series->setPen(trendPen);
    series->setPointsVisible(true);

    int maxY = 0;
    for (const auto &p : points)
    {
        const qint64 x = QDateTime(p.first, QTime(12, 0)).toMSecsSinceEpoch();
        series->append(static_cast<qreal>(x), p.second);
        maxY = qMax(maxY, p.second);
    }

    chart->addSeries(series);
    chart->setTitle(tr("Évolution des ajouts"));
    chart->setTitleBrush(QBrush(QColor(190, 245, 255)));
    chart->setBackgroundVisible(false);
    chart->setPlotAreaBackgroundVisible(false);
    chart->legend()->setVisible(false);
    chart->setAnimationOptions(QChart::SeriesAnimations);

    auto *axisX = new QDateTimeAxis(chart);
    axisX->setFormat("dd/MM");
    axisX->setTitleText(tr("Date d'ajout"));
    axisX->setLabelsColor(QColor(220, 245, 255));
    axisX->setTitleBrush(QBrush(QColor(220, 245, 255)));
    axisX->setGridLineColor(QColor(80, 120, 140));

    auto *axisY = new QValueAxis(chart);
    axisY->setRange(0, qMax(5, static_cast<int>(std::ceil(maxY * 1.15))));
    axisY->setTickCount(6);
    axisY->setLabelFormat("%d");
    axisY->setTitleText(tr("Nombre de stocks ajoutés"));
    axisY->setLabelsColor(QColor(220, 245, 255));
    axisY->setTitleBrush(QBrush(QColor(220, 245, 255)));
    axisY->setGridLineColor(QColor(80, 120, 140));

    chart->addAxis(axisX, Qt::AlignBottom);
    chart->addAxis(axisY, Qt::AlignLeft);
    series->attachAxis(axisX);
    series->attachAxis(axisY);
}

void MainWindow::on_stockSortCombo_currentTextChanged(const QString &arg1)
{
    Q_UNUSED(arg1);
    refreshTable();
}

void MainWindow::on_stockSearchEdit_textChanged(const QString &arg1)
{
    Q_UNUSED(arg1);
    refreshTable();
}

void MainWindow::on_btnStockModifier_clicked()
{
    QModelIndex index = ui->tableStockView->currentIndex();
    if (!index.isValid())
    {
        QMessageBox::warning(this, "Modification", "Selectionnez une ligne du tableau a modifier.");
        return;
    }

    int row = index.row();
    QAbstractItemModel *tableModel = ui->tableStockView->model();

    ui->stockIdEdit->setText(tableModel->data(tableModel->index(row, 0)).toString());
    ui->stockEspeceEdit->setText(tableModel->data(tableModel->index(row, 1)).toString());
    ui->stockQuantiteEdit->setText(tableModel->data(tableModel->index(row, 2)).toString());
    ui->stockEtatCombo->setCurrentText(tableModel->data(tableModel->index(row, 3)).toString());
    ui->stockSeuilMinEdit->setText(tableModel->data(tableModel->index(row, 4)).toString());
    ui->stockSeuilMaxEdit->setText(tableModel->data(tableModel->index(row, 5)).toString());
    ui->stockDateAjoutEdit->setText(dateOnlyText(tableModel->data(tableModel->index(row, 6))));
    ui->stockCIN->setText(tableModel->data(tableModel->index(row, 7)).toString());
    ui->stockCMD->setText(tableModel->data(tableModel->index(row, 8)).toString());
    ui->stockDateVente->setText(dateOnlyText(tableModel->data(tableModel->index(row, 9))));

    ui->btnStockModifier->setVisible(false);
    ui->btnStockAjouter->setVisible(false);
    ui->btnStockSave->setVisible(true);
    ui->btnStockAnnuler->setVisible(true);
}

void MainWindow::on_btnStockSave_clicked()
{
    if (!validateStockInputs())
        return;

    QString idText = ui->stockIdEdit->text().trimmed();
    if (idText.isEmpty())
    {
        QMessageBox::warning(this, "Modification", "ID introuvable pour la mise a jour.");
        return;
    }

    int id = idText.toInt();

    Stock s(
        id,
        ui->stockEspeceEdit->text(),
        ui->stockQuantiteEdit->text().toInt(),
        ui->stockEtatCombo->currentText(),
        ui->stockSeuilMinEdit->text().toInt(),
        ui->stockSeuilMaxEdit->text().toInt(),
        ui->stockDateAjoutEdit->text(),
        ui->stockCIN->text().toInt(),
        ui->stockCMD->text().toInt(),
        ui->stockDateVente->text());

    if (s.modifier())
    {
        appendStockHistoryEntry(QStringLiteral("MODIFY"),
                                id,
                                ui->stockEspeceEdit->text().trimmed(),
                                ui->stockQuantiteEdit->text().toInt(),
                                ui->stockEtatCombo->currentText().trimmed());
        refreshTable();
        QMessageBox::information(this, "Modification", "Stock modifie avec succes.");
        on_save_clean();
    }
    else
    {
        QMessageBox::critical(this, "Modification", "Echec de la modification du stock.");
        return;
    }

    ui->btnStockModifier->setVisible(true);
    ui->btnStockAjouter->setVisible(true);
    ui->btnStockSave->setVisible(false);
    ui->btnStockAnnuler->setVisible(false);
}

void MainWindow::on_btnStockAnnuler_clicked()
{
    on_save_clean();
    ui->btnStockModifier->setVisible(true);
    ui->btnStockAjouter->setVisible(true);
    ui->btnStockSave->setVisible(false);
    ui->btnStockAnnuler->setVisible(false);
}

QLabel *MainWindow::ensureErrorLabel(QWidget *field)
{
    if (!field)
        return nullptr;

    QString errorName = field->objectName() + "_errorLabel";
    QLabel *errorLabel = field->parentWidget()->findChild<QLabel *>(errorName);

    if (!errorLabel)
    {
        errorLabel = new QLabel(field->parentWidget());
        errorLabel->setObjectName(errorName);
        errorLabel->setStyleSheet("color: #ff4d4f; font-size: 10px; background: transparent; border: none;");
    }

    QRect fieldGeometry = field->geometry();
    errorLabel->setGeometry(fieldGeometry.x(), fieldGeometry.y() + fieldGeometry.height() + 2,
                            fieldGeometry.width(), 14);
    return errorLabel;
}

void MainWindow::setFieldError(QWidget *field, const QString &message)
{
    QLabel *errorLabel = ensureErrorLabel(field);
    if (!errorLabel)
        return;

    errorLabel->setText(message);
    errorLabel->setVisible(true);
}

void MainWindow::clearFieldError(QWidget *field)
{
    if (!field)
        return;

    QString errorName = field->objectName() + "_errorLabel";
    QLabel *errorLabel = field->parentWidget()->findChild<QLabel *>(errorName);
    if (errorLabel)
        errorLabel->setVisible(false);
}

void MainWindow::clearStockValidationErrors()
{
    clearFieldError(ui->stockIdEdit);
    clearFieldError(ui->stockEspeceEdit);
    clearFieldError(ui->stockQuantiteEdit);
    clearFieldError(ui->stockEtatCombo);
    clearFieldError(ui->stockSeuilMinEdit);
    clearFieldError(ui->stockSeuilMaxEdit);
    clearFieldError(ui->stockDateAjoutEdit);
    clearFieldError(ui->stockCIN);
    clearFieldError(ui->stockCMD);
    clearFieldError(ui->stockDateVente);
}

bool MainWindow::validateStockInputs()
{
    clearStockValidationErrors();
    bool isValid = true;

    if (ui->stockIdEdit->text().trimmed().isEmpty())
    {
        setFieldError(ui->stockIdEdit, "Champ obligatoire");
        isValid = false;
    }

    if (ui->stockEspeceEdit->text().trimmed().isEmpty())
    {
        setFieldError(ui->stockEspeceEdit, "Champ obligatoire");
        isValid = false;
    }

    QString quantiteText = ui->stockQuantiteEdit->text().trimmed();
    QString seuilMinText = ui->stockSeuilMinEdit->text().trimmed();
    QString seuilMaxText = ui->stockSeuilMaxEdit->text().trimmed();
    QString cinText = ui->stockCIN->text().trimmed();

    if (quantiteText.isEmpty())
    {
        setFieldError(ui->stockQuantiteEdit, "Champ obligatoire");
        isValid = false;
    }

    if (ui->stockEtatCombo->currentText().trimmed().isEmpty())
    {
        setFieldError(ui->stockEtatCombo, "Champ obligatoire");
        isValid = false;
    }

    if (seuilMinText.isEmpty())
    {
        setFieldError(ui->stockSeuilMinEdit, "Champ obligatoire");
        isValid = false;
    }

    if (seuilMaxText.isEmpty())
    {
        setFieldError(ui->stockSeuilMaxEdit, "Champ obligatoire");
        isValid = false;
    }

    if (ui->stockDateAjoutEdit->text().trimmed().isEmpty())
    {
        setFieldError(ui->stockDateAjoutEdit, "Champ obligatoire");
        isValid = false;
    }

    if (cinText.isEmpty())
    {
        setFieldError(ui->stockCIN, "Champ obligatoire");
        isValid = false;
    }
    else
    {
        QRegularExpression cinRegex("^\\d{8}$");
        if (!cinRegex.match(cinText).hasMatch())
        {
            setFieldError(ui->stockCIN, "CIN doit contenir exactement 8 chiffres");
            isValid = false;
        }
    }

    if (ui->stockCMD->text().trimmed().isEmpty())
    {
        setFieldError(ui->stockCMD, "Champ obligatoire");
        isValid = false;
    }

    if (ui->stockDateVente->text().trimmed().isEmpty())
    {
        setFieldError(ui->stockDateVente, "Champ obligatoire");
        isValid = false;
    }

    bool quantiteOk = false;
    bool seuilMinOk = false;
    bool seuilMaxOk = false;
    int quantite = quantiteText.toInt(&quantiteOk);
    int seuilMin = seuilMinText.toInt(&seuilMinOk);
    int seuilMax = seuilMaxText.toInt(&seuilMaxOk);

    if (!quantiteText.isEmpty() && !quantiteOk)
    {
        setFieldError(ui->stockQuantiteEdit, "Valeur numerique invalide");
        isValid = false;
    }

    if (!seuilMinText.isEmpty() && !seuilMinOk)
    {
        setFieldError(ui->stockSeuilMinEdit, "Valeur numerique invalide");
        isValid = false;
    }

    if (!seuilMaxText.isEmpty() && !seuilMaxOk)
    {
        setFieldError(ui->stockSeuilMaxEdit, "Valeur numerique invalide");
        isValid = false;
    }

    if (quantiteOk && seuilMinOk && seuilMaxOk)
    {
        if (quantite < seuilMin || quantite > seuilMax)
        {
            setFieldError(ui->stockQuantiteEdit, "Quantite invalide !");
            setFieldError(ui->stockSeuilMinEdit, "Quantite doit etre supérieure !");
            setFieldError(ui->stockSeuilMaxEdit, "Quantite doit etre inférieure !");
            isValid = false;
        }
    }

    return isValid;
}
