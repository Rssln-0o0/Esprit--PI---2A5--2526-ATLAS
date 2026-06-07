#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QListWidget>
#include <QListWidgetItem>
#include <QTableWidgetItem>
#include <QMap>
#include <QHash>
#include <QMessageBox>
#include <QTimer>
#include <QList>
#include <QLabel>
#include <QStandardItemModel>
#include <QSortFilterProxyModel>
#include <QtCharts/QChartView>
#include <QtCharts/QPieSeries>
#include <QSqlDatabase>
#include <QDate>
#include <QDateTime>
#include <QSet>
#include <QVector>
#include <QMenu>

class QGraphicsDropShadowEffect;
class QVariantAnimation;
class QLabel;
class QTableWidget;
class QTextToSpeech;
class QScrollArea;
class QGridLayout;
class QFrame;

#include <QMediaPlayer>
#include <QAudioOutput>
#include <QDialog>
#include <QProcess>
#include <QSerialPort>
#include "arduino.h"
#include "arduino_montage_2.h"
#include "arduinouno.h"
#include "employee.h"
#include "pecheur.h"
#include "rfid_pecheur.h"
#include "stock.h"
#include "bateaudb.h"
#include "equipement.h"
#include "quai.h"
#include "equip_http_server.h"

class BateauModule;
class QuaiArduinoWindow;
class EquipHttpServer;

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

/** KPI employés (stats + badge alertes Liste). */
struct EmployeeKpiSnapshot {
    int total = 0;
    int active = 0;
    int onLeave = 0;
    int inactive = 0;
    int certExp30 = 0;
    int certExpired = 0;
    int missingService = 0;
    int availableToday = 0;
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void refreshPhotoPrivacyControls();

private:
    // === MODULE EMPLOYE ===
    void assignRoleToEmployee(QString empID, QString newRole);
    bool ensureEmployeeTable();
    /** Si refreshAlertsAfter est faux : rechargement liste uniquement (ex. recherche), sans recalcul du badge. */
    void loadEmployees(const QString &searchTerm = QString(), int sortIndex = 0, bool refreshAlertsAfter = true);
    void clearEmployeeForm();
    bool validateEmployeeForm(QString *errorMessage = nullptr, bool markFields = false);
    QString selectedEmployeeStatus() const;
    void setEmployeeStatus(const QString &status);
    QString selectedEmployeeShift() const;
    void setEmployeeShift(const QString &shift);
    QString currentEmployeeCin() const;
    void setCurrentEmployeeCin(const QString &cin);
    void setupEmployeeStatsCharts();
    void updateEmployeeStats();
    void openSettingsDialog();
    void on_backgroundMusic_toggled(bool checked);
    void applyTheme(bool darkMode);
    void applyEmployesModuleTheme(bool darkMode);
    // === MODULE PECHEUR ===
    void setupPecheurCharts();
    void updatePecheurRoleChart();
    void updatePecheurStatusChart();
    void setupPecheurDemoRows();
    void afficherPecheurs(const QString &searchText = QString(), int sortIndex = -1);
    void filterPecheursTable(const QString &searchText);

    // === MODULE EQUIPEMENT ===
    void afficherEquipements();
    void clearEquipementForm();
    void updateMaintenanceUI();
    void updateStatistics();

