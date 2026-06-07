# -*- coding: utf-8 -*-
from pathlib import Path

ROOT = Path(r"c:\Users\saida\Desktop\integration\arduino\2a15-smart-fishing-port-management-Integration_Crud_FINAL0001")
BAT = Path(r"c:\Users\saida\Desktop\integration\arduino\2a15-smart-fishing-port-management-INTEGR-CRUD-FIN-BAT\mainwindow.cpp")
FINAL_CPP = ROOT / "mainwindow.cpp"
FINAL_H = ROOT / "mainwindow.h"
bat = BAT.read_text(encoding="utf-8").splitlines()
cpp = FINAL_CPP.read_text(encoding="utf-8")


def lines(a: int, b: int) -> str:
    return "\n".join(bat[a - 1 : b]) + "\n"


parsers = lines(46, 83)
needle = "static QString findNearbyScript(const QString &fileName)"
if needle in cpp and "parseRfidSerialLineToUid" not in cpp:
    cpp = cpp.replace(needle, parsers + needle, 1)

if "#include <QSignalBlocker>" not in cpp:
    cpp = cpp.replace(
        '#include "mainwindow.h"',
        '#include "mainwindow.h"\n#include <QSignalBlocker>',
        1,
    )

setup_block = (
    "    ui->setupUi(this);\n"
    "    m_mainWindowBaseStyle = this->styleSheet();"
)
setup_inj = (
    "    ui->setupUi(this);\n"
    "    qRegisterMetaType<SmartPortRealtimeState>(\"SmartPortRealtimeState\");\n"
    "    if (ui->lblSmartPortBanner)\n"
    "        m_smartPortBannerDefaultText = ui->lblSmartPortBanner->text();\n"
    "    m_mainWindowBaseStyle = this->styleSheet();"
)
if "qRegisterMetaType<SmartPortRealtimeState>" not in cpp:
    if setup_block not in cpp:
        raise SystemExit("setup block not found")
    cpp = cpp.replace(setup_block, setup_inj, 1)

theme_anchor = "    applyTheme(m_darkMode);\n\n    // Quai CRUD buttons"
if "m_rfidArduino.connect_arduino" not in cpp:
    if theme_anchor not in cpp:
        raise SystemExit("applyTheme anchor not found")
    cpp = cpp.replace(
        theme_anchor,
        "    applyTheme(m_darkMode);\n\n" + lines(492, 549) + "\n    // Quai CRUD buttons",
        1,
    )

mon_anchor = (
    "    setupEmployeeListeAlertBadge();\n\n"
    "    // DB init for Employees CRUD"
)
mon_inj = (
    "    setupEmployeeListeAlertBadge();\n"
    "    setupMonitorPeriodSelectors();\n"
    "    setupSmartPortSelectors();\n"
    "    setupSmartPortActuatorSliders();\n"
    "    {\n"
    "        const QString rfidBusy = (m_rfidArduino.getserial() && m_rfidArduino.getserial()->isOpen())\n"
    "            ? m_rfidArduino.getarduino_port_name()\n"
    "            : QString();\n"
    "        m_smartPortArduino.set_exclude_serial_port(rfidBusy);\n"
    "        const int smartRet = m_smartPortArduino.connect_arduino(rfidBusy);\n"
    "        if (smartRet == 0) {\n"
    "            qDebug() << \"SmartPort Arduino sur\" << m_smartPortArduino.portName();\n"
    "        } else {\n"
    "            qDebug() << \"SmartPort Arduino non detecte (USB) ou port indisponible.\";\n"
    "        }\n"
    "    }\n\n"
    "    // DB init for Employees CRUD"
)
if "setupSmartPortSelectors();" not in cpp:
    if mon_anchor not in cpp:
        raise SystemExit("monitor anchor not found")
    cpp = cpp.replace(mon_anchor, mon_inj, 1)

if "refreshPhotoPrivacyControls();" not in cpp or "chatbot_invisible();\n    refreshPhotoPrivacyControls();" not in cpp:
    cpp = cpp.replace(
        "    chatbot_invisible();\n}",
        "    chatbot_invisible();\n    refreshPhotoPrivacyControls();\n}",
        1,
    )

