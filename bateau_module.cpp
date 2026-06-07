#include "bateau_module.h"
#include "bateaudb.h"
#include "esp32_gps.h"
#include "ui_mainwindow.h"

#include <QMetaType>
#include <QTextEdit>

#include <QAbstractItemView>
#include <QBrush>
#include <QComboBox>
#include <QDate>
#include <QSettings>
#include <QDateEdit>
#include <QDateTime>
#include <QDebug>
#include <QEvent>
#include <QFileDialog>
#include <QFont>
#include <QFrame>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QMap>
#include <QMessageBox>
#include <QModelIndex>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QPaintEvent>
#include <QSet>
#include <QTime>
#include <QPlainTextEdit>
#include <QPrinter>
#include <QPushButton>
#include <QRandomGenerator>
#include <QRegularExpression>
#include <QSqlError>
#include <QSqlQuery>
#include <QStandardItem>
#include <QTextCursor>
#include <QTextDocument>
#include <QTimer>
#include <QVBoxLayout>
#include <QLineEdit>
#include <QSpinBox>
#include <QtMath>
#include <algorithm>
#include <cmath>
#include <QSizePolicy>

#include <QtCharts/QChart>
#include <QtCharts/QChartView>
#include <QtCharts/QPieSeries>

namespace Geo {

/* Fenêtre côtière type Golfe de Tunis — démo WGS84 (cohérente avec la maquette) */
constexpr double kMinLat = 36.75;
constexpr double kMaxLat = 36.92;
constexpr double kMinLon = 10.10;
constexpr double kMaxLon = 10.35;
constexpr double kPortLat = 36.805;
constexpr double kPortLon = 10.175;

double deterministic01(const QString &id, int salt)
{
    uint h = qHash(id + QString::number(salt));
    return (h % 100000) / 100000.0;
}

double sogFromMotorCv(const QString &motorField)
{
    QRegularExpression re(QStringLiteral("(\\d+)"));
    QRegularExpressionMatch m = re.match(motorField);
    int cv = m.hasMatch() ? m.captured(1).toInt() : 120;
    cv = qBound(40, cv, 520);
    return 3.5 + (cv / 520.0) * 9.0;
}

void seedSimTrack(BateauSimTrack &st, const BateauData &boat, int berthIndex)
{
    if (!boat.isAtSea) {
        const int col = berthIndex % 5;
        const int row = berthIndex / 5;
        st.lat = kPortLat + 0.00035 * col + deterministic01(boat.id, 1) * 0.0001;
        st.lon = kPortLon + 0.00045 * row + deterministic01(boat.id, 2) * 0.0001;
        st.headingDeg = 180.0 + deterministic01(boat.id, 3) * 40.0 - 20.0;
        st.sogKnots = 0.05;
    } else {
        st.lat = 36.78 + deterministic01(boat.id, 4) * (36.90 - 36.78);
        st.lon = 10.12 + deterministic01(boat.id, 5) * (10.32 - 10.12);
        if (qAbs(st.lat - kPortLat) < 0.012 && qAbs(st.lon - kPortLon) < 0.02) {
            st.lat += 0.02;
            st.lon += 0.03;
        }
        st.headingDeg = deterministic01(boat.id, 6) * 360.0;
        st.sogKnots = sogFromMotorCv(boat.motor);
    }
    st.lastAisEpochMs = QDateTime::currentMSecsSinceEpoch();
    st.initialized = true;
    st.lastDbAtSea = boat.isAtSea;
}

QPointF geoToScreen(double lat, double lon, const QRectF &plot)
{
    const double x = plot.left() + (lon - kMinLon) / (kMaxLon - kMinLon) * plot.width();
    const double y = plot.top() + (kMaxLat - lat) / (kMaxLat - kMinLat) * plot.height();
    return QPointF(x, y);
}

} // namespace Geo

namespace {

double haversineMetersPersist(double lat1, double lon1, double lat2, double lon2)
{
    constexpr double R = 6371000.0;
    const double p1 = qDegreesToRadians(lat1);
    const double p2 = qDegreesToRadians(lat2);
    const double dp = qDegreesToRadians(lat2 - lat1);
    const double dl = qDegreesToRadians(lon2 - lon1);
    const double a = std::sin(dp / 2) * std::sin(dp / 2)
        + std::cos(p1) * std::cos(p2) * std::sin(dl / 2) * std::sin(dl / 2);
    return 2 * R * std::asin(std::sqrt(std::min(1.0, a)));
}

/** ProjSem2’s `mainwindow.ui` does not declare these widgets on `Ui::MainWindow`; other UIs may. */
template<typename T>
T *findInCentral(Ui::MainWindow *ui, const QString &objectName)
{
    if (!ui || !ui->centralwidget)
        return nullptr;
    return ui->centralwidget->findChild<T *>(objectName, Qt::FindChildrenRecursively);
}

} // namespace

static bool wifiGpsLive(const Esp32GpsTcpClient *tcp)
{
    return tcp && tcp->isConnected();
}

BateauModule::BateauModule(Ui::MainWindow *ui, BateauDb *dbAccess, QWidget *dialogParent, QObject *parent)
    : QObject(parent)
    , m_ui(ui)
    , m_bateauDb(dbAccess)
    , m_dialogParent(dialogParent)
    , m_model(nullptr)
    , m_statsTimer(nullptr)
    , m_dateTechVisit(nullptr)
    , m_dateInsurance(nullptr)
    , m_chartType(nullptr)
    , m_chartStatus(nullptr)
    , m_chartLicense(nullptr)
    , m_blinkTimer(nullptr)
    , m_blinkState(false)
    , m_errName(nullptr)
    , m_errImmat(nullptr)
    , m_errLicense(nullptr)
    , m_errOwner(nullptr)
    , m_geoPage(nullptr)
    , m_gpsWorldMap(nullptr)
    , m_gpsTcp(nullptr)
    , m_lblGeoWgs84(nullptr)
    , m_lblGeoHover(nullptr)
    , m_txtAisLog(nullptr)
    , m_fleetTimer(nullptr)
    , m_geoTabReady(false)
{
}

BateauModule::~BateauModule() = default;

void BateauModule::reloadFromDatabase()
{
    refreshFromDb();
    updateTable();
    updateStatistics();
    if (m_geoTabReady)
        loadGpsFromDbOntoMap();
}

QString BateauModule::sqlBateauTable() const
{
    return m_bateauDb ? QStringLiteral("BATEAU") : QStringLiteral("bateau");
}

void BateauModule::initAfterSetupUi()
{
    initializeModels();
    initializeTimers();
    loadSampleDataIfEmpty();
    updateTable();
    updateStatistics();
    setupGeolocationTab();
}