    // === MODULE QUAI ===
    void loadQuaisTable();
    void clearQuaiForm();
    void fillQuaiFormFromRow(int row);
    void refreshQuaiStats();
    void setupQuaiCharts();
    void refreshQuaiCharts();
    void refreshQuaiSupervisionData();
    /** Met à jour alertes + résumé ; optionnellement le tableau supervision (évite le reset à chaque poll). */
    void refreshQuaiAlertesImportantFromDb(bool refreshSupervisionTable = true);
    void refreshQuaiSupervisionTable();
    void refreshQuaiResumeLine();
    void setupQuaiMapOverviewUi();
    void refreshQuaiMapOverview();
    void openQuaiGeographicMap();
    /** Envoie DB_ALLOW_OPEN / DB_BLOCK_OPEN au sketch Quai_sys selon QUAI actifs en base. */
    void syncQuaiGateArduinoFromDatabase();
    void setupQuaiGateSerialNotifier();
    void onQuaiGateSerialReadyRead();
    /** Capteur entrée Quai_sys : quai actif → Inactif puis ouverture. */
    void showQuaiSensorTriggerPickDialog();
    /** Capteur sortie Quai_sys : quai occupé (inactif) → Actif puis ouverture. */
    void showQuaiLeaveSensorPickDialog();
    void startQuaiAlertsPolling();
    void stopQuaiAlertsPolling();
    QString quaiWhereClause() const;
    QString quaiOrderClause() const;
    void processVoiceCommand(const QString &line);
    void on_voice_toggled(bool checked);
    void openEsp32CamDialog(QLabel *targetLabel, const QString &dialogTitle, const QString &messageContext);
    void on_voiceProcess_finished(int exitCode, QProcess::ExitStatus status);
    void applyQuaiInputPalette(bool darkMode);
    void applyStockHeaders();
    QLabel* ensureErrorLabel(QWidget *field);
    void setFieldError(QWidget *field, const QString &message);
    void clearFieldError(QWidget *field);
    void clearStockValidationErrors();
    bool validateStockInputs();
    void setupResponsiveStockLayouts();
    void setupResponsiveModuleLayouts();
    void updateStockGlobalChart();
    void updateStockDistributionCharts();
    void updateStockSpotlightCard();
    void updateStockTrendChart();
    // === MODULE QUAI / TTS ===
    void setupTtsUi();
    void appendTtsLog(const QString &type, const QString &message);
    bool isTtsEnabled() const;
    QString currentTtsLanguageCode() const;
    bool speakTtsMessage(const QString &message);
    void maybeSpeakDailySummary();
    void loadTtsSettings();
    void saveTtsSettings() const;
    void announceSelectedQuaiFromRow(QTableWidget *table, int row);
    void configureTtsEngineLanguage();
    void setupEmployeeListeAlertBadge();
    /** total = nombre d’alertes agrégées ; dbOk = base disponible. */
    void applyEmplListeBadgeLook(int total, bool dbOk);
    void updateEmplBadgePulse(bool active);
    /** Recalcule le badge alertes (appeler après modification employés / affectations moniteur, pas au simple changement d’onglet). */
    void refreshEmployeeListeAlerts(const EmployeeKpiSnapshot *snapIn = nullptr);