cpp = cpp.replace(
    """MainWindow::~MainWindow()
{
    saveTtsSettings();
    delete ui;
}""",
    """MainWindow::~MainWindow()
{
    stopRfidSerialPolling();
    m_rfidArduino.close_arduino();
    m_esp32Cam.close_arduino();
    m_smartPortArduino.close_arduino();
    saveTtsSettings();
    delete ui;
}""",
    1,
)

cpp = cpp.replace(
    """static void syncEmployesSubNavChecked(Ui::MainWindow *ui, int index) {
    if (!ui) return;
    if (ui->btnSubEmpl3) ui->btnSubEmpl3->setChecked(index == 0);
    if (ui->btnSubEmpl4_2) ui->btnSubEmpl4_2->setChecked(index == 1);
    if (ui->btnSubEmpl5) ui->btnSubEmpl5->setChecked(index == 2);
}""",
    """static void syncEmployesSubNavChecked(Ui::MainWindow *ui, int index) {
    if (!ui) return;
    if (ui->btnSubEmpl3) ui->btnSubEmpl3->setChecked(index == 0);
    if (ui->btnSubEmpl4_2) ui->btnSubEmpl4_2->setChecked(index == 1);
    if (ui->btnSubEmpl5) ui->btnSubEmpl5->setChecked(index == 2);
    if (ui->btnSubEmplSmartPort) ui->btnSubEmplSmartPort->setChecked(index == 3);
}""",
    1,
)

if "void MainWindow::reloadMonitorDbAssignmentsOnly()" not in cpp:
    start = cpp.find("void MainWindow::rebuildMonitorCacheFromDb()")
    if start < 0:
        raise SystemExit("rebuildMonitorCacheFromDb not found")
    end = cpp.find("QHash<QString, int> MainWindow::monitorAvailCountByDeptKey", start)
    if end < 0:
        raise SystemExit("monitorAvailCountByDeptKey not found")
    cpp = cpp[:start] + lines(2361, 2479) + cpp[end:]

append_block = ""
if "void MainWindow::connectRfidReadyReadSignal()" not in cpp:
    append_block += lines(776, 1063)
if "void MainWindow::setupSmartPortSelectors()" not in cpp:
    append_block += lines(2806, 3238)
if "void MainWindow::on_btnSubEmplSmartPort_clicked()" not in cpp:
    append_block += lines(3721, 3748)

if "m_esp32Cam.capture_esp32_cam_jpeg" not in cpp:
    o1 = cpp.find("bool MainWindow::openEsp32CamPort(")
    o2 = cpp.find("void MainWindow::on_btn_delete_photo_clicked()", o1)
    if o1 < 0 or o2 < 0:
        raise SystemExit("openEsp32 slice markers not found")
    esp32 = lines(8257, 8375)
    esp32 = esp32.replace(
        "        const bool privacyMask = (targetLabel == ui->label_photo);\n",
        "",
    )
    esp32 = esp32.replace(
        "setPhotoFromPath(targetLabel, path, privacyMask);",
        "setPhotoFromPath(targetLabel, path);",
    )
    cpp = cpp[:o1] + esp32 + cpp[o2:]

if "void MainWindow::refreshPhotoPrivacyControls()" not in cpp:
    ins = cpp.find("void MainWindow::on_tableWidget_2_cellClicked(int row, int column)")
    if ins < 0:
        raise SystemExit("on_tableWidget_2_cellClicked not found")
    cpp = cpp[:ins] + lines(8388, 8413) + cpp[ins:]

if append_block:
    cpp = cpp.rstrip() + "\n\n" + append_block

FINAL_CPP.write_text(cpp, encoding="utf-8")

h = FINAL_H.read_text(encoding="utf-8")
if "#include \"arduino.h\"" not in h:
    h = h.replace(
        '#include <QSerialPort>\n#include "employee.h"',
        '#include <QSerialPort>\n#include "arduino.h"\n#include "arduino_montage_2.h"\n#include "arduinouno.h"\n#include "employee.h"',
        1,
    )

if "void refreshPhotoPrivacyControls();" not in h:
    h = h.replace(
        "    ~MainWindow();\n\nprivate:",
        "    ~MainWindow();\n    void refreshPhotoPrivacyControls();\n\nprivate:",
        1,
    )

for sig in (
    "    bool openEsp32CamPort(QSerialPort *port);\n",
    "    bool captureImageFromEsp32Cam(QImage *out, QSerialPort *existingPort = nullptr);\n",
    "    bool captureImageFromEsp32CamViaHttp(QImage *out);\n",
):
    h = h.replace(sig, "")