void BateauModule::initializeModels()
{
    m_model = new QStandardItemModel(this);
    m_model->setHorizontalHeaderLabels({
        "ID", "Nom", "Type", "Immat", "Moteur",
        "Propriétaire", "N° Licence", "Date Licence",
        "Assurance", "Statut"
    });

    if (m_ui->tableBateaux_2) {
        m_ui->tableBateaux_2->setModel(m_model);
        m_ui->tableBateaux_2->horizontalHeader()->setStretchLastSection(true);
        m_ui->tableBateaux_2->setStyleSheet(
            "QTableView { background-color: #1a1a2e; border: 1px solid #16213e; gridline-color: transparent; selection-background-color: #9b59b6; alternate-background-color: #16213e; color: white; } "
            "QHeaderView::section { background-color: #0f3460; color: white; font-weight: bold; border: none; padding: 8px; } "
            "QTableView::item { padding: 5px; } "
            "QTableView::item:hover { background-color: #8e44ad; } "
            "QTableView::item:selected { background-color: #9b59b6; color: white; }");
        m_ui->tableBateaux_2->verticalHeader()->setVisible(false);
        m_ui->tableBateaux_2->horizontalHeader()->setStretchLastSection(true);
        m_ui->tableBateaux_2->setAlternatingRowColors(true);
        m_ui->tableBateaux_2->setSelectionBehavior(QAbstractItemView::SelectRows);
        m_ui->tableBateaux_2->setEditTriggers(QAbstractItemView::NoEditTriggers);
    }

    if (!m_chartType) {
        QWidget *chartHost = nullptr;
        QHBoxLayout *layout = nullptr;

        if (QFrame *statsFrame = findInCentral<QFrame>(m_ui, QStringLiteral("statsFrame"))) {
            chartHost = statsFrame;
            layout = qobject_cast<QHBoxLayout *>(chartHost->layout());
            if (!layout) {
                layout = new QHBoxLayout(chartHost);
                layout->setContentsMargins(5, 5, 5, 5);
                layout->setSpacing(10);
            }
        } else if (m_ui->tabStatistiques_8) {
            auto *grid = qobject_cast<QGridLayout *>(m_ui->tabStatistiques_8->layout());
            if (grid && !m_ui->tabStatistiques_8->findChild<QFrame *>(QStringLiteral("frameBateauChartsInjected"))) {
                auto *frameCharts = new QFrame(m_ui->tabStatistiques_8);
                frameCharts->setObjectName(QStringLiteral("frameBateauChartsInjected"));
                frameCharts->setMinimumHeight(220);
                frameCharts->setStyleSheet(
                    QStringLiteral("QFrame#frameBateauChartsInjected { background-color: #232323; border-radius: 10px; border: 2px solid #3498db; }"));
                layout = new QHBoxLayout(frameCharts);
                layout->setContentsMargins(5, 5, 5, 5);
                layout->setSpacing(10);
                grid->addWidget(frameCharts, 3, 0, 1, 4);
                chartHost = frameCharts;
            }
        }

        if (layout && chartHost) {
            m_chartType = new QChartView(new QChart(), chartHost);
            m_chartType->setRenderHint(QPainter::Antialiasing);
            m_chartType->setBackgroundBrush(QBrush(QColor("#232323")));
            m_chartType->chart()->setBackgroundVisible(false);
            m_chartType->chart()->setMargins(QMargins(0, 0, 0, 0));
            m_chartType->chart()->legend()->setVisible(true);
            m_chartType->chart()->legend()->setAlignment(Qt::AlignBottom);
            m_chartType->chart()->legend()->setLabelColor(Qt::white);
            m_chartType->chart()->legend()->setFont(QFont("Segoe UI", 8, QFont::Bold));
            layout->addWidget(m_chartType);

            m_chartStatus = new QChartView(new QChart(), chartHost);
            m_chartStatus->setRenderHint(QPainter::Antialiasing);
            m_chartStatus->setBackgroundBrush(QBrush(QColor("#232323")));
            m_chartStatus->chart()->setBackgroundVisible(false);
            m_chartStatus->chart()->setMargins(QMargins(0, 0, 0, 0));
            m_chartStatus->chart()->legend()->setVisible(true);
            m_chartStatus->chart()->legend()->setAlignment(Qt::AlignBottom);
            m_chartStatus->chart()->legend()->setLabelColor(Qt::white);
            m_chartStatus->chart()->legend()->setFont(QFont("Segoe UI", 8, QFont::Bold));
            layout->addWidget(m_chartStatus);

            m_chartLicense = new QChartView(new QChart(), chartHost);
            m_chartLicense->setRenderHint(QPainter::Antialiasing);
            m_chartLicense->setBackgroundBrush(QBrush(QColor("#232323")));
            m_chartLicense->chart()->setBackgroundVisible(false);
            m_chartLicense->chart()->setMargins(QMargins(0, 0, 0, 0));
            m_chartLicense->chart()->legend()->setVisible(true);
            m_chartLicense->chart()->legend()->setAlignment(Qt::AlignBottom);
            m_chartLicense->chart()->legend()->setLabelColor(Qt::white);
            m_chartLicense->chart()->legend()->setFont(QFont("Segoe UI", 8, QFont::Bold));
            layout->addWidget(m_chartLicense);

            layout->setStretch(0, 1);
            layout->setStretch(1, 1);
            layout->setStretch(2, 1);
        }
    }

    if (m_ui->lineID_15) {
        m_ui->lineID_15->setReadOnly(true);
        m_ui->lineID_15->setPlaceholderText("Auto-Generated");
    }

    if (m_ui->lineLicenseNum_8)
        m_ui->lineLicenseNum_8->setInputMask("\\L\\I\\C-0000;_");
    if (m_ui->linePrenom_36)
        m_ui->linePrenom_36->setInputMask("\\F\\R-0000-000;_");
    if (m_ui->linePrenom_37)
        m_ui->linePrenom_37->setInputMask("000 \\C\\V;_");

    m_dateTechVisit = m_ui->dateExpiry_2;
    m_dateInsurance = m_ui->centralwidget ? m_ui->centralwidget->findChild<QDateEdit *>(QStringLiteral("dateInsurance"))
                                           : nullptr;

    if (m_ui->lineLicence_15)
        m_errName = addErrorLabel(m_ui->lineLicence_15, "Nom requis");
    if (m_ui->linePrenom_36)
        m_errImmat = addErrorLabel(m_ui->linePrenom_36, "Immatriculation requise");
    if (m_ui->lineLicenseNum_8)
        m_errLicense = addErrorLabel(m_ui->lineLicenseNum_8, "Numéro de licence requis");
    if (m_ui->linePrenom_38)
        m_errOwner = addErrorLabel(m_ui->linePrenom_38, "Propriétaire requis");
}

void BateauModule::initializeTimers()
{
    m_statsTimer = new QTimer(this);
    connect(m_statsTimer, &QTimer::timeout, this, &BateauModule::updateStatistics);
    m_statsTimer->start(30000);

    m_blinkState = false;
    m_blinkTimer = new QTimer(this);
    connect(m_blinkTimer, &QTimer::timeout, this, &BateauModule::updateBlinkEffect);
    m_blinkTimer->start(500);
}

void BateauModule::loadSampleDataIfEmpty()
{
    if (m_bateauDb)
        return;

    QSqlQuery checkQuery;
    checkQuery.prepare(QStringLiteral("SELECT COUNT(*) FROM %1").arg(sqlBateauTable()));
    if (!checkQuery.exec() || !checkQuery.next() || checkQuery.value(0).toInt() != 0)
        return;
    {
        QStringList types = {"Commerce", "Pêche moins de 12m", "Pêche plus de 12m", "Voilier"};
        QStringList names = {"Neptune", "Poseidon", "Atlantis", "Triton", "Calypso",
                             "Odyssée", "Sirène", "Kraken", "Méduse", "Argonaute"};
        QStringList owners = {"Jean Dupont", "Marie Martin", "Pierre Durand", "Sophie Bernard",
                              "Luc Petit", "Claire Moreau", "Thomas Laurent", "Julie Simon", "Aline Leroy", "Marc Dubois"};

        QSqlQuery insertQuery;
        insertQuery.prepare(
            QStringLiteral("INSERT INTO %1 (id_bateau, nom, type, immatriculation, puissance_moteur, proprietaire, numero_license, date_expiration_licence, assurance, statut) "
                           "VALUES (:id, :nom, :type, :immat, :moteur, :prop, :lic, :exp, :assur, :enmer)")
                .arg(sqlBateauTable()));

        for (int i = 0; i < 10; i++) {
            QString id = QString::number(i + 1);
            insertQuery.bindValue(":id", id);
            insertQuery.bindValue(":nom", names[i]);
            insertQuery.bindValue(":type", types[QRandomGenerator::global()->bounded(types.size())]);
            insertQuery.bindValue(":immat", QString("FR-%1-%2")
                                                .arg(QRandomGenerator::global()->bounded(1000, 9999))
                                                .arg(QRandomGenerator::global()->bounded(100, 999)));
            insertQuery.bindValue(":moteur", QString("%1 CV").arg(QRandomGenerator::global()->bounded(50, 500)));
            insertQuery.bindValue(":prop", owners[i % owners.size()]);
            insertQuery.bindValue(":lic", QString("LIC-%1").arg(QRandomGenerator::global()->bounded(10000, 99999)));
            insertQuery.bindValue(":exp", QDate::currentDate().addDays(QRandomGenerator::global()->bounded(-30, 365)));
            insertQuery.bindValue(":assur", QDate::currentDate().addDays(QRandomGenerator::global()->bounded(-15, 400)));
            insertQuery.bindValue(":enmer", QRandomGenerator::global()->bounded(0, 2));

            if (!insertQuery.exec())
                qDebug() << "Error inserting test boat:" << insertQuery.lastError().text();
        }
        QSqlQuery(QStringLiteral("COMMIT"));
    }
    refreshFromDb();
}

static QDate dateFromOracleVariant(const QVariant &v)
{
    if (v.typeId() == QMetaType::QDate || v.typeId() == QMetaType::QDateTime) {
        QDate d = v.toDate();
        if (d.isValid())
            return d;
        return v.toDateTime().date();
    }
    const QString s = v.toString().trimmed();
    if (s.isEmpty())
        return {};
    QDate d = QDate::fromString(s, QStringLiteral("dd/MM/yyyy"));
    if (d.isValid())
        return d;
    d = QDate::fromString(s, QStringLiteral("yyyy-MM-dd"));
    return d;
}

void BateauModule::refreshFromDb()
{
    m_boats.clear();
    if (m_bateauDb) {
        QSqlQuery query = m_bateauDb->afficher();
        while (query.next()) {
            BateauData boat;
            boat.id = query.value(0).toString();
            boat.name = query.value(1).toString();
            boat.type = query.value(2).toString();
            boat.immatriculation = query.value(3).toString();
            boat.motor = query.value(4).toString();
            boat.owner = query.value(5).toString();
            const QString statutStr = query.value(6).toString();
            boat.isAtSea = statutStr.contains(QStringLiteral("Mer"), Qt::CaseInsensitive);
            boat.licenseNumber = query.value(7).toString();
            boat.licenseExpiry = dateFromOracleVariant(query.value(8));
            boat.insuranceExpiry = dateFromOracleVariant(query.value(11));
            m_boats.append(boat);
        }
    } else {
        QSqlQuery query(QStringLiteral("SELECT * FROM %1").arg(sqlBateauTable()));
        if (!query.exec()) {
            ensureFleetTracksForBoats();
            syncGeolocationViews();
            return;
        }
        while (query.next()) {
            BateauData boat;
            boat.id = query.value(QStringLiteral("ID_BATEAU")).toString();
            boat.name = query.value(QStringLiteral("NOM")).toString();
            boat.type = query.value(QStringLiteral("TYPE")).toString();
            boat.immatriculation = query.value(QStringLiteral("IMMATRICULATION")).toString();
            boat.motor = query.value(QStringLiteral("PUISSANCE_MOTEUR")).toString();
            boat.owner = query.value(QStringLiteral("PROPRIETAIRE")).toString();
            boat.licenseNumber = query.value(QStringLiteral("NUMERO_LICENSE")).toString();
            boat.licenseExpiry = dateFromOracleVariant(query.value(QStringLiteral("DATE_EXPIRATION_LICENCE")));
            boat.insuranceExpiry = dateFromOracleVariant(query.value(QStringLiteral("ASSURANCE")));
            const QVariant st = query.value(QStringLiteral("STATUT"));
            boat.isAtSea = st.typeId() == QMetaType::QString
                               ? st.toString().contains(QStringLiteral("Mer"), Qt::CaseInsensitive)
                               : (st.toInt() == 1);
            m_boats.append(boat);
        }
    }
    ensureFleetTracksForBoats();
    syncGeolocationViews();
}