    // STOCK - MODULE
    void setupStockChatbotUi();
    void positionStockChatbotOverlay();
    void submitStockChatbotQuery();
    void appendStockChatbotConversation(const QString &line, bool fromAtlas, bool isStatus = false);
    void setStockChatbotBusyState(bool busy);
    void setupStockHistory();
    void appendStockHistoryEntry(const QString &action,
                                 int stockId,
                                 const QString &espece,
                                 int quantite,
                                 const QString &etat);
    void loadStockHistoryFromFile();
    void clearStockHistoryTable();
    QString stockHistoryFilePath() const;

private Q_SLOTS:
    // === SLOTS GÉNÉRAUX / NAVIGATION (menu, stackedWidget) ===
    void on_btnEmployes_clicked();
    void on_btnPecheurs_clicked();
    void on_btnBateaux_clicked();
    void on_btnMaintenance_clicked();
    void on_btnQuais_clicked();
    void on_btnStock_clicked();
    void on_btnStock_2_clicked();
    // === MODULE EMPLOYE ===
    void on_btnSubEmpl1_clicked();
    void on_btnSubEmpl2_clicked();
    void on_btnSubEmpl3_clicked();
    void on_btnSubEmpl4_2_clicked();
    void on_btnSubEmpl5_clicked();
    void on_listRoles_itemClicked(QListWidgetItem *item);
    void on_tableWidget_cellClicked(int row, int column);
    void on_tableRoleAutoSuggestions_cellClicked(int row, int column);
    void on_tableRoleManualSelection_cellClicked(int row, int column);
    void on_add_btn_2_clicked();
    void on_update_btn_2_clicked();
    void on_delete_btn_2_clicked();
    void on_search_btn_2_clicked();
    void on_export_excel_btn_clicked();
    void on_clear_btn_2_clicked();
    void on_tableWidget_2_cellClicked(int row, int column);
    void on_sort_combo_currentIndexChanged(int index);
    void on_btn_upload_photo_clicked();
    void on_btn_camera_photo_clicked();
    void on_btn_esp32_photo_clicked();
    void on_btn_delete_photo_clicked();
    void on_btn_toggle_photo_visible_clicked();
    void on_btnGenPwd_clicked();
    void on_btnPwdEye_toggled(bool checked);
    void on_btnGenCode_clicked();
    void on_btnCodeEye_toggled(bool checked);
    void on_btnParametres_clicked();
    void on_employeeTableHeaderSectionClicked(int logicalIndex);
    void on_btnMonitorExportPdf_clicked();
    void on_btnSubEmplSmartPort_clicked();
    void on_tableSmartPortDays_cellClicked(int row, int column);
    void on_btnSmartPortExportPdf_clicked();
    // === MODULE PECHEUR ===
    void on_btnSubPech1_clicked();
    void on_btnSubPech2_clicked();
    void on_btnSubPech3_clicked();
    void on_btn_add_2_clicked();
    void on_btn_delete_2_clicked();
    void on_btn_edit_2_clicked();
    void on_tableWidget_4_cellClicked(int row, int column);
    void on_btn_export_2_clicked();
    void on_btn_recherche_2_clicked();
    void on_le_search_2_textChanged(const QString &text);
    void on_cb_sort_2_currentIndexChanged(int index);
    void on_btn_analyze_mission_2_clicked();
    // === MODULE BATEAU ===
    void on_btnAjouter_8_clicked();
    void on_btnModifier_15_clicked();
    void on_btnSupprimer_15_clicked();
    void on_btnExportPDF_15_clicked();
    void on_btnAIAssistant_8_clicked();
    void on_lineRecherche_15_textChanged(const QString &text);
    void on_comboTri_15_currentIndexChanged(int index);
    // === MODULE QUAI ===
    void on_mainTabWidget_currentChanged(int index);
    void on_btnAdd_clicked();
    void on_btnUpdate_clicked();
    void on_btnDelete_clicked();
    void on_btnClear_clicked();
    void on_btnSearch_clicked();
    void on_tableQuais_cellClicked(int row, int column);
    void on_filterComboType_currentIndexChanged(int index);
    void on_filterComboStatut_currentIndexChanged(int index);
    void on_filterComboSecurite_currentIndexChanged(int index);
    void on_comboSmartSort_currentIndexChanged(int index);
    void on_btnExport_clicked();
    void on_btnAddIncident_clicked();
    void on_btnSupprimerIncidents_clicked();
    void on_btnRetirerAlerteLigne_clicked();
    void on_btnRefreshAlertesQuai_clicked();
    void on_btnAnalyze_clicked();
    void runTtsManualTest();
    // === MODULE EQUIPEMENT ===
    void on_btnMaRepair_clicked();
    void on_btnMaWorkshop_clicked();
    void on_btnRefreshMaintenance_clicked();
    void on_tabWidget_currentChanged(int index);
    void on_comboSortEq_currentIndexChanged(int index);
    void on_btnGenerateQR_clicked();

    void on_pushButton_2_clicked();
    void on_pushButton_4_clicked();
    void on_pushButton_5_clicked();
    void on_pushButton_7_clicked();
    void on_lineEdit_8_textChanged(const QString &text);
    void on_tableWidget_3_cellClicked(int row, int column);
    void on_comboBox_3_currentIndexChanged(int index);
    // === AUTRES (Login, Face ID, etc.) ===
    void on_pushButton_clicked();    // Forgot password button (new)
    void on_pushButton_3_clicked();  // Login button
    void on_pushButton_6_clicked();  // Face ID button
    void on_label_34_linkActivated(const QString &link);  // Forgot password link
    
    // Face ID page navigation
    void on_pushButton_8_clicked();  // Return button
    
    // Password change page navigation
    void on_pushButton_9_clicked();   // Change password button
    void on_pushButton_11_clicked();  // Return button
    void on_btn_face_upload_clicked();
    void on_btn_face_camera_clicked();
    void on_btn_face_esp32_clicked();
    void on_rfid_serial_ready();
    void on_rfid_pecheur_serial_ready();
    void on_btn_face_verify_clicked();

