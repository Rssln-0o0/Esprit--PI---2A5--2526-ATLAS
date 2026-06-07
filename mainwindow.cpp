#include "libraries.h"
#include <QAbstractItemView>
#include <QMouseEvent>
#include <QTextEdit>
#include <QPlainTextEdit>
#include <QComboBox>
#include <QTabWidget>
#include <QTextDocument>
#include <QTextCursor>
#include <QFile>
#include <algorithm>
#include <QFrame>
#include <QDesktopServices>
#include <QStandardPaths>
#include <QDir>
#include <QProcess>
#include <QProcessEnvironment>
#include <QFileInfo>

#include <QUrl>
#include <QDialog>
#include <QTextBrowser>
#include <QDialogButtonBox>
#include <QQuickWidget>
#include <QQuickItem>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsEllipseItem>
#include <QGraphicsTextItem>
#include <QtCharts/QBarSet>
#include <QtCharts/QBarSeries>
#include <QToolTip>
#include <QtCharts/QBarCategoryAxis>
#include <QtCharts/QHorizontalBarSeries>
#include <QtCharts/QAbstractAxis>
#include <QtCharts/QValueAxis>
#include "mainwindow.h"
#include "quai_arduino_window.h"
#include <QAction>
#include <QKeySequence>
#include <QSignalBlocker>
#include "ui_mainwindow.h"
#include "bateau_module.h"
#include "connection.h"
#include "employee.h"
#include "stock.h"
#include <QSqlRecord>
#include <QDateTime>
#include <QTimeZone>
#include <QTime>
#include <QTimer>
#include <QApplication>
#include <QEventLoop>
#include <QMenu>
#include <QToolButton>
#include <QBrush>
#include <QSizePolicy>
#include <QScrollArea>
#include <QGridLayout>
#include <QGraphicsDropShadowEffect>
#include <QVariantAnimation>
#include <QTextToSpeech>
#include <QLocale>
#include <QMap>
#include <QFont>
#include <QMargins>
#include <cmath>

static void applyPieChart(QChartView *view, const QList<QPair<QString, int>> &counts, const QList<QColor> &colors, Qt::Alignment legendAlign);
static const QList<QColor> kStatsPaletteDept = { QColor("#2196F3"), QColor("#4CAF50"), QColor("#FFEB3B"), QColor("#FF9800"), QColor("#F44336"), QColor("#9C27B0"), QColor("#607D8B") };
static const QList<QColor> kStatsPaletteShift = { QColor("#FFEB3B"), QColor("#2196F3"), QColor("#4CAF50"), QColor("#FF9800"), QColor("#9C27B0") };
static bool ensureEquipPhotoTable(QSqlDatabase db, QString *error);
static bool saveEquipPhotoPath(int equipId, const QString &photoPath, QString *error);
static QString loadEquipPhotoPath(int equipId, QString *error);
static int nextEquipementId();
static QString persistEquipPhotoFile(int equipId, const QString &sourcePath, QString *error);
static void showEquipPhotoPreview(QLabel *preview, const QString &photoPath);

static QString psSingleQuoteEscape(QString s)
{
    return s.replace('\'', QStringLiteral("''"));
}

static QString formatEmployeeDbError(QString raw)
{
    const QString t = raw.trimmed();
    if (t.isEmpty())
        return QObject::tr("Erreur base de données inconnue.");

    // Keep a readable technical line only (drop noisy/garbled driver tails).
    QString tech = t;
    const QStringList lines = t.split(QRegularExpression(QStringLiteral("[\r\n]+")), Qt::SkipEmptyParts);
    for (const QString &ln : lines) {
        if (ln.contains(QStringLiteral("ORA-"), Qt::CaseInsensitive) ||
            ln.contains(QStringLiteral("QODBC"), Qt::CaseInsensitive)) {
            tech = ln.trimmed();
            break;
        }
    }

    if (tech.contains(QStringLiteral("ORA-00001"), Qt::CaseInsensitive))
        return QObject::tr(
            "Valeur déjà existante (contrainte d'unicité).\n"
            "Vérifiez qu'aucun autre employé n'utilise déjà la même valeur pour :\n"
            "• CIN (identifiant principal)\n"
            "• Adresse e-mail\n"
            "• Numéro de téléphone\n"
            "• Badge RFID\n"
            "• Code d'accès (8 chiffres)\n"
            "• Mot de passe");
    if (tech.contains(QStringLiteral("ORA-00904"), Qt::CaseInsensitive))
        return QObject::tr("Colonne invalide dans la requête SQL.");
    if (tech.contains(QStringLiteral("ORA-01400"), Qt::CaseInsensitive))
        return QObject::tr("Un champ obligatoire est vide (NULL interdit).");
    if (tech.contains(QStringLiteral("ORA-02291"), Qt::CaseInsensitive) ||
        tech.contains(QStringLiteral("ORA-02292"), Qt::CaseInsensitive))
        return QObject::tr("Contrainte de clé étrangère violée.");

    return QObject::tr("Erreur base de données.");
}

static QByteArray parseRfidSerialLineToUid(const QByteArray &rawLine)
{
    QString s = QString::fromUtf8(rawLine).trimmed();
    s.remove(QLatin1Char(' '));
    if (s.size() < 8)
        return {};
    if (s.left(4).compare(QStringLiteral("UID:"), Qt::CaseInsensitive) != 0)
        return {};
    const QString tail = s.mid(4);
    if (tail.size() < 4 || tail.size() > 32)
        return {};
    for (QChar c : tail) {
        const ushort u = c.unicode();
        const bool hex = (u >= '0' && u <= '9') || (u >= 'A' && u <= 'F') || (u >= 'a' && u <= 'f');
        if (!hex)
            return {};
    }
    return tail.toUpper().toUtf8();
}

/** Sketch : ligne « CODE:xxxxxxxx » (exactement 8 chiffres). Même recherche base que RFID (colonne code). */
static QByteArray parseKeypadSerialLineToCode(const QByteArray &rawLine)
{
    if (rawLine.size() < 6)
        return {};
    if (rawLine.left(5).toUpper() != QByteArrayLiteral("CODE:"))
        return {};
    QByteArray tail = rawLine.mid(5).trimmed();
    QByteArray digits;
    digits.reserve(tail.size());
    for (char c : tail) {
        if (c >= '0' && c <= '9')
            digits.append(c);
    }
    if (digits.size() != 8)
        return {};
    return digits;
}

static QString findNearbyScript(const QString &fileName)
{
    const QString appDir = QCoreApplication::applicationDirPath();
    const QString curDir = QDir::currentPath();
    QStringList roots;
    roots << appDir << curDir;
    QSet<QString> seen;
    QStringList dirsToScan;
    for (const QString &root : roots) {
        QString d = root;
        for (int up = 0; up < 10; ++up) {
            const QString canon = QDir(d).canonicalPath();
            const QString normalized = canon.isEmpty() ? QDir::cleanPath(d) : canon;
            if (!normalized.isEmpty() && !seen.contains(normalized)) {
                seen.insert(normalized);
                dirsToScan << normalized;
            }
            d = QDir(d).filePath("..");
        }
    }
    for (const QString &dir : dirsToScan) {
        const QString candidate = QDir(dir).filePath(fileName);
        if (QFileInfo::exists(candidate)) {
            const QString canon = QFileInfo(candidate).canonicalFilePath();
            return canon.isEmpty() ? QDir::cleanPath(candidate) : canon;
        }
    }
    return QString();
}

static QString ensureQrGeneratorScriptPath()
{
    QString p = findNearbyScript(QStringLiteral("qr_generator.py"));
    if (!p.isEmpty()) return p;
    const QString dir = QDir(QStandardPaths::writableLocation(QStandardPaths::TempLocation)).filePath(QStringLiteral("ATLAS_QR"));
    if (!QDir().mkpath(dir)) return QString();
    const QString dest = QDir(dir).filePath(QStringLiteral("qr_generator.py"));
    QFile existing(dest);
    if (existing.exists() && existing.size() > 0) return QDir::cleanPath(dest);
    static const char kQrGen[] =
        "import sys\nimport qrcode\n\ndef generate_qr(data, output_path):\n"
        "    qr = qrcode.QRCode(version=1, error_correction=qrcode.constants.ERROR_CORRECT_L, box_size=10, border=4)\n"
        "    qr.add_data(data)\nqr.make(fit=True)\n"
        "    img = qr.make_image(fill_color=\"black\", back_color=\"white\")\nimg.save(output_path)\n\n"
        "if __name__ == \"__main__\":\n    if len(sys.argv) < 3:\n"
        "        print(\"Usage: python qr_generator.py <data> <output_path>\")\n        sys.exit(1)\n"
        "    data = sys.argv[1]\n    output_path = sys.argv[2]\n"
        "    try:\n        generate_qr(data, output_path)\n        sys.exit(0)\n"
        "    except Exception as e:\n        print(f\"Error: {e}\")\n        sys.exit(1)\n";
    QFile out(dest);
    if (!out.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) return QString();
    out.write(kQrGen); out.close();
    return QDir::cleanPath(dest);
}

static QString findPythonExecutable()
{
    QString pythonExe = QStandardPaths::findExecutable("python");
    if (!pythonExe.isEmpty()) return pythonExe;
    pythonExe = QStandardPaths::findExecutable("python3");
    if (!pythonExe.isEmpty()) return pythonExe;
    return QString();
}

static QProcessEnvironment sanitizedPythonEnvironment()
{
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    env.remove(QStringLiteral("PYTHONHOME"));
    env.remove(QStringLiteral("PYTHONPATH"));
    return env;
}


// ==================== HELPERS RECONNAISSANCE FACIALE ====================
static bool isFaceDetectionScriptAvailable()
{
    const QStringList searchDirs = { QCoreApplication::applicationDirPath(), QDir::currentPath() };
    for (const QString &dir : searchDirs) {
        if (QFileInfo::exists(QDir(dir).absoluteFilePath(QStringLiteral("face_detector.py"))))
            return true;
    }
    return false;
}

static bool detectFaceInImageWithScript(const QImage &img)
{
    if (img.isNull()) return false;
    const QStringList searchDirs = { QCoreApplication::applicationDirPath(), QDir::currentPath() };
    QString scriptPath;
    for (const QString &dir : searchDirs) {
        const QString p = QDir(dir).absoluteFilePath(QStringLiteral("face_detector.py"));
        if (QFileInfo::exists(p)) { scriptPath = p; break; }
    }
    if (scriptPath.isEmpty()) return false;
    const QString tempPath = QDir::temp().filePath(QStringLiteral("atlas_face_check_%1.jpg").arg(QDateTime::currentMSecsSinceEpoch()));
    if (!img.save(tempPath, "JPG", 90)) { QFile::remove(tempPath); return false; }
    const QList<QStringList> pythonTry = {
        { QStringLiteral("python"), scriptPath, tempPath },
        { QStringLiteral("python3"), scriptPath, tempPath },
        { QStringLiteral("py"), scriptPath, tempPath },
        { QStringLiteral("py"), QStringLiteral("-3"), scriptPath, tempPath },
    };
    for (const QStringList &args : pythonTry) {
        if (args.size() < 3) continue;
        const int exitCode = QProcess::execute(args.first(), args.mid(1));
        if (exitCode == 0) { QFile::remove(tempPath); return true; }
        if (exitCode == 1) { QFile::remove(tempPath); return false; }
    }
    QFile::remove(tempPath);
    return false;
}

static QImage centerCropSquare(const QImage &img)
{
    if (img.isNull()) return {};
    const int s = qMin(img.width(), img.height());
    return img.copy((img.width() - s) / 2, (img.height() - s) / 2, s, s);
}

static QImage centerCropSquareAtScale(const QImage &img, double scale)
{
    if (img.isNull() || scale <= 0.0) return {};
    const int w = img.width(), h = img.height();
    const int newW = qMax(16, int(std::round(w * scale)));
    const int newH = qMax(16, int(std::round(h * scale)));
    QImage scaled = img.scaled(newW, newH, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    return centerCropSquare(scaled);
}

static QImage equalizeGrayscale(const QImage &gray8)
{
    if (gray8.isNull() || gray8.format() != QImage::Format_Grayscale8) return gray8;
    int hist[256] = {0};
    const int w = gray8.width(), h = gray8.height();
    for (int y = 0; y < h; ++y) {
        const uchar *row = gray8.constScanLine(y);
        for (int x = 0; x < w; ++x) ++hist[row[x]];
    }
    int cdf[256] = {0};
    cdf[0] = hist[0];
    for (int i = 1; i < 256; ++i) cdf[i] = cdf[i - 1] + hist[i];
    int cdfMin = 0;
    for (int i = 0; i < 256; ++i) { if (cdf[i] > 0) { cdfMin = cdf[i]; break; } }
    const int total = w * h;
    if (total <= 0 || cdfMin <= 0) return gray8;
    uchar lut[256];
    for (int i = 0; i < 256; ++i) {
        const double v = double(cdf[i] - cdfMin) / double(total - cdfMin);
        lut[i] = uchar(qBound(0, int(std::round(v * 255.0)), 255));
    }
    QImage out(gray8.size(), QImage::Format_Grayscale8);
    for (int y = 0; y < h; ++y) {
        const uchar *src = gray8.constScanLine(y);
        uchar *dst = out.scanLine(y);
        for (int x = 0; x < w; ++x) dst[x] = lut[src[x]];
    }
    return out;
}

static const int *uniformLbpMap256()
{
    static int map[256];
    static bool inited = false;
    if (inited) return map;
    int idx = 0;
    for (int code = 0; code < 256; ++code) {
        int transitions = 0, prev = (code >> 7) & 1;
        for (int b = 0; b < 8; ++b) {
            const int cur = (code >> b) & 1;
            if (cur != prev) ++transitions;
            prev = cur;
        }
        map[code] = (transitions <= 2) ? idx++ : 58;
    }
    for (int code = 0; code < 256; ++code)
        if (map[code] > 58) map[code] = 58;
    inited = true;
    return map;
}

static QVector<float> lbpGridDescriptor(const QImage &img, int sizePx = 128, int grid = 8)
{
    if (img.isNull() || sizePx < 32 || grid < 1) return {};
    QImage g = centerCropSquare(img).convertToFormat(QImage::Format_Grayscale8)
        .scaled(sizePx, sizePx, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    g = equalizeGrayscale(g);
    if (g.isNull()) return {};
    const int bins = 59;
    const int *uMap = uniformLbpMap256();
    QVector<float> hist(grid * grid * bins, 0.0f);
    const int cell = qMax(1, sizePx / grid);
    for (int y = 1; y < sizePx - 1; ++y) {
        const uchar *rowPrev = g.constScanLine(y - 1);
        const uchar *row = g.constScanLine(y);
        const uchar *rowNext = g.constScanLine(y + 1);
        for (int x = 1; x < sizePx - 1; ++x) {
            const int c = row[x];
            int code = 0;
            code |= (rowPrev[x - 1] >= c) << 7;
            code |= (rowPrev[x] >= c) << 6;
            code |= (rowPrev[x + 1] >= c) << 5;
            code |= (row[x + 1] >= c) << 4;
            code |= (rowNext[x + 1] >= c) << 3;
            code |= (rowNext[x] >= c) << 2;
            code |= (rowNext[x - 1] >= c) << 1;
            code |= (row[x - 1] >= c) << 0;
            const int gx = qMin(grid - 1, x / cell);
            const int gy = qMin(grid - 1, y / cell);
            hist[(gy * grid + gx) * bins + uMap[code]] += 1.0f;
        }
    }
    for (int gy = 0; gy < grid; ++gy) {
        for (int gx = 0; gx < grid; ++gx) {
            const int base = (gy * grid + gx) * bins;
            float sum = 0.0f;
            for (int b = 0; b < bins; ++b) sum += hist[base + b];
            if (sum <= 0.0f) continue;
            for (int b = 0; b < bins; ++b) hist[base + b] /= sum;
        }
    }
    return hist;
}

static QVector<float> lbpGridDescriptorMultiScale(const QImage &img, int grid = 8)
{
    const QList<int> sizes = { 96, 128, 160 };
    QVector<float> out;
    for (int sizePx : sizes) {
        QVector<float> h = lbpGridDescriptor(img, sizePx, grid);
        if (h.isEmpty()) return {};
        out.append(h);
    }
    return out;
}

namespace FaceDescriptorCache {
    static QHash<QString, QVector<QVector<float>>> cache;
}

static double chiSquareDistance(const QVector<float> &a, const QVector<float> &b)
{
    if (a.size() != b.size() || a.isEmpty()) return std::numeric_limits<double>::infinity();
    const double eps = 1e-12;
    double s = 0.0;
    for (int i = 0; i < a.size(); ++i) {
        const double d = a[i] - b[i];
        s += (d * d) / (a[i] + b[i] + eps);
    }
    return 0.5 * s;
}

static void hidePhotoPreview(QLabel *label);
static void showPhotoPreview(QLabel *label);

static void setPhotoPlaceholder(QLabel *label)
{
    if (!label) return;
    label->setProperty("photoPath", QString());
    label->setProperty("photoPrivacyHidden", false);
    label->setProperty("cachedPhotoPixmap", QVariant());
    label->setPixmap(QPixmap());
    label->setText("Photo Placeholder");
    label->setAlignment(Qt::AlignCenter);
    label->setScaledContents(false);
    label->setStyleSheet("border: 2px dashed #3498db; color: #7f8c8d;");
}

static bool setPhotoFromPath(QLabel *label, const QString &path)
{
    if (!label || path.trimmed().isEmpty()) return false;
    QPixmap pix(path);
    if (pix.isNull()) return false;
    label->setProperty("photoPath", path);
    label->setProperty("photoPrivacyHidden", false);
    label->setProperty("cachedPhotoPixmap", QVariant());
    qreal dpr = qMax(qreal(1.0), label->devicePixelRatioF());
    const int w = qMax(120, int(label->width() * dpr));
    const int h = qMax(120, int(label->height() * dpr));
    QPixmap result;
    if (pix.width() <= w && pix.height() <= h) {
        result = pix;
    } else {
        QPixmap scaled = pix.scaled(w, h, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
        const int x = qMax(0, (scaled.width() - w) / 2);
        const int y = qMax(0, (scaled.height() - h) / 2);
        result = scaled.copy(x, y, w, h);
    }
    result.setDevicePixelRatio(dpr);
    label->setText(QString());
    label->setAlignment(Qt::AlignCenter);
    label->setStyleSheet("border: 2px solid #2ecc71; border-radius: 6px;");
    label->setScaledContents(false);
    label->setPixmap(result);
    return true;
}

static bool setPhotoFromImage(QLabel *label, const QImage &img)
{
    if (!label || img.isNull()) return false;
    QPixmap pix = QPixmap::fromImage(img);
    if (pix.isNull()) return false;
    label->setProperty("photoPath", QString());
    label->setProperty("photoPrivacyHidden", false);
    label->setProperty("cachedPhotoPixmap", QVariant());
    qreal dpr = qMax(qreal(1.0), label->devicePixelRatioF());
    const int w = qMax(120, int(label->width() * dpr));
    const int h = qMax(120, int(label->height() * dpr));
    QPixmap result;
    if (pix.width() <= w && pix.height() <= h) {
        result = pix;
    } else {
        QPixmap scaled = pix.scaled(w, h, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
        const int x = qMax(0, (scaled.width() - w) / 2);
        const int y = qMax(0, (scaled.height() - h) / 2);
        result = scaled.copy(x, y, w, h);
    }
    result.setDevicePixelRatio(dpr);
    label->setText(QString());
    label->setAlignment(Qt::AlignCenter);
    label->setStyleSheet("border: 2px solid #2ecc71; border-radius: 6px;");
    label->setScaledContents(false);
    label->setPixmap(result);
    return true;
}

static void hidePhotoPreview(QLabel *label)
{
    if (!label) return;
    const QString path = label->property("photoPath").toString().trimmed();
    const QPixmap pix = label->pixmap(Qt::ReturnByValue);
    if (path.isEmpty() && pix.isNull())
        return;
    if (!pix.isNull())
        label->setProperty("cachedPhotoPixmap", QVariant::fromValue(pix));
    label->setPixmap(QPixmap());
    label->setText(QStringLiteral("Photo masquee\n\"Afficher la photo\""));
    label->setAlignment(Qt::AlignCenter);
    label->setStyleSheet(QStringLiteral("border: 2px dashed #95a5a6; color: #bdc3c7; padding: 8px;"));
    label->setProperty("photoPrivacyHidden", true);
    if (auto *mw = qobject_cast<MainWindow *>(label->window()))
        mw->refreshPhotoPrivacyControls();
}

static void showPhotoPreview(QLabel *label)
{
    if (!label || !label->property("photoPrivacyHidden").toBool())
        return;
    const QString path = label->property("photoPath").toString().trimmed();
    if (!path.isEmpty()) {
        setPhotoFromPath(label, path);
        return;
    }
    const QVariant v = label->property("cachedPhotoPixmap");
    if (!v.isValid() || !v.canConvert<QPixmap>())
        return;
    setPhotoFromImage(label, v.value<QPixmap>().toImage());
}

// ==================== CONSTRUCTEUR / DESTRUCTEUR ====================
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // Full Registry wipe on boot to prevent "Access Denied" or cross-wiring of COM ports
    QSettings settings(QStringLiteral("ATLAS"), QStringLiteral("ATLAS"));
    settings.remove(QStringLiteral("arduinorfid_employe/portName"));
    settings.remove(QStringLiteral("arduinorfid_pecheur/portName"));
    settings.remove(QStringLiteral("arduino_quai_gate/portName"));
    settings.remove(QStringLiteral("arduinorfid/portName"));
    settings.remove(QStringLiteral("smartport/portName"));
    settings.remove(QStringLiteral("esp32cam/portName"));
    settings.remove(QStringLiteral("StockBalancePort/portName"));

    m_equipServer = new EquipHttpServer(this);
    if (m_equipServer->start()) {
        qDebug() << "EquipLog Server started on" << m_equipServer->lanIp() << ":" << m_equipServer->port();
    } else {
        qDebug() << "EquipLog Server FAILED to start.";
    }
    qRegisterMetaType<SmartPortRealtimeState>("SmartPortRealtimeState");
    if (ui->lblSmartPortBanner)
        m_smartPortBannerDefaultText = ui->lblSmartPortBanner->text();
    m_mainWindowBaseStyle = this->styleSheet();
    m_stackedWidgetBaseStyle = ui->stackedWidget ? ui->stackedWidget->styleSheet() : QString();
    setupResponsiveStockLayouts();
    m_bateauModule = new BateauModule(ui, &Btmp, this, nullptr);
    m_bateauModule->initAfterSetupUi();
    setupResponsiveModuleLayouts();

    // Show login page first (index 1 = welcomePage); dashboard is index 0
    if (ui->mainStackedWidget)
        ui->mainStackedWidget->setCurrentIndex(1);

    if (ui->lineEdit_5)
        ui->lineEdit_5->setText(EmployeeCrud::defaultLoginEmail());
    if (ui->lineEdit_6)
        ui->lineEdit_6->setText(EmployeeCrud::defaultLoginPassword());

    applyTheme(m_darkMode);

    auto *actOpenQuaiArduino = new QAction(tr("Onglet Arduino (porte capteurs Quai)"), this);
    actOpenQuaiArduino->setShortcut(QKeySequence(tr("Ctrl+Shift+Q")));
    connect(actOpenQuaiArduino, &QAction::triggered, this, [this]() {
        if (!ui || !ui->stackedWidget)
            return;
        // Build Quai UI before showing the page — switching stack first repaints a half-initialized module.
        setupQuaiMapOverviewUi();
        setupQuaiCharts();
        loadQuaisTable();
        QTimer::singleShot(0, this, [this]() {
            if (!ui || !ui->stackedWidget)
                return;
            ui->stackedWidget->setCurrentIndex(4);
            if (m_quaiArduinoTab && ui->mainTabWidget) {
                ui->mainTabWidget->setCurrentWidget(m_quaiArduinoTab);
                if (m_quaiArduinoWindow)
                    m_quaiArduinoWindow->refreshActiveQuaisCombo();
            }
        });
    });
    addAction(actOpenQuaiArduino);

    /* RFID EMPLOYE (port dédié). */
    {
        const int ret = m_rfidArduino.connect_arduino(QString());
        switch (ret) {
        case 0:
            qDebug() << "RFID Arduino sur" << m_rfidArduino.getarduino_port_name();
            connectRfidReadyReadSignal();
            pushRfidArduinoDoorPolicy(true);
            break;
        case 1:
            qDebug() << "RFID: impossible d'ouvrir" << m_rfidArduino.getarduino_port_name();
            break;
        case -1:
            qDebug() << "RFID: port non detecte (USB ou moniteur serie ouvert).";
            break;
        }
        if (ret != 0) {
            QTimer::singleShot(1200, this, [this]() {
                if (m_rfidArduino.getserial() && m_rfidArduino.getserial()->isOpen())
                    return;
                const QString esp32Port = (m_esp32Cam.getserial() && m_esp32Cam.getserial()->isOpen())
                    ? m_esp32Cam.getarduino_port_name()
                    : QString();
                if (QMessageBox::question(this, tr("RFID"),
                        tr("Connecter le lecteur RFID ?\n(Fermez le moniteur série Arduino.)"),
                        QMessageBox::Yes | QMessageBox::No,
                        QMessageBox::Yes) != QMessageBox::Yes)
                    return;
                if (m_rfidArduino.prompt_configure_port(this, esp32Port)) {
                    connectRfidReadyReadSignal();
                    pushRfidArduinoDoorPolicy(true);
                }
            });
        }
    }

    /* RFID PECHEUR (2e maquette, port dédié différent). */
    {
        const QString employePort = (m_rfidArduino.getserial() && m_rfidArduino.getserial()->isOpen())
            ? m_rfidArduino.getarduino_port_name()
            : QString();
        const int ret = m_rfidPecheurArduino.connect_arduino(employePort);
        switch (ret) {
        case 0:
            qDebug() << "RFID Pecheur Arduino sur" << m_rfidPecheurArduino.getarduino_port_name();
            connectRfidPecheurReadyReadSignal();
            break;
        case 1:
            qDebug() << "RFID Pecheur: impossible d'ouvrir" << m_rfidPecheurArduino.getarduino_port_name();
            break;
        case -1:
            qDebug() << "RFID Pecheur: port non detecte (USB ou moniteur serie ouvert).";
            break;
        }
        if (ret != 0) {
            QTimer::singleShot(1400, this, [this]() {
                if (m_rfidPecheurArduino.getserial() && m_rfidPecheurArduino.getserial()->isOpen())
                    return;
                const QString employePort2 = (m_rfidArduino.getserial() && m_rfidArduino.getserial()->isOpen())
                    ? m_rfidArduino.getarduino_port_name()
                    : QString();
                if (QMessageBox::question(this, tr("RFID Pecheur"),
                        tr("Connecter le lecteur RFID du module pêcheur ?"),
                        QMessageBox::Yes | QMessageBox::No,
                        QMessageBox::Yes) != QMessageBox::Yes)
                    return;
                if (m_rfidPecheurArduino.prompt_configure_port(this, employePort2))
                    connectRfidPecheurReadyReadSignal();
            });
        }
    }

    const QString rfidBusy = (m_rfidArduino.getserial() && m_rfidArduino.getserial()->isOpen())
        ? m_rfidArduino.getarduino_port_name()
        : QString();
    m_esp32Cam.set_exclude_serial_port(rfidBusy);

    if (ui->lineEdit_rfid)
        ui->lineEdit_rfid->installEventFilter(this);
    if (ui->le_rfid_2)
        ui->le_rfid_2->installEventFilter(this);

    {
        const int ret = m_esp32Cam.connect_arduino();
        switch (ret) {
        case 0:
            qDebug() << "ESP32-CAM disponible et connectee sur" << m_esp32Cam.getarduino_port_name();
            break;
        case 1:
            qDebug() << "ESP32-CAM: port detecte mais ouverture impossible :" << m_esp32Cam.getarduino_port_name();
            break;
        case -1:
            qDebug() << "ESP32-CAM non detectee (USB). Configuration possible a la premiere capture.";
            break;
        }
    }

    // Quai CRUD buttons: subtle drop shadow
    for (QPushButton *btn : { ui->btnAdd, ui->btnUpdate, ui->btnDelete }) {
        if (btn) {
            auto *shadow = new QGraphicsDropShadowEffect(btn);
            shadow->setBlurRadius(4);
            shadow->setOffset(0, 2);
            shadow->setColor(QColor(0, 0, 0, 51));
            btn->setGraphicsEffect(shadow);
        }
    }

    // Quai toolbar: spacing and margins so buttons/filters are clearly separated (especially in full screen)
    if (ui->crudButtonLayout) {
        ui->crudButtonLayout->setSpacing(24);
        ui->crudButtonLayout->setContentsMargins(16, 10, 16, 10);
    }
    // Give stretch to the spacer between filters and search so extra width is absorbed there
    if (ui->crudButtonLayout && ui->crudButtonLayout->count() > 9) {
        ui->crudButtonLayout->setStretch(9, 1);
    }
    if (ui->tableSupervisionCombined) {
        ui->tableSupervisionCombined->setSelectionBehavior(QAbstractItemView::SelectRows);
        ui->tableSupervisionCombined->setSelectionMode(QAbstractItemView::SingleSelection);
        ui->tableSupervisionCombined->verticalHeader()->setDefaultSectionSize(34);
        ui->tableSupervisionCombined->setWordWrap(true);
    }
    if (ui->groupBoxOccupation) {
        // Ensure the 2x2 donut grid fits fully inside the occupation box.
        ui->groupBoxOccupation->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        ui->groupBoxOccupation->setMinimumHeight(520);
        ui->groupBoxOccupation->setMaximumHeight(540);
    }
    if (ui->occupationLayout) {
        ui->occupationLayout->setContentsMargins(6, 10, 6, 8);
        ui->occupationLayout->setSpacing(8);
    }
    // Give vertical slack to chartCardAlertes (incident box); table stays compact (see max height in .ui)
    if (ui->superRightLayout && ui->superRightLayout->count() >= 3) {
        ui->superRightLayout->setStretch(0, 0);
        ui->superRightLayout->setStretch(1, 0);
        ui->superRightLayout->setStretch(2, 1);
    }
    if (ui->textEditAlertes) {
        if (ui->chartCardAlertes) {
            ui->chartCardAlertes->setMinimumHeight(250);
        }
        ui->textEditAlertes->setMinimumHeight(235);
        ui->textEditAlertes->setMaximumHeight(500);
        if (QTextDocument *doc = ui->textEditAlertes->document())
            doc->setDocumentMargin(8);
        ui->textEditAlertes->viewport()->installEventFilter(this);
        ui->textEditAlertes->setCursor(Qt::PointingHandCursor);
        ui->textEditAlertes->setToolTip(
            tr("Indicateurs depuis la base. Utilisez le bouton « Actualiser » ou cliquez dans la zone. Actualisation auto toutes les 12 s sur le module Quais."));
    }

    m_quaiAlertsPollTimer = new QTimer(this);
    m_quaiAlertsPollTimer->setInterval(12000);
    connect(m_quaiAlertsPollTimer, &QTimer::timeout, this, [this]() {
        if (!ui || !ui->stackedWidget || ui->stackedWidget->currentIndex() != 4)
            return;
        QSqlDatabase db = QSqlDatabase::database(Connection::connectionName());
        if (!db.isOpen())
            return;
        refreshQuaiStats();
        refreshQuaiAlertesImportantFromDb(false);
    });
    if (ui->stackedWidget) {
        connect(ui->stackedWidget, &QStackedWidget::currentChanged, this, [this](int idx) {
            if (idx == 4)
                startQuaiAlertsPolling();
            else
                stopQuaiAlertsPolling();
        });
    }

    // Contrôles formulaire pêcheur: longueur max nom/prénom, date limite >= aujourd'hui, téléphone Tunisie +216
    if (ui->le_nom_2) ui->le_nom_2->setMaxLength(100);
    if (ui->le_prenom_2) ui->le_prenom_2->setMaxLength(100);
    if (ui->le_telephone_2) ui->le_telephone_2->setMaxLength(8); // 8 chiffres après +216
    if (ui->dateEdit_limit_2) ui->dateEdit_limit_2->setMinimumDate(QDate::currentDate());

    // Contrôles de saisie - module Equipement
    if (ui->label) ui->label->setVisible(false);   // Hide "Id_equipement:"
    if (ui->lineEdit) {
        ui->lineEdit->setVisible(false);            // Keep internal ID field hidden (still usable for update)
        ui->lineEdit->setMaxLength(10);
        ui->lineEdit->setValidator(new QRegularExpressionValidator(
            QRegularExpression(QStringLiteral("^[0-9]{0,10}$")), ui->lineEdit));
        ui->lineEdit->setPlaceholderText("ID numérique");
    }
    // Move left equip fields up (ID row hidden): keep layout visually aligned at the top.
    if (ui->gridLayout_equip_form) {
        // Hidden Id row still sits in cells (0,0)/(0,1) from the .ui file. Remove it from the grid
        // before placing Type/Nom on row 0 — otherwise two widgets share the same cell (undefined
        // layout / crashes on some Qt builds).
        if (ui->label)
            ui->gridLayout_equip_form->removeWidget(ui->label);
        if (ui->lineEdit)
            ui->gridLayout_equip_form->removeWidget(ui->lineEdit);
        if (ui->label_3 && ui->lineEdit_3) {
            ui->gridLayout_equip_form->addWidget(ui->label_3, 0, 0);
            ui->gridLayout_equip_form->addWidget(ui->lineEdit_3, 0, 1);
        }
        if (ui->label_5 && ui->dateEdit) {
            ui->gridLayout_equip_form->addWidget(ui->label_5, 1, 0);
            ui->gridLayout_equip_form->addWidget(ui->dateEdit, 1, 1);
        }
        if (ui->label_7 && ui->lineEdit_4) {
            ui->gridLayout_equip_form->addWidget(ui->label_7, 2, 0);
            ui->gridLayout_equip_form->addWidget(ui->lineEdit_4, 2, 1);
        }
        if (ui->label_2 && ui->lineEdit_2) {
            ui->gridLayout_equip_form->addWidget(ui->label_2, 0, 2);
            ui->gridLayout_equip_form->addWidget(ui->lineEdit_2, 0, 3);
        }
        if (ui->label_6 && ui->comboBox_2) {
            ui->gridLayout_equip_form->addWidget(ui->label_6, 1, 2);
            ui->gridLayout_equip_form->addWidget(ui->comboBox_2, 1, 3);
        }
        if (ui->label_8 && ui->lineEdit_11) {
            ui->gridLayout_equip_form->addWidget(ui->label_8, 2, 2);
            ui->gridLayout_equip_form->addWidget(ui->lineEdit_11, 2, 3);
        }
    }
    if (ui->lineEdit_2) {
        ui->lineEdit_2->setMaxLength(80);
        ui->lineEdit_2->setValidator(new QRegularExpressionValidator(
            QRegularExpression(QStringLiteral("^[\\p{L}0-9 '\\-]{0,80}$")), ui->lineEdit_2));
    }
    if (ui->lineEdit_3) {
        ui->lineEdit_3->setMaxLength(80);
        ui->lineEdit_3->setValidator(new QRegularExpressionValidator(
            QRegularExpression(QStringLiteral("^[\\p{L} '\\-]{0,80}$")), ui->lineEdit_3));
    }
    if (ui->lineEdit_11) {
        ui->lineEdit_11->setMaxLength(80);
        ui->lineEdit_11->setValidator(new QRegularExpressionValidator(
            QRegularExpression(QStringLiteral("^[\\p{L}0-9 '\\-]{0,80}$")), ui->lineEdit_11));
    }
    if (ui->lineEdit_4) {
        ui->lineEdit_4->setMaxLength(12);
        ui->lineEdit_4->setValidator(new QRegularExpressionValidator(
            QRegularExpression(QStringLiteral("^(|[0-9]{1,9}([\\.,][0-9]{0,2})?)$")), ui->lineEdit_4));
        ui->lineEdit_4->setPlaceholderText("0 ou positif (ex: 1200.50)");
    }
    if (ui->dateEdit) {
        ui->dateEdit->setMaximumDate(QDate::currentDate());
    }
    if (ui->label_4 && ui->comboBox && ui->gridLayout_equip_form) {
        ui->gridLayout_equip_form->removeWidget(ui->label_4);
        ui->gridLayout_equip_form->removeWidget(ui->comboBox);
        ui->gridLayout_equip_form->addWidget(ui->label_4, 3, 0);
        ui->gridLayout_equip_form->addWidget(ui->comboBox, 3, 1, 1, 3);
        ui->label_4->setVisible(true);
        ui->comboBox->setVisible(true);
    }

    // Visual polish - module Equipement aligned with Quai module look.
    if (ui->groupBox) {
        ui->groupBox->setMinimumHeight(320);
        ui->groupBox->setMaximumHeight(500);
        ui->groupBox->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
        ui->groupBox->setStyleSheet(QStringLiteral(
            "QGroupBox#groupBox {"
            "  background-color: #262626;"
            "  border: 2px solid #2a9d8f;"
            "  border-radius: 10px;"
            "  margin-top: 16px;"
            "  padding: 10px;"
            "}"
            "QGroupBox#groupBox::title {"
            "  subcontrol-origin: margin;"
            "  left: 14px;"
            "  padding: 4px 10px;"
            "  color: #ffffff;"
            "  background-color: #1e5f74;"
            "  border-radius: 6px;"
            "  font-weight: 700;"
            "}"
            "QGroupBox#groupBox QLabel { color: #ecf0f1; font-weight: 600; }"
            "QGroupBox#groupBox QLineEdit,"
            "QGroupBox#groupBox QComboBox,"
            "QGroupBox#groupBox QDateEdit {"
            "  background: #f4f4f4;"
            "  color: #1a1a1a;"
            "  border: 1px solid #d0d0d0;"
            "  border-radius: 6px;"
            "  padding: 6px 8px;"
            "}"
            "QGroupBox#groupBox QLineEdit:focus,"
            "QGroupBox#groupBox QComboBox:focus,"
            "QGroupBox#groupBox QDateEdit:focus {"
            "  border: 1px solid #3498db;"
            "}"));
    }
    if (ui->gridLayout_equip_form) {
        ui->gridLayout_equip_form->setContentsMargins(26, 12, 26, 12);
        ui->gridLayout_equip_form->setHorizontalSpacing(22);
        ui->gridLayout_equip_form->setVerticalSpacing(12);
        ui->gridLayout_equip_form->setColumnMinimumWidth(0, 110); // left labels
        ui->gridLayout_equip_form->setColumnMinimumWidth(2, 110); // right labels
        ui->gridLayout_equip_form->setColumnMinimumWidth(4, 180); // photo card column
        ui->gridLayout_equip_form->setColumnStretch(0, 0);
        ui->gridLayout_equip_form->setColumnStretch(1, 2);
        ui->gridLayout_equip_form->setColumnStretch(2, 0);
        ui->gridLayout_equip_form->setColumnStretch(3, 2);
        ui->gridLayout_equip_form->setColumnStretch(4, 1); // photo preview card slightly narrower
    }
    for (QLabel *lb : { ui->label_2, ui->label_3, ui->label_4, ui->label_5, ui->label_6, ui->label_7, ui->label_8 }) {
        if (!lb) continue;
        lb->setMinimumWidth(95);
        lb->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    }
    for (QWidget *w : { static_cast<QWidget*>(ui->lineEdit_2), static_cast<QWidget*>(ui->lineEdit_3), static_cast<QWidget*>(ui->lineEdit_4),
                        static_cast<QWidget*>(ui->lineEdit_11), static_cast<QWidget*>(ui->comboBox_2), static_cast<QWidget*>(ui->dateEdit) }) {
        if (!w) continue;
        w->setMinimumHeight(34);
        w->setMaximumWidth(QWIDGETSIZE_MAX);
        w->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    }
    if (ui->gridLayout_equip_form && !m_equipPhotoPreview) {
        QWidget *photoCard = new QWidget(this);
        photoCard->setObjectName(QStringLiteral("equipPhotoCard"));
        photoCard->setStyleSheet(QStringLiteral(
            "QWidget#equipPhotoCard { background:#202733; border:1px solid #395470; border-radius:8px; }"
            "QLabel#equipPhotoPreview { background:#161b24; color:#9fb3c8; border:1px dashed #4b6886; border-radius:6px; }"));
        QVBoxLayout *photoLayout = new QVBoxLayout(photoCard);
        photoLayout->setContentsMargins(8, 6, 8, 6);
        photoLayout->setSpacing(3);
        m_equipPhotoPreview = new QLabel(tr("Photo Placeholder"), photoCard);
        m_equipPhotoPreview->setObjectName(QStringLiteral("equipPhotoPreview"));
        m_equipPhotoPreview->setMinimumSize(0, 0);
        m_equipPhotoPreview->setMinimumHeight(165);
        m_equipPhotoPreview->setMaximumHeight(290);
        m_equipPhotoPreview->setMaximumWidth(QWIDGETSIZE_MAX);
        m_equipPhotoPreview->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        m_equipPhotoPreview->setAlignment(Qt::AlignCenter);
        m_equipPhotoPreview->setScaledContents(false);
        showEquipPhotoPreview(m_equipPhotoPreview, QString());
        QPushButton *btnPickPhoto = new QPushButton(tr("Choisir Photo"), photoCard);
        btnPickPhoto->setObjectName(QStringLiteral("btnEquipPickPhoto"));
        btnPickPhoto->setCursor(Qt::PointingHandCursor);
        btnPickPhoto->setMinimumHeight(24);
        btnPickPhoto->setMaximumHeight(28);
        photoLayout->addWidget(m_equipPhotoPreview, 1);
        photoLayout->addWidget(btnPickPhoto, 0, Qt::AlignCenter);
        photoCard->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        photoCard->setMaximumWidth(QWIDGETSIZE_MAX);
        photoCard->setMinimumWidth(185);
        photoCard->setMinimumHeight(260);
        photoCard->setMaximumHeight(330);
        ui->gridLayout_equip_form->addWidget(photoCard, 0, 4, 4, 1, Qt::AlignTop);

        connect(btnPickPhoto, &QPushButton::clicked, this, [this]() {
            const QString srcPath = QFileDialog::getOpenFileName(
                this, tr("Choisir une image"), QString(),
                tr("Images (*.png *.jpg *.jpeg *.bmp *.webp)"));
            if (srcPath.isEmpty())
                return;
            m_equipPendingPhotoPath = srcPath;
            showEquipPhotoPreview(m_equipPhotoPreview, srcPath);
        });
    }
    if (ui->frameEquipToolbar) {
        ui->frameEquipToolbar->setStyleSheet(QStringLiteral(
            "QFrame#frameEquipToolbar {"
            "  background-color: #31485f;"
            "  border: 2px solid #2a9d8f;"
            "  border-radius: 10px;"
            "  padding: 8px;"
            "}"));
    }
    if (ui->h_buttons_equip) {
        ui->h_buttons_equip->setSpacing(12);
        ui->h_buttons_equip->setContentsMargins(10, 8, 10, 8);
    }
    for (QPushButton *b : { ui->pushButton_2, ui->pushButton_4, ui->pushButton_5, ui->btnGenerateQR, ui->pushButton_7 }) {
        if (!b) continue;
        b->setMinimumHeight(34);
        b->setCursor(Qt::PointingHandCursor);
    }
    if (ui->pushButton_2) ui->pushButton_2->setStyleSheet(QStringLiteral("QPushButton { background:#3598DB; color:#000; border:none; border-radius:6px; font-weight:bold; padding:6px 12px; } QPushButton:hover { background:#2980b9; }"));
    if (ui->pushButton_4) ui->pushButton_4->setStyleSheet(QStringLiteral("QPushButton { background:#3598DB; color:#000; border:none; border-radius:6px; font-weight:bold; padding:6px 12px; } QPushButton:hover { background:#2980b9; }"));
    if (ui->pushButton_5) ui->pushButton_5->setStyleSheet(QStringLiteral("QPushButton { background:#e74c3c; color:#fff; border:none; border-radius:6px; font-weight:bold; padding:6px 12px; } QPushButton:hover { background:#c0392b; }"));
    if (ui->btnGenerateQR) ui->btnGenerateQR->setStyleSheet(QStringLiteral("QPushButton { background:#3598DB; color:#000; border:none; border-radius:6px; font-weight:bold; padding:6px 12px; } QPushButton:hover { background:#2980b9; }"));
    if (ui->pushButton_7) ui->pushButton_7->setStyleSheet(QStringLiteral("QPushButton { background:#d68910; color:#fff; border:none; border-radius:6px; font-weight:bold; padding:6px 12px; } QPushButton:hover { background:#b9770e; }"));
    if (ui->lineEdit_8) {
        ui->lineEdit_8->setMinimumHeight(36);
        ui->lineEdit_8->setClearButtonEnabled(true);
        ui->lineEdit_8->setStyleSheet(QStringLiteral(
            "QLineEdit { background:#1f2329; color:#ecf0f1; border:2px solid #2a9d8f; border-radius:6px; padding:6px 10px; }"
            "QLineEdit:focus { border:2px solid #3598DB; }"));
    }
    if (ui->comboBox_3) {
        ui->comboBox_3->setMinimumHeight(36);
        ui->comboBox_3->setStyleSheet(QStringLiteral(
            "QComboBox { background:#1f2329; color:#ecf0f1; border:2px solid #2a9d8f; border-radius:6px; padding:6px 10px; }"
            "QComboBox:focus { border:2px solid #3598DB; }"));
    }
    if (ui->label_10) {
        ui->label_10->setStyleSheet(QStringLiteral("color:#d7ecff; font-weight:700; border:none;"));
    }
    if (ui->chartContainer_2) {
        ui->chartContainer_2->setStyleSheet(QStringLiteral(
            "QFrame#chartContainer_2 { background:#1f2329; border:1px solid #2f8ccf; border-radius:10px; }"));
    }
    if (ui->scrollArea_equip_form) {
        // Let the form area adapt to screen height while keeping harmony with table.
        ui->scrollArea_equip_form->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
        ui->scrollArea_equip_form->setMinimumHeight(320);
        ui->scrollArea_equip_form->setMaximumHeight(460);
    }
    if (ui->scrollAreaWidgetContents_equip && ui->groupBox && !ui->scrollAreaWidgetContents_equip->layout()) {
        auto *equipRoot = new QVBoxLayout(ui->scrollAreaWidgetContents_equip);
        equipRoot->setContentsMargins(0, 0, 0, 0);
        equipRoot->setSpacing(0);
        equipRoot->addWidget(ui->groupBox);
    }
    if (ui->verticalLayout_equip_content) {
        ui->verticalLayout_equip_content->setContentsMargins(8, 8, 8, 8);
        ui->verticalLayout_equip_content->setSpacing(8);
        // 0=form card, 1=toolbar, 2=search row, 3=table container (dominant)
        ui->verticalLayout_equip_content->setStretch(0, 0);
        ui->verticalLayout_equip_content->setStretch(1, 0);
        ui->verticalLayout_equip_content->setStretch(2, 0);
        ui->verticalLayout_equip_content->setStretch(3, 2);
    }
    if (ui->chartContainer_2) {
        ui->chartContainer_2->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        ui->chartContainer_2->setMinimumHeight(320);
    }
    if (ui->tableWidget_3) {
        ui->tableWidget_3->setAlternatingRowColors(false);
        ui->tableWidget_3->setSelectionBehavior(QAbstractItemView::SelectRows);
        ui->tableWidget_3->setSelectionMode(QAbstractItemView::SingleSelection);
        ui->tableWidget_3->setSortingEnabled(true);
        ui->tableWidget_3->verticalHeader()->setVisible(false);
        ui->tableWidget_3->verticalHeader()->setDefaultSectionSize(34);
        ui->tableWidget_3->setShowGrid(false);
        ui->tableWidget_3->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        ui->tableWidget_3->setStyleSheet(QStringLiteral(
            "QTableWidget {"
            "  background-color: #2b2b2b;"
            "  gridline-color: #3d3d3d;"
            "  border-radius: 10px;"
            "  border: 2px solid #2a9d8f;"
            "  color: #ffffff;"
            "  font-size: 12px;"
            "}"
            "QHeaderView::section {"
            "  background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #3a3a3a, stop:1 #252525);"
            "  color: #ffffff;"
            "  padding: 10px 16px;"
            "  border: none;"
            "  border-right: 1px solid #454545;"
            "  font-weight: 700;"
            "  min-width: 72px;"
            "}"
            "QTableWidget::item { padding: 10px 12px; }"
            "QTableWidget::item:selected {"
            "  background: #2a9d8f;"
            "  color: white;"
            "}"));
        if (ui->tableWidget_3->horizontalHeader()) {
            ui->tableWidget_3->horizontalHeader()->setStretchLastSection(false);
            ui->tableWidget_3->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
            ui->tableWidget_3->horizontalHeader()->setDefaultAlignment(Qt::AlignCenter);
        }
    }

    setupTtsUi();

    // Contrôles de saisie - module Quai
    if (ui->lineEditNom) {
        ui->lineEditNom->setMaxLength(80);
        ui->lineEditNom->setValidator(new QRegularExpressionValidator(
            QRegularExpression(QStringLiteral("^[\\p{L}0-9 '\\-]{0,80}$")), ui->lineEditNom));
    }
    if (ui->doubleSpinBoxLongueur) ui->doubleSpinBoxLongueur->setMinimum(0.0);
    if (ui->doubleSpinBoxProfondeur) ui->doubleSpinBoxProfondeur->setMinimum(0.0);
    if (ui->spinBoxCapacite) ui->spinBoxCapacite->setMinimum(1);
    if (ui->spinBoxCurrentUsage) ui->spinBoxCurrentUsage->setMinimum(0);
    if (ui->doubleSpinBoxOccupation) {
        ui->doubleSpinBoxOccupation->setMinimum(0.0);
        ui->doubleSpinBoxOccupation->setMaximum(100.0);
    }

    setupPecheurCharts();
    setupEmployeeStatsCharts();
    setupEmployeeListeAlertBadge();
    setupMonitorPeriodSelectors();
    setupSmartPortSelectors();
    setupSmartPortActuatorSliders();
    {
        const QString rfidBusy = (m_rfidArduino.getserial() && m_rfidArduino.getserial()->isOpen())
            ? m_rfidArduino.getarduino_port_name()
            : QString();
        m_smartPortArduino.set_exclude_serial_port(rfidBusy);
        const int smartRet = m_smartPortArduino.connect_arduino(rfidBusy);
        if (smartRet == 0) {
            qDebug() << "SmartPort Arduino sur" << m_smartPortArduino.portName();
        } else {
            qDebug() << "SmartPort Arduino non detecte (USB) ou port indisponible. Configurer si necessaire.";
        }
    }

    // DB init for Employees CRUD
    m_db = Connection::createInstance().getDatabase();
    if (m_db.isValid() && m_db.isOpen()) {
        if (!ensureEmployeeTable()) {
            QMessageBox::warning(this, "Base de données", "Connexion OK, mais la table 'employe' est introuvable.");
        } else {
            loadEmployees();
        }
    }

    /* Quai_sys (porte capteurs) : liaison série dédiée + synchro périodique avec QUAI actifs en base. */
    {
        QStringList excludePorts;
        if (m_rfidArduino.getserial() && m_rfidArduino.getserial()->isOpen())
            excludePorts << m_rfidArduino.getarduino_port_name();
        if (m_rfidPecheurArduino.getserial() && m_rfidPecheurArduino.getserial()->isOpen())
            excludePorts << m_rfidPecheurArduino.getarduino_port_name();
        if (m_esp32Cam.getserial() && m_esp32Cam.getserial()->isOpen())
            excludePorts << m_esp32Cam.getarduino_port_name();
        if (m_smartPortArduino.isConnected())
            excludePorts << m_smartPortArduino.portName();
        const QString excludeJoined = excludePorts.join(QLatin1Char('|'));
        int qgRet = m_quaiGateArduino.connect_arduino(excludeJoined);
        /* Cas fréquent: une seule carte branchée, déjà prise par le module RFID.
           On libère ce port pour prioriser Quai_sys. */
        if (qgRet != 0) {
            const QList<QSerialPortInfo> portsNow = QSerialPortInfo::availablePorts();
            if (portsNow.size() == 1) {
                const QString onlyPort = portsNow.first().portName().trimmed();
                const QString rfidPort = m_rfidArduino.getarduino_port_name().trimmed();
                if (!onlyPort.isEmpty()
                    && !rfidPort.isEmpty()
                    && QString::compare(onlyPort, rfidPort, Qt::CaseInsensitive) == 0
                    && m_rfidArduino.getserial()
                    && m_rfidArduino.getserial()->isOpen()) {
                    qDebug() << "Quai gate: port unique" << onlyPort
                             << "deja utilise par RFID -> liberation du port pour Quai_sys.";
                    m_rfidArduino.close_arduino();
                    stopRfidSerialPolling();
                    qgRet = m_quaiGateArduino.connect_arduino(QString());
                }
            }
        }
        switch (qgRet) {
        case 0:
            qDebug() << "Quai gate (Quai_sys) sur" << m_quaiGateArduino.getarduino_port_name();
            setupQuaiGateSerialNotifier();
            syncQuaiGateArduinoFromDatabase();
            break;
        case 1:
            qDebug() << "Quai gate: impossible d'ouvrir le port" << m_quaiGateArduino.getarduino_port_name();
            break;
        case -1:
            qDebug() << "Quai gate: aucun port detecte (USB). Configurer dans parametres ATLAS arduino_quai_gate si besoin.";
            QTimer::singleShot(2000, this, [this, excludeJoined]() {
                if (m_quaiGateArduino.getserial() && m_quaiGateArduino.getserial()->isOpen())
                    return;
                if (QMessageBox::question(this, tr("Quai capteurs"),
                        tr("Connecter la carte Quai_sys (porte capteurs) ?\n"
                           "(Fermez le moniteur série Arduino sur ce port.)"),
                        QMessageBox::Yes | QMessageBox::No,
                        QMessageBox::Yes) != QMessageBox::Yes)
                    return;
                if (m_quaiGateArduino.prompt_configure_port(this, excludeJoined)) {
                    qDebug() << "Quai gate (Quai_sys) sur" << m_quaiGateArduino.getarduino_port_name();
                    setupQuaiGateSerialNotifier();
                    syncQuaiGateArduinoFromDatabase();
                }
            });
            break;
        }
        m_quaiGateDbPollTimer = new QTimer(this);
        m_quaiGateDbPollTimer->setInterval(3000);
        connect(m_quaiGateDbPollTimer, &QTimer::timeout, this, &MainWindow::syncQuaiGateArduinoFromDatabase);
        m_quaiGateDbPollTimer->start();
    }

    // Employee form validators and placeholders (exemples comme dans l'image)
    if (ui->lineEdit_cin) {
        ui->lineEdit_cin->setMaxLength(8);
        ui->lineEdit_cin->setPlaceholderText("8 chiffres");
        ui->lineEdit_cin->setInputMethodHints(Qt::ImhDigitsOnly);
    }
    if (ui->lineEdit_fname) {
        ui->lineEdit_fname->setMaxLength(50);
        ui->lineEdit_fname->setPlaceholderText("Prénom");
    }
    if (ui->lineEdit_lname) {
        ui->lineEdit_lname->setMaxLength(50);
        ui->lineEdit_lname->setPlaceholderText("Nom");
    }
    if (ui->lineEdit_phone) {
        ui->lineEdit_phone->setMaxLength(20);
        ui->lineEdit_phone->setPlaceholderText("+216XXXXXXXXX");
    }
    if (ui->lineEdit_email) {
        ui->lineEdit_email->setMaxLength(100);
        ui->lineEdit_email->setPlaceholderText("exemple@mail.com");
    }
    if (ui->lineEdit_pwd) {
        ui->lineEdit_pwd->setMaxLength(100);
        ui->lineEdit_pwd->setPlaceholderText("Au moins 6 caractères");
    }
    if (ui->lineEdit_addr) {
        ui->lineEdit_addr->setMaxLength(255);
        ui->lineEdit_addr->setPlaceholderText("Ex: 12 Rue Habib Bourguiba, Tunis");
    }
    if (ui->lineEdit_lic) {
        ui->lineEdit_lic->setPlaceholderText("Ex: 123456");
    }
    if (ui->combo_certs) {
        ui->combo_certs->setPlaceholderText("Ex: Navigation, Sécurité");
    }
    if (ui->search_input) connect(ui->search_input, &QLineEdit::returnPressed, this, &MainWindow::on_search_btn_2_clicked);
    if (ui->lineEdit_6) connect(ui->lineEdit_6, &QLineEdit::returnPressed, this, &MainWindow::on_pushButton_3_clicked);

    m_voiceTimer = new QTimer(this);
    connect(m_voiceTimer, &QTimer::timeout, this, [this]() {
        // Même fichier absolu que celui passé au script Python / PowerShell.
        const QString path = QFileInfo(QDir(QCoreApplication::applicationDirPath()).filePath(QStringLiteral("voice.txt")))
                                 .absoluteFilePath();
        QFile f(path);
        if (!f.open(QIODevice::ReadOnly))
            return;
        const QByteArray raw = f.readAll();
        f.close();
        QString content = QString::fromUtf8(raw);
        content.remove(QChar(0xFEFF));
        const QStringList lines = content.split(QRegularExpression(QStringLiteral("[\r\n]+")), Qt::SkipEmptyParts);
        QString lastLine;
        for (const QString &ln : lines) {
            const QString t = ln.trimmed();
            if (!t.isEmpty())
                lastLine = t;
        }
        if (!lastLine.isEmpty() && lastLine != m_voiceLastCommand) {
            m_voiceLastCommand = lastLine;
            processVoiceCommand(lastLine);
        }
    });
    m_btnVoice = ui->clear_btn_3;
    if (m_btnVoice) {
        m_btnVoice->setCheckable(true);
        m_btnVoice->setChecked(false);
        m_btnVoice->setToolTip(tr("Reconnaissance vocale OFF - clic pour activer"));
        connect(m_btnVoice, &QPushButton::toggled, this, &MainWindow::on_voice_toggled);
    }

    if (ui->tableWidget_2 && ui->tableWidget_2->horizontalHeader()) {
        connect(ui->tableWidget_2->horizontalHeader(), &QHeaderView::sectionClicked,
                this, &MainWindow::on_employeeTableHeaderSectionClicked);
    }

    if (ui->btnParametres) {
        QIcon settingsIcon(":/new/prefix1/parametres-cog.png");
        if (!settingsIcon.isNull())
            ui->btnParametres->setIcon(settingsIcon);
    }

    QUrl musicUrl;
    QStringList searchDirs;
    const QString appDir = QCoreApplication::applicationDirPath();
    searchDirs << appDir << QDir::currentPath();
    for (int up = 0; up < 5; ++up) {
        QString d = appDir;
        for (int i = 0; i < up; ++i) d = QDir(d).filePath("..");
        searchDirs << QDir(d).canonicalPath();
    }
    for (const QString &dir : searchDirs) {
        if (dir.isEmpty()) continue;
        const QString path = QDir(dir).filePath("musique/empty.mp3");
        if (QFileInfo::exists(path)) {
            musicUrl = QUrl::fromLocalFile(QFileInfo(path).canonicalFilePath());
            break;
        }
    }
    if (musicUrl.isValid()) {
        m_backgroundAudioOutput = new QAudioOutput(this);
        m_backgroundMusic = new QMediaPlayer(this);
        m_backgroundMusic->setAudioOutput(m_backgroundAudioOutput);
        m_backgroundMusic->setLoops(QMediaPlayer::Infinite);
        m_backgroundAudioOutput->setVolume(0.35f);
        m_backgroundMusic->setSource(musicUrl);
        m_backgroundMusic->play();
    }

    afficherPecheurs(); // Afficher les pêcheurs depuis la base de données
    if (ui->tableStockView && ui->tableStockView->horizontalHeader()) {
        ui->tableStockView->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
        ui->tableStockView->horizontalHeader()->setStretchLastSection(true);
    }
    refreshTable();
    setupStockHistory();
    if (ui->btnStockSave) ui->btnStockSave->setVisible(false);
    if (ui->btnStockAnnuler) ui->btnStockAnnuler->setVisible(false);
    chatbot_invisible();
    weight_window_invisible();
    refreshPhotoPrivacyControls();
}

MainWindow::~MainWindow()
{
    stopRfidSerialPolling();
    if (m_rfidPecheurPollTimer)
        m_rfidPecheurPollTimer->stop();
    if (m_quaiGateDbPollTimer)
        m_quaiGateDbPollTimer->stop();
    m_rfidArduino.close_arduino();
    m_rfidPecheurArduino.close_arduino();
    m_quaiGateArduino.close_arduino();
    m_esp32Cam.close_arduino();
    m_smartPortArduino.close_arduino();

    if (QSerialPort *stockSerial = findChild<QSerialPort *>(QStringLiteral("SharedStockSerialPort"))) {
        if (stockSerial->isOpen()) {
            stockSerial->close();
        }
    }

    saveTtsSettings();
    delete ui;
}

void MainWindow::setupTtsUi()
{
    if (!ui)
        return;
    if (!m_ttsEngine)
        m_ttsEngine = new QTextToSpeech(this);

    if (ui->checkBoxEnableTTS)
        ui->checkBoxEnableTTS->setVisible(false); // replaced by selector

    if (ui->tableTTSLog) {
        ui->tableTTSLog->setColumnCount(3);
        ui->tableTTSLog->setHorizontalHeaderLabels({tr("Heure"), tr("Type"), tr("Message")});
        ui->tableTTSLog->setSelectionBehavior(QAbstractItemView::SelectRows);
        ui->tableTTSLog->setSelectionMode(QAbstractItemView::SingleSelection);
        ui->tableTTSLog->setEditTriggers(QAbstractItemView::NoEditTriggers);
        ui->tableTTSLog->verticalHeader()->setVisible(false);
        if (ui->tableTTSLog->horizontalHeader()) {
            ui->tableTTSLog->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
            ui->tableTTSLog->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
            ui->tableTTSLog->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
        }
    }

    if (ui->btnTestTTS)
        connect(ui->btnTestTTS, &QPushButton::clicked, this, &MainWindow::runTtsManualTest);

    if (ui->ttsFormLayout && !this->findChild<QComboBox*>(QStringLiteral("comboBoxTTSEnable"))) {
        QLabel *lbl = new QLabel(tr("Synthèse vocale:"), this);
        lbl->setObjectName(QStringLiteral("labelTTSEnable"));
        QComboBox *cmb = new QComboBox(this);
        cmb->setObjectName(QStringLiteral("comboBoxTTSEnable"));
        cmb->addItem(tr("Activer"));
        cmb->addItem(tr("Désactiver"));
        cmb->setCurrentIndex(0); // auto enabled by default
        ui->ttsFormLayout->insertRow(0, lbl, cmb);
    }
    if (QComboBox *cmb = this->findChild<QComboBox*>(QStringLiteral("comboBoxTTSEnable"))) {
        connect(cmb, &QComboBox::currentIndexChanged, this, [this](int){ saveTtsSettings(); });
    }
    if (ui->comboBoxLanguage) {
        connect(ui->comboBoxLanguage, &QComboBox::currentIndexChanged, this, [this](int){
            configureTtsEngineLanguage();
            saveTtsSettings();
        });
    }
    if (ui->checkBoxReadSelected)
        connect(ui->checkBoxReadSelected, &QCheckBox::toggled, this, [this](bool){ saveTtsSettings(); });
    if (ui->checkBoxCriticalAlerts)
        connect(ui->checkBoxCriticalAlerts, &QCheckBox::toggled, this, [this](bool){ saveTtsSettings(); });
    if (ui->checkBoxDailySummary)
        connect(ui->checkBoxDailySummary, &QCheckBox::toggled, this, [this](bool){ saveTtsSettings(); });
    if (ui->timeEditSummary)
        connect(ui->timeEditSummary, &QTimeEdit::timeChanged, this, [this](const QTime &){ saveTtsSettings(); });

    if (ui->tableSupervisionCombined) {
        connect(ui->tableSupervisionCombined, &QTableWidget::cellClicked, this,
                [this](int row, int) { announceSelectedQuaiFromRow(ui->tableSupervisionCombined, row); });
    }

    if (!m_ttsTimer) {
        m_ttsTimer = new QTimer(this);
        m_ttsTimer->setInterval(30000); // 30s
        connect(m_ttsTimer, &QTimer::timeout, this, &MainWindow::maybeSpeakDailySummary);
        m_ttsTimer->start();
    }
    loadTtsSettings();
    configureTtsEngineLanguage();
    m_ttsLastSummaryDate = QDate::currentDate().addDays(-1);
}

void MainWindow::appendTtsLog(const QString &type, const QString &message)
{
    if (!ui || !ui->tableTTSLog)
        return;
    const int row = ui->tableTTSLog->rowCount();
    ui->tableTTSLog->insertRow(row);
    ui->tableTTSLog->setItem(row, 0, new QTableWidgetItem(QTime::currentTime().toString("HH:mm:ss")));
    ui->tableTTSLog->setItem(row, 1, new QTableWidgetItem(type));
    ui->tableTTSLog->setItem(row, 2, new QTableWidgetItem(message));
    ui->tableTTSLog->scrollToBottom();
}

bool MainWindow::isTtsEnabled() const
{
    if (!ui)
        return false;
    if (QComboBox *cmb = this->findChild<QComboBox*>(QStringLiteral("comboBoxTTSEnable")))
        return cmb->currentIndex() == 0;
    return ui->checkBoxEnableTTS && ui->checkBoxEnableTTS->isChecked();
}

void MainWindow::loadTtsSettings()
{
    if (!ui)
        return;
    QSettings s(QStringLiteral("ATLAS"), QStringLiteral("ATLAS"));
    if (QComboBox *cmb = this->findChild<QComboBox*>(QStringLiteral("comboBoxTTSEnable")))
        cmb->setCurrentIndex(s.value(QStringLiteral("tts/enabled"), 0).toInt() == 0 ? 0 : 1);
    if (ui->comboBoxLanguage)
        ui->comboBoxLanguage->setCurrentIndex(s.value(QStringLiteral("tts/languageIndex"), 0).toInt());
    if (ui->checkBoxReadSelected)
        ui->checkBoxReadSelected->setChecked(s.value(QStringLiteral("tts/readSelected"), true).toBool());
    if (ui->checkBoxCriticalAlerts)
        ui->checkBoxCriticalAlerts->setChecked(s.value(QStringLiteral("tts/criticalAlerts"), true).toBool());
    if (ui->checkBoxDailySummary)
        ui->checkBoxDailySummary->setChecked(s.value(QStringLiteral("tts/dailySummary"), false).toBool());
    if (ui->timeEditSummary)
        ui->timeEditSummary->setTime(s.value(QStringLiteral("tts/summaryTime"), QTime(8, 0)).toTime());
    configureTtsEngineLanguage();
}

void MainWindow::saveTtsSettings() const
{
    if (!ui)
        return;
    QSettings s(QStringLiteral("ATLAS"), QStringLiteral("ATLAS"));
    if (QComboBox *cmb = this->findChild<QComboBox*>(QStringLiteral("comboBoxTTSEnable")))
        s.setValue(QStringLiteral("tts/enabled"), cmb->currentIndex());
    if (ui->comboBoxLanguage)
        s.setValue(QStringLiteral("tts/languageIndex"), ui->comboBoxLanguage->currentIndex());
    if (ui->checkBoxReadSelected)
        s.setValue(QStringLiteral("tts/readSelected"), ui->checkBoxReadSelected->isChecked());
    if (ui->checkBoxCriticalAlerts)
        s.setValue(QStringLiteral("tts/criticalAlerts"), ui->checkBoxCriticalAlerts->isChecked());
    if (ui->checkBoxDailySummary)
        s.setValue(QStringLiteral("tts/dailySummary"), ui->checkBoxDailySummary->isChecked());
    if (ui->timeEditSummary)
        s.setValue(QStringLiteral("tts/summaryTime"), ui->timeEditSummary->time());
}

void MainWindow::announceSelectedQuaiFromRow(QTableWidget *table, int row)
{
    if (!ui || !table || row < 0 || !isTtsEnabled() || !ui->checkBoxReadSelected || !ui->checkBoxReadSelected->isChecked())
        return;

    QString quaiNom;
    QString quaiType;
    QString statut;
    if (table == ui->tableQuais) {
        if (QTableWidgetItem *it = table->item(row, 1)) quaiNom = it->text().trimmed();
        if (QTableWidgetItem *it = table->item(row, 2)) quaiType = it->text().trimmed();
        if (QTableWidgetItem *it = table->item(row, 6)) statut = it->text().trimmed();
    } else if (table == ui->tableSupervisionCombined) {
        if (QTableWidgetItem *it = table->item(row, 1)) quaiNom = it->text().trimmed();
        if (QTableWidgetItem *it = table->item(row, 3)) quaiType = it->text().trimmed();
        if (QTableWidgetItem *it = table->item(row, 4)) statut = it->text().trimmed();
    }
    if (quaiNom.isEmpty()) return;
    if (quaiType.isEmpty()) quaiType = tr("Type inconnu");
    if (statut.isEmpty()) statut = tr("Statut inconnu");

    const QString lang = currentTtsLanguageCode();
    QString msg;
    if (lang == QStringLiteral("ar"))
        msg = tr("الرصيف المحدد %1 نوعه %2 وحالته %3").arg(quaiNom, quaiType, statut);
    else if (lang == QStringLiteral("en"))
        msg = tr("Selected quay %1, type %2, status %3").arg(quaiNom, quaiType, statut);
    else
        msg = tr("Quai sélectionné %1, type %2, statut %3").arg(quaiNom, quaiType, statut);
    appendTtsLog(tr("Lecture"), msg);
    speakTtsMessage(msg);
}

void MainWindow::configureTtsEngineLanguage()
{
    if (!m_ttsEngine)
        return;
    const QString lang = currentTtsLanguageCode();
    const QLocale locale = (lang == QStringLiteral("ar")) ? QLocale(QLocale::Arabic, QLocale::SaudiArabia)
                         : (lang == QStringLiteral("en")) ? QLocale(QLocale::English, QLocale::UnitedStates)
                         : QLocale(QLocale::French, QLocale::France);
    m_ttsEngine->setLocale(locale);
    const auto voices = m_ttsEngine->availableVoices();
    m_ttsHasLanguageVoice = false;
    if (voices.isEmpty())
        return;

    int bestScore = -1;
    int bestIdx = -1;
    const QStringList preferredNameHints = (lang == QStringLiteral("fr"))
        ? QStringList{QStringLiteral("hortense"), QStringLiteral("julie"), QStringLiteral("paul"), QStringLiteral("french")}
        : (lang == QStringLiteral("ar"))
            ? QStringList{QStringLiteral("hoda"), QStringLiteral("naayf"), QStringLiteral("arab")}
            : QStringList{QStringLiteral("zira"), QStringLiteral("david"), QStringLiteral("english")};

    for (int i = 0; i < voices.size(); ++i) {
        const auto &v = voices.at(i);
        int score = 0;
        if (v.locale().language() == locale.language()) {
            score += 100;
            m_ttsHasLanguageVoice = true;
        }
        if (v.locale().territory() == locale.territory())
            score += 25;
        const QString name = v.name().toLower();
        for (const QString &hint : preferredNameHints) {
            if (name.contains(hint)) {
                score += 20;
                break;
            }
        }
        if (score > bestScore) {
            bestScore = score;
            bestIdx = i;
        }
    }
    if (bestIdx >= 0)
        m_ttsEngine->setVoice(voices.at(bestIdx));
}

QString MainWindow::currentTtsLanguageCode() const
{
    const QString lang = ui && ui->comboBoxLanguage ? ui->comboBoxLanguage->currentText().trimmed() : QStringLiteral("Français");
    if (lang.compare(QStringLiteral("Arabe"), Qt::CaseInsensitive) == 0
        || lang.compare(QStringLiteral("Arabic"), Qt::CaseInsensitive) == 0) return QStringLiteral("ar");
    if (lang.compare(QStringLiteral("Anglais"), Qt::CaseInsensitive) == 0
        || lang.compare(QStringLiteral("English"), Qt::CaseInsensitive) == 0) return QStringLiteral("en");
    if (lang.compare(QStringLiteral("Français"), Qt::CaseInsensitive) == 0
        || lang.compare(QStringLiteral("Francais"), Qt::CaseInsensitive) == 0
        || lang.compare(QStringLiteral("French"), Qt::CaseInsensitive) == 0) return QStringLiteral("fr");
    return QStringLiteral("fr");
}

bool MainWindow::speakTtsMessage(const QString &message)
{
    if (!ui || !isTtsEnabled())
        return false;
    if (!m_ttsEngine)
        m_ttsEngine = new QTextToSpeech(this);
    const QString normalizedMessage = message.trimmed();
    if (normalizedMessage.isEmpty())
        return false;
    const qint64 nowMs = QDateTime::currentMSecsSinceEpoch();
    // Guard against accidental duplicate signal firing (same message sent twice almost instantly).
    if (normalizedMessage == m_lastSpokenTtsMessage && (nowMs - m_lastSpokenTtsAtMs) < 1200)
        return true;

    configureTtsEngineLanguage();
    bool ok = false;
    if (m_ttsHasLanguageVoice) {
        if (m_ttsEngine->state() == QTextToSpeech::Speaking)
            m_ttsEngine->stop();
        m_ttsEngine->say(normalizedMessage);
        ok = (m_ttsEngine->state() != QTextToSpeech::Error);
    } else {
        // Fallback when Qt backend has no voice for selected language (common on Windows).
        const QString lang = currentTtsLanguageCode();
        const QString culture = (lang == QStringLiteral("ar")) ? QStringLiteral("ar-SA")
                             : (lang == QStringLiteral("en")) ? QStringLiteral("en-US")
                             : QStringLiteral("fr-FR");
        const QString needle = (lang == QStringLiteral("ar")) ? QStringLiteral("arab|hoda|naayf")
                             : (lang == QStringLiteral("en")) ? QStringLiteral("english|zira|david")
                             : QStringLiteral("french|hortense|julie|paul");
        const QString script = QStringLiteral(
            "$ErrorActionPreference='SilentlyContinue'; "
            "Add-Type -AssemblyName System.Speech; "
            "$s=New-Object System.Speech.Synthesis.SpeechSynthesizer; "
            "$c='%1'; $n='%2'; "
            "$voices=$s.GetInstalledVoices() | Where-Object { $_.Enabled }; "
            "$v=$voices | Where-Object { $_.VoiceInfo.Culture.Name -ieq $c } | Select-Object -First 1; "
            "if(-not $v){$v=$voices | Where-Object { $_.VoiceInfo.Culture.Name -like (($c.Split('-')[0]) + '*') } | Select-Object -First 1}; "
            "if(-not $v){$v=$voices | Where-Object { $_.VoiceInfo.Name -match $n } | Select-Object -First 1}; "
            "if($v){$s.SelectVoice($v.VoiceInfo.Name)}; "
            "$s.Rate=0; $s.Speak('%3');")
            .arg(psSingleQuoteEscape(culture),
                 psSingleQuoteEscape(needle),
                 psSingleQuoteEscape(normalizedMessage));
        QProcess p;
        p.start(QStringLiteral("powershell"),
                QStringList() << QStringLiteral("-NoProfile")
                              << QStringLiteral("-ExecutionPolicy") << QStringLiteral("Bypass")
                              << QStringLiteral("-Command") << script);
        if (p.waitForFinished(15000))
            ok = (p.exitStatus() == QProcess::NormalExit && p.exitCode() == 0);
    }
    if (ok) {
        m_lastSpokenTtsMessage = normalizedMessage;
        m_lastSpokenTtsAtMs = nowMs;
    }
    return ok;
}

void MainWindow::maybeSpeakDailySummary()
{
    if (!ui || !isTtsEnabled())
        return;
    if (!ui->checkBoxDailySummary || !ui->checkBoxDailySummary->isChecked() || !ui->timeEditSummary)
        return;

    const QDate today = QDate::currentDate();
    const QTime now = QTime::currentTime();
    const QTime target = ui->timeEditSummary->time();
    if (m_ttsLastSummaryDate == today || now < target)
        return;

    const QString lang = currentTtsLanguageCode();
    QString summary;
    if (lang == QStringLiteral("ar")) {
        summary = tr("ملخص يومي: متابعة حالة الأرصفة والتنبيهات مستمرة.");
    } else if (lang == QStringLiteral("en")) {
        summary = tr("Daily summary: quay monitoring and alerts are active.");
    } else {
        summary = tr("Résumé quotidien : la surveillance des quais et des alertes est active.");
    }
    appendTtsLog(tr("Résumé"), summary);
    speakTtsMessage(summary);
    m_ttsLastSummaryDate = today;
}

void MainWindow::runTtsManualTest()
{
    QString text;
    if (ui && ui->tableTTSLog && ui->tableTTSLog->currentRow() >= 0) {
        QTableWidgetItem *msgItem = ui->tableTTSLog->item(ui->tableTTSLog->currentRow(), 2);
        if (msgItem) text = msgItem->text().trimmed();
    }
    if (text.isEmpty()) {
        const QString lang = currentTtsLanguageCode();
        QString suggested;
        if (lang == QStringLiteral("ar"))
            suggested = tr("مرحبًا، هذا نص تجريبي للقراءة الصوتية.");
        else if (lang == QStringLiteral("en"))
            suggested = tr("Hello, this is a custom text to speech sample.");
        else
            suggested = tr("Bonjour, ceci est un exemple de texte personnalisé.");

        bool ok = false;
        text = QInputDialog::getMultiLineText(this, tr("Texte à lire"),
                                              tr("Entrez le texte à lire à voix haute :"),
                                              suggested, &ok).trimmed();
        if (!ok || text.isEmpty())
            return;
    }

    appendTtsLog(tr("Lecture manuelle"), text);
    if (!speakTtsMessage(text))
        QMessageBox::warning(this, tr("TTS"), tr("Impossible de lancer la synthèse vocale sur cette machine."));
}

void MainWindow::setupResponsiveStockLayouts()
{
    if (!ui || !ui->tabWidgetStockVentes) return;

    QWidget *statsTab = ui->tabWidgetStockVentes->findChild<QWidget*>(QStringLiteral("tab"), Qt::FindDirectChildrenOnly);
    if (statsTab) {
        QGridLayout *statsGrid = qobject_cast<QGridLayout*>(statsTab->layout());
        if (!statsGrid) {
            statsGrid = new QGridLayout(statsTab);
            statsGrid->setContentsMargins(16, 16, 16, 16);
            statsGrid->setHorizontalSpacing(14);
            statsGrid->setVerticalSpacing(14);
        }

        QWidget *card1 = statsTab->findChild<QWidget*>(QStringLiteral("frame"), Qt::FindDirectChildrenOnly);
        QWidget *card2 = statsTab->findChild<QWidget*>(QStringLiteral("frame_2"), Qt::FindDirectChildrenOnly);
        QWidget *card3 = statsTab->findChild<QWidget*>(QStringLiteral("frame_4"), Qt::FindDirectChildrenOnly);
        QWidget *card4 = statsTab->findChild<QWidget*>(QStringLiteral("frame_5"), Qt::FindDirectChildrenOnly);

        const QVector<QWidget*> cards = { card1, card2, card3, card4 };
        for (QWidget *card : cards) {
            if (!card) continue;
            card->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        }

        if (card1 && statsGrid->indexOf(card1) < 0) statsGrid->addWidget(card1, 0, 0);
        if (card2 && statsGrid->indexOf(card2) < 0) statsGrid->addWidget(card2, 0, 1);
        if (card3 && statsGrid->indexOf(card3) < 0) statsGrid->addWidget(card3, 1, 0);
        if (card4 && statsGrid->indexOf(card4) < 0) statsGrid->addWidget(card4, 1, 1);

        statsGrid->setRowStretch(0, 1);
        statsGrid->setRowStretch(1, 1);
        statsGrid->setColumnStretch(0, 1);
        statsGrid->setColumnStretch(1, 1);

        for (const QString &hostName : { QStringLiteral("chartStockGlobal"), QStringLiteral("holoStock"), QStringLiteral("chartStock"), QStringLiteral("curveStock") }) {
            QWidget *host = statsTab->findChild<QWidget*>(hostName);
            if (host) host->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        }
    }

    QWidget *stockTab = ui->tabWidgetStockVentes->findChild<QWidget*>(QStringLiteral("tabStock"), Qt::FindDirectChildrenOnly);
    if (stockTab && !stockTab->layout()) {
        auto *root = new QVBoxLayout(stockTab);
        root->setContentsMargins(16, 14, 16, 14);
        root->setSpacing(10);

        QWidget *form = stockTab->findChild<QWidget*>(QStringLiteral("frameStockForm"), Qt::FindDirectChildrenOnly);
        QLineEdit *searchEdit = stockTab->findChild<QLineEdit*>(QStringLiteral("stockSearchEdit"), Qt::FindDirectChildrenOnly);
        QLabel *sortLabel = stockTab->findChild<QLabel*>(QStringLiteral("stockSortLabel"), Qt::FindDirectChildrenOnly);
        QComboBox *sortCombo = stockTab->findChild<QComboBox*>(QStringLiteral("stockSortCombo"), Qt::FindDirectChildrenOnly);
        QTableView *table = stockTab->findChild<QTableView*>(QStringLiteral("tableStockView"), Qt::FindDirectChildrenOnly);
        QPushButton *deleteBtn = stockTab->findChild<QPushButton*>(QStringLiteral("btnSupprimer_Stock"), Qt::FindDirectChildrenOnly);
        QPushButton *refreshBtn = stockTab->findChild<QPushButton*>(QStringLiteral("btnRefresh"), Qt::FindDirectChildrenOnly);
        QFrame *chatFrame = stockTab->findChild<QFrame*>(QStringLiteral("chatbot_frame"), Qt::FindDirectChildrenOnly);
        QPushButton *chatBtn = stockTab->findChild<QPushButton*>(QStringLiteral("chatbot_button"), Qt::FindDirectChildrenOnly);

        if (form) {
            form->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
            root->addWidget(form);
        }

        auto *filtersRow = new QHBoxLayout();
        filtersRow->setSpacing(8);
        if (searchEdit) {
            searchEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
            filtersRow->addWidget(searchEdit, 1);
        }
        filtersRow->addStretch(1);
        if (sortLabel) filtersRow->addWidget(sortLabel);
        if (sortCombo) {
            sortCombo->setMinimumWidth(170);
            filtersRow->addWidget(sortCombo);
        }
        root->addLayout(filtersRow);

        auto *contentRow = new QHBoxLayout();
        contentRow->setSpacing(10);
        if (table) {
            table->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
            table->setMinimumHeight(280);
            contentRow->addWidget(table, 1);
        }
        if (chatFrame) {
            chatFrame->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
            chatFrame->setMinimumWidth(220);
            contentRow->addWidget(chatFrame);
        }
        root->addLayout(contentRow, 1);

        auto *actionsRow = new QHBoxLayout();
        actionsRow->setSpacing(8);
        if (deleteBtn) actionsRow->addWidget(deleteBtn);
        if (refreshBtn) actionsRow->addWidget(refreshBtn);
        actionsRow->addStretch(1);
        if (chatBtn) {
            chatBtn->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
            actionsRow->addWidget(chatBtn);
        }
        root->addLayout(actionsRow);
    }
}

void MainWindow::setupResponsiveModuleLayouts()
{
    if (!ui) return;

    if (ui->horizontalLayout) {
        ui->horizontalLayout->setStretch(0, 0);
        ui->horizontalLayout->setStretch(1, 1);
    }
    if (ui->stackedWidget) {
        ui->stackedWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    }
    if (ui->stackedWidgetEmployes) {
        ui->stackedWidgetEmployes->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    }

    if (ui->pageBateaux) {
        QWidget *mainContent = ui->pageBateaux->findChild<QWidget*>(QStringLiteral("mainContentArea"), Qt::FindDirectChildrenOnly);
        if (mainContent) {
            if (!ui->pageBateaux->layout()) {
                auto *root = new QVBoxLayout(ui->pageBateaux);
                root->setContentsMargins(20, 0, 20, 0);
                root->setSpacing(0);
                root->addWidget(mainContent, 1);
            }
            mainContent->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        }
    }

    QWidget *emplStats = ui->stackedWidgetEmployes
        ? ui->stackedWidgetEmployes->findChild<QWidget*>(QStringLiteral("subPageEmpl4"), Qt::FindDirectChildrenOnly)
        : nullptr;
    if (emplStats && !emplStats->layout()) {
        auto *root = new QVBoxLayout(emplStats);
        root->setContentsMargins(18, 18, 18, 18);
        root->setSpacing(12);

        QLabel *title = nullptr;
        const auto labels = emplStats->findChildren<QLabel*>(QString(), Qt::FindDirectChildrenOnly);
        for (QLabel *lbl : labels) {
            if (lbl && lbl->text().contains(QStringLiteral("Statistiques"), Qt::CaseInsensitive)) {
                title = lbl;
                break;
            }
        }
        if (title) {
            title->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
            root->addWidget(title);
        }

        QFrame *frameCard1 = emplStats->findChild<QFrame*>(QStringLiteral("frameCard1"), Qt::FindDirectChildrenOnly);
        QFrame *frameCard2 = emplStats->findChild<QFrame*>(QStringLiteral("frameCard2"), Qt::FindDirectChildrenOnly);
        QFrame *frameCard3 = emplStats->findChild<QFrame*>(QStringLiteral("frameCard3"), Qt::FindDirectChildrenOnly);
        QFrame *frameCard4 = emplStats->findChild<QFrame*>(QStringLiteral("frameCard4"), Qt::FindDirectChildrenOnly);
        QFrame *frameCard5 = emplStats->findChild<QFrame*>(QStringLiteral("frameCard5"), Qt::FindDirectChildrenOnly);
        auto *cardsRow = new QGridLayout();
        cardsRow->setHorizontalSpacing(10);
        cardsRow->setVerticalSpacing(10);
        if (frameCard1) cardsRow->addWidget(frameCard1, 0, 0);
        if (frameCard2) cardsRow->addWidget(frameCard2, 0, 1);
        if (frameCard3) cardsRow->addWidget(frameCard3, 0, 2);
        if (frameCard4) cardsRow->addWidget(frameCard4, 0, 3);
        if (frameCard5) cardsRow->addWidget(frameCard5, 0, 4);
        cardsRow->setColumnStretch(0, 1);
        cardsRow->setColumnStretch(1, 1);
        cardsRow->setColumnStretch(2, 1);
        cardsRow->setColumnStretch(3, 1);
        cardsRow->setColumnStretch(4, 1);
        root->addLayout(cardsRow);

        QFrame *frameDepartments = emplStats->findChild<QFrame*>(QStringLiteral("frameDepartments"), Qt::FindDirectChildrenOnly);
        QFrame *frameSkills = emplStats->findChild<QFrame*>(QStringLiteral("frameSkills"), Qt::FindDirectChildrenOnly);
        QFrame *frameShifts = emplStats->findChild<QFrame*>(QStringLiteral("frameShifts"), Qt::FindDirectChildrenOnly);
        QFrame *frameCities = emplStats->findChild<QFrame*>(QStringLiteral("frameCities"), Qt::FindDirectChildrenOnly);
        auto *middleGrid = new QGridLayout();
        middleGrid->setHorizontalSpacing(10);
        middleGrid->setVerticalSpacing(10);
        if (frameDepartments) middleGrid->addWidget(frameDepartments, 0, 0);
        if (frameSkills) middleGrid->addWidget(frameSkills, 0, 1);
        if (frameShifts) middleGrid->addWidget(frameShifts, 1, 0);
        if (frameCities) middleGrid->addWidget(frameCities, 1, 1);
        middleGrid->setColumnStretch(0, 1);
        middleGrid->setColumnStretch(1, 1);
        middleGrid->setRowStretch(0, 1);
        middleGrid->setRowStretch(1, 1);
        root->addLayout(middleGrid, 1);

        QFrame *frameAlerts = emplStats->findChild<QFrame*>(QStringLiteral("frameAlerts"), Qt::FindDirectChildrenOnly);
        if (frameAlerts) {
            frameAlerts->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
            root->addWidget(frameAlerts);
        }
    }
}

// ==================== SLOTS GÉNÉRAUX / NAVIGATION ====================
void MainWindow::on_btnEmployes_clicked() {
    ui->stackedWidget->setCurrentIndex(0);
}
void MainWindow::on_btnPecheurs_clicked() { ui->stackedWidget->setCurrentIndex(1); }
void MainWindow::on_btnBateaux_clicked() {
    ui->stackedWidget->setCurrentIndex(2);
    if (m_bateauModule)
        m_bateauModule->reloadFromDatabase();
}
void MainWindow::on_btnMaintenance_clicked() {
    ui->stackedWidget->setCurrentIndex(3);
    afficherEquipements();
    updateStatistics();
    updateMaintenanceUI();
}
void MainWindow::on_btnQuais_clicked() {
    // Do not delete m_bateauModule here: it owns the model for tableBateaux_2. Destroying it while
    // the table still references that model causes dangling pointers and crashes when the UI repaints.
    if (!ui || !ui->stackedWidget)
        return;
    setupQuaiMapOverviewUi();
    setupQuaiCharts();
    loadQuaisTable();
    // Show the page after the current event slice: painting Qt Charts while the stack is still
    // on another page has caused native crashes on some Windows/GPU setups.
    QTimer::singleShot(0, this, [this]() {
        if (ui && ui->stackedWidget)
            ui->stackedWidget->setCurrentIndex(4);
    });
}

void MainWindow::on_mainTabWidget_currentChanged(int index) {
    if (m_quaiMapTab && ui && ui->mainTabWidget && index == ui->mainTabWidget->indexOf(m_quaiMapTab)) {
        refreshQuaiMapOverview();
        return;
    }
    if (m_quaiArduinoTab && ui && ui->mainTabWidget && index == ui->mainTabWidget->indexOf(m_quaiArduinoTab)) {
        if (m_quaiArduinoWindow)
            m_quaiArduinoWindow->refreshActiveQuaisCombo();
        return;
    }
    if (index == 1) {
        setupQuaiCharts();
        refreshQuaiCharts();
        refreshQuaiSupervisionData();
        if (m_quaiChartView) {
            m_quaiChartView->update();
            m_quaiChartView->setVisible(true);
        }
    }
}
void MainWindow::on_btnStock_clicked() { ui->mainStackedWidget->setCurrentIndex(1); }  // Navigate to login page
void MainWindow::on_btnStock_2_clicked() {
    ui->stackedWidget->setCurrentIndex(5); // Stock page
}

// ==================== MODULE EMPLOYE ====================
static void syncEmployesSubNavChecked(Ui::MainWindow *ui, int index) {
    if (!ui) return;
    if (ui->btnSubEmpl3) ui->btnSubEmpl3->setChecked(index == 0);
    if (ui->btnSubEmpl4_2) ui->btnSubEmpl4_2->setChecked(index == 1);
    if (ui->btnSubEmpl5) ui->btnSubEmpl5->setChecked(index == 2);
    if (ui->btnSubEmplSmartPort) ui->btnSubEmplSmartPort->setChecked(index == 3);
}

void MainWindow::on_btnSubEmpl1_clicked() { ui->stackedWidgetEmployes->setCurrentIndex(0); syncEmployesSubNavChecked(ui, 0); }
void MainWindow::on_btnSubEmpl2_clicked() {
    ui->stackedWidgetEmployes->setCurrentIndex(1);
    syncEmployesSubNavChecked(ui, 1);
    updateEmployeeStats();
}
void MainWindow::on_btnSubEmpl3_clicked() {
    ui->stackedWidgetEmployes->setCurrentIndex(0);
    syncEmployesSubNavChecked(ui, 0);
}

static QString generateSecurePassword(int length) {
    if (length < 12) length = 12;
    const QString lowers = "abcdefghijklmnopqrstuvwxyz";
    const QString uppers = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    const QString digits = "0123456789";
    const QString symbols = "!@#$%&*_-+=?";
    const QString all = lowers + uppers + digits + symbols;
    QString pwd;
    pwd.reserve(length);
    auto pick = [](const QString &pool) -> QChar {
        return pool.at(int(QRandomGenerator::global()->bounded(quint32(pool.size()))));
    };
    pwd.append(pick(lowers));
    pwd.append(pick(uppers));
    pwd.append(pick(digits));
    pwd.append(pick(symbols));
    while (pwd.size() < length) pwd.append(pick(all));
    for (int i = pwd.size() - 1; i > 0; --i) {
        const int j = int(QRandomGenerator::global()->bounded(quint32(i + 1)));
        const QChar tmp = pwd.at(i);
        pwd[i] = pwd.at(j);
        pwd[j] = tmp;
    }
    return pwd;
}

void MainWindow::on_btnGenPwd_clicked() {
    if (!ui->lineEdit_pwd) return;
    ui->lineEdit_pwd->setText(generateSecurePassword(14));
    ui->lineEdit_pwd->setFocus();
    ui->lineEdit_pwd->selectAll();
    validateEmployeeForm(nullptr, true);
}

void MainWindow::on_btnPwdEye_toggled(bool checked) {
    if (!ui->lineEdit_pwd) return;
    ui->lineEdit_pwd->setEchoMode(checked ? QLineEdit::Normal : QLineEdit::Password);
    if (ui->btnPwdEye) ui->btnPwdEye->setText(checked ? "C" : "V");
}

void MainWindow::on_btnGenCode_clicked() {
    if (!ui->lineEdit_code) return;
    QString code;
    code.reserve(8);
    for (int i = 0; i < 8; ++i)
        code.append(QChar(u'0' + QRandomGenerator::global()->bounded(10)));
    ui->lineEdit_code->setText(code);
    ui->lineEdit_code->setFocus();
    ui->lineEdit_code->selectAll();
    validateEmployeeForm(nullptr, true);
}

void MainWindow::on_btnCodeEye_toggled(bool checked) {
    if (!ui->lineEdit_code) return;
    ui->lineEdit_code->setEchoMode(checked ? QLineEdit::Normal : QLineEdit::Password);
    if (ui->btnCodeEye) ui->btnCodeEye->setText(checked ? "C" : "V");
}

void MainWindow::on_btnParametres_clicked() {
    openSettingsDialog();
}

void MainWindow::openSettingsDialog() {
    if (m_settingsDialog) {
        m_settingsDialog->deleteLater();
        m_settingsDialog = nullptr;
    }
    m_settingsDialog = new QDialog(this);
    m_settingsDialog->setWindowTitle(tr("Paramètres"));
    m_settingsDialog->setWindowFlags(m_settingsDialog->windowFlags() & ~Qt::WindowContextHelpButtonHint);
    m_settingsDialog->setMinimumWidth(320);
    m_settingsDialog->setStyleSheet(
        "QDialog { background-color: #2d2d2d; color: #ffffff; }\n"
        "QPushButton { background-color: #3d3d3d; color: white; border-radius: 8px; min-width: 80px; min-height: 56px; }\n"
        "QPushButton:hover { background-color: #505050; border: 2px solid #3498db; }\n"
        "QPushButton:checked { background-color: #0d47a1; }\n"
    );

    QVBoxLayout *mainLayout = new QVBoxLayout(m_settingsDialog);

    QHBoxLayout *row = new QHBoxLayout();
    row->setSpacing(16);

    QIcon volIcon(QStringLiteral(":/new/prefix1/images/volume.png"));
    QIcon muteIcon(QStringLiteral(":/new/prefix1/images/mute.png"));
    if (volIcon.isNull()) volIcon = style()->standardIcon(QStyle::SP_MediaVolume);
    if (muteIcon.isNull()) muteIcon = style()->standardIcon(QStyle::SP_MediaVolumeMuted);
    QPushButton *btnMusic = new QPushButton(m_settingsDialog);
    btnMusic->setCheckable(true);
    btnMusic->setChecked(m_backgroundMusic && m_backgroundMusic->playbackState() != QMediaPlayer::PlayingState);
    btnMusic->setIcon(btnMusic->isChecked() ? muteIcon : volIcon);
    btnMusic->setIconSize(QSize(28, 28));
    btnMusic->setToolTip(btnMusic->isChecked() ? tr("Musique OFF") : tr("Musique ON"));
    btnMusic->setText(tr("Musique"));
    btnMusic->setFixedSize(100, 56);
    btnMusic->setCursor(Qt::PointingHandCursor);
    row->addWidget(btnMusic);

    connect(btnMusic, &QPushButton::toggled, this, [this, btnMusic, volIcon, muteIcon](bool checked) {
        on_backgroundMusic_toggled(checked);
        btnMusic->setIcon(checked ? muteIcon : volIcon);
        btnMusic->setToolTip(checked ? tr("Musique OFF") : tr("Musique ON"));
    });

    QIcon iconNormal(QStringLiteral(":/new/prefix1/images/employes (1).png"));
    QIcon iconDark(QStringLiteral(":/new/prefix1/images/employes.png"));
    if (iconNormal.isNull()) iconNormal = style()->standardIcon(QStyle::SP_ComputerIcon);
    if (iconDark.isNull()) iconDark = style()->standardIcon(QStyle::SP_DesktopIcon);
    QPushButton *btnMode = new QPushButton(m_settingsDialog);
    btnMode->setCheckable(true);
    btnMode->setChecked(m_darkMode);
    btnMode->setIcon(m_darkMode ? iconDark : iconNormal);
    btnMode->setIconSize(QSize(28, 28));
    btnMode->setToolTip(m_darkMode ? tr("Mode sombre") : tr("Mode normal"));
    btnMode->setText(tr("Mode sombre"));
    btnMode->setFixedSize(120, 56);
    btnMode->setCursor(Qt::PointingHandCursor);
    row->addWidget(btnMode);

    connect(btnMode, &QPushButton::toggled, this, [this, btnMode, iconNormal, iconDark](bool checked) {
        applyTheme(checked);
        btnMode->setIcon(checked ? iconDark : iconNormal);
        btnMode->setToolTip(checked ? tr("Mode sombre") : tr("Mode normal"));
    });

    mainLayout->addLayout(row);
    m_settingsDialog->adjustSize();
    m_settingsDialog->exec();
}

void MainWindow::on_backgroundMusic_toggled(bool checked) {
    if (!m_backgroundMusic) return;
    if (checked)
        m_backgroundMusic->pause();
    else
        m_backgroundMusic->play();
}

void MainWindow::applyQuaiInputPalette(bool darkMode)
{
    QPalette p;
    if (darkMode) {
        p.setColor(QPalette::Base, QColor(42, 42, 42));
        p.setColor(QPalette::Text, QColor(238, 238, 238));
        p.setColor(QPalette::PlaceholderText, QColor(170, 170, 170));
    } else {
        p.setColor(QPalette::Base, Qt::white);
        p.setColor(QPalette::Text, QColor(43, 43, 43));
        p.setColor(QPalette::PlaceholderText, QColor(120, 120, 120));
    }

    QWidget *inputs[] = {
        ui->lineEditQuaiID, ui->lineEditNom,
        ui->doubleSpinBoxOccupation, ui->spinBoxCurrentUsage,
        ui->doubleSpinBoxLongueur, ui->doubleSpinBoxProfondeur, ui->spinBoxCapacite,
        ui->lineEditSearch
    };
    for (QWidget *w : inputs) {
        if (w) {
            w->setAutoFillBackground(true);
            w->setPalette(p);
        }
    }
}

void MainWindow::applyEmployesModuleTheme(bool darkMode)
{
    if (!ui)
        return;

    const QString shellDark = QStringLiteral(
        "QWidget#pageEmployes { background-color: #2d2d2d; }\n"
        "QFrame#frame_3 { background-color: #0d0d0d; border: 2px solid #333333; border-radius: 8px; }\n"
        "QFrame#subNavBar { background-color: #1e5f74; border: none; }\n"
        "QFrame#subNavBar QPushButton { background-color: #2a9d8f; color: #ffffff; border: none; padding: 8px 15px; "
        "border-radius: 5px; font-weight: bold; }\n"
        "QFrame#subNavBar QPushButton:hover { background-color: #21867a; }\n"
        "QFrame#subNavBar QPushButton:checked { background-color: #3498db; color: #ffffff; }\n"
        "QFrame#subNavBar QToolButton { background: transparent; border: none; }\n");
    const QString shellLight = QStringLiteral(
        "QWidget#pageEmployes { background-color: #f0f4f8; }\n"
        "QFrame#frame_3 { background-color: #ffffff; border: 2px solid #cbd5e1; border-radius: 8px; }\n"
        "QFrame#subNavBar { background-color: #e2e8f0; border: none; }\n"
        "QFrame#subNavBar QPushButton { background-color: #2a9d8f; color: #ffffff; border: none; padding: 8px 15px; "
        "border-radius: 5px; font-weight: bold; }\n"
        "QFrame#subNavBar QPushButton:hover { background-color: #21867a; }\n"
        "QFrame#subNavBar QPushButton:checked { background-color: #1e5f74; color: #ffffff; }\n"
        "QFrame#subNavBar QToolButton { background: transparent; border: none; }\n");

    const QString infoDark = QStringLiteral(
        "QFrame { background-color: #232323; border-radius: 8px; border: 2px solid #3498db; }\n"
        "QLabel { color: #ffffff; font-family: \"Segoe UI\"; font-weight: bold; font-size: 12px; padding: 2px; border: none; }\n"
        "QLineEdit, QComboBox, QDateEdit { color: white; background-color: #2d2d2d; border: 1px solid #3d3d3d; padding: 5px; "
        "border-radius: 4px; font-family: \"Segoe UI\"; font-weight: bold; }\n"
        "QLineEdit[invalid=\"true\"], QComboBox[invalid=\"true\"], QDateEdit[invalid=\"true\"] { border: 2px solid #3498db; }\n"
        "QRadioButton, QCheckBox { color: #ffffff; }\n"
        "QTabWidget::pane { border: 1px solid #3498db; background-color: #232323; border-radius: 5px; }\n"
        "QTabBar::tab { background: #2d2d2d; color: #ffffff; padding: 10px 14px; border-top-left-radius: 5px; "
        "border-top-right-radius: 5px; border: 1px solid #3d3d3d; min-width: 140px; }\n"
        "QTabBar::tab:selected { background: #3498db; color: #ffffff; }\n");
    const QString infoLight = QStringLiteral(
        "QFrame { background-color: #f8fafc; border-radius: 8px; border: 2px solid #2a9d8f; }\n"
        "QLabel { color: #0f172a; font-family: \"Segoe UI\"; font-weight: bold; font-size: 12px; padding: 2px; border: none; }\n"
        "QLineEdit, QComboBox, QDateEdit { color: #0f172a; background-color: #ffffff; border: 1px solid #cbd5e1; padding: 5px; "
        "border-radius: 4px; font-family: \"Segoe UI\"; font-weight: bold; }\n"
        "QLineEdit[invalid=\"true\"], QComboBox[invalid=\"true\"], QDateEdit[invalid=\"true\"] { border: 2px solid #e11d48; }\n"
        "QRadioButton, QCheckBox { color: #0f172a; }\n"
        "QTabWidget::pane { border: 1px solid #2a9d8f; background-color: #f8fafc; border-radius: 5px; }\n"
        "QTabBar::tab { background: #e2e8f0; color: #0f172a; padding: 10px 14px; border-top-left-radius: 5px; "
        "border-top-right-radius: 5px; border: 1px solid #cbd5e1; min-width: 140px; }\n"
        "QTabBar::tab:selected { background: #2a9d8f; color: #ffffff; }\n");

    const QString funcDark = QStringLiteral(
        "background-color: #2c3e50;\n"
        "border-bottom-left-radius: 10px;\n"
        "border-bottom-right-radius: 10px;\n"
        "border: 2px solid #3498db;");
    const QString funcLight = QStringLiteral(
        "background-color: #e2e8f0;\n"
        "border-bottom-left-radius: 10px;\n"
        "border-bottom-right-radius: 10px;\n"
        "border: 2px solid #2a9d8f;");

    const QString tableListeDark = QStringLiteral(
        "QTableWidget {\n"
        "    background-color: #2d2d2d;\n"
        "    gridline-color: #3d3d3d;\n"
        "    font-family: \"Segoe UI\";\n"
        "    font-size: 12px;\n"
        "    font-weight: bold;\n"
        "    selection-background-color: #3498db;\n"
        "    border-radius: 8px;\n"
        "    border: 2px solid #3498db;\n"
        "    color: white;\n"
        "}\n"
        "QHeaderView::section {\n"
        "    background-color: #3d3d3d;\n"
        "    color: white;\n"
        "    padding: 6px;\n"
        "    border: 1px solid #232323;\n"
        "    font-weight: bold;\n"
        "    font-family: \"Segoe UI\";\n"
        "}\n");
    const QString tableListeLight = QStringLiteral(
        "QTableWidget {\n"
        "    background-color: #ffffff;\n"
        "    gridline-color: #cbd5e1;\n"
        "    font-family: \"Segoe UI\";\n"
        "    font-size: 12px;\n"
        "    font-weight: bold;\n"
        "    selection-background-color: #2a9d8f;\n"
        "    border-radius: 8px;\n"
        "    border: 2px solid #2a9d8f;\n"
        "    color: #0f172a;\n"
        "}\n"
        "QHeaderView::section {\n"
        "    background-color: #1e5f74;\n"
        "    color: #ffffff;\n"
        "    padding: 6px;\n"
        "    border: 1px solid #cbd5e1;\n"
        "    font-weight: bold;\n"
        "    font-family: \"Segoe UI\";\n"
        "}\n");

    const QString gbDark = QStringLiteral(
        "QGroupBox { border: 1px solid #3498db; border-radius: 6px; margin-top: 14px; }\n"
        "QGroupBox::title { subcontrol-origin: margin; left: 10px; padding: 0 4px; color: #ffffff; }\n");
    const QString gbLight = QStringLiteral(
        "QGroupBox { border: 1px solid #2a9d8f; border-radius: 6px; margin-top: 14px; background-color: #ffffff; }\n"
        "QGroupBox::title { subcontrol-origin: margin; left: 10px; padding: 0 4px; color: #0f172a; }\n");

    const QString photoDark = QStringLiteral("border: 2px dashed #3498db; color: #7f8c8d;");
    const QString photoLight = QStringLiteral("border: 2px dashed #2a9d8f; color: #64748b;");

    const QString statsPageDark = QStringLiteral("QWidget#subPageEmpl4 { background-color: #1a1f26; }");
    const QString statsPageLight = QStringLiteral("QWidget#subPageEmpl4 { background-color: #f0f4f8; }");

    const QString statsFooterDark = QStringLiteral("color: rgba(255,255,255,0.88); background: transparent;");
    const QString statsFooterLight = QStringLiteral("color: #475569; background: transparent;");

    const QString dispoPageDark = QStringLiteral(
        "QWidget#subPageEmpl5 {\n"
        "    background: qlineargradient(x1:0, y1:0, x2:1, y2:1,\n"
        "        stop:0 #0b1220, stop:0.5 #111827, stop:1 #0f172a);\n"
        "}\n");
    const QString dispoPageLight = QStringLiteral(
        "QWidget#subPageEmpl5 {\n"
        "    background: qlineargradient(x1:0, y1:0, x2:1, y2:1,\n"
        "        stop:0 #f0f9ff, stop:0.5 #e0f2fe, stop:1 #f8fafc);\n"
        "}\n");

    const QString dispoTitleDark = QStringLiteral(
        "QLabel#lblAdvancedMetierTitle {\n"
        "    color: #f8fafc;\n"
        "    border: 1px solid rgba(45, 212, 191, 0.5);\n"
        "    border-radius: 14px;\n"
        "    padding: 8px 28px;\n"
        "    background-image: url(:/new/prefix1/15.jpg);\n"
        "    background-position: center;\n"
        "    background-repeat: no-repeat;\n"
        "    background-color: rgba(15, 23, 42, 0.58);\n"
        "}\n");
    const QString dispoTitleLight = QStringLiteral(
        "QLabel#lblAdvancedMetierTitle {\n"
        "    color: #f8fafc;\n"
        "    border: 1px solid rgba(45, 212, 191, 0.55);\n"
        "    border-radius: 14px;\n"
        "    padding: 8px 28px;\n"
        "    background-image: url(:/new/prefix1/15.jpg);\n"
        "    background-position: center;\n"
        "    background-repeat: no-repeat;\n"
        "    background-color: rgba(15, 23, 42, 0.55);\n"
        "}\n");

    const QString dispoSideDark = QStringLiteral("color: #94a3b8; background: transparent; letter-spacing: 0.6px;");
    const QString dispoSideLight = QStringLiteral("color: #475569; background: transparent; letter-spacing: 0.6px;");

    const QString frameDetailDark = QStringLiteral(
        "QFrame#frameRoleDetail {\n"
        "    background-color: rgba(30, 41, 59, 0.65);\n"
        "    border: 1px solid rgba(45, 212, 191, 0.28);\n"
        "    border-radius: 16px;\n"
        "}\n");
    const QString frameDetailLight = QStringLiteral(
        "QFrame#frameRoleDetail {\n"
        "    background-color: rgba(255, 255, 255, 0.95);\n"
        "    border: 1px solid #2a9d8f;\n"
        "    border-radius: 16px;\n"
        "}\n");

    const QString lblMonDark = QStringLiteral(
        "color: #e2e8f0;\n"
        "background-color: rgba(15, 23, 42, 0.72);\n"
        "border: 1px solid rgba(45, 212, 191, 0.22);\n"
        "border-radius: 10px;\n"
        "padding: 6px 12px;");
    const QString lblMonLight = QStringLiteral(
        "color: #1e293b;\n"
        "background-color: #f1f5f9;\n"
        "border: 1px solid #cbd5e1;\n"
        "border-radius: 10px;\n"
        "padding: 6px 12px;");

    const QString autoTitleDark = QStringLiteral(
        "color: #5eead4; background: transparent;\n"
        "padding-left: 12px;\n"
        "border-left: 3px solid #2dd4bf;");
    const QString autoTitleLight = QStringLiteral(
        "color: #0f766e; background: transparent;\n"
        "padding-left: 12px;\n"
        "border-left: 3px solid #2a9d8f;");
    const QString manualTitleDark = QStringLiteral(
        "color: #a5f3fc; background: transparent;\n"
        "padding-left: 12px;\n"
        "border-left: 3px solid #22d3ee;");
    const QString manualTitleLight = QStringLiteral(
        "color: #155e75; background: transparent;\n"
        "padding-left: 12px;\n"
        "border-left: 3px solid #0ea5e9;");

    const QString tblAutoDark = QStringLiteral(
        "QTableWidget {\n"
        "    background-color: rgba(15, 23, 42, 0.85);\n"
        "    color: #f1f5f9;\n"
        "    border: 1px solid rgba(45, 212, 191, 0.22);\n"
        "    border-radius: 10px;\n"
        "    gridline-color: rgba(51, 65, 85, 0.7);\n"
        "    font-size: 12px;\n"
        "    alternate-background-color: rgba(30, 41, 59, 0.65);\n"
        "}\n"
        "QHeaderView::section {\n"
        "    background-color: #134e4a;\n"
        "    color: #ecfdf5;\n"
        "    padding: 10px 6px;\n"
        "    font-weight: bold;\n"
        "    border: none;\n"
        "    border-bottom: 2px solid #2dd4bf;\n"
        "}\n"
        "QTableWidget::item { padding: 8px 6px; }\n"
        "QTableWidget::item:selected { background-color: #0f766e; color: #ffffff; }\n"
        "QTableWidget::item:hover:!selected { background-color: rgba(45, 212, 191, 0.1); }\n");
    const QString tblAutoLight = QStringLiteral(
        "QTableWidget {\n"
        "    background-color: #ffffff;\n"
        "    color: #0f172a;\n"
        "    border: 1px solid #2a9d8f;\n"
        "    border-radius: 10px;\n"
        "    gridline-color: #e2e8f0;\n"
        "    font-size: 12px;\n"
        "    alternate-background-color: #f8fafc;\n"
        "}\n"
        "QHeaderView::section {\n"
        "    background-color: #1e5f74;\n"
        "    color: #ffffff;\n"
        "    padding: 10px 6px;\n"
        "    font-weight: bold;\n"
        "    border: none;\n"
        "    border-bottom: 2px solid #2a9d8f;\n"
        "}\n"
        "QTableWidget::item { padding: 8px 6px; }\n"
        "QTableWidget::item:selected { background-color: #2a9d8f; color: #ffffff; }\n"
        "QTableWidget::item:hover:!selected { background-color: rgba(42, 157, 143, 0.12); }\n");

    const QString tblManDark = QStringLiteral(
        "QTableWidget {\n"
        "    background-color: rgba(15, 23, 42, 0.85);\n"
        "    color: #f1f5f9;\n"
        "    border: 1px solid rgba(34, 211, 238, 0.2);\n"
        "    border-radius: 10px;\n"
        "    gridline-color: rgba(51, 65, 85, 0.7);\n"
        "    font-size: 12px;\n"
        "    alternate-background-color: rgba(30, 41, 59, 0.65);\n"
        "}\n"
        "QHeaderView::section {\n"
        "    background-color: #155e75;\n"
        "    color: #ecfeff;\n"
        "    padding: 10px 6px;\n"
        "    font-weight: bold;\n"
        "    border: none;\n"
        "    border-bottom: 2px solid #22d3ee;\n"
        "}\n"
        "QTableWidget::item { padding: 8px 6px; }\n"
        "QTableWidget::item:selected { background-color: #0e7490; color: #ffffff; }\n"
        "QTableWidget::item:hover:!selected { background-color: rgba(34, 211, 238, 0.1); }\n");
    const QString tblManLight = QStringLiteral(
        "QTableWidget {\n"
        "    background-color: #ffffff;\n"
        "    color: #0f172a;\n"
        "    border: 1px solid #0ea5e9;\n"
        "    border-radius: 10px;\n"
        "    gridline-color: #e2e8f0;\n"
        "    font-size: 12px;\n"
        "    alternate-background-color: #f8fafc;\n"
        "}\n"
        "QHeaderView::section {\n"
        "    background-color: #155e75;\n"
        "    color: #ecfeff;\n"
        "    padding: 10px 6px;\n"
        "    font-weight: bold;\n"
        "    border: none;\n"
        "    border-bottom: 2px solid #0ea5e9;\n"
        "}\n"
        "QTableWidget::item { padding: 8px 6px; }\n"
        "QTableWidget::item:selected { background-color: #0e7490; color: #ffffff; }\n"
        "QTableWidget::item:hover:!selected { background-color: rgba(14, 165, 233, 0.12); }\n");

    const QString tblDispoDark = QStringLiteral(
        "QTableWidget {\n"
        "    background-color: rgba(30, 41, 59, 0.9);\n"
        "    color: #e2e8f0;\n"
        "    border: 1px solid rgba(45, 212, 191, 0.25);\n"
        "    border-radius: 12px;\n"
        "    gridline-color: rgba(71, 85, 105, 0.6);\n"
        "    font-size: 13px;\n"
        "}\n"
        "QHeaderView::section {\n"
        "    background-color: #134e4a;\n"
        "    color: #ecfdf5;\n"
        "    padding: 10px 8px;\n"
        "    font-weight: bold;\n"
        "    border: none;\n"
        "    border-bottom: 2px solid #2dd4bf;\n"
        "}\n"
        "QTableWidget::item { padding: 8px 6px; }\n"
        "QTableWidget::item:selected { background-color: #0f766e; color: #ffffff; }\n"
        "QTableWidget::item:hover:!selected { background-color: rgba(45, 212, 191, 0.12); }\n");
    const QString tblDispoLight = QStringLiteral(
        "QTableWidget {\n"
        "    background-color: #ffffff;\n"
        "    color: #0f172a;\n"
        "    border: 1px solid #2a9d8f;\n"
        "    border-radius: 12px;\n"
        "    gridline-color: #e2e8f0;\n"
        "    font-size: 13px;\n"
        "}\n"
        "QHeaderView::section {\n"
        "    background-color: #1e5f74;\n"
        "    color: #ffffff;\n"
        "    padding: 10px 8px;\n"
        "    font-weight: bold;\n"
        "    border: none;\n"
        "    border-bottom: 2px solid #2a9d8f;\n"
        "}\n"
        "QTableWidget::item { padding: 8px 6px; }\n"
        "QTableWidget::item:selected { background-color: #2a9d8f; color: #ffffff; }\n"
        "QTableWidget::item:hover:!selected { background-color: rgba(42, 157, 143, 0.12); }\n");

    const QString listRolesDark = QStringLiteral(
        "QListWidget {\n"
        "    background-color: #2c3e50;\n"
        "    color: white;\n"
        "    border-radius: 10px;\n"
        "    padding: 5px;\n"
        "}\n"
        "QListWidget::item { height: 40px; border-bottom: 1px solid #34495e; }\n"
        "QListWidget::item:selected { background-color: #3498db; border-radius: 5px; }\n");
    const QString listRolesLight = QStringLiteral(
        "QListWidget {\n"
        "    background-color: #ffffff;\n"
        "    color: #0f172a;\n"
        "    border: 1px solid #cbd5e1;\n"
        "    border-radius: 10px;\n"
        "    padding: 5px;\n"
        "}\n"
        "QListWidget::item { height: 40px; border-bottom: 1px solid #e2e8f0; }\n"
        "QListWidget::item:selected { background-color: #2a9d8f; color: #ffffff; border-radius: 5px; }\n");

    if (ui->pageEmployes)
        ui->pageEmployes->setStyleSheet(darkMode ? shellDark : shellLight);
    if (ui->info_frame_2)
        ui->info_frame_2->setStyleSheet(darkMode ? infoDark : infoLight);
    if (ui->function_frame)
        ui->function_frame->setStyleSheet(darkMode ? funcDark : funcLight);
    if (ui->tableWidget_2)
        ui->tableWidget_2->setStyleSheet(darkMode ? tableListeDark : tableListeLight);

    for (QGroupBox *gb : { ui->groupBoxIdentification, ui->groupBoxInfoPersonnelle, ui->groupBoxCoordonnees, ui->groupBoxCertification }) {
        if (gb)
            gb->setStyleSheet(darkMode ? gbDark : gbLight);
    }
    if (ui->label_photo)
        ui->label_photo->setStyleSheet(darkMode ? photoDark : photoLight);

    if (ui->subPageEmpl4)
        ui->subPageEmpl4->setStyleSheet(darkMode ? statsPageDark : statsPageLight);
    if (ui->lblStatsLastUpdate)
        ui->lblStatsLastUpdate->setStyleSheet(darkMode ? statsFooterDark : statsFooterLight);

    if (ui->subPageEmpl5)
        ui->subPageEmpl5->setStyleSheet(darkMode ? dispoPageDark : dispoPageLight);
    if (ui->lblAdvancedMetierTitle)
        ui->lblAdvancedMetierTitle->setStyleSheet(darkMode ? dispoTitleDark : dispoTitleLight);
    if (QLabel *lblMonitorSidebarTitle = this->findChild<QLabel*>(QStringLiteral("lblMonitorSidebarTitle")))
        lblMonitorSidebarTitle->setStyleSheet(darkMode ? dispoSideDark : dispoSideLight);
    if (ui->frameRoleDetail) {
        ui->frameRoleDetail->setStyleSheet(darkMode ? frameDetailDark : frameDetailLight);
    }
    if (ui->lblMonitorAssignments)
        ui->lblMonitorAssignments->setStyleSheet(darkMode ? lblMonDark : lblMonLight);
    if (ui->lblAutoTitle)
        ui->lblAutoTitle->setStyleSheet(darkMode ? autoTitleDark : autoTitleLight);
    if (ui->lblManualTitle)
        ui->lblManualTitle->setStyleSheet(darkMode ? manualTitleDark : manualTitleLight);
    if (ui->tableRoleAutoSuggestions)
        ui->tableRoleAutoSuggestions->setStyleSheet(darkMode ? tblAutoDark : tblAutoLight);
    if (ui->tableRoleManualSelection)
        ui->tableRoleManualSelection->setStyleSheet(darkMode ? tblManDark : tblManLight);
    if (ui->tableWidget)
        ui->tableWidget->setStyleSheet(darkMode ? tblDispoDark : tblDispoLight);
    if (ui->listRoles)
        ui->listRoles->setStyleSheet(darkMode ? listRolesDark : listRolesLight);

    if (ui->lblRoleStatusText)
        ui->lblRoleStatusText->setStyleSheet(darkMode ? QStringLiteral("color: white;") : QStringLiteral("color: #0f172a;"));
    if (ui->lblRoleStatusIcon)
        ui->lblRoleStatusIcon->setStyleSheet(
            darkMode ? QStringLiteral("color: white; background-color: rgba(255,255,255,0.22); border-radius: 6px;")
                     : QStringLiteral("color: #ffffff; background-color: #2a9d8f; border-radius: 6px;"));
}

void MainWindow::applyTheme(bool darkMode) {
    m_darkMode = darkMode;

    static const QString kDarkMainOverlay = QStringLiteral(
        "\n/* Runtime dark-mode overlay */\n"
        "QMainWindow { background-color: #1f242b; }\n"
        "#centralwidget { background-color: #1f242b; }\n"
        "#sidebar { background-color: #172c3c; border-right: 3px solid #1f8ca1; }\n"
        "#lblTitle { color: #f5f8fa; }\n"
    );

    static const QString kDarkStackedOverlay = QStringLiteral(
        "\n/* Runtime dark-mode overlay */\n"
        "QWidget { color: #e9edf1; }\n"
        "QLineEdit, QComboBox, QDateEdit, QSpinBox, QDoubleSpinBox, QTimeEdit, QPlainTextEdit, QTextEdit {"
        " background-color: #2a2f35; color: #eef2f5; border: 1px solid #4a5b68; }\n"
        "QTableWidget, QTableView { background-color: #242a30; color: #eef2f5; gridline-color: #3d4b56; }\n"
        "QHeaderView::section { background-color: #30414e; color: #eef2f5; }\n"
    );

    this->setStyleSheet(darkMode ? (m_mainWindowBaseStyle + kDarkMainOverlay) : m_mainWindowBaseStyle);
    if (ui->stackedWidget)
        ui->stackedWidget->setStyleSheet(darkMode ? (m_stackedWidgetBaseStyle + kDarkStackedOverlay) : m_stackedWidgetBaseStyle);

    applyEmployesModuleTheme(darkMode);

    applyQuaiInputPalette(darkMode);
}

void MainWindow::on_employeeTableHeaderSectionClicked(int logicalIndex) {
    if (!ui->tableWidget_2) return;
    const int pwdCol = 10;
    const int codeCol = 18;

    if (logicalIndex == pwdCol) {
        m_showPasswordsInList = !m_showPasswordsInList;
        if (auto *hi = ui->tableWidget_2->horizontalHeaderItem(pwdCol))
            hi->setText(QStringLiteral("Mot de passe  %1").arg(m_showPasswordsInList ? "C" : "V"));
        for (int r = 0; r < ui->tableWidget_2->rowCount(); ++r) {
            auto *it = ui->tableWidget_2->item(r, pwdCol);
            if (!it) continue;
            const QString realPwd = it->data(Qt::UserRole + 1).toString();
            if (realPwd.isEmpty()) continue;
            it->setText(m_showPasswordsInList ? realPwd : QStringLiteral("********"));
        }
    } else if (logicalIndex == codeCol) {
        m_showCodesInList = !m_showCodesInList;
        if (auto *hi = ui->tableWidget_2->horizontalHeaderItem(codeCol))
            hi->setText(QStringLiteral("code  %1").arg(m_showCodesInList ? "C" : "V"));
        for (int r = 0; r < ui->tableWidget_2->rowCount(); ++r) {
            auto *it = ui->tableWidget_2->item(r, codeCol);
            if (!it) continue;
            const QString realCode = it->data(Qt::UserRole + 1).toString();
            if (realCode.isEmpty()) continue;
            it->setText(m_showCodesInList ? realCode : QStringLiteral("********"));
        }
    } else {
        return;
    }

    auto *h = ui->tableWidget_2->horizontalHeader();
    if (h->sortIndicatorSection() == logicalIndex) {
        h->setSortIndicator(0, Qt::AscendingOrder);
        ui->tableWidget_2->sortItems(0, Qt::AscendingOrder);
    }
}
void MainWindow::on_btnSubEmpl4_2_clicked() {
    ui->stackedWidgetEmployes->setCurrentIndex(1);
    syncEmployesSubNavChecked(ui, 1);
    updateEmployeeStats();
}

// ==================== MODULE PECHEUR ====================
void MainWindow::on_btnSubPech1_clicked() { ui->stackedWidgetPecheurs->setCurrentIndex(0); }
void MainWindow::on_btnSubPech2_clicked() {
    // UI -> DB: opening the list page triggers a fresh SELECT from Pecheur model.
    // Path: MainWindow::afficherPecheurs(...) -> Ptmp.afficherAvecFiltres(...) -> SELECT ... FROM PECHEUR
    afficherPecheurs(); // Rafraîchir les données depuis la BDD avant d'afficher les stats
    ui->stackedWidgetPecheurs->setCurrentIndex(1);
    updatePecheurRoleChart();
    updatePecheurStatusChart();
}
void MainWindow::on_btnSubPech3_clicked() { ui->stackedWidgetPecheurs->setCurrentIndex(2); }

void MainWindow::afficherPecheurs(const QString &searchText, int sortIndex)
{
    // Dynamic DB reload for Pecheur page (SELECT + optional WHERE/ORDER BY in model).
    if (!ui->tableWidget_4)
        return;

    int effectiveSort = sortIndex;
    if (effectiveSort < 0 && ui->cb_sort_2)
        effectiveSort = ui->cb_sort_2->currentIndex();

    QString effectiveSearch = searchText;
    if (effectiveSearch.isEmpty() && ui->le_search_2)
        effectiveSearch = ui->le_search_2->text();

    // Main SQL call used by table rendering.
    QSqlQuery query = Ptmp.afficherAvecFiltres(effectiveSearch, effectiveSort);
    
    ui->tableWidget_4->clearContents();
    ui->tableWidget_4->setRowCount(0);
    
    // Colonnes: 0=ID, 1=Nom, 2=Prenom, 3=Role, 4=Exp, 5=Statut, 6=Téléphone, 7=RFID, 8=Date limite
    ui->tableWidget_4->setColumnCount(9);
    ui->tableWidget_4->setHorizontalHeaderLabels({
        QStringLiteral("ID"),
        QStringLiteral("Nom"),
        QStringLiteral("Prenom"),
        QStringLiteral("Role"),
        QStringLiteral("Experience"),
        QStringLiteral("Statut"),
        QStringLiteral("Telephone"),
        QStringLiteral("RFID"),
        QStringLiteral("Date limite")
    });
    const QDate today = QDate::currentDate();
    m_pecheurWarningRows.clear();
    int row = 0;
    while (query.next()) {
        ui->tableWidget_4->insertRow(row);
        QString nomprenom = query.value(1).toString().trimmed();
        int spaceIdx = nomprenom.indexOf(' ');
        QString nom = spaceIdx > 0 ? nomprenom.left(spaceIdx) : nomprenom;
        QString prenom = spaceIdx > 0 ? nomprenom.mid(spaceIdx + 1) : QString();
        QDate dateLimite = query.value(5).toDate();
        QString statut = query.value(4).toString().trimmed();

        ui->tableWidget_4->setItem(row, 0, new QTableWidgetItem(query.value(0).toString()));
        ui->tableWidget_4->setItem(row, 1, new QTableWidgetItem(nom));
        ui->tableWidget_4->setItem(row, 2, new QTableWidgetItem(prenom));
        ui->tableWidget_4->setItem(row, 3, new QTableWidgetItem(query.value(2).toString()));
        ui->tableWidget_4->setItem(row, 4, new QTableWidgetItem(QString::number(query.value(3).toInt()) + " ans"));
        ui->tableWidget_4->setItem(row, 5, new QTableWidgetItem(statut));
        ui->tableWidget_4->setItem(row, 6, new QTableWidgetItem(query.value(6).toString()));
        ui->tableWidget_4->setItem(row, 7, new QTableWidgetItem(query.value(7).toString()));
        QTableWidgetItem *dateItem = new QTableWidgetItem(dateLimite.isValid() ? dateLimite.toString(QStringLiteral("dd/MM/yyyy")) : QString());
        dateItem->setData(Qt::UserRole, dateLimite);
        ui->tableWidget_4->setItem(row, 8, dateItem);
        if (statut.compare(QStringLiteral("Inactif"), Qt::CaseInsensitive) == 0 && dateLimite.isValid() && dateLimite <= today)
            m_pecheurWarningRows.append(row);
        row++;
    }
    if (ui->tableWidget_4->columnCount() > 8)
        ui->tableWidget_4->setColumnHidden(8, false);
    if (!m_pecheurBlinkTimer) {
        m_pecheurBlinkTimer = new QTimer(this);
        connect(m_pecheurBlinkTimer, &QTimer::timeout, this, &MainWindow::updatePecheurWarningBlink);
    }
    m_pecheurBlinkTimer->stop();
    if (!m_pecheurWarningRows.isEmpty())
        m_pecheurBlinkTimer->start(500);
    updatePecheurWarningBlink();
    updatePecheurRoleChart();
    updatePecheurStatusChart();
}

void MainWindow::setupPecheurCharts()
{
    // Create chart views inside the two statistic boxes (role & status)
    // Role chart
    {
        auto layout = new QVBoxLayout(ui->chartViewRole_2);
        layout->setContentsMargins(0, 0, 0, 0);
        layout->setSpacing(0);

        m_roleChartView = new QChartView(new QChart(), ui->chartViewRole_2);
        m_roleChartView->setStyleSheet("background: transparent;");
        m_roleChartView->setRenderHint(QPainter::Antialiasing);
        m_roleChartView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        QChart *roleChart = m_roleChartView->chart();
        roleChart->setAnimationOptions(QChart::SeriesAnimations);
        roleChart->setAnimationDuration(800);
        roleChart->setAnimationEasingCurve(QEasingCurve::OutCubic);
        layout->addWidget(m_roleChartView);
    }

    // Status chart
    {
        auto layout = new QVBoxLayout(ui->chartViewStatus_2);
        layout->setContentsMargins(0, 0, 0, 0);
        layout->setSpacing(0);

        m_statusChartView = new QChartView(new QChart(), ui->chartViewStatus_2);
        m_statusChartView->setStyleSheet("background: transparent;");
        m_statusChartView->setRenderHint(QPainter::Antialiasing);
        m_statusChartView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        QChart *statusChart = m_statusChartView->chart();
        statusChart->setAnimationOptions(QChart::SeriesAnimations);
        statusChart->setAnimationDuration(800);
        statusChart->setAnimationEasingCurve(QEasingCurve::OutCubic);
        layout->addWidget(m_statusChartView);
    }

    updatePecheurRoleChart();
    updatePecheurStatusChart();
}

void MainWindow::updatePecheurRoleChart()
{
    if (!m_roleChartView)
        return;

    const QString searchText = ui->le_search_2 ? ui->le_search_2->text() : QString();
    // Pure DB stats: SQL GROUP BY role with optional current search filter.
    QList<QPair<QString, int>> counts = Ptmp.statistiquesParRole(searchText);
    if (counts.isEmpty())
        counts.append({QStringLiteral("Aucun rôle"), 1});

    applyPieChart(m_roleChartView, counts, kStatsPaletteDept, Qt::AlignBottom);
}

void MainWindow::updatePecheurStatusChart()
{
    if (!m_statusChartView)
        return;

    const QString searchText = ui->le_search_2 ? ui->le_search_2->text() : QString();
    // Pure DB stats: SQL GROUP BY statut with optional current search filter.
    QList<QPair<QString, int>> counts = Ptmp.statistiquesParStatut(searchText);
    if (counts.isEmpty())
        counts.append({QStringLiteral("Aucun pêcheur"), 1});

    applyPieChart(m_statusChartView, counts, kStatsPaletteShift, Qt::AlignBottom);
}

// Helpers for Availability (Moniteur de Dotation)
static QDate parseFrDate(const QString &s) { return QDate::fromString(s.trimmed(), "dd/MM/yyyy"); }
static QString displayFrDate(const QDate &d) { return d.isValid() ? d.toString("dd/MM/yyyy") : QString(); }
static QString formatDeptsListForAlert(const QStringList &list) {
    if (list.isEmpty()) return QString();
    if (list.size() == 1) return list.first();
    if (list.size() == 2) return list.at(0) + " et " + list.at(1);
    QStringList copy = list;
    QString last = copy.takeLast();
    return copy.join(", ") + " et " + last;
}
static bool isEmployeeAvailableForDate(const QString &status, const QDate &date) {
    const QString s = status.trimmed().toLower();
    if (s.contains("cong")) return false;
    if (s.contains("inactif")) return date != QDate::currentDate();
    return s.contains("actif") || s.isEmpty();
}
/** Équivalent SQL: UPPER(TRIM(certifications)) LIKE UPPER('%dept%') */
static bool certificationsMatchDept(const QString &certs, const QString &deptNeedle)
{
    const QString n = deptNeedle.trimmed();
    if (n.isEmpty())
        return false;
    const QString c = certs.trimmed();
    if (c.isEmpty())
        return false;
    return c.contains(n, Qt::CaseInsensitive);
}
static QSqlRecord employeRecordFromDb(const QSqlDatabase &db)
{
    QSqlRecord r = db.record(QStringLiteral("employe"));
    if (r.count() == 0)
        r = db.record(QStringLiteral("EMPLOYE"));
    return r;
}

static QString employeTableNameForQSql(const QSqlDatabase &db)
{
    if (db.record(QStringLiteral("employe")).count() > 0)
        return QStringLiteral("employe");
    if (db.record(QStringLiteral("EMPLOYE")).count() > 0)
        return QStringLiteral("EMPLOYE");
    return QStringLiteral("employe");
}

static QString findColumnNameInRecord(const QSqlRecord &rec, const QStringList &candidates)
{
    for (const QString &want : candidates) {
        if (rec.indexOf(want) >= 0)
            return want;
    }
    for (const QString &want : candidates) {
        for (int i = 0; i < rec.count(); ++i) {
            if (QString::compare(rec.fieldName(i), want, Qt::CaseInsensitive) == 0)
                return rec.fieldName(i);
        }
    }
    return QString();
}

static QString serviceColumnName(const QSqlDatabase &db)
{
    const QSqlRecord rec = employeRecordFromDb(db);
    QString n = findColumnNameInRecord(rec, {QStringLiteral("service"), QStringLiteral("SERVICE")});
    if (!n.isEmpty())
        return n;
    return QStringLiteral("service");
}

static QString posteColumnName(const QSqlDatabase &db)
{
    const QSqlRecord rec = employeRecordFromDb(db);
    QString n = findColumnNameInRecord(rec, {QStringLiteral("poste"), QStringLiteral("POSTE")});
    if (!n.isEmpty())
        return n;
    return QStringLiteral("poste");
}

/** Département logique pour le moniteur : SERVICE si renseigné, sinon POSTE (Oracle souvent avec SERVICE vide). */
static QString effectiveDeptSqlExpr(const QSqlDatabase &db)
{
    const QSqlRecord rec = employeRecordFromDb(db);
    const QString s = serviceColumnName(db);
    const QString p = posteColumnName(db);
    const bool hasS = !findColumnNameInRecord(rec, {QStringLiteral("service"), QStringLiteral("SERVICE")}).isEmpty();
    const bool hasP = !findColumnNameInRecord(rec, {QStringLiteral("poste"), QStringLiteral("POSTE")}).isEmpty();
    if (hasS && hasP) {
        return QStringLiteral("COALESCE(NULLIF(TRIM(%1),''), NULLIF(TRIM(%2),''))")
            .arg(s, p);
    }
    if (hasS)
        return QStringLiteral("NULLIF(TRIM(%1),'')").arg(s);
    if (hasP)
        return QStringLiteral("NULLIF(TRIM(%1),'')").arg(p);
    return QStringLiteral("NULLIF(TRIM(%1),'')").arg(s);
}

static bool hasAffectDotColumns(const QSqlDatabase &db)
{
    const QSqlRecord rec = employeRecordFromDb(db);
    const QString d1 = findColumnNameInRecord(rec, {QStringLiteral("affect_dot_date"), QStringLiteral("AFFECT_DOT_DATE")});
    const QString d2 = findColumnNameInRecord(rec, {QStringLiteral("affect_dot_dept"), QStringLiteral("AFFECT_DOT_DEPT")});
    return !d1.isEmpty() && !d2.isEmpty();
}

static QString monitorAssignKey(const QDate &d, const QString &dept)
{
    return QString::number(d.toJulianDay()) + QLatin1Char('|') + dept.trimmed();
}

/** Fixe le 1er jour du mois de ref et le nombre de jours (grille « tout le mois »). */
static void monitorMonthRangeFromDate(const QDate &anyDayInMonth, QDate *outStart, int *outDays)
{
    const QDate ms(anyDayInMonth.year(), anyDayInMonth.month(), 1);
    *outStart = ms;
    *outDays = ms.daysInMonth();
}

void MainWindow::reloadMonitorDbAssignmentsOnly()
{
    m_monitorDbAssignmentKeys.clear();
    m_monitorDbAssignments.clear();
    if (!m_db.isValid() || !m_db.isOpen())
        return;
    if (!hasAffectDotColumns(m_db))
        return;
    const QSqlRecord rec = employeRecordFromDb(m_db);
    const QString tbl = employeTableNameForQSql(m_db);
    const QString cinCol = findColumnNameInRecord(rec, {QStringLiteral("cin"), QStringLiteral("CIN")});
    const QString cinColFinal = cinCol.isEmpty() ? QStringLiteral("cin") : cinCol;
    const QString nomCol = findColumnNameInRecord(rec, {QStringLiteral("nom"), QStringLiteral("NOM")});
    const QString nomColFinal = nomCol.isEmpty() ? QStringLiteral("nom") : nomCol;
    const QString prenomCol = findColumnNameInRecord(rec, {QStringLiteral("prenom"), QStringLiteral("PRENOM")});
    const QString prenomColFinal = prenomCol.isEmpty() ? QStringLiteral("prenom") : prenomCol;
    const QString colDate = findColumnNameInRecord(rec, {QStringLiteral("affect_dot_date"), QStringLiteral("AFFECT_DOT_DATE")});
    const QString colDept = findColumnNameInRecord(rec, {QStringLiteral("affect_dot_dept"), QStringLiteral("AFFECT_DOT_DEPT")});
    if (colDate.isEmpty() || colDept.isEmpty())
        return;
    QSqlQuery q2(m_db);
    q2.prepare(QStringLiteral("SELECT %1, %2, %3, %4, %5 FROM %6 WHERE %1 >= ? AND %1 < ?")
        .arg(colDate, colDept, cinColFinal, nomColFinal, prenomColFinal, tbl));
    q2.addBindValue(m_monitorWeekStart);
    q2.addBindValue(m_monitorWeekStart.addDays(m_monitorDays));
    if (!q2.exec())
        return;
    while (q2.next()) {
        const QVariant v = q2.value(0);
        QDate d = v.toDate();
        if (!d.isValid()) {
            const QDateTime dt = v.toDateTime();
            if (dt.isValid())
                d = dt.date();
        }
        const QString dep = q2.value(1).toString().trimmed();
        if (d.isValid() && !dep.isEmpty()) {
            m_monitorDbAssignmentKeys.insert(monitorAssignKey(d, dep));
            MonitorDbAssignment a;
            a.date = d;
            a.dept = dep;
            a.cin = q2.value(2).toString().trimmed();
            a.nom = q2.value(3).toString().trimmed();
            a.prenom = q2.value(4).toString().trimmed();
            m_monitorDbAssignments.append(a);
        }
    }
}

void MainWindow::rebuildMonitorCacheFromDb(bool preferAssignmentsOnly)
{
    const bool skipEmployeeQuery = preferAssignmentsOnly && m_monitorCacheValid && !m_monitorEmployees.isEmpty();

    m_monitorDbAssignmentKeys.clear();
    m_monitorDbAssignments.clear();
    m_monitorCacheValid = false;
    m_monitorCacheLoadedForMonthStart = QDate();
    if (!m_db.isValid() || !m_db.isOpen())
        return;
    if (!m_monitorWeekStart.isValid())
        monitorMonthRangeFromDate(QDate::currentDate(), &m_monitorWeekStart, &m_monitorDays);

    const QString deptExpr = effectiveDeptSqlExpr(m_db);
    const QString tbl = employeTableNameForQSql(m_db);
    const QSqlRecord rec = employeRecordFromDb(m_db);
    const QString statCol = findColumnNameInRecord(rec, {QStringLiteral("statut_professionnel"), QStringLiteral("STATUT_PROFESSIONNEL")});
    const QString statColFinal = statCol.isEmpty() ? QStringLiteral("statut_professionnel") : statCol;
    const QString certCol = findColumnNameInRecord(rec, {QStringLiteral("certifications"), QStringLiteral("CERTIFICATIONS")});
    const QString certColFinal = certCol.isEmpty() ? QStringLiteral("certifications") : certCol;
    const QString cinCol = findColumnNameInRecord(rec, {QStringLiteral("cin"), QStringLiteral("CIN")});
    const QString cinColFinal = cinCol.isEmpty() ? QStringLiteral("cin") : cinCol;
    const QString nomCol = findColumnNameInRecord(rec, {QStringLiteral("nom"), QStringLiteral("NOM")});
    const QString nomColFinal = nomCol.isEmpty() ? QStringLiteral("nom") : nomCol;
    const QString prenomCol = findColumnNameInRecord(rec, {QStringLiteral("prenom"), QStringLiteral("PRENOM")});
    const QString prenomColFinal = prenomCol.isEmpty() ? QStringLiteral("prenom") : prenomCol;

    if (!skipEmployeeQuery) {
        m_monitorEmployees.clear();
        m_monitorDeptList.clear();

        QSqlQuery q(m_db);
        q.prepare(QStringLiteral("SELECT %1, %2, %3, %4, %5, %6 FROM %7")
            .arg(cinColFinal, nomColFinal, prenomColFinal, deptExpr, certColFinal, statColFinal, tbl));
        if (q.exec()) {
            while (q.next()) {
                MonitorEmpRow row;
                row.cin = q.value(0).toString();
                row.nom = q.value(1).toString();
                row.prenom = q.value(2).toString();
                row.deptEff = q.value(3).toString().trimmed();
                row.certs = q.value(4).toString();
                row.statut = q.value(5).toString();
                if (row.deptEff.isEmpty())
                    continue;
                m_monitorEmployees.append(row);
            }
        }

        std::sort(m_monitorEmployees.begin(), m_monitorEmployees.end(),
            [](const MonitorEmpRow &a, const MonitorEmpRow &b) {
                const int c = QString::compare(a.nom, b.nom, Qt::CaseInsensitive);
                if (c != 0)
                    return c < 0;
                return QString::compare(a.prenom, b.prenom, Qt::CaseInsensitive) < 0;
            });

        QSet<QString> seen;
        for (const MonitorEmpRow &e : m_monitorEmployees)
            seen.insert(e.deptEff);
        m_monitorDeptList.clear();
        for (const QString &s : seen)
            m_monitorDeptList.append(s);
        std::sort(m_monitorDeptList.begin(), m_monitorDeptList.end());
    }

    reloadMonitorDbAssignmentsOnly();
    m_monitorCacheValid = true;
    m_monitorCacheLoadedForMonthStart = m_monitorWeekStart;
}
QHash<QString, int> MainWindow::monitorAvailCountByDeptKey(const QDate &d) const
{
    QHash<QString, int> availCount;
    for (const MonitorEmpRow &e : m_monitorEmployees) {
        if (!isEmployeeAvailableForDate(e.statut, d))
            continue;
        availCount[e.deptEff] += 1;
    }
    return availCount;
}

bool MainWindow::monitorHasAssignment(const QDate &d, const QString &dept) const
{
    const QString k = dept.trimmed();
    if (m_assignmentsSession.contains(qMakePair(d, k)))
        return true;
    if (!hasAffectDotColumns(m_db))
        return false;
    return m_monitorDbAssignmentKeys.contains(monitorAssignKey(d, k));
}

int MainWindow::monitorAvailableCount(const QDate &d, const QString &dept) const
{
    const QString k = dept.trimmed();
    const QHash<QString, int> avail = monitorAvailCountByDeptKey(d);
    return avail.value(k, 0);
}

QStringList MainWindow::monitorMissingDeptsForDate(const QDate &d) const
{
    const QHash<QString, int> availCount = monitorAvailCountByDeptKey(d);
    QStringList out;
    for (const QString &dept : m_monitorDeptList) {
        if (dept.trimmed().isEmpty())
            continue;
        if (monitorHasAssignment(d, dept))
            continue;
        if (availCount.value(dept.trimmed(), 0) == 0)
            out << dept;
    }
    return out;
}

int MainWindow::monitorMissingCountForDate(const QDate &d) const
{
    const QHash<QString, int> availCount = monitorAvailCountByDeptKey(d);
    int n = 0;
    for (const QString &dept : m_monitorDeptList) {
        if (dept.trimmed().isEmpty())
            continue;
        if (monitorHasAssignment(d, dept))
            continue;
        if (availCount.value(dept.trimmed(), 0) == 0)
            ++n;
    }
    return n;
}

QString MainWindow::monitorAssignmentsSummaryForDate(const QDate &d) const
{
    QStringList lines;
    for (auto it = m_assignmentsSession.constBegin(); it != m_assignmentsSession.constEnd(); ++it) {
        if (it.key().first != d)
            continue;
        const QString dept = it.key().second.trimmed();
        const QString cin = it.value().trimmed();
        QString nom;
        QString prenom;
        for (const MonitorEmpRow &e : m_monitorEmployees) {
            if (e.cin == cin) {
                nom = e.nom;
                prenom = e.prenom;
                break;
            }
        }
        if (nom.isEmpty() && prenom.isEmpty())
            lines << QStringLiteral("• CIN %1 → %2").arg(cin, dept);
        else
            lines << QStringLiteral("• %1 %2 → %3").arg(prenom, nom, dept);
    }

    for (const MonitorDbAssignment &a : m_monitorDbAssignments) {
        if (a.date != d)
            continue;
        const QString dept = a.dept.trimmed();
        bool overriddenBySession = false;
        for (auto it = m_assignmentsSession.constBegin(); it != m_assignmentsSession.constEnd(); ++it) {
            if (it.key().first == d && it.key().second.trimmed() == dept) {
                overriddenBySession = true;
                break;
            }
        }
        if (overriddenBySession)
            continue;
        if (a.nom.isEmpty() && a.prenom.isEmpty())
            lines << QStringLiteral("• CIN %1 → %2").arg(a.cin, dept);
        else
            lines << QStringLiteral("• %1 %2 → %3").arg(a.prenom, a.nom, dept);
    }

    if (lines.isEmpty())
        return tr("Aucune affectation enregistrée pour cette date (session ou base).");

    return tr("Affectations :\n") + lines.join(QLatin1Char('\n'));
}

bool MainWindow::monitorDateHasAssignments(const QDate &d) const
{
    for (auto it = m_assignmentsSession.constBegin(); it != m_assignmentsSession.constEnd(); ++it) {
        if (it.key().first == d)
            return true;
    }
    for (const MonitorDbAssignment &a : m_monitorDbAssignments) {
        if (a.date == d)
            return true;
    }
    return false;
}

QString MainWindow::monitorAssignmentsForDept(const QDate &d, const QString &dept) const
{
    const QString k = dept.trimmed();
    QStringList lines;

    for (auto it = m_assignmentsSession.constBegin(); it != m_assignmentsSession.constEnd(); ++it) {
        if (it.key().first != d || it.key().second.trimmed() != k)
            continue;
        const QString cin = it.value().trimmed();
        QString nom;
        QString prenom;
        for (const MonitorEmpRow &e : m_monitorEmployees) {
            if (e.cin == cin) {
                nom = e.nom;
                prenom = e.prenom;
                break;
            }
        }
        if (nom.isEmpty() && prenom.isEmpty())
            lines << QStringLiteral("• CIN %1 → %2").arg(cin, k);
        else
            lines << QStringLiteral("• %1 %2 → %3").arg(prenom, nom, k);
    }

    for (const MonitorDbAssignment &a : m_monitorDbAssignments) {
        if (a.date != d || a.dept.trimmed() != k)
            continue;
        bool overriddenBySession = false;
        for (auto it = m_assignmentsSession.constBegin(); it != m_assignmentsSession.constEnd(); ++it) {
            if (it.key().first == d && it.key().second.trimmed() == k) {
                overriddenBySession = true;
                break;
            }
        }
        if (overriddenBySession)
            continue;
        if (a.nom.isEmpty() && a.prenom.isEmpty())
            lines << QStringLiteral("• CIN %1 → %2").arg(a.cin, k);
        else
            lines << QStringLiteral("• %1 %2 → %3").arg(a.prenom, a.nom, k);
    }

    if (lines.isEmpty())
        return tr("Aucune affectation enregistrée pour %1 le %2.").arg(k, displayFrDate(d));

    return tr("Affectations :\n") + lines.join(QLatin1Char('\n'));
}

void MainWindow::layoutMonitorLeftColumn()
{
    if (!ui->tableWidget || !ui->frameRoleDetail)
        return;
    const int x = 20;
    const int w = 260;
    // Sous la ligne mois / année / PDF (.ui : combos y=92, h=42 → table dès ~141).
    int yTop = 141;
    if (QComboBox *mon = this->findChild<QComboBox *>(QStringLiteral("comboMonitorMonth"))) {
        const QRect gr = mon->geometry();
        if (gr.isValid() && (gr.y() > 0 || gr.height() > 0))
            yTop = qMax(yTop, gr.bottom() + 7);
    }
    const QRect fr = ui->frameRoleDetail->geometry();
    const int bottom = fr.y() + fr.height();
    const bool showList = ui->listRoles && ui->listRoles->isVisible() && ui->listRoles->count() > 0;
    if (showList) {
        const int listH = 168;
        const int tableH = qMax(120, bottom - yTop - listH - 8);
        ui->tableWidget->setGeometry(x, yTop, w, tableH);
        ui->listRoles->setGeometry(x, yTop + tableH + 8, w, listH);
    } else {
        ui->tableWidget->setGeometry(x, yTop, w, qMax(120, bottom - yTop));
    }
}

static bool aggregateEmployeeKpi(const QSqlDatabase &db, const QDate &today, EmployeeKpiSnapshot *snap,
    QMap<QString, int> *deptCounts, QMap<QString, int> *skillCounts,
    QMap<QString, int> *shiftCounts, QMap<QString, int> *cityCounts)
{
    if (!snap)
        return false;
    *snap = EmployeeKpiSnapshot{};
    const QDate plus30 = today.addDays(30);

    QSqlQuery q(db);
    q.prepare("SELECT statut_professionnel, service, niveau_competence, horaire_travail, ville, certifications, date_expiration_certification FROM employe");
    if (!q.exec())
        return false;

    while (q.next()) {
        ++snap->total;
        const QString statutRaw = q.value(0).toString();
        const QString serviceRaw = q.value(1).toString().trimmed();
        if (serviceRaw.isEmpty())
            ++snap->missingService;
        const QString dept = serviceRaw.isEmpty() ? QStringLiteral("Inconnu") : serviceRaw;
        const QString level = q.value(2).toString().trimmed().isEmpty() ? QStringLiteral("Inconnu") : q.value(2).toString().trimmed();
        const QString shift = q.value(3).toString().trimmed().isEmpty() ? QStringLiteral("Inconnu") : q.value(3).toString().trimmed();
        const QString city = q.value(4).toString().trimmed().isEmpty() ? QStringLiteral("Inconnu") : q.value(4).toString().trimmed();
        if (deptCounts)
            (*deptCounts)[dept] += 1;
        if (skillCounts)
            (*skillCounts)[level] += 1;
        if (shiftCounts)
            (*shiftCounts)[shift] += 1;
        if (cityCounts)
            (*cityCounts)[city] += 1;
        const QString s = statutRaw.trimmed().toLower();
        if (s.contains(QStringLiteral("cong")))
            ++snap->onLeave;
        else if (s.contains(QStringLiteral("inactif")))
            ++snap->inactive;
        else if (s.contains(QStringLiteral("actif")))
            ++snap->active;
        if (isEmployeeAvailableForDate(statutRaw, today))
            ++snap->availableToday;
        const QString certText = q.value(5).toString().trimmed();
        const QDate exp = q.value(6).toDate();
        if (!certText.isEmpty() && exp.isValid() && exp >= today && exp <= plus30)
            ++snap->certExp30;
        if (!certText.isEmpty() && exp.isValid() && exp < today)
            ++snap->certExpired;
    }
    return true;
}

void MainWindow::updateEmplBadgePulse(bool active)
{
    if (!m_emplBadgeShadow)
        return;
    if (!m_emplBadgePulseAnim) {
        m_emplBadgePulseAnim = new QVariantAnimation(this);
        m_emplBadgePulseAnim->setDuration(1600);
        m_emplBadgePulseAnim->setStartValue(0);
        m_emplBadgePulseAnim->setEndValue(100);
        m_emplBadgePulseAnim->setLoopCount(-1);
        m_emplBadgePulseAnim->setEasingCurve(QEasingCurve::Linear);
        connect(m_emplBadgePulseAnim, &QVariantAnimation::valueChanged, this, [this](const QVariant &v) {
            if (!m_emplBadgeShadow)
                return;
            const double t = v.toDouble() / 100.0;
            const double blur = 14.0 + 5.0 * std::sin(6.28318530717958647692 * t);
            m_emplBadgeShadow->setBlurRadius(blur);
        });
    }
    if (active) {
        m_emplBadgePulseAnim->start();
    } else {
        m_emplBadgePulseAnim->stop();
    }
}

void MainWindow::applyEmplListeBadgeLook(int total, bool dbOk)
{
    if (!ui->btnEmplListeAlertBadge)
        return;

    ui->btnEmplListeAlertBadge->setMinimumSize(32, 32);
    ui->btnEmplListeAlertBadge->setMaximumSize(34, 34);
    ui->btnEmplListeAlertBadge->setAccessibleName(tr("Notifications et alertes employés"));

    if (!m_emplBadgeShadow) {
        m_emplBadgeShadow = new QGraphicsDropShadowEffect(ui->btnEmplListeAlertBadge);
        m_emplBadgeShadow->setOffset(0, 2);
        ui->btnEmplListeAlertBadge->setGraphicsEffect(m_emplBadgeShadow);
    }

    const QString menuInd = QStringLiteral(
        "QToolButton::menu-indicator { image: none; width: 0; height: 0; subcontrol-position: right center; }");

    QFont f = ui->btnEmplListeAlertBadge->font();
    f.setBold(true);
    if (!dbOk) {
        updateEmplBadgePulse(false);
        m_emplBadgeShadow->setEnabled(false);
        ui->btnEmplListeAlertBadge->setText(QStringLiteral("…"));
        f.setPointSize(14);
        ui->btnEmplListeAlertBadge->setFont(f);
        ui->btnEmplListeAlertBadge->setStyleSheet(
            QStringLiteral(
                "QToolButton { border-radius: 17px; min-width: 32px; min-height: 32px; max-width: 34px; max-height: 34px; "
                "font-weight: bold; padding: 0; border-style: solid; border-width: 1px; border-color: rgba(255,255,255,0.2); "
                "background: qlineargradient(x1:0,y1:0,x2:0,y2:1, stop:0 #64748b, stop:1 #334155); color: #e2e8f0; }"
                "QToolButton:hover { background: qlineargradient(x1:0,y1:0,x2:0,y2:1, stop:0 #94a3b8, stop:1 #475569); }"
                "QToolButton:pressed { padding-top: 1px; }")
            + menuInd);
        return;
    }

    m_emplBadgeShadow->setEnabled(true);

    if (total > 0) {
        m_emplBadgeShadow->setColor(QColor(220, 38, 38, 220));
        ui->btnEmplListeAlertBadge->setText(total > 99 ? QStringLiteral("99+") : QString::number(total));
        f.setPointSize(total > 99 ? 9 : (total > 9 ? 10 : 12));
        ui->btnEmplListeAlertBadge->setFont(f);
        ui->btnEmplListeAlertBadge->setStyleSheet(
            QStringLiteral(
                "QToolButton { border-radius: 17px; min-width: 32px; min-height: 32px; max-width: 34px; max-height: 34px; "
                "font-weight: bold; padding: 0; border-style: solid; border-width: 2px; border-color: rgba(255,255,255,0.55); "
                "background: qlineargradient(x1:0,y1:0,x2:0,y2:1, stop:0 #fecaca, stop:0.45 #ef4444, stop:1 #991b1b); color: #fffbeb; }"
                "QToolButton:hover { background: qlineargradient(x1:0,y1:0,x2:0,y2:1, stop:0 #fca5a5, stop:0.5 #f87171, stop:1 #b91c1c); }"
                "QToolButton:pressed { padding-top: 1px; }")
            + menuInd);
        updateEmplBadgePulse(true);
        return;
    }

    updateEmplBadgePulse(false);
    m_emplBadgeShadow->setBlurRadius(10);
    m_emplBadgeShadow->setColor(QColor(16, 185, 129, 100));
    ui->btnEmplListeAlertBadge->setText(QChar(0x2713));
    f.setPointSize(14);
    ui->btnEmplListeAlertBadge->setFont(f);
    ui->btnEmplListeAlertBadge->setStyleSheet(
        QStringLiteral(
            "QToolButton { border-radius: 17px; min-width: 32px; min-height: 32px; max-width: 34px; max-height: 34px; "
            "font-weight: bold; padding: 0; border-style: solid; border-width: 1px; border-color: rgba(255,255,255,0.35); "
            "background: qlineargradient(x1:0,y1:0,x2:0,y2:1, stop:0 #6ee7b7, stop:1 #047857); color: #ecfdf5; }"
            "QToolButton:hover { background: qlineargradient(x1:0,y1:0,x2:0,y2:1, stop:0 #34d399, stop:1 #059669); }"
            "QToolButton:pressed { padding-top: 1px; }")
        + menuInd);
}

void MainWindow::setupEmployeeListeAlertBadge()
{
    if (!ui->btnEmplListeAlertBadge)
        return;
    m_menuEmplListeAlerts = new QMenu(this);
    m_menuEmplListeAlerts->setToolTipsVisible(true);
    m_menuEmplListeAlerts->setStyleSheet(
        QStringLiteral("QMenu { background-color: #0f172a; color: #f1f5f9; border: 1px solid #334155; padding: 6px; border-radius: 10px; }"
                       "QMenu::separator { height: 1px; background: #334155; margin: 6px 10px; }"
                       "QMenu::item { padding: 10px 18px; border-radius: 6px; }"
                       "QMenu::item:selected { background-color: #0f766e; color: #ecfdf5; }"
                       "QMenu::icon { padding-left: 8px; }"));
    ui->btnEmplListeAlertBadge->setMenu(m_menuEmplListeAlerts);
    ui->btnEmplListeAlertBadge->setCursor(Qt::PointingHandCursor);
    ui->btnEmplListeAlertBadge->setVisible(true);
    ui->btnEmplListeAlertBadge->setAutoRaise(false);
    applyEmplListeBadgeLook(0, false);
}

void MainWindow::refreshEmployeeListeAlerts(const EmployeeKpiSnapshot *snapIn)
{
    if (!ui->btnEmplListeAlertBadge || !m_menuEmplListeAlerts)
        return;
    if (!m_db.isValid() || !m_db.isOpen()) {
        applyEmplListeBadgeLook(0, false);
        if (ui->btnEmplListeAlertBadge)
            ui->btnEmplListeAlertBadge->setToolTip(tr("Connexion base de données indisponible"));
        return;
    }

    EmployeeKpiSnapshot snap;
    if (snapIn) {
        snap = *snapIn;
    } else {
        const qint64 kpiAgeMs = m_empKpiCacheTime.isValid()
            ? m_empKpiCacheTime.msecsTo(QDateTime::currentDateTime())
            : 999999;
        const bool useKpiCache = m_empKpiCacheTime.isValid() && kpiAgeMs >= 0 && kpiAgeMs < 5000;
        if (useKpiCache)
            snap = m_empKpiCache;
        else if (!aggregateEmployeeKpi(m_db, QDate::currentDate(), &snap, nullptr, nullptr, nullptr, nullptr)) {
            applyEmplListeBadgeLook(0, false);
            if (ui->btnEmplListeAlertBadge)
                ui->btnEmplListeAlertBadge->setToolTip(tr("Impossible de charger les indicateurs employés."));
            return;
        } else {
            m_empKpiCache = snap;
            m_empKpiCacheTime = QDateTime::currentDateTime();
        }
    }

    int monthGapDays = 0;
    {
        QDate mStart;
        int mDays = 31;
        monitorMonthRangeFromDate(QDate::currentDate(), &mStart, &mDays);
        const bool needMonitorRebuild = !m_monitorCacheValid
            || m_monitorWeekStart != mStart
            || m_monitorDays != mDays;
        if (needMonitorRebuild) {
            m_monitorWeekStart = mStart;
            m_monitorDays = mDays;
            rebuildMonitorCacheFromDb();
        }
        for (int i = 0; i < m_monitorDays; ++i) {
            const QDate d = m_monitorWeekStart.addDays(i);
            if (monitorMissingCountForDate(d) > 0)
                ++monthGapDays;
        }
    }

    const int total = snap.certExp30 + snap.certExpired + snap.onLeave + snap.inactive + snap.missingService + monthGapDays;
    const int pctPresence = snap.total > 0 ? qRound(100.0 * snap.availableToday / snap.total) : 0;

    m_menuEmplListeAlerts->clear();
    m_menuEmplListeAlerts->addSection(tr("Synthèse des alertes"));
    m_menuEmplListeAlerts->addAction(
        (snap.certExp30 <= 1)
            ? tr("• %1 certification à renouveler (≤30 jours)").arg(snap.certExp30)
            : tr("• %1 certifications à renouveler (≤30 jours)").arg(snap.certExp30))->setEnabled(false);
    if (snap.certExpired > 0) {
        m_menuEmplListeAlerts->addAction(
            (snap.certExpired <= 1)
                ? tr("• %1 certification expirée (non renouvelée)").arg(snap.certExpired)
                : tr("• %1 certifications expirées (non renouvelées)").arg(snap.certExpired))->setEnabled(false);
    }
    m_menuEmplListeAlerts->addAction(
        (snap.onLeave <= 1)
            ? tr("• %1 employé en congé").arg(snap.onLeave)
            : tr("• %1 employés en congé").arg(snap.onLeave))->setEnabled(false);
    if (snap.inactive > 0) {
        m_menuEmplListeAlerts->addAction(
            (snap.inactive <= 1)
                ? tr("• %1 employé inactif").arg(snap.inactive)
                : tr("• %1 employés inactifs").arg(snap.inactive))->setEnabled(false);
    }
    if (snap.missingService > 0) {
        m_menuEmplListeAlerts->addAction(
            (snap.missingService <= 1)
                ? tr("• %1 employé sans service (champ vide)").arg(snap.missingService)
                : tr("• %1 employés sans service (champ vide)").arg(snap.missingService))->setEnabled(false);
    }
    m_menuEmplListeAlerts->addAction(
        tr("• %1 jour(s) du mois avec manque (dotation / remplacements)").arg(monthGapDays))->setEnabled(false);
    m_menuEmplListeAlerts->addAction(
        tr("• Présence aujourd'hui : %1% (%2/%3 employés)").arg(pctPresence).arg(snap.availableToday).arg(snap.total))->setEnabled(false);

    ui->btnEmplListeAlertBadge->setToolTip(
        total > 0 ? tr("%1 point(s) d’alerte — cliquez pour le détail").arg(total)
                  : tr("Aucune alerte prioritaire — cliquez pour la synthèse"));

    applyEmplListeBadgeLook(total, true);
}

void MainWindow::on_btnSubEmpl5_clicked() {
    ui->stackedWidgetEmployes->setCurrentIndex(2);  // Disponibilité
    syncEmployesSubNavChecked(ui, 2);
    // Ne pas appeler processEvents ici : ça force des peintures intermédiaires visibles.

    m_selectedMonitorDate = QDate();
    m_selectedMonitorDepartment.clear();

    if (ui->listRoles) ui->listRoles->clear();
    // Ne pas vider les tableaux ici : le chargement est différé ; garder les anciens résultats visibles.
    // Ne pas réinitialiser le bandeau d’alerte ici : sinon l’ALERTE / le message disparaît avant le
    // chargement ; il sera mis à jour par on_tableWidget_cellClicked après rebuildMonitorCacheFromDb.

    if (!m_db.isValid() || !m_db.isOpen())
        return;
    if (!ui->tableWidget)
        return;

    // Délai court : laisser le QStackedWidget peindre l’onglet avant le gros travail BDD / grille.
    QTimer::singleShot(1, this, [this]() {
        if (!ui->tableWidget)
            return;
        const QDate today = QDate::currentDate();
        syncMonitorCombosToMonth(today);

        QDate monthStart;
        int monthDays = 0;
        monitorMonthRangeFromDate(today, &monthStart, &monthDays);
        m_monitorWeekStart = monthStart;
        m_monitorDays = monthDays;

        const bool sameMonthAlreadyLoaded = m_monitorCacheValid && !m_monitorEmployees.isEmpty()
            && m_monitorCacheLoadedForMonthStart.isValid()
            && m_monitorCacheLoadedForMonthStart == monthStart;
        // Mois déjà en cache : recharger seulement les affectations (requête légère), pas toute la table employé.
        rebuildMonitorCacheFromDb(sameMonthAlreadyLoaded);

        refreshMonitorDisponibiliteTable();
    });
}

void MainWindow::on_tableWidget_cellClicked(int row, int column) {
    Q_UNUSED(column)
    if (!ui->tableWidget || !m_db.isValid() || !m_db.isOpen()) return;

    const QString dateLabel = ui->tableWidget->item(row, 0) ? ui->tableWidget->item(row, 0)->text() : QString();
    const QDate d = parseFrDate(dateLabel);
    if (!d.isValid()) return;

    m_selectedMonitorDate = d;
    m_selectedMonitorDepartment.clear();

    if (ui->listRoles)
        ui->listRoles->clear();
    // Ne pas vider les tableaux ici : on_listRoles remplace en une fois (anciens résultats jusqu’au nouveau rendu).

    if (!m_monitorCacheValid) {
        monitorMonthRangeFromDate(QDate::currentDate(), &m_monitorWeekStart, &m_monitorDays);
        if (ui->tableWidget && ui->tableWidget->rowCount() > 0) {
            const QDate w0 = parseFrDate(ui->tableWidget->item(0, 0) ? ui->tableWidget->item(0, 0)->text() : QString());
            if (w0.isValid())
                monitorMonthRangeFromDate(w0, &m_monitorWeekStart, &m_monitorDays);
        }
        rebuildMonitorCacheFromDb();
    }

    const QStringList depts = m_monitorDeptList;
    if (depts.isEmpty()) {
        ui->lblRoleStatusIcon->setText(QStringLiteral("ℹ️"));
        ui->lblRoleStatusText->setText(QStringLiteral(
            "Aucun service ou poste exploitable : tous les champs SERVICE et POSTE sont vides. "
            "Renseignez au moins l'un des deux pour chaque employé, ou vérifiez la connexion Oracle."));
        ui->frameRoleStatus->setStyleSheet(
            QStringLiteral("QFrame { background-color: #334155; border-radius: 12px; border: 1px solid rgba(255,255,255,0.14); } QLabel { color: white; border: none; }"));
        if (!ui->tableWidget->item(row, 1))
            ui->tableWidget->setItem(row, 1, new QTableWidgetItem());
        ui->tableWidget->item(row, 1)->setText(QStringLiteral("—"));
        ui->tableWidget->item(row, 1)->setTextAlignment(Qt::AlignCenter);
        if (ui->listRoles) {
            ui->listRoles->clear();
            ui->listRoles->setVisible(false);
        }
        ui->lblAutoTitle->setText(QStringLiteral("Suggestions Automatiques (Certifiés)"));
        ui->lblManualTitle->setText(QStringLiteral("Remplacement Manuel (Tous les Actifs)"));
        ui->tableRoleAutoSuggestions->setRowCount(0);
        ui->tableRoleManualSelection->setRowCount(0);
        if (ui->lblMonitorAssignments)
            ui->lblMonitorAssignments->clear();
        layoutMonitorLeftColumn();
        return;
    }

    const QDate today = QDate::currentDate();
    const bool isPast = d < today;
    const bool isFuture = d > today;

    QStringList missingDepts = monitorMissingDeptsForDate(d);
    int missing = missingDepts.size();
    if (isPast) {
        missing = 0;
        missingDepts.clear();
    }

    if (!ui->tableWidget->item(row, 1))
        ui->tableWidget->setItem(row, 1, new QTableWidgetItem());
    ui->tableWidget->item(row, 1)->setText(QString::number(missing));
    ui->tableWidget->item(row, 1)->setTextAlignment(Qt::AlignCenter);

    if (isPast) {
        const QString summary = monitorAssignmentsSummaryForDate(d);
        const bool hasAny = monitorDateHasAssignments(d);
        ui->lblRoleStatusIcon->setText(hasAny ? QStringLiteral("📋") : QStringLiteral("ℹ️"));
        ui->lblRoleStatusText->setText(summary);
        ui->frameRoleStatus->setStyleSheet(hasAny
            ? QStringLiteral("QFrame { background-color: #0f766e; border-radius: 12px; border: 1px solid rgba(255,255,255,0.14); } QLabel { color: white; border: none; }")
            : QStringLiteral("QFrame { background-color: #334155; border-radius: 12px; border: 1px solid rgba(255,255,255,0.14); } QLabel { color: white; border: none; }"));
        if (ui->lblMonitorAssignments) {
            ui->lblMonitorAssignments->clear();
            ui->lblMonitorAssignments->setVisible(false);
        }
        ui->lblAutoTitle->setText(QStringLiteral("Suggestions Automatiques (Certifiés)"));
        ui->lblManualTitle->setText(QStringLiteral("Remplacement Manuel (Tous les Actifs)"));
        ui->tableRoleAutoSuggestions->setRowCount(0);
        ui->tableRoleManualSelection->setRowCount(0);
        if (ui->listRoles) {
            ui->listRoles->clear();
            for (const QString &dep : depts) {
                if (dep.trimmed().isEmpty())
                    continue;
                QListWidgetItem *it = new QListWidgetItem(dep);
                it->setData(Qt::UserRole, dep);
                ui->listRoles->addItem(it);
            }
            ui->listRoles->setVisible(!depts.isEmpty());
        }
        layoutMonitorLeftColumn();
        return;
    }

    if (isFuture) {
        ui->lblRoleStatusIcon->setText(missing == 0 ? QStringLiteral("✅") : QStringLiteral("🚨"));
        if (missing == 0) {
            ui->lblRoleStatusText->setText(QStringLiteral("Date %1: aucun manque.").arg(displayFrDate(d)));
        } else {
            const QString listDepts = formatDeptsListForAlert(missingDepts);
            ui->lblRoleStatusText->setText(QStringLiteral("ALERTE: %1 département(s) sans employé\n%2\nle %3.")
                .arg(missing).arg(listDepts).arg(displayFrDate(d)));
        }
        ui->frameRoleStatus->setStyleSheet(missing == 0
            ? QStringLiteral("QFrame { background-color: #15803d; border-radius: 12px; border: 1px solid rgba(255,255,255,0.15); } QLabel { color: white; border: none; }")
            : QStringLiteral("QFrame { background-color: #b91c1c; border-radius: 12px; border: 1px solid rgba(255,255,255,0.12); } QLabel { color: white; border: none; }"));
        if (ui->lblMonitorAssignments) {
            ui->lblMonitorAssignments->clear();
            ui->lblMonitorAssignments->setVisible(false);
        }
        if (!missingDepts.isEmpty()) {
            m_selectedMonitorDepartment = missingDepts.first();
            ui->lblAutoTitle->setText(QStringLiteral("Suggestions Automatiques (Certifiés) - %1").arg(m_selectedMonitorDepartment));
            ui->lblManualTitle->setText(QStringLiteral("Remplacement Manuel (Tous les Actifs) - %1").arg(m_selectedMonitorDepartment));

            if (ui->listRoles) {
                ui->listRoles->clear();
                for (const QString &dep : depts) {
                    if (dep.trimmed().isEmpty())
                        continue;
                    QListWidgetItem *it = new QListWidgetItem(dep);
                    it->setData(Qt::UserRole, dep);
                    ui->listRoles->addItem(it);
                }
                ui->listRoles->setVisible(!depts.isEmpty());
            }

            QListWidgetItem fake(m_selectedMonitorDepartment);
            fake.setData(Qt::UserRole, m_selectedMonitorDepartment);
            on_listRoles_itemClicked(&fake);
        } else {
            ui->lblAutoTitle->setText(QStringLiteral("Suggestions Automatiques (Certifiés)"));
            ui->lblManualTitle->setText(QStringLiteral("Remplacement Manuel (Tous les Actifs)"));
            ui->tableRoleAutoSuggestions->setRowCount(0);
            ui->tableRoleManualSelection->setRowCount(0);
            if (ui->listRoles) {
                ui->listRoles->clear();
                ui->listRoles->setVisible(false);
            }
        }
        layoutMonitorLeftColumn();
        return;
    }

    // Aujourd’hui : alertes + détail des affectations
    ui->lblRoleStatusIcon->setText(missing == 0 ? QStringLiteral("✅") : QStringLiteral("🚨"));
    if (missing == 0) {
        ui->lblRoleStatusText->setText(QStringLiteral("Date %1: aucun manque.").arg(displayFrDate(d)));
    } else {
        const QString listDepts = formatDeptsListForAlert(missingDepts);
        ui->lblRoleStatusText->setText(QStringLiteral("ALERTE: %1 département(s) sans employé\n%2\nle %3.")
            .arg(missing).arg(listDepts).arg(displayFrDate(d)));
    }
    ui->frameRoleStatus->setStyleSheet(missing == 0
        ? QStringLiteral("QFrame { background-color: #15803d; border-radius: 12px; border: 1px solid rgba(255,255,255,0.15); } QLabel { color: white; border: none; }")
        : QStringLiteral("QFrame { background-color: #b91c1c; border-radius: 12px; border: 1px solid rgba(255,255,255,0.12); } QLabel { color: white; border: none; }"));

    if (ui->lblMonitorAssignments) {
        ui->lblMonitorAssignments->setText(monitorAssignmentsSummaryForDate(d));
        ui->lblMonitorAssignments->setVisible(true);
    }

    if (!missingDepts.isEmpty()) {
        m_selectedMonitorDepartment = missingDepts.first();
        ui->lblAutoTitle->setText(QStringLiteral("Suggestions Automatiques (Certifiés) - %1").arg(m_selectedMonitorDepartment));
        ui->lblManualTitle->setText(QStringLiteral("Remplacement Manuel (Tous les Actifs) - %1").arg(m_selectedMonitorDepartment));

        if (ui->listRoles) {
            ui->listRoles->clear();
            for (const QString &dep : depts) {
                if (dep.trimmed().isEmpty())
                    continue;
                QListWidgetItem *it = new QListWidgetItem(dep);
                it->setData(Qt::UserRole, dep);
                ui->listRoles->addItem(it);
            }
            ui->listRoles->setVisible(!depts.isEmpty());
        }

        QListWidgetItem fake(m_selectedMonitorDepartment);
        fake.setData(Qt::UserRole, m_selectedMonitorDepartment);
        on_listRoles_itemClicked(&fake);
    } else {
        ui->lblAutoTitle->setText(QStringLiteral("Suggestions Automatiques (Certifiés)"));
        ui->lblManualTitle->setText(QStringLiteral("Remplacement Manuel (Tous les Actifs)"));
        ui->tableRoleAutoSuggestions->setRowCount(0);
        ui->tableRoleManualSelection->setRowCount(0);
    }
    layoutMonitorLeftColumn();
}

void MainWindow::on_listRoles_itemClicked(QListWidgetItem *item) {
    if (!item) return;
    if (!m_db.isValid() || !m_db.isOpen()) return;
    if (!m_selectedMonitorDate.isValid()) {
        ui->lblRoleStatusIcon->setText("ℹ️");
        ui->lblRoleStatusText->setText("Sélectionnez d'abord une date.");
        return;
    }

    const QString dept = item->data(Qt::UserRole).toString().trimmed().isEmpty()
        ? item->text().trimmed()
        : item->data(Qt::UserRole).toString().trimmed();
    m_selectedMonitorDepartment = dept;

    const QDate todayRoles = QDate::currentDate();
    if (m_selectedMonitorDate < todayRoles) {
        const QString deptText = monitorAssignmentsForDept(m_selectedMonitorDate, dept);
        const bool hasAssign = monitorHasAssignment(m_selectedMonitorDate, dept);
        ui->lblRoleStatusIcon->setText(hasAssign ? QStringLiteral("📋") : QStringLiteral("ℹ️"));
        ui->lblRoleStatusText->setText(deptText);
        ui->frameRoleStatus->setStyleSheet(hasAssign
            ? QStringLiteral("QFrame { background-color: #0f766e; border-radius: 12px; border: 1px solid rgba(255,255,255,0.14); } QLabel { color: white; border: none; }")
            : QStringLiteral("QFrame { background-color: #334155; border-radius: 12px; border: 1px solid rgba(255,255,255,0.14); } QLabel { color: white; border: none; }"));
        ui->tableRoleAutoSuggestions->setRowCount(0);
        ui->tableRoleManualSelection->setRowCount(0);
        return;
    }

    struct TableUpdGuard {
        QTableWidget *a;
        QTableWidget *b;
        TableUpdGuard(QTableWidget *x, QTableWidget *y) : a(x), b(y)
        {
            a->setUpdatesEnabled(false);
            b->setUpdatesEnabled(false);
        }
        ~TableUpdGuard()
        {
            a->setUpdatesEnabled(true);
            b->setUpdatesEnabled(true);
        }
    } tableUpdGuard(ui->tableRoleAutoSuggestions, ui->tableRoleManualSelection);

    // Données déjà en cache — remplir sans vider avant (évite une liste vide visible au rafraîchissement).
    QVector<QStringList> autoRows;
    autoRows.reserve(m_monitorEmployees.size());
    for (const MonitorEmpRow &e : m_monitorEmployees) {
        if (!certificationsMatchDept(e.certs, dept))
            continue;
        if (!isEmployeeAvailableForDate(e.statut, m_selectedMonitorDate))
            continue;
        QStringList row;
        row << e.cin << e.nom << e.prenom << e.deptEff << e.certs << QStringLiteral("Affecter");
        autoRows.append(row);
    }
    ui->tableRoleAutoSuggestions->setRowCount(autoRows.size());
    for (int r = 0; r < autoRows.size(); ++r) {
        const QStringList &row = autoRows[r];
        for (int c = 0; c < 6; ++c)
            ui->tableRoleAutoSuggestions->setItem(r, c, new QTableWidgetItem(row[c]));
    }

    QVector<QStringList> manualRows;
    manualRows.reserve(m_monitorEmployees.size());
    QHash<QString, int> availToday;
    for (const MonitorEmpRow &e : m_monitorEmployees) {
        const bool available = isEmployeeAvailableForDate(e.statut, m_selectedMonitorDate);
        if (available)
            availToday[e.deptEff] += 1;
        QStringList row;
        row << e.cin << e.nom << e.prenom << e.deptEff << e.certs
            << (available ? QStringLiteral("Assigner") : QStringLiteral("Non disponible"));
        manualRows.append(row);
    }
    ui->tableRoleManualSelection->setRowCount(manualRows.size());
    for (int r = 0; r < manualRows.size(); ++r) {
        const QStringList &row = manualRows[r];
        for (int c = 0; c < 6; ++c)
            ui->tableRoleManualSelection->setItem(r, c, new QTableWidgetItem(row[c]));
    }

    const bool covered = monitorHasAssignment(m_selectedMonitorDate, dept)
        || (availToday.value(dept.trimmed(), 0) > 0);

    if (covered) {
        ui->lblRoleStatusIcon->setText("✅");
        ui->lblRoleStatusText->setText(QString("OK: %1 est couvert le %2.").arg(dept, displayFrDate(m_selectedMonitorDate)));
        ui->frameRoleStatus->setStyleSheet("QFrame { background-color: #15803d; border-radius: 12px; border: 1px solid rgba(255,255,255,0.15); } QLabel { color: white; border: none; }");
    } else {
        ui->lblRoleStatusIcon->setText("🚨");
        QStringList allMissing;
        for (const QString &dep : m_monitorDeptList) {
            if (dep.trimmed().isEmpty())
                continue;
            if (monitorHasAssignment(m_selectedMonitorDate, dep))
                continue;
            if (availToday.value(dep.trimmed(), 0) == 0)
                allMissing << dep;
        }
        const QString listDepts = formatDeptsListForAlert(allMissing);
        ui->lblRoleStatusText->setText(QStringLiteral("ALERTE: Aucun employé disponible pour\n%1\nle %2.")
            .arg(listDepts, displayFrDate(m_selectedMonitorDate)));
        ui->frameRoleStatus->setStyleSheet("QFrame { background-color: #b91c1c; border-radius: 12px; border: 1px solid rgba(255,255,255,0.12); } QLabel { color: white; border: none; }");
    }
}

void MainWindow::on_tableRoleAutoSuggestions_cellClicked(int row, int column) {
    if (column != 5) return;
    if (!m_db.isValid() || !m_db.isOpen()) return;
    if (!m_selectedMonitorDate.isValid() || m_selectedMonitorDepartment.isEmpty()) return;

    const QString empCin = ui->tableRoleAutoSuggestions->item(row, 0)->text().trimmed();
    const QString nom = ui->tableRoleAutoSuggestions->item(row, 1) ? ui->tableRoleAutoSuggestions->item(row, 1)->text() : QString();
    const QString prenom = ui->tableRoleAutoSuggestions->item(row, 2) ? ui->tableRoleAutoSuggestions->item(row, 2)->text() : QString();

    const auto reply = QMessageBox::question(this, QStringLiteral("Affectation automatique"),
        QStringLiteral("Affecter l'employé %1 %2 à %3 le %4 ? Un email de notification lui sera envoyé.")
            .arg(prenom, nom, m_selectedMonitorDepartment, displayFrDate(m_selectedMonitorDate)));
    if (reply != QMessageBox::Yes) return;

    QString empEmail;
    QSqlQuery qEmail(m_db);
    qEmail.prepare("SELECT adresse_email FROM employe WHERE cin=?");
    qEmail.addBindValue(empCin);
    if (qEmail.exec() && qEmail.next())
        empEmail = qEmail.value(0).toString().trimmed();

    empEmail = EmployeePassword::normalizeEmailInput(empEmail);
    if (!empEmail.isEmpty() && EmployeePassword::isEmailFormatValid(empEmail)) {
        SmtpConfig cfg;
        QString err;
        if (EmployeePassword::loadSmtpConfigAuto(&cfg, nullptr, nullptr, &err)) {
            const QString subject = QStringLiteral("ATLAS - Affectation %1").arg(m_selectedMonitorDepartment);
            const QString body = QStringLiteral(
                "Bonjour %1 %2,\n\n"
                "Vous êtes affecté(e) pour le %3 au département %4.\n\n"
                "Cordialement,\nSystème ATLAS - Port de pêche")
                .arg(prenom, nom, displayFrDate(m_selectedMonitorDate), m_selectedMonitorDepartment);
            QString sendErr;
            if (!SmtpClient::sendMail(cfg, empEmail, subject, body, &sendErr))
                QMessageBox::warning(this, QStringLiteral("Email"), tr("Notification envoyée mais l'email n'a pas pu être envoyé : %1").arg(sendErr));
        } else {
            QMessageBox::warning(this, QStringLiteral("Email"), tr("Configuration SMTP manquante (smtp.ini). L'employé ne recevra pas d'email.\n%1").arg(err));
        }
    } else if (!empEmail.isEmpty()) {
        QMessageBox::warning(this, QStringLiteral("Email"), tr("Adresse email invalide pour cet employé: %1").arg(empEmail));
    }

    if (hasAffectDotColumns(m_db)) {
        QSqlQuery clear(m_db);
        clear.prepare("UPDATE employe SET affect_dot_date = NULL, affect_dot_dept = NULL WHERE affect_dot_date >= ? AND affect_dot_date < ? AND affect_dot_dept = ?");
        clear.addBindValue(m_selectedMonitorDate);
        clear.addBindValue(m_selectedMonitorDate.addDays(1));
        clear.addBindValue(m_selectedMonitorDepartment.trimmed());
        clear.exec();
        QSqlQuery upd(m_db);
        upd.prepare("UPDATE employe SET affect_dot_date = ?, affect_dot_dept = ? WHERE cin = ?");
        upd.addBindValue(m_selectedMonitorDate);
        upd.addBindValue(m_selectedMonitorDepartment.trimmed());
        upd.addBindValue(empCin);
        if (!upd.exec())
            QMessageBox::warning(this, QStringLiteral("Affectation"), tr("Erreur enregistrement en base : %1").arg(upd.lastError().text()));
    }

    m_assignmentsSession.insert(qMakePair(m_selectedMonitorDate, m_selectedMonitorDepartment), empCin);
    if (ui->tableWidget && ui->tableWidget->rowCount() > 0) {
        const QDate w0 = parseFrDate(ui->tableWidget->item(0, 0) ? ui->tableWidget->item(0, 0)->text() : QString());
        if (w0.isValid())
            monitorMonthRangeFromDate(w0, &m_monitorWeekStart, &m_monitorDays);
    }
    rebuildMonitorCacheFromDb();
    const int currentRow = ui->tableWidget ? ui->tableWidget->currentRow() : 0;
    on_tableWidget_cellClicked(currentRow >= 0 ? currentRow : 0, 0);
    refreshEmployeeListeAlerts();
    QMessageBox::information(this, QStringLiteral("Affectation"), tr("Affectation enregistrée. L'alerte a été mise à jour."));
}

void MainWindow::on_tableRoleManualSelection_cellClicked(int row, int column) {
    if (column != 5) return;
    if (!m_db.isValid() || !m_db.isOpen()) return;
    if (!m_selectedMonitorDate.isValid() || m_selectedMonitorDepartment.isEmpty()) return;

    QTableWidgetItem *actionItem = ui->tableRoleManualSelection->item(row, 5);
    if (!actionItem || actionItem->text().trimmed() != QStringLiteral("Assigner")) {
        if (actionItem && actionItem->text().trimmed() == QStringLiteral("Non disponible"))
            QMessageBox::information(this, QStringLiteral("Non disponible"),
                QStringLiteral("Cet employé n'est pas disponible pour cette date (congé, inactif, etc.)."));
        return;
    }

    const QString empCin = ui->tableRoleManualSelection->item(row, 0)->text().trimmed();
    const QString nom = ui->tableRoleManualSelection->item(row, 1) ? ui->tableRoleManualSelection->item(row, 1)->text() : QString();
    const QString prenom = ui->tableRoleManualSelection->item(row, 2) ? ui->tableRoleManualSelection->item(row, 2)->text() : QString();

    const auto reply = QMessageBox::question(this, QStringLiteral("Affectation manuelle"),
        QStringLiteral("Assigner l'employé %1 %2 à %3 le %4 ? Un email de notification lui sera envoyé.")
            .arg(prenom, nom, m_selectedMonitorDepartment, displayFrDate(m_selectedMonitorDate)));
    if (reply != QMessageBox::Yes) return;

    QString empEmail;
    QSqlQuery qEmail(m_db);
    qEmail.prepare("SELECT adresse_email FROM employe WHERE cin=?");
    qEmail.addBindValue(empCin);
    if (qEmail.exec() && qEmail.next())
        empEmail = qEmail.value(0).toString().trimmed();

    empEmail = EmployeePassword::normalizeEmailInput(empEmail);
    if (!empEmail.isEmpty() && EmployeePassword::isEmailFormatValid(empEmail)) {
        SmtpConfig cfg;
        QString err;
        if (EmployeePassword::loadSmtpConfigAuto(&cfg, nullptr, nullptr, &err)) {
            const QString subject = QStringLiteral("ATLAS - Affectation %1").arg(m_selectedMonitorDepartment);
            const QString body = QStringLiteral(
                "Bonjour %1 %2,\n\n"
                "Vous êtes affecté(e) pour le %3 au département %4.\n\n"
                "Cordialement,\nSystème ATLAS - Port de pêche")
                .arg(prenom, nom, displayFrDate(m_selectedMonitorDate), m_selectedMonitorDepartment);
            QString sendErr;
            if (!SmtpClient::sendMail(cfg, empEmail, subject, body, &sendErr))
                QMessageBox::warning(this, QStringLiteral("Email"), tr("Notification envoyée mais l'email n'a pas pu être envoyé : %1").arg(sendErr));
        } else {
            QMessageBox::warning(this, QStringLiteral("Email"), tr("Configuration SMTP manquante (smtp.ini). L'employé ne recevra pas d'email.\n%1").arg(err));
        }
    } else if (!empEmail.isEmpty()) {
        QMessageBox::warning(this, QStringLiteral("Email"), tr("Adresse email invalide pour cet employé: %1").arg(empEmail));
    }

    if (hasAffectDotColumns(m_db)) {
        QSqlQuery clear(m_db);
        clear.prepare("UPDATE employe SET affect_dot_date = NULL, affect_dot_dept = NULL WHERE affect_dot_date >= ? AND affect_dot_date < ? AND affect_dot_dept = ?");
        clear.addBindValue(m_selectedMonitorDate);
        clear.addBindValue(m_selectedMonitorDate.addDays(1));
        clear.addBindValue(m_selectedMonitorDepartment.trimmed());
        clear.exec();

        QSqlQuery upd(m_db);
        upd.prepare("UPDATE employe SET affect_dot_date = ?, affect_dot_dept = ? WHERE cin = ?");
        upd.addBindValue(m_selectedMonitorDate);
        upd.addBindValue(m_selectedMonitorDepartment.trimmed());
        upd.addBindValue(empCin);
        if (!upd.exec())
            QMessageBox::warning(this, QStringLiteral("Affectation"), tr("Erreur enregistrement en base : %1").arg(upd.lastError().text()));
    }
    m_assignmentsSession.insert(qMakePair(m_selectedMonitorDate, m_selectedMonitorDepartment), empCin);
    if (ui->tableWidget && ui->tableWidget->rowCount() > 0) {
        const QDate w0 = parseFrDate(ui->tableWidget->item(0, 0) ? ui->tableWidget->item(0, 0)->text() : QString());
        if (w0.isValid())
            monitorMonthRangeFromDate(w0, &m_monitorWeekStart, &m_monitorDays);
    }
    rebuildMonitorCacheFromDb();
    const int currentRow = ui->tableWidget ? ui->tableWidget->currentRow() : 0;
    on_tableWidget_cellClicked(currentRow >= 0 ? currentRow : 0, 0);
    refreshEmployeeListeAlerts();
    QMessageBox::information(this, QStringLiteral("Affectation"), tr("Affectation enregistrée. L'alerte a été mise à jour."));
}

void MainWindow::assignRoleToEmployee(QString empID, QString newRole) {
    Q_UNUSED(empID)
    Q_UNUSED(newRole)
}


// ==================== AUTRES (Login, Face ID, etc.) ====================
void MainWindow::on_pushButton_clicked() {
    QSqlDatabase db = QSqlDatabase::database(Connection::connectionName());
    QString emailRaw = ui->lineEdit_5 ? ui->lineEdit_5->text().trimmed() : QString();
    QString email = emailRaw;
    if (email.isEmpty()) {
        bool ok = false;
        email = QInputDialog::getText(this, "Mot de passe oublié", "Entrez votre email:", QLineEdit::Normal, "", &ok).trimmed();
        if (!ok) return;
    }
    email = EmployeePassword::normalizeEmailInput(emailRaw);
    if (ui->lineEdit_5) ui->lineEdit_5->setText(email);
    if (!EmployeePassword::isEmailFormatValid(email)) {
        QMessageBox::warning(this, "Mot de passe oublié", "Email invalide.");
        return;
    }
    if (!db.isValid() || !db.isOpen()) {
        QMessageBox::warning(this, "Mot de passe oublié", "Connexion base de données indisponible.");
        return;
    }
    bool exists = false;
    QString dbErr;
    if (!EmployeePassword::emailExists(db, email, &exists, &dbErr)) {
        QMessageBox::warning(this, "Mot de passe oublié", "Erreur base de données:\n" + dbErr);
        return;
    }
    if (!exists) {
        QMessageBox::warning(this, "Mot de passe oublié", "Aucun compte trouvé avec cet email.");
        return;
    }
    const QString code = EmployeePassword::generateResetCode(6);
    QString sendErr;
    if (!EmployeePassword::sendResetCodeEmail(email, code, &sendErr)) {
        const QString picked = QFileDialog::getOpenFileName(this, "Choisir smtp.ini", QDir::currentPath(), "INI (*.ini);;Tous les fichiers (*.*)");
        if (!picked.trimmed().isEmpty()) {
            QSettings appSettings("ATLAS", "ATLAS");
            appSettings.setValue("smtp/iniPath", picked.trimmed());
            sendErr.clear();
            if (!EmployeePassword::sendResetCodeEmail(email, code, &sendErr)) {
                QMessageBox::warning(this, "Mot de passe oublié", "Impossible d'envoyer le mail.\n" + sendErr);
                return;
            }
        } else {
            QMessageBox::warning(this, "Mot de passe oublié", "Impossible d'envoyer le mail.\n" + sendErr);
            return;
        }
    }
    bool ok = false;
    const QString entered = QInputDialog::getText(this, "Vérification Email",
        "Un code a été envoyé à votre email.\nEntrez le code (6 chiffres):",
        QLineEdit::Normal, "", &ok).trimmed();
    if (!ok) return;
    if (entered != code) {
        QMessageBox::warning(this, "Vérification Email", "Code incorrect.");
        return;
    }
    m_resetEmail = email;
    m_resetVerified = true;
    if (ui->lineEdit_7) ui->lineEdit_7->clear();
    if (ui->lineEdit_9) ui->lineEdit_9->clear();
    ui->mainStackedWidget->setCurrentIndex(3);  // password change page
}

void MainWindow::on_pushButton_3_clicked() {
    const QString emailRaw = ui->lineEdit_5 ? ui->lineEdit_5->text().trimmed() : QString();
    QString email = emailRaw;
    const QString password = ui->lineEdit_6 ? ui->lineEdit_6->text() : QString();
    if (email.isEmpty() || password.isEmpty()) {
        QMessageBox::warning(this, "Login", "Veuillez saisir le mail et le mot de passe.");
        return;
    }
    email = EmployeePassword::normalizeEmailInput(email);
    if (ui->lineEdit_5) ui->lineEdit_5->setText(email);
    if (!EmployeePassword::isEmailFormatValid(email)) {
        QMessageBox::warning(this, "Login", "Mail invalide.");
        return;
    }
    QSqlDatabase db = QSqlDatabase::database(Connection::connectionName());
    if (!db.isValid() || !db.isOpen()) {
        QMessageBox::warning(this, "Login", "Connexion base de données indisponible.");
        return;
    }
    QSqlQuery q(db);
    q.prepare("SELECT cin, statut_professionnel FROM employe WHERE UPPER(adresse_email)=UPPER(?) AND mot_de_passe=?");
    q.addBindValue(email);
    q.addBindValue(password);
    if (!q.exec()) {
        QMessageBox::warning(this, "Login", "Erreur base de données:\n" + q.lastError().text());
        return;
    }
    if (!q.next()) {
        if (QString::compare(emailRaw, email, Qt::CaseInsensitive) != 0) {
            q.clear();
            q.prepare("SELECT cin, statut_professionnel FROM employe WHERE UPPER(adresse_email)=UPPER(?) AND mot_de_passe=?");
            q.addBindValue(emailRaw);
            q.addBindValue(password);
            if (!q.exec()) {
                QMessageBox::warning(this, "Login", "Erreur base de données:\n" + q.lastError().text());
                return;
            }
        }
        if (!q.next()) {
            QMessageBox::warning(this, "Login", "Mail ou mot de passe incorrect.");
            return;
        }
    }
    const QString statut = q.value(1).toString().trimmed().toLower();
    if (statut.contains("inactif") || statut.contains("cong")) {
        QMessageBox::warning(this, "Login", "Compte non actif (Inactif / En congé).");
        return;
    }
    ui->mainStackedWidget->setCurrentIndex(0);
    if (ui->lineEdit_6) ui->lineEdit_6->clear();
}

void MainWindow::on_pushButton_6_clicked() {
    ui->mainStackedWidget->setCurrentIndex(2);  // Face ID page
    setPhotoPlaceholder(ui->label_photo_3);
}

void MainWindow::on_label_34_linkActivated(const QString &) {
    on_pushButton_clicked();  // Forgot password
}

void MainWindow::on_pushButton_8_clicked() {
    ui->mainStackedWidget->setCurrentIndex(1);  // Return to login page
}

void MainWindow::on_pushButton_9_clicked() {
    if (!m_resetVerified || m_resetEmail.isEmpty()) {
        QMessageBox::warning(this, "Mot de passe", "Veuillez utiliser 'Mot de passe oublié' (email) avant de changer le mot de passe.");
        return;
    }
    const QString p1 = ui->lineEdit_7 ? ui->lineEdit_7->text() : QString();
    const QString p2 = ui->lineEdit_9 ? ui->lineEdit_9->text() : QString();
    if (p1.trimmed().isEmpty() || p2.trimmed().isEmpty()) {
        QMessageBox::warning(this, "Mot de passe", "Veuillez saisir et confirmer le mot de passe.");
        return;
    }
    if (p1 != p2) {
        QMessageBox::warning(this, "Mot de passe", "Les mots de passe ne correspondent pas.");
        return;
    }
    if (p1.size() < 6) {
        QMessageBox::warning(this, "Mot de passe", "Mot de passe trop court (min 6 caractères).");
        return;
    }
    QSqlDatabase db = QSqlDatabase::database(Connection::connectionName());
    if (!db.isValid() || !db.isOpen()) {
        QMessageBox::warning(this, "Mot de passe", "Connexion base de données indisponible.");
        return;
    }
    QString dbErr;
    if (!EmployeePassword::updatePasswordByEmail(db, m_resetEmail, p1, &dbErr)) {
        QMessageBox::warning(this, "Mot de passe", "Erreur base de données:\n" + dbErr);
        return;
    }
    QMessageBox::information(this, "Mot de passe", "Mot de passe modifié avec succès.");
    m_resetVerified = false;
    m_resetEmail.clear();
    if (ui->lineEdit_7) ui->lineEdit_7->clear();
    if (ui->lineEdit_9) ui->lineEdit_9->clear();
    loadEmployees(ui->search_input ? ui->search_input->text() : QString());
    ui->mainStackedWidget->setCurrentIndex(1);
}

void MainWindow::on_pushButton_11_clicked() {
    m_resetVerified = false;
    m_resetEmail.clear();
    if (ui->lineEdit_7) ui->lineEdit_7->clear();
    if (ui->lineEdit_9) ui->lineEdit_9->clear();
    ui->mainStackedWidget->setCurrentIndex(1);
}

// Script PowerShell intégré pour la reconnaissance vocale Windows (SAPI)
static const char s_voiceRecognitionScript[] = R"PS1(
# Reconnaissance vocale Windows (SAPI)
param([string]$outFile)
if (-not $outFile) { exit 1 }
try { Add-Type -AssemblyName System.Speech } catch { exit 2 }
$recognizer = New-Object System.Speech.Recognition.SpeechRecognitionEngine
try { $recognizer.SetInputToDefaultAudioDevice() } catch { exit 3 }
$grammar = New-Object System.Speech.Recognition.DictationGrammar
$recognizer.LoadGrammar($grammar) | Out-Null
$action = {
    $path = $Event.MessageData
    $text = $EventArgs.Result.Text.Trim()
    if ($text -and $path) { try { [System.IO.File]::AppendAllText($path, $text + [Environment]::NewLine, [System.Text.Encoding]::UTF8) } catch {} }
}
Register-ObjectEvent -InputObject $recognizer -EventName SpeechRecognized -Action $action -MessageData $outFile | Out-Null
$recognizer.RecognizeAsync([System.Speech.Recognition.RecognizeMode]::Multiple)
while ($true) { Start-Sleep 2 }
)PS1";

void MainWindow::on_voice_toggled(bool checked)
{
    if (!m_btnVoice || !m_voiceTimer)
        return;
    if (checked) {
        const QString appDir = QCoreApplication::applicationDirPath();
        const QString voiceTxtAbs = QFileInfo(QDir(appDir).filePath(QStringLiteral("voice.txt"))).absoluteFilePath();
        // Troncature explicite (même chemin absolu que la lecture Qt et l’argument Python).
        {
            QFile vf(voiceTxtAbs);
            if (vf.open(QIODevice::WriteOnly | QIODevice::Truncate))
                vf.close();
        }
        m_voiceLastCommand.clear();

        if (!m_voiceProcess) {
            m_voiceProcess = new QProcess(this);
            connect(m_voiceProcess, &QProcess::finished, this, &MainWindow::on_voiceProcess_finished);
        }
        if (m_voiceProcess->state() == QProcess::NotRunning) {
            m_voiceProcess->setWorkingDirectory(appDir);
            QStringList env = QProcess::systemEnvironment();
            env << QStringLiteral("PYTHONUTF8=1") << QStringLiteral("PYTHONUNBUFFERED=1");
            m_voiceProcess->setEnvironment(env);
            QString pyScript;
            for (const QString &dir : { appDir, QDir(appDir).filePath(QStringLiteral("python")),
                                         QDir::currentPath(), QDir(QDir::currentPath()).filePath(QStringLiteral("python")) }) {
                const QString candidate = QDir(dir).filePath(QStringLiteral("voice_recognition.py"));
                if (QFileInfo::exists(candidate)) {
                    pyScript = QFileInfo(candidate).absoluteFilePath();
                    break;
                }
            }
            bool started = false;
            if (!pyScript.isEmpty()) {
                m_voiceProcess->setWorkingDirectory(QFileInfo(pyScript).absolutePath());
                const QString pyLauncher = QStandardPaths::findExecutable(QStringLiteral("py"));
                // Le lanceur « py » gère mal « -u » entre -3.xx et le script : on utilise PYTHONUNBUFFERED.
                for (const QString &ver : { QStringLiteral("-3.12"), QStringLiteral("-3.11"), QStringLiteral("-3.10"), QStringLiteral("-3") }) {
                    if (!pyLauncher.isEmpty()) {
                        m_voiceProcess->start(pyLauncher, QStringList() << ver << pyScript << voiceTxtAbs);
                        if (m_voiceProcess->waitForStarted(8000)) {
                            started = true;
                            break;
                        }
                    }
                }
                if (!started) {
                    const QString pythonExe = QStandardPaths::findExecutable(QStringLiteral("python"));
                    const QString python3Exe = QStandardPaths::findExecutable(QStringLiteral("python3"));
                    const QString py = pythonExe.isEmpty() ? python3Exe : pythonExe;
                    if (!py.isEmpty()) {
                        m_voiceProcess->start(py, QStringList() << QStringLiteral("-u") << pyScript << voiceTxtAbs);
                        started = m_voiceProcess->waitForStarted(8000);
                    }
                }
                if (!started) {
                    const QString home = QDir::homePath();
                    for (const QString &rel : { QStringLiteral("AppData/Local/Programs/Python/Python312/python.exe"),
                                               QStringLiteral("AppData/Local/Programs/Python/Python311/python.exe"),
                                               QStringLiteral("AppData/Local/Programs/Python/Python310/python.exe") }) {
                        const QString fullPath = QDir(home).filePath(rel);
                        if (QFileInfo::exists(fullPath)) {
                            m_voiceProcess->start(fullPath, QStringList() << QStringLiteral("-u") << pyScript << voiceTxtAbs);
                            if (m_voiceProcess->waitForStarted(8000)) {
                                started = true;
                                break;
                            }
                        }
                    }
                }
            }
            if (!started) {
                const QString actualScriptPath = QDir::temp().filePath(QStringLiteral("atlas_voice_recognition.ps1"));
                QFile scriptFile(actualScriptPath);
                if (scriptFile.open(QIODevice::WriteOnly)) {
                    const QByteArray utf8Bom("\xEF\xBB\xBF");
                    scriptFile.write(utf8Bom);
                    scriptFile.write(s_voiceRecognitionScript, int(sizeof(s_voiceRecognitionScript)) - 1);
                    scriptFile.close();
                    const QString pw = QStandardPaths::findExecutable(QStringLiteral("powershell"));
                    const QString exe = pw.isEmpty() ? QStringLiteral("powershell") : pw;
                    m_voiceProcess->start(exe, QStringList()
                        << QStringLiteral("-ExecutionPolicy") << QStringLiteral("Bypass")
                        << QStringLiteral("-NoProfile") << QStringLiteral("-WindowStyle") << QStringLiteral("Hidden")
                        << QStringLiteral("-File") << actualScriptPath << voiceTxtAbs);
                    started = m_voiceProcess->waitForStarted(8000);
                }
                if (!started) {
                    m_btnVoice->setChecked(false);
                    if (pyScript.isEmpty())
                        m_btnVoice->setToolTip(tr("Reconnaissance vocale : placez voice_recognition.py dans le dossier python/ à côté de l'exe ou du projet"));
                    else
                        m_btnVoice->setToolTip(tr("Reconnaissance vocale : Python introuvable (installez Python 3.12, ou vérifiez PowerShell)"));
                    return;
                }
            }
        }
        m_voiceTimer->start(250);
        m_btnVoice->setToolTip(tr("Reconnaissance vocale ON — ex. : « liste employés », « disponibilité », « smart port », « exporter PDF smart port »"));
    } else {
        m_voiceTimer->stop();
        m_voiceLastCommand.clear();
        m_btnVoice->setToolTip(tr("Reconnaissance vocale OFF - clic pour activer"));
        if (m_voiceProcess && m_voiceProcess->state() != QProcess::NotRunning) {
            m_voiceProcess->terminate();
            m_voiceProcess->waitForFinished(2000);
        }
    }
}

void MainWindow::on_voiceProcess_finished(int exitCode, QProcess::ExitStatus status)
{
    if (!m_btnVoice || !m_voiceTimer)
        return;
    if (m_btnVoice->isChecked()) {
        m_voiceTimer->stop();
        m_btnVoice->setChecked(false);
        if (status == QProcess::CrashExit || exitCode != 0) {
            const QString appDir = QCoreApplication::applicationDirPath();
            const QByteArray err = m_voiceProcess ? m_voiceProcess->readAllStandardError() : QByteArray();
            const QByteArray out = m_voiceProcess ? m_voiceProcess->readAllStandardOutput() : QByteArray();
            if (!err.isEmpty() || !out.isEmpty()) {
                const QString errPath = QDir(appDir).filePath(QStringLiteral("voice_error.txt"));
                QFile f(errPath);
                if (f.open(QIODevice::WriteOnly | QIODevice::Text)) {
                    f.write("Exit code: ");
                    f.write(QString::number(exitCode).toUtf8());
                    f.write("\n\n=== stderr ===\n");
                    f.write(err);
                    f.write("\n=== stdout ===\n");
                    f.write(out);
                    f.close();
                    m_btnVoice->setToolTip(tr("Reconnaissance vocale : erreur - ouvrez voice_error.txt à côté de l'exe"));
                    return;
                }
            }
            if (exitCode == 2)
                m_btnVoice->setToolTip(tr("Reconnaissance vocale : System.Speech indisponible"));
            else if (exitCode == 3)
                m_btnVoice->setToolTip(tr("Reconnaissance vocale : micro inaccessible"));
            else if (exitCode == 10)
                m_btnVoice->setToolTip(tr("Reconnaissance vocale : installez Python (pip install -r requirements_voice.txt)"));
            else
                m_btnVoice->setToolTip(tr("Reconnaissance vocale : script arrêté (code %1)").arg(exitCode));
        } else
            m_btnVoice->setToolTip(tr("Reconnaissance vocale OFF - clic pour activer"));
    }
}

void MainWindow::processVoiceCommand(const QString &line)
{
    QString normalized = line;
    normalized.remove(QChar(0xFEFF));
    const QString cmd = normalized.trimmed().toLower();
    if (cmd.isEmpty())
        return;
    const bool emp = cmd.contains(QStringLiteral("employ")) || cmd.contains(QStringLiteral("emp"));

    const bool wantsSmartPort = cmd.contains(QStringLiteral("smart port")) || cmd.contains(QStringLiteral("smartport"))
        || cmd.contains(QStringLiteral("port intelligent"))
        || (cmd.contains(QStringLiteral("onglet")) && cmd.contains(QStringLiteral("smart")));

    const bool isEmployeeCmd = cmd.contains(QStringLiteral("liste")) || cmd.contains(QStringLiteral("tableau")) || cmd.contains(QStringLiteral("statistique")) || cmd.contains(QStringLiteral("stats"))
        || cmd.contains(QStringLiteral("planning")) || cmd.contains(QStringLiteral("plan")) || cmd.contains(QStringLiteral("métier")) || cmd.contains(QStringLiteral("metier")) || cmd.contains(QStringLiteral("disponibilité")) || cmd.contains(QStringLiteral("disponibilite"))
        || cmd.contains(QStringLiteral("ajout")) || cmd.contains(QStringLiteral("nouveau")) || cmd.contains(QStringLiteral("créer")) || cmd.contains(QStringLiteral("creer"))
        || cmd.contains(QStringLiteral("modifier")) || cmd.contains(QStringLiteral("modif")) || cmd.contains(QStringLiteral("éditer")) || cmd.contains(QStringLiteral("editer"))
        || cmd.contains(QStringLiteral("supprimer")) || cmd.contains(QStringLiteral("suppr")) || cmd.contains(QStringLiteral("rechercher")) || cmd.contains(QStringLiteral("recherche"))
        || cmd.contains(QStringLiteral("effacer")) || cmd.contains(QStringLiteral("vider")) || cmd.contains(QStringLiteral("exporter")) || cmd.contains(QStringLiteral("excel")) || cmd.contains(QStringLiteral("csv"))
        || cmd.contains(QStringLiteral("rafraîchir")) || cmd.contains(QStringLiteral("rafraichir")) || cmd.contains(QStringLiteral("recharger")) || cmd.contains(QStringLiteral("actualiser"))
        || cmd.contains(QStringLiteral("générer")) || cmd.contains(QStringLiteral("generer")) || cmd.contains(QStringLiteral("mot de passe"))
        || cmd.contains(QStringLiteral("premier")) || cmd.contains(QStringLiteral("dernier")) || cmd.contains(QStringLiteral("suivant")) || cmd.contains(QStringLiteral("précédent")) || cmd.contains(QStringLiteral("precedent"))
        || cmd.contains(QStringLiteral("ligne")) || cmd.contains(QStringLiteral("numéro")) || cmd.contains(QStringLiteral("numero")) || cmd.contains(QStringLiteral("avant")) || cmd.contains(QStringLiteral("après"))
        || cmd.contains(QStringLiteral("onglet")) || cmd.contains(QStringLiteral("identification")) || cmd.contains(QStringLiteral("coordonnées")) || cmd.contains(QStringLiteral("coordonnees")) || cmd.contains(QStringLiteral("certification"))
        || cmd.contains(QStringLiteral("photo")) || cmd.contains(QStringLiteral("esp32")) || cmd.contains(QStringLiteral("caméra")) || cmd.contains(QStringLiteral("camera"))
        || cmd.contains(QStringLiteral("visage")) || cmd.contains(QStringLiteral("facial"))
        || cmd.contains(QStringLiteral("tri")) || cmd.contains(QStringLiteral("trier")) || cmd.contains(QStringLiteral("alerte")) || cmd.contains(QStringLiteral("notification"))
        || cmd.contains(QStringLiteral("moniteur")) || cmd.contains(QStringLiteral("manque")) || cmd.contains(QStringLiteral("dotation")) || cmd.contains(QStringLiteral("remplacement"))
        || cmd.contains(QStringLiteral("statut")) || cmd.contains(QStringLiteral("statut actif")) || cmd.contains(QStringLiteral("statut inactif")) || cmd.contains(QStringLiteral("en congé")) || cmd.contains(QStringLiteral("en conge"))
        || cmd.contains(QStringLiteral("quart matin")) || cmd.contains(QStringLiteral("quart nuit")) || cmd.contains(QStringLiteral("quart rotation")) || cmd.contains(QStringLiteral("horaire matin")) || cmd.contains(QStringLiteral("horaire nuit")) || cmd.contains(QStringLiteral("horaire rotation"))
        || cmd.contains(QStringLiteral("œil")) || cmd.contains(QStringLiteral("oeil")) || cmd.contains(QStringLiteral("bascule"))
        || (cmd.contains(QStringLiteral("crud")) && emp)
        || wantsSmartPort;
    if (isEmployeeCmd && ui->stackedWidget && ui->stackedWidget->currentIndex() != 0 && ui->btnEmployes)
        on_btnEmployes_clicked();

    if (QWidget *w = QApplication::activeModalWidget()) {
        if (QDialog *dlg = qobject_cast<QDialog *>(w)) {
            if (cmd.contains(QStringLiteral("non")) || cmd == QStringLiteral("no") || cmd.startsWith(QStringLiteral("no "))
                || cmd.contains(QStringLiteral("annuler")) || cmd.contains(QStringLiteral("fermer")) || cmd.contains(QStringLiteral("ferme")) || cmd.contains(QStringLiteral("cancel"))) {
                dlg->reject();
                return;
            }
            if (cmd.contains(QStringLiteral("oui")) || cmd.contains(QStringLiteral("yes")) || cmd.contains(QStringLiteral("ok")) || cmd.contains(QStringLiteral("valider")) || cmd.contains(QStringLiteral("confirmer")) || cmd.contains(QStringLiteral("accept"))) {
                dlg->accept();
                return;
            }
        }
    }

    if (cmd.contains(QStringLiteral("paramètre")) || cmd.contains(QStringLiteral("parametre")) || cmd.contains(QStringLiteral("réglage")) || cmd.contains(QStringLiteral("reglage")) || cmd.contains(QStringLiteral("settings"))) {
        openSettingsDialog();
        return;
    }

    if (m_backgroundMusic) {
        if (cmd.contains(QStringLiteral("mute")) || cmd.contains(QStringLiteral("muse")) || cmd.contains(QStringLiteral("couper le son")) || cmd.contains(QStringLiteral("couper musique")) || cmd.contains(QStringLiteral("musique off")) || cmd.contains(QStringLiteral("son off")) || cmd.contains(QStringLiteral("silence")) || cmd.contains(QStringLiteral("couper"))) {
            on_backgroundMusic_toggled(true);
            return;
        }
        if (cmd.contains(QStringLiteral("activer musique")) || cmd.contains(QStringLiteral("musique on")) || cmd.contains(QStringLiteral("son on")) || cmd.contains(QStringLiteral("lancer la musique")) || cmd.contains(QStringLiteral("jouer musique"))) {
            on_backgroundMusic_toggled(false);
            return;
        }
    }

    if (cmd.contains(QStringLiteral("mode sombre")) || cmd.contains(QStringLiteral("sombre")) || cmd.contains(QStringLiteral("dark"))) {
        m_darkMode = true;
        applyTheme(true);
        return;
    }
    if (cmd.contains(QStringLiteral("mode normal")) || cmd.contains(QStringLiteral("clair")) || cmd.contains(QStringLiteral("light"))) {
        m_darkMode = false;
        applyTheme(false);
        return;
    }

    if ((cmd.contains(QStringLiteral("module")) && emp) || (cmd.contains(QStringLiteral("employ")) && cmd.contains(QStringLiteral("module")))) {
        if (ui->btnEmployes)
            on_btnEmployes_clicked();
        return;
    }

    if (cmd.contains(QStringLiteral("liste")) || cmd.contains(QStringLiteral("tableau")) || (cmd.contains(QStringLiteral("crud")) && emp)) {
        if (ui->btnSubEmpl3) {
            on_btnSubEmpl3_clicked();
            return;
        }
    }
    if (cmd.contains(QStringLiteral("statistique")) || cmd.contains(QStringLiteral("stats"))) {
        if (ui->btnSubEmpl4_2) {
            on_btnSubEmpl4_2_clicked();
            return;
        }
    }
    if (!wantsSmartPort && (cmd.contains(QStringLiteral("exporter")) || cmd.contains(QStringLiteral("export")) || cmd.contains(QStringLiteral("pdf")))
        && (cmd.contains(QStringLiteral("disponibilité")) || cmd.contains(QStringLiteral("disponibilite")) || cmd.contains(QStringLiteral("manque")) || cmd.contains(QStringLiteral("moniteur"))
            || cmd.contains(QStringLiteral("dotation")) || cmd.contains(QStringLiteral("remplacement")))) {
        if (ui->btnSubEmpl5)
            on_btnSubEmpl5_clicked();
        if (ui->btnMonitorExportPdf)
            on_btnMonitorExportPdf_clicked();
        return;
    }
    if (cmd.contains(QStringLiteral("planning")) || cmd.contains(QStringLiteral("plan")) || cmd.contains(QStringLiteral("métier")) || cmd.contains(QStringLiteral("metier")) || cmd.contains(QStringLiteral("disponibilité")) || cmd.contains(QStringLiteral("disponibilite"))) {
        if (ui->btnSubEmpl5) {
            on_btnSubEmpl5_clicked();
            return;
        }
    }

    if (wantsSmartPort && (cmd.contains(QStringLiteral("exporter")) || cmd.contains(QStringLiteral("export")) || cmd.contains(QStringLiteral("pdf")))) {
        if (ui->btnSubEmplSmartPort)
            on_btnSubEmplSmartPort_clicked();
        if (ui->btnSmartPortExportPdf)
            on_btnSmartPortExportPdf_clicked();
        return;
    }
    if (wantsSmartPort) {
        if (ui->btnSubEmplSmartPort)
            on_btnSubEmplSmartPort_clicked();
        return;
    }

    if (cmd.contains(QStringLiteral("ajout")) || cmd.contains(QStringLiteral("nouveau")) || cmd.contains(QStringLiteral("créer")) || cmd.contains(QStringLiteral("creer"))) {
        if (ui->btnSubEmpl3)
            on_btnSubEmpl3_clicked();
        if (ui->add_btn_2) {
            on_add_btn_2_clicked();
            return;
        }
    }
    if (cmd.contains(QStringLiteral("modifier")) || cmd.contains(QStringLiteral("modif")) || cmd.contains(QStringLiteral("éditer")) || cmd.contains(QStringLiteral("editer"))) {
        if (ui->update_btn_2) {
            on_update_btn_2_clicked();
            return;
        }
    }
    if (cmd.contains(QStringLiteral("supprimer")) || cmd.contains(QStringLiteral("suppr")) || cmd.contains(QStringLiteral("effacer employé")) || cmd.contains(QStringLiteral("delete"))) {
        if (ui->delete_btn_2) {
            on_delete_btn_2_clicked();
            return;
        }
    }
    if (cmd.contains(QStringLiteral("rechercher")) || cmd.contains(QStringLiteral("search")) || cmd.contains(QStringLiteral("recherche"))) {
        if (ui->search_btn_2) {
            on_search_btn_2_clicked();
            return;
        }
    }
    if ((cmd.contains(QStringLiteral("supprimer")) || cmd.contains(QStringLiteral("effacer")) || cmd.contains(QStringLiteral("enlever"))) && cmd.contains(QStringLiteral("photo"))) {
        if (ui->btn_delete_photo) {
            on_btn_delete_photo_clicked();
            return;
        }
    }
    if ((cmd.contains(QStringLiteral("effacer")) && !cmd.contains(QStringLiteral("employé"))) || cmd.contains(QStringLiteral("clear")) || cmd.contains(QStringLiteral("vider"))) {
        if (ui->clear_btn_2) {
            on_clear_btn_2_clicked();
            return;
        }
    }
    if (cmd.contains(QStringLiteral("exporter")) || cmd.contains(QStringLiteral("export")) || cmd.contains(QStringLiteral("excel")) || cmd.contains(QStringLiteral("csv"))) {
        if (ui->export_excel_btn) {
            on_export_excel_btn_clicked();
            return;
        }
    }
    if (cmd.contains(QStringLiteral("rafraîchir")) || cmd.contains(QStringLiteral("rafraichir")) || cmd.contains(QStringLiteral("recharger")) || cmd.contains(QStringLiteral("actualiser"))) {
        if (ui->stackedWidgetEmployes && ui->stackedWidgetEmployes->currentIndex() == 3) {
            applySmartPortPeriodFromUi();
            return;
        }
        if (ui->tableWidget_2 && ui->search_input)
            loadEmployees(ui->search_input->text(), 0, false);
        return;
    }
    if (cmd.contains(QStringLiteral("générer mot de passe")) || cmd.contains(QStringLiteral("generer mot de passe")) || cmd.contains(QStringLiteral("mot de passe aléatoire")) || cmd.contains(QStringLiteral("mot de passe aleatoire"))
        || ((cmd.contains(QStringLiteral("générer")) || cmd.contains(QStringLiteral("generer"))) && cmd.contains(QStringLiteral("mot de passe")))) {
        if (ui->btnGenPwd)
            on_btnGenPwd_clicked();
        return;
    }
    if (ui->btnGenCode && (((cmd.contains(QStringLiteral("générer")) || cmd.contains(QStringLiteral("generer"))) && cmd.contains(QStringLiteral("code")) && !cmd.contains(QStringLiteral("mot de passe")))
            || cmd.contains(QStringLiteral("code aléatoire")) || cmd.contains(QStringLiteral("code aleatoire")) || cmd.contains(QStringLiteral("nouveau code")) || cmd.contains(QStringLiteral("générer code")) || cmd.contains(QStringLiteral("generer code")))) {
        on_btnGenCode_clicked();
        return;
    }
    if (ui->sort_combo && (cmd.contains(QStringLiteral("tri")) || cmd.contains(QStringLiteral("trier")))) {
        int ix = -1;
        if (cmd.contains(QStringLiteral("id")) && (cmd.contains(QStringLiteral("desc")) || cmd.contains(QStringLiteral("décroissant")) || cmd.contains(QStringLiteral("decroissant"))))
            ix = 1;
        else if (cmd.contains(QStringLiteral("id")) || cmd.contains(QStringLiteral("identifiant")))
            ix = 0;
        else if ((cmd.contains(QStringLiteral("nom")) || cmd.contains(QStringLiteral("name"))) && (cmd.contains(QStringLiteral("desc")) || cmd.contains(QStringLiteral("z-a")) || cmd.contains(QStringLiteral("z a")) || cmd.contains(QStringLiteral("inverse"))))
            ix = 3;
        else if (cmd.contains(QStringLiteral("nom")) || cmd.contains(QStringLiteral("name")) || cmd.contains(QStringLiteral("alphabet")))
            ix = 2;
        if (ix >= 0 && ix < ui->sort_combo->count()) {
            ui->sort_combo->setCurrentIndex(ix);
            on_sort_combo_currentIndexChanged(ix);
            return;
        }
    }
    if (ui->btnEmplListeAlertBadge && (cmd == QStringLiteral("alerte") || cmd == QStringLiteral("alertes") || cmd.contains(QStringLiteral("menu alerte")) || cmd.contains(QStringLiteral("menu alertes")) || cmd.contains(QStringLiteral("alertes employ")) || (cmd.contains(QStringLiteral("alerte")) && (emp || cmd.contains(QStringLiteral("liste")))))) {
        if (auto *tb = qobject_cast<QToolButton *>(ui->btnEmplListeAlertBadge))
            tb->showMenu();
        return;
    }
    if (ui->btnPwdEye && (cmd.contains(QStringLiteral("œil mot de passe")) || cmd.contains(QStringLiteral("oeil mot de passe")) || cmd.contains(QStringLiteral("bascule mot de passe")) || cmd.contains(QStringLiteral("afficher mot de passe champ")) || cmd.contains(QStringLiteral("masquer mot de passe champ")))) {
        ui->btnPwdEye->click();
        return;
    }
    if (ui->btnCodeEye && (cmd.contains(QStringLiteral("œil code")) || cmd.contains(QStringLiteral("oeil code")) || cmd.contains(QStringLiteral("bascule code")) || cmd.contains(QStringLiteral("afficher code champ")) || cmd.contains(QStringLiteral("masquer code champ")))) {
        ui->btnCodeEye->click();
        return;
    }
    if ((cmd.contains(QStringLiteral("mot de passe")) || cmd.contains(QStringLiteral("mots de passe"))) && (cmd.contains(QStringLiteral("liste")) || cmd.contains(QStringLiteral("tableau")) || cmd.contains(QStringLiteral("colonnes")))) {
        on_employeeTableHeaderSectionClicked(10);
        return;
    }
    if (cmd.contains(QStringLiteral("code")) && (cmd.contains(QStringLiteral("liste")) || cmd.contains(QStringLiteral("tableau")) || cmd.contains(QStringLiteral("colonnes")))) {
        on_employeeTableHeaderSectionClicked(18);
        return;
    }

    if (cmd.contains(QStringLiteral("upload photo")) || cmd.contains(QStringLiteral("upload image")) || cmd.contains(QStringLiteral("appload photo")) || cmd.contains(QStringLiteral("charger photo")) || cmd.contains(QStringLiteral("charger image")) || cmd.contains(QStringLiteral("importer photo"))) {
        if (ui->btn_upload_photo) {
            on_btn_upload_photo_clicked();
            return;
        }
    }
    if (cmd.contains(QStringLiteral("caméra")) || cmd.contains(QStringLiteral("camera")) || cmd.contains(QStringLiteral("prendre photo")) || cmd.contains(QStringLiteral("photo caméra")) || cmd.contains(QStringLiteral("photo camera"))) {
        if (ui->btn_camera_photo) {
            on_btn_camera_photo_clicked();
            return;
        }
    }
    if (cmd.contains(QStringLiteral("esp32")) || cmd.contains(QStringLiteral("photo esp32"))) {
        if (ui->btn_esp32_photo) {
            on_btn_esp32_photo_clicked();
            return;
        }
    }
    if ((cmd.contains(QStringLiteral("afficher")) || cmd.contains(QStringLiteral("masquer")) || cmd.contains(QStringLiteral("cacher")) || cmd.contains(QStringLiteral("voir")) || cmd.contains(QStringLiteral("toggle")))
        && cmd.contains(QStringLiteral("photo")) && !cmd.contains(QStringLiteral("caméra")) && !cmd.contains(QStringLiteral("camera")) && !cmd.contains(QStringLiteral("charger")) && !cmd.contains(QStringLiteral("upload")) && !cmd.contains(QStringLiteral("importer"))) {
        if (ui->btn_toggle_photo_visible) {
            on_btn_toggle_photo_visible_clicked();
            return;
        }
    }
    if (ui->btn_face_verify && (cmd.contains(QStringLiteral("vérifier visage")) || cmd.contains(QStringLiteral("verifier visage")) || cmd.contains(QStringLiteral("reconnaissance visage")) || cmd.contains(QStringLiteral("contrôle visage")) || cmd.contains(QStringLiteral("controle visage")))) {
        on_btn_face_verify_clicked();
        return;
    }
    if (ui->btn_face_upload && (cmd.contains(QStringLiteral("upload visage")) || cmd.contains(QStringLiteral("charger visage")) || cmd.contains(QStringLiteral("importer visage")))) {
        on_btn_face_upload_clicked();
        return;
    }
    if (ui->btn_face_camera && (cmd.contains(QStringLiteral("caméra visage")) || cmd.contains(QStringLiteral("camera visage")) || cmd.contains(QStringLiteral("photo visage")))) {
        on_btn_face_camera_clicked();
        return;
    }
    if (ui->btn_face_esp32 && (cmd.contains(QStringLiteral("esp32 visage")) || cmd.contains(QStringLiteral("visage esp32")))) {
        on_btn_face_esp32_clicked();
        return;
    }

    if (ui->tableWidget_2 && ui->stackedWidgetEmployes && ui->stackedWidgetEmployes->currentIndex() == 0) {
        const int rowCount = ui->tableWidget_2->rowCount();
        if (rowCount == 0)
            return;
        int row = -1;
        if (cmd.contains(QStringLiteral("premier")) || cmd.contains(QStringLiteral("première")) || cmd.contains(QStringLiteral("ligne 1")) || cmd.contains(QStringLiteral("ligne un")) || cmd.contains(QStringLiteral("sélectionne premier")) || cmd.contains(QStringLiteral("selectionne premier")))
            row = 0;
        else if (cmd.contains(QStringLiteral("dernier")) || cmd.contains(QStringLiteral("dernière")) || cmd.contains(QStringLiteral("sélectionne dernier")) || cmd.contains(QStringLiteral("selectionne dernier")))
            row = rowCount - 1;
        else if (cmd.contains(QStringLiteral("suivant")) || cmd.contains(QStringLiteral("ligne suivante")) || cmd.contains(QStringLiteral("après"))) {
            const int cur = ui->tableWidget_2->currentRow();
            if (cur >= 0 && cur + 1 < rowCount)
                row = cur + 1;
        } else if (cmd.contains(QStringLiteral("précédent")) || cmd.contains(QStringLiteral("precedent")) || cmd.contains(QStringLiteral("ligne précédente")) || cmd.contains(QStringLiteral("avant"))) {
            const int cur = ui->tableWidget_2->currentRow();
            if (cur > 0)
                row = cur - 1;
        } else {
            for (int i = 1; i <= 20 && i <= rowCount; ++i) {
                const QString n = QString::number(i);
                QString un;
                if (i == 1) un = QStringLiteral("un");
                else if (i == 2) un = QStringLiteral("deux");
                else if (i == 3) un = QStringLiteral("trois");
                else if (i == 4) un = QStringLiteral("quatre");
                else if (i == 5) un = QStringLiteral("cinq");
                else if (i == 6) un = QStringLiteral("six");
                else if (i == 7) un = QStringLiteral("sept");
                else if (i == 8) un = QStringLiteral("huit");
                else if (i == 9) un = QStringLiteral("neuf");
                else if (i == 10) un = QStringLiteral("dix");
                if (cmd.contains(QStringLiteral("ligne ") + n) || cmd.contains(QStringLiteral("ligne ") + un) || cmd.contains(QStringLiteral("numéro ") + n) || cmd.contains(QStringLiteral("numero ") + n)) {
                    row = i - 1;
                    break;
                }
            }
        }
        if (row >= 0 && row < rowCount) {
            ui->tableWidget_2->setCurrentCell(row, 0);
            on_tableWidget_2_cellClicked(row, 0);
            return;
        }
    }

    if (ui->info_frame_2) {
        QTabWidget *tw = ui->info_frame_2->findChild<QTabWidget *>(QStringLiteral("tabWidget_employee"));
        if (tw && tw->count() >= 2) {
            if (cmd.contains(QStringLiteral("onglet identification")) || (cmd.contains(QStringLiteral("identification")) && !cmd.contains(QStringLiteral("coordonnées")))) {
                tw->setCurrentIndex(0);
                return;
            }
            if (cmd.contains(QStringLiteral("onglet coordonnées")) || cmd.contains(QStringLiteral("coordonnées")) || cmd.contains(QStringLiteral("coordonnees"))) {
                tw->setCurrentIndex(1);
                return;
            }
            if (cmd.contains(QStringLiteral("onglet certification")) || cmd.contains(QStringLiteral("certification"))) {
                if (tw->count() > 2)
                    tw->setCurrentIndex(2);
                else
                    tw->setCurrentIndex(1);
                return;
            }
        }
    }

    if (ui->stackedWidgetEmployes && ui->stackedWidgetEmployes->currentIndex() == 0) {
        if (ui->radio_status_actif && (cmd.contains(QStringLiteral("statut actif")) || ((cmd.contains(QStringLiteral("employé")) || cmd.contains(QStringLiteral("employe"))) && cmd.contains(QStringLiteral("actif")) && !cmd.contains(QStringLiteral("inactif"))))) {
            ui->radio_status_actif->setChecked(true);
            return;
        }
        if (ui->radio_status_inactif && (cmd.contains(QStringLiteral("statut inactif")) || ((cmd.contains(QStringLiteral("employé")) || cmd.contains(QStringLiteral("employe"))) && cmd.contains(QStringLiteral("inactif"))))) {
            ui->radio_status_inactif->setChecked(true);
            return;
        }
        if (ui->radio_status_conge && (cmd.contains(QStringLiteral("statut congé")) || cmd.contains(QStringLiteral("statut conge")) || cmd.contains(QStringLiteral("en congé")) || cmd.contains(QStringLiteral("en conge")))) {
            ui->radio_status_conge->setChecked(true);
            return;
        }
        if (ui->radio_shift_matin && (cmd.contains(QStringLiteral("quart matin")) || cmd.contains(QStringLiteral("shift matin")) || cmd.contains(QStringLiteral("horaire matin")) || cmd.contains(QStringLiteral("poste matin")))) {
            ui->radio_shift_matin->setChecked(true);
            return;
        }
        if (ui->radio_shift_nuit && (cmd.contains(QStringLiteral("quart nuit")) || cmd.contains(QStringLiteral("shift nuit")) || cmd.contains(QStringLiteral("horaire nuit")) || cmd.contains(QStringLiteral("poste nuit")))) {
            ui->radio_shift_nuit->setChecked(true);
            return;
        }
        if (ui->radio_shift_rotation && (cmd.contains(QStringLiteral("quart rotation")) || cmd.contains(QStringLiteral("shift rotation")) || cmd.contains(QStringLiteral("horaire rotation")) || cmd.contains(QStringLiteral("poste rotation")))) {
            ui->radio_shift_rotation->setChecked(true);
            return;
        }
    }

    const QString toType = normalized.trimmed();
    if (!toType.isEmpty() && QApplication::focusWidget()) {
        QWidget *fw = QApplication::focusWidget();
        if (QLineEdit *le = qobject_cast<QLineEdit *>(fw)) {
            le->setText(toType);
            return;
        }
        if (QComboBox *cb = qobject_cast<QComboBox *>(fw)) {
            if (cb->isEditable()) {
                cb->setCurrentText(toType);
                return;
            }
        }
        if (QPlainTextEdit *pe = qobject_cast<QPlainTextEdit *>(fw)) {
            pe->setPlainText(toType);
            return;
        }
        if (QTextEdit *te = qobject_cast<QTextEdit *>(fw)) {
            te->setText(toType);
            return;
        }
    }
}

// ============ PECHEUR CRUD OPERATIONS ============

void MainWindow::on_btn_add_2_clicked()
{
    // UI -> DB (Create): this action calls Ptmp.ajouter(...), which executes INSERT INTO PECHEUR.
    // Récupération des valeurs du formulaire
    QString nom = ui->le_nom_2->text().trimmed();
    QString prenom = ui->le_prenom_2->text().trimmed();
    QString role = ui->cb_role_2->currentText();
    QString statut = ui->radio_status_actif_2->isChecked() ? "Actif" : "Inactif";
    int experience = ui->sb_experience_2->value();
    QDate dateLimite = ui->dateEdit_limit_2->date();
    QString telephone = ui->le_telephone_2->text().trimmed();
    QString rfid = ui->le_rfid_2 ? ui->le_rfid_2->text().trimmed().toUpper().remove(' ') : QString();
    
    if (nom.isEmpty() || prenom.isEmpty()) {
        QMessageBox::warning(this, "Champs manquants", "Veuillez remplir au moins le nom et le prénom.");
        return;
    }
    if (nom.length() < 2) {
        QMessageBox::warning(this, "Nom invalide", "Le nom doit contenir au moins 2 caractères.");
        return;
    }
    if (nom.length() > 100) {
        QMessageBox::warning(this, "Nom invalide", "Le nom ne doit pas dépasser 100 caractères.");
        return;
    }
    if (prenom.length() < 2) {
        QMessageBox::warning(this, "Prénom invalide", "Le prénom doit contenir au moins 2 caractères.");
        return;
    }
    if (prenom.length() > 100) {
        QMessageBox::warning(this, "Prénom invalide", "Le prénom ne doit pas dépasser 100 caractères.");
        return;
    }
    if (!telephone.isEmpty()) {
        QString digitsOnly = telephone;
        digitsOnly.remove(QRegularExpression(QStringLiteral("[^0-9]")));
        if (digitsOnly.length() != 8) {
            QMessageBox::warning(this, "Contact invalide", "Le numéro doit être un numéro tunisien (+216) à 8 chiffres (ex: 12 345 678).");
            return;
        }
    }
    if (!rfid.isEmpty()) {
        for (QChar c : rfid) {
            if (!c.isDigit() && (c < QLatin1Char('A') || c > QLatin1Char('F'))) {
                QMessageBox::warning(this, "RFID invalide", "Le badge RFID doit contenir uniquement des caractères hexadécimaux (0-9, A-F).");
                return;
            }
        }
    }
    if (dateLimite < QDate::currentDate()) {
        QMessageBox::warning(this, "Date limite invalide", "La date limite doit être supérieure ou égale à la date du jour.");
        return;
    }
    
    // ID auto-généré par la séquence Oracle seq_pecheur_id
    bool test = Ptmp.ajouter(QString(), nom, prenom, role, statut, experience, dateLimite, telephone, rfid, QString());
    
    if (test) {
        QMessageBox::information(this, "Succès", "Pêcheur ajouté avec succès.");
        // Actualiser l'affichage après l'ajout
        afficherPecheurs();
        // Vider les champs du formulaire
        ui->le_nom_2->clear();
        ui->le_prenom_2->clear();
        ui->sb_experience_2->setValue(0);
        ui->dateEdit_limit_2->setDate(QDate::currentDate());
        ui->le_telephone_2->clear();
        if (ui->le_rfid_2)
            ui->le_rfid_2->clear();
        ui->radio_status_actif_2->setChecked(true);
    } else {
        QString msg = Ptmp.lastError().isEmpty() ? "Échec de l'ajout du pêcheur." : Ptmp.lastError();
        QMessageBox::critical(this, "Erreur", msg);
    }
}

void MainWindow::on_btn_delete_2_clicked()
{
    // UI -> DB (Delete): this action calls Ptmp.supprimer(id), which executes DELETE FROM PECHEUR.
    // Récupérer l'ID du pêcheur sélectionné dans la table
    int row = ui->tableWidget_4->currentRow();
    
    if (row < 0) {
        QMessageBox::warning(this, "Sélection requise", "Veuillez sélectionner un pêcheur à supprimer.");
        return;
    }
    
    QString id = ui->tableWidget_4->item(row, 0)->text();
    
    // Demander confirmation
    int ret = QMessageBox::question(this, "Confirmation", 
                                     "Êtes-vous sûr de vouloir supprimer ce pêcheur ?",
                                     QMessageBox::Yes | QMessageBox::No);
    
    if (ret == QMessageBox::Yes) {
        // Appel de la méthode supprimer() du modèle
        bool test = Ptmp.supprimer(id);
        
        if (test) {
            QMessageBox::information(this, "Succès", "Pêcheur supprimé avec succès.");
            // Actualiser l'affichage après la suppression
            afficherPecheurs();
        } else {
            QMessageBox::critical(this, "Erreur", "Échec de la suppression du pêcheur.");
        }
    }
}

void MainWindow::on_btn_edit_2_clicked()
{
    // UI -> DB (Update): this action calls Ptmp.modifier(...), which executes UPDATE PECHEUR.
    // Récupérer l'ID du pêcheur sélectionné dans la table
    int row = ui->tableWidget_4->currentRow();
    
    if (row < 0) {
        QMessageBox::warning(this, "Sélection requise", "Veuillez sélectionner un pêcheur à modifier.");
        return;
    }
    
    QString id = ui->tableWidget_4->item(row, 0)->text();
    
    // Récupération des valeurs du formulaire
    QString nom = ui->le_nom_2->text().trimmed();
    QString prenom = ui->le_prenom_2->text().trimmed();
    QString role = ui->cb_role_2->currentText();
    QString statut = ui->radio_status_actif_2->isChecked() ? "Actif" : "Inactif";
    int experience = ui->sb_experience_2->value();
    QDate dateLimite = ui->dateEdit_limit_2->date();
    QString telephone = ui->le_telephone_2->text().trimmed();
    QString rfid = ui->le_rfid_2 ? ui->le_rfid_2->text().trimmed().toUpper().remove(' ') : QString();
    
    if (nom.isEmpty() || prenom.isEmpty()) {
        QMessageBox::warning(this, "Champs manquants", "Veuillez remplir au moins le nom et le prénom.");
        return;
    }
    if (nom.length() < 2) {
        QMessageBox::warning(this, "Nom invalide", "Le nom doit contenir au moins 2 caractères.");
        return;
    }
    if (nom.length() > 100) {
        QMessageBox::warning(this, "Nom invalide", "Le nom ne doit pas dépasser 100 caractères.");
        return;
    }
    if (prenom.length() < 2) {
        QMessageBox::warning(this, "Prénom invalide", "Le prénom doit contenir au moins 2 caractères.");
        return;
    }
    if (prenom.length() > 100) {
        QMessageBox::warning(this, "Prénom invalide", "Le prénom ne doit pas dépasser 100 caractères.");
        return;
    }
    if (!telephone.isEmpty()) {
        QString digitsOnly = telephone;
        digitsOnly.remove(QRegularExpression(QStringLiteral("[^0-9]")));
        if (digitsOnly.length() != 8) {
            QMessageBox::warning(this, "Contact invalide", "Le numéro doit être un numéro tunisien (+216) à 8 chiffres (ex: 12 345 678).");
            return;
        }
    }
    if (dateLimite < QDate::currentDate()) {
        QMessageBox::warning(this, "Date limite invalide", "La date limite doit être supérieure ou égale à la date du jour.");
        return;
    }
    if (!rfid.isEmpty()) {
        for (QChar c : rfid) {
            if (!c.isDigit() && (c < QLatin1Char('A') || c > QLatin1Char('F'))) {
                QMessageBox::warning(this, "RFID invalide", "Le badge RFID doit contenir uniquement des caractères hexadécimaux (0-9, A-F).");
                return;
            }
        }
    }
    QString cin;
    bool test = Ptmp.modifier(id, nom, prenom, role, statut, experience, dateLimite, telephone, rfid, cin);
    
    if (test) {
        QMessageBox::information(this, "Succès", "Pêcheur modifié avec succès.");
        afficherPecheurs();
        ui->le_nom_2->clear();
        ui->le_prenom_2->clear();
        ui->sb_experience_2->setValue(0);
        ui->dateEdit_limit_2->setDate(QDate::currentDate());
        ui->le_telephone_2->clear();
        if (ui->le_rfid_2)
            ui->le_rfid_2->clear();
        ui->radio_status_actif_2->setChecked(true);
    } else {
        QMessageBox::critical(this, "Erreur", "Échec de la modification du pêcheur.");
    }
}

void MainWindow::updatePecheurWarningBlink()
{
    if (!ui->tableWidget_4)
        return;
    m_pecheurBlinkOn = !m_pecheurBlinkOn;
    const QColor warningRed(0xb2, 0x22, 0x22);   // firebrick
    const QColor defaultBg(0x2d, 0x2d, 0x2d);
    const QColor blinkColor = m_pecheurBlinkOn ? warningRed : defaultBg;
    const int colCount = ui->tableWidget_4->columnCount();
    for (int r : m_pecheurWarningRows) {
        if (r < 0 || r >= ui->tableWidget_4->rowCount())
            continue;
        for (int c = 0; c < colCount; ++c) {
            QTableWidgetItem *item = ui->tableWidget_4->item(r, c);
            if (item)
                item->setBackground(QBrush(blinkColor));
        }
    }
}

void MainWindow::on_tableWidget_4_cellClicked(int row, int /* column */)
{
    if (row >= 0 && ui->tableWidget_4->item(row, 0)) {
        ui->le_nom_2->setText(ui->tableWidget_4->item(row, 1)->text());
        ui->le_prenom_2->setText(ui->tableWidget_4->item(row, 2)->text());
        QString role = ui->tableWidget_4->item(row, 3)->text();
        int roleIndex = ui->cb_role_2->findText(role);
        if (roleIndex >= 0)
            ui->cb_role_2->setCurrentIndex(roleIndex);
        QString experienceStr = ui->tableWidget_4->item(row, 4)->text();
        experienceStr.remove(" ans");
        ui->sb_experience_2->setValue(experienceStr.toInt());
        QString statut = ui->tableWidget_4->item(row, 5)->text();
        if (statut == "Actif")
            ui->radio_status_actif_2->setChecked(true);
        else
            ui->radio_status_inactif_2->setChecked(true);
        if (ui->tableWidget_4->item(row, 6))
            ui->le_telephone_2->setText(ui->tableWidget_4->item(row, 6)->text());
        if (ui->le_rfid_2 && ui->tableWidget_4->item(row, 7))
            ui->le_rfid_2->setText(ui->tableWidget_4->item(row, 7)->text());
        if (ui->tableWidget_4->item(row, 8)) {
            const QDate d = ui->tableWidget_4->item(row, 8)->data(Qt::UserRole).toDate();
            if (d.isValid())
                ui->dateEdit_limit_2->setDate(d);
        }
    }
}

void MainWindow::on_btn_export_2_clicked()
{
    // Export is DB-driven and always fetches the full Pecheur list (no search filter).
    // UI -> DB (Export source): this action calls Ptmp.afficherAvecFiltres("", sortIndex) before PDF rendering.
    // SQL used by exporter (via Ptmp.afficherAvecFiltres("", sortIndex)):
    // SELECT ID_PECHEUR, NOMPRENOM, ROLE, ANNEES_EXPERIENCE, STATUT, DATE_LIMITE, CONTACT, CIN
    // FROM PECHEUR
    // ORDER BY ID_PECHEUR
    // Possible ORDER BY replacements depending on sortIndex:
    // 0 -> ORDER BY UPPER(NOMPRENOM) ASC
    // 1 -> ORDER BY UPPER(NVL(REGEXP_SUBSTR(TRIM(NOMPRENOM), '[^ ]+', 1, 2), NOMPRENOM)) ASC
    // 2 -> ORDER BY ANNEES_EXPERIENCE ASC
    // 3 -> ORDER BY DATE_LIMITE ASC NULLS LAST
    QString defaultName = QString("liste_pecheurs_%1.pdf")
        .arg(QDate::currentDate().toString("yyyy-MM-dd"));
    QString filePath = QFileDialog::getSaveFileName(this,
        tr("Exporter la liste des pêcheurs en PDF"),
        defaultName,
        tr("PDF (*.pdf)"));

    if (filePath.isEmpty())
        return;

    if (!filePath.endsWith(".pdf", Qt::CaseInsensitive))
        filePath += ".pdf";

    QPrinter printer(QPrinter::HighResolution);
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setOutputFileName(filePath);
    printer.setPageSize(QPageSize(QPageSize::A4));
    printer.setPageOrientation(QPageLayout::Landscape);
    printer.setPageMargins(QMarginsF(9, 9, 10, 10), QPageLayout::Millimeter);
    printer.setResolution(150);

    const QString searchText;
    const int sortIndex = ui->cb_sort_2 ? ui->cb_sort_2->currentIndex() : -1;
    QSqlQuery dataQuery = Ptmp.afficherAvecFiltres(searchText, sortIndex);
    if (!Ptmp.lastError().isEmpty()) {
        QMessageBox::warning(this, tr("Export PDF"),
            tr("Impossible de charger les données depuis la base :\n%1").arg(Ptmp.lastError()));
        return;
    }

    QVector<QStringList> rows;
    while (dataQuery.next()) {
        QString nomprenom = dataQuery.value(1).toString().trimmed();
        int spaceIdx = nomprenom.indexOf(' ');
        QString nom = spaceIdx > 0 ? nomprenom.left(spaceIdx) : nomprenom;
        QString prenom = spaceIdx > 0 ? nomprenom.mid(spaceIdx + 1) : QString();
        const QDate dateLimite = dataQuery.value(5).toDate();
        rows.append({
            dataQuery.value(0).toString().trimmed(),
            nom,
            prenom,
            dataQuery.value(2).toString().trimmed(),
            QString::number(dataQuery.value(3).toInt()) + QStringLiteral(" ans"),
            dataQuery.value(4).toString().trimmed(),
            dataQuery.value(6).toString().trimmed(),
            dataQuery.value(7).toString().trimmed(),
            dateLimite.isValid() ? dateLimite.toString(QStringLiteral("dd/MM/yyyy")) : QString()
        });
    }

    const QVector<int> exportCols = {0, 1, 2, 3, 4, 5, 6, 7, 8};
    const QStringList headers = {
        tr("ID"), tr("Nom"), tr("Prénom"), tr("Rôle"),
        tr("Expérience"), tr("Statut"), tr("Téléphone"), tr("RFID"), tr("Date limite")
    };
    const QVector<double> minW = {58, 86, 86, 96, 104, 90, 128, 110, 92};
    const QVector<double> maxW = {88, 170, 170, 170, 165, 150, 220, 200, 145};

    QVector<double> colPx;
    colPx.reserve(exportCols.size());
    QFontMetrics fm(QFont("Helvetica", 9));
    for (int i = 0; i < exportCols.size(); ++i) {
        const int col = exportCols[i];
        double desired = fm.horizontalAdvance(headers[i]) + 26.0;
        for (int r = 0; r < rows.size(); ++r) {
            const QString text = (col < rows[r].size()) ? rows[r][col].trimmed() : QString();
            if (!text.isEmpty())
                desired = qMax(desired, double(fm.horizontalAdvance(text.left(26))) + 24.0);
        }
        desired = qBound(minW[i], desired, maxW[i]);
        colPx.push_back(desired);
    }

    double totalPx = 0.0;
    for (double w : colPx)
        totalPx += w;
    QVector<double> colPct;
    colPct.reserve(colPx.size());
    for (double w : colPx)
        colPct.push_back((w / totalPx) * 100.0);

    QString html;
    html += QStringLiteral("<!DOCTYPE html><html><head><meta charset=\"utf-8\">"
                           "<style>"
                           "@page { size: A4 landscape; margin: 9mm 9mm 10mm 10mm; }"
                           "body { font-family: 'Helvetica', 'Segoe UI', sans-serif; color:#1f2937; font-size:9pt; margin:0; }"
                           ".header { background: linear-gradient(180deg,#1a3a52,#2c5f7d); color:#ffffff; border-radius:8px; padding:12px 16px; }"
                           ".title { font-size:18pt; font-weight:700; margin:0; }"
                           ".sub { font-size:10pt; color:#e2e8f0; margin-top:3px; }"
                           ".meta { margin:10px 0 8px 0; font-size:9pt; color:#334155; }"
                           ".rule { border-top:2px solid #2c5f7d; margin:0 0 8px 0; }"
                           "table { width:100%; border-collapse:collapse; table-layout:fixed; }"
                           "thead { display: table-header-group; }"
                           "tfoot { display: table-footer-group; }"
                           "tr { page-break-inside: avoid; }"
                           "th, td { border:1px solid #cbd5e1; padding:8px 9px; vertical-align:top; text-align:left; word-break:break-word; overflow-wrap:anywhere; white-space:normal; }"
                           "th { background:#2c5f7d; color:#ffffff; font-size:9.2pt; font-weight:700; }"
                           "tbody tr:nth-child(even) td { background:#f8fafc; }"
                           "td.c-id, th.c-id { text-align:center; }"
                           ".footer { margin-top:8px; border-top:1px solid #cbd5e1; padding-top:6px; font-weight:600; color:#374151; }"
                           "</style></head><body>");

    html += QStringLiteral("<div class=\"header\"><p class=\"title\">%1</p><div class=\"sub\">%2</div></div>")
                .arg(tr("Liste des pêcheurs").toHtmlEscaped(), tr("Smart Fishing & Port Management").toHtmlEscaped());
    html += QStringLiteral("<div class=\"meta\">%1</div>")
                .arg(tr("Date du rapport : %1").arg(QLocale(QLocale::French).toString(QDate::currentDate(), QLocale::LongFormat)).toHtmlEscaped());
    html += QStringLiteral("<div class=\"rule\"></div><table><colgroup>");
    for (int i = 0; i < colPct.size(); ++i)
        html += QStringLiteral("<col style=\"width:%1%;\">").arg(QString::number(colPct[i], 'f', 2));
    html += QStringLiteral("</colgroup><thead><tr>");
    for (int i = 0; i < headers.size(); ++i) {
        const QString cls = (i == 0) ? QStringLiteral(" class=\"c-id\"") : QString();
        html += QStringLiteral("<th%1>%2</th>").arg(cls, headers[i].toHtmlEscaped());
    }
    html += QStringLiteral("</tr></thead><tbody>");

    int exportedRows = 0;
    for (int r = 0; r < rows.size(); ++r) {
        html += QStringLiteral("<tr>");
        for (int i = 0; i < exportCols.size(); ++i) {
            const int col = exportCols[i];
            QString text = (col < rows[r].size()) ? rows[r][col].trimmed() : QString();
            if (text.isEmpty())
                text = QStringLiteral("-");
            text = text.toHtmlEscaped();
            text.replace('\n', QStringLiteral("<br>"));
            const QString cls = (i == 0) ? QStringLiteral(" class=\"c-id\"") : QString();
            html += QStringLiteral("<td%1>%2</td>").arg(cls, text);
        }
        html += QStringLiteral("</tr>");
        exportedRows++;
    }

    html += QStringLiteral("</tbody></table>");
    html += QStringLiteral("<div class=\"footer\">%1</div>")
                .arg(tr("Total : %1 pêcheur(s)").arg(exportedRows).toHtmlEscaped());
    html += QStringLiteral("</body></html>");

    QTextDocument doc;
    doc.setDocumentMargin(0);
    doc.setDefaultFont(QFont("Helvetica", 9));
    doc.setHtml(html);
    doc.print(&printer);

    QMessageBox::information(this, tr("Export PDF"),
        tr("La liste des pêcheurs a été exportée vers :\n%1").arg(filePath));
}

void MainWindow::filterPecheursTable(const QString &searchText)
{
    // Search pipeline: delegate to dynamic SQL reload instead of local row filtering.
    const int sortIndex = ui->cb_sort_2 ? ui->cb_sort_2->currentIndex() : -1;
    afficherPecheurs(searchText, sortIndex);
}

void MainWindow::on_btn_recherche_2_clicked()
{
    // Search button -> dynamic SQL WHERE.
    // UI -> DB: filterPecheursTable(...) -> afficherPecheurs(...) -> Ptmp.afficherAvecFiltres(search, sort)
    filterPecheursTable(ui->le_search_2->text());
}

void MainWindow::on_le_search_2_textChanged(const QString &text)
{
    // Live search -> dynamic SQL WHERE.
    filterPecheursTable(text);
}

void MainWindow::on_cb_sort_2_currentIndexChanged(int index)
{
    if (index < 0)
        return;
    // Sort combo -> dynamic SQL ORDER BY.
    // UI -> DB: afficherPecheurs(currentSearch, index) -> Ptmp.afficherAvecFiltres(...)
    const QString searchText = ui->le_search_2 ? ui->le_search_2->text() : QString();
    afficherPecheurs(searchText, index);
}

// ==================== MODULE BATEAU ====================
void MainWindow::on_btnAjouter_8_clicked()
{
    if (m_bateauModule)
        m_bateauModule->onAjouterClicked();
}

void MainWindow::on_btnModifier_15_clicked()
{
    if (m_bateauModule)
        m_bateauModule->onModifierClicked();
}

void MainWindow::on_btnSupprimer_15_clicked()
{
    if (m_bateauModule)
        m_bateauModule->onSupprimerClicked();
}

void MainWindow::on_btnExportPDF_15_clicked()
{
    if (m_bateauModule)
        m_bateauModule->onExportPdfClicked();
}

void MainWindow::on_lineRecherche_15_textChanged(const QString &text)
{
    if (m_bateauModule)
        m_bateauModule->onRechercheTextChanged(text);
}

void MainWindow::on_comboTri_15_currentIndexChanged(int index)
{
    if (m_bateauModule)
        m_bateauModule->onTriIndexChanged(index);
}

void MainWindow::on_btnAIAssistant_8_clicked()
{
    if (m_bateauModule)
        m_bateauModule->onBoatRapportClicked();
}

// ==================== MODULE QUAI ====================
QString MainWindow::quaiWhereClause() const
{
    const QString colNom = Quai::sqlQuaiColQuaiNom();
    const QString colType = Quai::sqlQuaiColQuaiType();
    const QString colStatut = Quai::sqlQuaiColStatut();
    const QString colSafety = Quai::sqlQuaiColSafetyLevel();
    const auto qcol = [](const QString &c, const char *fb) { return c.isEmpty() ? QString::fromLatin1(fb) : c; };
    const QString qNom = qcol(colNom, "QUAI_NOM");
    const QString qType = qcol(colType, "QUAI_TYPE");
    const QString qStatut = qcol(colStatut, "STATUT");

    QStringList andParts;
    if (ui->filterComboType && ui->filterComboType->currentIndex() > 0) {
        QString t = ui->filterComboType->currentText().trimmed();
        if (!t.isEmpty()) { t.replace("'", "''"); andParts << "UPPER(" + qType + ") = UPPER('" + t + "')"; }
    }
    if (ui->filterComboStatut && ui->filterComboStatut->currentIndex() > 0) {
        QString s = ui->filterComboStatut->currentText().trimmed();
        if (!s.isEmpty()) { s.replace("'", "''"); andParts << "UPPER(" + qStatut + ") = UPPER('" + s + "')"; }
    }
    if (ui->filterComboSecurite && ui->filterComboSecurite->currentIndex() > 0) {
        QString s = ui->filterComboSecurite->currentText().trimmed();
        if (!s.isEmpty() && !colSafety.isEmpty()) {
            s.replace("'", "''");
            andParts << "UPPER(" + colSafety + ") = UPPER('" + s + "')";
        }
    }
    QString term = ui->lineEditSearch ? ui->lineEditSearch->text().trimmed() : QString();
    if (!term.isEmpty()) {
        term.replace("'", "''");
        QString like = "'%" + term + "%'";
        QStringList searchOr;
        searchOr << "UPPER(" + qNom + ") LIKE UPPER(" + like + ")";
        searchOr << "UPPER(" + qType + ") LIKE UPPER(" + like + ")";
        searchOr << "UPPER(" + qStatut + ") LIKE UPPER(" + like + ")";
        if (!colSafety.isEmpty())
            searchOr << "UPPER(" + colSafety + ") LIKE UPPER(" + like + ")";
        andParts << "(" + searchOr.join(" OR ") + ")";
    }
    if (andParts.isEmpty()) return QString();
    return " AND " + andParts.join(" AND ");
}

QString MainWindow::quaiOrderClause() const
{
    int idx = ui->comboSmartSort ? ui->comboSmartSort->currentIndex() : 0;
    const QString occ = Quai::sqlQuaiColOccupancyRate();
    const QString cur = Quai::sqlQuaiColCurrentUsage();
    const QString pri = Quai::sqlQuaiColPriorityLevel();
    const QString idCol = Quai::sqlQuaiColQuaiId().isEmpty() ? QStringLiteral("QUAI_ID") : Quai::sqlQuaiColQuaiId();
    const QString capCol = Quai::sqlQuaiColCapacite().isEmpty() ? QStringLiteral("CAPACITE") : Quai::sqlQuaiColCapacite();
    switch (idx) {
    case 0: return QStringLiteral(" ORDER BY %1 DESC").arg(capCol);
    case 1:
        if (!occ.isEmpty())
            return QStringLiteral(" ORDER BY NVL(%1, 0) DESC").arg(occ);
        return QStringLiteral(" ORDER BY %1").arg(idCol);
    case 2:
        if (!cur.isEmpty())
            return QStringLiteral(" ORDER BY NVL(%1, 0) ASC").arg(cur);
        return QStringLiteral(" ORDER BY %1").arg(idCol);
    case 3:
        if (!pri.isEmpty())
            return QStringLiteral(" ORDER BY %1").arg(pri);
        return QStringLiteral(" ORDER BY %1").arg(idCol);
    default: return QStringLiteral(" ORDER BY %1").arg(idCol);
    }
}

void MainWindow::refreshQuaiStats()
{
    QSqlDatabase db = QSqlDatabase::database(Connection::connectionName());
    if (!db.isOpen()) return;
    const QString cur = Quai::sqlQuaiColCurrentUsage();
    QSqlQuery q(db);
    if (!q.exec("SELECT COUNT(*) FROM QUAI")) return;
    int total = 0;
    if (q.next()) total = q.value(0).toInt();
    int libres = 0;
    int occupes = 0;
    if (!cur.isEmpty()) {
        if (q.exec("SELECT COUNT(*) FROM QUAI WHERE NVL(" + cur + ", 0) = 0") && q.next())
            libres = q.value(0).toInt();
        if (q.exec("SELECT COUNT(*) FROM QUAI WHERE " + cur + " > 0") && q.next())
            occupes = q.value(0).toInt();
    } else {
        libres = total;
        occupes = 0;
    }
    const QString stCol = Quai::sqlQuaiColStatut().isEmpty() ? QStringLiteral("STATUT") : Quai::sqlQuaiColStatut();
    const QString tyCol = Quai::sqlQuaiColQuaiType().isEmpty() ? QStringLiteral("QUAI_TYPE") : Quai::sqlQuaiColQuaiType();
    if (!q.exec(QStringLiteral("SELECT COUNT(*) FROM QUAI WHERE LOWER(NVL(%1,'')) LIKE '%%maintenance%%' OR LOWER(NVL(%2,'')) = 'maintenance'")
                    .arg(stCol, tyCol)))
        return;
    int maintenance = 0;
    if (q.next()) maintenance = q.value(0).toInt();
    if (ui->lblKpiTotalValue) ui->lblKpiTotalValue->setText(QString::number(total));
    if (ui->lblKpiLibresValue) ui->lblKpiLibresValue->setText(total ? QString("%1 (%2%)").arg(libres).arg(qRound(100.0 * libres / total)) : "0 (0%)");
    if (ui->lblKpiOccupesValue) ui->lblKpiOccupesValue->setText(total ? QString("%1 (%2%)").arg(occupes).arg(qRound(100.0 * occupes / total)) : "0 (0%)");
    if (ui->lblKpiMaintenanceValue) ui->lblKpiMaintenanceValue->setText(total ? QString("%1 (%2%)").arg(maintenance).arg(qRound(100.0 * maintenance / total)) : "0 (0%)");
}

// ---------- QUAI Supervision: bar + 2x2 donuts ----------
static const int QUAI_CHART_SIZE = 232;
static const int QUAI_CHART_TITLE_PT = 12;
static const int QUAI_CHART_SLICE_LABEL_PT = 10;

static QString quaiPieTwoLineLabel(const QString &head, int count, int pct)
{
    return head + QLatin1Char('\n') + QStringLiteral("%1 (%2%)").arg(count).arg(pct);
}

static QChartView *createDonutChartFrame(QFrame *frame)
{
    frame->setStyleSheet(QStringLiteral("QFrame { background-color: #262626; border: none; }"));
    frame->setMinimumSize(QUAI_CHART_SIZE, QUAI_CHART_SIZE);
    frame->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    auto *layout = new QVBoxLayout(frame);
    layout->setContentsMargins(0, 0, 0, 0);
    auto *chart = new QChart();
    chart->setBackgroundBrush(QBrush(QColor(0x26, 0x26, 0x26)));
    chart->setBackgroundVisible(true);
    chart->setPlotAreaBackgroundVisible(false);
    chart->setAnimationOptions(QChart::AnimationOption::NoAnimation);
    chart->setMargins(QMargins(16, 24, 16, 48));
    chart->legend()->setVisible(false);
    chart->setTitleFont(QFont(QStringLiteral("Segoe UI"), QUAI_CHART_TITLE_PT, QFont::Bold));
    chart->setTitleBrush(QBrush(QColor(0x2a, 0x9d, 0x8f)));
    auto *view = new QChartView(chart, frame);
    view->setRenderHint(QPainter::Antialiasing);
    view->setRenderHint(QPainter::SmoothPixmapTransform);
    view->setRubberBand(QChartView::NoRubberBand);
    view->setMinimumSize(QUAI_CHART_SIZE, QUAI_CHART_SIZE);
    view->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    layout->addWidget(view, 1);
    return view;
}

static void fillDonut(QChartView *view, const QString &title, const QList<QPair<QString, int>> &orderedCounts,
                      const QList<QColor> &colors, double holeSize = 0.36, double pieSize = 0.82)
{
    if (!view || !view->chart())
        return;
    QChart *chart = view->chart();
    chart->setBackgroundBrush(QBrush(QColor(0x26, 0x26, 0x26)));
    chart->setBackgroundVisible(true);
    chart->removeAllSeries();
    chart->setTitle(title);
    chart->setTitleBrush(QBrush(QColor(0x2a, 0x9d, 0x8f)));
    chart->setTitleFont(QFont(QStringLiteral("Segoe UI"), QUAI_CHART_TITLE_PT, QFont::Bold));
    chart->setMargins(QMargins(16, 24, 16, 48));

    QList<QPair<QString, int>> positive;
    for (const auto &p : orderedCounts) {
        if (p.second > 0)
            positive.append(p);
    }
    int total = 0;
    for (const auto &p : positive)
        total += p.second;
    if (positive.isEmpty() || total == 0) {
        auto *emptySeries = new QPieSeries();
        emptySeries->setHoleSize(holeSize);
        emptySeries->setPieSize(pieSize);
        QPieSlice *empty = emptySeries->append(QObject::tr("Aucune donnée"), 1);
        empty->setBrush(QBrush(QColor(127, 140, 141), Qt::SolidPattern));
        empty->setColor(QColor(127, 140, 141));
        empty->setLabelVisible(false);
        chart->addSeries(emptySeries);
        return;
    }
    auto *series = new QPieSeries();
    series->setHoleSize(holeSize);
    series->setPieSize(pieSize);
    for (int idx = 0; idx < positive.size(); ++idx) {
        const QString categoryName = positive.at(idx).first;
        int count = positive.at(idx).second;
        const int pct = qRound(100.0 * count / total);
        const QString labelText = quaiPieTwoLineLabel(categoryName, count, pct);
        const QString tooltipText = QStringLiteral("%1 — %2 (%3%)").arg(categoryName).arg(count).arg(pct);
        QColor baseColor = colors.value(idx % colors.size(), QColor(149, 165, 166));
        QPieSlice *slice = series->append(categoryName, count);
        slice->setLabel(labelText);
        slice->setLabelVisible(true);
        slice->setLabelPosition(QPieSlice::LabelOutside);
        slice->setLabelColor(QColor(0xe0, 0xe0, 0xe0));
        slice->setLabelFont(QFont(QStringLiteral("Segoe UI"), QUAI_CHART_SLICE_LABEL_PT, QFont::DemiBold));
        slice->setLabelArmLengthFactor(0.22);
        slice->setBrush(QBrush(baseColor, Qt::SolidPattern));
        slice->setColor(baseColor);
        slice->setBorderColor(QColor(0x26, 0x26, 0x26));
        slice->setBorderWidth(3);
        slice->setExploded(false);
        QObject::connect(slice, &QPieSlice::hovered, [slice, baseColor, tooltipText](bool state) {
            slice->setExploded(state);
            if (state) {
                QColor hoverColor = baseColor.lighter(115);
                slice->setBrush(QBrush(hoverColor, Qt::SolidPattern));
                slice->setColor(hoverColor);
                slice->setBorderWidth(4);
                slice->setBorderColor(baseColor.lighter(130));
                QToolTip::showText(QCursor::pos(), tooltipText, nullptr, QRect(), 2500);
            } else {
                slice->setBrush(QBrush(baseColor, Qt::SolidPattern));
                slice->setColor(baseColor);
                slice->setBorderWidth(3);
                slice->setBorderColor(QColor(0x26, 0x26, 0x26));
                QToolTip::hideText();
            }
        });
    }
    chart->addSeries(series);
    const QList<QPieSlice *> slices = series->slices();
    for (int i = 0; i < slices.size(); ++i) {
        QColor c = colors.value(i % colors.size(), QColor(149, 165, 166));
        slices.at(i)->setBrush(QBrush(c, Qt::SolidPattern));
        slices.at(i)->setColor(c);
    }
}

static void refreshQuaiOccByTypeBarView(QChartView *view, const QMap<QString, int> &typeCounts,
                                        const QMap<QString, double> &typeAvgOcc)
{
    if (!view || !view->chart())
        return;
    QChart *chart = view->chart();
    chart->setBackgroundBrush(QBrush(QColor(0x26, 0x26, 0x26)));
    chart->setBackgroundVisible(true);
    chart->setAnimationOptions(QChart::AnimationOption::NoAnimation);
    chart->legend()->setVisible(false);
    chart->setTitle(QObject::tr("Réf. : Taux d'occupation moyen par type"));
    chart->setTitleBrush(QBrush(QColor(0x2a, 0x9d, 0x8f)));
    chart->setTitleFont(QFont(QStringLiteral("Segoe UI"), QUAI_CHART_TITLE_PT, QFont::Bold));
    chart->setMargins(QMargins(12, 20, 12, 12));
    chart->removeAllSeries();
    const QList<QAbstractAxis *> oldAxes = chart->axes();
    for (QAbstractAxis *ax : oldAxes)
        chart->removeAxis(ax);

    if (typeCounts.isEmpty()) {
        auto *emptySeries = new QHorizontalBarSeries();
        auto *set = new QBarSet(QObject::tr("—"));
        *set << 1.0;
        emptySeries->append(set);
        chart->addSeries(emptySeries);
        auto *axisY = new QBarCategoryAxis();
        axisY->append(QObject::tr("Aucune donnée"));
        chart->addAxis(axisY, Qt::AlignLeft);
        auto *axisX = new QValueAxis();
        axisX->setRange(0, 1);
        axisX->setVisible(false);
        chart->addAxis(axisX, Qt::AlignBottom);
        emptySeries->attachAxis(axisY);
        emptySeries->attachAxis(axisX);
        return;
    }

    QStringList categories;
    auto *barSet = new QBarSet(QObject::tr("Moy. %"));
    const QList<QString> keys = typeCounts.keys();
    QList<QString> sortedKeys = keys;
    std::sort(sortedKeys.begin(), sortedKeys.end(), [](const QString &a, const QString &b) {
        return QString::localeAwareCompare(a, b) < 0;
    });
    for (const QString &k : sortedKeys) {
        categories << k;
        *barSet << typeAvgOcc.value(k, 0.0);
    }
    auto *series = new QHorizontalBarSeries();
    series->append(barSet);
    chart->addSeries(series);
    auto *axisY = new QBarCategoryAxis();
    axisY->append(categories);
    chart->addAxis(axisY, Qt::AlignLeft);
    auto *axisX = new QValueAxis();
    axisX->setRange(0, 100);
    axisX->setLabelFormat(QStringLiteral("%.0f"));
    axisX->setTitleText(QStringLiteral("%"));
    chart->addAxis(axisX, Qt::AlignBottom);
    series->attachAxis(axisY);
    series->attachAxis(axisX);
}

void MainWindow::setupQuaiCharts()
{
    if (m_quaiChartView || !ui->pieChartFrame)
        return;
    if (ui->tableOccupationByType)
        ui->tableOccupationByType->hide();
    if (ui->legendFrame)
        ui->legendFrame->hide();
    if (ui->chartTitle)
        ui->chartTitle->hide();

    if (!m_quaiOccByTypeBarFrame && ui->chartTitle) {
        QWidget *pieHost = ui->chartTitle->parentWidget();
        auto *pieCol = qobject_cast<QVBoxLayout *>(pieHost ? pieHost->layout() : nullptr);
        if (pieCol) {
            m_quaiOccByTypeBarFrame = new QFrame(pieHost);
            m_quaiOccByTypeBarFrame->setMinimumHeight(200);
            m_quaiOccByTypeBarFrame->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
            m_quaiOccByTypeBarFrame->setStyleSheet(QStringLiteral("QFrame { background-color: #262626; border: none; }"));
            auto *bl = new QVBoxLayout(m_quaiOccByTypeBarFrame);
            bl->setContentsMargins(0, 0, 0, 8);
            auto *bc = new QChart();
            bc->setBackgroundBrush(QBrush(QColor(0x26, 0x26, 0x26)));
            bc->setBackgroundVisible(true);
            bc->setPlotAreaBackgroundVisible(false);
            bc->setAnimationOptions(QChart::AnimationOption::NoAnimation);
            bc->legend()->setVisible(false);
            bc->setTitleBrush(QBrush(QColor(0x2a, 0x9d, 0x8f)));
            bc->setTitleFont(QFont(QStringLiteral("Segoe UI"), QUAI_CHART_TITLE_PT, QFont::Bold));
            m_quaiOccByTypeBarView = new QChartView(bc, m_quaiOccByTypeBarFrame);
            m_quaiOccByTypeBarView->setRenderHint(QPainter::Antialiasing);
            m_quaiOccByTypeBarView->setRenderHint(QPainter::SmoothPixmapTransform);
            m_quaiOccByTypeBarView->setRubberBand(QChartView::NoRubberBand);
            m_quaiOccByTypeBarView->setMinimumHeight(180);
            m_quaiOccByTypeBarView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
            bl->addWidget(m_quaiOccByTypeBarView, 1);
            pieCol->insertWidget(0, m_quaiOccByTypeBarFrame);
        }
    }

    ui->pieChartFrame->setStyleSheet(QStringLiteral("QFrame { background-color: transparent; border: none; }"));
    auto *layout0 = new QVBoxLayout(ui->pieChartFrame);
    layout0->setContentsMargins(0, 0, 0, 0);
    auto *chart = new QChart();
    chart->setBackgroundBrush(QBrush(QColor(0x26, 0x26, 0x26)));
    chart->setBackgroundVisible(true);
    chart->setPlotAreaBackgroundVisible(false);
    chart->setAnimationOptions(QChart::AnimationOption::NoAnimation);
    chart->setMargins(QMargins(20, 28, 20, 64));
    chart->legend()->setVisible(false);
    chart->setTitleBrush(QBrush(QColor(0x2a, 0x9d, 0x8f)));
    chart->setTitleFont(QFont(QStringLiteral("Segoe UI"), QUAI_CHART_TITLE_PT, QFont::Bold));
    chart->setTitle(QStringLiteral("Réf. : Quais par type"));
    m_quaiChartView = new QChartView(chart, ui->pieChartFrame);
    m_quaiChartView->setRenderHint(QPainter::Antialiasing);
    m_quaiChartView->setRenderHint(QPainter::SmoothPixmapTransform);
    m_quaiChartView->setRubberBand(QChartView::NoRubberBand);
    m_quaiChartView->setMinimumSize(QUAI_CHART_SIZE, QUAI_CHART_SIZE);
    m_quaiChartView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    layout0->addWidget(m_quaiChartView, 1);

    if (!m_quaiChartsGridWidget && ui->occupationLayout && ui->chartAndLegendLayout) {
        QLayoutItem *chartItem = ui->chartAndLegendLayout->takeAt(0);
        if (chartItem && chartItem->widget()) {
            QWidget *pieFrame = chartItem->widget();
            delete chartItem;
            m_quaiChartsGridWidget = new QWidget(ui->groupBoxOccupation);
            m_quaiChartsGridWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
            m_quaiChartsGridWidget->setMinimumHeight(440);
            m_quaiChartsGridWidget->setStyleSheet(QStringLiteral("QWidget { background-color: #262626; }"));
            auto *grid = new QGridLayout(m_quaiChartsGridWidget);
            grid->setSpacing(4);
            grid->setContentsMargins(4, 2, 4, 2);
            grid->setRowStretch(0, 1);
            grid->setRowStretch(1, 1);
            grid->setColumnStretch(0, 1);
            grid->setColumnStretch(1, 1);
            pieFrame->setParent(m_quaiChartsGridWidget);
            pieFrame->setStyleSheet(QStringLiteral("QFrame { background-color: #262626; border: none; }"));
            ui->pieChartFrame->setMinimumSize(QUAI_CHART_SIZE, QUAI_CHART_SIZE);
            ui->pieChartFrame->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
            grid->addWidget(pieFrame, 0, 0);
            auto *f2 = new QFrame(m_quaiChartsGridWidget);
            m_quaiChartView2 = createDonutChartFrame(f2);
            grid->addWidget(f2, 0, 1);
            auto *f3 = new QFrame(m_quaiChartsGridWidget);
            m_quaiChartView3 = createDonutChartFrame(f3);
            grid->addWidget(f3, 1, 0);
            auto *f4 = new QFrame(m_quaiChartsGridWidget);
            m_quaiChartView4 = createDonutChartFrame(f4);
            grid->addWidget(f4, 1, 1);
            f2->show();
            f3->show();
            f4->show();
            QLayoutItem *taken = ui->occupationLayout->takeAt(1);
            if (taken)
                delete taken;
            ui->occupationLayout->addWidget(m_quaiChartsGridWidget, 1);
        }
    }
}

void MainWindow::setupQuaiMapOverviewUi()
{
    if (!ui || !ui->mainTabWidget)
        return;

    if (!m_quaiMapTab) {
    m_quaiMapTab = new QWidget(ui->mainTabWidget);
    m_quaiMapTab->setObjectName(QStringLiteral("tabQuaiMapOverview"));
    auto *root = new QVBoxLayout(m_quaiMapTab);
    root->setContentsMargins(10, 10, 10, 10);
    root->setSpacing(8);

    auto *header = new QFrame(m_quaiMapTab);
    header->setStyleSheet(QStringLiteral(
        "QFrame { background:#23374d; border:1px solid #2f8ccf; border-radius:10px; }"
        "QLabel { color:#eaf1f8; font-weight:700; }"));
    auto *h = new QHBoxLayout(header);
    h->setContentsMargins(10, 8, 10, 8);
    QLabel *title = new QLabel(tr("Carte globale des quais"), header);
    title->setStyleSheet(QStringLiteral("font-size:14px; color:#d7ecff;"));
    QPushButton *btnOpenGeoMap = new QPushButton(tr("Ouvrir carte géographique"), header);
    btnOpenGeoMap->setCursor(Qt::PointingHandCursor);
    btnOpenGeoMap->setMinimumHeight(32);
    btnOpenGeoMap->setStyleSheet(QStringLiteral(
        "QPushButton { background:#3498db; color:white; border:none; border-radius:6px; padding:6px 12px; font-weight:700; }"
        "QPushButton:hover { background:#2f89c5; }"));
    QPushButton *btnRefreshMap = new QPushButton(tr("Actualiser carte"), header);
    btnRefreshMap->setCursor(Qt::PointingHandCursor);
    btnRefreshMap->setMinimumHeight(32);
    btnRefreshMap->setStyleSheet(QStringLiteral(
        "QPushButton { background:#1abc9c; color:#102027; border:none; border-radius:6px; padding:6px 12px; font-weight:700; }"
        "QPushButton:hover { background:#16a085; }"));
    h->addWidget(title);
    h->addStretch(1);
    h->addWidget(btnOpenGeoMap);
    h->addWidget(btnRefreshMap);
    root->addWidget(header);

    m_quaiMapSummary = new QLabel(tr("Chargement de la carte..."), m_quaiMapTab);
    m_quaiMapSummary->setStyleSheet(QStringLiteral("color:#cfe6ff; background:#1f2a38; border:1px solid #2f4054; border-radius:8px; padding:8px 10px;"));
    root->addWidget(m_quaiMapSummary);

    QLabel *legend = new QLabel(
        tr("Légende: <font color='#2ecc71'><b>Disponible</b></font>  |  "
           "<font color='#f39c12'><b>Occupé</b></font>  |  "
           "<font color='#e74c3c'><b>Incident</b></font>  |  "
           "<font color='#8e44ad'><b>Maintenance</b></font>"), m_quaiMapTab);
    legend->setTextFormat(Qt::RichText);
    legend->setStyleSheet(QStringLiteral("color:#e0e0e0;"));
    root->addWidget(legend);

    m_quaiMapScroll = new QScrollArea(m_quaiMapTab);
    m_quaiMapScroll->setWidgetResizable(true);
    m_quaiMapScroll->setFrameShape(QFrame::NoFrame);
    m_quaiMapGridHost = new QWidget(m_quaiMapScroll);
    m_quaiMapGrid = new QGridLayout(m_quaiMapGridHost);
    m_quaiMapGrid->setContentsMargins(2, 2, 2, 2);
    m_quaiMapGrid->setHorizontalSpacing(10);
    m_quaiMapGrid->setVerticalSpacing(10);
    m_quaiMapScroll->setWidget(m_quaiMapGridHost);
    root->addWidget(m_quaiMapScroll, 1);

    ui->mainTabWidget->addTab(m_quaiMapTab, tr("Carte Quai"));
    connect(btnOpenGeoMap, &QPushButton::clicked, this, &MainWindow::openQuaiGeographicMap);
    connect(btnRefreshMap, &QPushButton::clicked, this, &MainWindow::refreshQuaiMapOverview);
    refreshQuaiMapOverview();
    }

    if (!m_quaiArduinoTab) {
        m_quaiArduinoTab = new QWidget(ui->mainTabWidget);
        auto *arduinoLay = new QVBoxLayout(m_quaiArduinoTab);
        arduinoLay->setContentsMargins(0, 0, 0, 0);
        if (!m_quaiArduinoWindow) {
            m_quaiArduinoWindow = new QuaiArduinoWindow(m_quaiArduinoTab);
            connect(m_quaiArduinoWindow, &QuaiArduinoWindow::assignmentCompleted, this, [this](int) {
                loadQuaisTable();
                refreshQuaiStats();
                refreshQuaiCharts();
                syncQuaiGateArduinoFromDatabase();
                if (m_quaiArduinoWindow)
                    m_quaiArduinoWindow->refreshActiveQuaisCombo();
            });
            connect(m_quaiArduinoWindow, &QuaiArduinoWindow::forceOpenQuaiGateRequested, this, [this] {
                if (m_quaiGateArduino.getserial() && m_quaiGateArduino.getserial()->isOpen())
                    m_quaiGateArduino.write_to_arduino(QByteArrayLiteral("PORTE_OPEN\n"));
            });
            connect(m_quaiArduinoWindow, &QuaiArduinoWindow::forceCloseQuaiGateRequested, this, [this] {
                if (m_quaiGateArduino.getserial() && m_quaiGateArduino.getserial()->isOpen())
                    m_quaiGateArduino.write_to_arduino(QByteArrayLiteral("PORTE_CLOSE\n"));
            });
        }
        arduinoLay->addWidget(m_quaiArduinoWindow);
        const int mapIdx = ui->mainTabWidget->indexOf(m_quaiMapTab);
        const int insertAt = mapIdx >= 0 ? mapIdx + 1 : ui->mainTabWidget->count();
        ui->mainTabWidget->insertTab(insertAt, m_quaiArduinoTab, tr("Arduino"));
    }
}

void MainWindow::refreshQuaiMapOverview()
{
    if (!m_quaiMapGrid || !m_quaiMapSummary)
        return;
    while (QLayoutItem *item = m_quaiMapGrid->takeAt(0)) {
        if (QWidget *w = item->widget())
            w->deleteLater();
        delete item;
    }

    QSqlDatabase db = QSqlDatabase::database(Connection::connectionName());
    if (!db.isOpen()) {
        m_quaiMapSummary->setText(tr("Base non connectée - impossible de charger la carte des quais."));
        return;
    }

    const QString idCol = Quai::sqlQuaiColQuaiId().isEmpty() ? QStringLiteral("QUAI_ID") : Quai::sqlQuaiColQuaiId();
    const QString nomCol = Quai::sqlQuaiColQuaiNom();
    const QString typeCol = Quai::sqlQuaiColQuaiType();
    const QString statutCol = Quai::sqlQuaiColStatut();
    const QString occCol = Quai::sqlQuaiColOccupancyRate();
    const QString curUCol = Quai::sqlQuaiColCurrentUsage();
    const QString incCol = Quai::sqlQuaiColIncidentLog();
    if (nomCol.isEmpty())
        return;

    QSet<QString> quaiCols;
    {
        QSqlQuery colsQ(db);
        if (colsQ.exec(QStringLiteral("SELECT COLUMN_NAME FROM USER_TAB_COLUMNS WHERE UPPER(TABLE_NAME)=UPPER('QUAI')"))) {
            while (colsQ.next()) quaiCols.insert(colsQ.value(0).toString().trimmed().toUpper());
        }
    }
    const QString mapLatCol = quaiCols.contains(QStringLiteral("MAP_LAT")) ? QStringLiteral("MAP_LAT") : QString();
    const QString mapLngCol = quaiCols.contains(QStringLiteral("MAP_LNG")) ? QStringLiteral("MAP_LNG") : QString();
    if (mapLatCol.isEmpty() || mapLngCol.isEmpty()) {
        QSqlQuery alterQ(db);
        if (!alterQ.exec(QStringLiteral("ALTER TABLE QUAI ADD (MAP_LAT NUMBER(10,6), MAP_LNG NUMBER(10,6))"))) {
            QMessageBox::warning(this, tr("Carte géographique"),
                                 tr("Impossible d'ajouter les colonnes MAP_LAT/MAP_LNG dans QUAI: %1")
                                     .arg(alterQ.lastError().text()));
            return;
        }
    }

    const QString sql = QStringLiteral(
        "SELECT %1, %2, %3, %4, NVL(%5,0), NVL(%6,0), %7, MAP_LAT, MAP_LNG FROM QUAI ORDER BY %2")
        .arg(idCol,
             nomCol,
             typeCol.isEmpty() ? QStringLiteral("CAST(NULL AS VARCHAR2(1))") : typeCol,
             statutCol.isEmpty() ? QStringLiteral("CAST(NULL AS VARCHAR2(1))") : statutCol,
             occCol.isEmpty() ? QStringLiteral("0") : occCol,
             curUCol.isEmpty() ? QStringLiteral("0") : curUCol,
             incCol.isEmpty() ? QStringLiteral("CAST(NULL AS VARCHAR2(1))") : incCol);
    QSqlQuery q(db);
    if (!q.exec(sql)) {
        m_quaiMapSummary->setText(tr("Erreur SQL carte quais: %1").arg(q.lastError().text()));
        return;
    }

    int total = 0, dispo = 0, occupe = 0, maint = 0, incident = 0;
    int cardIndex = 0;
    while (q.next()) {
        const QString nom = q.value(1).toString().trimmed();
        const QString type = q.value(2).toString().trimmed();
        const QString statut = q.value(3).toString().trimmed();
        const double occRate = q.value(4).toDouble();
        const int usage = q.value(5).toInt();
        const QString incidentLog = q.value(6).toString().trimmed();

        const bool hasIncident = !incidentLog.isEmpty();
        const bool isMaint = statut.contains(QStringLiteral("maintenance"), Qt::CaseInsensitive)
                             || type.compare(QStringLiteral("Maintenance"), Qt::CaseInsensitive) == 0;
        const bool isBusy = !isMaint && !hasIncident && (usage > 0 || occRate >= 70.0);
        QString stateLabel;
        QString stateColor;
        if (hasIncident) { stateLabel = tr("Incident"); stateColor = QStringLiteral("#e74c3c"); incident++; }
        else if (isMaint) { stateLabel = tr("Maintenance"); stateColor = QStringLiteral("#8e44ad"); maint++; }
        else if (isBusy) { stateLabel = tr("Occupé"); stateColor = QStringLiteral("#f39c12"); occupe++; }
        else { stateLabel = tr("Disponible"); stateColor = QStringLiteral("#2ecc71"); dispo++; }

        total++;
        QFrame *card = new QFrame(m_quaiMapGridHost);
        card->setStyleSheet(QStringLiteral(
            "QFrame { background:#1f2329; border:1px solid #3a4b60; border-radius:10px; }"
            "QLabel { color:#dfe7f1; }"));
        auto *vl = new QVBoxLayout(card);
        vl->setContentsMargins(10, 8, 10, 8);
        vl->setSpacing(4);

        QLabel *lblName = new QLabel(nom.isEmpty() ? tr("Quai sans nom") : nom, card);
        lblName->setStyleSheet(QStringLiteral("font-weight:700; color:#eaf1ff; font-size:13px;"));
        QLabel *lblState = new QLabel(tr("Etat: %1").arg(stateLabel), card);
        lblState->setStyleSheet(QStringLiteral("font-weight:700; color:%1;").arg(stateColor));
        QLabel *lblType = new QLabel(tr("Type: %1").arg(type.isEmpty() ? tr("N/D") : type), card);
        QLabel *lblStat = new QLabel(tr("Statut DB: %1").arg(statut.isEmpty() ? tr("N/D") : statut), card);
        QLabel *lblOcc = new QLabel(tr("Occupation: %1% | Usage: %2").arg(QString::number(occRate, 'f', 1)).arg(usage), card);
        QLabel *lblInc = new QLabel(tr("Incident: %1").arg(hasIncident ? tr("Oui") : tr("Non")), card);
        vl->addWidget(lblName);
        vl->addWidget(lblState);
        vl->addWidget(lblType);
        vl->addWidget(lblStat);
        vl->addWidget(lblOcc);
        vl->addWidget(lblInc);

        const int row = cardIndex / 4;
        const int col = cardIndex % 4;
        m_quaiMapGrid->addWidget(card, row, col);
        cardIndex++;
    }

    m_quaiMapSummary->setText(
        tr("Vue globale: %1 quai(s) | Disponible: %2 | Occupé: %3 | Maintenance: %4 | Incident: %5")
            .arg(total).arg(dispo).arg(occupe).arg(maint).arg(incident));
}

void MainWindow::openQuaiGeographicMap()
{
    QSqlDatabase db = QSqlDatabase::database(Connection::connectionName());
    if (!db.isOpen()) {
        QMessageBox::warning(this, tr("Carte géographique"), tr("Base non connectée."));
        return;
    }
    const QString idCol = Quai::sqlQuaiColQuaiId().isEmpty() ? QStringLiteral("QUAI_ID") : Quai::sqlQuaiColQuaiId();
    const QString nomCol = Quai::sqlQuaiColQuaiNom();
    const QString typeCol = Quai::sqlQuaiColQuaiType();
    const QString statutCol = Quai::sqlQuaiColStatut();
    const QString occCol = Quai::sqlQuaiColOccupancyRate();
    const QString curUCol = Quai::sqlQuaiColCurrentUsage();
    const QString incCol = Quai::sqlQuaiColIncidentLog();
    if (nomCol.isEmpty()) {
        QMessageBox::warning(this, tr("Carte géographique"), tr("Colonne nom quai introuvable."));
        return;
    }

    QSet<QString> quaiCols;
    {
        QSqlQuery colsQ(db);
        if (colsQ.exec(QStringLiteral("SELECT COLUMN_NAME FROM USER_TAB_COLUMNS WHERE UPPER(TABLE_NAME)=UPPER('QUAI')"))) {
            while (colsQ.next()) quaiCols.insert(colsQ.value(0).toString().trimmed().toUpper());
        }
    }
    const QString mapLatCol = quaiCols.contains(QStringLiteral("MAP_LAT")) ? QStringLiteral("MAP_LAT") : QString();
    const QString mapLngCol = quaiCols.contains(QStringLiteral("MAP_LNG")) ? QStringLiteral("MAP_LNG") : QString();
    if (mapLatCol.isEmpty() || mapLngCol.isEmpty()) {
        QSqlQuery alterQ(db);
        if (!alterQ.exec(QStringLiteral("ALTER TABLE QUAI ADD (MAP_LAT NUMBER(10,6), MAP_LNG NUMBER(10,6))"))) {
            QMessageBox::warning(this, tr("Carte géographique"),
                                 tr("Impossible d'ajouter les colonnes MAP_LAT/MAP_LNG dans QUAI: %1")
                                     .arg(alterQ.lastError().text()));
            return;
        }
    }

    const QString sql = QStringLiteral(
        "SELECT %1, %2, %3, %4, NVL(%5,0), NVL(%6,0), %7, MAP_LAT, MAP_LNG FROM QUAI ORDER BY %2")
        .arg(idCol,
             nomCol,
             typeCol.isEmpty() ? QStringLiteral("CAST(NULL AS VARCHAR2(1))") : typeCol,
             statutCol.isEmpty() ? QStringLiteral("CAST(NULL AS VARCHAR2(1))") : statutCol,
             occCol.isEmpty() ? QStringLiteral("0") : occCol,
             curUCol.isEmpty() ? QStringLiteral("0") : curUCol,
             incCol.isEmpty() ? QStringLiteral("CAST(NULL AS VARCHAR2(1))") : incCol);
    QSqlQuery q(db);
    if (!q.exec(sql)) {
        QMessageBox::warning(this, tr("Carte géographique"), tr("Erreur SQL: %1").arg(q.lastError().text()));
        return;
    }

    struct Row {
        int id = 0;
        QString nom, type, statut, state, color;
        double occ = 0.0;
        int usage = 0;
        bool hasDbGeo = false;
        double dbLat = 0.0;
        double dbLng = 0.0;
    };
    QVector<Row> rows;
    while (q.next()) {
        Row r;
        r.id = q.value(0).toInt();
        r.nom = q.value(1).toString().trimmed();
        r.type = q.value(2).toString().trimmed();
        r.statut = q.value(3).toString().trimmed();
        r.occ = q.value(4).toDouble();
        r.usage = q.value(5).toInt();
        const QString incidentLog = q.value(6).toString().trimmed();
        bool okLat = false, okLng = false;
        const double lat = q.value(7).toDouble(&okLat);
        const double lng = q.value(8).toDouble(&okLng);
        if (okLat && okLng) {
            r.hasDbGeo = true;
            r.dbLat = lat;
            r.dbLng = lng;
        }
        const bool isIncident = !incidentLog.isEmpty();
        const bool isMaint = r.statut.contains(QStringLiteral("maintenance"), Qt::CaseInsensitive)
                             || r.type.compare(QStringLiteral("Maintenance"), Qt::CaseInsensitive) == 0;
        const bool isBusy = !isMaint && !isIncident && (r.usage > 0 || r.occ >= 70.0);
        if (isIncident) { r.state = tr("Incident"); r.color = QStringLiteral("#e74c3c"); }
        else if (isMaint) { r.state = tr("Maintenance"); r.color = QStringLiteral("#8e44ad"); }
        else if (isBusy) { r.state = tr("Occupé"); r.color = QStringLiteral("#f39c12"); }
        else { r.state = tr("Disponible"); r.color = QStringLiteral("#2ecc71"); }
        if (r.nom.isEmpty()) r.nom = tr("Quai sans nom");
        rows.push_back(r);
    }

    struct GeoMarker {
        double lat = 0.0;
        double lng = 0.0;
        QString color;
        QString label;
        QString detail;
    };

    const QVector<QPair<double, double>> shorelinePath = {
        // Follow user red-line: coast-edge berths from north to south.
        {36.81935, 10.30690},
        {36.81895, 10.30700},
        {36.81850, 10.30708},
        {36.81805, 10.30718},
        {36.81760, 10.30728},
        {36.81710, 10.30738},
        {36.81660, 10.30750},
        {36.81610, 10.30766},
        {36.81560, 10.30784},
        {36.81510, 10.30808},
        {36.81460, 10.30835},
        {36.81405, 10.30866},
        {36.81350, 10.30900},
        {36.81295, 10.30935},
        {36.81240, 10.30975},
        {36.81185, 10.31015},
        {36.81130, 10.31055},
        {36.81075, 10.31095},
        {36.81020, 10.31135}
    };
    auto interpolateOnPath = [&shorelinePath](double t) -> QPair<double, double> {
        if (shorelinePath.isEmpty()) return {36.8105, 10.3158};
        if (shorelinePath.size() == 1) return shorelinePath.first();
        t = qBound(0.0, t, 1.0);
        const int segCount = shorelinePath.size() - 1;
        const double pos = t * segCount;
        const int seg = qBound(0, static_cast<int>(std::floor(pos)), segCount - 1);
        const double localT = pos - seg;
        const auto a = shorelinePath.at(seg);
        const auto b = shorelinePath.at(seg + 1);
        return { a.first + (b.first - a.first) * localT, a.second + (b.second - a.second) * localT };
    };

    QVector<GeoMarker> markers;
    for (int i = 0; i < rows.size(); ++i) {
        const Row &r = rows.at(i);
        const int count = qMax(1, rows.size());
        const double t = (count == 1) ? 0.5 : static_cast<double>(i) / static_cast<double>(count - 1);
        auto p = interpolateOnPath(t);
        double lat = p.first;
        double lng = p.second;
        if (r.hasDbGeo) { lat = r.dbLat; lng = r.dbLng; }
        const QString detail = tr("%1 | Etat: %2 | Type: %3 | Statut DB: %4 | Occupation: %5% | Usage: %6")
                                   .arg(r.nom, r.state, r.type.isEmpty() ? tr("N/D") : r.type,
                                        r.statut.isEmpty() ? tr("N/D") : r.statut,
                                        QString::number(r.occ, 'f', 1), QString::number(r.usage));
        markers.push_back({lat, lng, r.color, r.nom, detail});
    }

    QDialog dlg(this);
    dlg.setWindowTitle(tr("Carte géographique des quais (Tunisie)"));
    dlg.resize(1180, 760);
    auto *layout = new QVBoxLayout(&dlg);
    layout->setContentsMargins(10, 10, 10, 10);
    layout->setSpacing(8);

    auto *top = new QFrame(&dlg);
    top->setStyleSheet(QStringLiteral("QFrame { background:#1f2a38; border:1px solid #2f4054; border-radius:8px; } QLabel{color:#dfe9f5;}"));
    auto *topL = new QHBoxLayout(top);
    topL->setContentsMargins(10, 8, 10, 8);
    QLabel *lbl = new QLabel(tr("Sélectionnez un quai existant, ajustez sa position, puis enregistrez."), top);
    QComboBox *cmbQuai = new QComboBox(top);
    cmbQuai->setMinimumWidth(230);
    for (const Row &r : rows) cmbQuai->addItem(r.nom, r.id);
    auto *spinLat = new QDoubleSpinBox(top);
    spinLat->setDecimals(6); spinLat->setRange(30.0, 38.0); spinLat->setPrefix(QStringLiteral("Lat "));
    auto *spinLng = new QDoubleSpinBox(top);
    spinLng->setDecimals(6); spinLng->setRange(7.0, 12.5); spinLng->setPrefix(QStringLiteral("Lng "));
    QPushButton *btnSaveLatLng = new QPushButton(tr("Enregistrer Lat/Lng"), top);
    btnSaveLatLng->setStyleSheet(QStringLiteral(
        "QPushButton { background:#2ecc71; color:#102027; border:none; border-radius:6px; padding:6px 12px; font-weight:700; }"
        "QPushButton:hover { background:#28b765; }"));
    QPushButton *btnSaveDrag = new QPushButton(tr("Enregistrer depuis drag"), top);
    btnSaveDrag->setStyleSheet(QStringLiteral(
        "QPushButton { background:#2ecc71; color:#102027; border:none; border-radius:6px; padding:6px 12px; font-weight:700; }"
        "QPushButton:hover { background:#28b765; }"));
    QPushButton *btnResetPos = new QPushButton(tr("Réinitialiser positions"), top);
    btnResetPos->setStyleSheet(QStringLiteral(
        "QPushButton { background:#e67e22; color:#102027; border:none; border-radius:6px; padding:6px 12px; font-weight:700; }"
        "QPushButton:hover { background:#d35400; }"));
    topL->addWidget(lbl);
    topL->addSpacing(8);
    topL->addWidget(cmbQuai);
    topL->addWidget(spinLat);
    topL->addWidget(spinLng);
    topL->addWidget(btnSaveLatLng);
    topL->addWidget(btnSaveDrag);
    topL->addWidget(btnResetPos);
    topL->addStretch(1);
    layout->addWidget(top);

    QHash<int, QPair<double, double>> dbCoordsById;
    for (const Row &r : rows) {
        if (r.hasDbGeo) dbCoordsById.insert(r.id, qMakePair(r.dbLat, r.dbLng));
    }
    auto syncSpin = [cmbQuai, spinLat, spinLng, dbCoordsById]() {
        if (!cmbQuai || !spinLat || !spinLng) return;
        const int qid = cmbQuai->currentData().toInt();
        if (dbCoordsById.contains(qid)) {
            const auto p = dbCoordsById.value(qid);
            spinLat->setValue(p.first);
            spinLng->setValue(p.second);
            return;
        }
        spinLat->setValue(36.8148);
        spinLng->setValue(10.3098);
    };
    connect(cmbQuai, &QComboBox::currentIndexChanged, &dlg, [syncSpin](int){ syncSpin(); });
    syncSpin();

    auto qmlEsc = [](QString s) {
        s.replace('\\', QStringLiteral("\\\\"));
        s.replace('"', QStringLiteral("\\\""));
        s.replace('\n', QStringLiteral(" "));
        s.replace('\r', QStringLiteral(" "));
        return s;
    };
    QString markerItemsQml;
    QString dragInitQml;
    for (int markerIdx = 0; markerIdx < markers.size(); ++markerIdx) {
        const GeoMarker &m = markers.at(markerIdx);
        dragInitQml += QStringLiteral("dragCoords[\"%1\"] = QtPositioning.coordinate(%2, %3);\n")
                           .arg(QString::number(rows.at(markerIdx).id),
                                QString::number(m.lat, 'f', 6),
                                QString::number(m.lng, 'f', 6));
        markerItemsQml += QStringLiteral(
            "MapQuickItem { id: mk%1; coordinate: QtPositioning.coordinate(%2, %3); anchorPoint.x: 8; anchorPoint.y: 8; "
            "sourceItem: Item { width: 120; height: 30;"
            "Rectangle { x: 0; y: 0; width: 16; height: 16; radius: 2; color: \"%4\"; border.width: 2; border.color: \"#0f1722\"; "
            "MouseArea { anchors.fill: parent; acceptedButtons: Qt.LeftButton; cursorShape: pressed ? Qt.ClosedHandCursor : Qt.OpenHandCursor; "
            "property point pressPos: Qt.point(0, 0);"
            "onPressed: { pressPos = Qt.point(mouse.x, mouse.y); infoText.text = \"%5\" + \" (glisser pour déplacer)\"; }"
            "onClicked: infoText.text = \"%5\";"
            "onPositionChanged: { if (!pressed) return; var dx = mouse.x - pressPos.x; var dy = mouse.y - pressPos.y; var mapPoint = map.fromCoordinate(mk%1.coordinate, false); var nextPoint = Qt.point(mapPoint.x + dx, mapPoint.y + dy); mk%1.coordinate = map.toCoordinate(nextPoint, false); }"
            "onReleased: { dragCoords[\"%7\"] = mk%1.coordinate; rebuildDraggedDump(); infoText.text = \"%5\" + \" | Nouvelle position: \" + mk%1.coordinate.latitude.toFixed(6) + \", \" + mk%1.coordinate.longitude.toFixed(6); }"
            "} }"
            "Text { x: 20; y: -1; text: \"%6\"; color: \"#f0f4fa\"; font.pixelSize: 11; font.bold: true; style: Text.Outline; styleColor: \"#152235\" }"
            "} }\n")
            .arg(QString::number(markerIdx),
                 QString::number(m.lat, 'f', 6),
                 QString::number(m.lng, 'f', 6),
                 m.color,
                 qmlEsc(m.detail),
                 qmlEsc(m.label),
                 QString::number(rows.at(markerIdx).id));
    }

    const QString qml = QStringLiteral(
        "import QtQuick\n"
        "import QtQuick.Controls\n"
        "import QtLocation\n"
        "import QtPositioning\n"
        "Rectangle {\n"
        "  property var dragCoords: ({})\n"
        "  property string draggedCoordsDump: \"\"\n"
        "  function rebuildDraggedDump() {\n"
        "    var lines = [];\n"
        "    for (var k in dragCoords) {\n"
        "      if (!dragCoords.hasOwnProperty(k) || !dragCoords[k]) continue;\n"
        "      lines.push(k + \"|\" + dragCoords[k].latitude.toFixed(6) + \",\" + dragCoords[k].longitude.toFixed(6));\n"
        "    }\n"
        "    draggedCoordsDump = lines.join(\"\\n\");\n"
        "  }\n"
        "  Component.onCompleted: {\n"
        "    %2\n"
        "    rebuildDraggedDump();\n"
        "  }\n"
        "  color: \"#0f1722\"\n"
        "  Plugin { id: osmPlugin; name: \"osm\"; PluginParameter { name: \"osm.mapping.providersrepository.disabled\"; value: true } }\n"
        "  Map { id: map; anchors.fill: parent; plugin: osmPlugin; center: QtPositioning.coordinate(36.8148, 10.3098); zoomLevel: 13.3; minimumZoomLevel: 5; maximumZoomLevel: 17;\n"
        "    activeMapType: supportedMapTypes.length > 0 ? supportedMapTypes[0] : null;\n"
        "    %1 }\n"
        "  WheelHandler { target: null; onWheel: (e) => { map.zoomLevel += e.angleDelta.y > 0 ? 0.6 : -0.6; } }\n"
        "  DragHandler { target: null; acceptedButtons: Qt.RightButton; onTranslationChanged: map.pan(-translation.x, -translation.y) }\n"
        "  Rectangle { anchors.right: parent.right; anchors.top: parent.top; anchors.margins: 12; width: 44; height: 92; radius: 8; color: \"#1f2a38\"; opacity: 0.92; border.color: \"#2f4054\"; border.width: 1\n"
        "    Column { anchors.fill: parent; anchors.margins: 4; spacing: 4\n"
        "      Button { text: \"+\"; onClicked: map.zoomLevel += 1 }\n"
        "      Button { text: \"-\"; onClicked: map.zoomLevel -= 1 }\n"
        "    }\n"
        "  }\n"
        "  Rectangle { anchors.left: parent.left; anchors.right: parent.right; anchors.bottom: parent.bottom; height: 42; color: \"#1f2a38\"; opacity: 0.93\n"
        "    Text { id: infoText; anchors.verticalCenter: parent.verticalCenter; anchors.left: parent.left; anchors.leftMargin: 10; color: \"#e6eff8\"; text: \"Marqueurs carrés = quais existants\"; font.pixelSize: 12 }\n"
        "  }\n"
        "}")
        .arg(markerItemsQml, dragInitQml);

    const QString qmlPath = QDir(QStandardPaths::writableLocation(QStandardPaths::TempLocation))
                                .filePath(QStringLiteral("atlas_quai_map.qml"));
    QFile qmlFile(qmlPath);
    if (qmlFile.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
        QTextStream qmlOut(&qmlFile);
        qmlOut.setEncoding(QStringConverter::Utf8);
        qmlOut << qml;
        qmlFile.close();
    }

    auto *quick = new QQuickWidget(&dlg);
    quick->setResizeMode(QQuickWidget::SizeRootObjectToView);
    quick->setClearColor(QColor("#0f1722"));
    quick->setSource(QUrl::fromLocalFile(qmlPath));
    if (!quick->errors().isEmpty() || quick->status() == QQuickWidget::Error || !quick->rootObject()) {
        QString err;
        for (const auto &e : quick->errors()) {
            if (!err.isEmpty()) err += QStringLiteral("\n");
            err += e.toString();
        }
        delete quick;
        auto *fallback = new QTextBrowser(&dlg);
        fallback->setStyleSheet(QStringLiteral("QTextBrowser { background:#0f1722; color:#dfe9f5; border:1px solid #2f4054; border-radius:8px; padding:8px; }"));
        fallback->setHtml(tr("<h3>Erreur de carte Qt</h3><p>%1</p>").arg(err.toHtmlEscaped()));
        layout->addWidget(fallback, 1);
    } else {
        layout->addWidget(quick, 1);
    }

    auto saveOneToDb = [this, &db, idCol](int quaiId, double lat, double lng) -> bool {
        if (quaiId <= 0) return false;
        QSqlQuery uq(db);
        uq.prepare(QStringLiteral("UPDATE QUAI SET MAP_LAT = :lat, MAP_LNG = :lng WHERE %1 = :id").arg(idCol));
        uq.bindValue(QStringLiteral(":lat"), lat);
        uq.bindValue(QStringLiteral(":lng"), lng);
        uq.bindValue(QStringLiteral(":id"), quaiId);
        if (!uq.exec()) {
            QMessageBox::warning(this, tr("Carte géographique"),
                                 tr("Erreur SQL enregistrement position (ID %1): %2").arg(quaiId).arg(uq.lastError().text()));
            return false;
        }
        if (uq.numRowsAffected() <= 0) {
            QMessageBox::warning(this, tr("Carte géographique"),
                                 tr("Aucune ligne mise à jour pour l'ID %1.").arg(quaiId));
            return false;
        }
        return true;
    };

    connect(btnSaveLatLng, &QPushButton::clicked, &dlg, [this, cmbQuai, spinLat, spinLng, saveOneToDb]() {
        if (!cmbQuai || !spinLat || !spinLng) return;
        const int qid = cmbQuai->currentData().toInt();
        const QString name = cmbQuai->currentText().trimmed();
        if (qid <= 0 || name.isEmpty()) return;
        if (saveOneToDb(qid, spinLat->value(), spinLng->value())) {
            QMessageBox::information(this, tr("Carte géographique"),
                                     tr("Position enregistrée en base pour %1.").arg(name));
        }
    });

    connect(btnSaveDrag, &QPushButton::clicked, &dlg, [this, quick, saveOneToDb]() {
        if (!quick || !quick->rootObject()) return;
        const QString dump = quick->rootObject()->property("draggedCoordsDump").toString();
        if (dump.trimmed().isEmpty()) {
            QMessageBox::warning(this, tr("Carte géographique"), tr("Impossible de lire les positions glissées."));
            return;
        }
        const QStringList lines = dump.split(QLatin1Char('\n'), Qt::SkipEmptyParts);
        int saved = 0;
        for (const QString &line : lines) {
            const QStringList pair = line.split(QLatin1Char('|'));
            if (pair.size() != 2) continue;
            bool okId = false;
            const int qid = pair.at(0).trimmed().toInt(&okId);
            if (!okId || qid <= 0) continue;
            const QStringList coords = pair.at(1).split(QLatin1Char(','), Qt::SkipEmptyParts);
            if (coords.size() != 2) continue;
            bool ok1 = false, ok2 = false;
            const double lat = coords.at(0).toDouble(&ok1);
            const double lng = coords.at(1).toDouble(&ok2);
            if (!ok1 || !ok2) continue;
            if (saveOneToDb(qid, lat, lng)) ++saved;
        }
        QMessageBox::information(this, tr("Carte géographique"),
                                 tr("%1 position(s) glissée(s) enregistrée(s) en base.").arg(saved));
    });

    connect(btnResetPos, &QPushButton::clicked, &dlg, [this, &db]() {
        QSqlQuery qReset(db);
        if (!qReset.exec(QStringLiteral("UPDATE QUAI SET MAP_LAT = NULL, MAP_LNG = NULL"))) {
            QMessageBox::warning(this, tr("Carte géographique"),
                                 tr("Erreur SQL réinitialisation: %1").arg(qReset.lastError().text()));
            return;
        }
        QMessageBox::information(this, tr("Carte géographique"),
                                 tr("Toutes les positions ont été réinitialisées en base.\nLes quais reviendront sur la ligne côtière au prochain affichage."));
    });

    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Close, &dlg);
    connect(buttons, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
    layout->addWidget(buttons);
    dlg.exec();
}

void MainWindow::refreshQuaiCharts()
{
    QSqlDatabase db = QSqlDatabase::database(Connection::connectionName());
    QSqlQuery q(db);
    const QString stCol = Quai::sqlQuaiColStatut().isEmpty() ? QStringLiteral("STATUT") : Quai::sqlQuaiColStatut();
    const QString tyCol = Quai::sqlQuaiColQuaiType().isEmpty() ? QStringLiteral("QUAI_TYPE") : Quai::sqlQuaiColQuaiType();
    QMap<QString, int> typeCounts;
    QMap<QString, double> typeAvgOcc;
    const QString occCol = Quai::sqlQuaiColOccupancyRate();
    const QString groupSql = occCol.isEmpty()
        ? QStringLiteral("SELECT %1, COUNT(*), 0 FROM QUAI GROUP BY %1").arg(tyCol)
        : QStringLiteral("SELECT %1, COUNT(*), NVL(AVG(%2), 0) FROM QUAI GROUP BY %1").arg(tyCol, occCol);
    if (db.isOpen() && q.exec(groupSql)) {
        while (q.next()) {
            QString typeName = q.value(0).toString().trimmed();
            if (typeName.isEmpty())
                typeName = tr("Non défini");
            typeCounts[typeName] = q.value(1).toInt();
            typeAvgOcc[typeName] = q.value(2).toDouble();
        }
    }

    refreshQuaiOccByTypeBarView(m_quaiOccByTypeBarView, typeCounts, typeAvgOcc);

    if (ui->tableOccupationByType) {
        ui->tableOccupationByType->setRowCount(0);
        int row = 0;
        for (auto it = typeCounts.cbegin(); it != typeCounts.cend(); ++it) {
            QString typeName = it.key();
            int count = it.value();
            double avgOcc = typeAvgOcc.value(typeName, 0);
            ui->tableOccupationByType->insertRow(row);
            ui->tableOccupationByType->setItem(row, 0, new QTableWidgetItem(typeName));
            ui->tableOccupationByType->setItem(row, 1, new QTableWidgetItem(QString::number(count)));
            ui->tableOccupationByType->setItem(row, 2, new QTableWidgetItem(QString::number(avgOcc, 'f', 1) + "%"));
            ui->tableOccupationByType->setItem(row, 3, new QTableWidgetItem(""));
            ++row;
        }
    }

    if (m_quaiChartView) {
        QChart *chart = m_quaiChartView->chart();
        chart->setBackgroundBrush(QBrush(QColor(0x26, 0x26, 0x26)));
        chart->setBackgroundVisible(true);
        chart->removeAllSeries();
        chart->setTitle(tr("Réf. : Quais par type"));
        chart->setTitleBrush(QBrush(QColor(0x2a, 0x9d, 0x8f)));
        chart->setTitleFont(QFont(QStringLiteral("Segoe UI"), QUAI_CHART_TITLE_PT, QFont::Bold));
        chart->setMargins(QMargins(20, 28, 20, 64));
        static const QMap<QString, QColor> typeColors = {
            { QStringLiteral("Pêche"), QColor(52, 152, 219) },
            { QStringLiteral("Commerce"), QColor(42, 157, 143) },
            { QStringLiteral("Maintenance"), QColor(243, 156, 18) },
            { QStringLiteral("Mixte"), QColor(231, 76, 60) },
            { QStringLiteral("Non défini"), QColor(149, 165, 166) }
        };
        static const QList<QColor> fallbackColors = {
            QColor(52, 152, 219), QColor(42, 157, 143), QColor(243, 156, 18), QColor(231, 76, 60), QColor(149, 165, 166)
        };
        auto colorForType = [&](const QString &type, int sliceIndex) -> QColor {
            for (auto it = typeColors.cbegin(); it != typeColors.cend(); ++it)
                if (it.key().compare(type, Qt::CaseInsensitive) == 0)
                    return it.value();
            return fallbackColors.value(sliceIndex % fallbackColors.size(), QColor(149, 165, 166));
        };
        int total = 0;
        for (auto it = typeCounts.cbegin(); it != typeCounts.cend(); ++it)
            total += it.value();
        auto *series = new QPieSeries();
        series->setHoleSize(0.36);
        series->setPieSize(0.82);
        QList<QColor> chart1Colors;
        int sliceIndex = 0;
        for (auto it = typeCounts.cbegin(); it != typeCounts.cend(); ++it, ++sliceIndex) {
            const QString typeName = it.key();
            const int count = it.value();
            const int pct = total > 0 ? qRound(100.0 * count / total) : 0;
            const QString tooltipLabel = total > 0
                ? QStringLiteral("%1 — %2 quais (%3%)").arg(typeName).arg(count).arg(pct)
                : typeName;
            const QString sliceLabel = quaiPieTwoLineLabel(typeName, count, pct);
            QColor baseColor = colorForType(typeName, sliceIndex);
            chart1Colors.append(baseColor);
            QPieSlice *slice = series->append(typeName, count);
            slice->setLabel(sliceLabel);
            slice->setLabelVisible(true);
            slice->setLabelPosition(QPieSlice::LabelOutside);
            slice->setLabelColor(QColor(0xe0, 0xe0, 0xe0));
            slice->setLabelFont(QFont(QStringLiteral("Segoe UI"), QUAI_CHART_SLICE_LABEL_PT, QFont::DemiBold));
            slice->setLabelArmLengthFactor(0.28);
            slice->setBrush(QBrush(baseColor, Qt::SolidPattern));
            slice->setColor(baseColor);
            slice->setBorderColor(QColor(0x26, 0x26, 0x26));
            slice->setBorderWidth(3);
            slice->setExploded(false);
            connect(slice, &QPieSlice::hovered, this, [slice, baseColor, tooltipLabel](bool state) {
                slice->setExploded(state);
                if (state) {
                    QColor hoverColor = baseColor.lighter(115);
                    slice->setBrush(QBrush(hoverColor, Qt::SolidPattern));
                    slice->setColor(hoverColor);
                    slice->setBorderWidth(4);
                    slice->setBorderColor(baseColor.lighter(130));
                    QToolTip::showText(QCursor::pos(), tooltipLabel, nullptr, QRect(), 2500);
                } else {
                    slice->setBrush(QBrush(baseColor, Qt::SolidPattern));
                    slice->setColor(baseColor);
                    slice->setBorderWidth(3);
                    slice->setBorderColor(QColor(0x26, 0x26, 0x26));
                    QToolTip::hideText();
                }
            });
        }
        if (series->count() == 0) {
            QPieSlice *empty = series->append(tr("Aucun quai"), 1);
            empty->setBrush(QBrush(QColor(0x7f, 0x8c, 0x8d), Qt::SolidPattern));
            empty->setColor(QColor(0x7f, 0x8c, 0x8d));
            empty->setLabelVisible(false);
        }
        chart->addSeries(series);
        const QList<QPieSlice *> slices1 = series->slices();
        for (int i = 0; i < slices1.size(); ++i) {
            QColor c = chart1Colors.value(i, QColor(0x7f, 0x8c, 0x8d));
            slices1.at(i)->setBrush(QBrush(c, Qt::SolidPattern));
            slices1.at(i)->setColor(c);
        }
    }

    int actifCount = 0, inactifCount = 0;
    if (q.exec(QStringLiteral("SELECT NVL(%1,'Actif'), COUNT(*) FROM QUAI GROUP BY NVL(%1,'Actif')").arg(stCol))) {
        while (q.next()) {
            QString s = q.value(0).toString().trimmed();
            if (s.isEmpty())
                s = tr("Actif");
            bool inactif = s.contains(tr("Inactif"), Qt::CaseInsensitive) || s.contains(QStringLiteral("Inactif"), Qt::CaseInsensitive);
            if (inactif)
                inactifCount += q.value(1).toInt();
            else
                actifCount += q.value(1).toInt();
        }
    }
    QList<QPair<QString, int>> actifInactifList;
    if (actifCount > 0)
        actifInactifList.append(qMakePair(tr("Actif"), actifCount));
    if (inactifCount > 0)
        actifInactifList.append(qMakePair(tr("Inactif"), inactifCount));
    if (m_quaiChartView2)
        fillDonut(m_quaiChartView2, tr("Réf. : Actifs / Inactifs"), actifInactifList,
                  { QColor(46, 204, 113), QColor(231, 76, 60), QColor(149, 165, 166) });

    int libres = 0, occupes = 0;
    QSqlQuery q3(db);
    const QString curU = Quai::sqlQuaiColCurrentUsage();
    if (!curU.isEmpty()) {
        if (q3.exec(QStringLiteral("SELECT COUNT(*) FROM QUAI WHERE NVL(%1, 0) = 0").arg(curU)) && q3.next())
            libres = q3.value(0).toInt();
        if (q3.exec(QStringLiteral("SELECT COUNT(*) FROM QUAI WHERE %1 > 0").arg(curU)) && q3.next())
            occupes = q3.value(0).toInt();
    }
    QList<QPair<QString, int>> libreOccupeList;
    libreOccupeList.append(qMakePair(tr("Libres"), libres));
    libreOccupeList.append(qMakePair(tr("Occupés"), occupes));
    if (m_quaiChartView3)
        fillDonut(m_quaiChartView3, tr("Réf. : Libres / Occupés"), libreOccupeList,
                  { QColor(46, 204, 113), QColor(231, 76, 60) });

    int maint = 0, autres = 0;
    QSqlQuery q4(db);
    if (q4.exec(QStringLiteral("SELECT COUNT(*) FROM QUAI WHERE LOWER(NVL(%1,'')) LIKE '%%maintenance%%' OR LOWER(NVL(%2,'')) = 'maintenance'")
                    .arg(stCol, tyCol))
        && q4.next())
        maint = q4.value(0).toInt();
    if (q4.exec(QStringLiteral("SELECT COUNT(*) FROM QUAI")) && q4.next())
        autres = q4.value(0).toInt() - maint;
    if (autres < 0)
        autres = 0;
    QList<QPair<QString, int>> maintList;
    maintList.append(qMakePair(tr("En maint."), maint));
    maintList.append(qMakePair(tr("Hors maint."), autres));
    if (m_quaiChartView4)
        fillDonut(m_quaiChartView4, tr("Réf. : En / Hors maint."), maintList,
                  { QColor(52, 152, 219), QColor(243, 156, 18) });
}

void MainWindow::refreshQuaiResumeLine()
{
    if (!ui->lblResumeQuais && !ui->lblResumeMAJ)
        return;
    QSqlDatabase db = QSqlDatabase::database(Connection::connectionName());
    if (!db.isOpen())
        return;
    const QString typeCol = Quai::sqlQuaiColQuaiType();
    const QString statutCol = Quai::sqlQuaiColStatut();
    const QString capCol = Quai::sqlQuaiColCapacite();
    QSqlQuery q(db);
    int total = 0;
    if (q.exec(QStringLiteral("SELECT COUNT(*) FROM QUAI")) && q.next())
        total = q.value(0).toInt();
    int actifs = total;
    if (!statutCol.isEmpty()) {
        if (q.exec(QStringLiteral(
                       "SELECT COUNT(*) FROM QUAI WHERE LOWER(NVL(%1,'')) NOT LIKE '%%inactif%%' AND LOWER(NVL(%1,'')) NOT LIKE '%%maintenance%%'")
                       .arg(statutCol))
            && q.next())
            actifs = q.value(0).toInt();
    }
    qint64 capSum = 0;
    if (!capCol.isEmpty()) {
        if (q.exec(QStringLiteral("SELECT NVL(SUM(%1), 0) FROM QUAI").arg(capCol)) && q.next())
            capSum = q.value(0).toLongLong();
    }
    int zoneCount = 0;
    if (!typeCol.isEmpty()) {
        if (q.exec(QStringLiteral("SELECT COUNT(DISTINCT %1) FROM QUAI WHERE %1 IS NOT NULL").arg(typeCol)) && q.next())
            zoneCount = q.value(0).toInt();
    }
    if (ui->lblResumeQuais) {
        ui->lblResumeQuais->setText(tr("Quais actifs: %1 | Capacité totale: %2 | Types distincts: %3")
                                        .arg(actifs)
                                        .arg(capSum)
                                        .arg(zoneCount));
    }
    if (ui->lblResumeMAJ) {
        ui->lblResumeMAJ->setText(
            tr("Dernière synchro: %1").arg(QLocale().toString(QDateTime::currentDateTime(), QLocale::ShortFormat)));
    }
}

void MainWindow::refreshQuaiSupervisionTable()
{
    if (!ui->tableSupervisionCombined)
        return;
    QTableWidget *tw = ui->tableSupervisionCombined;
    tw->setSortingEnabled(false);
    tw->setRowCount(0);
    QSqlDatabase db = QSqlDatabase::database(Connection::connectionName());
    if (!db.isOpen())
        return;
    const QString nomCol = Quai::sqlQuaiColQuaiNom();
    const QString typeCol = Quai::sqlQuaiColQuaiType();
    const QString statutCol = Quai::sqlQuaiColStatut();
    const QString occCol = Quai::sqlQuaiColOccupancyRate();
    const QString incCol = Quai::sqlQuaiColIncidentLog();
    const QString curU = Quai::sqlQuaiColCurrentUsage();
    if (nomCol.isEmpty())
        return;

    const QString occExpr = occCol.isEmpty() ? QStringLiteral("0") : QStringLiteral("NVL(%1, 0)").arg(occCol);
    const QString typeExpr = typeCol.isEmpty() ? QStringLiteral("CAST(NULL AS VARCHAR2(1))") : typeCol;
    const QString statExpr = statutCol.isEmpty() ? QStringLiteral("CAST(NULL AS VARCHAR2(1))") : statutCol;
    const QString incExpr = incCol.isEmpty() ? QStringLiteral("CAST(NULL AS VARCHAR2(1))") : incCol;
    const QString usageExpr = curU.isEmpty() ? QStringLiteral("0") : QStringLiteral("NVL(%1, 0)").arg(curU);

    const QString sql = QStringLiteral("SELECT %1, %2, %3, %4, %5, %6 FROM QUAI")
                            .arg(nomCol, typeExpr, statExpr, occExpr, incExpr, usageExpr);
    QSqlQuery q(db);
    if (!q.exec(sql))
        return;

    auto addRow = [tw](const QString &source, const QString &quai, const QString &zone, const QString &type,
                       const QString &statut, const QString &detail, const QString &dateStr) {
        const int r = tw->rowCount();
        tw->insertRow(r);
        tw->setItem(r, 0, new QTableWidgetItem(source));
        tw->setItem(r, 1, new QTableWidgetItem(quai));
        tw->setItem(r, 2, new QTableWidgetItem(zone));
        tw->setItem(r, 3, new QTableWidgetItem(type));
        tw->setItem(r, 4, new QTableWidgetItem(statut));
        tw->setItem(r, 5, new QTableWidgetItem(detail));
        tw->setItem(r, 6, new QTableWidgetItem(dateStr));
    };

    const QString nowStr = QLocale().toString(QDateTime::currentDateTime(), QLocale::ShortFormat);
    while (q.next()) {
        const QString nom = q.value(0).toString();
        const QString type = q.value(1).toString();
        const QString statut = q.value(2).toString();
        const double occ = q.value(3).toDouble();
        const QString incident = q.value(4).toString().trimmed();
        Q_UNUSED(q.value(5));

        if (!incident.isEmpty())
            addRow(tr("Journal"), nom, type, type, statut, incident, nowStr);

        const bool maint = statut.contains(QStringLiteral("maintenance"), Qt::CaseInsensitive)
            || type.compare(QStringLiteral("Maintenance"), Qt::CaseInsensitive) == 0;
        if (maint) {
            addRow(tr("Supervision"), nom, type, type, statut,
                   tr("Quai signalé en maintenance ou type maintenance."), nowStr);
        }
        if (occ >= 85.0) {
            addRow(tr("Alerte"), nom, type, type, statut,
                   tr("Taux d'occupation élevé (%1%).").arg(QString::number(occ, 'f', 1)), nowStr);
        }
    }
    if (tw->rowCount() == 0) {
        addRow(tr("Système"), QStringLiteral("—"), QStringLiteral("—"), QStringLiteral("—"), QStringLiteral("—"),
               tr("Aucun incident enregistré et aucune alerte critique."), nowStr);
    }
    tw->resizeColumnsToContents();
    if (tw->columnCount() > 5)
        tw->horizontalHeader()->setSectionResizeMode(5, QHeaderView::Stretch);
    tw->resizeRowsToContents();
    tw->setSortingEnabled(true);
    if (tw->rowCount() > 0) {
        tw->setCurrentCell(0, 0);
        tw->selectRow(0);
    }
}

struct QuaiIncidentAlertStats {
    int quaisWithJournal = 0;
    int totalLines = 0;
    QDateTime lastEntry;
};

static QuaiIncidentAlertStats computeQuaiIncidentAlertStats()
{
    QuaiIncidentAlertStats s;
    const QString inc = Quai::sqlQuaiColIncidentLog();
    if (inc.isEmpty())
        return s;
    QSqlDatabase db = QSqlDatabase::database(Connection::connectionName());
    if (!db.isOpen())
        return s;
    QSqlQuery q(db);
    if (!q.exec(QStringLiteral("SELECT %1 FROM QUAI WHERE %1 IS NOT NULL").arg(inc)))
        return s;
    static const QRegularExpression isoRe(QStringLiteral(
        R"(^(\d{4}-\d{2}-\d{2}[T ]\d{2}:\d{2}:\d{2}(?:\.\d+)?))"));
    while (q.next()) {
        const QString log = q.value(0).toString();
        if (log.trimmed().isEmpty())
            continue;
        s.quaisWithJournal++;
        const QStringList lines = log.split(QChar('\n'), Qt::SkipEmptyParts);
        for (QString line : lines) {
            line = line.trimmed();
            if (line.isEmpty())
                continue;
            s.totalLines++;
            const QRegularExpressionMatch m = isoRe.match(line);
            if (!m.hasMatch())
                continue;
            QString iso = m.captured(1);
            iso.replace(QLatin1Char(' '), QLatin1Char('T'));
            QDateTime dt = QDateTime::fromString(iso, Qt::ISODateWithMs);
            if (!dt.isValid())
                dt = QDateTime::fromString(iso, Qt::ISODate);
            if (dt.isValid() && (!s.lastEntry.isValid() || dt > s.lastEntry))
                s.lastEntry = dt;
        }
    }
    return s;
}

void MainWindow::startQuaiAlertsPolling()
{
    if (!m_quaiAlertsPollTimer)
        return;
    QSqlDatabase db = QSqlDatabase::database(Connection::connectionName());
    if (!db.isOpen())
        return;
    m_quaiAlertsPollTimer->start();
}

void MainWindow::stopQuaiAlertsPolling()
{
    if (m_quaiAlertsPollTimer)
        m_quaiAlertsPollTimer->stop();
}

void MainWindow::refreshQuaiAlertesImportantFromDb(bool refreshSupervisionTable)
{
    QSqlDatabase db = QSqlDatabase::database(Connection::connectionName());
    if (!db.isOpen() || !ui->textEditAlertes)
        return;
    double avgOcc = 0;
    int maintCount = 0;
    int highOccQuais = 0;
    int inactiveQuais = 0;
    QSqlQuery q(db);
    const QString occAvg = Quai::sqlQuaiColOccupancyRate();
    const QString stCol = Quai::sqlQuaiColStatut().isEmpty() ? QStringLiteral("STATUT") : Quai::sqlQuaiColStatut();
    const QString tyCol = Quai::sqlQuaiColQuaiType().isEmpty() ? QStringLiteral("QUAI_TYPE") : Quai::sqlQuaiColQuaiType();
    if (!occAvg.isEmpty() && q.exec(QStringLiteral("SELECT NVL(AVG(%1), 0) FROM QUAI").arg(occAvg)) && q.next())
        avgOcc = q.value(0).toDouble();
    if (!occAvg.isEmpty()
        && q.exec(QStringLiteral("SELECT COUNT(*) FROM QUAI WHERE NVL(%1, 0) >= 85").arg(occAvg)) && q.next())
        highOccQuais = q.value(0).toInt();
    if (q.exec(QStringLiteral("SELECT COUNT(*) FROM QUAI WHERE LOWER(NVL(%1,'')) LIKE '%%maintenance%%' OR LOWER(NVL(%2,'')) = 'maintenance'")
                   .arg(stCol, tyCol))
        && q.next())
        maintCount = q.value(0).toInt();
    if (!stCol.isEmpty()
        && q.exec(QStringLiteral("SELECT COUNT(*) FROM QUAI WHERE LOWER(NVL(%1,'')) LIKE '%%inactif%%'").arg(stCol))
        && q.next())
        inactiveQuais = q.value(0).toInt();

    const QuaiIncidentAlertStats inc = computeQuaiIncidentAlertStats();
    const QDateTime now = QDateTime::currentDateTime();
    const QString nowStr = QLocale().toString(now, QLocale::ShortFormat);
    const QString lastIncStr = inc.lastEntry.isValid()
        ? QLocale().toString(inc.lastEntry, QLocale::ShortFormat)
        : QStringLiteral("—");

    const qint64 secSinceLastIncident = inc.lastEntry.isValid() ? inc.lastEntry.secsTo(now) : -1;
    const bool incidentLastHour = inc.lastEntry.isValid() && secSinceLastIncident >= 0 && secSinceLastIncident <= 3600;
    const bool incidentLastDay = inc.lastEntry.isValid() && secSinceLastIncident >= 0 && secSinceLastIncident <= 86400;

    QString html;
    html += QStringLiteral("<p style='margin:0 0 8px 0;font-size:13px;font-weight:bold;color:#e74c3c;letter-spacing:0.5px;'>%1</p>")
                .arg(tr("AVIS IMPORTANTS (temps réel)"));

    QStringList priorityBlocks;
    if (maintCount > 0) {
        priorityBlocks << QStringLiteral(
            "<p style='margin:5px 0;padding:8px 10px;border-radius:6px;background:#5c1f1f;border-left:4px solid #e74c3c;'>"
            "<b style='color:#ffcccc;'>%1</b> <span style='color:#fadbd8;'>%2</span></p>")
                                .arg(tr("MAINTENANCE"),
                                     tr("%1 quai(s) en maintenance ou type maintenance — prioriser contrôle et planning.")
                                         .arg(maintCount));
    }
    if (highOccQuais > 0) {
        priorityBlocks << QStringLiteral(
            "<p style='margin:5px 0;padding:8px 10px;border-radius:6px;background:#5d4a1a;border-left:4px solid #f39c12;'>"
            "<b style='color:#fdebd0;'>%1</b> <span style='color:#f9e79f;'>%2</span></p>")
                                .arg(tr("FORTE OCCUPATION"),
                                     tr("%1 quai(s) avec taux d'occupation ≥ 85 % — risque de saturation.")
                                         .arg(highOccQuais));
    }
    if (inactiveQuais > 0) {
        priorityBlocks << QStringLiteral(
            "<p style='margin:5px 0;padding:8px 10px;border-radius:6px;background:#2c3e50;border-left:4px solid #9b59b6;'>"
            "<b style='color:#e8daef;'>%1</b> <span style='color:#d7bde2;'>%2</span></p>")
                                .arg(tr("QUAIS INACTIFS"),
                                     tr("%1 quai(s) marqué(s) inactif — vérifier disponibilité portuaire.")
                                         .arg(inactiveQuais));
    }
    if (!occAvg.isEmpty() && avgOcc >= 75.0 && highOccQuais == 0) {
        priorityBlocks << QStringLiteral(
            "<p style='margin:5px 0;padding:8px 10px;border-radius:6px;background:#4a3728;border-left:4px solid #e67e22;'>"
            "<b style='color:#fdedec;'>%1</b> <span style='color:#f5cba7;'>%2</span></p>")
                                .arg(tr("PRESSION GLOBALE"),
                                     tr("Occupation moyenne élevée (%1 %) — surveiller l'affectation des quais.")
                                         .arg(QString::number(avgOcc, 'f', 1)));
    }
    if (incidentLastHour && inc.totalLines > 0) {
        priorityBlocks << QStringLiteral(
            "<p style='margin:5px 0;padding:8px 10px;border-radius:6px;background:#1a3a52;border-left:4px solid #3498db;'>"
            "<b style='color:#aed6f1;'>%1</b> <span style='color:#d6eaf8;'>%2</span></p>")
                                .arg(tr("INCIDENT RÉCENT"),
                                     tr("Journal mis à jour il y a moins d'une heure (dernier : %1).").arg(lastIncStr));
    } else if (incidentLastDay && inc.totalLines > 0 && !incidentLastHour) {
        priorityBlocks << QStringLiteral(
            "<p style='margin:5px 0;padding:8px 10px;border-radius:6px;background:#1e3d2f;border-left:4px solid #1abc9c;'>"
            "<b style='color:#d5f5e3;'>%1</b> <span style='color:#abebc6;'>%2</span></p>")
                                .arg(tr("ACTIVITÉ INCIDENT"),
                                     tr("Entrée dans les dernières 24 h. Dernier horodatage : %1.").arg(lastIncStr));
    }

    if (priorityBlocks.isEmpty()) {
        html += QStringLiteral(
            "<p style='margin:4px 0 10px 0;padding:8px 10px;border-radius:6px;background:#1e3d2e;border-left:4px solid #27ae60;'>"
            "<span style='color:#d5f5e3;'>%1</span></p>")
                    .arg(tr("Aucune alerte critique prioritaire. Surveillance automatique active (base de données)."));
    } else {
        for (const QString &b : priorityBlocks)
            html += b;
    }

    html += QStringLiteral("<p style='margin:12px 0 6px 0;font-size:12px;font-weight:bold;color:#bdc3c7;border-bottom:1px solid #555;padding-bottom:4px;'>%1</p>")
                .arg(tr("Indicateurs détaillés"));

    html += QStringLiteral("<p style='margin:4px 0;line-height:1.35;'><span style='color:#e74c3c;font-weight:bold;'>•</span> "
                           "<b style='color:#ecf0f1;'>%1</b> <span style='color:#e0e0e0;'>%2%</span></p>")
                .arg(tr("Taux d'occupation (moyenne) :"), QString::number(avgOcc, 'f', 1));
    html += QStringLiteral("<p style='margin:4px 0;line-height:1.35;'><span style='color:#e74c3c;font-weight:bold;'>•</span> "
                           "<b style='color:#ecf0f1;'>%1</b> <span style='color:#e0e0e0;'>%2</span></p>")
                .arg(tr("Quais en maintenance :"), QString::number(maintCount));
    if (!stCol.isEmpty()) {
        html += QStringLiteral("<p style='margin:4px 0;line-height:1.35;'><span style='color:#9b59b6;font-weight:bold;'>•</span> "
                               "<b style='color:#ecf0f1;'>%1</b> <span style='color:#e0e0e0;'>%2</span></p>")
                    .arg(tr("Quais inactifs :"), QString::number(inactiveQuais));
    }
    if (!occAvg.isEmpty()) {
        html += QStringLiteral("<p style='margin:4px 0;line-height:1.35;'><span style='color:#f39c12;font-weight:bold;'>•</span> "
                               "<b style='color:#ecf0f1;'>%1</b> <span style='color:#e0e0e0;'>%2</span></p>")
                    .arg(tr("Quais à occupation ≥ 85 % :"), QString::number(highOccQuais));
    }
    if (!Quai::sqlQuaiColIncidentLog().isEmpty()) {
        html += QStringLiteral("<p style='margin:4px 0;line-height:1.35;'><span style='color:#3498db;font-weight:bold;'>•</span> "
                               "<b style='color:#ecf0f1;'>%1</b> <span style='color:#e0e0e0;'>%2</span></p>")
                    .arg(tr("Quais avec journal d'incidents :"), QString::number(inc.quaisWithJournal));
        html += QStringLiteral("<p style='margin:4px 0;line-height:1.35;'><span style='color:#3498db;font-weight:bold;'>•</span> "
                               "<b style='color:#ecf0f1;'>%1</b> <span style='color:#e0e0e0;'>%2</span></p>")
                    .arg(tr("Entrées enregistrées (lignes) :"), QString::number(inc.totalLines));
        html += QStringLiteral("<p style='margin:4px 0;line-height:1.35;'><span style='color:#3498db;font-weight:bold;'>•</span> "
                               "<b style='color:#ecf0f1;'>%1</b> <span style='color:#aed6f1;'>%2</span></p>")
                    .arg(tr("Dernier incident (horodatage) :"), lastIncStr);
    }
    html += QStringLiteral("<p style='margin:6px 0 2px 0;line-height:1.35;'><span style='color:#95a5a6;font-weight:bold;'>•</span> "
                           "<b style='color:#bdc3c7;'>%1</b> <span style='color:#95a5a6;'>%2</span></p>")
                .arg(tr("Dernière actualisation de la vue :"), nowStr);
    html += QStringLiteral("<p style='margin:4px 0 0 0;font-size:11px;color:#7f8c8d;font-style:italic;'>%1</p>")
                .arg(tr("Bouton « Actualiser » ou clic sur ce texte • Mise à jour auto toutes les 12 s (module Quais)."));

    const QString fullHtml = QStringLiteral(
        "<html><head><meta charset=\"utf-8\"/></head>"
        "<body style=\"background-color:#3d2525;margin:0;padding:6px 4px 10px 4px;\">%1</body></html>")
                                 .arg(html);
    ui->textEditAlertes->setHtml(fullHtml);
    ui->textEditAlertes->moveCursor(QTextCursor::Start);
    ui->textEditAlertes->ensureCursorVisible();
    refreshQuaiResumeLine();
    if (refreshSupervisionTable)
        refreshQuaiSupervisionTable();
}

void MainWindow::refreshQuaiSupervisionData()
{
    refreshQuaiAlertesImportantFromDb(true);
}

void MainWindow::loadQuaisTable()
{
    if (!ui->tableQuais) return;
    QSqlDatabase db = QSqlDatabase::database(Connection::connectionName());
    if (!db.isOpen()) return;
    QString where = quaiWhereClause();
    QString order = quaiOrderClause();
    QSqlQuery q(db);
    QString sql = QStringLiteral("SELECT %1 FROM QUAI WHERE 1=1").arg(Quai::sqlQuaiSelectList()) + where + order;
    if (!q.exec(sql)) { ui->tableQuais->setRowCount(0); return; }
    ui->tableQuais->setSortingEnabled(false);
    ui->tableQuais->setRowCount(0);
    while (q.next()) {
        int row = ui->tableQuais->rowCount();
        ui->tableQuais->insertRow(row);
        for (int c = 0; c < 12; ++c) {
            QVariant v = q.value(c);
            QString t;
            if (v.isNull()) t = "";
            else if (v.typeId() == QMetaType::Double) t = QString::number(v.toDouble(), 'f', 2);
            else t = v.toString();
            ui->tableQuais->setItem(row, c, new QTableWidgetItem(t));
        }

        // Readability + visual status coding for operators.
        const QString statut = ui->tableQuais->item(row, 6) ? ui->tableQuais->item(row, 6)->text().trimmed() : QString();
        const QString statutLower = statut.toLower();
        const bool isMaintenance = statutLower.contains(QStringLiteral("maintenance"));
        const bool isIncident = statutLower.contains(QStringLiteral("incident"));
        const bool isInactif = statutLower.contains(QStringLiteral("inactif"));
        const bool isActif = !isInactif && statutLower.contains(QStringLiteral("actif"));
        const QColor fg = QColor("#f4f7fb");
        // Force a color on every row: red (incident/maintenance), green (active), yellow (others).
        const QColor bg = (isIncident || isMaintenance) ? QColor(140, 38, 38, 170)   // red
                        : isActif ? QColor(28, 110, 74, 160)                            // green
                                   : QColor(138, 110, 24, 160);                         // yellow

        for (int c = 0; c < 12; ++c) {
            if (QTableWidgetItem *it = ui->tableQuais->item(row, c)) {
                it->setBackground(QBrush(bg));
                it->setForeground(QBrush(fg));
                if (c == 6) {
                    QFont f = it->font();
                    f.setBold(true);
                    it->setFont(f);
                    it->setTextAlignment(Qt::AlignCenter);
                }
            }
        }
        ui->tableQuais->setRowHeight(row, 34);
    }
    ui->tableQuais->setSortingEnabled(true);
    ui->tableQuais->setAlternatingRowColors(false);
    ui->tableQuais->setStyleSheet(ui->tableQuais->styleSheet() +
        QStringLiteral("QTableWidget::item:alternate { background: transparent; }"));
    if (ui->tableQuais->horizontalHeader())
        ui->tableQuais->horizontalHeader()->setDefaultAlignment(Qt::AlignCenter);
    refreshQuaiStats();
    refreshQuaiCharts();
    refreshQuaiSupervisionData();
    refreshQuaiMapOverview();
}

void MainWindow::clearQuaiForm()
{
    if (ui->lineEditQuaiID) ui->lineEditQuaiID->clear();
    if (ui->lineEditNom) ui->lineEditNom->clear();
    if (ui->radioTypePeche) ui->radioTypePeche->setChecked(true);
    if (ui->doubleSpinBoxLongueur) ui->doubleSpinBoxLongueur->setValue(0);
    if (ui->doubleSpinBoxProfondeur) ui->doubleSpinBoxProfondeur->setValue(0);
    if (ui->spinBoxCapacite) ui->spinBoxCapacite->setValue(1);
    if (ui->radioStatutActif) ui->radioStatutActif->setChecked(true);
    if (ui->spinBoxCurrentUsage) ui->spinBoxCurrentUsage->setValue(0);
    if (ui->doubleSpinBoxOccupation) ui->doubleSpinBoxOccupation->setValue(0);
    if (ui->radioPrioriteBasse) ui->radioPrioriteBasse->setChecked(true);
    if (ui->radioSecurite1) ui->radioSecurite1->setChecked(true);
    if (ui->radioEclairageAllume) ui->radioEclairageAllume->setChecked(true);
}

void MainWindow::fillQuaiFormFromRow(int row)
{
    if (!ui->tableQuais || row < 0 || row >= ui->tableQuais->rowCount()) return;
    auto cell = [this, row](int col) -> QString {
        QTableWidgetItem *it = ui->tableQuais->item(row, col);
        return it ? it->text().trimmed() : QString();
    };
    if (ui->lineEditQuaiID) ui->lineEditQuaiID->setText(cell(0));
    if (ui->lineEditNom) ui->lineEditNom->setText(cell(1));
    QString type = cell(2);
    if (ui->radioTypePeche) ui->radioTypePeche->setChecked(type.contains("Pêche", Qt::CaseInsensitive));
    if (ui->radioTypeCommerce) ui->radioTypeCommerce->setChecked(type.contains("Commerce", Qt::CaseInsensitive));
    if (ui->radioTypeMaintenance) ui->radioTypeMaintenance->setChecked(type.contains("Maintenance", Qt::CaseInsensitive));
    if (ui->radioTypeMixte) ui->radioTypeMixte->setChecked(type.contains("Mixte", Qt::CaseInsensitive));
    if (ui->doubleSpinBoxLongueur) ui->doubleSpinBoxLongueur->setValue(cell(3).replace(',', '.').toDouble());
    if (ui->doubleSpinBoxProfondeur) ui->doubleSpinBoxProfondeur->setValue(cell(4).replace(',', '.').toDouble());
    if (ui->spinBoxCapacite) ui->spinBoxCapacite->setValue(cell(5).toInt());
    QString statut = cell(6);
    if (ui->radioStatutActif) ui->radioStatutActif->setChecked(statut.contains("Actif", Qt::CaseInsensitive) || statut.isEmpty());
    if (ui->radioStatutInactif) ui->radioStatutInactif->setChecked(statut.contains("Inactif", Qt::CaseInsensitive));
    if (ui->radioStatutMaintenance) ui->radioStatutMaintenance->setChecked(statut.contains("Maintenance", Qt::CaseInsensitive));
    if (ui->spinBoxCurrentUsage) ui->spinBoxCurrentUsage->setValue(cell(7).toInt());
    if (ui->doubleSpinBoxOccupation) ui->doubleSpinBoxOccupation->setValue(cell(8).replace(',', '.').toDouble());
    QString priorite = cell(9);
    if (ui->radioPrioriteBasse) ui->radioPrioriteBasse->setChecked(priorite.contains("Basse", Qt::CaseInsensitive));
    if (ui->radioPrioriteNormale)
        ui->radioPrioriteNormale->setChecked(priorite.contains("Normale", Qt::CaseInsensitive)
                                             || priorite.contains("Moyenne", Qt::CaseInsensitive));
    if (ui->radioPrioriteHaute) ui->radioPrioriteHaute->setChecked(priorite.contains("Haute", Qt::CaseInsensitive));
    QString safety = cell(10);
    const QString su = safety.trimmed().toUpper();
    if (ui->radioSecurite1) {
        ui->radioSecurite1->setChecked(safety.contains("Niveau 1", Qt::CaseInsensitive) || safety.isEmpty()
                                       || su == QLatin1String("A") || su.startsWith(QLatin1String("A ")));
    }
    if (ui->radioSecurite2) {
        ui->radioSecurite2->setChecked(safety.contains("Niveau 2", Qt::CaseInsensitive) || su == QLatin1String("B")
                                       || su.startsWith(QLatin1String("B ")));
    }
    if (ui->radioSecurite3) {
        ui->radioSecurite3->setChecked(safety.contains("Niveau 3", Qt::CaseInsensitive) || su == QLatin1String("C")
                                       || su.startsWith(QLatin1String("C ")));
    }
    QString lighting = cell(11);
    if (ui->radioEclairageAllume) {
        ui->radioEclairageAllume->setChecked(lighting.contains("Allumé", Qt::CaseInsensitive) || lighting.isEmpty()
                                             || lighting.contains("Fonctionnel", Qt::CaseInsensitive));
    }
    if (ui->radioEclairageEteint) ui->radioEclairageEteint->setChecked(lighting.contains("Éteint", Qt::CaseInsensitive));
    if (ui->radioEclairageDefaillant) {
        ui->radioEclairageDefaillant->setChecked(lighting.contains("Défaillant", Qt::CaseInsensitive)
                                                 || lighting.contains("En panne", Qt::CaseInsensitive));
    }
}

static QString quaiTypeFromRadios(Ui::MainWindow *ui) {
    if (ui->radioTypePeche && ui->radioTypePeche->isChecked()) return QStringLiteral("Pêche");
    if (ui->radioTypeCommerce && ui->radioTypeCommerce->isChecked()) return QStringLiteral("Commerce");
    if (ui->radioTypeMaintenance && ui->radioTypeMaintenance->isChecked()) return QStringLiteral("Maintenance");
    return QStringLiteral("Mixte");
}
static QString quaiStatutFromRadios(Ui::MainWindow *ui) {
    if (ui->radioStatutActif && ui->radioStatutActif->isChecked()) return QStringLiteral("Actif");
    if (ui->radioStatutInactif && ui->radioStatutInactif->isChecked()) return QStringLiteral("Inactif");
    return QStringLiteral("En Maintenance");
}
static QString quaiPrioriteFromRadios(Ui::MainWindow *ui) {
    if (ui->radioPrioriteBasse && ui->radioPrioriteBasse->isChecked()) return QStringLiteral("Basse");
    if (ui->radioPrioriteHaute && ui->radioPrioriteHaute->isChecked()) return QStringLiteral("Haute");
    return QStringLiteral("Normale");
}
static QString quaiSafetyFromRadios(Ui::MainWindow *ui) {
    if (ui->radioSecurite2 && ui->radioSecurite2->isChecked()) return QStringLiteral("Niveau 2");
    if (ui->radioSecurite3 && ui->radioSecurite3->isChecked()) return QStringLiteral("Niveau 3");
    return QStringLiteral("Niveau 1");
}
static QString quaiLightingFromRadios(Ui::MainWindow *ui) {
    if (ui->radioEclairageEteint && ui->radioEclairageEteint->isChecked()) return QStringLiteral("Éteint");
    if (ui->radioEclairageDefaillant && ui->radioEclairageDefaillant->isChecked()) return QStringLiteral("Défaillant");
    return QStringLiteral("Allumé");
}

static bool isSafeLabelText(const QString &value)
{
    // Letters (incl. accents), digits, spaces, apostrophe and hyphen.
    static const QRegularExpression rx(QStringLiteral("^[\\p{L}0-9][\\p{L}0-9 '\\-]{1,79}$"));
    return rx.match(value).hasMatch();
}

static bool isLettersWordsOnly(const QString &value)
{
    // Letters (incl. accents), spaces, apostrophe and hyphen. No digits/symbols.
    static const QRegularExpression rx(QStringLiteral("^[\\p{L}][\\p{L} '\\-]{1,79}$"));
    return rx.match(value).hasMatch();
}

static bool ensureEquipPhotoTable(QSqlDatabase db, QString *error = nullptr)
{
    if (!db.isOpen()) {
        if (error) *error = QObject::tr("Base de données non ouverte.");
        return false;
    }
    QSqlQuery q(db);
    q.prepare(QStringLiteral("SELECT COUNT(*) FROM USER_TABLES WHERE UPPER(TABLE_NAME)=UPPER('EQUIPEMENT_PHOTO')"));
    if (!q.exec() || !q.next()) {
        if (error) *error = q.lastError().text();
        return false;
    }
    if (q.value(0).toInt() > 0)
        return true;

    const QString ddl = QStringLiteral(
        "CREATE TABLE EQUIPEMENT_PHOTO ("
        "ID_EQUIPEMENT NUMBER PRIMARY KEY, "
        "PHOTO_PATH VARCHAR2(1024), "
        "UPDATED_AT DATE DEFAULT SYSDATE)");
    if (!q.exec(ddl)) {
        if (error) *error = q.lastError().text();
        return false;
    }
    db.commit();
    return true;
}

static bool saveEquipPhotoPath(int equipId, const QString &photoPath, QString *error = nullptr)
{
    QSqlDatabase db = QSqlDatabase::database(Connection::connectionName());
    if (!ensureEquipPhotoTable(db, error))
        return false;

    QSqlQuery q(db);
    q.prepare(QStringLiteral(
        "MERGE INTO EQUIPEMENT_PHOTO t "
        "USING (SELECT :id AS ID_EQUIPEMENT, :path AS PHOTO_PATH FROM DUAL) s "
        "ON (t.ID_EQUIPEMENT = s.ID_EQUIPEMENT) "
        "WHEN MATCHED THEN UPDATE SET t.PHOTO_PATH = s.PHOTO_PATH, t.UPDATED_AT = SYSDATE "
        "WHEN NOT MATCHED THEN INSERT (ID_EQUIPEMENT, PHOTO_PATH, UPDATED_AT) "
        "VALUES (s.ID_EQUIPEMENT, s.PHOTO_PATH, SYSDATE)"));
    q.bindValue(QStringLiteral(":id"), equipId);
    q.bindValue(QStringLiteral(":path"), photoPath);
    if (!q.exec()) {
        if (error) *error = q.lastError().text();
        return false;
    }
    db.commit();
    return true;
}

static QString loadEquipPhotoPath(int equipId, QString *error = nullptr)
{
    QSqlDatabase db = QSqlDatabase::database(Connection::connectionName());
    if (!db.isOpen()) {
        if (error) *error = QObject::tr("Base de données non ouverte.");
        return QString();
    }
    if (!ensureEquipPhotoTable(db, nullptr))
        return QString();

    QSqlQuery q(db);
    q.prepare(QStringLiteral("SELECT PHOTO_PATH FROM EQUIPEMENT_PHOTO WHERE ID_EQUIPEMENT = :id"));
    q.bindValue(QStringLiteral(":id"), equipId);
    if (!q.exec()) {
        if (error) *error = q.lastError().text();
        return QString();
    }
    if (!q.next())
        return QString();
    return q.value(0).toString().trimmed();
}

static int nextEquipementId()
{
    QSqlDatabase db = QSqlDatabase::database(Connection::connectionName());
    if (!db.isOpen())
        return 0;
    QSqlQuery q(db);
    if (q.exec(QStringLiteral("SELECT SEQ_EQUIPEMENT_ID.NEXTVAL FROM DUAL")) && q.next())
        return q.value(0).toInt();
    if (q.exec(QStringLiteral("SELECT NVL(MAX(ID_EQUIPEMENT),0)+1 FROM EQUIPEMENT")) && q.next())
        return q.value(0).toInt();
    return 0;
}

static void showEquipPhotoPreview(QLabel *preview, const QString &photoPath)
{
    if (!preview)
        return;
    if (photoPath.isEmpty() || !QFileInfo::exists(photoPath)) {
        preview->setPixmap(QPixmap());
        preview->setText(QObject::tr("Photo Placeholder"));
        return;
    }
    QPixmap pix(photoPath);
    if (pix.isNull()) {
        preview->setPixmap(QPixmap());
        preview->setText(QObject::tr("Image invalide"));
        return;
    }
    preview->setText(QString());
    preview->setPixmap(pix.scaled(preview->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
}

static QString persistEquipPhotoFile(int equipId, const QString &sourcePath, QString *error = nullptr)
{
    if (equipId <= 0 || sourcePath.isEmpty()) {
        if (error) *error = QObject::tr("ID équipement ou source image invalide.");
        return QString();
    }
    if (!QFileInfo::exists(sourcePath)) {
        if (error) *error = QObject::tr("Le fichier image n'existe pas.");
        return QString();
    }
    const QString photosDir = QDir(QCoreApplication::applicationDirPath()).filePath(QStringLiteral("equipment_photos"));
    if (!QDir().mkpath(photosDir)) {
        if (error) *error = QObject::tr("Impossible de créer le dossier de stockage des photos.");
        return QString();
    }
    const QFileInfo fi(sourcePath);
    const QString ext = fi.suffix().isEmpty() ? QStringLiteral("png") : fi.suffix().toLower();
    const QString dstPath = QDir(photosDir).filePath(
        QStringLiteral("equip_%1_%2.%3").arg(equipId).arg(QDateTime::currentMSecsSinceEpoch()).arg(ext));
    if (QFile::exists(dstPath))
        QFile::remove(dstPath);
    if (!QFile::copy(sourcePath, dstPath)) {
        if (error) *error = QObject::tr("Impossible de copier l'image dans le projet.");
        return QString();
    }
    return dstPath;
}

static bool validateQuaiInputs(Ui::MainWindow *ui, QString *error)
{
    const QString nom = ui->lineEditNom ? ui->lineEditNom->text().trimmed() : QString();
    const double longueur = ui->doubleSpinBoxLongueur ? ui->doubleSpinBoxLongueur->value() : 0.0;
    const double profondeur = ui->doubleSpinBoxProfondeur ? ui->doubleSpinBoxProfondeur->value() : 0.0;
    const int capacite = ui->spinBoxCapacite ? ui->spinBoxCapacite->value() : 0;
    const int currentUsage = ui->spinBoxCurrentUsage ? ui->spinBoxCurrentUsage->value() : 0;
    const double occupancy = ui->doubleSpinBoxOccupation ? ui->doubleSpinBoxOccupation->value() : 0.0;

    if (nom.isEmpty()) { *error = QObject::tr("Le nom du quai est obligatoire."); return false; }
    if (!isSafeLabelText(nom)) { *error = QObject::tr("Nom du quai invalide (lettres/chiffres/espaces/-/' uniquement)."); return false; }
    if (longueur < 0.0 || profondeur < 0.0 || capacite < 1 || currentUsage < 0 || occupancy < 0.0) {
        *error = QObject::tr("Valeurs invalides : les nombres ne peuvent pas être négatifs et la capacité minimale est 1.");
        return false;
    }
    if (capacite > 0 && currentUsage > capacite) {
        *error = QObject::tr("L'occupation actuelle ne peut pas dépasser la capacité.");
        return false;
    }
    if (occupancy > 100.0) {
        *error = QObject::tr("Le taux d'occupation doit être compris entre 0 et 100.");
        return false;
    }
    return true;
}

/** Normalise les libellés venant de la base vers le combo État (Disponible / Non-disponible / Maintenance). */
static QString mapEquipementEtatForForm(const QString &raw)
{
    const QString s = raw.trimmed();
    const QString l = s.toLower();
    if (l.contains(QStringLiteral("maintenance")) || l.contains(QStringLiteral("atelier")))
        return QStringLiteral("Maintenance");
    if (l.contains(QStringLiteral("non")) && l.contains(QStringLiteral("dispon")))
        return QStringLiteral("Non-disponible");
    if (l.contains(QStringLiteral("en panne")) || l.contains(QStringLiteral("panne"))
        || l.contains(QStringLiteral("indisponible")))
        return QStringLiteral("Non-disponible");
    if (l.contains(QStringLiteral("disponible")) && !l.contains(QStringLiteral("non")))
        return QStringLiteral("Disponible");
    if (l == QStringLiteral("fonctionnel"))
        return QStringLiteral("Disponible");
    return s;
}

/** Colonne DISPONIBILITE (Oui/Non) : dérivée de l'état (plus de saisie séparée). */
static QString equipementDisponibiliteFromEtat(const QString &etat)
{
    if (etat.trimmed().compare(QStringLiteral("Disponible"), Qt::CaseInsensitive) == 0)
        return QStringLiteral("Oui");
    return QStringLiteral("Non");
}

static bool validateEquipementInputs(Ui::MainWindow *ui, QString *error)
{
    const QString nom = ui->lineEdit_2 ? ui->lineEdit_2->text().trimmed() : QString();
    const QString type = ui->lineEdit_3 ? ui->lineEdit_3->text().trimmed() : QString();
    const QString fournisseur = ui->lineEdit_11 ? ui->lineEdit_11->text().trimmed() : QString();
    const QString etat = ui->comboBox ? ui->comboBox->currentText().trimmed() : QString();
    const QString localisation = ui->comboBox_2 ? ui->comboBox_2->currentText().trimmed() : QString();
    const QDate dateAcq = ui->dateEdit ? ui->dateEdit->date() : QDate();
    QString coutStr = ui->lineEdit_4 ? ui->lineEdit_4->text().trimmed() : QString();
    coutStr.replace(QLatin1Char(','), QLatin1Char('.'));

    if (nom.isEmpty()) { *error = QObject::tr("Le nom de l'équipement est obligatoire."); return false; }
    if (!isSafeLabelText(nom)) { *error = QObject::tr("Nom invalide (lettres/chiffres/espaces/-/' uniquement)."); return false; }
    if (!type.isEmpty() && !isLettersWordsOnly(type)) { *error = QObject::tr("Type invalide (lettres uniquement, sans chiffres)."); return false; }
    if (!fournisseur.isEmpty() && !isSafeLabelText(fournisseur)) { *error = QObject::tr("Fournisseur invalide (lettres/chiffres/espaces/-/' uniquement)."); return false; }
    if (etat.isEmpty()) { *error = QObject::tr("L'état de l'équipement est obligatoire."); return false; }
    const QStringList etatsOk = { QStringLiteral("Disponible"), QStringLiteral("Non-disponible"),
                                  QStringLiteral("Maintenance") };
    if (!etatsOk.contains(etat)) {
        *error = QObject::tr("État invalide (choisissez Disponible, Non-disponible ou Maintenance).");
        return false;
    }
    if (localisation.isEmpty()) { *error = QObject::tr("La localisation de l'équipement est obligatoire."); return false; }
    if (!dateAcq.isValid()) { *error = QObject::tr("Date d'acquisition invalide."); return false; }
    if (dateAcq > QDate::currentDate()) { *error = QObject::tr("La date d'acquisition ne peut pas être dans le futur."); return false; }

    bool okCost = false;
    const double cout = coutStr.isEmpty() ? 0.0 : coutStr.toDouble(&okCost);
    if (!coutStr.isEmpty() && !okCost) {
        *error = QObject::tr("Coût estimé invalide.");
        return false;
    }
    if (cout < 0.0) {
        *error = QObject::tr("Le coût estimé ne peut pas être négatif.");
        return false;
    }
    return true;
}

void MainWindow::on_btnAdd_clicked()
{
    QString validationError;
    if (!validateQuaiInputs(ui, &validationError)) {
        QMessageBox::warning(this, tr("Contrôle de saisie"), validationError);
        return;
    }
    QString nom = ui->lineEditNom ? ui->lineEditNom->text().trimmed() : QString();
    Quai q;
    q.setQuaiId(Quai::nextId());
    q.setQuaiNom(nom);
    q.setQuaiType(quaiTypeFromRadios(ui));
    q.setLongueur(ui->doubleSpinBoxLongueur ? ui->doubleSpinBoxLongueur->value() : 0);
    q.setProfondeur(ui->doubleSpinBoxProfondeur ? ui->doubleSpinBoxProfondeur->value() : 0);
    q.setCapacite(ui->spinBoxCapacite ? ui->spinBoxCapacite->value() : 0);
    q.setStatut(quaiStatutFromRadios(ui));
    q.setCurrentUsage(ui->spinBoxCurrentUsage ? ui->spinBoxCurrentUsage->value() : 0);
    q.setOccupancyRate(ui->doubleSpinBoxOccupation ? ui->doubleSpinBoxOccupation->value() : 0);
    q.setPriorityLevel(quaiPrioriteFromRadios(ui));
    q.setSafetyLevel(quaiSafetyFromRadios(ui));
    q.setLightingStatus(quaiLightingFromRadios(ui));
    if (q.ajouter()) {
        loadQuaisTable();
        const QString lang = currentTtsLanguageCode();
        QString msg;
        if (lang == QStringLiteral("ar"))
            msg = tr("تمت إضافة رصيف جديد: %1").arg(nom);
        else if (lang == QStringLiteral("en"))
            msg = tr("A new quay was added: %1").arg(nom);
        else
            msg = tr("Nouveau quai ajouté : %1").arg(nom);
        appendTtsLog(tr("Ajout Quai"), msg);
        speakTtsMessage(msg);
        clearQuaiForm();
        QMessageBox::information(this, tr("Quai"), tr("Quai ajouté."));
    }
    else {
        const QString detail = Quai::lastErrorMessage();
        QMessageBox::warning(this, tr("Erreur"),
            detail.isEmpty() ? tr("Impossible d'ajouter le quai. Vérifiez la base de données.")
                             : tr("Impossible d'ajouter le quai.\n%1").arg(detail));
    }
}

void MainWindow::on_btnUpdate_clicked()
{
    QString idStr = ui->lineEditQuaiID ? ui->lineEditQuaiID->text().trimmed() : QString();
    if (idStr.isEmpty()) { QMessageBox::warning(this, tr("Modifier"), tr("Sélectionnez un quai dans le tableau.")); return; }
    bool okId = false;
    int quaiId = idStr.toInt(&okId);
    if (!okId) {
        QString idNorm = idStr;
        const double asDouble = idNorm.replace(QLatin1Char(','), QLatin1Char('.')).toDouble(&okId);
        if (okId)
            quaiId = qMax(1, static_cast<int>(qRound(asDouble)));
    }
    if (!okId || quaiId <= 0) {
        QMessageBox::warning(this, tr("Modifier"), tr("ID quai invalide."));
        return;
    }
    QString validationError;
    if (!validateQuaiInputs(ui, &validationError)) {
        QMessageBox::warning(this, tr("Contrôle de saisie"), validationError);
        return;
    }
    QString nom = ui->lineEditNom ? ui->lineEditNom->text().trimmed() : QString();
    Quai q;
    q.setQuaiId(quaiId);
    q.setQuaiNom(nom);
    q.setQuaiType(quaiTypeFromRadios(ui));
    q.setLongueur(ui->doubleSpinBoxLongueur ? ui->doubleSpinBoxLongueur->value() : 0);
    q.setProfondeur(ui->doubleSpinBoxProfondeur ? ui->doubleSpinBoxProfondeur->value() : 0);
    q.setCapacite(ui->spinBoxCapacite ? ui->spinBoxCapacite->value() : 0);
    q.setStatut(quaiStatutFromRadios(ui));
    q.setCurrentUsage(ui->spinBoxCurrentUsage ? ui->spinBoxCurrentUsage->value() : 0);
    q.setOccupancyRate(ui->doubleSpinBoxOccupation ? ui->doubleSpinBoxOccupation->value() : 0);
    q.setPriorityLevel(quaiPrioriteFromRadios(ui));
    q.setSafetyLevel(quaiSafetyFromRadios(ui));
    q.setLightingStatus(quaiLightingFromRadios(ui));
    if (q.modifier()) { loadQuaisTable(); QMessageBox::information(this, tr("Quai"), tr("Quai modifié.")); }
    else {
        const QString detail = Quai::lastErrorMessage();
        QMessageBox::warning(this, tr("Erreur"),
            detail.isEmpty() ? tr("Impossible de modifier le quai.")
                             : tr("Impossible de modifier le quai.\n%1").arg(detail));
    }
}

void MainWindow::on_btnDelete_clicked()
{
    QString idStr = ui->lineEditQuaiID ? ui->lineEditQuaiID->text().trimmed() : QString();
    if (idStr.isEmpty()) { QMessageBox::warning(this, tr("Supprimer"), tr("Sélectionnez un quai dans le tableau.")); return; }
    if (QMessageBox::question(this, tr("Supprimer"), tr("Supprimer ce quai ?"), QMessageBox::Yes | QMessageBox::No, QMessageBox::No) != QMessageBox::Yes) return;
    int idToDelete = static_cast<int>(idStr.replace(',', '.').toDouble());
    if (idToDelete <= 0) { QMessageBox::warning(this, tr("Supprimer"), tr("ID quai invalide.")); return; }
    Quai qDel;
    if (qDel.supprimer(idToDelete)) { clearQuaiForm(); loadQuaisTable(); QMessageBox::information(this, tr("Quai"), tr("Quai supprimé.")); }
    else { QMessageBox::warning(this, tr("Erreur"), tr("Impossible de supprimer le quai: %1").arg(Quai::lastErrorMessage().isEmpty() ? tr("Vérifiez la base de données.") : Quai::lastErrorMessage())); }
}

void MainWindow::on_btnClear_clicked() { clearQuaiForm(); }

void MainWindow::on_btnSearch_clicked() { loadQuaisTable(); }

void MainWindow::on_tableQuais_cellClicked(int row, int column)
{
    Q_UNUSED(column);
    fillQuaiFormFromRow(row);
    announceSelectedQuaiFromRow(ui ? ui->tableQuais : nullptr, row);
}

void MainWindow::on_filterComboType_currentIndexChanged(int) { loadQuaisTable(); }
void MainWindow::on_filterComboStatut_currentIndexChanged(int) { loadQuaisTable(); }
void MainWindow::on_filterComboSecurite_currentIndexChanged(int) { loadQuaisTable(); }
void MainWindow::on_comboSmartSort_currentIndexChanged(int) { loadQuaisTable(); }

/** IDs affichés comme "1.0" (Oracle / double) font échouer QString::toInt() → 0. */
static int quaiParseIdFlexible(const QString &s)
{
    const QString t = s.trimmed();
    if (t.isEmpty())
        return 0;
    bool ok = false;
    const int asInt = t.toInt(&ok);
    if (ok && asInt > 0)
        return asInt;
    QString t2 = t;
    const double asDbl = t2.replace(QLatin1Char(','), QLatin1Char('.')).toDouble(&ok);
    if (ok && asDbl > 0.0)
        return qMax(1, static_cast<int>(qRound(asDbl)));
    return 0;
}

static int quaiDbIdForNom(const QString &nom)
{
    const QString n = nom.trimmed();
    if (n.isEmpty() || n == QLatin1String("—"))
        return 0;
    QSqlDatabase db = QSqlDatabase::database(Connection::connectionName());
    if (!db.isOpen())
        return 0;
    const QString colId = Quai::sqlQuaiColQuaiId();
    const QString colNom = Quai::sqlQuaiColQuaiNom();
    if (colId.isEmpty() || colNom.isEmpty())
        return 0;
    QSqlQuery q(db);
    q.prepare(QStringLiteral("SELECT %1 FROM QUAI WHERE UPPER(TRIM(%2)) = UPPER(TRIM(?))").arg(colId, colNom));
    q.addBindValue(n);
    if (!q.exec() || !q.next())
        return 0;
    const QVariant v = q.value(0);
    if (v.isNull())
        return 0;
    return quaiParseIdFlexible(v.toString());
}

static int quaiResolveIdForIncidents(Ui::MainWindow *uiw)
{
    if (!uiw)
        return 0;
    int id = 0;
    const QString idStr = uiw->lineEditQuaiID ? uiw->lineEditQuaiID->text().trimmed() : QString();
    if (!idStr.isEmpty())
        id = quaiParseIdFlexible(idStr);
    if (id <= 0 && uiw->tableSupervisionCombined && uiw->tableSupervisionCombined->rowCount() > 0) {
        const int r = uiw->tableSupervisionCombined->currentRow();
        const int useRow = (r >= 0) ? r : 0;
        if (QTableWidgetItem *it = uiw->tableSupervisionCombined->item(useRow, 1))
            id = quaiDbIdForNom(it->text());
    }
    if (id <= 0 && uiw->tableQuais && uiw->tableQuais->rowCount() > 0) {
        const int r = uiw->tableQuais->currentRow();
        const int useRow = (r >= 0) ? r : 0;
        if (QTableWidgetItem *it = uiw->tableQuais->item(useRow, 0))
            id = quaiParseIdFlexible(it->text());
        if (id <= 0) {
            if (QTableWidgetItem *it = uiw->tableQuais->item(useRow, 1))
                id = quaiDbIdForNom(it->text());
        }
    }
    return id;
}

void MainWindow::on_btnExport_clicked()
{
    QSqlDatabase db = QSqlDatabase::database(Connection::connectionName());
    if (!db.isOpen()) {
        QMessageBox::warning(this, tr("Export"), tr("Base de données non connectée."));
        return;
    }

    const QString defaultName = QStringLiteral("liste_quais_%1.pdf")
                                  .arg(QDate::currentDate().toString(QStringLiteral("yyyy-MM-dd")));
    QString path = QFileDialog::getSaveFileName(this, tr("Exporter les quais en PDF"), defaultName,
                                                  tr("PDF (*.pdf);;Tous (*)"));
    if (path.isEmpty())
        return;
    if (!path.endsWith(QStringLiteral(".pdf"), Qt::CaseInsensitive))
        path += QStringLiteral(".pdf");

    int rowCount = 0;
    if (!Quai::exportListePdf(path, quaiWhereClause(), quaiOrderClause(), &rowCount)) {
        const QString detail = Quai::lastErrorMessage();
        QMessageBox::warning(this, tr("Export"),
                             detail.isEmpty() ? tr("Export PDF impossible.") : detail);
        return;
    }

    QMessageBox::information(
        this, tr("Export"),
        tr("Export PDF terminé (%1 ligne(s)) :\n%2").arg(rowCount).arg(path));
}

void MainWindow::on_btnAddIncident_clicked()
{
    const int id = quaiResolveIdForIncidents(ui);
    if (id <= 0) {
        QMessageBox::information(
            this, tr("Incident"),
            tr("Impossible de déterminer le quai.\n"
               "• Cliquez une ligne du tableau de supervision (colonne Quai), ou\n"
               "• Onglet LISTE : sélectionnez un quai, ou saisissez un ID numérique valide dans le formulaire."));
        return;
    }
    bool ok = false;
    const QString text = QInputDialog::getMultiLineText(this, tr("Nouvel incident"), tr("Description :"), QString(), &ok);
    if (!ok || text.trimmed().isEmpty())
        return;
    if (Quai::sqlQuaiColIncidentLog().isEmpty()) {
        QMessageBox::warning(
            this, tr("Incident"),
            tr("La table QUAI n'a pas de colonne INCIDENT_LOG. Ajoutez-la en base pour enregistrer les incidents."));
        return;
    }
    if (Quai::appendIncidentLog(id, text)) {
        refreshQuaiSupervisionData();
        QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
        if (ui && ui->checkBoxCriticalAlerts && ui->checkBoxCriticalAlerts->isChecked()) {
            QString quaiNom = ui->lineEditNom ? ui->lineEditNom->text().trimmed() : QString();
            QString statut = quaiStatutFromRadios(ui);
            if (quaiNom.isEmpty() && ui->tableQuais && ui->tableQuais->currentRow() >= 0) {
                const int r = ui->tableQuais->currentRow();
                if (QTableWidgetItem *it = ui->tableQuais->item(r, 1)) quaiNom = it->text().trimmed();
                if (QTableWidgetItem *it = ui->tableQuais->item(r, 6)) statut = it->text().trimmed();
            }
            if (statut.isEmpty()) statut = tr("Statut inconnu");
            const QString detail = text.simplified();
            const QString lang = currentTtsLanguageCode();
            QString msg;
            if (lang == QStringLiteral("ar"))
                msg = tr("حادث جديد في الرصيف %1. الحالة %2. التفاصيل: %3")
                          .arg(quaiNom.isEmpty() ? QString::number(id) : quaiNom, statut, detail);
            else if (lang == QStringLiteral("en"))
                msg = tr("New incident on quay %1. Status %2. Details: %3")
                          .arg(quaiNom.isEmpty() ? QString::number(id) : quaiNom, statut, detail);
            else
                msg = tr("Nouvel incident sur le quai %1. Statut %2. Détails : %3")
                          .arg(quaiNom.isEmpty() ? QString::number(id) : quaiNom, statut, detail);
            appendTtsLog(tr("Incident"), msg);
            speakTtsMessage(msg);
        }
        QMessageBox::information(this, tr("Incident"), tr("Incident enregistré en base. Les alertes et le tableau ont été actualisés."));
    } else {
        QMessageBox::warning(this, tr("Incident"),
                             Quai::lastErrorMessage().isEmpty() ? tr("Échec de l'enregistrement.") : Quai::lastErrorMessage());
    }
}

void MainWindow::on_btnRefreshAlertesQuai_clicked()
{
    QSqlDatabase db = QSqlDatabase::database(Connection::connectionName());
    if (!db.isOpen()) {
        QMessageBox::warning(this, tr("Actualiser"), tr("Base de données non connectée."));
        return;
    }
    refreshQuaiStats();
    refreshQuaiCharts();
    refreshQuaiSupervisionData();
    QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
}

void MainWindow::on_btnSupprimerIncidents_clicked()
{
    const int id = quaiResolveIdForIncidents(ui);
    if (id <= 0) {
        QMessageBox::information(
            this, tr("Incidents"),
            tr("Impossible de déterminer le quai.\n"
               "• Sélectionnez une ligne du tableau de supervision (colonne Quai), ou\n"
               "• Onglet LISTE : sélectionnez un quai, ou saisissez l'ID dans le formulaire."));
        return;
    }
    if (Quai::sqlQuaiColIncidentLog().isEmpty()) {
        QMessageBox::warning(
            this, tr("Incidents"),
            tr("La table QUAI n'a pas de colonne INCIDENT_LOG."));
        return;
    }
    if (QMessageBox::question(this, tr("Supprimer les incidents"),
                              tr("Effacer tout le journal d'incidents (INCIDENT_LOG) pour ce quai ?\n"
                                 "Cette action ne supprime pas le quai."),
                              QMessageBox::Yes | QMessageBox::No, QMessageBox::No)
        != QMessageBox::Yes) {
        return;
    }
    if (Quai::clearIncidentLog(id)) {
        refreshQuaiSupervisionData();
        QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
        QMessageBox::information(this, tr("Incidents"), tr("Journal d'incidents vidé pour ce quai."));
    } else {
        QMessageBox::warning(this, tr("Incidents"),
                             Quai::lastErrorMessage().isEmpty() ? tr("Échec de la suppression.") : Quai::lastErrorMessage());
    }
}

void MainWindow::on_btnRetirerAlerteLigne_clicked()
{
    QTableWidget *tw = ui->tableSupervisionCombined;
    if (!tw || tw->rowCount() == 0) {
        QMessageBox::information(this, tr("Alerte"), tr("Aucune ligne dans le tableau."));
        return;
    }
    int r = tw->currentRow();
    if (r < 0)
        r = 0;
    QTableWidgetItem *srcIt = tw->item(r, 0);
    QTableWidgetItem *quaiIt = tw->item(r, 1);
    const QString source = srcIt ? srcIt->text().trimmed() : QString();
    const QString quaiNom = quaiIt ? quaiIt->text().trimmed() : QString();
    if (quaiNom.isEmpty() || quaiNom == QLatin1String("—")) {
        QMessageBox::information(this, tr("Alerte"), tr("Sélectionnez une ligne avec un nom de quai valide."));
        return;
    }
    const int id = quaiDbIdForNom(quaiNom);
    if (id <= 0) {
        QMessageBox::warning(this, tr("Alerte"), tr("Quai « %1 » introuvable en base.").arg(quaiNom));
        return;
    }

    const QString journalLabel = tr("Journal");
    const QString supervisionLabel = tr("Supervision");
    const QString alerteLabel = tr("Alerte");
    const QString systemeLabel = tr("Système");

    if (source == systemeLabel) {
        QMessageBox::information(this, tr("Alerte"), tr("Cette ligne est informative seulement."));
        return;
    }
    if (source == alerteLabel) {
        QMessageBox::information(
            this, tr("Alerte"),
            tr("Pour une alerte d'occupation, modifiez le taux ou l'usage du quai « %1 » dans l'onglet LISTE.")
                .arg(quaiNom));
        return;
    }
    if (source == journalLabel) {
        if (Quai::sqlQuaiColIncidentLog().isEmpty()) {
            QMessageBox::warning(this, tr("Alerte"), tr("Colonne INCIDENT_LOG absente."));
            return;
        }
        if (QMessageBox::question(this, tr("Alerte"),
                                  tr("Effacer tout le journal d'incidents pour « %1 » ?").arg(quaiNom),
                                  QMessageBox::Yes | QMessageBox::No, QMessageBox::No)
            != QMessageBox::Yes)
            return;
        if (Quai::clearIncidentLog(id)) {
            loadQuaisTable();
            QMessageBox::information(this, tr("Alerte"), tr("Journal vidé pour « %1 ».").arg(quaiNom));
        } else {
            QMessageBox::warning(this, tr("Alerte"), Quai::lastErrorMessage());
        }
        return;
    }
    if (source == supervisionLabel) {
        if (QMessageBox::question(
                this, tr("Alerte"),
                tr("Lever la maintenance pour « %1 » ?\n\n"
                   "En base : type « Maintenance » → « Pêche », et un statut contenant « maintenance » → « Actif ».\n"
                   "Vous pourrez corriger type/statut ensuite dans LISTE si besoin.")
                    .arg(quaiNom),
                QMessageBox::Yes | QMessageBox::No, QMessageBox::No)
            != QMessageBox::Yes)
            return;
        if (Quai::endMaintenanceAlert(id)) {
            loadQuaisTable();
            QMessageBox::information(this, tr("Alerte"), tr("Maintenance levée pour « %1 ».").arg(quaiNom));
        } else {
            QMessageBox::warning(this, tr("Alerte"), Quai::lastErrorMessage());
        }
        return;
    }

    QMessageBox::information(
        this, tr("Alerte"),
        tr("Type de ligne non géré. Utilisez « Supprimer incidents » pour vider tout le journal du quai sélectionné."));
}

void MainWindow::on_btnAnalyze_clicked()
{
    if (!ui->tableRecommendations)
        return;
    QSqlDatabase db = QSqlDatabase::database(Connection::connectionName());
    if (!db.isOpen()) {
        QMessageBox::warning(this, tr("Quai"), tr("Base de données non connectée."));
        return;
    }
    const int boatLen = ui->spinBoxLongueurBateau ? ui->spinBoxLongueurBateau->value() : 0;
    const int tonnage = ui->spinBoxTonnage ? ui->spinBoxTonnage->value() : 0;
    const QString boatType = ui->comboBoxTypeBateau ? ui->comboBoxTypeBateau->currentText().trimmed() : QString();

    const QString nomCol = Quai::sqlQuaiColQuaiNom();
    const QString typeCol = Quai::sqlQuaiColQuaiType();
    const QString statCol = Quai::sqlQuaiColStatut();
    const QString lenCol = Quai::sqlQuaiColLongueur();
    const QString capCol = Quai::sqlQuaiColCapacite();
    const QString profCol = Quai::sqlQuaiColProfondeur();
    const QString occCol = Quai::sqlQuaiColOccupancyRate();
    const QString curCol = Quai::sqlQuaiColCurrentUsage();

    if (nomCol.isEmpty()) {
        QMessageBox::warning(this, tr("Analyse"), tr("Schéma QUAI illisible."));
        return;
    }

    const QString lenExpr = lenCol.isEmpty() ? QStringLiteral("0") : QStringLiteral("NVL(%1, 0)").arg(lenCol);
    const QString capExpr = capCol.isEmpty() ? QStringLiteral("0") : QStringLiteral("NVL(%1, 0)").arg(capCol);
    const QString profExpr = profCol.isEmpty() ? QStringLiteral("0") : QStringLiteral("NVL(%1, 0)").arg(profCol);
    const QString occExpr = occCol.isEmpty() ? QStringLiteral("0") : QStringLiteral("NVL(%1, 0)").arg(occCol);
    const QString curExpr = curCol.isEmpty() ? QStringLiteral("0") : QStringLiteral("NVL(%1, 0)").arg(curCol);
    const QString typeSel = typeCol.isEmpty() ? QStringLiteral("CAST(NULL AS VARCHAR2(50))") : typeCol;

    QStringList wparts;
    if (!statCol.isEmpty()) {
        wparts << QStringLiteral("(LOWER(NVL(%1,'')) NOT LIKE '%%maintenance%%' AND LOWER(NVL(%1,'')) NOT LIKE '%%inactif%%')")
                      .arg(statCol);
    }
    if (!typeCol.isEmpty())
        wparts << QStringLiteral("(LOWER(NVL(%1,'')) <> 'maintenance')").arg(typeCol);

    QString sql = QStringLiteral("SELECT %1, %2, %3, %4, %5, %6, %7 FROM QUAI")
                      .arg(nomCol, typeSel, lenExpr, capExpr, profExpr, occExpr, curExpr);
    if (wparts.isEmpty())
        sql += QStringLiteral(" WHERE 1=1");
    else
        sql += QStringLiteral(" WHERE ") + wparts.join(QStringLiteral(" AND "));

    if (boatLen > 0 && !lenCol.isEmpty())
        sql += QStringLiteral(" AND (%1 IS NULL OR %1 >= %2)").arg(lenCol).arg(boatLen);

    sql += QStringLiteral(" ORDER BY %1 ASC, %2 DESC").arg(occExpr, capExpr);

    struct AnlRow {
        QString nom;
        QString type;
        double score = 0;
        QString disp;
        QString reason;
    };
    QList<AnlRow> rows;
    QSqlQuery q(db);
    if (!q.exec(sql)) {
        QMessageBox::warning(this, tr("Analyse"), q.lastError().text());
        return;
    }
    while (q.next()) {
        AnlRow r;
        r.nom = q.value(0).toString();
        r.type = q.value(1).toString();
        const double L = q.value(2).toDouble();
        const double cap = q.value(3).toDouble();
        const double prof = q.value(4).toDouble();
        const double occ = q.value(5).toDouble();
        const int usage = q.value(6).toInt();

        double score = 60.0;
        if (boatLen > 0) {
            if (L >= boatLen)
                score += 20;
            else
                score -= 25;
        } else {
            score += 10;
        }
        if (tonnage > 0 && cap > 0) {
            const double ratio = cap / double(qMax(1, tonnage / 50 + 1));
            score += qBound(0.0, ratio * 5.0, 15.0);
        } else {
            score += 5;
        }
        if (boatType.contains(tr("Chalutier"), Qt::CaseInsensitive) && r.type.contains(tr("Pêche"), Qt::CaseInsensitive))
            score += 10;
        if (boatType.contains(tr("Cargo"), Qt::CaseInsensitive) && r.type.contains(tr("Commerce"), Qt::CaseInsensitive))
            score += 10;
        if (boatType.contains(tr("Tanker"), Qt::CaseInsensitive) && prof >= 8)
            score += 8;
        score -= occ * 0.35;
        if (usage == 0)
            score += 12;
        else
            score -= 5;
        r.score = score;
        r.disp = (usage == 0 && occ < 50) ? tr("Disponible") : tr("Occupé / contrainte");
        r.reason = tr("Longueur, capacité, profondeur, occupation et adéquation de type.");
        rows.append(r);
    }
    std::stable_sort(rows.begin(), rows.end(), [](const AnlRow &a, const AnlRow &b) { return a.score > b.score; });

    ui->tableRecommendations->setRowCount(0);
    int rank = 1;
    for (const AnlRow &r : rows) {
        if (rank > 25)
            break;
        const int row = ui->tableRecommendations->rowCount();
        ui->tableRecommendations->insertRow(row);
        ui->tableRecommendations->setItem(row, 0, new QTableWidgetItem(QString::number(rank)));
        ui->tableRecommendations->setItem(row, 1, new QTableWidgetItem(r.nom));
        ui->tableRecommendations->setItem(row, 2, new QTableWidgetItem(QString::number(qRound(r.score))));
        ui->tableRecommendations->setItem(row, 3, new QTableWidgetItem(r.disp));
        ui->tableRecommendations->setItem(row, 4, new QTableWidgetItem(r.reason));
        ++rank;
    }
    if (rows.isEmpty())
        QMessageBox::information(this, tr("Analyse"), tr("Aucun quai ne correspond aux critères."));
}

// ==================== MODULE EQUIPEMENT ====================
void MainWindow::afficherEquipements()
{
    if (!ui->tableWidget_3)
        return;
    QString search = ui->lineEdit_8 ? ui->lineEdit_8->text().trimmed() : QString();
    QString orderBy = "ID_EQUIPEMENT";

    if (QComboBox *comboSortEq = this->findChild<QComboBox*>(QStringLiteral("comboSortEq"))) {
        switch (comboSortEq->currentIndex()) {
        case 1: orderBy = "NOM"; break;
        case 2: orderBy = "TYPE"; break;
        case 3: orderBy = "ETAT"; break;
        case 4: orderBy = "COUT_ESTIME"; break;
        default: orderBy = "ID_EQUIPEMENT"; break;
        }
    } else if (ui->comboBox_3) {
        // UI fallback: Id / Nom / Disp / Cout
        switch (ui->comboBox_3->currentIndex()) {
        case 1: orderBy = "NOM"; break;
        case 3: orderBy = "COUT_ESTIME"; break;
        default: orderBy = "ID_EQUIPEMENT"; break;
        }
    }

    QSqlQuery query = search.isEmpty() ? Etmp.afficher(orderBy) : Etmp.rechercher(search, orderBy);

    ui->tableWidget_3->setRowCount(0);
    const QStringList headers = { tr("ID"), tr("Nom"), tr("Type"), tr("État"), tr("Date acq."),
        tr("Localisation"), tr("Coût"), tr("Fournisseur"), tr("Disponibilité") };
    if (ui->tableWidget_3->columnCount() != 9)
        ui->tableWidget_3->setColumnCount(9);
    ui->tableWidget_3->setHorizontalHeaderLabels(headers);

    int row = 0;
    while (query.next()) {
        ui->tableWidget_3->insertRow(row);
        QDate d = query.value(4).toDate();
        QString dateStr = d.isValid() ? d.toString(QStringLiteral("dd/MM/yyyy")) : QString();
        ui->tableWidget_3->setItem(row, 0, new QTableWidgetItem(query.value(0).toString()));
        ui->tableWidget_3->setItem(row, 1, new QTableWidgetItem(query.value(1).toString().trimmed()));
        ui->tableWidget_3->setItem(row, 2, new QTableWidgetItem(query.value(2).toString().trimmed()));
        ui->tableWidget_3->setItem(row, 3, new QTableWidgetItem(query.value(3).toString().trimmed()));
        ui->tableWidget_3->setItem(row, 4, new QTableWidgetItem(dateStr));
        ui->tableWidget_3->setItem(row, 5, new QTableWidgetItem(query.value(5).toString().trimmed()));
        ui->tableWidget_3->setItem(row, 6, new QTableWidgetItem(QString::number(query.value(6).toDouble(), 'f', 2)));
        ui->tableWidget_3->setItem(row, 7, new QTableWidgetItem(query.value(7).toString().trimmed()));
        ui->tableWidget_3->setItem(row, 8, new QTableWidgetItem(query.value(8).toString().trimmed()));

        // Color rows by state for quick operator readability.
        const QString etat = query.value(3).toString().trimmed().toLower();
        QColor bg;
        if (etat.contains(QStringLiteral("maintenance")))
            bg = QColor(140, 38, 38, 170);      // red
        else if (etat.contains(QStringLiteral("non")) && etat.contains(QStringLiteral("dispon")))
            bg = QColor(160, 95, 20, 170);      // amber — non-disponible
        else if (etat.contains(QStringLiteral("panne")) || etat.contains(QStringLiteral("indisponible")))
            bg = QColor(160, 95, 20, 170);
        else if (etat.contains(QStringLiteral("disponible")))
            bg = QColor(28, 110, 74, 160);      // green
        const QColor fg = QColor("#f4f7fb");
        if (bg.isValid()) {
            for (int c = 0; c < ui->tableWidget_3->columnCount(); ++c) {
                if (QTableWidgetItem *it = ui->tableWidget_3->item(row, c)) {
                    it->setBackground(QBrush(bg));
                    it->setForeground(QBrush(fg));
                }
            }
            if (QTableWidgetItem *etatItem = ui->tableWidget_3->item(row, 3)) {
                QFont f = etatItem->font();
                f.setBold(true);
                etatItem->setFont(f);
                etatItem->setTextAlignment(Qt::AlignCenter);
            }
        }
        row++;
    }
    if (ui->tableWidget_3->horizontalHeader()) {
        ui->tableWidget_3->horizontalHeader()->setStretchLastSection(false);
        ui->tableWidget_3->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    }
}

void MainWindow::on_tabWidget_currentChanged(int index)
{
    Q_UNUSED(index);
    if (!ui->tabWidget || !ui->tabWidget->currentWidget()) return;
    const QString tabName = ui->tabWidget->currentWidget()->objectName();
    if (tabName == QStringLiteral("tab_statistique")) {
        updateStatistics();
    } else if (tabName == QStringLiteral("tab_maintenance")) {
        updateMaintenanceUI();
    }
}

void MainWindow::updateStatistics()
{
    QWidget *statsTab = this->findChild<QWidget*>(QStringLiteral("tab_statistique"));
    if (!statsTab) return;

    QFrame *typeFrame = ui->container_chart_type;
    QFrame *statusFrame = ui->container_chart_status;

    QMap<QString, int> typeStats = Etmp.getCountByType();
    QMap<QString, int> statusStats = Etmp.getCountByStatus();
    double totalVal = Etmp.getTotalValue();
    int totalCount = Etmp.getTotalCount();

    if (ui->lbl_kpi_total_count) ui->lbl_kpi_total_count->setText(QString::number(totalCount));
    if (ui->lbl_kpi_total_cost) ui->lbl_kpi_total_cost->setText(QString("%1 DT").arg(totalVal, 0, 'f', 2));

    QPieSeries *seriesType = new QPieSeries();
    for (auto it = typeStats.begin(); it != typeStats.end(); ++it) {
        QPieSlice *slice = seriesType->append(it.key(), it.value());
        slice->setLabelVisible(true);
        slice->setLabel(QString("%1: %2").arg(it.key()).arg(it.value()));
    }
    QChart *chartType = new QChart();
    chartType->addSeries(seriesType);
    chartType->setTitle("Repartition par Type d'Equipement");
    chartType->setAnimationOptions(QChart::AllAnimations);
    chartType->legend()->setAlignment(Qt::AlignRight);
    chartType->setBackgroundVisible(false);
    chartType->setTitleBrush(QBrush(Qt::white));
    chartType->legend()->setLabelColor(Qt::white);

    if (m_equipTypeChartView) delete m_equipTypeChartView;
    m_equipTypeChartView = new QChartView(chartType);
    m_equipTypeChartView->setRenderHint(QPainter::Antialiasing);
    if (typeFrame) {
        if (typeFrame->layout()) delete typeFrame->layout();
        QVBoxLayout *layout1 = new QVBoxLayout(typeFrame);
        layout1->addWidget(m_equipTypeChartView);
    }

    QBarSet *set0 = new QBarSet("Quantite");
    QStringList categories;
    for (auto it = statusStats.begin(); it != statusStats.end(); ++it) {
        *set0 << it.value();
        categories << it.key();
    }
    QBarSeries *seriesStatus = new QBarSeries();
    seriesStatus->append(set0);
    QChart *chartStatus = new QChart();
    chartStatus->addSeries(seriesStatus);
    chartStatus->setTitle("Etat Operationnel du Parc");
    chartStatus->setAnimationOptions(QChart::SeriesAnimations);
    chartStatus->setBackgroundVisible(false);
    chartStatus->setTitleBrush(QBrush(Qt::white));
    chartStatus->legend()->setLabelColor(Qt::white);

    QBarCategoryAxis *axisX = new QBarCategoryAxis();
    axisX->append(categories);
    axisX->setLabelsColor(Qt::white);
    chartStatus->addAxis(axisX, Qt::AlignBottom);
    seriesStatus->attachAxis(axisX);

    QValueAxis *axisY = new QValueAxis();
    axisY->setLabelsColor(Qt::white);
    chartStatus->addAxis(axisY, Qt::AlignLeft);
    seriesStatus->attachAxis(axisY);

    if (m_equipStatusChartView) delete m_equipStatusChartView;
    m_equipStatusChartView = new QChartView(chartStatus);
    m_equipStatusChartView->setRenderHint(QPainter::Antialiasing);
    if (statusFrame) {
        if (statusFrame->layout()) delete statusFrame->layout();
        QVBoxLayout *layout2 = new QVBoxLayout(statusFrame);
        layout2->addWidget(m_equipStatusChartView);
    }
}

void MainWindow::updateMaintenanceUI()
{
    if (!ui->tab_maintenance) return;

    QTableWidget *table = ui->tableMaintenance;
    QLabel *lblAlerts = ui->lbl_ma_alert_count;
    QLabel *lblOngoing = ui->lbl_ma_ongoing_count;

    if (!table || !lblAlerts || !lblOngoing) return;

    table->setColumnCount(7);
    table->setHorizontalHeaderLabels({"ID", "Nom", "Type", "Etat", "Acquisition", "Zone", "Alerte"});
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    table->verticalHeader()->setVisible(false);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);

    QSqlQuery query = Etmp.getMaintenanceAlerts();
    table->setRowCount(0);
    int alerts = 0;
    int ongoing = 0;
    while (query.next()) {
        int row = table->rowCount();
        table->insertRow(row);
        QString etat = query.value(3).toString();
        QDate acq = query.value(4).toDate();
        table->setItem(row, 0, new QTableWidgetItem(query.value(0).toString()));
        table->setItem(row, 1, new QTableWidgetItem(query.value(1).toString()));
        table->setItem(row, 2, new QTableWidgetItem(query.value(2).toString()));
        table->setItem(row, 3, new QTableWidgetItem(etat));
        table->setItem(row, 4, new QTableWidgetItem(acq.toString("dd/MM/yyyy")));
        table->setItem(row, 5, new QTableWidgetItem(query.value(5).toString()));

        QString alertStatus = "OK";
        if (etat.toLower().contains("maintenance")) {
            alertStatus = "EN REPARATION";
            ongoing++;
        } else if (etat.toLower().contains("mauvais") || (acq.isValid() && acq.daysTo(QDate::currentDate()) > 365)) {
            alertStatus = "ALERTE ACTIVE";
            alerts++;
        }
        table->setItem(row, 6, new QTableWidgetItem(alertStatus));
    }
    if (lblAlerts) lblAlerts->setText(QString::number(alerts));
    if (lblOngoing) lblOngoing->setText(QString::number(ongoing));
}

void MainWindow::on_btnMaRepair_clicked()
{
    QTableWidget *table = ui->tableMaintenance;
    if (!table) return;
    int row = table->currentRow();
    if (row < 0 || !table->item(row, 0)) return;
    int id = table->item(row, 0)->text().toInt();
    if (Etmp.updateStatus(id, QStringLiteral("Disponible"))) {
        updateMaintenanceUI();
        updateStatistics();
        afficherEquipements();
    }
}

void MainWindow::on_btnMaWorkshop_clicked()
{
    QTableWidget *table = ui->tableMaintenance;
    if (!table) return;
    int row = table->currentRow();
    if (row < 0 || !table->item(row, 0)) return;
    int id = table->item(row, 0)->text().toInt();
    if (Etmp.updateStatus(id, QStringLiteral("Maintenance"))) {
        updateMaintenanceUI();
        updateStatistics();
        afficherEquipements();
    }
}

void MainWindow::on_btnRefreshMaintenance_clicked()
{
    updateMaintenanceUI();
}

void MainWindow::on_comboSortEq_currentIndexChanged(int)
{
    afficherEquipements();
}

void MainWindow::on_btnGenerateQR_clicked()
{
    if (!ui->tableWidget_3 || ui->tableWidget_3->currentRow() < 0) {
        QMessageBox::warning(this, tr("Generer QR"), tr("Veuillez sélectionner un équipement dans le tableau."));
        return;
    }

    int row = ui->tableWidget_3->currentRow();
    int id = ui->tableWidget_3->item(row, 0)->text().toInt();
    if (id <= 0) return;

    QString url = m_equipServer->equipUrl(id);
    const QString appDir = QCoreApplication::applicationDirPath();
    const QString qrPath = QDir(appDir).filePath(QString("qr_%1.png").arg(id));
    const QString pyScript = ensureQrGeneratorScriptPath();

    QProcess gen;
    gen.setProcessEnvironment(sanitizedPythonEnvironment());
    QStringList args;
    args << pyScript << url << qrPath;

    const QString pythonExe = findPythonExecutable();
    if (pythonExe.isEmpty()) {
        QMessageBox::warning(this, tr("Erreur"), tr("Python introuvable (requis pour générer le QR Code)."));
        return;
    }

    gen.start(pythonExe, args);
    if (gen.waitForFinished(7000) && gen.exitCode() == 0) {
        QMessageBox::information(this, tr("Succès"), tr("QR Code généré vers : %1\nIl pointe vers : %2").arg(qrPath).arg(url));
        QDesktopServices::openUrl(QUrl::fromLocalFile(qrPath));
    } else {
        QMessageBox::warning(this, tr("Erreur"), tr("Échec de la génération du QR Code."));
    }
}



void MainWindow::clearEquipementForm()
{
    if (ui->lineEdit) ui->lineEdit->clear();
    if (ui->lineEdit_2) ui->lineEdit_2->clear();
    if (ui->lineEdit_3) ui->lineEdit_3->clear();
    if (ui->comboBox) ui->comboBox->setCurrentIndex(0);
    if (ui->dateEdit) ui->dateEdit->setDate(QDate::currentDate());
    if (ui->comboBox_2) ui->comboBox_2->setCurrentIndex(0);
    if (ui->lineEdit_4) ui->lineEdit_4->clear();
    if (ui->lineEdit_11) ui->lineEdit_11->clear();
    m_equipPendingPhotoPath.clear();
    showEquipPhotoPreview(m_equipPhotoPreview, QString());
}

void MainWindow::on_pushButton_2_clicked()
{
    QString validationError;
    if (!validateEquipementInputs(ui, &validationError)) {
        QMessageBox::warning(this, tr("Contrôle de saisie"), validationError);
        return;
    }
    QString nom = ui->lineEdit_2 ? ui->lineEdit_2->text().trimmed() : QString();
    QString type = ui->lineEdit_3 ? ui->lineEdit_3->text().trimmed() : QString();
    QString etat = ui->comboBox ? ui->comboBox->currentText().trimmed() : QStringLiteral("Disponible");
    QDate dateAcq = ui->dateEdit ? ui->dateEdit->date() : QDate();
    QString localisation = ui->comboBox_2 ? ui->comboBox_2->currentText() : QString();
    double cout = ui->lineEdit_4 ? ui->lineEdit_4->text().trimmed().replace(',', '.').toDouble() : 0;
    QString fournisseur = ui->lineEdit_11 ? ui->lineEdit_11->text().trimmed() : QString();
    const QString disp = equipementDisponibiliteFromEtat(etat);

    const int newEquipId = nextEquipementId();
    if (newEquipId <= 0) {
        QMessageBox::critical(this, tr("Erreur"), tr("Impossible de générer un ID équipement."));
        return;
    }
    bool ok = Etmp.ajouter(newEquipId, nom, type, etat, dateAcq, localisation, cout, fournisseur, disp, 0);
    if (ok) {
        if (!m_equipPendingPhotoPath.isEmpty()) {
            QString ioErr;
            const QString finalPhotoPath = persistEquipPhotoFile(newEquipId, m_equipPendingPhotoPath, &ioErr);
            if (!finalPhotoPath.isEmpty()) {
                QString dbErr;
                if (!saveEquipPhotoPath(newEquipId, finalPhotoPath, &dbErr)) {
                    QMessageBox::warning(this, tr("Photo équipement"),
                                         dbErr.isEmpty() ? tr("Équipement ajouté, mais la photo n'a pas été liée.") : dbErr);
                } else {
                    showEquipPhotoPreview(m_equipPhotoPreview, finalPhotoPath);
                }
            } else {
                QMessageBox::warning(this, tr("Photo équipement"),
                                     ioErr.isEmpty() ? tr("Équipement ajouté, mais la photo n'a pas été copiée.") : ioErr);
            }
        }
        QMessageBox::information(this, tr("Succès"), tr("Équipement ajouté avec succès."));
        afficherEquipements();
        clearEquipementForm();
    } else {
        QMessageBox::critical(this, tr("Erreur"), Etmp.lastError().isEmpty() ? tr("Échec ajout.") : Etmp.lastError());
    }
}

void MainWindow::on_pushButton_4_clicked()
{
    int id = ui->lineEdit ? ui->lineEdit->text().trimmed().toInt() : 0;
    if (id <= 0) {
        QMessageBox::warning(this, tr("Sélection requise"), tr("Veuillez sélectionner un équipement dans le tableau (ou renseigner l'ID) pour modifier."));
        return;
    }
    QString validationError;
    if (!validateEquipementInputs(ui, &validationError)) {
        QMessageBox::warning(this, tr("Contrôle de saisie"), validationError);
        return;
    }
    QString nom = ui->lineEdit_2 ? ui->lineEdit_2->text().trimmed() : QString();
    QString type = ui->lineEdit_3 ? ui->lineEdit_3->text().trimmed() : QString();
    QString etat = ui->comboBox ? ui->comboBox->currentText().trimmed() : QStringLiteral("Disponible");
    QDate dateAcq = ui->dateEdit ? ui->dateEdit->date() : QDate();
    QString localisation = ui->comboBox_2 ? ui->comboBox_2->currentText() : QString();
    double cout = ui->lineEdit_4 ? ui->lineEdit_4->text().trimmed().replace(',', '.').toDouble() : 0;
    QString fournisseur = ui->lineEdit_11 ? ui->lineEdit_11->text().trimmed() : QString();
    const QString disp = equipementDisponibiliteFromEtat(etat);

    bool ok = Etmp.modifier(id, nom, type, etat, dateAcq, localisation, cout, fournisseur, disp, 0);
    if (ok) {
        if (!m_equipPendingPhotoPath.isEmpty()) {
            QString ioErr;
            const QString finalPhotoPath = persistEquipPhotoFile(id, m_equipPendingPhotoPath, &ioErr);
            if (!finalPhotoPath.isEmpty()) {
                QString dbErr;
                if (!saveEquipPhotoPath(id, finalPhotoPath, &dbErr)) {
                    QMessageBox::warning(this, tr("Photo équipement"),
                                         dbErr.isEmpty() ? tr("Équipement modifié, mais la photo n'a pas été liée.") : dbErr);
                } else {
                    showEquipPhotoPreview(m_equipPhotoPreview, finalPhotoPath);
                }
            } else {
                QMessageBox::warning(this, tr("Photo équipement"),
                                     ioErr.isEmpty() ? tr("Équipement modifié, mais la photo n'a pas été copiée.") : ioErr);
            }
        }
        QMessageBox::information(this, tr("Succès"), tr("Équipement modifié avec succès."));
        afficherEquipements();
        clearEquipementForm();
    } else {
        QMessageBox::critical(this, tr("Erreur"), Etmp.lastError().isEmpty() ? tr("Échec modification.") : Etmp.lastError());
    }
}

void MainWindow::on_pushButton_5_clicked()
{
    int id = ui->lineEdit ? ui->lineEdit->text().trimmed().toInt() : 0;
    if (id <= 0) {
        QMessageBox::warning(this, tr("Sélection requise"), tr("Veuillez sélectionner un équipement dans le tableau pour supprimer."));
        return;
    }
    QMessageBox::StandardButton r = QMessageBox::question(this, tr("Confirmer"), tr("Supprimer cet équipement ?"), QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
    if (r != QMessageBox::Yes) return;
    if (Etmp.supprimer(id)) {
        QMessageBox::information(this, tr("Succès"), tr("Équipement supprimé avec succès."));
        afficherEquipements();
        clearEquipementForm();
    } else {
        QMessageBox::critical(this, tr("Erreur"), Etmp.lastError().isEmpty() ? tr("Échec suppression.") : Etmp.lastError());
    }
}

void MainWindow::on_pushButton_7_clicked()
{
    QString fileName = QFileDialog::getSaveFileName(this, tr("Exporter en PDF"), QString(),
                                                    tr("PDF (*.pdf)"));
    if (fileName.isEmpty())
        return;
    if (!fileName.endsWith(QStringLiteral(".pdf"), Qt::CaseInsensitive))
        fileName += QStringLiteral(".pdf");

    const QString search = ui->lineEdit_8 ? ui->lineEdit_8->text().trimmed() : QString();
    QString orderBy = QStringLiteral("ID_EQUIPEMENT");
    if (QComboBox *comboSortEq = this->findChild<QComboBox *>(QStringLiteral("comboSortEq"))) {
        switch (comboSortEq->currentIndex()) {
        case 1: orderBy = QStringLiteral("NOM"); break;
        case 2: orderBy = QStringLiteral("TYPE"); break;
        case 3: orderBy = QStringLiteral("ETAT"); break;
        case 4: orderBy = QStringLiteral("COUT_ESTIME"); break;
        default: orderBy = QStringLiteral("ID_EQUIPEMENT"); break;
        }
    } else if (ui->comboBox_3) {
        switch (ui->comboBox_3->currentIndex()) {
        case 1: orderBy = QStringLiteral("NOM"); break;
        case 3: orderBy = QStringLiteral("COUT_ESTIME"); break;
        default: orderBy = QStringLiteral("ID_EQUIPEMENT"); break;
        }
    }

    int rowCount = 0;
    if (!Etmp.exportListePdf(fileName, search, orderBy, &rowCount)) {
        QMessageBox::warning(this, tr("Export PDF"),
                             Etmp.lastError().isEmpty() ? tr("Échec de l'export PDF.") : Etmp.lastError());
        return;
    }
    QMessageBox::information(this, tr("Export PDF"),
                             tr("Liste exportée avec succès (%1 ligne(s)).").arg(rowCount));
}

void MainWindow::on_lineEdit_8_textChanged(const QString &)
{
    afficherEquipements();
}

void MainWindow::on_tableWidget_3_cellClicked(int row, int column)
{
    Q_UNUSED(column);
    if (!ui->tableWidget_3 || row < 0) return;
    ui->lineEdit->setText(ui->tableWidget_3->item(row, 0) ? ui->tableWidget_3->item(row, 0)->text() : QString());
    ui->lineEdit_2->setText(ui->tableWidget_3->item(row, 1) ? ui->tableWidget_3->item(row, 1)->text() : QString());
    ui->lineEdit_3->setText(ui->tableWidget_3->item(row, 2) ? ui->tableWidget_3->item(row, 2)->text() : QString());
    const QString etatRaw = ui->tableWidget_3->item(row, 3) ? ui->tableWidget_3->item(row, 3)->text() : QString();
    const QString etatNorm = mapEquipementEtatForForm(etatRaw);
    if (ui->comboBox) {
        int etatIdx = ui->comboBox->findText(etatNorm, Qt::MatchExactly);
        if (etatIdx < 0)
            etatIdx = ui->comboBox->findText(etatRaw.trimmed(), Qt::MatchExactly);
        if (etatIdx >= 0)
            ui->comboBox->setCurrentIndex(etatIdx);
    }
    QString dateStr = ui->tableWidget_3->item(row, 4) ? ui->tableWidget_3->item(row, 4)->text() : QString();
    QDate d = QDate::fromString(dateStr, QStringLiteral("dd/MM/yyyy"));
    if (d.isValid()) ui->dateEdit->setDate(d);
    QString loc = ui->tableWidget_3->item(row, 5) ? ui->tableWidget_3->item(row, 5)->text() : QString();
    int locIdx = ui->comboBox_2->findText(loc, Qt::MatchExactly);
    if (locIdx >= 0) ui->comboBox_2->setCurrentIndex(locIdx);
    ui->lineEdit_4->setText(ui->tableWidget_3->item(row, 6) ? ui->tableWidget_3->item(row, 6)->text() : QString());
    ui->lineEdit_11->setText(ui->tableWidget_3->item(row, 7) ? ui->tableWidget_3->item(row, 7)->text() : QString());
    const int equipId = ui->lineEdit ? ui->lineEdit->text().trimmed().toInt() : 0;
    m_equipPendingPhotoPath = loadEquipPhotoPath(equipId);
    showEquipPhotoPreview(m_equipPhotoPreview, m_equipPendingPhotoPath);
}

void MainWindow::on_comboBox_3_currentIndexChanged(int index)
{
    if (!ui->tableWidget_3 || ui->tableWidget_3->columnCount() < 9) return;
    int col = 0;
    switch (index) {
    case 1: col = 1; break;  // Nom
    case 2: col = 8; break;   // Disp
    case 3: col = 6; break;  // Cout
    default: col = 0; break;  // Id
    }
    ui->tableWidget_3->sortItems(col, Qt::AscendingOrder);
}

// === MODULE PECHEUR (suite : Mission IA) ===
void MainWindow::on_btn_analyze_mission_2_clicked()
{
    struct PecheurInfo {
        QString id;
        QString nomprenom;
        QString role;
        int experience;
        QString statut;
    };

    QString missionType = ui->cb_mission_type_2->currentText();
    int duration = ui->sb_duration_2->value();
    QString difficulty = ui->cb_difficulty_2->currentText();

    // Rôles requis par type de mission
    QList<QString> requiredRoles;
    if (missionType == QStringLiteral("Pêche Côtière")) {
        requiredRoles << "Capitaine" << "Matelot" << "Matelot";
    } else if (missionType == QStringLiteral("Haute Mer (Deep Sea)")) {
        requiredRoles << "Capitaine" << "Matelot" << "Matelot" << "Matelot";
    } else if (missionType == QStringLiteral("Maintenance Navale")) {
        requiredRoles << "Matelot" << "Matelot";
    } else if (missionType == QStringLiteral("Sauvetage / Urgence")) {
        requiredRoles << "Capitaine" << "Matelot" << "Matelot";
    } else {
        requiredRoles << "Capitaine" << "Matelot";
    }

    // DB check: mission analysis is SQL-driven, so we validate connection first.
    QSqlDatabase db = QSqlDatabase::database(Connection::connectionName());
    if (!db.isValid() || !db.isOpen()) {
        ui->lbl_ai_status_2->setText(QStringLiteral("Connexion base indisponible pour l'analyse mission."));
        ui->table_mission_results_2->setRowCount(0);
        return;
    }

    // SQL command #1: fast existence check to keep previous user message behavior.
    QSqlQuery qCount(db);
    if (!qCount.exec(QStringLiteral("SELECT COUNT(*) FROM PECHEUR")) || !qCount.next() || qCount.value(0).toInt() <= 0) {
        ui->lbl_ai_status_2->setText(QStringLiteral("Aucun pêcheur en base. Ajoutez des pêcheurs avant d'analyser une mission."));
        ui->table_mission_results_2->setRowCount(0);
        return;
    }

    auto reasonText = [](const PecheurInfo &p, const QString &targetRole, int /* score */) -> QString {
        QStringList parts;
        if (p.role.compare(targetRole, Qt::CaseInsensitive) == 0)
            parts << QStringLiteral("Rôle correspondant.");
        else
            parts << QStringLiteral("Rôle proche (%1).").arg(p.role);
        parts << QStringLiteral("Expérience: %1 ans.").arg(p.experience);
        if (p.statut.compare(QStringLiteral("Actif"), Qt::CaseInsensitive) == 0)
            parts << QStringLiteral("Statut actif.");
        else
            parts << QStringLiteral("Statut inactif (à confirmer).");
        return parts.join(QStringLiteral(" "));
    };

    QSet<QString> usedIds;
    struct ResultRow { QString role; QString candidat; int score; QString raisonnement; };
    QList<ResultRow> results;

    for (const QString &targetRole : requiredRoles) {
        // SQL command #2: dynamic mission scoring query for one required role.
        // Same scoring formula as before, now computed in DB with ORDER BY score.
        const QString scoreExpr = QStringLiteral(
            "LEAST((CASE "
            "WHEN UPPER(TRIM(NVL(ROLE, ''))) = UPPER(:target_exact) THEN 40 "
            "WHEN INSTR(UPPER(TRIM(NVL(ROLE, ''))), UPPER(:target_like)) > 0 "
            "  OR INSTR(UPPER(:target_reverse), UPPER(TRIM(NVL(ROLE, '')))) > 0 THEN 28 "
            "ELSE 15 END) "
            "+ LEAST(NVL(ANNEES_EXPERIENCE, 0) * 2, 40) "
            "+ CASE WHEN UPPER(TRIM(NVL(STATUT, ''))) = 'ACTIF' THEN 20 ELSE 0 END, 100)");

        QString sql = QStringLiteral(
            "SELECT ID_PECHEUR, NOMPRENOM, ROLE, ANNEES_EXPERIENCE, STATUT, %1 AS SCORE "
            "FROM PECHEUR").arg(scoreExpr);

        QStringList excludeBindNames;
        if (!usedIds.isEmpty()) {
            QStringList placeholders;
            int idx = 0;
            for (const QString &id : usedIds) {
                const QString bindName = QStringLiteral(":ex%1").arg(idx++);
                placeholders << bindName;
                excludeBindNames << id;
            }
            sql += QStringLiteral(" WHERE ID_PECHEUR NOT IN (") + placeholders.join(QStringLiteral(",")) + QStringLiteral(")");
        }

        sql += QStringLiteral(" ORDER BY SCORE DESC, ANNEES_EXPERIENCE DESC, ID_PECHEUR ASC");

        // Oracle-safe top-1 selection.
        QSqlQuery qBest(db);
        const QString wrappedSql = QStringLiteral("SELECT * FROM (") + sql + QStringLiteral(") WHERE ROWNUM = 1");
        qBest.prepare(wrappedSql);
        qBest.bindValue(QStringLiteral(":target_exact"), targetRole);
        qBest.bindValue(QStringLiteral(":target_like"), targetRole);
        qBest.bindValue(QStringLiteral(":target_reverse"), targetRole);

        for (int i = 0; i < excludeBindNames.size(); ++i)
            qBest.bindValue(QStringLiteral(":ex%1").arg(i), excludeBindNames.at(i));

        if (qBest.exec() && qBest.next()) {
            PecheurInfo p;
            p.id = qBest.value(0).toString();
            p.nomprenom = qBest.value(1).toString().trimmed();
            p.role = qBest.value(2).toString().trimmed();
            p.experience = qBest.value(3).toInt();
            p.statut = qBest.value(4).toString().trimmed();
            const int bestScore = qBest.value(5).toInt();
            usedIds.insert(p.id);
            results.append({ targetRole, p.nomprenom, bestScore, reasonText(p, targetRole, bestScore) });
        } else {
            results.append({ targetRole, QStringLiteral("— Aucun candidat —"), 0, QStringLiteral("Aucun pêcheur disponible pour ce rôle.") });
        }
    }

    ui->table_mission_results_2->setRowCount(results.size());
    const QBrush textBrush(QColor(255, 255, 255));
    const QBrush scoreBrush(QColor(20, 255, 236)); // #14ffec - accent cyan for score
    QFont scoreFont(QStringLiteral("Segoe UI"), 12, QFont::Bold);
    for (int r = 0; r < results.size(); ++r) {
        auto *item0 = new QTableWidgetItem(results[r].role);
        auto *item1 = new QTableWidgetItem(results[r].candidat);
        auto *item2 = new QTableWidgetItem(QString::number(results[r].score) + QStringLiteral(" %"));
        auto *item3 = new QTableWidgetItem(results[r].raisonnement);
        item0->setForeground(textBrush);
        item1->setForeground(textBrush);
        item2->setForeground(scoreBrush);
        item2->setFont(scoreFont);
        item3->setForeground(textBrush);
        item1->setFont(QFont(QStringLiteral("Segoe UI"), 11, QFont::DemiBold));
        ui->table_mission_results_2->setItem(r, 0, item0);
        ui->table_mission_results_2->setItem(r, 1, item1);
        ui->table_mission_results_2->setItem(r, 2, item2);
        ui->table_mission_results_2->setItem(r, 3, item3);
    }
    ui->table_mission_results_2->setColumnWidth(1, 220);
    ui->table_mission_results_2->setColumnWidth(2, 120);
    ui->lbl_ai_status_2->setText(QStringLiteral("Analyse terminée. Mission: %1 | Durée: %2 jours | Difficulté: %3 — %4 recommandation(s).")
        .arg(missionType).arg(duration).arg(difficulty).arg(results.size()));
}

// ==================== EMPLOYEE CRUD ====================
class EmployeeSortItem final : public QTableWidgetItem {
public:
    explicit EmployeeSortItem(const QString &displayText, const QVariant &sortKey)
        : QTableWidgetItem(displayText) { setData(Qt::UserRole, sortKey); }
    bool operator<(const QTableWidgetItem &other) const override {
        const QVariant a = data(Qt::UserRole), b = other.data(Qt::UserRole);
        if (a.isValid() && b.isValid() && a.metaType() == b.metaType()) {
            if (a.metaType().id() == QMetaType::Int) return a.toInt() < b.toInt();
            if (a.metaType().id() == QMetaType::QDate) return a.toDate() < b.toDate();
            if (a.metaType().id() == QMetaType::QString) return a.toString().localeAwareCompare(b.toString()) < 0;
        }
        return QTableWidgetItem::operator<(other);
    }
};

static QString empDisplayDate(const QDate &d) { return d.isValid() ? d.toString("dd/MM/yyyy") : QString(); }

bool MainWindow::ensureEmployeeTable() {
    QString err;
    if (!EmployeeCrud::ensureTable(m_db, &err))
        return false;
    if (m_db.driverName().contains("QODBC", Qt::CaseInsensitive) && !hasAffectDotColumns(m_db)) {
        const QList<QPair<QString,QString>> alterPairs = {
            {"ALTER TABLE employe ADD affect_dot_date DATE", "ALTER TABLE employe ADD affect_dot_dept VARCHAR2(100)"},
            {"ALTER TABLE employe ADD affect_dot_date DATETIME", "ALTER TABLE employe ADD affect_dot_dept VARCHAR(100)"}
        };
        for (const auto &p : alterPairs) {
            for (const QString &sql : {p.first, p.second}) {
                QSqlQuery q(m_db);
                if (!q.exec(sql)) {
                    const QString err = q.lastError().text();
                    if (!err.contains("already exists", Qt::CaseInsensitive) && !err.contains("ORA-01430", Qt::CaseInsensitive)
                        && !err.contains("duplicate column", Qt::CaseInsensitive)) {
                        ;  // real error - ignore and try next pair
                    }
                }
            }
            if (hasAffectDotColumns(m_db)) break;
        }
    }
    return true;
}

QString MainWindow::currentEmployeeCin() const {
    return ui->lineEdit_id ? ui->lineEdit_id->text().trimmed() : QString();
}

void MainWindow::setCurrentEmployeeCin(const QString &cin) {
    if (ui->lineEdit_id) ui->lineEdit_id->setText(cin.trimmed());
}

QString MainWindow::selectedEmployeeStatus() const {
    if (ui->radio_status_actif && ui->radio_status_actif->isChecked()) return "Actif";
    if (ui->radio_status_inactif && ui->radio_status_inactif->isChecked()) return "Inactif";
    if (ui->radio_status_conge && ui->radio_status_conge->isChecked()) return "En congé";
    return "Actif";
}

void MainWindow::setEmployeeStatus(const QString &status) {
    const QString s = status.trimmed().toLower();
    if (ui->radio_status_actif) ui->radio_status_actif->setChecked(s.contains("actif") && !s.contains("inactif"));
    if (ui->radio_status_inactif) ui->radio_status_inactif->setChecked(s.contains("inactif"));
    if (ui->radio_status_conge) ui->radio_status_conge->setChecked(s.contains("cong"));
    if (ui->radio_status_actif && !ui->radio_status_actif->isChecked()
        && ui->radio_status_inactif && !ui->radio_status_inactif->isChecked()
        && ui->radio_status_conge && !ui->radio_status_conge->isChecked())
        ui->radio_status_actif->setChecked(true);
}

QString MainWindow::selectedEmployeeShift() const {
    if (ui->radio_shift_matin && ui->radio_shift_matin->isChecked()) return "Matin";
    if (ui->radio_shift_nuit && ui->radio_shift_nuit->isChecked()) return "Nuit";
    if (ui->radio_shift_rotation && ui->radio_shift_rotation->isChecked()) return "Rotation";
    return "Matin";
}

void MainWindow::setEmployeeShift(const QString &shift) {
    const QString s = shift.trimmed().toLower();
    if (ui->radio_shift_matin) ui->radio_shift_matin->setChecked(s.contains("matin"));
    if (ui->radio_shift_nuit) ui->radio_shift_nuit->setChecked(s.contains("nuit"));
    if (ui->radio_shift_rotation) ui->radio_shift_rotation->setChecked(s.contains("rot"));
    if (ui->radio_shift_matin && !ui->radio_shift_matin->isChecked()
        && ui->radio_shift_nuit && !ui->radio_shift_nuit->isChecked()
        && ui->radio_shift_rotation && !ui->radio_shift_rotation->isChecked())
        ui->radio_shift_matin->setChecked(true);
}

static void setInvalid(QWidget *w, bool invalid) {
    if (!w) return;
    w->setProperty("invalid", invalid);
    w->style()->unpolish(w);
    w->style()->polish(w);
    w->update();
}

bool MainWindow::validateEmployeeForm(QString *errorMessage, bool markFields) {
    const QString cin = ui->lineEdit_cin ? ui->lineEdit_cin->text().trimmed() : QString();
    const QString prenom = ui->lineEdit_fname ? ui->lineEdit_fname->text().trimmed() : QString();
    const QString nom = ui->lineEdit_lname ? ui->lineEdit_lname->text().trimmed() : QString();
    const QString occupation = ui->combo_job ? ui->combo_job->currentText().trimmed() : QString();
    const QString dept = ui->combo_dept ? ui->combo_dept->currentText().trimmed() : QString();
    const QString ville = ui->combo_city ? ui->combo_city->currentText().trimmed() : QString();
    const QString certs = ui->combo_certs ? ui->combo_certs->currentText().trimmed() : QString();
    const QString phone = ui->lineEdit_phone ? ui->lineEdit_phone->text().trimmed() : QString();
    const QString emailRaw = ui->lineEdit_email ? ui->lineEdit_email->text().trimmed() : QString();
    const QString email = EmployeePassword::normalizeEmailInput(emailRaw);
    const QString pwd = ui->lineEdit_pwd ? ui->lineEdit_pwd->text() : QString();
    const QString lic = ui->lineEdit_lic ? ui->lineEdit_lic->text().trimmed() : QString();
    const QDate hireDate = ui->dateEdit_hire ? ui->dateEdit_hire->date() : QDate();
    const QDate expDate = ui->dateEdit_exp ? ui->dateEdit_exp->date() : QDate();
    const QString currentCin = currentEmployeeCin();
    const bool isUpdate = !currentCin.isEmpty();

    QStringList errors;
    const bool cinOk = (cin.size() == 8) && QRegularExpression("^\\d{8}$").match(cin).hasMatch();
    const bool prenomOk = !prenom.isEmpty();
    const bool nomOk = !nom.isEmpty();
    const bool occupationOk = !occupation.isEmpty();
    const bool deptOk = !dept.isEmpty();
    const bool phoneOk = phone.isEmpty() || QRegularExpression("^\\+?\\d{8,15}$").match(phone).hasMatch();
    const bool emailOk = email.isEmpty() || EmployeePassword::isEmailFormatValid(email);
    const bool pwdOk = pwd.size() >= 6;
    const bool villeOk = !ville.isEmpty();
    const bool hireOk = !hireDate.isValid() || hireDate <= QDate::currentDate();
    const bool expOk = !expDate.isValid() || expDate >= QDate::currentDate();
    const bool licOk = certs.isEmpty() ? true : !lic.isEmpty();

    bool cinUniqueOk = true;
    if (m_db.isValid() && m_db.isOpen() && cinOk) {
        bool exists = false;
        EmployeeCrud::cinExists(m_db, cin, isUpdate ? currentCin : QString(), &exists, nullptr);
        cinUniqueOk = !exists;
    }

    if (!cinOk) errors << "CIN: doit contenir exactement 8 chiffres.";
    if (!cinUniqueOk) errors << "CIN: déjà existant.";
    if (!prenomOk) errors << "Prenom: obligatoire.";
    if (!nomOk) errors << "Nom: obligatoire.";
    if (!occupationOk) errors << "Occupation: obligatoire.";
    if (!deptOk) errors << "Departement: obligatoire.";
    if (!phoneOk) errors << "Telephone: 8 à 15 chiffres.";
    if (!emailOk) errors << "Email: format invalide.";
    if (emailOk && !emailRaw.isEmpty() && ui->lineEdit_email && ui->lineEdit_email->text() != email)
        ui->lineEdit_email->setText(email);
    if (!pwdOk) errors << "Mot de passe: au moins 6 caractères.";
    if (!villeOk) errors << "Ville: obligatoire.";
    if (!hireOk) errors << "Date d'embauchement: ne peut pas être dans le futur.";
    if (!expOk) errors << "Date d'expiration: doit être aujourd'hui ou plus.";
    if (!licOk) errors << "Numero de License: obligatoire si Certifications rempli.";

    if (markFields) {
        setInvalid(ui->lineEdit_cin, !cinOk || !cinUniqueOk);
        setInvalid(ui->lineEdit_fname, !prenomOk);
        setInvalid(ui->lineEdit_lname, !nomOk);
        setInvalid(ui->combo_job, !occupationOk);
        setInvalid(ui->combo_dept, !deptOk);
        setInvalid(ui->lineEdit_phone, !phoneOk);
        setInvalid(ui->lineEdit_email, !emailOk);
        setInvalid(ui->lineEdit_pwd, !pwdOk);
        setInvalid(ui->combo_city, !villeOk);
        setInvalid(ui->dateEdit_hire, !hireOk);
        setInvalid(ui->dateEdit_exp, !expOk);
        setInvalid(ui->lineEdit_lic, !licOk);
    }

    if (!errors.isEmpty()) { if (errorMessage) *errorMessage = errors.join("\n"); return false; }
    if (errorMessage) errorMessage->clear();
    return true;
}

void MainWindow::clearEmployeeForm() {
    setCurrentEmployeeCin(QString());
    if (ui->lineEdit_cin) ui->lineEdit_cin->clear();
    if (ui->lineEdit_fname) ui->lineEdit_fname->clear();
    if (ui->lineEdit_lname) ui->lineEdit_lname->clear();
    if (ui->combo_job) ui->combo_job->setCurrentIndex(0);
    if (ui->combo_dept) ui->combo_dept->setCurrentIndex(0);
    if (ui->dateEdit_hire) ui->dateEdit_hire->setDate(QDate::currentDate());
    setEmployeeStatus("Actif");
    if (ui->lineEdit_phone) ui->lineEdit_phone->clear();
    if (ui->lineEdit_email) ui->lineEdit_email->clear();
    if (ui->lineEdit_pwd) ui->lineEdit_pwd->clear();
    if (ui->combo_city) ui->combo_city->setCurrentIndex(0);
    if (ui->lineEdit_addr) ui->lineEdit_addr->clear();
    if (ui->combo_certs) ui->combo_certs->setCurrentIndex(0);
    if (ui->combo_slevel) ui->combo_slevel->setCurrentIndex(0);
    if (ui->lineEdit_lic) ui->lineEdit_lic->clear();
    if (ui->dateEdit_exp) ui->dateEdit_exp->setDate(QDate::currentDate().addYears(1));
    setEmployeeShift("Matin");
    if (ui->lineEdit_rfid) ui->lineEdit_rfid->clear();
    if (ui->lineEdit_code) ui->lineEdit_code->clear();
    if (ui->label_photo) {
        ui->label_photo->clear();
        ui->label_photo->setText("Photo Placeholder");
    }
}

void MainWindow::loadEmployees(const QString &searchTerm, int sortIndex, bool refreshAlertsAfter) {
    if (!m_db.isValid() || !m_db.isOpen() || !ui->tableWidget_2) return;

    QString err;
    // UI -> DB: load employees with dynamic WHERE (search) and ORDER BY (sort index).
    const QList<EmployeeRow> rows = EmployeeCrud::listWithSort(m_db, searchTerm, sortIndex, &err);
    if (!err.trimmed().isEmpty()) {
        QMessageBox::warning(this, "Employés", "Erreur chargement:\n" + err);
        return;
    }

    const bool sorting = ui->tableWidget_2->isSortingEnabled();
    auto *header = ui->tableWidget_2->horizontalHeader();
    const int sortCol = header ? header->sortIndicatorSection() : 0;
    const Qt::SortOrder sortOrder = header ? header->sortIndicatorOrder() : Qt::AscendingOrder;

    ui->tableWidget_2->setSortingEnabled(false);
    ui->tableWidget_2->setRowCount(0);

    int row = 0;
    for (const EmployeeRow &e : rows) {
        ui->tableWidget_2->insertRow(row);
        auto setTextCell = [&](int col, const QString &text) {
            auto *item = new EmployeeSortItem(text, text.toLower());
            item->setFlags(item->flags() & ~Qt::ItemIsEditable);
            ui->tableWidget_2->setItem(row, col, item);
        };

        const int displayId = (e.id > 0) ? e.id : (row + 1);
        auto *item0 = new EmployeeSortItem(QString::number(displayId), displayId);
        item0->setData(Qt::UserRole + 1, e.cin);
        item0->setFlags(item0->flags() & ~Qt::ItemIsEditable);
        ui->tableWidget_2->setItem(row, 0, item0);

        setTextCell(1, e.cin);
        setTextCell(2, e.prenom);
        setTextCell(3, e.nom);
        setTextCell(4, e.occupation);
        setTextCell(5, e.departement);
        {
            auto *ditem = new EmployeeSortItem(empDisplayDate(e.date_embauche), e.date_embauche);
            ditem->setFlags(ditem->flags() & ~Qt::ItemIsEditable);
            ui->tableWidget_2->setItem(row, 6, ditem);
        }
        setTextCell(7, e.statut);
        setTextCell(8, e.telephone);
        setTextCell(9, e.email);
        {
            auto *pitem = new EmployeeSortItem("********", e.motdepasse.toLower());
            pitem->setData(Qt::UserRole + 1, e.motdepasse);
            pitem->setFlags(pitem->flags() & ~Qt::ItemIsEditable);
            ui->tableWidget_2->setItem(row, 10, pitem);
        }
        setTextCell(11, e.ville);
        setTextCell(12, e.certifications);
        setTextCell(13, e.niveaux);
        setTextCell(14, e.num_licence);
        {
            auto *ditem = new EmployeeSortItem(empDisplayDate(e.date_expiration), e.date_expiration);
            ditem->setFlags(ditem->flags() & ~Qt::ItemIsEditable);
            ui->tableWidget_2->setItem(row, 15, ditem);
        }
        setTextCell(16, e.quart);
        setTextCell(17, e.rfid);
        {
            const QString codePlain = e.emp_code;
            const QString codeShown = codePlain.isEmpty()
                ? QString()
                : (m_showCodesInList ? codePlain : QStringLiteral("********"));
            auto *citem = new EmployeeSortItem(codeShown, codePlain.toLower());
            citem->setData(Qt::UserRole + 1, codePlain);
            citem->setFlags(citem->flags() & ~Qt::ItemIsEditable);
            ui->tableWidget_2->setItem(row, 18, citem);
        }
        ++row;
    }

    ui->tableWidget_2->setSortingEnabled(sorting);
    if (ui->sort_combo) {
        int indicatorCol = 0;
        Qt::SortOrder indicatorOrder = Qt::AscendingOrder;
        switch (ui->sort_combo->currentIndex()) {
        case 1: indicatorCol = 0; indicatorOrder = Qt::DescendingOrder; break;
        case 2: indicatorCol = 3; indicatorOrder = Qt::AscendingOrder; break;
        case 3: indicatorCol = 3; indicatorOrder = Qt::DescendingOrder; break;
        default: indicatorCol = 0; indicatorOrder = Qt::AscendingOrder; break;
        }
        if (auto *h = ui->tableWidget_2->horizontalHeader())
            h->setSortIndicator(indicatorCol, indicatorOrder);
    } else if (sorting) {
        ui->tableWidget_2->sortItems(sortCol, sortOrder);
    }
    QTimer::singleShot(0, this, [this]() {
        if (ui->tableWidget_2)
            ui->tableWidget_2->resizeColumnsToContents();
    });
    if (auto *hi = ui->tableWidget_2->horizontalHeaderItem(10))
        hi->setText(QStringLiteral("Mot de passe  %1").arg(m_showPasswordsInList ? "C" : "V"));
    if (auto *hiCode = ui->tableWidget_2->horizontalHeaderItem(18))
        hiCode->setText(QStringLiteral("code  %1").arg(m_showCodesInList ? "C" : "V"));

    if (refreshAlertsAfter) {
        m_monitorCacheValid = false;
        m_empKpiCacheTime = QDateTime();
        refreshEmployeeListeAlerts();
    }
}

void MainWindow::on_sort_combo_currentIndexChanged(int index) {
    // Sort combo -> dynamic SQL ORDER BY (via DB query, not local table sort).
    if (!ui->tableWidget_2 || !ui->sort_combo) return;
    const QString searchText = ui->search_input ? ui->search_input->text() : QString();
    loadEmployees(searchText, index, false);
}

void MainWindow::on_add_btn_2_clicked() {
    QSqlDatabase db = QSqlDatabase::database(Connection::connectionName());
    if (!db.isValid() || !db.isOpen()) {
        QMessageBox::warning(this, "Employés", "Connexion base de données indisponible.");
        return;
    }
    QString err;
    if (!validateEmployeeForm(&err, true)) { QMessageBox::warning(this, "Validation", err); return; }

    EmployeeRow e;
    e.cin = ui->lineEdit_cin->text().trimmed();
    e.prenom = ui->lineEdit_fname->text().trimmed();
    e.nom = ui->lineEdit_lname->text().trimmed();
    e.occupation = ui->combo_job ? ui->combo_job->currentText().trimmed() : QString();
    e.departement = ui->combo_dept->currentText().trimmed();
    e.date_embauche = ui->dateEdit_hire->date();
    e.statut = selectedEmployeeStatus();
    e.telephone = ui->lineEdit_phone->text().trimmed();
    e.email = EmployeePassword::normalizeEmailInput(ui->lineEdit_email->text());
    e.motdepasse = ui->lineEdit_pwd->text();
    e.ville = ui->combo_city ? ui->combo_city->currentText().trimmed() : QString();
    e.adresse = ui->lineEdit_addr->text().trimmed();
    e.certifications = ui->combo_certs ? ui->combo_certs->currentText().trimmed() : QString();
    e.niveaux = ui->combo_slevel ? ui->combo_slevel->currentText().trimmed() : QString();
    e.num_licence = ui->lineEdit_lic->text().trimmed();
    e.date_expiration = ui->dateEdit_exp->date();
    e.quart = selectedEmployeeShift();
    e.rfid = ui->lineEdit_rfid ? ui->lineEdit_rfid->text().trimmed() : QString();
    e.emp_code = ui->lineEdit_code ? ui->lineEdit_code->text().trimmed() : QString();
    e.photo_path = ui->label_photo ? ui->label_photo->property("photoPath").toString() : QString();

    QString dbErr;
    if (!EmployeeCrud::insert(db, e, nullptr, &dbErr)) {
        QMessageBox::warning(this, tr("Employés"), tr("Erreur d'ajout:\n%1").arg(formatEmployeeDbError(dbErr)));
        return;
    }
    loadEmployees(ui->search_input ? ui->search_input->text() : QString());
    clearEmployeeForm();
}

void MainWindow::on_update_btn_2_clicked() {
    QSqlDatabase db = QSqlDatabase::database(Connection::connectionName());
    if (!db.isValid() || !db.isOpen()) {
        QMessageBox::warning(this, "Employés", "Connexion base de données indisponible.");
        return;
    }
    const QString cin = currentEmployeeCin();
    if (cin.isEmpty()) { QMessageBox::warning(this, "Employés", "Sélectionnez un employé pour modifier."); return; }

    QString err;
    if (!validateEmployeeForm(&err, true)) { QMessageBox::warning(this, "Validation", err); return; }

    EmployeeRow e;
    e.cin = ui->lineEdit_cin->text().trimmed();
    e.prenom = ui->lineEdit_fname->text().trimmed();
    e.nom = ui->lineEdit_lname->text().trimmed();
    e.occupation = ui->combo_job ? ui->combo_job->currentText().trimmed() : QString();
    e.departement = ui->combo_dept->currentText().trimmed();
    e.date_embauche = ui->dateEdit_hire->date();
    e.statut = selectedEmployeeStatus();
    e.telephone = ui->lineEdit_phone->text().trimmed();
    e.email = EmployeePassword::normalizeEmailInput(ui->lineEdit_email->text());
    e.motdepasse = ui->lineEdit_pwd->text();
    e.ville = ui->combo_city ? ui->combo_city->currentText().trimmed() : QString();
    e.adresse = ui->lineEdit_addr->text().trimmed();
    e.certifications = ui->combo_certs ? ui->combo_certs->currentText().trimmed() : QString();
    e.niveaux = ui->combo_slevel ? ui->combo_slevel->currentText().trimmed() : QString();
    e.num_licence = ui->lineEdit_lic->text().trimmed();
    e.date_expiration = ui->dateEdit_exp->date();
    e.quart = selectedEmployeeShift();
    e.rfid = ui->lineEdit_rfid ? ui->lineEdit_rfid->text().trimmed() : QString();
    e.emp_code = ui->lineEdit_code ? ui->lineEdit_code->text().trimmed() : QString();
    e.photo_path = ui->label_photo ? ui->label_photo->property("photoPath").toString() : QString();

    QString dbErr;
    if (!EmployeeCrud::update(db, cin, e, &dbErr)) {
        QMessageBox::warning(this, tr("Employés"), tr("Erreur mise à jour:\n%1").arg(formatEmployeeDbError(dbErr)));
        return;
    }
    const int row = ui->tableWidget_2->currentRow();
    if (row >= 0 && row < ui->tableWidget_2->rowCount()) {
        auto setCell = [this, row](int col, const QString &text) {
            auto *it = ui->tableWidget_2->item(row, col);
            if (it) {
                it->setText(text);
                it->setData(Qt::UserRole, text.toLower());
            } else {
                auto *item = new EmployeeSortItem(text, text.toLower());
                item->setFlags(item->flags() & ~Qt::ItemIsEditable);
                ui->tableWidget_2->setItem(row, col, item);
            }
        };
        setCell(1, e.cin);
        setCell(2, e.prenom);
        setCell(3, e.nom);
        setCell(4, e.occupation);
        setCell(5, e.departement);
        if (auto *ditem = ui->tableWidget_2->item(row, 6)) {
            ditem->setText(empDisplayDate(e.date_embauche));
            ditem->setData(Qt::UserRole, e.date_embauche);
        }
        setCell(7, e.statut);
        setCell(8, e.telephone);
        setCell(9, e.email);
        auto *pitem = ui->tableWidget_2->item(row, 10);
        if (pitem) {
            pitem->setData(Qt::UserRole + 1, e.motdepasse);
            pitem->setText(m_showPasswordsInList ? e.motdepasse : QStringLiteral("********"));
        }
        setCell(11, e.ville);
        setCell(12, e.certifications);
        setCell(13, e.niveaux);
        setCell(14, e.num_licence);
        if (auto *expItem = ui->tableWidget_2->item(row, 15)) {
            expItem->setText(empDisplayDate(e.date_expiration));
            expItem->setData(Qt::UserRole, e.date_expiration);
        }
        setCell(16, e.quart);
        setCurrentEmployeeCin(e.cin);
        if (auto *it0 = ui->tableWidget_2->item(row, 0))
            it0->setData(Qt::UserRole + 1, e.cin);
        m_monitorCacheValid = false;
        m_empKpiCacheTime = QDateTime();
        refreshEmployeeListeAlerts();
    } else {
        loadEmployees(ui->search_input ? ui->search_input->text() : QString());
    }
}

void MainWindow::on_delete_btn_2_clicked() {
    QSqlDatabase db = QSqlDatabase::database(Connection::connectionName());
    if (!db.isValid() || !db.isOpen()) {
        QMessageBox::warning(this, "Employés", "Connexion base de données indisponible.");
        return;
    }
    const QString cin = currentEmployeeCin();
    if (cin.isEmpty()) { QMessageBox::warning(this, "Employés", "Sélectionnez un employé pour supprimer."); return; }
    if (QMessageBox::question(this, "Suppression", "Supprimer cet employé ?") != QMessageBox::Yes) return;

    QString dbErr;
    if (!EmployeeCrud::remove(db, cin, &dbErr)) {
        QMessageBox::warning(this, tr("Employés"), tr("Erreur suppression:\n%1").arg(formatEmployeeDbError(dbErr)));
        return;
    }
    loadEmployees(ui->search_input ? ui->search_input->text() : QString());
    clearEmployeeForm();
}

void MainWindow::on_search_btn_2_clicked() {
    loadEmployees(ui->search_input ? ui->search_input->text() : QString(), 0, false);
}

static QString employeeExportOrderBySql(int sortIndex, const QString &idColOrFallback)
{
    switch (sortIndex) {
    case 1: return QStringLiteral(" ORDER BY %1 DESC").arg(idColOrFallback);
    case 2: return QStringLiteral(" ORDER BY nom ASC, prenom ASC");
    case 3: return QStringLiteral(" ORDER BY nom DESC, prenom DESC");
    case 0:
    default:
        return QStringLiteral(" ORDER BY %1 ASC").arg(idColOrFallback);
    }
}

static bool fetchEmployeesForExport(QSqlDatabase &db, const QString &searchTerm, int sortIndex, QList<EmployeeRow> *outRows, QString *errorMessage)
{
    if (outRows) outRows->clear();
    if (!db.isValid() || !db.isOpen()) {
        if (errorMessage) *errorMessage = QObject::tr("Connexion base de données indisponible.");
        return false;
    }

    QSqlRecord rec = db.record(QStringLiteral("employe"));
    if (rec.count() == 0)
        rec = db.record(QStringLiteral("EMPLOYE"));
    auto hasCol = [&](const QString &c) { return rec.indexOf(c) != -1; };
    auto pickCol = [&](const QString &a, const QString &b) { return hasCol(a) ? a : (hasCol(b) ? b : QString()); };

    QString idCol = pickCol(QStringLiteral("id"), QStringLiteral("ID"));
    QString rfidCol = pickCol(QStringLiteral("rfid"), QStringLiteral("RFID"));
    QString codeCol = pickCol(QStringLiteral("code"), QStringLiteral("CODE"));

    // ODBC Oracle : QSqlRecord("employe") est souvent vide avant toute requête → éviter NULL AS id (tous les 0 en export).
    const bool metaEmpty = (rec.count() == 0);
    if (metaEmpty) {
        if (idCol.isEmpty())
            idCol = QStringLiteral("id");
        if (rfidCol.isEmpty())
            rfidCol = QStringLiteral("rfid");
        if (codeCol.isEmpty())
            codeCol = QStringLiteral("code");
    }

    const QString idExpr = idCol.isEmpty() ? QStringLiteral("NULL AS id") : idCol;
    const QString rfidExpr = rfidCol.isEmpty() ? QStringLiteral("NULL AS rfid") : rfidCol;
    const QString codeExpr = codeCol.isEmpty() ? QStringLiteral("NULL AS code") : codeCol;

    QSqlQuery q(db);
    QString sql =
        QStringLiteral("SELECT %1, cin, prenom, nom, poste, service, date_embauche, statut_professionnel, "
                       "numero_telephone, adresse_email, mot_de_passe, ville, certifications, niveau_competence, "
                       "numero_license, date_expiration_certification, horaire_travail, "
                       "%2, %3 "
                       "FROM employe ")
            .arg(idExpr, rfidExpr, codeExpr);

    const QString term = searchTerm.trimmed();
    if (!term.isEmpty()) {
        sql += QStringLiteral(
            "WHERE UPPER(cin) LIKE UPPER(:term) "
            "OR UPPER(prenom) LIKE UPPER(:term) "
            "OR UPPER(nom) LIKE UPPER(:term) "
            "OR UPPER(poste) LIKE UPPER(:term) "
            "OR UPPER(service) LIKE UPPER(:term) "
            "OR UPPER(statut_professionnel) LIKE UPPER(:term) "
            "OR UPPER(numero_telephone) LIKE UPPER(:term) "
            "OR UPPER(adresse_email) LIKE UPPER(:term) "
            "OR UPPER(ville) LIKE UPPER(:term) "
            "OR UPPER(certifications) LIKE UPPER(:term) "
            "OR UPPER(niveau_competence) LIKE UPPER(:term) "
            "OR UPPER(numero_license) LIKE UPPER(:term) "
            "OR UPPER(horaire_travail) LIKE UPPER(:term) ");
        if (!rfidCol.isEmpty())
            sql += QStringLiteral("OR UPPER(%1) LIKE UPPER(:term) ").arg(rfidCol);
        if (!codeCol.isEmpty())
            sql += QStringLiteral("OR UPPER(%1) LIKE UPPER(:term) ").arg(codeCol);
    }
    sql += employeeExportOrderBySql(sortIndex, idCol.isEmpty() ? QStringLiteral("cin") : idCol);

    if (!q.prepare(sql)) {
        if (errorMessage) *errorMessage = q.lastError().text();
        return false;
    }
    if (!term.isEmpty())
        q.bindValue(QStringLiteral(":term"), QStringLiteral("%%1%").arg(term));

    if (!q.exec()) {
        if (errorMessage) *errorMessage = q.lastError().text();
        return false;
    }

    QList<EmployeeRow> rows;
    while (q.next()) {
        EmployeeRow e;
        e.id = q.value(0).isNull() ? 0 : static_cast<int>(q.value(0).toLongLong());
        e.cin = q.value(1).toString();
        e.prenom = q.value(2).toString();
        e.nom = q.value(3).toString();
        e.occupation = q.value(4).toString();
        e.departement = q.value(5).toString();
        e.date_embauche = q.value(6).toDate();
        e.statut = q.value(7).toString();
        e.telephone = q.value(8).toString();
        e.email = q.value(9).toString();
        e.motdepasse = q.value(10).toString();
        e.ville = q.value(11).toString();
        e.certifications = q.value(12).toString();
        e.niveaux = q.value(13).toString();
        e.num_licence = q.value(14).toString();
        e.date_expiration = q.value(15).toDate();
        e.quart = q.value(16).toString();
        e.rfid = q.value(17).toString();
        e.emp_code = q.value(18).toString();
        rows.append(e);
    }

    if (outRows) *outRows = rows;
    return true;
}

void MainWindow::on_export_excel_btn_clicked() {
    const int sortIndex = ui->sort_combo ? ui->sort_combo->currentIndex() : 0;

    QList<EmployeeRow> rows;
    QString dbErr;
    // Export = liste complète en base (ignorer le filtre de recherche de l'écran).
    if (!fetchEmployeesForExport(m_db, QString(), sortIndex, &rows, &dbErr)) {
        QMessageBox::warning(this, tr("Export"), tr("Erreur lors de la lecture depuis la base:\n%1").arg(formatEmployeeDbError(dbErr)));
        return;
    }

    QString filePath = QFileDialog::getSaveFileName(
        this,
        tr("Exporter les employés"),
        QStringLiteral("employees.csv"),
        tr("CSV (*.csv);;PDF (*.pdf)"));
    if (filePath.isEmpty())
        return;

    const bool toPdf = filePath.endsWith(QStringLiteral(".pdf"), Qt::CaseInsensitive);
    if (!toPdf && !filePath.endsWith(QStringLiteral(".csv"), Qt::CaseInsensitive))
        filePath += QStringLiteral(".csv");

    const QStringList headers = {
        QStringLiteral("ID"), QStringLiteral("CIN"), QStringLiteral("Prenom"), QStringLiteral("Nom"),
        QStringLiteral("Poste"), QStringLiteral("Service"), QStringLiteral("Date embauche"), QStringLiteral("Statut"),
        QStringLiteral("Telephone"), QStringLiteral("Email"), QStringLiteral("Mot de passe"), QStringLiteral("Ville"),
        QStringLiteral("Certifications"), QStringLiteral("Niveau"), QStringLiteral("Numero licence"),
        QStringLiteral("Date expiration"), QStringLiteral("Quart"), QStringLiteral("RFID"), QStringLiteral("Code")
    };

    if (!toPdf) {
        QFile f(filePath);
        if (!f.open(QIODevice::WriteOnly)) {
            QMessageBox::warning(this, tr("Export"), tr("Impossible d'écrire le fichier."));
            return;
        }
        // BOM UTF-8 en octets bruts (évite Ã¯Â»Â¿ devant la première colonne si Excel ouvre en ANSI).
        if (f.write(QByteArrayLiteral("\xEF\xBB\xBF")) != 3) {
            QMessageBox::warning(this, tr("Export"), tr("Impossible d'écrire le fichier."));
            return;
        }
        QTextStream out(&f);
        out.setEncoding(QStringConverter::Utf8);

        for (int c = 0; c < headers.size(); ++c) {
            QString headerText = headers.at(c);
            out << "\"" << headerText.replace("\"", "\"\"") << "\"";
            if (c < headers.size() - 1) out << ";";
        }
        out << "\n";

        for (const EmployeeRow &e : rows) {
            const QStringList values = {
                QString::number(e.id), e.cin, e.prenom, e.nom, e.occupation, e.departement,
                empDisplayDate(e.date_embauche), e.statut, e.telephone, e.email, e.motdepasse,
                e.ville, e.certifications, e.niveaux, e.num_licence, empDisplayDate(e.date_expiration),
                e.quart, e.rfid, e.emp_code
            };
            for (int c = 0; c < values.size(); ++c) {
                QString text = values[c];
                out << "\"" << text.replace("\"", "\"\"") << "\"";
                if (c < values.size() - 1) out << ";";
            }
            out << "\n";
        }
        f.close();
        QMessageBox::information(this, tr("Export"), tr("Export CSV réussi :\n%1").arg(filePath));
        return;
    }

    QPrinter printer(QPrinter::HighResolution);
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setOutputFileName(filePath);
    printer.setPageSize(QPageSize(QPageSize::A4));
    printer.setPageOrientation(QPageLayout::Landscape);
    printer.setPageMargins(QMarginsF(8, 8, 8, 8), QPageLayout::Millimeter);
    printer.setResolution(150);

    QString html = QStringLiteral("<!DOCTYPE html><html><head><meta charset=\"utf-8\">"
                                  "<style>"
                                  "body{font-family:'Segoe UI',sans-serif;font-size:8.5pt;color:#111827;}"
                                  "h2{color:#0f766e;margin-bottom:4px;}"
                                  ".meta{color:#475569;margin-bottom:8px;}"
                                  "table{width:100%;border-collapse:collapse;}"
                                  "th,td{border:1px solid #cbd5e1;padding:4px 6px;text-align:left;}"
                                  "th{background:#0f766e;color:white;font-weight:bold;}"
                                  "tr:nth-child(even) td{background:#f8fafc;}"
                                  "</style></head><body>");
    html += QStringLiteral("<h2>%1</h2>").arg(tr("Export Employés"));
    html += QStringLiteral("<div class=\"meta\">%1<br/>%2<br/>%3: %4</div>")
        .arg(tr("Liste complète : tous les employés en base (filtre de recherche non appliqué)."))
        .arg(tr("Données chargées depuis la base (requête SQL dynamique)"))
        .arg(tr("Tri"), ui->sort_combo ? ui->sort_combo->currentText().toHtmlEscaped() : tr("ID (Asc)"));

    html += QStringLiteral("<table><thead><tr>");
    for (const QString &h : headers)
        html += QStringLiteral("<th>%1</th>").arg(h.toHtmlEscaped());
    html += QStringLiteral("</tr></thead><tbody>");

    for (const EmployeeRow &e : rows) {
        const QStringList values = {
            QString::number(e.id), e.cin, e.prenom, e.nom, e.occupation, e.departement,
            empDisplayDate(e.date_embauche), e.statut, e.telephone, e.email, e.motdepasse,
            e.ville, e.certifications, e.niveaux, e.num_licence, empDisplayDate(e.date_expiration),
            e.quart, e.rfid, e.emp_code
        };
        html += QStringLiteral("<tr>");
        for (const QString &v : values)
            html += QStringLiteral("<td>%1</td>").arg(v.toHtmlEscaped());
        html += QStringLiteral("</tr>");
    }
    html += QStringLiteral("</tbody></table></body></html>");

    QTextDocument doc;
    doc.setDocumentMargin(0);
    doc.setHtml(html);
    doc.print(&printer);
    QMessageBox::information(this, tr("Export"), tr("Export PDF réussi :\n%1").arg(filePath));
}

void MainWindow::on_clear_btn_2_clicked() {
    clearEmployeeForm();
}

void MainWindow::on_btn_upload_photo_clicked() {
    if (!ui->label_photo) return;
    const QString filePath = QFileDialog::getOpenFileName(this, "Choisir une photo", QString(), "Images (*.png *.jpg *.jpeg *.bmp *.webp)");
    if (filePath.isEmpty()) return;
    if (!setPhotoFromPath(ui->label_photo, filePath)) {
        QMessageBox::warning(this, "Photo", "Impossible de charger cette image.");
        return;
    }
}

void MainWindow::on_btn_camera_photo_clicked() {
    if (!ui->label_photo) return;
    const QList<QCameraDevice> cams = QMediaDevices::videoInputs();
    if (cams.isEmpty()) {
        QMessageBox::warning(this, "Camera", "Aucune caméra détectée.");
        return;
    }
    auto *dlg = new QDialog(this);
    dlg->setWindowTitle("Camera - Prendre une photo");
    dlg->setModal(true);
    dlg->resize(520, 420);
    dlg->setAttribute(Qt::WA_DeleteOnClose, true);
    auto *layout = new QVBoxLayout(dlg);
    layout->setContentsMargins(10, 10, 10, 10);
    layout->setSpacing(10);
    auto *video = new QVideoWidget(dlg);
    video->setMinimumSize(480, 320);
    layout->addWidget(video, 1);
    auto *row = new QHBoxLayout();
    auto *btnCapture = new QPushButton("Prendre Photo", dlg);
    auto *btnClose = new QPushButton("Fermer", dlg);
    row->addStretch(1);
    row->addWidget(btnCapture);
    row->addWidget(btnClose);
    layout->addLayout(row);
    auto *camera = new QCamera(cams.first(), dlg);
    auto *capture = new QImageCapture(dlg);
    auto *session = new QMediaCaptureSession(dlg);
    session->setCamera(camera);
    session->setVideoOutput(video);
    session->setImageCapture(capture);
    connect(btnClose, &QPushButton::clicked, dlg, &QDialog::reject);
    connect(dlg, &QDialog::finished, camera, &QCamera::stop);
    connect(btnCapture, &QPushButton::clicked, dlg, [=]() {
        const QString dir = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
        const QString file = QDir(dir).filePath("employee_photo_" + QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss") + ".jpg");
        capture->setProperty("lastFile", file);
        capture->captureToFile(file);
    });
    connect(capture, &QImageCapture::imageCaptured, dlg, [=](int, const QImage &img) {
        setPhotoFromImage(ui->label_photo, img);
        btnCapture->setEnabled(false);
    });
    connect(capture, &QImageCapture::imageSaved, dlg, [=](int, const QString &fileName) {
        if (!fileName.trimmed().isEmpty() && QFileInfo::exists(fileName)) {
            QImage img(fileName);
            if (!img.isNull() && detectFaceInImageWithScript(img)) {
                setPhotoFromPath(ui->label_photo, fileName);
            } else {
                setPhotoPlaceholder(ui->label_photo);
                if (!isFaceDetectionScriptAvailable())
                    QMessageBox::warning(dlg, tr("Photo"),
                        tr("La vérification du visage n’est pas disponible sur cet ordinateur.\nRéessayez plus tard ou utilisez une autre photo."));
                else
                    QMessageBox::warning(dlg, tr("Photo"),
                        tr("Aucun visage détecté.\nPlacez votre visage au centre, avec un bon éclairage, puis réessayez."));
            }
        }
        dlg->accept();
    });
    connect(capture, &QImageCapture::errorOccurred, dlg, [=](int, QImageCapture::Error, const QString &errorString) {
        QMessageBox::warning(dlg, "Camera", "Erreur capture:\n" + errorString);
    });
    camera->start();
    dlg->exec();
}

void MainWindow::on_btn_esp32_photo_clicked() {
    if (!ui->label_photo) return;
    openEsp32CamDialog(ui->label_photo, tr("ESP32-CAM — Prendre une photo"), tr("Photo"));
}

// --- Face ID page (label_photo_3) ---
void MainWindow::on_btn_face_upload_clicked() {
    if (!ui->label_photo_3) return;
    const QString filePath = QFileDialog::getOpenFileName(this, "Choisir une photo (Login)", QString(), "Images (*.png *.jpg *.jpeg *.bmp *.webp)");
    if (filePath.isEmpty()) return;
    if (!setPhotoFromPath(ui->label_photo_3, filePath)) {
        QMessageBox::warning(this, "Login Image", "Impossible de charger cette image.");
        setPhotoPlaceholder(ui->label_photo_3);
        return;
    }
    QImage img(filePath);
    if (!img.isNull() && !detectFaceInImageWithScript(img)) {
        setPhotoPlaceholder(ui->label_photo_3);
        if (!isFaceDetectionScriptAvailable())
            QMessageBox::warning(this, tr("Connexion"),
                tr("La vérification du visage n’est pas disponible sur cet ordinateur.\nRéessayez plus tard ou utilisez une autre photo."));
        else
            QMessageBox::warning(this, tr("Connexion"),
                tr("Aucun visage détecté.\nPlacez votre visage au centre, avec un bon éclairage, puis réessayez."));
    }
}

void MainWindow::on_btn_face_camera_clicked() {
    if (!ui->label_photo_3) return;
    const QList<QCameraDevice> cams = QMediaDevices::videoInputs();
    if (cams.isEmpty()) {
        QMessageBox::warning(this, "Camera", "Aucune caméra détectée.");
        return;
    }
    auto *dlg = new QDialog(this);
    dlg->setWindowTitle("Camera (Login) - Prendre une photo");
    dlg->setModal(true);
    dlg->resize(520, 420);
    dlg->setAttribute(Qt::WA_DeleteOnClose, true);
    auto *layout = new QVBoxLayout(dlg);
    layout->setContentsMargins(10, 10, 10, 10);
    layout->setSpacing(10);
    auto *video = new QVideoWidget(dlg);
    video->setMinimumSize(480, 320);
    layout->addWidget(video, 1);
    auto *row = new QHBoxLayout();
    auto *btnCapture = new QPushButton("Prendre Photo", dlg);
    auto *btnClose = new QPushButton("Fermer", dlg);
    row->addStretch(1);
    row->addWidget(btnCapture);
    row->addWidget(btnClose);
    layout->addLayout(row);
    auto *camera = new QCamera(cams.first(), dlg);
    auto *capture = new QImageCapture(dlg);
    auto *session = new QMediaCaptureSession(dlg);
    session->setCamera(camera);
    session->setVideoOutput(video);
    session->setImageCapture(capture);
    connect(btnClose, &QPushButton::clicked, dlg, &QDialog::reject);
    connect(dlg, &QDialog::finished, camera, &QCamera::stop);
    connect(btnCapture, &QPushButton::clicked, dlg, [=]() {
        const QString dir = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
        const QString file = QDir(dir).filePath("login_photo_" + QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss") + ".jpg");
        capture->setProperty("lastFile", file);
        capture->captureToFile(file);
    });
    connect(capture, &QImageCapture::imageCaptured, dlg, [=](int, const QImage &img) {
        setPhotoFromImage(ui->label_photo_3, img);
        btnCapture->setEnabled(false);
    });
    connect(capture, &QImageCapture::imageSaved, dlg, [=](int, const QString &fileName) {
        if (!fileName.trimmed().isEmpty() && QFileInfo::exists(fileName)) {
            QImage img(fileName);
            if (!img.isNull() && detectFaceInImageWithScript(img)) {
                setPhotoFromPath(ui->label_photo_3, fileName);
            } else {
                setPhotoPlaceholder(ui->label_photo_3);
                if (!isFaceDetectionScriptAvailable())
                    QMessageBox::warning(dlg, tr("Connexion"),
                        tr("La vérification du visage n’est pas disponible sur cet ordinateur.\nRéessayez plus tard ou utilisez une autre photo."));
                else
                    QMessageBox::warning(dlg, tr("Connexion"),
                        tr("Aucun visage détecté.\nPlacez votre visage au centre, avec un bon éclairage, puis réessayez."));
            }
        }
        dlg->accept();
    });
    connect(capture, &QImageCapture::errorOccurred, dlg, [=](int, QImageCapture::Error, const QString &errorString) {
        QMessageBox::warning(dlg, "Camera", "Erreur capture:\n" + errorString);
    });
    camera->start();
    dlg->exec();
}

void MainWindow::on_btn_face_esp32_clicked() {
    if (!ui->label_photo_3) return;
    openEsp32CamDialog(ui->label_photo_3, tr("ESP32-CAM (connexion) — Prendre une photo"), tr("Connexion"));
}

void MainWindow::on_btn_face_verify_clicked() {
    if (!ui->label_photo_3) {
        QMessageBox::warning(this, "Login Image", "Widget photo introuvable.");
        return;
    }
    if (!m_db.isValid() || !m_db.isOpen()) {
        QMessageBox::warning(this, "Login Image", "Connexion base de données indisponible.");
        return;
    }
    const QPixmap pix = ui->label_photo_3->pixmap(Qt::ReturnByValue);
    if (pix.isNull()) {
        QMessageBox::warning(this, "Login Image", "Veuillez choisir une photo (Upload ou Camera).");
        return;
    }
    const QImage queryImg = pix.toImage();
    if (!detectFaceInImageWithScript(queryImg)) {
        if (!isFaceDetectionScriptAvailable())
            QMessageBox::warning(this, tr("Connexion"),
                tr("La vérification du visage n’est pas disponible sur cet ordinateur.\nRéessayez plus tard ou utilisez une autre photo."));
        else
            QMessageBox::warning(this, tr("Connexion"),
                tr("Aucun visage détecté.\nPlacez votre visage au centre, avec un bon éclairage, puis réessayez."));
        return;
    }
    const QList<double> queryCropScales = { 0.84, 0.90, 0.96, 1.0, 1.06, 1.12 };
    QVector<QVector<float>> queryDescriptors;
    for (double scale : queryCropScales) {
        QImage cropped = centerCropSquareAtScale(queryImg, scale);
        if (cropped.isNull()) continue;
        QVector<float> desc = lbpGridDescriptorMultiScale(cropped, 8);
        if (!desc.isEmpty()) queryDescriptors.append(desc);
    }
    if (queryDescriptors.isEmpty()) {
        QMessageBox::warning(this, "Login Image", "Image invalide ou trop petite.");
        return;
    }
    QSqlQuery q(m_db);
    q.prepare("SELECT cin, prenom, nom, statut_professionnel, photo FROM employe WHERE photo IS NOT NULL");
    if (!q.exec()) {
        QMessageBox::warning(this, "Login Image", "Erreur base de données:\n" + q.lastError().text());
        return;
    }
    QString bestCin, bestName;
    double bestDist = std::numeric_limits<double>::infinity();
    double secondDist = std::numeric_limits<double>::infinity();
    int scanned = 0;
    while (q.next()) {
        const QString cin = q.value(0).toString();
        const QString prenom = q.value(1).toString();
        const QString nom = q.value(2).toString();
        const QString statut = q.value(3).toString().trimmed().toLower();
        const QString path = q.value(4).toString().trimmed();
        if (statut.contains("inactif") || statut.contains("cong")) continue;
        if (path.isEmpty() || !QFileInfo::exists(path)) continue;
        QVector<QVector<float>> refDescriptors;
        if (FaceDescriptorCache::cache.contains(path)) {
            refDescriptors = FaceDescriptorCache::cache.value(path);
        } else {
            QImage ref(path);
            if (ref.isNull()) continue;
            const QList<double> refScales = { 0.86, 0.92, 0.97, 1.0, 1.05, 1.12 };
            for (double scale : refScales) {
                QImage cropped = centerCropSquareAtScale(ref, scale);
                if (cropped.isNull()) continue;
                QVector<float> desc = lbpGridDescriptorMultiScale(cropped, 8);
                if (!desc.isEmpty()) refDescriptors.append(desc);
            }
            if (refDescriptors.isEmpty()) continue;
            FaceDescriptorCache::cache.insert(path, refDescriptors);
        }
        ++scanned;
        double dist = std::numeric_limits<double>::infinity();
        for (const QVector<float> &qDesc : queryDescriptors) {
            for (const QVector<float> &refDesc : refDescriptors) {
                if (qDesc.size() != refDesc.size()) continue;
                const double d = chiSquareDistance(qDesc, refDesc);
                if (std::isfinite(d) && d < dist) dist = d;
            }
        }
        if (!std::isfinite(dist)) continue;
        if (dist < bestDist) {
            secondDist = bestDist;
            bestDist = dist;
            bestCin = cin;
            bestName = prenom + " " + nom;
        } else if (dist < secondDist) {
            secondDist = dist;
        }
    }
    if (scanned == 0) {
        QMessageBox::warning(this, "Login Image", "Aucune photo employé enregistrée.\nAjoutez une photo à un employé puis enregistrez.");
        return;
    }
    const double absThreshold = 52.0;
    const double ratioThreshold = 0.93;
    const bool hasSecond = std::isfinite(secondDist);
    const bool acceptAbs = (!bestCin.isEmpty()) && std::isfinite(bestDist) && (bestDist <= absThreshold);
    const bool acceptRatio = acceptAbs && (!hasSecond || (bestDist / qMax(1e-9, secondDist) <= ratioThreshold));
    if (acceptRatio) {
        QMessageBox::information(this, "Login Image", "Bienvenue " + bestName + " !");
        ui->mainStackedWidget->setCurrentIndex(0);
        setPhotoPlaceholder(ui->label_photo_3);
        return;
    }
    QMessageBox::warning(this, "Login Image", "Aucun employé correspondant (photo non reconnue).\nAstuce : utilisez une photo bien éclairée, visage centré, sans filtre.");
}

void MainWindow::openEsp32CamDialog(QLabel *targetLabel, const QString &dialogTitle, const QString &messageContext) {
    if (!targetLabel) return;
    const QString rfidPort = (m_rfidArduino.getserial() && m_rfidArduino.getserial()->isOpen())
        ? m_rfidArduino.getarduino_port_name()
        : QString();
    m_esp32Cam.set_exclude_serial_port(rfidPort);
    int linkRet = m_esp32Cam.connect_arduino();
    if (linkRet != 0) {
        if (!m_esp32Cam.prompt_configure_port(this))
            return;
    }
    QImage lastCaptured, selectedImage;
    auto *dlg = new QDialog(this);
    dlg->setWindowTitle(dialogTitle);
    dlg->setModal(true);
    dlg->resize(520, 420);
    dlg->setAttribute(Qt::WA_DeleteOnClose, true);
    auto *layout = new QVBoxLayout(dlg);
    layout->setContentsMargins(10, 10, 10, 10);
    layout->setSpacing(10);
    auto *preview = new QLabel(dlg);
    preview->setMinimumSize(480, 320);
    preview->setAlignment(Qt::AlignCenter);
    preview->setStyleSheet("border: 2px dashed #3498db; color: #7f8c8d; background-color: #2b2b2b;");
    preview->setText(QStringLiteral("Aperçu live (câble USB)..."));
    layout->addWidget(preview, 1);
    auto *ledRow = new QHBoxLayout();
    auto *lblLed = new QLabel("LED flash:", dlg);
    auto *sliderLed = new QSlider(Qt::Horizontal, dlg);
    sliderLed->setRange(0, 255);
    sliderLed->setValue(0);
    sliderLed->setMinimumWidth(120);
    ledRow->addWidget(lblLed);
    ledRow->addWidget(sliderLed, 1);
    layout->addLayout(ledRow);
    connect(sliderLed, &QSlider::valueChanged, dlg, [this](int v) {
        m_esp32Cam.write_flash_led(static_cast<quint8>(qBound(0, v, 255)));
    });
    m_esp32Cam.write_flash_led(0);
    auto *row = new QHBoxLayout();
    auto *btnCapture = new QPushButton("Prendre Photo", dlg);
    auto *btnClose = new QPushButton("Fermer", dlg);
    row->addStretch(1);
    row->addWidget(btnCapture);
    row->addWidget(btnClose);
    layout->addLayout(row);
    /** Comportement identique à la caméra PC : validation + fermeture automatique après capture. */
    auto applyEsp32PhotoAndClose = [this, dlg, targetLabel, messageContext](const QImage &toApply) {
        if (toApply.isNull())
            return;
        if (detectFaceInImageWithScript(toApply)) {
            const QString prefix = (targetLabel == ui->label_photo_3) ? "login_photo_esp32_" : "employee_photo_esp32_";
            const QString path = QDir::temp().filePath(prefix + QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss") + ".jpg");
            if (toApply.save(path, "JPEG", 90))
                setPhotoFromPath(targetLabel, path);
        } else {
            setPhotoPlaceholder(targetLabel);
            if (!isFaceDetectionScriptAvailable())
                QMessageBox::warning(dlg, messageContext,
                    tr("La vérification du visage n’est pas disponible sur cet ordinateur.\nRéessayez plus tard ou utilisez une autre photo."));
            else
                QMessageBox::warning(dlg, messageContext,
                    tr("Aucun visage détecté.\nPlacez votre visage au centre, avec un bon éclairage, puis réessayez."));
        }
        dlg->accept();
    };
    auto *liveTimer = new QTimer(dlg);
    liveTimer->setSingleShot(true);
    connect(liveTimer, &QTimer::timeout, dlg, [this, dlg, preview, &lastCaptured, &selectedImage, liveTimer]() {
        QImage img;
        QString err;
        if (m_esp32Cam.capture_esp32_cam_jpeg(&img, &err)) {
            lastCaptured = img;
            setPhotoFromImage(preview, img);
        }
        if (selectedImage.isNull() && dlg->isVisible())
            liveTimer->start(2500);
    });
    connect(dlg, &QDialog::finished, liveTimer, &QTimer::stop);
    connect(dlg, &QDialog::finished, this, [this, sliderLed](int) {
        if (sliderLed) {
            const QSignalBlocker b(sliderLed);
            sliderLed->setValue(0);
        }
        m_esp32Cam.write_flash_led(0);
    });
    liveTimer->start(800);
    connect(btnCapture, &QPushButton::clicked, dlg, [this, dlg, preview, btnCapture, sliderLed, &lastCaptured, &selectedImage, liveTimer, applyEsp32PhotoAndClose]() {
        liveTimer->stop();
        btnCapture->setEnabled(false);
        QImage img;
        QString err;
        if (!m_esp32Cam.capture_esp32_cam_jpeg(&img, &err)) {
            if (!err.isEmpty())
                QMessageBox::warning(dlg, QStringLiteral("ESP32-CAM"), err);
            btnCapture->setEnabled(true);
            if (dlg->isVisible())
                liveTimer->start(800);
            return;
        }
        {
            const QSignalBlocker b(sliderLed);
            sliderLed->setValue(0);
        }
        m_esp32Cam.write_flash_led(0);
        lastCaptured = img;
        selectedImage = img;
        setPhotoFromImage(preview, img);
        preview->setStyleSheet("border: 2px solid #2ecc71; border-radius: 6px;");
        applyEsp32PhotoAndClose(img);
    });
    connect(btnClose, &QPushButton::clicked, dlg, [liveTimer, dlg]() {
        liveTimer->stop();
        dlg->reject();
    });
    dlg->exec();
}

void MainWindow::on_btn_delete_photo_clicked() {
    if (!ui->label_photo) return;
    ui->label_photo->clear();
    ui->label_photo->setText("Photo Placeholder");
    ui->label_photo->setProperty("photoPath", QString());
    const QString cin = currentEmployeeCin();
    if (!cin.isEmpty() && m_db.isValid() && m_db.isOpen()) {
        QSqlQuery q(m_db);
        q.prepare("UPDATE employe SET photo=NULL WHERE cin=?");
        q.addBindValue(cin);
        q.exec();
    }
}

void MainWindow::refreshPhotoPrivacyControls()
{
    auto upd = [this](QLabel *lbl, QPushButton *btn) {
        if (!btn)
            return;
               if (!lbl) {
            btn->setEnabled(false);
            btn->setText(QStringLiteral("V"));
            return;
        }
        const bool placeholder = (lbl->text() == QStringLiteral("Photo Placeholder"));
        const bool hasPath = !lbl->property("photoPath").toString().trimmed().isEmpty();
        const bool hasPixmap = !lbl->pixmap(Qt::ReturnByValue).isNull();
        const bool hidden = lbl->property("photoPrivacyHidden").toBool();
        const bool hasPhoto = (!placeholder && (hasPath || hasPixmap || hidden));
        if (!hasPhoto) {
            btn->setEnabled(false);
            btn->setText(QStringLiteral("V"));
            return;
        }
        btn->setEnabled(true);
        // V = Voir (photo masquée), C = Cacher (photo visible)
        btn->setText(hidden ? QStringLiteral("V") : QStringLiteral("C"));
    };
    QPushButton *togglePhotoBtn = this->findChild<QPushButton *>(QStringLiteral("btn_toggle_photo_visible"));
    upd(ui->label_photo, togglePhotoBtn);
}

void MainWindow::on_btn_toggle_photo_visible_clicked()
{
    if (!ui->label_photo)
        return;
    if (ui->label_photo->property("photoPrivacyHidden").toBool())
        showPhotoPreview(ui->label_photo);
    else
        hidePhotoPreview(ui->label_photo);
}

void MainWindow::on_tableWidget_2_cellClicked(int row, int column) {
    if (!ui->tableWidget_2 || row < 0) return;
    const int pwdCol = 10;
    const int codeCol = 18;
    if (column == pwdCol) {
        on_employeeTableHeaderSectionClicked(pwdCol);
        return;
    }
    if (column == codeCol) {
        on_employeeTableHeaderSectionClicked(codeCol);
        return;
    }
    auto *item = ui->tableWidget_2->item(row, 0);
    if (!item) return;
    const QString cin = item->data(Qt::UserRole + 1).toString();
    if (cin.isEmpty()) return;

    setCurrentEmployeeCin(cin);
    EmployeeRow e;
    QString dbErr;
    if (!EmployeeCrud::getByCin(m_db, cin, &e, &dbErr)) {
        QMessageBox::warning(this, "Employés", "Erreur chargement:\n" + dbErr);
        return;
    }

    if (ui->lineEdit_cin) ui->lineEdit_cin->setText(e.cin);
    if (ui->lineEdit_fname) ui->lineEdit_fname->setText(e.prenom);
    if (ui->lineEdit_lname) ui->lineEdit_lname->setText(e.nom);
    if (ui->combo_job) { int idx = ui->combo_job->findText(e.occupation); if (idx >= 0) ui->combo_job->setCurrentIndex(idx); }
    if (ui->combo_dept) { int idx = ui->combo_dept->findText(e.departement); if (idx >= 0) ui->combo_dept->setCurrentIndex(idx); }
    if (ui->dateEdit_hire) ui->dateEdit_hire->setDate(e.date_embauche.isValid() ? e.date_embauche : QDate::currentDate());
    setEmployeeStatus(e.statut);
    if (ui->lineEdit_phone) ui->lineEdit_phone->setText(e.telephone);
    if (ui->lineEdit_email) ui->lineEdit_email->setText(e.email);
    if (ui->lineEdit_pwd) ui->lineEdit_pwd->setText(e.motdepasse);
    if (ui->combo_city) { int idx = ui->combo_city->findText(e.ville); if (idx >= 0) ui->combo_city->setCurrentIndex(idx); else ui->combo_city->setCurrentText(e.ville); }
    if (ui->lineEdit_addr) ui->lineEdit_addr->setText(e.adresse);
    if (ui->combo_certs) { int idx = ui->combo_certs->findText(e.certifications); if (idx >= 0) ui->combo_certs->setCurrentIndex(idx); else ui->combo_certs->setCurrentText(e.certifications); }
    if (ui->combo_slevel) { int idx = ui->combo_slevel->findText(e.niveaux); if (idx >= 0) ui->combo_slevel->setCurrentIndex(idx); }
    if (ui->lineEdit_lic) ui->lineEdit_lic->setText(e.num_licence);
    if (ui->dateEdit_exp) ui->dateEdit_exp->setDate(e.date_expiration.isValid() ? e.date_expiration : QDate::currentDate().addYears(1));
    setEmployeeShift(e.quart);
    if (ui->lineEdit_rfid) ui->lineEdit_rfid->setText(e.rfid);
    if (ui->lineEdit_code) ui->lineEdit_code->setText(e.emp_code);
    if (ui->label_photo) {
        if (!e.photo_path.isEmpty())
            setPhotoFromPath(ui->label_photo, e.photo_path);
        else
            setPhotoPlaceholder(ui->label_photo);
    }
}

static QList<QPair<QString, int>> topNWithOthers(const QMap<QString, int> &in, int n, const QString &othersLabel = "Autres") {
    QList<QPair<QString, int>> items;
    for (auto it = in.begin(); it != in.end(); ++it) {
        const QString k = it.key().trimmed().isEmpty() ? "Inconnu" : it.key().trimmed();
        items.append({k, it.value()});
    }
    std::sort(items.begin(), items.end(), [](const auto &a, const auto &b) { return a.second > b.second; });
    QList<QPair<QString, int>> out;
    int others = 0;
    for (int i = 0; i < items.size(); ++i) {
        if (i < n) out.append(items[i]);
        else others += items[i].second;
    }
    if (others > 0) out.append({othersLabel, others});
    return out;
}

static QString abbreviateLabelForLegend(const QString &label) {
    const QString trimmed = label.trimmed();
    if (trimmed.length() <= 12) return trimmed;
    if (trimmed.compare("Ressources Humaines", Qt::CaseInsensitive) == 0) return "R. Humaines";
    if (trimmed.compare("Comptabilité", Qt::CaseInsensitive) == 0) return "Compta.";
    if (trimmed.compare("Administration", Qt::CaseInsensitive) == 0) return "Admin.";
    if (trimmed.compare("Direction", Qt::CaseInsensitive) == 0) return "Dir.";
    if (trimmed.compare("Maintenance", Qt::CaseInsensitive) == 0) return "Maint.";
    if (trimmed.compare("Intermédiaire", Qt::CaseInsensitive) == 0) return "Interm.";
    if (trimmed.compare("Débutant", Qt::CaseInsensitive) == 0) return "Début.";
    if (trimmed.compare("Rotation", Qt::CaseInsensitive) == 0) return "Rot.";
    if (trimmed.compare("Bizerte", Qt::CaseInsensitive) == 0) return "Biz.";
    if (trimmed.compare("Monastir", Qt::CaseInsensitive) == 0) return "Mon.";
    if (trimmed.compare("Sousse", Qt::CaseInsensitive) == 0) return "Sou.";
    if (trimmed.compare("Casablanca", Qt::CaseInsensitive) == 0) return "Casa.";
    if (trimmed.compare("Tanger", Qt::CaseInsensitive) == 0) return "Tang.";
    if (trimmed.compare("Agadir", Qt::CaseInsensitive) == 0) return "Aga.";
    if (trimmed.compare("Gabès", Qt::CaseInsensitive) == 0) return "Gabès";
    if (trimmed.compare("Gabes", Qt::CaseInsensitive) == 0) return "Gabès";
    if (trimmed.compare("Autres", Qt::CaseInsensitive) == 0) return "Autre";
    if (trimmed.length() > 8) return trimmed.left(6) + ".";
    return trimmed;
}

static void applyPieChart(QChartView *view, const QList<QPair<QString, int>> &counts, const QList<QColor> &colors, Qt::Alignment legendAlign) {
    if (!view) return;
    QChart *chart = view->chart();
    chart->removeAllSeries();
    auto *series = new QPieSeries(chart);
    series->setHoleSize(0.35);
    int total = 0;
    for (const auto &p : counts) total += p.second;
    if (total == 0) series->append("Aucune donnée", 1);
    else {
        for (const auto &p : counts) {
            if (p.second <= 0) continue;
            series->append(p.first, p.second);
        }
    }
    chart->addSeries(series);
    chart->setBackgroundVisible(false);
    chart->setPlotAreaBackgroundVisible(false);
    chart->setAnimationOptions(QChart::SeriesAnimations);
    const bool legendBottom = (legendAlign == Qt::AlignBottom);
    const int n = counts.size();
    const bool fewSlices = (n <= 4);
    const int marginLeft = fewSlices ? 10 : 20;
    const int marginTop = fewSlices ? 5 : 10;
    const int marginRight = legendBottom ? (fewSlices ? 10 : 16) : 50;
    int marginBottom = legendBottom ? (fewSlices ? 22 : (n >= 5 ? 44 : 40)) : 24;
    chart->setMargins(QMargins(marginLeft, marginTop, marginRight, marginBottom));
    if (chart->layout()) chart->layout()->setContentsMargins(2, 2, 2, 2);
    chart->legend()->setVisible(true);
    chart->legend()->setAlignment(legendAlign);
    chart->legend()->setBrush(QBrush(QColor("#ffffff")));
    chart->legend()->setLabelColor(QColor("#ffffff"));
    QFont lf = chart->legend()->font();
    lf.setPointSize(qMax(5, legendBottom ? (n >= 5 ? 5 : (n >= 3 ? 6 : 7)) : (n > 4 ? 6 : 8)));
    lf.setBold(true);
    chart->legend()->setFont(lf);
    int idx = 0;
    for (QPieSlice *slice : series->slices()) {
        const QColor segColor = colors.isEmpty() ? QColor::fromHsv((idx * 50) % 360, 160, 220) : colors.at(idx % colors.size());
        slice->setBrush(segColor);
        slice->setBorderColor(QColor(0, 0, 0, 60));
        slice->setBorderWidth(2.0);
        slice->setExploded(false);
        slice->setLabelPosition(QPieSlice::LabelOutside);
        slice->setLabelVisible(true);
        slice->setLabelColor(segColor);
        slice->setLabelFont(QFont("Segoe UI", 9, QFont::DemiBold));
        if (total > 0 && slice->value() > 0 && slice->label().trimmed() != QLatin1String("Aucune donnée")) {
            const int pct = qRound(100.0 * slice->value() / total);
            slice->setLabel(QStringLiteral("%1: %2 (%3%)").arg(abbreviateLabelForLegend(slice->label().trimmed())).arg(int(slice->value())).arg(pct));
        }
        QObject::connect(slice, &QPieSlice::hovered, [slice, segColor](bool state) {
            slice->setExploded(state);
            if (state) {
                slice->setBrush(QBrush(segColor.lighter(115), Qt::SolidPattern));
            } else {
                slice->setBrush(QBrush(segColor, Qt::SolidPattern));
            }
        });
        ++idx;
    }
    series->setLabelsVisible(true);
}

void MainWindow::setupEmployeeStatsCharts() {
    if (ui->frameDepartments && !m_empDeptChartView) {
        m_empDeptChartView = new QChartView(new QChart(), ui->frameDepartments);
        m_empDeptChartView->setGeometry(15, 45, 490, 150);
        m_empDeptChartView->setStyleSheet("background: transparent;");
        m_empDeptChartView->setRenderHint(QPainter::Antialiasing);
    }
    if (ui->frameSkills && !m_empSkillChartView) {
        m_empSkillChartView = new QChartView(new QChart(), ui->frameSkills);
        m_empSkillChartView->setGeometry(15, 45, 490, 150);
        m_empSkillChartView->setStyleSheet("background: transparent;");
        m_empSkillChartView->setRenderHint(QPainter::Antialiasing);
    }
    if (ui->frameShifts && !m_empShiftChartView) {
        m_empShiftChartView = new QChartView(new QChart(), ui->frameShifts);
        m_empShiftChartView->setGeometry(15, 45, 490, 150);
        m_empShiftChartView->setStyleSheet("background: transparent;");
        m_empShiftChartView->setRenderHint(QPainter::Antialiasing);
    }
    if (ui->frameCities && !m_empCityChartView) {
        m_empCityChartView = new QChartView(new QChart(), ui->frameCities);
        m_empCityChartView->setGeometry(15, 45, 490, 150);
        m_empCityChartView->setStyleSheet("background: transparent;");
        m_empCityChartView->setRenderHint(QPainter::Antialiasing);
    }

    if (ui->frameCard1)
        ui->frameCard1->setToolTip(tr("Nombre total d'employés enregistrés dans le système."));
    if (ui->frameCard2)
        ui->frameCard2->setToolTip(tr("Employés avec le statut Actif (en poste)."));
    if (ui->frameCard3)
        ui->frameCard3->setToolTip(tr("Employés actuellement en congé."));
    if (ui->frameCard4)
        ui->frameCard4->setToolTip(tr("Certifications qui expirent dans les 30 prochains jours."));
    if (ui->frameCard5)
        ui->frameCard5->setToolTip(tr("Synthèse des alertes : certifications à renouveler, congés, taux de présence du jour."));

    for (QLabel *lbl : { ui->lblTotalEmployees, ui->lblTotalEmployeesValue, ui->lblActiveEmployees, ui->lblActiveEmployeesValue,
                         ui->lblOnLeave, ui->lblOnLeaveValue, ui->lblCertExpiring, ui->lblCertExpiringValue,
                         ui->lblAlertsKpiTitle, ui->lblAlertsKpiValue,
                         ui->lblAlertsTitle, ui->lblAlert1, ui->lblAlert2, ui->lblAlert3, ui->lblStatsLastUpdate }) {
        if (lbl)
            lbl->setAutoFillBackground(false);
    }

    if (ui->frameDepartments)
        ui->frameDepartments->installEventFilter(this);
    if (ui->frameSkills)
        ui->frameSkills->installEventFilter(this);
    if (ui->frameShifts)
        ui->frameShifts->installEventFilter(this);
    if (ui->frameCities)
        ui->frameCities->installEventFilter(this);

    QEvent ev(QEvent::Resize);
    if (ui->frameDepartments)
        eventFilter(ui->frameDepartments, &ev);
    if (ui->frameSkills)
        eventFilter(ui->frameSkills, &ev);
    if (ui->frameShifts)
        eventFilter(ui->frameShifts, &ev);
    if (ui->frameCities)
        eventFilter(ui->frameCities, &ev);
}

void MainWindow::updateEmployeeStats() {
    if (!m_db.isValid() || !m_db.isOpen()) return;

    const QDate today = QDate::currentDate();
    QMap<QString, int> deptCounts, skillCounts, shiftCounts, cityCounts;
    EmployeeKpiSnapshot snap;
    if (!aggregateEmployeeKpi(m_db, today, &snap, &deptCounts, &skillCounts, &shiftCounts, &cityCounts))
        return;

    const int total = snap.total;
    const int active = snap.active;
    const int onLeave = snap.onLeave;
    const int certExp30 = snap.certExp30;
    const int availableToday = snap.availableToday;

    const int activePct = total > 0 ? qRound(100.0 * active / total) : 0;
    const int onLeavePct = total > 0 ? qRound(100.0 * onLeave / total) : 0;
    const int certExpPct = total > 0 ? qRound(100.0 * certExp30 / total) : 0;
    const int presencePct = total > 0 ? qRound(100.0 * availableToday / total) : 0;

    if (ui->lblTotalEmployeesValue) ui->lblTotalEmployeesValue->setText(QString::number(total));
    if (ui->lblActiveEmployeesValue) ui->lblActiveEmployeesValue->setText(QString("%1 (%2%)").arg(active).arg(activePct));
    if (ui->lblOnLeaveValue) ui->lblOnLeaveValue->setText(QString("%1 (%2%)").arg(onLeave).arg(onLeavePct));
    if (ui->lblCertExpiringValue) ui->lblCertExpiringValue->setText(QString("%1 (%2%)").arg(certExp30).arg(certExpPct));
    if (ui->lblAlertsKpiValue) {
        ui->lblAlertsKpiValue->setText(
            QStringLiteral("Certif. %1 · Congés %2\nPrésence %3% (%4/%5)")
                .arg(certExp30)
                .arg(onLeave)
                .arg(presencePct)
                .arg(availableToday)
                .arg(total));
    }
    if (ui->lblAlert1) ui->lblAlert1->setText((certExp30 <= 1) ? QString("• %1 certification expire dans 30 jours (%2% des employés)").arg(certExp30).arg(certExpPct) : QString("• %1 certifications expirent dans 30 jours (%2% des employés)").arg(certExp30).arg(certExpPct));
    if (ui->lblAlert2) ui->lblAlert2->setText((onLeave <= 1) ? QString("• %1 employé en congé (%2% du total)").arg(onLeave).arg(onLeavePct) : QString("• %1 employés en congé (%2% du total)").arg(onLeave).arg(onLeavePct));
    if (ui->lblAlert3) ui->lblAlert3->setText(QString("• Taux de présence (aujourd'hui): %1% (%2/%3)").arg(presencePct).arg(availableToday).arg(total));
    if (ui->lblStatsLastUpdate) ui->lblStatsLastUpdate->setText(tr("Dernière MAJ: %1").arg(QLocale(QLocale::French).toString(QDateTime::currentDateTime(), QLocale::ShortFormat)));

    const QList<QColor> paletteDept = { QColor("#26A69A"), QColor("#42A5F5"), QColor("#66BB6A"), QColor("#FFA726"), QColor("#EF5350"), QColor("#AB47BC"), QColor("#78909C") };
    const QList<QColor> paletteSkill = { QColor("#00897B"), QColor("#7CB342"), QColor("#FB8C00"), QColor("#EC407A"), QColor("#8E24AA") };
    const QList<QColor> paletteShift = { QColor("#FFCA28"), QColor("#26C6DA"), QColor("#66BB6A"), QColor("#FF7043"), QColor("#8E24AA") };
    const QList<QColor> paletteCity = { QColor("#FF7043"), QColor("#26C6DA"), QColor("#66BB6A"), QColor("#FFCA28"), QColor("#E53935") };
    applyPieChart(m_empDeptChartView, topNWithOthers(deptCounts, 6), paletteDept, Qt::AlignRight);
    applyPieChart(m_empSkillChartView, topNWithOthers(skillCounts, 4), paletteSkill, Qt::AlignRight);
    applyPieChart(m_empShiftChartView, topNWithOthers(shiftCounts, 4), paletteShift, Qt::AlignRight);
    applyPieChart(m_empCityChartView, topNWithOthers(cityCounts, 5), paletteCity, Qt::AlignRight);

    m_empKpiCache = snap;
    m_empKpiCacheTime = QDateTime::currentDateTime();
}

bool MainWindow::eventFilter(QObject *watched, QEvent *event)
{
    if (ui && ui->lineEdit_rfid && watched == ui->lineEdit_rfid && event) {
        if (event->type() == QEvent::FocusIn)
            pushRfidArduinoDoorPolicy(true);
        else if (event->type() == QEvent::FocusOut)
            pushRfidArduinoDoorPolicy(true);
    }
    if (ui && ui->le_rfid_2 && watched == ui->le_rfid_2 && event) {
        if (event->type() == QEvent::FocusIn)
            pushRfidArduinoDoorPolicy(true);
        else if (event->type() == QEvent::FocusOut)
            pushRfidArduinoDoorPolicy(true);
    }
    if (ui && ui->textEditAlertes && watched == ui->textEditAlertes->viewport() && event) {
        if (event->type() == QEvent::MouseButtonRelease) {
            const auto *me = static_cast<const QMouseEvent *>(event);
            if (me->button() == Qt::LeftButton) {
                refreshQuaiSupervisionData();
                return true;
            }
        }
    }
    if (ui && ui->tabWidgetStockVentes && watched == ui->tabWidgetStockVentes) {
        if (event && (event->type() == QEvent::Resize || event->type() == QEvent::Show)) {
            positionStockChatbotOverlay();
        }
    }
    if (event && event->type() == QEvent::Resize) {
        auto resizeEmpChart = [](QFrame *frame, QChartView *view) {
            if (!frame || !view)
                return;
            const int left = 10;
            const int top = 44;
            const int right = 12;
            const int bottom = 20;
            const QRect r = frame->rect();
            view->setGeometry(left, top, qMax(20, r.width() - left - right), qMax(20, r.height() - top - bottom));
        };
        if (ui) {
            if (watched == ui->frameDepartments)
                resizeEmpChart(ui->frameDepartments, m_empDeptChartView);
            else if (watched == ui->frameSkills)
                resizeEmpChart(ui->frameSkills, m_empSkillChartView);
            else if (watched == ui->frameShifts)
                resizeEmpChart(ui->frameShifts, m_empShiftChartView);
            else if (watched == ui->frameCities)
                resizeEmpChart(ui->frameCities, m_empCityChartView);
        }
    }
    return QMainWindow::eventFilter(watched, event);
}

void MainWindow::connectRfidReadyReadSignal()
{
    if (QSerialPort *sp = m_rfidArduino.getserial()) {
        QObject::connect(
            sp,
            &QSerialPort::readyRead,
            this,
            &MainWindow::on_rfid_serial_ready,
            static_cast<Qt::ConnectionType>(Qt::DirectConnection | Qt::UniqueConnection));
    }
    startRfidSerialPolling();
}

void MainWindow::connectRfidPecheurReadyReadSignal()
{
    if (QSerialPort *sp = m_rfidPecheurArduino.getserial()) {
        QObject::connect(
            sp,
            &QSerialPort::readyRead,
            this,
            &MainWindow::on_rfid_pecheur_serial_ready,
            static_cast<Qt::ConnectionType>(Qt::DirectConnection | Qt::UniqueConnection));
    }
    if (!m_rfidPecheurPollTimer) {
        m_rfidPecheurPollTimer = new QTimer(this);
        m_rfidPecheurPollTimer->setInterval(50);
        connect(m_rfidPecheurPollTimer, &QTimer::timeout, this, [this]() {
            if (!m_rfidPecheurArduino.getserial() || !m_rfidPecheurArduino.getserial()->isOpen())
                return;
            if (m_rfidPecheurArduino.getserial()->bytesAvailable() <= 0)
                return;
            appendAndParseRfidPecheurChunk(m_rfidPecheurArduino.read_from_arduino());
        });
    }
    m_rfidPecheurPollTimer->start();
}

void MainWindow::startRfidSerialPolling()
{
    if (!m_rfidPollTimer) {
        m_rfidPollTimer = new QTimer(this);
        m_rfidPollTimer->setInterval(50);
        connect(m_rfidPollTimer, &QTimer::timeout, this, [this]() {
            if (!m_rfidArduino.getserial() || !m_rfidArduino.getserial()->isOpen())
                return;
            if (m_rfidArduino.getserial()->bytesAvailable() <= 0)
                return;
            appendAndParseRfidChunk(m_rfidArduino.read_from_arduino());
        });
    }
    m_rfidPollTimer->start();
}

void MainWindow::stopRfidSerialPolling()
{
    if (m_rfidPollTimer)
        m_rfidPollTimer->stop();
}

void MainWindow::syncQuaiGateArduinoFromDatabase()
{
    if (!m_quaiGateArduino.getserial() || !m_quaiGateArduino.getserial()->isOpen())
        return;
    QSqlDatabase db = QSqlDatabase::database(Connection::connectionName());
    if (!db.isOpen())
        return;
    const bool allow = Quai::hasAnyActiveQuai();
    m_quaiGateArduino.write_to_arduino(allow ? QByteArrayLiteral("DB_ALLOW_OPEN\n")
                                             : QByteArrayLiteral("DB_BLOCK_OPEN\n"));
    const int st = allow ? 1 : 0;
    if (m_quaiArduinoWindow && m_quaiGateLastAllowLogState != st) {
        m_quaiGateLastAllowLogState = st;
        m_quaiArduinoWindow->appendQuaiGateLog(allow
            ? tr("Politique porte : capteur autorisé (DB_ALLOW_OPEN).")
            : tr("Politique porte : capteur bloqué (DB_BLOCK_OPEN)."));
    }
}

void MainWindow::setupQuaiGateSerialNotifier()
{
    QSerialPort *sp = m_quaiGateArduino.getserial();
    if (!sp || !sp->isOpen())
        return;
    connect(sp, &QSerialPort::readyRead, this, &MainWindow::onQuaiGateSerialReadyRead, Qt::UniqueConnection);
}

void MainWindow::onQuaiGateSerialReadyRead()
{
    m_quaiGateSerialRxBuf.append(m_quaiGateArduino.read_from_arduino());
    while (true) {
        int n = m_quaiGateSerialRxBuf.indexOf('\n');
        if (n < 0)
            n = m_quaiGateSerialRxBuf.indexOf('\r');
        if (n < 0)
            break;
        const QByteArray line = m_quaiGateSerialRxBuf.left(n).trimmed();
        m_quaiGateSerialRxBuf.remove(0, n + 1);
        if (line.isEmpty())
            continue;
        if (m_quaiArduinoWindow)
            m_quaiArduinoWindow->appendQuaiGateLog(tr("← %1").arg(QString::fromUtf8(line)));
        if (line == "SENSOR:TRIG" || line == "SENSOR:ENTER") {
            QTimer::singleShot(0, this, [this]() { showQuaiSensorTriggerPickDialog(); });
        } else if (line == "SENSOR:LEAVE") {
            QTimer::singleShot(0, this, [this]() { showQuaiLeaveSensorPickDialog(); });
        }
    }
    if (m_quaiGateSerialRxBuf.size() > 2048)
        m_quaiGateSerialRxBuf.clear();
}

void MainWindow::showQuaiSensorTriggerPickDialog()
{
    if (m_quaiSensorPickDialogOpen)
        return;
    m_quaiSensorPickDialogOpen = true;

    QSqlDatabase db = QSqlDatabase::database(Connection::connectionName());
    if (!db.isOpen()) {
        if (m_quaiArduinoWindow)
            m_quaiArduinoWindow->appendQuaiGateLog(tr("Entrée : annulation — base indisponible."));
        QMessageBox::warning(this, tr("Entrée — capteur"), tr("Base de données non disponible."));
        m_quaiSensorPickDialogOpen = false;
        return;
    }

    const QVector<QPair<int, QString>> rows = Quai::activeQuaisIdAndLabel();
    if (rows.isEmpty()) {
        if (m_quaiArduinoWindow)
            m_quaiArduinoWindow->appendQuaiGateLog(tr("Entrée : aucun quai actif — ouverture impossible."));
        QMessageBox::information(this, tr("Entrée — capteur"),
                                 tr("Aucun quai actif : impossible d’ouvrir la porte depuis le capteur."));
        m_quaiSensorPickDialogOpen = false;
        return;
    }

    if (m_quaiArduinoWindow)
        m_quaiArduinoWindow->appendQuaiGateLog(tr("Entrée : choix du quai à occuper…"));

    QStringList labels;
    QVector<int> ids;
    labels.reserve(rows.size());
    ids.reserve(rows.size());
    for (const auto &p : rows) {
        labels.append(p.second);
        ids.append(p.first);
    }

    bool ok = false;
    const QString choice = QInputDialog::getItem(this, tr("Entrée — capteur"),
                                                 tr("Quel quai actif occupez-vous ?"),
                                                 labels, 0, false, &ok);
    if (!ok || choice.isEmpty()) {
        if (m_quaiArduinoWindow)
            m_quaiArduinoWindow->appendQuaiGateLog(tr("Entrée : choix annulé."));
        m_quaiSensorPickDialogOpen = false;
        return;
    }

    const int idx = labels.indexOf(choice);
    if (idx < 0 || idx >= ids.size()) {
        m_quaiSensorPickDialogOpen = false;
        return;
    }
    const int quaiId = ids.at(idx);
    if (!Quai::setStatutForQuaiId(quaiId, QStringLiteral("Inactif"))) {
        if (m_quaiArduinoWindow)
            m_quaiArduinoWindow->appendQuaiGateLog(tr("Entrée : échec mise à jour quai %1.").arg(quaiId));
        QMessageBox::warning(this, tr("Entrée — capteur"),
                             tr("Échec de la mise à jour : %1").arg(Quai::lastErrorMessage()));
        m_quaiSensorPickDialogOpen = false;
        return;
    }

    if (m_quaiArduinoWindow)
        m_quaiArduinoWindow->appendQuaiGateLog(tr("Entrée : quai %1 → Inactif ; envoi PORTE_OPEN.").arg(quaiId));

    loadQuaisTable();
    refreshQuaiStats();
    refreshQuaiCharts();
    syncQuaiGateArduinoFromDatabase();
    if (m_quaiArduinoWindow)
        m_quaiArduinoWindow->refreshActiveQuaisCombo();

    if (m_quaiGateArduino.getserial() && m_quaiGateArduino.getserial()->isOpen())
        m_quaiGateArduino.write_to_arduino(QByteArrayLiteral("PORTE_OPEN\n"));

    m_quaiSensorPickDialogOpen = false;
}

void MainWindow::showQuaiLeaveSensorPickDialog()
{
    if (m_quaiSensorPickDialogOpen)
        return;
    m_quaiSensorPickDialogOpen = true;

    QSqlDatabase db = QSqlDatabase::database(Connection::connectionName());
    if (!db.isOpen()) {
        if (m_quaiArduinoWindow)
            m_quaiArduinoWindow->appendQuaiGateLog(tr("Sortie : annulation — base indisponible."));
        QMessageBox::warning(this, tr("Sortie — capteur"), tr("Base de données non disponible."));
        m_quaiSensorPickDialogOpen = false;
        return;
    }

    const QVector<QPair<int, QString>> rows = Quai::occupiedQuaisIdAndLabel();
    if (rows.isEmpty()) {
        if (m_quaiArduinoWindow)
            m_quaiArduinoWindow->appendQuaiGateLog(tr("Sortie : aucun quai occupé (inactif) à libérer."));
        QMessageBox::information(this, tr("Sortie — capteur"),
                                 tr("Aucun quai inactif (occupé) à libérer — ouverture impossible."));
        m_quaiSensorPickDialogOpen = false;
        return;
    }

    if (m_quaiArduinoWindow)
        m_quaiArduinoWindow->appendQuaiGateLog(tr("Sortie : quel quai libérez-vous ?"));

    QStringList labels;
    QVector<int> ids;
    labels.reserve(rows.size());
    ids.reserve(rows.size());
    for (const auto &p : rows) {
        labels.append(p.second);
        ids.append(p.first);
    }

    bool ok = false;
    const QString choice = QInputDialog::getItem(this, tr("Sortie — capteur"),
                                                 tr("Quel quai occupé (inactif) libérez-vous ?"),
                                                 labels, 0, false, &ok);
    if (!ok || choice.isEmpty()) {
        if (m_quaiArduinoWindow)
            m_quaiArduinoWindow->appendQuaiGateLog(tr("Sortie : choix annulé."));
        m_quaiSensorPickDialogOpen = false;
        return;
    }

    const int idx = labels.indexOf(choice);
    if (idx < 0 || idx >= ids.size()) {
        m_quaiSensorPickDialogOpen = false;
        return;
    }
    const int quaiId = ids.at(idx);
    if (!Quai::setStatutForQuaiId(quaiId, QStringLiteral("Actif"))) {
        if (m_quaiArduinoWindow)
            m_quaiArduinoWindow->appendQuaiGateLog(tr("Sortie : échec mise à jour quai %1.").arg(quaiId));
        QMessageBox::warning(this, tr("Sortie — capteur"),
                             tr("Échec de la mise à jour : %1").arg(Quai::lastErrorMessage()));
        m_quaiSensorPickDialogOpen = false;
        return;
    }

    if (m_quaiArduinoWindow)
        m_quaiArduinoWindow->appendQuaiGateLog(tr("Sortie : quai %1 → Actif ; envoi PORTE_OPEN.").arg(quaiId));

    loadQuaisTable();
    refreshQuaiStats();
    refreshQuaiCharts();
    syncQuaiGateArduinoFromDatabase();
    if (m_quaiArduinoWindow)
        m_quaiArduinoWindow->refreshActiveQuaisCombo();

    if (m_quaiGateArduino.getserial() && m_quaiGateArduino.getserial()->isOpen())
        m_quaiGateArduino.write_to_arduino(QByteArrayLiteral("PORTE_OPEN\n"));

    m_quaiSensorPickDialogOpen = false;
}

void MainWindow::pushRfidArduinoDoorPolicy(bool captureUidOnly)
{
    if (!m_rfidArduino.getserial() || !m_rfidArduino.getserial()->isOpen())
        return;
    m_rfidArduino.write_to_arduino(captureUidOnly ? QByteArrayLiteral("RFID_CAPTURE\n")
                                                  : QByteArrayLiteral("RFID_DOOR\n"));
    if (captureUidOnly)
        pushRfidArduinoDoorHardwareMode();
}

void MainWindow::pushRfidArduinoDoorHardwareMode()
{
    if (!m_rfidArduino.getserial() || !m_rfidArduino.getserial()->isOpen())
        return;
    QSettings settings(QStringLiteral("ATLAS"), QStringLiteral("ATLAS"));
    const QString mode = settings.value(QStringLiteral("smartPort/doorLogic"), QStringLiteral("rfidOpens")).toString();
    if (mode == QStringLiteral("touchOpens"))
        m_rfidArduino.write_to_arduino(QByteArrayLiteral("DOOR_TOUCH_OPENS\n"));
    else
        m_rfidArduino.write_to_arduino(QByteArrayLiteral("DOOR_RFID_OPENS\n"));
}

void MainWindow::applyPorteUiState(bool open)
{
    if (!ui->sliderSmartPortActDoor || !ui->lblSmartPortActDoorVal)
        return;
    const QSignalBlocker blocker(ui->sliderSmartPortActDoor);
    ui->sliderSmartPortActDoor->setValue(open ? 1 : 0);
    ui->lblSmartPortActDoorVal->setText(open ? tr("On") : tr("Off"));
}

void MainWindow::pushPorteCommandToArduino(bool open)
{
    if (!m_rfidArduino.getserial() || !m_rfidArduino.getserial()->isOpen())
        return;
    bool shouldOpen = open;
    const bool hasActiveQuai = Quai::hasAnyActiveQuai();
    if (open && !hasActiveQuai) {
        shouldOpen = false;
        qDebug() << "Gate blocked: no active quai in database.";
        applyPorteUiState(false);
    }
    m_rfidArduino.write_to_arduino(shouldOpen ? QByteArrayLiteral("PORTE_OPEN\n")
                                              : QByteArrayLiteral("PORTE_CLOSE\n"));
}

void MainWindow::pushLcdLineToArduino(int row, const QString &text)
{
    if (row < 0 || row > 3)
        return;
    if (!m_rfidArduino.getserial() || !m_rfidArduino.getserial()->isOpen())
        return;
    QString t = text;
    t.replace(QLatin1Char('\r'), QLatin1Char(' '));
    t.replace(QLatin1Char('\n'), QLatin1Char(' '));
    QByteArray latin = t.toLatin1();
    if (latin.size() > 20)
        latin = latin.left(20);
    QByteArray cmd = QByteArrayLiteral("L") + QByteArray(1, char('0' + row)) + QByteArrayLiteral(":") + latin + QByteArrayLiteral("\n");
    m_rfidArduino.write_to_arduino(cmd);
}

void MainWindow::pushLcdHardwareClearToArduino()
{
    if (!m_rfidArduino.getserial() || !m_rfidArduino.getserial()->isOpen())
        return;
    m_rfidArduino.write_to_arduino(QByteArrayLiteral("LCDCLR\n"));
}

void MainWindow::sendLcdForRfidGate(bool accessGranted, const EmployeeRow &e, const QByteArray &uidAscii, bool fromKeypad)
{
    if (!m_rfidArduino.getserial() || !m_rfidArduino.getserial()->isOpen())
        return;
    pushLcdHardwareClearToArduino();
    const QString uid = QString::fromLatin1(uidAscii);
    const QString lineRead = fromKeypad ? QStringLiteral("LECTURE CODE") : QStringLiteral("LECTURE BADGE");
    if (accessGranted) {
        /* Lignes 20 car. max (LCD) — ordre : lecture, NOM Prenom, Shift, Bienvenue + porte. */
        const QString nomPrenom = (e.nom.trimmed() + QLatin1Char(' ') + e.prenom.trimmed()).trimmed();
        QString shift = e.quart.trimmed();
        if (shift.isEmpty())
            shift = QStringLiteral("--");
        pushLcdLineToArduino(0, lineRead);
        pushLcdLineToArduino(1, nomPrenom.isEmpty() ? QStringLiteral("Employe") : nomPrenom);
        pushLcdLineToArduino(2, QStringLiteral("Shift: %1").arg(shift.left(12)));
        pushLcdLineToArduino(3, QStringLiteral("Bienvenue! P.OUVERT"));
    } else {
        pushLcdLineToArduino(0, lineRead);
        pushLcdLineToArduino(1, fromKeypad ? QStringLiteral("Code inconnu") : QStringLiteral("Badge inconnu"));
        pushLcdLineToArduino(2, fromKeypad ? QStringLiteral("------") : uid.left(20));
        pushLcdLineToArduino(3, QStringLiteral("Acces refuse"));
    }
}

void MainWindow::showTemporarySmartPortBanner(bool accessGranted, const EmployeeRow &employee, const QString &uidDisplay)
{
    if (!ui->lblSmartPortBanner)
        return;
    if (m_smartPortBannerResetTimer == nullptr) {
        m_smartPortBannerResetTimer = new QTimer(this);
        m_smartPortBannerResetTimer->setSingleShot(true);
    } else {
        m_smartPortBannerResetTimer->stop();
    }
    QObject::disconnect(m_smartPortBannerResetTimer, &QTimer::timeout, nullptr, nullptr);
    QString msg;
    if (accessGranted) {
        const QString p = employee.prenom.trimmed();
        const QString n = employee.nom.trimmed();
        if (!p.isEmpty() || !n.isEmpty())
            msg = tr("Accès autorisé — %1 %2").arg(p, n).trimmed();
        else
            msg = tr("Accès autorisé");
    } else {
        msg = tr("Accès refusé — badge inconnu (%1)").arg(uidDisplay.trimmed().left(14));
    }
    ui->lblSmartPortBanner->setText(msg);
    connect(m_smartPortBannerResetTimer, &QTimer::timeout, this, [this]() {
        if (ui->lblSmartPortBanner)
            ui->lblSmartPortBanner->setText(m_smartPortBannerDefaultText);
    });
    m_smartPortBannerResetTimer->start(10000);
}

void MainWindow::sendLcdForPortSortie(const EmployeeRow &e)
{
    if (!m_rfidArduino.getserial() || !m_rfidArduino.getserial()->isOpen())
        return;
    pushLcdHardwareClearToArduino();
    const QString nomPrenom = (e.nom.trimmed() + QLatin1Char(' ') + e.prenom.trimmed()).trimmed();
    pushLcdLineToArduino(0, QStringLiteral("   SORTIE PORT"));
    pushLcdLineToArduino(1, nomPrenom.isEmpty() ? QStringLiteral("Employe") : nomPrenom);
    pushLcdLineToArduino(2, QStringLiteral("Sortie enregistree"));
    pushLcdLineToArduino(3, QStringLiteral(" Bonne journee !   "));
}

void MainWindow::sendLcdForPecheurRfid(bool found, const PecheurRfidRecord &pecheur, const QByteArray &uidAscii)
{
    if (!m_rfidPecheurArduino.getserial() || !m_rfidPecheurArduino.getserial()->isOpen())
        return;
    m_rfidPecheurArduino.write_to_arduino(QByteArrayLiteral("LCDCLR\n"));
    auto writeLcdPecheur = [this](int row, const QString &text) {
        if (row < 0 || row > 3)
            return;
        QString t = text;
        t.replace(QLatin1Char('\r'), QLatin1Char(' '));
        t.replace(QLatin1Char('\n'), QLatin1Char(' '));
        QByteArray latin = t.toLatin1();
        if (latin.size() > 20)
            latin = latin.left(20);
        QByteArray cmd = QByteArrayLiteral("L") + QByteArray(1, char('0' + row)) + QByteArrayLiteral(":") + latin + QByteArrayLiteral("\n");
        m_rfidPecheurArduino.write_to_arduino(cmd);
    };
    if (found) {
        const QString nomPrenom = (pecheur.nom + QLatin1Char(' ') + pecheur.prenom).trimmed();
        auto sanitize = [](QString s) {
            s.replace(QLatin1Char('|'), QLatin1Char(' '));
            s.replace(QLatin1Char('\r'), QLatin1Char(' '));
            s.replace(QLatin1Char('\n'), QLatin1Char(' '));
            return s.trimmed();
        };
        const QString nom = sanitize(pecheur.nom);
        const QString prenom = sanitize(pecheur.prenom);
        const QString role = sanitize(pecheur.role);
        const QString statut = sanitize(pecheur.statut);
        const QByteArray pechCmd = QStringLiteral("PECH:%1|%2|%3|%4\n")
            .arg(nom, prenom, role, statut)
            .toUtf8();
        m_rfidPecheurArduino.write_to_arduino(pechCmd);
        writeLcdPecheur(0, nomPrenom.isEmpty() ? QStringLiteral("Nom inconnu") : nomPrenom);
        writeLcdPecheur(1, pecheur.role.isEmpty() ? QStringLiteral("Role inconnu") : pecheur.role);
        return;
    }
    m_rfidPecheurArduino.write_to_arduino(QByteArrayLiteral("STATUT:INACTIF\n"));
    writeLcdPecheur(0, QStringLiteral("Badge inconnu"));
    writeLcdPecheur(1, QString::fromLatin1(uidAscii).left(20));
}

void MainWindow::fillPecheurFormFromRfid(const PecheurRfidRecord &pecheur)
{
    if (ui->le_nom_2)
        ui->le_nom_2->setText(pecheur.nom);
    if (ui->le_prenom_2)
        ui->le_prenom_2->setText(pecheur.prenom);
    if (ui->cb_role_2) {
        const int roleIndex = ui->cb_role_2->findText(pecheur.role, Qt::MatchFixedString);
        if (roleIndex >= 0)
            ui->cb_role_2->setCurrentIndex(roleIndex);
        else if (!pecheur.role.trimmed().isEmpty())
            ui->cb_role_2->setCurrentText(pecheur.role.trimmed());
    }
    if (ui->sb_experience_2)
        ui->sb_experience_2->setValue(qMax(0, pecheur.experience));
    if (ui->radio_status_actif_2 && ui->radio_status_inactif_2) {
        if (pecheur.statut.compare(QStringLiteral("Inactif"), Qt::CaseInsensitive) == 0)
            ui->radio_status_inactif_2->setChecked(true);
        else
            ui->radio_status_actif_2->setChecked(true);
    }
    if (ui->dateEdit_limit_2 && pecheur.dateLimite.isValid())
        ui->dateEdit_limit_2->setDate(pecheur.dateLimite);
    if (ui->le_telephone_2)
        ui->le_telephone_2->setText(pecheur.telephone);
    if (ui->le_rfid_2)
        ui->le_rfid_2->setText(pecheur.rfid);
}

void MainWindow::refreshSmartPortPresenceUi()
{
    const QDate d = m_smartPortSelectedDate.isValid() ? m_smartPortSelectedDate : QDate::currentDate();
    refreshSmartPortEmployeeTable(d);
}

void MainWindow::appendAndParseRfidChunk(const QByteArray &chunk)
{
    if (chunk.isEmpty())
        return;
    m_rfidSerialBuffer.append(chunk);
    if (m_rfidSerialBuffer.size() > 1024) {
        const int cut = m_rfidSerialBuffer.size() - 512;
        m_rfidSerialBuffer.remove(0, cut);
    }
    while (true) {
        int n = m_rfidSerialBuffer.indexOf('\n');
        if (n < 0)
            n = m_rfidSerialBuffer.indexOf('\r');
        if (n < 0)
            break;
        const QByteArray rawLine = m_rfidSerialBuffer.left(n).trimmed();
        m_rfidSerialBuffer.remove(0, n + 1);
        if (rawLine.isEmpty())
            continue;
        if (rawLine.toUpper().startsWith("PORTE:")) {
            const QByteArray up = rawLine.toUpper();
            if (up.contains("OUVERTE")) {
                applyPorteUiState(true);
                bool touchExit = false;
                bool pcOrRfidOpen = false;
                for (const QByteArray &seg : up.split(';')) {
                    const QByteArray t = seg.trimmed();
                    if (t == "SRC=T" || t == "SRC=2")
                        touchExit = true;
                    if (t == "SRC=P" || t == "SRC=R")
                        pcOrRfidOpen = true;
                }
                if (touchExit)
                    m_smartPortAwaitRfidForSortie = true;
                else if (pcOrRfidOpen)
                    m_smartPortAwaitRfidForSortie = false;
            } else if (up.contains("FERMEE")) {
                applyPorteUiState(false);
                m_smartPortAwaitRfidForSortie = false;
            }
            continue;
        }
        QByteArray cred = parseRfidSerialLineToUid(rawLine);
        bool fromKeypad = false;
        if (cred.isEmpty()) {
            cred = parseKeypadSerialLineToCode(rawLine);
            fromKeypad = !cred.isEmpty();
        }
        if (cred.isEmpty())
            continue;
        if (ui->lineEdit_rfid && ui->lineEdit_rfid->hasFocus() && !fromKeypad) {
            ui->lineEdit_rfid->setText(QString::fromUtf8(cred));
            continue;
        }
        if (ui->lineEdit_code && ui->lineEdit_code->hasFocus() && fromKeypad) {
            ui->lineEdit_code->setText(QString::fromUtf8(cred));
            continue;
        }
        const bool employeFormVisible = ui->stackedWidget
            && ui->stackedWidget->currentWidget() == ui->pageEmployes
            && ui->stackedWidgetEmployes
            && ui->stackedWidgetEmployes->currentIndex() == 0;
        if (employeFormVisible) {
            if (!fromKeypad && ui->lineEdit_rfid)
                ui->lineEdit_rfid->setText(QString::fromUtf8(cred));
            if (fromKeypad && ui->lineEdit_code)
                ui->lineEdit_code->setText(QString::fromUtf8(cred));
            continue;
        }
        const qint64 uidEvtMs = QDateTime::currentMSecsSinceEpoch();
        if (cred == m_rfidLastSerialUid && (uidEvtMs - m_rfidLastSerialUidMs) < 1200) {
            /* Même badge / même code, double ligne série : ne pas enchaîner sortie puis entrée. */
            continue;
        }
        QSqlDatabase dbGate = m_db;
        if (!dbGate.isValid() || !dbGate.isOpen())
            dbGate = QSqlDatabase::database(Connection::connectionName());

        if (m_smartPortAwaitRfidForSortie) {
            m_smartPortAwaitRfidForSortie = false;
            EmployeeRow empEx;
            const bool ok = dbGate.isValid() && dbGate.isOpen()
                && EmployeeCrud::getByRfid(dbGate, QString::fromLatin1(cred), &empEx, nullptr);
            if (ok) {
                QString errTs;
                if (!EmployeeCrud::recordSmartPortSortie(dbGate, empEx.cin, &errTs)) {
                    if (!errTs.isEmpty())
                        qDebug() << "recordSmartPortSortie:" << errTs;
                }
                sendLcdForPortSortie(empEx);
                showTemporarySmartPortBanner(true, empEx, QString());
            } else {
                pushPorteCommandToArduino(false);
                sendLcdForRfidGate(false, EmployeeRow{}, cred, fromKeypad);
                showTemporarySmartPortBanner(false, EmployeeRow{}, fromKeypad ? tr("code inconnu") : QString::fromLatin1(cred));
            }
            refreshSmartPortPresenceUi();
            m_rfidLastSerialUid = cred;
            m_rfidLastSerialUidMs = uidEvtMs;
            continue;
        }

        EmployeeRow empGate;
        const bool known = dbGate.isValid() && dbGate.isOpen()
            && EmployeeCrud::getByRfid(dbGate, QString::fromLatin1(cred), &empGate, nullptr);
        if (known) {
            QString errEn;
            if (!EmployeeCrud::recordSmartPortEntree(dbGate, empGate.cin, &errEn)) {
                if (!errEn.isEmpty())
                    qDebug() << "recordSmartPortEntree:" << errEn;
            }
            pushPorteCommandToArduino(true);
            sendLcdForRfidGate(true, empGate, cred, fromKeypad);
            showTemporarySmartPortBanner(true, empGate, QString());
            refreshSmartPortPresenceUi();
        } else {
            pushPorteCommandToArduino(false);
            sendLcdForRfidGate(false, EmployeeRow{}, cred, fromKeypad);
            showTemporarySmartPortBanner(false, EmployeeRow{}, fromKeypad ? tr("code inconnu") : QString::fromLatin1(cred));
        }
        m_rfidLastSerialUid = cred;
        m_rfidLastSerialUidMs = uidEvtMs;
    }
}

void MainWindow::appendAndParseRfidPecheurChunk(const QByteArray &chunk)
{
    if (chunk.isEmpty())
        return;
    m_rfidPecheurSerialBuffer.append(chunk);
    if (m_rfidPecheurSerialBuffer.size() > 1024) {
        const int cut = m_rfidPecheurSerialBuffer.size() - 512;
        m_rfidPecheurSerialBuffer.remove(0, cut);
    }
    while (true) {
        int n = m_rfidPecheurSerialBuffer.indexOf('\n');
        if (n < 0)
            n = m_rfidPecheurSerialBuffer.indexOf('\r');
        if (n < 0)
            break;
        const QByteArray rawLine = m_rfidPecheurSerialBuffer.left(n).trimmed();
        m_rfidPecheurSerialBuffer.remove(0, n + 1);
        if (rawLine.isEmpty())
            continue;
        const QByteArray cred = parseRfidSerialLineToUid(rawLine);
        if (cred.isEmpty())
            continue;

        const bool pecheurFormVisible = ui->stackedWidget
            && ui->stackedWidget->currentWidget() == ui->pagePecheurs
            && ui->stackedWidgetPecheurs
            && ui->stackedWidgetPecheurs->currentIndex() == 0;
        if (!(ui->le_rfid_2 && (ui->le_rfid_2->hasFocus() || pecheurFormVisible)))
            continue;

        const QString uid = RfidPecheur::normalizeUid(QString::fromLatin1(cred));
        ui->le_rfid_2->setText(uid);
        QSqlDatabase dbPecheur = m_db;
        if (!dbPecheur.isValid() || !dbPecheur.isOpen())
            dbPecheur = QSqlDatabase::database(Connection::connectionName());
        PecheurRfidRecord pecheur;
        QString errRfid;
        const bool foundPecheur = dbPecheur.isValid() && dbPecheur.isOpen()
            && RfidPecheur::lookupByRfid(dbPecheur, uid, &pecheur, &errRfid);
        if (foundPecheur) {
            fillPecheurFormFromRfid(pecheur);
        } else {
            if (ui->le_nom_2) ui->le_nom_2->clear();
            if (ui->le_prenom_2) ui->le_prenom_2->clear();
            if (ui->cb_role_2) ui->cb_role_2->setCurrentIndex(0);
            if (ui->sb_experience_2) ui->sb_experience_2->setValue(0);
            if (ui->radio_status_actif_2) ui->radio_status_actif_2->setChecked(true);
            if (ui->dateEdit_limit_2) ui->dateEdit_limit_2->setDate(QDate::currentDate());
            if (ui->le_telephone_2) ui->le_telephone_2->clear();
            // Keep scanned RFID visible for immediate add.
            if (ui->le_rfid_2) ui->le_rfid_2->setText(uid);
        }
        sendLcdForPecheurRfid(foundPecheur, pecheur, cred);
    }
}

void MainWindow::on_rfid_serial_ready()
{
    appendAndParseRfidChunk(m_rfidArduino.read_from_arduino());
}
void MainWindow::on_rfid_pecheur_serial_ready()
{
    appendAndParseRfidPecheurChunk(m_rfidPecheurArduino.read_from_arduino());
}
void MainWindow::setupSmartPortSelectors()
{
    if (!ui->comboSmartPortMonth || !ui->comboSmartPortYear)
        return;
    ui->comboSmartPortMonth->clear();
    const QLocale fr(QLocale::French);
    for (int m = 1; m <= 12; ++m)
        ui->comboSmartPortMonth->addItem(fr.standaloneMonthName(m, QLocale::LongFormat), m);
    ui->comboSmartPortYear->clear();
    const int y0 = QDate::currentDate().year();
    for (int y = y0 - 10; y <= y0 + 10; ++y)
        ui->comboSmartPortYear->addItem(QString::number(y), y);
    m_smartPortPeriodDebounceTimer = new QTimer(this);
    m_smartPortPeriodDebounceTimer->setSingleShot(true);
    m_smartPortPeriodDebounceTimer->setInterval(75);
    connect(m_smartPortPeriodDebounceTimer, &QTimer::timeout, this, &MainWindow::applySmartPortPeriodFromUi);
    connect(ui->comboSmartPortMonth, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, [this](int) { scheduleSmartPortPeriodApply(); });
    connect(ui->comboSmartPortYear, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, [this](int) { scheduleSmartPortPeriodApply(); });
    ui->comboSmartPortYear->setMinimumContentsLength(4);
    // Pas de connect() manuel pour btnSmartPortExportPdf : connectSlotsByName (setupUi) le fait déjà.
    syncSmartPortCombosToMonth(QDate::currentDate());
}

void MainWindow::setupSmartPortActuatorSliders()
{
    auto applyOnOff = [this](QLabel *lbl, int v) {
        if (lbl)
            lbl->setText(v <= 0 ? tr("Off") : tr("On"));
    };

    if (ui->sliderSmartPortSenSecMode && ui->lblSmartPortSenSecModeVal) {
        connect(ui->sliderSmartPortSenSecMode, &QSlider::valueChanged, this, [this, applyOnOff](int v) {
            applyOnOff(ui->lblSmartPortSenSecModeVal, v);
            if (!m_syncingSmartPortFromSerial)
                m_smartPortArduino.setSecurityMode(v > 0);
        });
        applyOnOff(ui->lblSmartPortSenSecModeVal, ui->sliderSmartPortSenSecMode->value());
    }

    if (ui->sliderSmartPortActFan && ui->lblSmartPortActFanVal) {
        connect(ui->sliderSmartPortActFan, &QSlider::valueChanged, this, [this, applyOnOff](int v) {
            applyOnOff(ui->lblSmartPortActFanVal, v);
            if (!m_syncingSmartPortFromSerial)
                m_smartPortArduino.setFan(v > 0);
        });
        applyOnOff(ui->lblSmartPortActFanVal, ui->sliderSmartPortActFan->value());
    }

    if (ui->sliderSmartPortActDoor && ui->lblSmartPortActDoorVal) {
        connect(ui->sliderSmartPortActDoor, &QSlider::valueChanged, this, [this, applyOnOff](int v) {
            applyOnOff(ui->lblSmartPortActDoorVal, v);
            pushPorteCommandToArduino(v > 0);
        });
        applyOnOff(ui->lblSmartPortActDoorVal, ui->sliderSmartPortActDoor->value());
    }

    if (ui->sliderSmartPortActPump && ui->lblSmartPortActPumpVal) {
        connect(ui->sliderSmartPortActPump, &QSlider::valueChanged, this, [this, applyOnOff](int v) {
            applyOnOff(ui->lblSmartPortActPumpVal, v);
            if (!m_syncingSmartPortFromSerial)
                m_smartPortArduino.setPump(v > 0);
        });
        applyOnOff(ui->lblSmartPortActPumpVal, ui->sliderSmartPortActPump->value());
    }

    if (ui->sliderSmartPortActLight && ui->lblSmartPortActLightVal) {
        connect(ui->sliderSmartPortActLight, &QSlider::valueChanged, this, [this, applyOnOff](int v) {
            applyOnOff(ui->lblSmartPortActLightVal, v);
            if (!m_syncingSmartPortFromSerial)
                m_smartPortArduino.setLighting(v > 0);
        });
        applyOnOff(ui->lblSmartPortActLightVal, ui->sliderSmartPortActLight->value());
    }

    if (ui->sliderSmartPortActBuzzer && ui->lblSmartPortActBuzzerVal) {
        connect(ui->sliderSmartPortActBuzzer, &QSlider::valueChanged, this, [this, applyOnOff](int v) {
            applyOnOff(ui->lblSmartPortActBuzzerVal, v);
            if (!m_syncingSmartPortFromSerial)
                m_smartPortArduino.setBuzzer(v > 0);
        });
        applyOnOff(ui->lblSmartPortActBuzzerVal, ui->sliderSmartPortActBuzzer->value());
    }

    connect(&m_smartPortArduino, &ArduinoMontage2::stateUpdated,
            this, &MainWindow::applySmartPortRealtimeState,
            Qt::UniqueConnection);
    connect(&m_smartPortArduino, &ArduinoMontage2::logLine, this, [](const QString &line) {
        if (line.startsWith(QStringLiteral("STATE;")))
            return;
        qDebug() << "[SmartPort]" << line;
    }, Qt::QueuedConnection);

    connect(&m_smartPortArduino, &ArduinoMontage2::connectedChanged, this,
            [this](bool connected, const QString &) {
                if (connected)
                    m_smartPortArduino.setAllAuto();
            }, Qt::QueuedConnection);
}

void MainWindow::applySmartPortRealtimeState(const SmartPortRealtimeState &state)
{
    m_syncingSmartPortFromSerial = true;
    const bool blinkRed = ((QDateTime::currentMSecsSinceEpoch() / 280) % 2) == 0;

    if (ui->sliderSmartPortSenSecMode) {
        const QSignalBlocker b(ui->sliderSmartPortSenSecMode);
        ui->sliderSmartPortSenSecMode->setValue(state.securityMode ? 1 : 0);
    }
    if (ui->sliderSmartPortActFan) {
        const QSignalBlocker b(ui->sliderSmartPortActFan);
        ui->sliderSmartPortActFan->setValue(state.fan ? 1 : 0);
    }
    // Porte reste independante (RFID), pompe a son slider dedie.
    if (ui->sliderSmartPortActPump) {
        const QSignalBlocker b(ui->sliderSmartPortActPump);
        ui->sliderSmartPortActPump->setValue(state.pump ? 1 : 0);
    }
    if (ui->sliderSmartPortActLight) {
        const QSignalBlocker b(ui->sliderSmartPortActLight);
        ui->sliderSmartPortActLight->setValue(state.light ? 1 : 0);
    }
    if (ui->sliderSmartPortActBuzzer) {
        const QSignalBlocker b(ui->sliderSmartPortActBuzzer);
        ui->sliderSmartPortActBuzzer->setValue(state.buzzer ? 1 : 0);
    }

    auto setLabel = [](QLabel *lbl, bool on) {
        if (lbl)
            lbl->setText(on ? QObject::tr("On") : QObject::tr("Off"));
    };
    auto setSensorBadge = [blinkRed](QLabel *lbl, bool on) {
        if (!lbl)
            return;
        lbl->setText(on ? QObject::tr("On") : QObject::tr("Off"));
        if (!on) {
            lbl->setStyleSheet(QStringLiteral(
                "color:#22c55e;"
                "border:1px solid #22c55e;"
                "border-radius:11px;"
                "background:rgba(34,197,94,0.12);"
                "padding:1px 10px;"));
            return;
        }
        const QString redStyle = blinkRed
            ? QStringLiteral("color:#fecaca;border:1px solid #ef4444;border-radius:11px;background:rgba(239,68,68,0.45);padding:1px 10px;")
            : QStringLiteral("color:#fca5a5;border:1px solid #b91c1c;border-radius:11px;background:rgba(127,29,29,0.70);padding:1px 10px;");
        lbl->setStyleSheet(redStyle);
    };

    setLabel(ui->lblSmartPortSenSecModeVal, state.securityMode);
    setSensorBadge(ui->lblSmartPortSenHumiditySt, state.humidity);
    setSensorBadge(ui->lblSmartPortSenGasSt, state.gas);
    setSensorBadge(ui->lblSmartPortSenFlameSt, state.flame);
    // Mouvement/magnetique actif visuellement seulement quand mode securite ON.
    setSensorBadge(ui->lblSmartPortSenMotionSt, state.securityMode && (state.motion || state.magnetic));
    setSensorBadge(ui->lblSmartPortSenRainSt, state.rain);
    if (ui->lblSmartPortSenWaterLevelSt) {
        if (std::isnan(state.waterCm)) {
            ui->lblSmartPortSenWaterLevelSt->setText(tr("-- cm | Marée basse | Marée haute"));
            ui->lblSmartPortSenWaterLevelSt->setStyleSheet(QStringLiteral("color:#94a3b8;"));
        } else {
            const QString cmTxt = QString::number(state.waterCm, 'f', 1) + tr(" cm");
            const QString lowColor = state.tideLow ? QStringLiteral("#22c55e") : QStringLiteral("#64748b");
            const QString highColor = state.tideLow ? QStringLiteral("#64748b") : QStringLiteral("#22c55e");
            ui->lblSmartPortSenWaterLevelSt->setText(
                QStringLiteral("%1 | <span style='color:%2;'>Marée basse</span> | <span style='color:%3;'>Marée haute</span>")
                    .arg(cmTxt, lowColor, highColor));
            ui->lblSmartPortSenWaterLevelSt->setStyleSheet(QStringLiteral("color:#22c55e;"));
        }
    }

    setLabel(ui->lblSmartPortActFanVal, state.fan);
    setLabel(ui->lblSmartPortActPumpVal, state.pump);
    setLabel(ui->lblSmartPortActLightVal, state.light);
    setLabel(ui->lblSmartPortActBuzzerVal, state.buzzer);

    m_syncingSmartPortFromSerial = false;
}

void MainWindow::scheduleSmartPortPeriodApply()
{
    if (m_populatingSmartPortCombos || !m_smartPortPeriodDebounceTimer)
        return;
    m_smartPortPeriodDebounceTimer->start();
}

void MainWindow::syncSmartPortCombosToMonth(const QDate &anyDayInMonth)
{
    if (!ui->comboSmartPortMonth || !ui->comboSmartPortYear)
        return;
    if (!anyDayInMonth.isValid())
        return;
    const int y = anyDayInMonth.year();
    const int m = anyDayInMonth.month();
    m_populatingSmartPortCombos = true;
    {
        QSignalBlocker bm(ui->comboSmartPortMonth);
        QSignalBlocker by(ui->comboSmartPortYear);
        int mi = ui->comboSmartPortMonth->findData(m);
        if (mi < 0)
            mi = qBound(0, m - 1, ui->comboSmartPortMonth->count() - 1);
        ui->comboSmartPortMonth->setCurrentIndex(mi);
        int yi = ui->comboSmartPortYear->findData(y);
        if (yi < 0) {
            ui->comboSmartPortYear->addItem(QString::number(y), y);
            yi = ui->comboSmartPortYear->findData(y);
        }
        if (yi >= 0)
            ui->comboSmartPortYear->setCurrentIndex(yi);
    }
    m_populatingSmartPortCombos = false;
}

void MainWindow::applySmartPortPeriodFromUi()
{
    if (m_populatingSmartPortCombos)
        return;
    if (!ui->comboSmartPortMonth || !ui->comboSmartPortYear || !ui->tableSmartPortDays)
        return;
    const int m = ui->comboSmartPortMonth->currentData().toInt();
    const int y = ui->comboSmartPortYear->currentData().toInt();
    if (m < 1 || m > 12 || y < 1900)
        return;
    monitorMonthRangeFromDate(QDate(y, m, 1), &m_smartPortMonthStart, &m_smartPortDays);
    refreshSmartPortDayList();
}

void MainWindow::refreshSmartPortDayList()
{
    if (!ui->tableSmartPortDays)
        return;
    QWidget *const root = ui->tableSmartPortDays->parentWidget();
    if (root)
        root->setUpdatesEnabled(false);

    const int days = m_smartPortDays;
    ui->tableSmartPortDays->setRowCount(days);
    ui->tableSmartPortDays->setColumnCount(1);
    ui->tableSmartPortDays->setHorizontalHeaderLabels(QStringList() << tr("Date"));
    ui->tableSmartPortDays->verticalHeader()->setVisible(false);
    ui->tableSmartPortDays->horizontalHeader()->setStretchLastSection(true);

    const QDate today = QDate::currentDate();
    const QColor todayBg(QStringLiteral("#0e7490"));
    const QColor todayFg(QStringLiteral("#ecfeff"));
    int todayRow = -1;
    for (int r = 0; r < days; ++r) {
        const QDate d = m_smartPortMonthStart.addDays(r);
        auto *dateItem = new QTableWidgetItem(displayFrDate(d));
        dateItem->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        dateItem->setFlags(dateItem->flags() & ~Qt::ItemIsEditable);
        ui->tableSmartPortDays->setItem(r, 0, dateItem);
        if (d == today) {
            todayRow = r;
            dateItem->setBackground(QBrush(todayBg));
            dateItem->setForeground(QBrush(todayFg));
        }
    }

    const int startRow = (todayRow >= 0) ? todayRow : 0;
    ui->tableSmartPortDays->setCurrentCell(startRow, 0);
    if (todayRow >= 0) {
        if (QTableWidgetItem *it = ui->tableSmartPortDays->item(todayRow, 0))
            ui->tableSmartPortDays->scrollToItem(it, QAbstractItemView::PositionAtCenter);
    }
    if (root)
        root->setUpdatesEnabled(true);

    const int rowForDetail = startRow;
    QTimer::singleShot(0, this, [this, rowForDetail]() {
        on_tableSmartPortDays_cellClicked(rowForDetail, 0);
    });
}

static QString smartPortTotalHoursForDay(const EmployeeRow &e, const QDate &day)
{
    const QDateTime &ti = e.port_date_entree;
    const QDateTime &to = e.port_date_sortie;
    if (!ti.isValid() || ti.date() != day)
        return QStringLiteral("—");
    if (!to.isValid() || to.date() != day)
        return QStringLiteral("—");
    if (to <= ti)
        return QStringLiteral("—");
    const qint64 secs = ti.secsTo(to);
    const int h = static_cast<int>(secs / 3600);
    const int mins = static_cast<int>((secs % 3600) / 60);
    const int s = static_cast<int>(secs % 60);
    return QStringLiteral("%1h %2m %3s")
        .arg(h)
        .arg(mins, 2, 10, QChar('0'))
        .arg(s, 2, 10, QChar('0'));
}

void MainWindow::refreshSmartPortEmployeeTable(const QDate &day)
{
    if (!ui->tableSmartPortEmployees)
        return;
    if (!day.isValid()) {
        ui->tableSmartPortEmployees->setRowCount(0);
        return;
    }
    QString err;
    QList<EmployeeRow> rows = EmployeeCrud::list(m_db, QString(), &err);
    if (!err.trimmed().isEmpty()) {
        ui->tableSmartPortEmployees->setRowCount(0);
        return;
    }
    std::sort(rows.begin(), rows.end(), [](const EmployeeRow &a, const EmployeeRow &b) {
        const int c = QString::compare(a.nom, b.nom, Qt::CaseInsensitive);
        if (c != 0) return c < 0;
        return QString::compare(a.prenom, b.prenom, Qt::CaseInsensitive) < 0;
    });

    const int kCols = 7;
    ui->tableSmartPortEmployees->setColumnCount(kCols);
    ui->tableSmartPortEmployees->setHorizontalHeaderLabels({
        QStringLiteral("CIN"),
        QStringLiteral("Nom"),
        QStringLiteral("Prénom"),
        QStringLiteral("Service"),
        QStringLiteral("Entrée"),
        QStringLiteral("Sortie"),
        QStringLiteral("Heure totale"),
    });
    ui->tableSmartPortEmployees->setRowCount(rows.size());

    auto fmtTime = [](const QDateTime &dt, const QDate &d) -> QString {
        if (!dt.isValid() || dt.date() != d)
            return QStringLiteral("—");
        return dt.time().toString(QStringLiteral("HH:mm:ss"));
    };

    for (int r = 0; r < rows.size(); ++r) {
        const EmployeeRow &e = rows.at(r);
        ui->tableSmartPortEmployees->setItem(r, 0, new QTableWidgetItem(e.cin));
        ui->tableSmartPortEmployees->setItem(r, 1, new QTableWidgetItem(e.nom));
        ui->tableSmartPortEmployees->setItem(r, 2, new QTableWidgetItem(e.prenom));
        ui->tableSmartPortEmployees->setItem(r, 3, new QTableWidgetItem(e.departement));
        ui->tableSmartPortEmployees->setItem(r, 4, new QTableWidgetItem(fmtTime(e.port_date_entree, day)));
        ui->tableSmartPortEmployees->setItem(r, 5, new QTableWidgetItem(fmtTime(e.port_date_sortie, day)));
        ui->tableSmartPortEmployees->setItem(r, 6, new QTableWidgetItem(smartPortTotalHoursForDay(e, day)));
    }
    for (int c = 0; c < kCols; ++c) {
        if (QTableWidgetItem *h = ui->tableSmartPortEmployees->horizontalHeaderItem(c))
            h->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    }
}

void MainWindow::setupMonitorPeriodSelectors()
{
    QComboBox *monthCombo = this->findChild<QComboBox *>(QStringLiteral("comboMonitorMonth"));
    QComboBox *yearCombo = this->findChild<QComboBox *>(QStringLiteral("comboMonitorYear"));
    QPushButton *pdfBtn = this->findChild<QPushButton *>(QStringLiteral("btnMonitorExportPdf"));
    if (!monthCombo || !yearCombo || !pdfBtn)
        return;

    monthCombo->clear();
    const QLocale fr(QLocale::French);
    for (int m = 1; m <= 12; ++m)
        monthCombo->addItem(fr.standaloneMonthName(m, QLocale::LongFormat), m);

    yearCombo->clear();
    const int y0 = QDate::currentDate().year();
    for (int y = y0 - 10; y <= y0 + 10; ++y)
        yearCombo->addItem(QString::number(y), y);

    m_monitorPeriodDebounceTimer = new QTimer(this);
    m_monitorPeriodDebounceTimer->setSingleShot(true);
    m_monitorPeriodDebounceTimer->setInterval(75);
    connect(m_monitorPeriodDebounceTimer, &QTimer::timeout, this, &MainWindow::applyMonitorPeriodFromUi);
    connect(monthCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, [this](int) { scheduleMonitorPeriodApply(); });
    connect(yearCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, [this](int) { scheduleMonitorPeriodApply(); });
    yearCombo->setMinimumContentsLength(4);
    syncMonitorCombosToMonth(QDate::currentDate());
}

void MainWindow::scheduleMonitorPeriodApply()
{
    if (m_populatingMonitorCombos || !m_monitorPeriodDebounceTimer)
        return;
    m_monitorPeriodDebounceTimer->start();
}

void MainWindow::syncMonitorCombosToMonth(const QDate &anyDayInMonth)
{
    QComboBox *monthCombo = this->findChild<QComboBox *>(QStringLiteral("comboMonitorMonth"));
    QComboBox *yearCombo = this->findChild<QComboBox *>(QStringLiteral("comboMonitorYear"));
    if (!monthCombo || !yearCombo)
        return;
    if (!anyDayInMonth.isValid())
        return;

    const int y = anyDayInMonth.year();
    const int m = anyDayInMonth.month();
    m_populatingMonitorCombos = true;
    {
        QSignalBlocker bm(monthCombo);
        QSignalBlocker by(yearCombo);
        int mi = monthCombo->findData(m);
        if (mi < 0)
            mi = qBound(0, m - 1, monthCombo->count() - 1);
        monthCombo->setCurrentIndex(mi);
        int yi = yearCombo->findData(y);
        if (yi < 0) {
            yearCombo->addItem(QString::number(y), y);
            yi = yearCombo->findData(y);
        }
        if (yi >= 0)
            yearCombo->setCurrentIndex(yi);
    }
    m_populatingMonitorCombos = false;
}

void MainWindow::applyMonitorPeriodFromUi()
{
    if (m_populatingMonitorCombos)
        return;
    QComboBox *monthCombo = this->findChild<QComboBox *>(QStringLiteral("comboMonitorMonth"));
    QComboBox *yearCombo = this->findChild<QComboBox *>(QStringLiteral("comboMonitorYear"));
    if (!monthCombo || !yearCombo || !ui->tableWidget)
        return;
    if (!m_db.isValid() || !m_db.isOpen())
        return;
    const int m = monthCombo->currentData().toInt();
    const int y = yearCombo->currentData().toInt();
    if (m < 1 || m > 12 || y < 1900)
        return;
    monitorMonthRangeFromDate(QDate(y, m, 1), &m_monitorWeekStart, &m_monitorDays);
    rebuildMonitorCacheFromDb(true);
    refreshMonitorDisponibiliteTable();
}

void MainWindow::refreshMonitorDisponibiliteTable()
{
    if (!ui->tableWidget)
        return;
    QWidget *const root = ui->tableWidget->parentWidget();
    if (root)
        root->setUpdatesEnabled(false);

    const int days = m_monitorDays;
    ui->tableWidget->setRowCount(days);
    ui->tableWidget->setColumnCount(2);
    ui->tableWidget->setHorizontalHeaderLabels(QStringList() << tr("Date") << tr("Manque"));
    ui->tableWidget->verticalHeader()->setVisible(false);
    ui->tableWidget->horizontalHeader()->setStretchLastSection(true);

    const QDate today = QDate::currentDate();
    const QColor todayBg(QStringLiteral("#0e7490"));
    const QColor todayFg(QStringLiteral("#ecfeff"));
    int todayRow = -1;
    for (int r = 0; r < days; ++r) {
        const QDate d = m_monitorWeekStart.addDays(r);
        auto *dateItem = new QTableWidgetItem(displayFrDate(d));
        dateItem->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        dateItem->setFlags(dateItem->flags() & ~Qt::ItemIsEditable);
        ui->tableWidget->setItem(r, 0, dateItem);

        const int missing = (d < today) ? 0 : monitorMissingCountForDate(d);
        auto *cell = new QTableWidgetItem(QString::number(missing));
        cell->setTextAlignment(Qt::AlignCenter);
        cell->setFlags(cell->flags() & ~Qt::ItemIsEditable);
        ui->tableWidget->setItem(r, 1, cell);

        if (d == today) {
            todayRow = r;
            dateItem->setBackground(QBrush(todayBg));
            dateItem->setForeground(QBrush(todayFg));
            cell->setBackground(QBrush(todayBg));
            cell->setForeground(QBrush(todayFg));
        }
    }

    const int startRow = (todayRow >= 0) ? todayRow : 0;
    ui->tableWidget->setCurrentCell(startRow, 0);
    if (todayRow >= 0) {
        if (QTableWidgetItem *it = ui->tableWidget->item(todayRow, 0))
            ui->tableWidget->scrollToItem(it, QAbstractItemView::PositionAtCenter);
    }
    if (root)
        root->setUpdatesEnabled(true);

    const int rowForDetail = startRow;
    QTimer::singleShot(0, this, [this, rowForDetail]() {
        on_tableWidget_cellClicked(rowForDetail, 0);
        layoutMonitorLeftColumn();
    });
}

void MainWindow::on_btnMonitorExportPdf_clicked()
{
    QComboBox *monthCombo = this->findChild<QComboBox *>(QStringLiteral("comboMonitorMonth"));
    QComboBox *yearCombo = this->findChild<QComboBox *>(QStringLiteral("comboMonitorYear"));
    if (!monthCombo || !yearCombo) {
        monthCombo = ui->comboSmartPortMonth;
        yearCombo = ui->comboSmartPortYear;
    }

    if (!monthCombo || !yearCombo)
        return;
    if (!m_db.isValid() || !m_db.isOpen()) {
        QMessageBox::warning(this, tr("Export PDF"), tr("Connexion base de données indisponible."));
        return;
    }
    const int m = monthCombo->currentData().toInt();
    const int y = yearCombo->currentData().toInt();
    if (m < 1 || m > 12 || y < 1900)
        return;

    monitorMonthRangeFromDate(QDate(y, m, 1), &m_monitorWeekStart, &m_monitorDays);
    rebuildMonitorCacheFromDb(true);

    const QLocale fr(QLocale::French);
    const QString periodLabel = fr.standaloneMonthName(m, QLocale::LongFormat) + QLatin1Char(' ') + QString::number(y);

    QString defaultName = QStringLiteral("disponibilite_manques_%1_%2.pdf")
        .arg(y, 4, 10, QLatin1Char('0'))
        .arg(m, 2, 10, QLatin1Char('0'));
    QString filePath = QFileDialog::getSaveFileName(this,
        tr("Exporter la grille des manques (PDF)"),
        defaultName,
        tr("PDF (*.pdf)"));
    if (filePath.isEmpty())
        return;
    if (!filePath.endsWith(QStringLiteral(".pdf"), Qt::CaseInsensitive))
        filePath += QStringLiteral(".pdf");

    QPrinter printer(QPrinter::HighResolution);
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setOutputFileName(filePath);
    printer.setPageSize(QPageSize(QPageSize::A4));
    printer.setPageOrientation(QPageLayout::Portrait);
    printer.setPageMargins(QMarginsF(12, 12, 12, 12), QPageLayout::Millimeter);
    printer.setResolution(144);

    const QDate today = QDate::currentDate();
    const QDateTime now = QDateTime::currentDateTime();
    QString tz = now.timeZoneAbbreviation();
    if (tz.isEmpty())
        tz = QString::fromUtf8(QTimeZone::systemTimeZoneId()).split(QLatin1Char('/')).last();

    auto plainToHtmlBreaks = [](const QString &plain) {
        const QStringList parts = plain.split(QLatin1Char('\n'));
        QStringList esc;
        esc.reserve(parts.size());
        for (const QString &p : parts)
            esc << p.toHtmlEscaped();
        return esc.join(QStringLiteral("<br/>"));
    };

    const QString docTitle = tr("Moniteur de dotation & remplacements — %1").arg(periodLabel);

    QString html;
    html += QStringLiteral("<!DOCTYPE html><html><head><meta charset=\"utf-8\">"
                           "<style>"
                           "@page { size: A4 portrait; margin: 14mm 12mm 16mm 12mm; }"
                           "body { font-family: 'Segoe UI', 'Helvetica Neue', Helvetica, Arial, sans-serif; color:#1e293b; "
                           "font-size:10pt; margin:0; -webkit-print-color-adjust: exact; print-color-adjust: exact; }"
                           ".kicker { font-size:8.5pt; font-weight:600; color:#64748b; letter-spacing:0.04em; "
                           "text-transform:uppercase; margin:0 0 6px 0; }"
                           ".docTitle { font-size:16pt; font-weight:600; color:#0891b2; margin:0 0 14px 0; "
                           "line-height:1.25; letter-spacing:-0.02em; }"
                           ".exportLine { font-size:9.5pt; color:#475569; margin:0 0 10px 0; line-height:1.35; }"
                           ".intro { font-size:9.25pt; color:#475569; line-height:1.45; margin:0 0 6px 0; }"
                           "table { width:100%; border-collapse:collapse; margin-top:14px; }"
                           "thead { display: table-header-group; }"
                           "tr { page-break-inside: avoid; }"
                           "th, td { border:1px solid #e2e8f0; padding:8px 11px; text-align:left; vertical-align:top; }"
                           "th { background:#0f766e; color:#ffffff; font-weight:700; font-size:9.5pt; "
                           "border-color:#0d9488; }"
                           "th.dateCol, td.dateCol { width:12%; white-space:nowrap; }"
                           "th.dateCol, th.deptCol, th.affectCol { color:#ffffff; }"
                           "th.deptCol, td.deptCol { width:26%; }"
                           "td.deptCol { color:#0f172a; }"
                           "tbody td { background:#ffffff; color:#334155; font-size:9.25pt; }"
                           "tbody tr td.dateCol { color:#0f172a; font-weight:600; }"
                           "td.affectCol { line-height:1.4; word-break:break-word; }"
                           "</style></head><body>");
    html += QStringLiteral("<p class=\"kicker\">%1</p><h1 class=\"docTitle\">%2</h1>")
        .arg(tr("Mois · disponibilité & affectations").toHtmlEscaped(), docTitle.toHtmlEscaped());
    html += QStringLiteral("<p class=\"exportLine\">%1</p>")
        .arg(tr("Export le %1 %2")
                 .arg(fr.toString(now, QStringLiteral("dddd d MMMM yyyy HH:mm:ss")),
                      tz)
                 .toHtmlEscaped());
    html += QStringLiteral("<p class=\"intro\">%1</p>")
        .arg(tr("Chaque ligne : services sans dotation (aucun employé disponible sans affectation) et personnes affectées.")
                 .toHtmlEscaped());
    html += QStringLiteral("<p class=\"intro\">%1</p>")
        .arg(tr("Jours passés : « départements manquants » affiche — (disponibilité actuelle des employés, pas historique).")
                 .toHtmlEscaped());
    html += QStringLiteral("<table><thead><tr>"
                           "<th class=\"dateCol\">%1</th>"
                           "<th class=\"deptCol\">%2</th>"
                           "<th class=\"affectCol\">%3</th></tr></thead><tbody>")
        .arg(tr("Date").toHtmlEscaped(),
             tr("Départements manquants").toHtmlEscaped(),
             tr("Affectations").toHtmlEscaped());
    for (int i = 0; i < m_monitorDays; ++i) {
        const QDate d = m_monitorWeekStart.addDays(i);
        QString deptCell;
        if (d < today)
            deptCell = QStringLiteral("\u2014"); // tiret cadratin (comme le PDF de référence)
        else {
            const QStringList missing = monitorMissingDeptsForDate(d);
            deptCell = missing.isEmpty() ? QStringLiteral("\u2014") : missing.join(QStringLiteral(", "));
        }
        const QString affectPlain = monitorAssignmentsSummaryForDate(d);
        html += QStringLiteral("<tr><td class=\"dateCol\">%1</td><td class=\"deptCol\">%2</td><td class=\"affectCol\">%3</td></tr>")
            .arg(displayFrDate(d).toHtmlEscaped(), deptCell.toHtmlEscaped(), plainToHtmlBreaks(affectPlain));
    }
    html += QStringLiteral("</tbody></table></body></html>");

    QTextDocument doc;
    doc.setDocumentMargin(0);
    doc.setDefaultFont(QFont(QStringLiteral("Helvetica"), 10));
    doc.setHtml(html);
    doc.print(&printer);

    QMessageBox::information(this, tr("Export PDF"),
        tr("Le rapport a été enregistré :\n%1").arg(filePath));
}

void MainWindow::on_btnSmartPortExportPdf_clicked()
{
    if (!ui->comboSmartPortMonth || !ui->comboSmartPortYear)
        return;
    if (!m_db.isValid() || !m_db.isOpen()) {
        QMessageBox::warning(this, tr("Export PDF"), tr("Connexion base de données indisponible."));
        return;
    }
    const int m = ui->comboSmartPortMonth->currentData().toInt();
    const int y = ui->comboSmartPortYear->currentData().toInt();
    if (m < 1 || m > 12 || y < 1900)
        return;

    QDate monthStart;
    int nDays = 0;
    monitorMonthRangeFromDate(QDate(y, m, 1), &monthStart, &nDays);

    QString err;
    QList<EmployeeRow> rows = EmployeeCrud::list(m_db, QString(), &err);
    if (!err.trimmed().isEmpty()) {
        QMessageBox::warning(this, tr("Export PDF"), tr("Impossible de charger les employés."));
        return;
    }
    std::sort(rows.begin(), rows.end(), [](const EmployeeRow &a, const EmployeeRow &b) {
        const int c = QString::compare(a.nom, b.nom, Qt::CaseInsensitive);
        if (c != 0)
            return c < 0;
        return QString::compare(a.prenom, b.prenom, Qt::CaseInsensitive) < 0;
    });

    auto fmtTime = [](const QDateTime &dt, const QDate &d) -> QString {
        if (!dt.isValid() || dt.date() != d)
            return QString();
        return dt.time().toString(QStringLiteral("HH:mm"));
    };

    const QLocale fr(QLocale::French);
    const QString periodLabel = fr.standaloneMonthName(m, QLocale::LongFormat) + QLatin1Char(' ') + QString::number(y);

    QString defaultName = QStringLiteral("smart_port_pointages_%1_%2.pdf")
                              .arg(y, 4, 10, QLatin1Char('0'))
                              .arg(m, 2, 10, QLatin1Char('0'));
    QString filePath = QFileDialog::getSaveFileName(this,
                                                    tr("Exporter Smart Port (PDF)"),
                                                    defaultName,
                                                    tr("PDF (*.pdf)"));
    if (filePath.isEmpty())
        return;
    if (!filePath.endsWith(QStringLiteral(".pdf"), Qt::CaseInsensitive))
        filePath += QStringLiteral(".pdf");

    QPrinter printer(QPrinter::HighResolution);
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setOutputFileName(filePath);
    printer.setPageSize(QPageSize(QPageSize::A4));
    printer.setPageOrientation(QPageLayout::Portrait);
    printer.setPageMargins(QMarginsF(12, 12, 12, 12), QPageLayout::Millimeter);
    printer.setResolution(120);

    int daysWithPointages = 0;
    int totalPointageRows = 0;
    for (int i = 0; i < nDays; ++i) {
        const QDate d = monthStart.addDays(i);
        int nDay = 0;
        for (const EmployeeRow &e : rows) {
            const QString inT = fmtTime(e.port_date_entree, d);
            const QString outT = fmtTime(e.port_date_sortie, d);
            if (inT.isEmpty() && outT.isEmpty())
                continue;
            ++nDay;
        }
        if (nDay > 0)
            ++daysWithPointages;
        totalPointageRows += nDay;
    }

    const QString thead = QStringLiteral(
        "<thead><tr><th>CIN</th><th>Nom</th><th>Prénom</th><th>Service</th><th>Entrée</th><th>Sortie</th><th>Heure "
        "totale</th></tr></thead>");

    QString html;
    html += QStringLiteral("<!DOCTYPE html><html><head><meta charset=\"utf-8\">"
                           "<style>"
                           "@page { size: A4 portrait; margin: 11mm; }"
                           "body { font-family: 'Segoe UI','Helvetica',sans-serif; color:#1e293b; font-size:9.5pt; margin:0; }"
                           ".header { background-color:#0f766e; color:#ffffff; padding:14px 18px; border-radius:6px; "
                           "border:1px solid #0d5c54; }"
                           ".title { font-size:17pt; font-weight:700; margin:0 0 4px 0; }"
                           ".sub { font-size:10pt; color:#ccfbf1; margin:0; }"
                           ".meta { margin:12px 0 10px 0; font-size:9pt; color:#475569; border-bottom:1px solid #e2e8f0; "
                           "padding-bottom:8px; }"
                           ".summary { margin:0 0 14px 0; }"
                           ".sumtable { width:100%; border-collapse:collapse; margin:0; }"
                           ".sumtable td { border:1px solid #cbd5e1; padding:10px 12px; vertical-align:top; background:#f8fafc; "
                           "width:33%; }"
                           ".sumtable strong { display:block; color:#0f766e; font-size:8.5pt; text-transform:uppercase; "
                           "margin-bottom:4px; }"
                           ".day { margin-top:16px; page-break-inside:avoid; }"
                           ".day-h { margin:0 0 8px 0; padding:6px 10px; background:#ecfdf5; border-left:4px solid #0f766e; "
                           "font-size:11pt; color:#0f766e; font-weight:700; }"
                           "table.data { width:100%; border-collapse:collapse; margin:0 0 6px 0; }"
                           "table.data th, table.data td { border:1px solid #94a3b8; padding:6px 8px; text-align:left; "
                           "font-size:8.5pt; }"
                           "table.data th { background-color:#134e4a; color:#ffffff; font-weight:700; }"
                           "table.data tbody tr:nth-child(even) td { background:#f1f5f9; }"
                           ".muted { color:#64748b; font-style:italic; text-align:center; }"
                           ".footer { margin-top:18px; padding-top:10px; border-top:1px solid #e2e8f0; font-size:8pt; "
                           "color:#94a3b8; text-align:center; }"
                           "</style></head><body>");
    html += QStringLiteral("<div class=\"header\"><p class=\"title\">%1</p><p class=\"sub\">%2 — %3</p></div>")
                .arg(tr("Smart Port — pointages").toHtmlEscaped(),
                     tr("Smart Port Management").toHtmlEscaped(),
                     periodLabel.toHtmlEscaped());
    html += QStringLiteral("<div class=\"meta\">%1</div>")
                .arg(tr("Export le %1").arg(fr.toString(QDateTime::currentDateTime(), QLocale::LongFormat)).toHtmlEscaped());
    html += QStringLiteral("<div class=\"summary\"><table class=\"sumtable\"><tr>"
                           "<td><strong>%1</strong>%2</td>"
                           "<td><strong>%3</strong>%4 / %5</td>"
                           "<td><strong>%6</strong>%7</td>"
                           "</tr></table></div>")
                .arg(tr("Période").toHtmlEscaped(),
                     periodLabel.toHtmlEscaped(),
                     tr("Jours avec pointages").toHtmlEscaped(),
                     QString::number(daysWithPointages),
                     QString::number(nDays),
                     tr("Enregistrements").toHtmlEscaped(),
                     QString::number(totalPointageRows));

    for (int i = 0; i < nDays; ++i) {
        const QDate d = monthStart.addDays(i);
        html += QStringLiteral("<div class=\"day\"><p class=\"day-h\">%1</p>").arg(displayFrDate(d).toHtmlEscaped());
        QString bodyRows;
        for (const EmployeeRow &e : rows) {
            const QString inT = fmtTime(e.port_date_entree, d);
            const QString outT = fmtTime(e.port_date_sortie, d);
            if (inT.isEmpty() && outT.isEmpty())
                continue;
            const QString totalH = smartPortTotalHoursForDay(e, d);
            bodyRows += QStringLiteral("<tr><td>%1</td><td>%2</td><td>%3</td><td>%4</td><td>%5</td><td>%6</td><td>%7</td></tr>")
                             .arg(e.cin.toHtmlEscaped(),
                                  e.nom.toHtmlEscaped(),
                                  e.prenom.toHtmlEscaped(),
                                  e.departement.toHtmlEscaped(),
                                  inT.isEmpty() ? QStringLiteral("—") : inT.toHtmlEscaped(),
                                  outT.isEmpty() ? QStringLiteral("—") : outT.toHtmlEscaped(),
                                  totalH.toHtmlEscaped());
        }
        if (bodyRows.isEmpty()) {
            bodyRows = QStringLiteral("<tr><td colspan=\"7\" class=\"muted\">%1</td></tr>")
                           .arg(tr("Aucun pointage enregistré pour cette journée.").toHtmlEscaped());
        }
        html += QStringLiteral("<table class=\"data\">%1<tbody>%2</tbody></table></div>").arg(thead, bodyRows);
    }
    html += QStringLiteral("<div class=\"footer\">%1</div></body></html>")
                .arg(tr("Document généré par ATLAS — Smart Port Management").toHtmlEscaped());

    QTextDocument doc;
    doc.setDocumentMargin(0);
    doc.setDefaultFont(QFont(QStringLiteral("Helvetica"), 9));
    doc.setHtml(html);
    doc.print(&printer);

    QMessageBox::information(this, tr("Export PDF"),
                             tr("Le rapport a été enregistré :\n%1").arg(filePath));
}

void MainWindow::on_btnSubEmplSmartPort_clicked()
{
    ui->stackedWidgetEmployes->setCurrentIndex(3);
    syncEmployesSubNavChecked(ui, 3);
    QTimer::singleShot(0, this, [this]() {
        if (!ui->comboSmartPortMonth || !ui->comboSmartPortYear || !ui->tableSmartPortDays)
            return;
        if (!m_db.isValid() || !m_db.isOpen())
            return;
        applySmartPortPeriodFromUi();
    });
}

void MainWindow::on_tableSmartPortDays_cellClicked(int row, int column)
{
    Q_UNUSED(column)
    if (!ui->tableSmartPortDays)
        return;
    QTableWidgetItem *it = ui->tableSmartPortDays->item(row, 0);
    if (!it)
        return;
    const QDate d = parseFrDate(it->text());
    if (!d.isValid())
        return;
    m_smartPortSelectedDate = d;
    refreshSmartPortEmployeeTable(d);
}