void BateauModule::updateTable()
{
    m_model->removeRows(0, m_model->rowCount());
    for (const auto &boat : m_boats) {
        QList<QStandardItem *> row;
        QStandardItem *idItem = new QStandardItem();
        idItem->setData(boat.id.toInt(), Qt::DisplayRole);
        row.append(idItem);
        QStandardItem *nameItem = new QStandardItem(boat.name);
        nameItem->setFont(QFont("Segoe UI", 10, QFont::Bold));
        row.append(nameItem);
        row.append(new QStandardItem(boat.type));
        row.append(new QStandardItem(boat.immatriculation));
        row.append(new QStandardItem(boat.motor));
        row.append(new QStandardItem(boat.owner));
        row.append(new QStandardItem(boat.licenseNumber));

        int daysToTech = QDate::currentDate().daysTo(boat.licenseExpiry);
        QStandardItem *techItem = new QStandardItem(boat.licenseExpiry.toString("dd/MM/yyyy"));
        if (daysToTech < 0)
            techItem->setForeground(QBrush(QColor("#e63946")));
        else if (daysToTech < 30)
            techItem->setForeground(QBrush(QColor("#ee9b00")));
        else
            techItem->setForeground(QBrush(QColor("#2ecc71")));
        row.append(techItem);

        int daysToInsur = QDate::currentDate().daysTo(boat.insuranceExpiry);
        QStandardItem *insurItem = new QStandardItem(boat.insuranceExpiry.toString("dd/MM/yyyy"));
        if (daysToInsur < 0)
            insurItem->setForeground(QBrush(QColor("#e63946")));
        else if (daysToInsur < 30)
            insurItem->setForeground(QBrush(QColor("#ee9b00")));
        else
            insurItem->setForeground(QBrush(QColor("#2ecc71")));
        row.append(insurItem);

        QStandardItem *statusItem = new QStandardItem(boat.isAtSea ? "🌊 EN MER" : "⚓ AU PORT");
        statusItem->setFont(QFont("Segoe UI", 9, QFont::Bold));
        statusItem->setForeground(QBrush(boat.isAtSea ? QColor("#3498db") : QColor("#2ecc71")));
        row.append(statusItem);

        m_model->appendRow(row);
    }
}

void BateauModule::updateStatistics()
{
    int total = m_boats.size();
    int atSea = atSeaCount();
    int atPort = total - atSea;

    QStringList alerts;
    int expiringCount = 0;
    int readyCount = 0;
    int dangerCount = 0;
    QDate today = QDate::currentDate();

    for (const auto &boat : m_boats) {
        bool urgent = false;
        QString boatAlerts;

        int daysToTech = today.daysTo(boat.licenseExpiry);
        int daysToInsur = today.daysTo(boat.insuranceExpiry);

        if (!boat.isAtSea && (daysToTech >= 30 && daysToInsur >= 30))
            readyCount++;
        if (boat.isAtSea && (daysToTech < 2 || daysToInsur < 2))
            dangerCount++;

        if (daysToTech < 0) {
            boatAlerts += "• Licence expirée ";
            urgent = true;
        } else if (daysToTech < 30) {
            boatAlerts += QString("• Licence expire dans %1j ").arg(daysToTech);
            urgent = true;
        }

        if (daysToInsur < 0) {
            boatAlerts += "• Assurance expirée ";
            urgent = true;
        } else if (daysToInsur < 30) {
            boatAlerts += QString("• Assurance expire dans %1j ").arg(daysToInsur);
            urgent = true;
        }

        if (urgent) {
            expiringCount++;
            alerts.append(QString("<b>%1 (%2):</b> %3").arg(boat.name).arg(boat.id).arg(boatAlerts.trimmed()));
        }
    }

    if (QLabel *w = findInCentral<QLabel>(m_ui, QStringLiteral("lblCardTotalValue")))
        w->setText(QString::number(total));
    if (QLabel *w = findInCentral<QLabel>(m_ui, QStringLiteral("lblCardAtSeaValue")))
        w->setText(QString::number(atSea));
    if (QLabel *w = findInCentral<QLabel>(m_ui, QStringLiteral("lblCardAtPortValue")))
        w->setText(QString::number(atPort));
    if (QLabel *w = findInCentral<QLabel>(m_ui, QStringLiteral("lblCardAlertsValue")))
        w->setText(QString::number(expiringCount));
    if (QLabel *w = findInCentral<QLabel>(m_ui, QStringLiteral("lblCardReadyValue")))
        w->setText(QString::number(readyCount));
    if (QLabel *w = findInCentral<QLabel>(m_ui, QStringLiteral("lblCardDangerValue")))
        w->setText(QString::number(dangerCount));

    const int activeLic = activeLicenses();
    const int expiredLic = expiredLicenses();
    if (m_ui->lblTotalBateauxCount_8)
        m_ui->lblTotalBateauxCount_8->setText(QString::number(total));
    if (m_ui->lblBateauxMerCount_8)
        m_ui->lblBateauxMerCount_8->setText(QString::number(atSea));
    if (m_ui->lblLicencesActivesCount_8)
        m_ui->lblLicencesActivesCount_8->setText(QString::number(activeLic));
    if (m_ui->lblLicencesExpireesCount_8)
        m_ui->lblLicencesExpireesCount_8->setText(QString::number(expiredLic));
    if (m_ui->textNotifications_8) {
        if (alerts.isEmpty()) {
            m_ui->textNotifications_8->setHtml(
                QStringLiteral("<p style='color:#2ecc71;'>✅ Toutes les certifications sont à jour.</p>"));
        } else {
            m_ui->textNotifications_8->setHtml(alerts.join(QStringLiteral("<br>")));
        }
    }

    if (QLabel *lblAlertsList = findInCentral<QLabel>(m_ui, QStringLiteral("lblAlertsList"))) {
        if (alerts.isEmpty()) {
            lblAlertsList->setTextFormat(Qt::PlainText);
            lblAlertsList->setText(QStringLiteral("✅ Toutes les certifications sont à jour."));
            if (QFrame *panel = findInCentral<QFrame>(m_ui, QStringLiteral("frameAlertsPanel")))
                panel->setStyleSheet(
                    QStringLiteral("QFrame { background-color: #27ae60; border-radius: 10px; color: white; padding: 10px; }"));
        } else {
            lblAlertsList->setTextFormat(Qt::RichText);
            lblAlertsList->setText(alerts.join(QStringLiteral("<br>")));
            if (QFrame *panel = findInCentral<QFrame>(m_ui, QStringLiteral("frameAlertsPanel")))
                panel->setStyleSheet(
                    QStringLiteral("QFrame { background-color: #c0392b; border-radius: 10px; color: white; padding: 10px; }"));
        }
    }

    if (m_chartType) {
        QMap<QString, int> typeCounts;
        for (const auto &boat : m_boats)
            typeCounts[boat.type]++;

        auto *series0 = new QPieSeries();
        series0->setHoleSize(0.35);
        series0->setPieSize(0.7);
        for (auto it = typeCounts.begin(); it != typeCounts.end(); ++it)
            series0->append(it.key(), it.value());

        QList<QColor> colors = {QColor("#3498db"), QColor("#e74c3c"), QColor("#f1c40f"), QColor("#2ecc71")};
        int i = 0;
        for (QPieSlice *slice : series0->slices()) {
            slice->setBrush(colors[i % colors.size()]);
            slice->setBorderColor(Qt::transparent);
            int pct = (total > 0) ? qRound(100.0 * slice->value() / total) : 0;
            slice->setLabel(QString("%1 (%2%)").arg(slice->label()).arg(pct));
            i++;
        }
        QChart *tChart = m_chartType->chart();
        tChart->removeAllSeries();
        tChart->addSeries(series0);
        tChart->setTitle(QString("➡ Répartition par Type (%1 bateaux)").arg(total));
        tChart->setTitleBrush(QBrush(Qt::white));
    }

    if (m_chartStatus) {
        auto *series1 = new QPieSeries();
        series1->setHoleSize(0.35);
        series1->setPieSize(0.7);
        series1->append("🌊 En Mer", atSea);
        series1->append("⚓ Au Port", atPort);

        QList<QColor> statusColors = {QColor("#e67e22"), QColor("#1abc9c")};
        int i = 0;
        for (QPieSlice *slice : series1->slices()) {
            slice->setBrush(statusColors[i % statusColors.size()]);
            slice->setBorderColor(Qt::transparent);
            int pct = (total > 0) ? qRound(100.0 * slice->value() / total) : 0;
            slice->setLabel(QString("%1 (%2%)").arg(slice->label()).arg(pct));
            i++;
        }
        QChart *sChart = m_chartStatus->chart();
        sChart->removeAllSeries();
        sChart->addSeries(series1);
        sChart->setTitle(QString("➡ Statut de Navigation (%1 en mer, %2 au port)").arg(atSea).arg(atPort));
        sChart->setTitleBrush(QBrush(Qt::white));
    }

    if (m_chartLicense) {
        auto *series2 = new QPieSeries();
        series2->setHoleSize(0.35);
        series2->setPieSize(0.7);
        series2->append("Licences valides", activeLic);
        series2->append("Licences expirées", expiredLic);

        QList<QColor> licColors = {QColor("#2ecc71"), QColor("#e74c3c")};
        int i2 = 0;
        int totalLic = activeLic + expiredLic;
        for (QPieSlice *slice : series2->slices()) {
            slice->setBrush(licColors[i2 % licColors.size()]);
            slice->setBorderColor(Qt::transparent);
            int pct = (totalLic > 0) ? qRound(100.0 * slice->value() / totalLic) : 0;
            slice->setLabel(QString("%1 (%2%)").arg(slice->label()).arg(pct));
            i2++;
        }

        QChart *lChart = m_chartLicense->chart();
        lChart->removeAllSeries();
        lChart->addSeries(series2);
        lChart->setTitle(QString("➡ Licences (%1 valides, %2 expirées)").arg(activeLic).arg(expiredLic));
        lChart->setTitleBrush(QBrush(Qt::white));
    }
}