    //STOCK
    void on_chatbot_button_clicked();
    void on_btnCB_Stock_clicked();
    void on_inputCB_Stock_returnPressed();
    void on_btnStockAjouter_clicked();
    void on_btnSupprimer_Stock_clicked();
    void on_btnStockClean_clicked();
    void on_btnRefresh_clicked();
    void on_btnStockModifier_clicked();
    void on_btnStockSave_clicked();
    void on_btnStockAnnuler_clicked();
    void on_save_clean();
    void on_stockSortCombo_currentTextChanged(const QString &arg1);
    void on_stockSearchEdit_textChanged(const QString &arg1);
    void on_btnNetHistoire_clicked();
    void on_btnExpHistPdf_clicked();
    void chatbot_invisible();
    void on_btnExpDataPdf_clicked();
    void refreshTable();
    void weight_window_invisible();
    void on_btnStockExtWeight_clicked();
    void on_btnStockWeight_clicked();
    void on_btnStockWeightAnnuler_clicked();

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;
private:
    void updatePecheurWarningBlink();

    void connectRfidReadyReadSignal();
    void connectRfidPecheurReadyReadSignal();
    void appendAndParseRfidChunk(const QByteArray &chunk);
    void appendAndParseRfidPecheurChunk(const QByteArray &chunk);
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
    void sendLcdForPecheurRfid(bool found, const PecheurRfidRecord &pecheur, const QByteArray &uidAscii);
    void showTemporarySmartPortBanner(bool accessGranted, const EmployeeRow &employee, const QString &uidDisplay);
    void fillPecheurFormFromRfid(const PecheurRfidRecord &pecheur);
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

    Ui::MainWindow *ui;
    QChartView *m_roleChartView = nullptr;
    QChartView *m_statusChartView = nullptr;
    QChartView *m_quaiChartView = nullptr;
    QChartView *m_quaiChartView2 = nullptr;
    bool m_stockChatbotUiReady = false;
    bool m_stockChatbotBusy = false;
    QChartView *m_quaiChartView3 = nullptr;
    QChartView *m_quaiChartView4 = nullptr;
    /** Barres horizontales QChart : effectifs par type (ex-tableau). */
    QChartView *m_quaiOccByTypeBarView = nullptr;
    QFrame *m_quaiOccByTypeBarFrame = nullptr;
    QChartView *m_empDeptChartView = nullptr;
    QChartView *m_empSkillChartView = nullptr;
    QChartView *m_empShiftChartView = nullptr;
    QChartView *m_empCityChartView = nullptr;
    QChartView *m_stockGlobalChartView = nullptr;
    QChartView *m_stockEtatChartView = nullptr;
    QChartView *m_stockEspeceChartView = nullptr;
    QChartView *m_stockSpotlightChartView = nullptr;
    QChartView *m_stockTrendChartView = nullptr;
    QChartView *m_equipTypeChartView = nullptr;
    QChartView *m_equipStatusChartView = nullptr;
    int m_stockGlobalAnimToken = 0;
    double m_stockGlobalDisplayedProgress = 0.0;
    QSqlDatabase m_db;
    QString m_resetEmail;
    bool m_resetVerified = false;
    QDate m_selectedMonitorDate;
    QString m_selectedMonitorDepartment;
    QMap<QPair<QDate, QString>, QString> m_assignmentsSession;
    /** Cache moniteur Disponibilité : employés complets (1 requête) + affectations sur le mois affiché. */
    struct MonitorEmpRow {
        QString cin;
        QString nom;
        QString prenom;
        QString deptEff;
        QString certs;
        QString statut;
    };
    QVector<MonitorEmpRow> m_monitorEmployees;
    QStringList m_monitorDeptList;
    QSet<QString> m_monitorDbAssignmentKeys; // julian|dept
    /** Affectations dotation lues en base (mois affiché). */
    struct MonitorDbAssignment {
        QDate date;
        QString dept;
        QString cin;
        QString nom;
        QString prenom;
    };
    QVector<MonitorDbAssignment> m_monitorDbAssignments;
    /** Début de période affichée (1er jour du mois) ; m_monitorDays = jours dans ce mois. */
    QDate m_monitorWeekStart;
    int m_monitorDays = 31;
    bool m_monitorCacheValid = false;
    void rebuildMonitorCacheFromDb(bool preferAssignmentsOnly = false);
    /** Une passe sur les employés : disponibles par clé e.deptEff (lookup avec dept.trimmed()). */
    QHash<QString, int> monitorAvailCountByDeptKey(const QDate &d) const;
    bool monitorHasAssignment(const QDate &d, const QString &dept) const;
    int monitorAvailableCount(const QDate &d, const QString &dept) const;
    int monitorMissingCountForDate(const QDate &d) const;
    QStringList monitorMissingDeptsForDate(const QDate &d) const;
    QString monitorAssignmentsSummaryForDate(const QDate &d) const;
    QString monitorAssignmentsForDept(const QDate &d, const QString &dept) const;
    bool monitorDateHasAssignments(const QDate &d) const;
    void layoutMonitorLeftColumn();
    QWidget *m_quaiChartsGridWidget = nullptr;
    QWidget *m_quaiMapTab = nullptr;
    QWidget *m_quaiArduinoTab = nullptr;
    QScrollArea *m_quaiMapScroll = nullptr;
    QWidget *m_quaiMapGridHost = nullptr;
    QGridLayout *m_quaiMapGrid = nullptr;
    QLabel *m_quaiMapSummary = nullptr;
    // Classes métier (une par module) — CRUD et logique métier
    Pecheur Ptmp; // MODULE PECHEUR
    Stock Stmp;   // MODULE STOCK
    BateauDb Btmp;
    BateauModule *m_bateauModule = nullptr;
    Equipement Etmp;   // MODULE EQUIPEMENT
    Quai Qtmp;         // MODULE QUAI
    QMenu *m_menuEmplListeAlerts = nullptr;
    QGraphicsDropShadowEffect *m_emplBadgeShadow = nullptr;
    QVariantAnimation *m_emplBadgePulseAnim = nullptr;
    /** Cache court pour éviter un SELECT complet employé à chaque rafraîchissement du badge. */
    EmployeeKpiSnapshot m_empKpiCache;
    QDateTime m_empKpiCacheTime;
    QTimer *m_quaiAlertsPollTimer = nullptr;
    QTimer *m_pecheurBlinkTimer = nullptr;
    QTimer *m_ttsTimer = nullptr;
    QDate m_ttsLastSummaryDate;
    QString m_lastSpokenTtsMessage;
    qint64 m_lastSpokenTtsAtMs = 0;
    QTextToSpeech *m_ttsEngine = nullptr;
    bool m_ttsHasLanguageVoice = false;
    QList<int> m_pecheurWarningRows;
    bool m_pecheurBlinkOn = false;
    bool m_showPasswordsInList = false;
    bool m_showCodesInList = false;
    QMediaPlayer *m_backgroundMusic = nullptr;
    QAudioOutput *m_backgroundAudioOutput = nullptr;
    QPushButton *m_btnVoice = nullptr;
    QProcess *m_voiceProcess = nullptr;
    QTimer *m_voiceTimer = nullptr;
    QString m_voiceLastCommand;
    QDialog *m_settingsDialog = nullptr;
    bool m_darkMode = true;
    QString m_mainWindowBaseStyle;
    QString m_stackedWidgetBaseStyle;