if "on_btnSubEmplSmartPort_clicked" not in h:
    h = h.replace(
        "    void on_employeeTableHeaderSectionClicked(int logicalIndex);\n",
        "    void on_employeeTableHeaderSectionClicked(int logicalIndex);\n"
        "    void on_btnSubEmplSmartPort_clicked();\n"
        "    void on_tableSmartPortDays_cellClicked(int row, int column);\n"
        "    void on_btnSmartPortExportPdf_clicked();\n",
        1,
    )
if "on_rfid_serial_ready();" not in h:
    h = h.replace(
        "    void on_btn_face_esp32_clicked();\n",
        "    void on_btn_face_esp32_clicked();\n    void on_rfid_serial_ready();\n",
        1,
    )

priv_inj = """
    void connectRfidReadyReadSignal();
    void appendAndParseRfidChunk(const QByteArray &chunk);
    void startRfidSerialPolling();
    void stopRfidSerialPolling();
    void pushRfidArduinoDoorPolicy(bool captureUidOnly);
    void pushRfidArduinoDoorHardwareMode();
    void applyPorteUiState(bool open);
    void pushPorteCommandToArduino(bool open);
    void pushLcdLineToArduino(int row, const QString &text);
    void pushLcdHardwareClearToArduino();
    void sendLcdForRfidGate(bool accessGranted, const EmployeeRow &employee, const QByteArray &uidAscii, bool fromKeypad = false);
    void sendLcdForPortSortie(const EmployeeRow &employee);
    void showTemporarySmartPortBanner(bool accessGranted, const EmployeeRow &employee, const QString &uidDisplay);
    void refreshSmartPortPresenceUi();
    void reloadMonitorDbAssignmentsOnly();
    void setupMonitorPeriodSelectors();
    void scheduleMonitorPeriodApply();
    void syncMonitorCombosToMonth(const QDate &anyDayInMonth);
    void applyMonitorPeriodFromUi();
    void refreshMonitorDisponibiliteTable();
    void setupSmartPortSelectors();
    void setupSmartPortActuatorSliders();
    void applySmartPortRealtimeState(const SmartPortRealtimeState &state);
    void scheduleSmartPortPeriodApply();
    void syncSmartPortCombosToMonth(const QDate &anyDayInMonth);
    void applySmartPortPeriodFromUi();
    void refreshSmartPortDayList();
    void refreshSmartPortEmployeeTable(const QDate &day);
"""
if "pushRfidArduinoDoorPolicy" not in h:
    h = h.replace(
        "    void updatePecheurWarningBlink();\n\n    Ui::MainWindow *ui;",
        "    void updatePecheurWarningBlink();\n" + priv_inj + "\n    Ui::MainWindow *ui;",
        1,
    )

h = h.replace(
    "    void rebuildMonitorCacheFromDb();",
    "    void rebuildMonitorCacheFromDb(bool preferAssignmentsOnly = false);",
    1,
)

mem = """
    Arduino m_esp32Cam;
    ArduinoMontage2 m_smartPortArduino;
    ArduinoUno m_rfidArduino;
    QByteArray m_rfidSerialBuffer;
    QTimer *m_rfidPollTimer = nullptr;
    QString m_smartPortBannerDefaultText;
    QTimer *m_smartPortBannerResetTimer = nullptr;
    bool m_smartPortAwaitRfidForSortie = false;
    QByteArray m_rfidLastSerialUid;
    qint64 m_rfidLastSerialUidMs = 0;
    bool m_syncingSmartPortFromSerial = false;
    QDate m_smartPortMonthStart;
    int m_smartPortDays = 31;
    QDate m_smartPortSelectedDate;
    bool m_populatingSmartPortCombos = false;
    QTimer *m_smartPortPeriodDebounceTimer = nullptr;
    QTimer *m_smartPortPollTimer = nullptr;
    QTimer *m_monitorPeriodDebounceTimer = nullptr;
    bool m_populatingMonitorCombos = false;
    QDate m_monitorCacheLoadedForMonthStart;
"""
if "m_esp32Cam" not in h:
    h = h.replace(
        "    QString m_stackedWidgetBaseStyle;\n",
        "    QString m_stackedWidgetBaseStyle;\n" + mem,
        1,
    )

FINAL_H.write_text(h, encoding="utf-8")
print("OK")