int BateauModule::activeLicenses() const
{
    int count = 0;
    for (const auto &boat : m_boats)
        if (boat.licenseExpiry >= QDate::currentDate())
            count++;
    return count;
}

int BateauModule::expiredLicenses() const
{
    int count = 0;
    for (const auto &boat : m_boats)
        if (boat.licenseExpiry < QDate::currentDate())
            count++;
    return count;
}

int BateauModule::atSeaCount() const
{
    int count = 0;
    for (const auto &boat : m_boats)
        if (boat.isAtSea)
            count++;
    return count;
}

QLabel *BateauModule::addErrorLabel(QWidget *field, const QString &errorText)
{
    QWidget *parent = field->parentWidget();
    QGridLayout *layout = qobject_cast<QGridLayout *>(parent->layout());
    if (!layout)
        return nullptr;

    int index = layout->indexOf(field);
    int r, c, rs, cs;
    layout->getItemPosition(index, &r, &c, &rs, &cs);
    layout->removeWidget(field);

    QWidget *container = new QWidget(parent);
    QVBoxLayout *vbox = new QVBoxLayout(container);
    vbox->setContentsMargins(0, 0, 0, 0);
    vbox->setSpacing(2);

    field->setParent(container);
    vbox->addWidget(field);

    QLabel *label = new QLabel(errorText, container);
    label->setStyleSheet("color: #e63946; font-size: 10px; font-weight: bold; margin-left: 2px;");
    label->hide();
    vbox->addWidget(label);

    layout->addWidget(container, r, c, rs, cs);
    return label;
}

void BateauModule::updateBlinkEffect()
{
    m_blinkState = !m_blinkState;
    QDate current = QDate::currentDate();

    for (int i = 0; i < m_boats.size(); ++i) {
        if (m_boats[i].licenseExpiry < current) {
            QStandardItem *item = m_model->item(i, 7);
            if (item) {
                if (m_blinkState) {
                    item->setForeground(QBrush(QColor("#e63946")));
                    QFont font = m_ui->tableBateaux_2->font();
                    font.setBold(true);
                    item->setFont(font);
                } else {
                    item->setForeground(QBrush(Qt::transparent));
                }
            }
        }
    }
}

void BateauModule::clearForm()
{
    m_ui->lineID_15->clear();
    m_ui->lineLicence_15->clear();
    m_ui->linePrenom_36->clear();
    m_ui->linePrenom_37->clear();
    m_ui->linePrenom_38->clear();
    m_ui->lineLicenseNum_8->clear();
    m_ui->dateExpiry_2->setDate(QDate::currentDate());
    m_ui->comboStatus_2->setCurrentIndex(0);
    if (m_dateInsurance)
        m_dateInsurance->setDate(QDate::currentDate());
    m_ui->btnAjouter_8->setProperty("isEditMode", false);
    m_ui->btnAjouter_8->setText("ajouter");
    if (m_errName)
        m_errName->hide();
    if (m_errImmat)
        m_errImmat->hide();
    if (m_errLicense)
        m_errLicense->hide();
    if (m_errOwner)
        m_errOwner->hide();
}

void BateauModule::onAjouterClicked()
{
    bool isEditMode = m_ui->btnAjouter_8->property("isEditMode").toBool();
    QString currentId;
    if (isEditMode) {
        int row = m_ui->btnAjouter_8->property("editingRow").toInt();
        if (row >= 0 && row < m_boats.size())
            currentId = m_boats[row].id;
    }

    QString newName = m_ui->lineLicence_15->text().trimmed();
    QString newImmat = m_ui->linePrenom_36->text().trimmed();
    QString newOwner = m_ui->linePrenom_38->text().trimmed();
    QString newLicNum = m_ui->lineLicenseNum_8->text().trimmed();
    QString newType = m_ui->comboRole_15->currentText();
    QString newMotor = m_ui->linePrenom_37->text().trimmed();
    QDate newExp = m_ui->dateExpiry_2->date();
    bool newAtSea = (m_ui->comboStatus_2->currentIndex() == 1);

    if (newName.isEmpty() || newImmat.isEmpty() || newOwner.isEmpty() || newLicNum.isEmpty()) {
        QMessageBox::warning(m_dialogParent, "Champs Manquants",
                             "Veuillez remplir tous les champs obligatoires (Nom, Immatriculation, Propriétaire, Licence).");
        if (m_errName)
            m_errName->setVisible(newName.isEmpty());
        if (m_errImmat)
            m_errImmat->setVisible(newImmat.isEmpty());
        if (m_errLicense)
            m_errLicense->setVisible(newLicNum.isEmpty());
        if (m_errOwner)
            m_errOwner->setVisible(newOwner.isEmpty());
        return;
    }

    static QRegularExpression nameRegex("^[a-zA-ZàâäéèêëîïôöùûüÿçÀÂÄÉÈÊËÎÏÔÖÙÛÜŸÇ\\s]+$");
    if (!nameRegex.match(newName).hasMatch()) {
        QMessageBox::warning(m_dialogParent, "Validation Nom", "Le nom du bateau ne doit contenir que des lettres.");
        return;
    }

    static QRegularExpression ownerRegex("^[a-zA-ZàâäéèêëîïôöùûüÿçÀÂÄÉÈÊËÎÏÔÖÙÛÜŸÇ]+\\s+[a-zA-ZàâäéèêëîïôöùûüÿçÀÂÄÉÈÊËÎÏÔÖÙÛÜŸÇ]+.*$");
    if (!ownerRegex.match(newOwner).hasMatch()) {
        QMessageBox::warning(m_dialogParent, "Validation Propriétaire",
                             "Le champ Propriétaire doit contenir le nom ET le prénom.\n"
                             "Exemple : 'Dupont Jean'\n\n"
                             "Veuillez entrer au moins deux mots (nom et prénom).");
        return;
    }

    const QString tbl = sqlBateauTable();
    const QString colLic = m_bateauDb ? QStringLiteral("NUMERO_LICENSE") : QStringLiteral("numero_license");
    const QString colId = m_bateauDb ? QStringLiteral("ID_BATEAU") : QStringLiteral("id_bateau");
    const QString colImmat = m_bateauDb ? QStringLiteral("IMMATRICULATION") : QStringLiteral("immatriculation");

    QSqlQuery checkLic;
    if (isEditMode) {
        checkLic.prepare(QStringLiteral("SELECT COUNT(*) FROM %1 WHERE %2 = :lic AND %3 <> :id").arg(tbl, colLic, colId));
        checkLic.bindValue(":id", currentId);
    } else {
        checkLic.prepare(QStringLiteral("SELECT COUNT(*) FROM %1 WHERE %2 = :lic").arg(tbl, colLic));
    }
    checkLic.bindValue(":lic", newLicNum);
    if (checkLic.exec() && checkLic.next() && checkLic.value(0).toInt() > 0) {
        QMessageBox::warning(m_dialogParent, "Doublon", "Ce numéro de licence existe déjà dans la base de données.");
        return;
    }

    QSqlQuery checkImmat;
    if (isEditMode) {
        checkImmat.prepare(QStringLiteral("SELECT COUNT(*) FROM %1 WHERE %2 = :immat AND %3 <> :id").arg(tbl, colImmat, colId));
        checkImmat.bindValue(":id", currentId);
    } else {
        checkImmat.prepare(QStringLiteral("SELECT COUNT(*) FROM %1 WHERE %2 = :immat").arg(tbl, colImmat));
    }
    checkImmat.bindValue(":immat", newImmat);
    if (checkImmat.exec() && checkImmat.next() && checkImmat.value(0).toInt() > 0) {
        QMessageBox::warning(m_dialogParent, "Doublon", "Cette immatriculation existe déjà dans la base de données.");
        return;
    }

    if (newAtSea) {
        QDate today = QDate::currentDate();
        const QDate assurD = m_dateInsurance ? m_dateInsurance->date() : newExp;
        if (newExp < today || assurD < today) {
            QString raison;
            if (newExp < today)
                raison += "\n- Licence expirée";
            if (assurD < today)
                raison += "\n- Assurance expirée";

            QMessageBox::critical(m_dialogParent, "Sécurité Maritime - Action Refusée",
                                  QString("Le bateau ne peut pas être mis 'En Mer' car il n'est pas en règle de conformité.%1\n\nVeuillez corriger les dates ou changer le statut.").arg(raison));
            return;
        }
    }

    const QString statutStr = newAtSea ? QStringLiteral("En Mer") : QStringLiteral("Au Port");
    const QDate assuranceDate = m_dateInsurance ? m_dateInsurance->date() : newExp;

    if (m_bateauDb) {
        bool ok = false;
        if (isEditMode) {
            ok = m_bateauDb->modifier(currentId.toInt(), newName, newType, newImmat, newMotor, newOwner, statutStr, newLicNum, newExp, -1, -1,
                                      assuranceDate);
        } else {
            ok = m_bateauDb->ajouter(0, newName, newType, newImmat, newMotor, newOwner, statutStr, newLicNum, newExp, -1, -1, assuranceDate);
        }
        if (ok) {
            QSqlQuery(QStringLiteral("COMMIT"));
            refreshFromDb();
            updateTable();
            updateStatistics();
            clearForm();
            QMessageBox::information(m_dialogParent, "Succès", isEditMode ? "Bateau modifié avec succès." : "Bateau ajouté avec succès.");
        } else {
            QMessageBox::critical(m_dialogParent, "Erreur DB",
                                  m_bateauDb->lastError().isEmpty() ? QStringLiteral("L'opération a échoué.") : m_bateauDb->lastError());
        }
        return;
    }

    QSqlQuery query;
    if (isEditMode) {
        query.prepare(QStringLiteral("UPDATE %1 SET nom=:nom, type=:type, immatriculation=:immat, puissance_moteur=:moteur, proprietaire=:prop, numero_license=:lic, date_expiration_licence=:exp, assurance=:assur, statut=:enmer WHERE id_bateau=:id").arg(tbl));
        query.bindValue(":id", currentId);
    } else {
        int maxId = 0;
        for (const auto &b : m_boats) {
            int currentIdVal = b.id.toInt();
            if (currentIdVal > maxId)
                maxId = currentIdVal;
        }
        query.prepare(QStringLiteral("INSERT INTO %1 (id_bateau, nom, type, immatriculation, puissance_moteur, proprietaire, numero_license, date_expiration_licence, assurance, statut) "
                                     "VALUES (:id, :nom, :type, :immat, :moteur, :prop, :lic, :exp, :assur, :enmer)")
                          .arg(tbl));
        query.bindValue(":id", QString::number(maxId + 1));
    }

    query.bindValue(":nom", newName);
    query.bindValue(":type", newType);
    query.bindValue(":immat", newImmat);
    query.bindValue(":moteur", newMotor);
    query.bindValue(":prop", newOwner);
    query.bindValue(":lic", newLicNum);
    query.bindValue(":exp", newExp);
    query.bindValue(":assur", assuranceDate);
    query.bindValue(":enmer", newAtSea ? 1 : 0);

    if (query.exec()) {
        QSqlQuery(QStringLiteral("COMMIT"));
        refreshFromDb();
        updateTable();
        updateStatistics();
        clearForm();
        QMessageBox::information(m_dialogParent, "Succès", isEditMode ? "Bateau modifié avec succès." : "Bateau ajouté avec succès.");
    } else {
        QMessageBox::critical(m_dialogParent, "Erreur DB", "L'opération a échoué: " + query.lastError().text());
    }
}