    Arduino m_esp32Cam;
    ArduinoMontage2 m_smartPortArduino;
    ArduinoUno m_rfidArduino{QStringLiteral("arduinorfid_employe"), QStringLiteral("RFID Employe")};
    ArduinoUno m_rfidPecheurArduino{QStringLiteral("arduinorfid_pecheur"), QStringLiteral("RFID Pecheur")};
    /** Sketch arduino/Quai_system — port COM distinct des autres maquettes. */
    ArduinoUno m_quaiGateArduino{QStringLiteral("arduino_quai_gate"), QStringLiteral("Quai capteurs (Quai_sys)")};
    QByteArray m_quaiGateSerialRxBuf;
    QuaiArduinoWindow *m_quaiArduinoWindow = nullptr;
    QByteArray m_rfidSerialBuffer;
    QByteArray m_rfidPecheurSerialBuffer;
    QTimer *m_rfidPollTimer = nullptr;
    QTimer *m_rfidPecheurPollTimer = nullptr;
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
    QTimer *m_quaiGateDbPollTimer = nullptr;
    /** Entrée ou sortie capteur : une seule boîte de dialogue à la fois. */
    bool m_quaiSensorPickDialogOpen = false;
    /** Pour journal : éviter spam du timer sync (log seulement si la politique change). */
    int m_quaiGateLastAllowLogState = -1;
    QTimer *m_monitorPeriodDebounceTimer = nullptr;
    bool m_populatingMonitorCombos = false;
    QDate m_monitorCacheLoadedForMonthStart;
    QLabel *m_equipPhotoPreview = nullptr;
    QString m_equipPendingPhotoPath;
    EquipHttpServer *m_equipServer = nullptr;
};
#endif