void BateauModule::onModifierClicked()
{
    QModelIndex index = m_ui->tableBateaux_2->currentIndex();
    if (!index.isValid()) {
        QMessageBox::warning(m_dialogParent, "Aucune sélection", "Veuillez sélectionner un bateau à modifier.");
        return;
    }
    int row = index.row();
    m_ui->lineID_15->setText(m_boats[row].id);
    m_ui->lineLicence_15->setText(m_boats[row].name);
    m_ui->linePrenom_36->setText(m_boats[row].immatriculation);
    m_ui->linePrenom_37->setText(m_boats[row].motor);
    m_ui->linePrenom_38->setText(m_boats[row].owner);
    m_ui->lineLicenseNum_8->setText(m_boats[row].licenseNumber);
    m_ui->dateExpiry_2->setDate(m_boats[row].licenseExpiry);
    if (m_dateInsurance)
        m_dateInsurance->setDate(m_boats[row].insuranceExpiry);

    int typeIdx = m_ui->comboRole_15->findText(m_boats[row].type, Qt::MatchStartsWith);
    if (typeIdx >= 0)
        m_ui->comboRole_15->setCurrentIndex(typeIdx);

    m_ui->comboStatus_2->setCurrentIndex(m_boats[row].isAtSea ? 1 : 0);
    m_ui->btnAjouter_8->setProperty("isEditMode", true);
    m_ui->btnAjouter_8->setProperty("editingRow", row);
    m_ui->btnAjouter_8->setText("Confirmer");
}

void BateauModule::onSupprimerClicked()
{
    QModelIndex index = m_ui->tableBateaux_2->currentIndex();
    if (!index.isValid()) {
        QMessageBox::warning(m_dialogParent, "Aucune sélection", "Veuillez sélectionner un bateau à supprimer.");
        return;
    }
    const int selectedRow = index.row();
    if (selectedRow < 0 || selectedRow >= m_boats.size())
        return;

    const QString boatId = m_boats[selectedRow].id;
    if (QMessageBox::question(m_dialogParent, tr("Confirmer"), tr("Supprimer ce bateau ?"),
                              QMessageBox::Yes | QMessageBox::No)
        != QMessageBox::Yes)
        return;

    bool ok = false;
    QString err;
    if (m_bateauDb) {
        ok = m_bateauDb->supprimer(boatId.toInt());
        err = m_bateauDb->lastError();
        if (ok)
            QSqlQuery(QStringLiteral("COMMIT"));
    } else {
        QSqlQuery query;
        query.prepare(QStringLiteral("DELETE FROM %1 WHERE id_bateau = :id").arg(sqlBateauTable()));
        query.bindValue(":id", boatId);
        ok = query.exec();
        if (ok)
            QSqlQuery(QStringLiteral("COMMIT"));
        else
            err = query.lastError().text();
    }

    if (ok) {
        refreshFromDb();
        updateTable();
        updateStatistics();
        QMessageBox::information(m_dialogParent, "Succès", "Bateau supprimé avec succès.");
    } else {
        QMessageBox::critical(m_dialogParent, "Erreur DB", err.isEmpty() ? QStringLiteral("Suppression impossible.") : err);
    }
}

void BateauModule::onExportPdfClicked()
{
    QString fileName = QFileDialog::getSaveFileName(m_dialogParent, "Exporter en PDF", "", "PDF Files (*.pdf)");
    if (!fileName.isEmpty()) {
        QPrinter printer(QPrinter::HighResolution);
        printer.setOutputFormat(QPrinter::PdfFormat);
        printer.setOutputFileName(fileName);

        QTextDocument doc;
        QString html = "<h1 style='text-align: center; color: #2c3e50;'>Liste des Bateaux - SmartPort</h1>";
        html += "<table border='1' cellpadding='10' cellspacing='0' style='width: 100%; border-collapse: collapse;'>";
        html += "<tr style='background-color: #3498db; color: white;'><th>ID</th><th>Nom</th><th>Type</th><th>Propriétaire</th><th>Immatriculation</th><th>Moteur</th></tr>";

        for (const auto &boat : m_boats) {
            html += QString("<tr><td>%1</td><td>%2</td><td>%3</td><td>%4</td><td>%5</td><td>%6</td></tr>")
                        .arg(boat.id, boat.name, boat.type, boat.owner, boat.immatriculation, boat.motor);
        }

        html += "</table>";
        doc.setHtml(html);
        doc.print(&printer);

        QMessageBox::information(m_dialogParent, "Export PDF", "La liste des bateaux a été exportée avec succès.");
    }
}

void BateauModule::onRechercheTextChanged(const QString &text)
{
    for (int i = 0; i < m_model->rowCount(); i++) {
        bool match = false;
        for (int j = 0; j < m_model->columnCount(); j++) {
            QStandardItem *item = m_model->item(i, j);
            if (item && item->text().contains(text, Qt::CaseInsensitive)) {
                match = true;
                break;
            }
        }
        m_ui->tableBateaux_2->setRowHidden(i, !match);
    }
}

void BateauModule::onTriIndexChanged(int index)
{
    if (!m_model || m_boats.isEmpty())
        return;

    switch (index) {
    case 0:
        std::sort(m_boats.begin(), m_boats.end(), [](const BateauData &a, const BateauData &b) {
            return a.id.toInt() < b.id.toInt();
        });
        break;
    case 1:
        std::sort(m_boats.begin(), m_boats.end(), [](const BateauData &a, const BateauData &b) {
            return a.type.toLower() < b.type.toLower();
        });
        break;
    case 2:
        std::sort(m_boats.begin(), m_boats.end(), [](const BateauData &a, const BateauData &b) {
            return a.owner.toLower() < b.owner.toLower();
        });
        break;
    case 3:
        std::sort(m_boats.begin(), m_boats.end(), [](const BateauData &a, const BateauData &b) {
            int cvA = a.motor.section(' ', 0, 0).toInt();
            int cvB = b.motor.section(' ', 0, 0).toInt();
            return cvA < cvB;
        });
        break;
    default:
        std::sort(m_boats.begin(), m_boats.end(), [](const BateauData &a, const BateauData &b) {
            return a.id.toInt() < b.id.toInt();
        });
        break;
    }

    updateTable();
}

void BateauModule::onIASendClicked()
{
}

void BateauModule::onIAReportClicked()
{
}

void BateauModule::onBoatRapportClicked()
{
    QString boatId;

    QModelIndex current = m_ui->tableBateaux_2->currentIndex();
    if (current.isValid()) {
        boatId = m_ui->tableBateaux_2->model()->index(current.row(), 0).data().toString();
    } else if (!m_boats.isEmpty()) {
        boatId = m_boats[0].id;
    }

    QTextEdit *const chatHistoryIA =
        m_ui->tabWidget_2 ? m_ui->tabWidget_2->findChild<QTextEdit *>(QStringLiteral("chatHistoryIA")) : nullptr;
    QWidget *const tabAssistantIABateau =
        m_ui->tabWidget_2 ? m_ui->tabWidget_2->findChild<QWidget *>(QStringLiteral("tabAssistantIABateau")) : nullptr;

    auto showHtml = [this, chatHistoryIA, tabAssistantIABateau](const QString &html, bool isError) {
        if (chatHistoryIA && m_ui->tabWidget_2 && tabAssistantIABateau) {
            chatHistoryIA->setHtml(html);
            m_ui->tabWidget_2->setCurrentWidget(tabAssistantIABateau);
        } else if (m_ui->textNotifications_8) {
            m_ui->textNotifications_8->setHtml(html);
        } else {
            QTextDocument doc;
            doc.setHtml(html);
            const QString plain = doc.toPlainText().trimmed();
            if (isError)
                QMessageBox::warning(m_dialogParent, tr("Rapport bateau"), plain.isEmpty() ? tr("Action impossible.") : plain);
            else
                QMessageBox::information(m_dialogParent, tr("Rapport bateau"), plain.isEmpty() ? tr("Rapport généré.") : plain);
        }
    };

    if (boatId.isEmpty()) {
        showHtml(QStringLiteral("<span style='color:#e74c3c;'>Veuillez sélectionner un bateau dans la liste pour générer un rapport.</span>"),
                 true);
        return;
    }

    const QString report = generateAiReport(boatId);

    const QString wrapped =
        QStringLiteral(
            "<div style='margin-bottom:26px; text-align:left;'>"
            "  <div style='background-color:#232323; color:#ffffff; padding:16px 18px; border-radius:18px; "
            "              display:inline-block; max-width:100%; font-family:\"Segoe UI\", sans-serif; "
            "              line-height:1.6; border:1px solid #3498db; box-shadow:0 4px 10px rgba(0,0,0,0.45);'>"
            "    <div style='color:#f1c40f; font-size:11px; margin-bottom:8px; font-weight:600;'>💡 Smart Analytics</div>"
            "    %1"
            "  </div>"
            "</div>")
            .arg(report);

    showHtml(wrapped, false);
}

QString BateauModule::generateAiReport(const QString &boatId)
{
    BateauData target;
    bool found = false;
    for (const auto &b : m_boats) {
        if (b.id == boatId) {
            target = b;
            found = true;
            break;
        }
    }

    if (!found) {
        return QString("<span style='color: #e74c3c;'>❌ Erreur : Le bateau ID %1 est introuvable dans la base ATLAS.</span>").arg(boatId);
    }

    QDate today = QDate::currentDate();
    int daysToInsur = today.daysTo(target.insuranceExpiry);
    int daysToLic = today.daysTo(target.licenseExpiry);

    int hpValue = 100;
    QRegularExpression re("(\\d+)");
    QRegularExpressionMatch m = re.match(target.motor);
    if (m.hasMatch())
        hpValue = m.captured(1).toInt();

    double hourlyConsum = hpValue * 0.18;
    double currentSeaHours = target.isAtSea ? 14.5 : 0.0;
    double totalFuelEst = currentSeaHours * hourlyConsum;
    QDate nextService = today.addMonths(4);
    if (hpValue > 250)
        nextService = today.addMonths(2);

    QString report = "<b style='font-size: 14px; color: #3498db;'>Rapport ATLAS - " + target.name.toUpper() + " (" + target.id + ")</b><br><br>";

    const BateauSimTrack trk = m_simByBoatId.value(target.id);
    if (target.isAtSea && trk.initialized) {
        report += " <b>Position (WGS84 simulée) :</b> " + QString::number(trk.lat, 'f', 5) + "°N, "
                  + QString::number(trk.lon, 'f', 5) + "°E — cap " + QString::number(trk.headingDeg, 'f', 0)
                  + "°, SOG " + QString::number(trk.sogKnots, 'f', 1) + " kn<br><br>";
    }

    report += " <b>Synthèse :</b> Navire " + target.type + " ";
    report += (hpValue > 100 ? "à forte propulsion (<b>" + target.motor + "</b>)." : "à motorisation standard (<b>" + target.motor + "</b>).");
    report += " Propriété de <b>" + target.owner + "</b>.<br><br>";

    report += " <b>Performance :</b><br>";
    report += "• Consommation estimée : " + QString::number(hourlyConsum, 'f', 1) + " L/h<br>";
    if (target.isAtSea)
        report += "• Mission : <span style='color: #f1c40f;'>" + QString::number(totalFuelEst, 'f', 1) + " L consommés</span><br>";
    report += "• Prochaine révision : " + nextService.toString("dd/MM/yyyy") + "<br><br>";

    report += " <b>Conformité :</b><br>";
    if (daysToLic < 0)
        report += "• <span style='color: #ff4757;'>Licence expirée (Action Requise)</span><br>";
    else
        report += "• Licence valide jusqu'au " + target.licenseExpiry.toString("dd/MM/yyyy") + "<br>";

    if (daysToInsur < 0)
        report += "• <span style='color: #ff4757;'>Assurance expirée (Risque Critique)</span><br>";
    else
        report += "• Assurance valide (" + QString::number(daysToInsur) + " jours restants)<br><br>";

    report += "🔍 <b>Diagnostic final :</b><br>";
    if (target.isAtSea) {
        if (daysToLic < 7 || daysToInsur < 7)
            report += "⚠️ <span style='color: #ffa502;'>Risque élevé. Retour au port conseillé immédiatement.</span>";
        else
            report += " Navigation stable. Paramètres nominaux.";
    } else {
        if (daysToLic > 30 && daysToInsur > 30)
            report += " Navire opérationnel. Prêt pour départ.";
        else
            report += "🔧 Maintenance administrative ou technique nécessaire.";
    }

    report += "<br><br><i style='color: #888; font-size: 10px;'>Généré le " + today.toString("dd/MM/yyyy") + "</i>";

    return report;
}

void BateauModule::applyGpsPositionUi(double lat, double lon, double heading)
{
    persistEsp32GpsForBoat1(lat, lon, heading);
    if (m_gpsWorldMap)
        m_gpsWorldMap->setGpsPosition(lat, lon, heading);
    if (m_lblGeoHover) {
        m_lblGeoHover->setText(
            QStringLiteral("<b>GPS Wi‑Fi (TCP) — bateau 1 (ID %1)</b><br>Trame <b>ATLAS_GPS</b> ou <b>NMEA</b> — "
                           "Latitude : %2 °<br>Longitude : %3 °<br>Cap : %4 °")
                .arg(kEsp32GpsBoatId)
                .arg(lat, 0, 'f', 6)
                .arg(lon, 0, 'f', 6)
                .arg(heading, 0, 'f', 1));
    }
    if (m_lblGeoWgs84) {
        m_lblGeoWgs84->setText(
            QStringLiteral("Bateau 1 — position <b>réelle</b> (Wi‑Fi) : %1 °N, %2 °E — %3 · enregistrée en base")
                .arg(lat, 0, 'f', 6)
                .arg(lon, 0, 'f', 6)
                .arg(QTime::currentTime().toString(QStringLiteral("HH:mm:ss"))));
    }
}

void BateauModule::onGpsLinkOpened()
{
    if (m_bateauDb)
        loadGpsFromDbOntoMap();
    else if (m_gpsWorldMap)
        m_gpsWorldMap->clearLiveGps();
    if (m_lblGeoWgs84) {
        m_lblGeoWgs84->setText(
            QStringLiteral("Liaison active — en attente de position (ATLAS_GPS ou NMEA RMC/GGA)…"));
    }
}

void BateauModule::persistEsp32GpsForBoat1(double lat, double lon, double capDeg)
{
    if (!m_bateauDb)
        return;
    constexpr double kMinM = 12.0;
    if (!m_bateauDb->updateBateauGps(kEsp32GpsBoatId, lat, lon, capDeg))
        qDebug() << "BateauModule::persistEsp32GpsForBoat1 UPDATE:" << m_bateauDb->lastError();

    const bool first = !m_gpsTracePrimed;
    const double d = first ? kMinM * 10
                           : haversineMetersPersist(m_lastTracePersistLat, m_lastTracePersistLon, lat, lon);
    if (first || d >= kMinM) {
        if (m_bateauDb->appendGpsTracePoint(kEsp32GpsBoatId, lat, lon)) {
            m_lastTracePersistLat = lat;
            m_lastTracePersistLon = lon;
            m_gpsTracePrimed = true;
        } else {
            qDebug() << "BateauModule::persistEsp32GpsForBoat1 TRACE:" << m_bateauDb->lastError();
        }
    }
}

void BateauModule::loadGpsFromDbOntoMap()
{
    if (!m_gpsWorldMap || !m_bateauDb)
        return;

    QVector<QPointF> trail = m_bateauDb->loadGpsTrace(kEsp32GpsBoatId, 2000);
    double lat = 0;
    double lon = 0;
    double cap = 0;
    bool haveBoat = m_bateauDb->loadBateauGps(kEsp32GpsBoatId, &lat, &lon, &cap);
    if (!haveBoat && !trail.isEmpty()) {
        lat = trail.last().y();
        lon = trail.last().x();
        haveBoat = true;
    }

    if (!trail.isEmpty()) {
        m_lastTracePersistLat = trail.last().y();
        m_lastTracePersistLon = trail.last().x();
        m_gpsTracePrimed = true;
    } else if (haveBoat) {
        m_lastTracePersistLat = lat;
        m_lastTracePersistLon = lon;
        m_gpsTracePrimed = true;
    } else {
        m_gpsTracePrimed = false;
    }

    if (haveBoat || !trail.isEmpty())
        m_gpsWorldMap->loadPersistedGpsState(trail, haveBoat, lat, lon, cap);
    else
        m_gpsWorldMap->clearLiveGps();
}

void BateauModule::setupGeolocationTab()
{
    if (m_geoTabReady || !m_ui->tabWidget_2)
        return;

    m_geoPage = new QWidget(m_ui->tabWidget_2);
    auto *v = new QVBoxLayout(m_geoPage);
    v->setContentsMargins(12, 12, 12, 12);
    v->setSpacing(8);

    m_lblGeoWgs84 = new QLabel(QStringLiteral(
        "Bateau 1 — connectez le <b>PC au Wi‑Fi « TT-ALHN-8E64-2.4 »</b>, puis liaison TCP vers l’ESP32 (ATLAS_GPS / NMEA)."));
    m_lblGeoWgs84->setStyleSheet(QStringLiteral("color:#34495e; font-size:11px;"));

    m_geoHoverHelpHtml = QStringLiteral(
        "<span style='color:#7f8c8d'>L’ESP32 rejoint le même réseau (identifiants dans le sketch <code>esp32_gps_wifi.ino</code>). "
        "IP par défaut côté PC : <b>192.168.1.188</b>, port <b>3333</b> — modifiez si votre box utilise une autre plage "
        "(ex. 192.168.0.x). Sans TCP : position <b>simulée</b>. Les messages <b>TCP</b> s’affichent dans le journal ci‑dessous.</span>");
    m_lblGeoHover = new QLabel(m_geoHoverHelpHtml);
    m_lblGeoHover->setWordWrap(true);
    m_lblGeoHover->setTextFormat(Qt::RichText);
    m_lblGeoHover->setMinimumHeight(52);
    m_lblGeoHover->setStyleSheet(QStringLiteral("background:#ecf0f1; border-radius:8px; padding:8px;"));

    auto *rowWifi = new QHBoxLayout();
    rowWifi->setSpacing(8);
    m_editGpsWifiHost = new QLineEdit(m_geoPage);
    m_editGpsWifiHost->setPlaceholderText(QStringLiteral("192.168.1.188"));
    m_spinGpsWifiPort = new QSpinBox(m_geoPage);
    m_spinGpsWifiPort->setRange(1, 65535);
    m_spinGpsWifiPort->setValue(3333);
    m_spinGpsWifiPort->setMinimumWidth(90);
    m_btnGpsWifiConnect = new QPushButton(QStringLiteral("Connecter"), m_geoPage);
    m_btnGpsWifiDisconnect = new QPushButton(QStringLiteral("Déconnecter"), m_geoPage);
    m_btnGpsClearTrail = new QPushButton(QStringLiteral("Réinitialiser la trace"), m_geoPage);
    m_btnGpsClearTrail->setToolTip(
        QStringLiteral("Efface la ligne bleue sur la carte (la position GPS reste stockée dans BATEAU)."));
    {
        QSettings gpsSet;
        gpsSet.beginGroup(QStringLiteral("Esp32Gps"));
        m_editGpsWifiHost->setText(
            gpsSet.value(QStringLiteral("wifiHost"), QStringLiteral("192.168.1.188")).toString());
        m_spinGpsWifiPort->setValue(gpsSet.value(QStringLiteral("wifiPort"), 3333).toInt());
        gpsSet.endGroup();
    }
    rowWifi->addWidget(new QLabel(QStringLiteral("IP ESP32 :"), m_geoPage));
    rowWifi->addWidget(m_editGpsWifiHost, 1);
    rowWifi->addWidget(new QLabel(QStringLiteral("Port TCP :"), m_geoPage));
    rowWifi->addWidget(m_spinGpsWifiPort);
    rowWifi->addWidget(m_btnGpsWifiConnect);
    rowWifi->addWidget(m_btnGpsWifiDisconnect);
    rowWifi->addWidget(m_btnGpsClearTrail);

    connect(m_btnGpsWifiConnect, &QPushButton::clicked, this, [this]() {
        if (!m_gpsTcp || !m_editGpsWifiHost || !m_spinGpsWifiPort)
            return;
        QSettings gpsSet;
        gpsSet.beginGroup(QStringLiteral("Esp32Gps"));
        gpsSet.setValue(QStringLiteral("wifiHost"), m_editGpsWifiHost->text().trimmed());
        gpsSet.setValue(QStringLiteral("wifiPort"), m_spinGpsWifiPort->value());
        gpsSet.endGroup();
        const QString host = m_editGpsWifiHost->text().trimmed();
        if (host.isEmpty()) {
            if (m_lblGeoWgs84)
                m_lblGeoWgs84->setText(
                    QStringLiteral("Indiquez l’adresse IP de l’ESP32 sur le réseau TT-ALHN-8E64-2.4."));
            return;
        }
        m_gpsTcp->connectToTcpHost(host, quint16(m_spinGpsWifiPort->value()));
    });
    connect(m_btnGpsWifiDisconnect, &QPushButton::clicked, this, [this]() {
        if (m_gpsTcp)
            m_gpsTcp->disconnectDevice();
    });
    connect(m_btnGpsClearTrail, &QPushButton::clicked, this, [this]() {
        if (m_gpsWorldMap)
            m_gpsWorldMap->clearTrail();
        m_gpsTracePrimed = false;
        m_lastTracePersistLat = 0.0;
        m_lastTracePersistLon = 0.0;
        if (m_bateauDb && !m_bateauDb->clearGpsTrace(kEsp32GpsBoatId))
            qDebug() << "clearGpsTrace:" << m_bateauDb->lastError();
        if (m_txtAisLog) {
            m_txtAisLog->moveCursor(QTextCursor::End);
            m_txtAisLog->insertPlainText(
                QStringLiteral("[%1] Trace GPS (ligne bleue) effacée de l’affichage.\n")
                    .arg(QTime::currentTime().toString(QStringLiteral("HH:mm:ss"))));
            m_txtAisLog->moveCursor(QTextCursor::End);
        }
    });

    m_txtAisLog = new QPlainTextEdit(m_geoPage);
    m_txtAisLog->setReadOnly(true);
    m_txtAisLog->setFixedHeight(96);
    m_txtAisLog->setPlaceholderText(QStringLiteral("Journal liaison ESP32 (trames brutes)…"));
    m_txtAisLog->setStyleSheet(
        QStringLiteral("QPlainTextEdit { background:#1a1a2e; color:#ecf0f1; font-family: Consolas, monospace; font-size:10px; border:1px solid #3498db; border-radius:6px; }"));

    m_gpsWorldMap = new GpsWorldMapWidget(m_geoPage);
    m_gpsWorldMap->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_gpsWorldMap->setMinimumHeight(320);

    m_gpsTcp = new Esp32GpsTcpClient(this);

    connect(m_gpsTcp, &Esp32GpsTcpClient::positionUpdated, this, &BateauModule::applyGpsPositionUi);
    connect(m_gpsTcp, &Esp32GpsTcpClient::tcpLinkOpened, this, &BateauModule::onGpsLinkOpened);
    connect(m_gpsTcp, &Esp32GpsTcpClient::statusMessage, this, [this](const QString &msg) {
        if (!m_txtAisLog)
            return;
        m_txtAisLog->moveCursor(QTextCursor::Start);
        m_txtAisLog->insertPlainText(QStringLiteral("[TCP] %1\n").arg(msg));
        m_txtAisLog->moveCursor(QTextCursor::End);
    });
    connect(m_gpsTcp, &Esp32GpsTcpClient::logLine, this, [this](const QString &line) {
        if (!m_txtAisLog)
            return;
        m_txtAisLog->moveCursor(QTextCursor::End);
        m_txtAisLog->insertPlainText(line + QLatin1Char('\n'));
        m_txtAisLog->moveCursor(QTextCursor::End);
    });

    v->addWidget(m_gpsWorldMap, 1);
    v->addWidget(m_lblGeoWgs84);
    v->addWidget(m_lblGeoHover);
    v->addLayout(rowWifi);
    v->addWidget(m_txtAisLog);

    // Index 1 = after « Information Bateaux » (onglet Historique retiré du .ui)
    m_ui->tabWidget_2->insertTab(1, m_geoPage, QStringLiteral("Géolocalisation"));

    m_fleetTimer = new QTimer(this);
    connect(m_fleetTimer, &QTimer::timeout, this, &BateauModule::advanceFleetSimulation);
    m_fleetTimer->start(2000);

    m_geoTabReady = true;
    loadGpsFromDbOntoMap();
    syncGeolocationViews();

    /* Même réseau Wi‑Fi que l’ESP32 : tentative TCP automatique (IP / port depuis QSettings). */
    QTimer::singleShot(400, this, [this]() {
        if (!m_gpsTcp || !m_editGpsWifiHost || !m_spinGpsWifiPort)
            return;
        const QString host = m_editGpsWifiHost->text().trimmed();
        if (host.isEmpty())
            return;
        m_gpsTcp->connectToTcpHost(host, quint16(m_spinGpsWifiPort->value()));
    });
}

void BateauModule::ensureFleetTracksForBoats()
{
    QSet<QString> alive;
    for (const auto &b : m_boats)
        alive.insert(b.id);

    for (auto it = m_simByBoatId.begin(); it != m_simByBoatId.end();) {
        if (!alive.contains(it.key()))
            it = m_simByBoatId.erase(it);
        else
            ++it;
    }

    int portSlot = 0;
    for (const auto &b : m_boats) {
        BateauSimTrack &st = m_simByBoatId[b.id];
        const bool reseed = !st.initialized || (st.lastDbAtSea != b.isAtSea);
        if (reseed) {
            const int berth = b.isAtSea ? 0 : portSlot++;
            Geo::seedSimTrack(st, b, berth);
        }
    }
}

void BateauModule::syncGeolocationViews()
{
    if (!m_geoTabReady)
        return;

    m_trackSnapshots.clear();
    m_trackSnapshots.reserve(m_boats.size());
    for (const auto &b : m_boats) {
        const BateauSimTrack st = m_simByBoatId.value(b.id);
        BateauTrackSnapshot sn;
        sn.id = b.id;
        sn.name = b.name;
        sn.type = b.type;
        sn.immat = b.immatriculation;
        sn.lat = st.lat;
        sn.lon = st.lon;
        sn.headingDeg = st.headingDeg;
        sn.sogKnots = st.sogKnots;
        sn.atSea = b.isAtSea;
        sn.lastAisEpochMs = st.lastAisEpochMs;
        m_trackSnapshots.append(sn);
    }

    if (m_lblGeoHover && !wifiGpsLive(m_gpsTcp)) {
        const QString metier = evaluateFleetMetierAlerts();
        QString color = QStringLiteral("#27ae60");
        if (metier.contains(QStringLiteral("🚨")))
            color = QStringLiteral("#c0392b");
        else if (metier.contains(QStringLiteral("⚠️")))
            color = QStringLiteral("#d35400");
        m_lblGeoHover->setText(
            QStringLiteral("<div style=\"font-weight:bold;color:%1\">%2</div><div style=\"margin-top:8px;\">%3</div>")
                .arg(color, metier.toHtmlEscaped(), m_geoHoverHelpHtml));
    }
}

void BateauModule::advanceFleetSimulation()
{
    if (!m_geoTabReady || m_boats.isEmpty())
        return;

    constexpr double dtSec = 2.0;
    constexpr double dtH = dtSec / 3600.0;

    for (const auto &b : m_boats) {
        if (!m_simByBoatId.contains(b.id))
            continue;
        BateauSimTrack &st = m_simByBoatId[b.id];

        if (!b.isAtSea) {
            st.sogKnots = qMax(0.0, st.sogKnots * 0.85);
            st.lastAisEpochMs = QDateTime::currentMSecsSinceEpoch();
            continue;
        }

        const double h = qDegreesToRadians(st.headingDeg);
        const double distNm = st.sogKnots * dtH;
        st.lat += (distNm / 60.0) * qCos(h);
        st.lon += (distNm / 60.0) * qSin(h) / qMax(0.25, qCos(qDegreesToRadians(st.lat)));

        if (st.lat < Geo::kMinLat + 0.015 || st.lat > Geo::kMaxLat - 0.015
            || st.lon < Geo::kMinLon + 0.015 || st.lon > Geo::kMaxLon - 0.015) {
            st.headingDeg = std::fmod(st.headingDeg + 140.0 + QRandomGenerator::global()->bounded(60), 360.0);
            st.lat = qBound(Geo::kMinLat + 0.02, st.lat, Geo::kMaxLat - 0.02);
            st.lon = qBound(Geo::kMinLon + 0.02, st.lon, Geo::kMaxLon - 0.02);
        }

        if (QRandomGenerator::global()->bounded(25) == 0)
            st.headingDeg = std::fmod(st.headingDeg + QRandomGenerator::global()->bounded(17) - 8.0 + 360.0, 360.0);

        st.lastAisEpochMs = QDateTime::currentMSecsSinceEpoch();
    }

    syncGeolocationViews();

    /* Sans ESP32 : afficher sur la carte la position simulée du bateau 1 (flèche rouge + piste bleue). */
    if (m_gpsWorldMap && !wifiGpsLive(m_gpsTcp)) {
        const QString targetId = QString::number(kEsp32GpsBoatId);
        const BateauData *boatPtr = nullptr;
        for (const auto &b : m_boats) {
            if (b.id == targetId) {
                boatPtr = &b;
                break;
            }
        }
        if (!boatPtr && !m_boats.isEmpty())
            boatPtr = &m_boats.first();
        if (boatPtr) {
            const BateauSimTrack st = m_simByBoatId.value(boatPtr->id);
            if (st.initialized) {
                m_gpsWorldMap->setGpsPosition(st.lat, st.lon, st.headingDeg);
                if (m_lblGeoWgs84) {
                    m_lblGeoWgs84->setText(
                        QStringLiteral("Bateau 1 — position <b>simulée</b> (démo sans liaison TCP vers l’ESP32) : %1 °N, %2 °E — %3")
                            .arg(st.lat, 0, 'f', 5)
                            .arg(st.lon, 0, 'f', 5)
                            .arg(QTime::currentTime().toString(QStringLiteral("HH:mm:ss"))));
                }
                if (m_lblGeoHover) {
                    m_lblGeoHover->setText(
                        QStringLiteral("<span style='color:#7f8c8d'>Les lignes <b>!AIVDM demo</b> du journal ne sont <b>pas</b> le flux GPS. "
                                       "Connectez le TCP (Wi‑Fi) pour recevoir <b>ATLAS_GPS,…</b></span>"));
                }
            }
        }
    }

    if (m_txtAisLog && wifiGpsLive(m_gpsTcp))
        return;

    if (m_txtAisLog && QRandomGenerator::global()->bounded(3) == 0) {
        int idx = QRandomGenerator::global()->bounded(m_boats.size());
        const BateauData &b = m_boats.at(idx);
        const BateauSimTrack st = m_simByBoatId.value(b.id);
        const QString line = QStringLiteral("[%1] !AIVDM demo — %2 | %3°N %4°E | cap %5° | %6 kn | %7\n")
                                 .arg(QTime::currentTime().toString(QStringLiteral("HH:mm:ss")))
                                 .arg(b.immatriculation)
                                 .arg(st.lat, 0, 'f', 5)
                                 .arg(st.lon, 0, 'f', 5)
                                 .arg(st.headingDeg, 0, 'f', 0)
                                 .arg(st.sogKnots, 0, 'f', 1)
                                 .arg(b.isAtSea ? QStringLiteral("EN MER") : QStringLiteral("À QUAI"));
        m_txtAisLog->moveCursor(QTextCursor::End);
        m_txtAisLog->insertPlainText(line);
        m_txtAisLog->moveCursor(QTextCursor::End);
    }
}

QString BateauModule::evaluateFleetMetierAlerts() const
{
    QStringList parts;
    int atSea = 0;
    for (const auto &t : m_trackSnapshots) {
        if (t.atSea)
            ++atSea;
    }
    parts.append(QStringLiteral("🛰 Flotte : %1 navire(s) en mer sur %2 suivis.")
                       .arg(atSea)
                       .arg(m_trackSnapshots.size()));

    const double proxDeg = 0.022;
    QStringList prox;
    for (int i = 0; i < m_trackSnapshots.size(); ++i) {
        if (!m_trackSnapshots.at(i).atSea)
            continue;
        for (int j = i + 1; j < m_trackSnapshots.size(); ++j) {
            if (!m_trackSnapshots.at(j).atSea)
                continue;
            const auto &a = m_trackSnapshots.at(i);
            const auto &b = m_trackSnapshots.at(j);
            const double dlat = a.lat - b.lat;
            const double dlon = a.lon - b.lon;
            const double d = std::sqrt(dlat * dlat + dlon * dlon);
            if (d < proxDeg)
                prox.append(QStringLiteral("%1 ↔ %2 (~%3 m)")
                                .arg(a.name)
                                .arg(b.name)
                                .arg(qRound(d * 60.0 * 1852.0 * 0.75)));
        }
    }
    if (!prox.isEmpty())
        parts.append(QStringLiteral("⚠️ Proximité / risque croisement : %1").arg(prox.join(QStringLiteral(" · "))));

    bool fenceOk = true;
    for (const auto &t : m_trackSnapshots) {
        if (t.lat < Geo::kMinLat || t.lat > Geo::kMaxLat || t.lon < Geo::kMinLon || t.lon > Geo::kMaxLon) {
            fenceOk = false;
            break;
        }
    }
    if (!fenceOk)
        parts.append(QStringLiteral("🚨 Géorepère : un navire hors zone opérationnelle WGS84 déclarée."));

    if (atSea > 0 && prox.isEmpty() && fenceOk)
        parts.append(QStringLiteral("✅ Aucune alerte de proximité critique sur la fenêtre %1×%2 nm (approx.).")
                         .arg(int((Geo::kMaxLat - Geo::kMinLat) * 60.0))
                         .arg(int((Geo::kMaxLon - Geo::kMinLon) * 60.0 * qCos(qDegreesToRadians(36.83)))));

    return parts.join(QStringLiteral("\n"));
}

QColor BateauModule::colorForBoatType(const QString &type) const
{
    const QString t = type.toLower();
    if (t.contains(QStringLiteral("commerce")))
        return QColor(231, 76, 60);
    if (t.contains(QStringLiteral("moins")) || t.contains(QStringLiteral("<")))
        return QColor(46, 204, 113);
    if (t.contains(QStringLiteral("plus")) || t.contains(QStringLiteral(">")))
        return QColor(52, 152, 219);
    if (t.contains(QStringLiteral("voilier")))
        return QColor(155, 89, 182);
    return QColor(149, 165, 166);
}
