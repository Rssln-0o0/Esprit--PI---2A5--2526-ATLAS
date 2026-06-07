/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created by: Qt User Interface Compiler version 6.7.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_TEST_H
#define UI_MAINWINDOW_TEST_H

#include <QtCore/QVariant>
#include <QtGui/QIcon>
#include <QtWidgets/QApplication>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QDateEdit>
#include <QtWidgets/QDoubleSpinBox>
#include <QtWidgets/QFormLayout>
#include <QtWidgets/QFrame>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QProgressBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QRadioButton>
#include <QtWidgets/QScrollArea>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QStackedWidget>
#include <QtWidgets/QTabWidget>
#include <QtWidgets/QTableView>
#include <QtWidgets/QTableWidget>
#include <QtWidgets/QTextEdit>
#include <QtWidgets/QTimeEdit>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QWidget *centralwidget;
    QVBoxLayout *mainVerticalLayout;
    QStackedWidget *mainStackedWidget;
    QWidget *dashboardPage;
    QHBoxLayout *horizontalLayout;
    QFrame *sidebar;
    QVBoxLayout *verticalLayout;
    QLabel *lblTitle;
    QSpacerItem *verticalSpacer_2;
    QPushButton *btnEmployes;
    QPushButton *btnPecheurs;
    QPushButton *btnBateaux;
    QPushButton *btnMaintenance;
    QPushButton *btnQuais;
    QPushButton *btnStock_2;
    QSpacerItem *verticalSpacer_3;
    QSpacerItem *verticalSpacer_4;
    QPushButton *btnStock;
    QSpacerItem *verticalSpacer;
    QStackedWidget *stackedWidget;
    QWidget *pageEmployes;
    QVBoxLayout *verticalLayout_2;
    QFrame *frame_3;
    QHBoxLayout *horizontalLayout_4;
    QLabel *title_label_2;
    QFrame *subNavBar;
    QHBoxLayout *subNavBarLayout;
    QPushButton *btnSubEmpl3;
    QPushButton *btnSubEmpl4_2;
    QPushButton *btnSubEmpl5;
    QSpacerItem *horizontalSpacer;
    QStackedWidget *stackedWidgetEmployes;
    QWidget *subPageEmpl3;
    QFrame *info_frame_2;
    QVBoxLayout *verticalLayout_tab;
    QTabWidget *tabWidget_employee;
    QWidget *tab_id;
    QGridLayout *gridLayout_id;
    QLabel *label_lname;
    QLabel *label_id;
    QLabel *label_photo;
    QLabel *label_cin;
    QLabel *label_fname;
    QPushButton *btn_upload_photo;
    QLineEdit *lineEdit_cin;
    QLineEdit *lineEdit_fname;
    QLineEdit *lineEdit_lname;
    QLineEdit *lineEdit_id;
    QWidget *tab_pro;
    QGridLayout *gridLayout_pro;
    QLabel *label_job;
    QLineEdit *lineEdit_job;
    QLabel *label_dept;
    QComboBox *combo_dept;
    QLabel *label_hdate;
    QDateEdit *dateEdit_hire;
    QLabel *label_status;
    QComboBox *combo_status;
    QWidget *tab_contact;
    QGridLayout *gridLayout_contact;
    QLabel *label_phone;
    QLineEdit *lineEdit_phone;
    QLabel *label_email;
    QLineEdit *lineEdit_email;
    QLabel *label_pwd;
    QLineEdit *lineEdit_pwd;
    QLabel *label_city;
    QLineEdit *lineEdit_city;
    QLabel *label_addr;
    QLineEdit *lineEdit_addr;
    QWidget *tab_skills;
    QGridLayout *gridLayout_skills;
    QLabel *label_certs;
    QLineEdit *lineEdit_certs;
    QLabel *label_slevel;
    QComboBox *combo_slevel;
    QLabel *label_lic;
    QLineEdit *lineEdit_lic;
    QLabel *label_exp;
    QDateEdit *dateEdit_exp;
    QLabel *label_shift;
    QComboBox *combo_shift;
    QFrame *function_frame;
    QHBoxLayout *horizontalLayout_2;
    QPushButton *add_btn_2;
    QPushButton *clear_btn_2;
    QPushButton *update_btn_2;
    QPushButton *delete_btn_2;
    QLineEdit *search_input_2;
    QPushButton *clear_btn_3;
    QLineEdit *search_input;
    QPushButton *search_btn_2;
    QComboBox *sort_combo;
    QPushButton *export_excel_btn;
    QFrame *result_frame;
    QGridLayout *gridLayout_5;
    QTableWidget *tableWidget_2;
    QWidget *subPageEmpl4;
    QLabel *lblMetierTitle;
    QFrame *frameCard1;
    QLabel *lblTotalEmployees;
    QLabel *lblTotalEmployeesValue;
    QFrame *frameCard2;
    QLabel *lblActiveEmployees;
    QLabel *lblActiveEmployeesValue;
    QFrame *frameCard3;
    QLabel *lblOnLeave;
    QLabel *lblOnLeaveValue;
    QFrame *frameCard4;
    QLabel *lblCertExpiring;
    QLabel *lblCertExpiringValue;
    QFrame *frameDepartments;
    QLabel *lblDeptTitle;
    QLabel *lblDept1;
    QProgressBar *progressDept1;
    QLabel *lblDept2;
    QProgressBar *progressDept2;
    QLabel *lblDept3;
    QProgressBar *progressDept3;
    QLabel *lblDept4;
    QProgressBar *progressDept4;
    QFrame *frameSkills;
    QLabel *lblSkillsTitle;
    QLabel *lblSkill1;
    QProgressBar *progressSkill1;
    QLabel *lblSkill2;
    QProgressBar *progressSkill2;
    QLabel *lblSkill3;
    QProgressBar *progressSkill3;
    QFrame *frameShifts;
    QLabel *lblShiftsTitle;
    QLabel *lblShift1;
    QProgressBar *progressShift1;
    QLabel *lblShift2;
    QProgressBar *progressShift2;
    QFrame *frameCities;
    QLabel *lblCitiesTitle;
    QLabel *lblCity1;
    QProgressBar *progressCity1;
    QLabel *lblCity2;
    QProgressBar *progressCity2;
    QLabel *lblCity3;
    QProgressBar *progressCity3;
    QFrame *frameAlerts;
    QLabel *lblAlertsTitle;
    QLabel *lblAlert1;
    QLabel *lblAlert2;
    QLabel *lblAlert3;
    QWidget *subPageEmpl5;
    QLabel *lblAdvancedMetierTitle;
    QListWidget *listRoles;
    QFrame *frameRoleDetail;
    QFrame *frameRoleStatus;
    QLabel *lblRoleStatusIcon;
    QLabel *lblRoleStatusText;
    QLabel *lblAutoTitle;
    QTableWidget *tableRoleAutoSuggestions;
    QLabel *lblManualTitle;
    QTableWidget *tableRoleManualSelection;
    QTableWidget *tableWidget;
    QWidget *pagePecheurs;
    QVBoxLayout *verticalLayout_Pecheurs;
    QFrame *frame_Pecheurs_Header;
    QVBoxLayout *verticalLayout_Header;
    QFrame *frame_Pecheurs_Header_Overlay;
    QVBoxLayout *verticalLayout_Title;
    QLabel *title_label_pecheurs;
    QFrame *subNavBarPecheurs;
    QHBoxLayout *horizontalLayout_SubNavPech;
    QPushButton *btnSubPech1;
    QPushButton *btnSubPech2;
    QPushButton *btnSubPech3;
    QSpacerItem *spacerSubNav;
    QStackedWidget *stackedWidgetPecheurs;
    QWidget *subPagePech1;
    QVBoxLayout *verticalLayout_sub1;
    QGroupBox *groupBox_pecheur_2;
    QGridLayout *gridLayout_form_2;
    QLabel *l2_2;
    QLineEdit *le_nom_2;
    QLabel *l3_2;
    QLineEdit *le_prenom_2;
    QLabel *l5_2;
    QComboBox *cb_role_2;
    QLabel *l7_2;
    QWidget *widget_status_2;
    QHBoxLayout *horizontalLayout_status_2;
    QRadioButton *radio_status_actif_2;
    QRadioButton *radio_status_inactif_2;
    QLabel *l6_2;
    QSpinBox *sb_experience_2;
    QLabel *l_date_limit_2;
    QDateEdit *dateEdit_limit_2;
    QLabel *l_contact_2;
    QWidget *widget_contact_2;
    QHBoxLayout *horizontalLayout_contact_2;
    QLabel *l_prefix_216_2;
    QLineEdit *le_telephone_2;
    QFrame *framePecheurToolbar;
    QVBoxLayout *verticalLayout_pecheur_toolbar;
    QHBoxLayout *h_actions_2;
    QPushButton *btn_add_2;
    QPushButton *btn_edit_2;
    QPushButton *btn_delete_2;
    QSpacerItem *s1_2;
    QPushButton *btn_export_2;
    QHBoxLayout *h_search_sort_2;
    QLineEdit *le_search_2;
    QPushButton *btn_recherche_2;
    QSpacerItem *s2_2;
    QLabel *lsort_2;
    QComboBox *cb_sort_2;
    QTableWidget *tableWidget_4;
    QWidget *subPagePech2;
    QVBoxLayout *verticalLayout_sub2;
    QHBoxLayout *h_charts_2;
    QFrame *frame_chart_role_2;
    QVBoxLayout *v_chart_role_2;
    QLabel *label_role_chart_title_2;
    QWidget *chartViewRole_2;
    QFrame *frame_chart_status_2;
    QVBoxLayout *v_chart_status_2;
    QLabel *label_status_chart_title_2;
    QWidget *chartViewStatus_2;
    QWidget *subPagePech3;
    QVBoxLayout *verticalLayout_sub3;
    QGroupBox *gb_mission_setup_2;
    QGridLayout *grid_mission_setup_2;
    QLabel *lbl_mission_type_2;
    QComboBox *cb_mission_type_2;
    QLabel *lbl_duration_2;
    QSpinBox *sb_duration_2;
    QLabel *lbl_difficulty_2;
    QComboBox *cb_difficulty_2;
    QPushButton *btn_analyze_mission_2;
    QGroupBox *gb_ai_results_2;
    QVBoxLayout *v_ai_results_2;
    QLabel *lbl_ai_status_2;
    QTableWidget *table_mission_results_2;
    QWidget *pageBateaux;
    QWidget *mainContentArea;
    QVBoxLayout *verticalLayout_MainContent_2;
    QFrame *frame_Bateaux_Header;
    QVBoxLayout *verticalLayout_Header_Bateaux;
    QFrame *frame_Bateaux_Header_Overlay;
    QVBoxLayout *verticalLayout_Title_Bateaux;
    QLabel *title_label_bateaux;
    QTabWidget *tabWidget_2;
    QWidget *tabGestion_8;
    QVBoxLayout *verticalLayout_Gestion_2;
    QGroupBox *groupForm_15;
    QGridLayout *gridLayout_Form_2;
    QLabel *label_id_16;
    QLineEdit *lineID_15;
    QLabel *label_nom_15;
    QLineEdit *lineLicence_15;
    QLabel *label_exp_51;
    QLineEdit *linePrenom_36;
    QLabel *label_exp_52;
    QLineEdit *linePrenom_37;
    QLabel *label_exp_53;
    QLineEdit *linePrenom_38;
    QLabel *label_status_field_2;
    QComboBox *comboStatus_2;
    QLabel *label_license_8;
    QLineEdit *lineLicenseNum_8;
    QLabel *label_expiry_date_2;
    QDateEdit *dateExpiry_2;
    QLabel *label_exp_54;
    QComboBox *comboRole_15;
    QFrame *frameToolbar_3;
    QHBoxLayout *horizontalLayout_Toolbar_3;
    QPushButton *btnAjouter_8;
    QPushButton *btnModifier_15;
    QPushButton *btnSupprimer_15;
    QLineEdit *lineRecherche_15;
    QComboBox *comboTri_15;
    QPushButton *btnExportPDF_15;
    QTableView *tableBateaux_2;
    QWidget *tabStats_8;
    QVBoxLayout *verticalLayout_Stats_2;
    QGroupBox *groupForm_16;
    QGridLayout *gridLayout_3;
    QLabel *label_id_17;
    QLineEdit *lineID_16;
    QLabel *label_nom_16;
    QLineEdit *lineLicence_16;
    QLabel *label_exp_55;
    QLineEdit *linePrenom_39;
    QLabel *label_exp_56;
    QLineEdit *linePrenom_40;
    QLabel *label_exp_57;
    QComboBox *comboRole_16;
    QFrame *frameToolbar_4;
    QHBoxLayout *horizontalLayout_Toolbar_4;
    QPushButton *btnAjouter_21;
    QPushButton *btnModifier_16;
    QPushButton *btnSupprimer_16;
    QLineEdit *lineRecherche_16;
    QComboBox *comboTri_16;
    QPushButton *btnExportPDF_16;
    QTableView *tableHistorique_3;
    QWidget *tabStatistiques_8;
    QGridLayout *gridLayout_Stats_8;
    QGroupBox *groupTotal_8;
    QVBoxLayout *v1_2;
    QLabel *lblTotalBateauxCount_8;
    QLabel *lblTotalBateauxText_8;
    QGroupBox *groupActif_8;
    QVBoxLayout *v2_2;
    QLabel *lblLicencesActivesCount_8;
    QLabel *lblLicencesActivesText_8;
    QGroupBox *groupMer_8;
    QVBoxLayout *verticalLayout_Mer_8;
    QLabel *lblBateauxMerCount_8;
    QLabel *lblBateauxMerText_8;
    QGroupBox *groupExpire_8;
    QVBoxLayout *verticalLayout_Expire_8;
    QLabel *lblLicencesExpireesCount_8;
    QLabel *lblLicencesExpireesText_8;
    QGroupBox *groupNotifications_8;
    QVBoxLayout *verticalLayout_Notifications_2;
    QTextEdit *textNotifications_8;
    QPushButton *btnAIAssistant_8;
    QWidget *pageMaintenance;
    QVBoxLayout *verticalLayout_4;
    QFrame *frame_Equipement_Header;
    QVBoxLayout *verticalLayout_Header_Equipement;
    QFrame *frame_Equipement_Header_Overlay;
    QVBoxLayout *verticalLayout_Title_Equipement;
    QLabel *title_label_equipement;
    QTabWidget *tabWidget;
    QWidget *tab_gestion;
    QVBoxLayout *verticalLayout_equip_content;
    QScrollArea *scrollArea_equip_form;
    QWidget *scrollAreaWidgetContents_equip;
    QGroupBox *groupBox;
    QGridLayout *gridLayout_equip_form;
    QLabel *label;
    QLineEdit *lineEdit;
    QLabel *label_2;
    QLineEdit *lineEdit_2;
    QLabel *label_3;
    QLineEdit *lineEdit_3;
    QLabel *label_4;
    QComboBox *comboBox;
    QLabel *label_5;
    QDateEdit *dateEdit;
    QLabel *label_6;
    QComboBox *comboBox_2;
    QLabel *label_7;
    QLineEdit *lineEdit_4;
    QLabel *label_8;
    QLineEdit *lineEdit_11;
    QLabel *label_9;
    QWidget *widget_disp_equip;
    QHBoxLayout *horizontalLayout_disp;
    QRadioButton *radioButton;
    QRadioButton *radioButton_2;
    QSpacerItem *spacerDisp;
    QFrame *frameEquipToolbar;
    QHBoxLayout *h_buttons_equip;
    QPushButton *pushButton_2;
    QPushButton *pushButton_4;
    QPushButton *pushButton_5;
    QSpacerItem *spacerEquipBtn;
    QPushButton *pushButton_7;
    QHBoxLayout *h_search_equip;
    QLineEdit *lineEdit_8;
    QSpacerItem *spacerEquipSearch;
    QLabel *label_10;
    QComboBox *comboBox_3;
    QFrame *chartContainer_2;
    QVBoxLayout *verticalLayout_table_equip;
    QTableWidget *tableWidget_3;
    QWidget *tab_statistique;
    QFrame *frame_stat;
    QVBoxLayout *verticalLayout_3;
    QLabel *label_stat;
    QWidget *tab_smart_location;
    QFrame *frame_smart_location;
    QVBoxLayout *verticalLayout_smart;
    QLabel *label_smart_location;
    QWidget *tab_maintenance;
    QFrame *frame_maintenance;
    QVBoxLayout *verticalLayout_maintenance;
    QLabel *label_maintenance;
    QWidget *pageQuais;
    QVBoxLayout *pageQuaisLayout;
    QFrame *contentFrame;
    QVBoxLayout *contentLayout;
    QFrame *frame_Quais_Header;
    QVBoxLayout *verticalLayout_Header_Quais;
    QFrame *frame_Quais_Header_Overlay;
    QVBoxLayout *verticalLayout_Title_Quais;
    QLabel *title_label_quais;
    QTabWidget *mainTabWidget;
    QWidget *tabCRUD;
    QVBoxLayout *verticalLayout_5;
    QFrame *frame_Quais_SectionTitle;
    QHBoxLayout *sectionTitleLayout;
    QLabel *label_Quais_SectionTitle;
    QHBoxLayout *formAndPhotoLayout;
    QGroupBox *groupBoxQuaiInfo;
    QGridLayout *gridLayoutQuaiInfo;
    QWidget *statutRadioContainer;
    QHBoxLayout *statutRadioLayout;
    QRadioButton *radioStatutActif;
    QRadioButton *radioStatutInactif;
    QRadioButton *radioStatutMaintenance;
    QDoubleSpinBox *doubleSpinBoxOccupation;
    QLabel *labelCapacite;
    QLabel *labelType;
    QDoubleSpinBox *doubleSpinBoxLongueur;
    QWidget *eclairageRadioContainer;
    QHBoxLayout *eclairageRadioLayout;
    QRadioButton *radioEclairageAllume;
    QRadioButton *radioEclairageEteint;
    QRadioButton *radioEclairageDefaillant;
    QLabel *labelLongueur;
    QLabel *labelCurrentUsage;
    QWidget *prioriteRadioContainer;
    QHBoxLayout *prioriteRadioLayout;
    QRadioButton *radioPrioriteNormale;
    QRadioButton *radioPrioriteHaute;
    QRadioButton *radioPrioriteBasse;
    QSpinBox *spinBoxCurrentUsage;
    QSpinBox *spinBoxCapacite;
    QLabel *labelNom;
    QLabel *labelSecurite;
    QLabel *labelOccupation;
    QLabel *labelQuaiID;
    QWidget *typeRadioContainer;
    QHBoxLayout *typeRadioLayout;
    QRadioButton *radioTypePeche;
    QRadioButton *radioTypeCommerce;
    QRadioButton *radioTypeMaintenance;
    QRadioButton *radioTypeMixte;
    QLabel *labelProfondeur;
    QLabel *labelPriorite;
    QLabel *labelLightingStatus;
    QLineEdit *lineEditNom;
    QWidget *securiteRadioContainer;
    QHBoxLayout *securiteRadioLayout;
    QRadioButton *radioSecurite1;
    QRadioButton *radioSecurite2;
    QRadioButton *radioSecurite3;
    QLabel *labelStatut;
    QLineEdit *lineEditQuaiID;
    QDoubleSpinBox *doubleSpinBoxProfondeur;
    QFrame *frameQuaiToolbar;
    QHBoxLayout *crudButtonLayout;
    QPushButton *btnAdd;
    QPushButton *btnUpdate;
    QPushButton *btnDelete;
    QPushButton *btnClear;
    QSpacerItem *spacerBeforeFilter;
    QLabel *labelFilterBy;
    QComboBox *filterComboType;
    QComboBox *filterComboStatut;
    QComboBox *filterComboSecurite;
    QSpacerItem *horizontalSpacer_2;
    QLineEdit *lineEditSearch;
    QPushButton *btnSearch;
    QSpacerItem *spacerBeforeSort;
    QLabel *labelSmartSort;
    QComboBox *comboSmartSort;
    QPushButton *btnExport;
    QTableWidget *tableQuais;
    QWidget *tabSupervision;
    QVBoxLayout *supervisionLayout;
    QFrame *frame_Supervision_SectionTitle;
    QHBoxLayout *supervisionSectionTitleLayout;
    QLabel *label_Supervision_SectionTitle;
    QHBoxLayout *kpiCardsLayout;
    QFrame *kpiCardTotal;
    QVBoxLayout *kpiTotalLayout;
    QLabel *lblKpiTotalTitle;
    QLabel *lblKpiTotalValue;
    QFrame *kpiCardLibres;
    QVBoxLayout *kpiLibresLayout;
    QLabel *lblKpiLibresTitle;
    QLabel *lblKpiLibresValue;
    QFrame *kpiCardOccupes;
    QVBoxLayout *kpiOccupesLayout;
    QLabel *lblKpiOccupesTitle;
    QLabel *lblKpiOccupesValue;
    QFrame *kpiCardMaintenance;
    QVBoxLayout *kpiMaintenanceLayout;
    QLabel *lblKpiMaintenanceTitle;
    QLabel *lblKpiMaintenanceValue;
    QHBoxLayout *supervisionSplitLayout;
    QVBoxLayout *superLeftLayout;
    QGroupBox *groupBoxOccupation;
    QHBoxLayout *occupationLayout;
    QTableWidget *tableOccupationByType;
    QVBoxLayout *pieContainerLayout;
    QLabel *chartTitle;
    QHBoxLayout *chartAndLegendLayout;
    QFrame *pieChartFrame;
    QFrame *legendFrame;
    QVBoxLayout *legendLayout;
    QLabel *legend1;
    QLabel *legend2;
    QLabel *legend3;
    QLabel *legend4;
    QVBoxLayout *superRightLayout;
    QHBoxLayout *incidentButtonsInner;
    QPushButton *btnAddIncident;
    QSpacerItem *horizontalSpacer_3;
    QTableWidget *tableSupervisionCombined;
    QFrame *chartCardAlertes;
    QVBoxLayout *chartAlertesLayout;
    QLabel *lblChartAlertesTitle;
    QLabel *lblAlertesContent;
    QFrame *frame_Supervision_Resume;
    QHBoxLayout *resumeLayout;
    QLabel *lblResumeQuais;
    QSpacerItem *spacerResume;
    QLabel *lblResumeMAJ;
    QWidget *tabAideDecision;
    QVBoxLayout *aideDecisionLayout;
    QGroupBox *groupBoxBoatInfo;
    QFormLayout *boatFormLayout;
    QLabel *labelTypeBateau;
    QComboBox *comboBoxTypeBateau;
    QLabel *labelLongueurBateau;
    QSpinBox *spinBoxLongueurBateau;
    QLabel *labelTonnage;
    QSpinBox *spinBoxTonnage;
    QPushButton *btnAnalyze;
    QTableWidget *tableRecommendations;
    QWidget *tabParametres;
    QVBoxLayout *paramsLayout;
    QTabWidget *paramsTabs;
    QWidget *tabSyntheseVocale;
    QVBoxLayout *ttsInnerLayout;
    QGroupBox *groupBoxTTSSettings;
    QFormLayout *ttsFormLayout;
    QCheckBox *checkBoxEnableTTS;
    QLabel *labelLanguage;
    QComboBox *comboBoxLanguage;
    QCheckBox *checkBoxReadSelected;
    QCheckBox *checkBoxCriticalAlerts;
    QCheckBox *checkBoxDailySummary;
    QLabel *labelSummaryTime;
    QTimeEdit *timeEditSummary;
    QPushButton *btnTestTTS;
    QTableWidget *tableTTSLog;
    QWidget *pageStock;
    QVBoxLayout *verticalLayout_stock;
    QFrame *frame_Stock_Header;
    QVBoxLayout *verticalLayout_Header_Stock;
    QFrame *frame_Stock_Header_Overlay;
    QVBoxLayout *verticalLayout_Title_Stock;
    QLabel *title_label_stock;
    QTabWidget *tabWidgetStockVentes;
    QWidget *tabStock;
    QVBoxLayout *verticalLayout_Stock_Gestion;
    QGroupBox *groupBoxStockForm;
    QGridLayout *gridLayout_Stock_Form;
    QLabel *stockIdLabel;
    QLineEdit *stockIdEdit;
    QLabel *stockEspeceLabel;
    QLineEdit *stockEspeceEdit;
    QLabel *stockQuantiteLabel;
    QLineEdit *stockQuantiteEdit;
    QLabel *stockEtatLabel;
    QComboBox *stockEtatCombo;
    QLabel *stockSeuilMinLabel;
    QLineEdit *stockSeuilMinEdit;
    QLabel *stockSeuilMaxLabel;
    QLineEdit *stockSeuilMaxEdit;
    QLabel *stockDateAjoutLabel;
    QLineEdit *stockDateAjoutEdit;
    QLabel *stockCINLabel;
    QLineEdit *stockCIN;
    QLabel *stockCMDLabel;
    QLineEdit *stockCMD;
    QLabel *stockDateVenteLabel;
    QLineEdit *stockDateVente;
    QFrame *frameStockToolbar;
    QHBoxLayout *horizontalLayout_Stock_Toolbar;
    QPushButton *btnStockAjouter;
    QPushButton *btnStockModifier;
    QPushButton *btnStockSupprimer;
    QPushButton *btnStockSave;
    QPushButton *btnStockAnnuler;
    QSpacerItem *stockToolbarSpacer;
    QLineEdit *stockSearchEdit;
    QLabel *stockSortLabel;
    QComboBox *stockSortCombo;
    QTableWidget *tableStock;
    QWidget *tabVentes;
    QFrame *frameVentesForm;
    QLineEdit *venteAcheteurEdit;
    QLabel *venteAcheteurLabel;
    QLabel *venteCinLabel;
    QLineEdit *venteCinEdit;
    QLabel *venteQuantiteLabel;
    QLineEdit *venteQuantiteEdit;
    QLabel *venteIdLabel;
    QLineEdit *venteIdEdit;
    QLabel *venteStockIdLabel;
    QLineEdit *venteStockIdEdit;
    QLabel *venteSeuilMaxLabel;
    QLineEdit *venteSeuilMaxEdit;
    QLabel *venteDateLabel;
    QLineEdit *venteDateEdit;
    QFrame *frameVentesToolbar;
    QPushButton *btnVenteAjouter;
    QPushButton *btnVenteModifier;
    QPushButton *btnVenteSupprimer;
    QTableWidget *tableVentes;
    QLineEdit *venteSearchEdit;
    QLabel *venteSortLabel;
    QComboBox *venteSortCombo;
    QWidget *tabHistorique;
    QFrame *frameHistorique;
    QTableWidget *tableHistorique;
    QPushButton *btnExportHistoriquePdf;
    QWidget *tabAssistantIA;
    QFrame *frameAssistantIA;
    QTextEdit *textEditAssistant;
    QLineEdit *lineEditAssistant;
    QPushButton *btnAssistantGo;
    QWidget *tabSource;
    QFrame *frameSourceMain;
    QPushButton *btnSourceAny;
    QWidget *welcomePage;
    QLabel *label_welcome;
    QWidget *widget_5;
    QLabel *label_20;
    QLabel *label_28;
    QLabel *label_32;
    QLabel *label_33;
    QLineEdit *lineEdit_5;
    QLineEdit *lineEdit_6;
    QPushButton *pushButton_3;
    QLabel *label_35;
    QLabel *label_36;
    QLabel *label_37;
    QPushButton *pushButton_6;
    QPushButton *pushButton;
    QLabel *label_38;
    QWidget *placeholderPage1;
    QLabel *label_p1;
    QLabel *label_39;
    QWidget *widget_6;
    QLabel *label_40;
    QLabel *label_41;
    QLabel *label_42;
    QLabel *label_43;
    QLabel *label_44;
    QLabel *label_45;
    QLabel *label_46;
    QPushButton *pushButton_8;
    QLabel *label_photo_3;
    QWidget *placeholderPage2;
    QLabel *label_p2;
    QLabel *label_47;
    QWidget *widget_7;
    QLabel *label_48;
    QLabel *label_49;
    QLabel *label_50;
    QLabel *label_51;
    QLineEdit *lineEdit_7;
    QLineEdit *lineEdit_9;
    QPushButton *pushButton_9;
    QLabel *label_52;
    QLabel *label_53;
    QLabel *label_54;
    QPushButton *pushButton_11;
    QWidget *placeholderPage3;
    QLabel *label_p3;
    QLabel *label_55;
    QWidget *widget_8;
    QLabel *label_56;
    QLabel *label_57;
    QLabel *label_58;
    QLabel *label_59;
    QLineEdit *lineEdit_10;
    QPushButton *pushButton_10;
    QLabel *label_60;
    QLabel *label_61;
    QLabel *label_62;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName("MainWindow");
        MainWindow->resize(1769, 934);
        MainWindow->setStyleSheet(QString::fromUtf8("/* Main Window */\n"
"QMainWindow {\n"
"    background-color: #f0f4f8;\n"
"}\n"
"\n"
"/* Sidebar */\n"
"#sidebar {\n"
"    background-color: #1a3a52;\n"
"    border-right: 3px solid #0d7377;\n"
"}\n"
"\n"
"#lblTitle {\n"
"    color: #ffffff;\n"
"    font-size: 18px;\n"
"    font-weight: bold;\n"
"    padding: 10px;\n"
"}\n"
"\n"
"/* Common Button Style (Base) */\n"
"QPushButton {\n"
"    border: none;\n"
"    border-radius: 5px;\n"
"    font-size: 14px;\n"
"    font-weight: 500;\n"
"}\n"
"\n"
"/* Sidebar Buttons */\n"
"#sidebar QPushButton {\n"
"    background-color: #2c5f7d;\n"
"    color: #ffffff;\n"
"    padding: 15px 20px;\n"
"    text-align: left;\n"
"    margin: 5px 10px;\n"
"    border-left: 4px solid transparent;\n"
"}\n"
"\n"
"#sidebar QPushButton:hover {\n"
"    background-color: #3e7a9e;\n"
"    border-left: 4px solid #14ffec;\n"
"}\n"
"\n"
"#sidebar QPushButton:checked {\n"
"    background-color: #0d7377;\n"
"    border-left: 6px solid #14ffec;\n"
"    font-weight: bold;\n"
"}\n"
"\n"
"/* Sub-navig"
                        "ation Bar Container */\n"
"#subNavBar {\n"
"    background-color: transparent;\n"
"}\n"
"\n"
"/* Sub-navigation Buttons */\n"
"#subNavBar QPushButton {\n"
"    background-color: #2c5f7d;\n"
"    color: #ffffff;\n"
"    padding: 8px 15px;\n"
"    text-align: center;\n"
"    margin: 5px;\n"
"    min-width: 120px;\n"
"    border-bottom: 3px solid transparent;\n"
"}\n"
"\n"
"#subNavBar QPushButton:hover {\n"
"    background-color: #3e7a9e;\n"
"    border-bottom: 3px solid #14ffec;\n"
"}\n"
"\n"
"#subNavBar QPushButton:checked {\n"
"    background-color: #0d7377;\n"
"    border-bottom: 4px solid #14ffec;\n"
"    font-weight: bold;\n"
"}\n"
"\n"
"/* Pecheur Sub-navigation Bar */\n"
"#subNavBarPecheurs {\n"
"    background-color: transparent;\n"
"}\n"
"\n"
"#subNavBarPecheurs QPushButton {\n"
"    background-color: #3CA99C;\n"
"    color: #ffffff;\n"
"    padding: 6px 14px;\n"
"    text-align: center;\n"
"    margin: 4px 6px;\n"
"    min-width: 90px;\n"
"    border: none;\n"
"    border-radius: 8px;\n"
"    font-fami"
                        "ly: \"Segoe UI\", Arial, sans-serif;\n"
"    font-size: 12px;\n"
"    font-weight: bold;\n"
"}\n"
"\n"
"#subNavBarPecheurs QPushButton:hover {\n"
"    background-color: #36a399;\n"
"}\n"
"\n"
"#subNavBarPecheurs QPushButton:checked {\n"
"    background-color: #2d8f85;\n"
"    font-weight: bold;\n"
"}\n"
"\n"
"/* Page Titles */\n"
".pageTitle {\n"
"    color: #1a3a52;\n"
"    font-size: 28px;\n"
"    font-weight: bold;\n"
"}\n"
"\n"
".subPageTitle {\n"
"    color: #2c5f7d;\n"
"    font-size: 24px;\n"
"    font-weight: 600;\n"
"}"));
        centralwidget = new QWidget(MainWindow);
        centralwidget->setObjectName("centralwidget");
        mainVerticalLayout = new QVBoxLayout(centralwidget);
        mainVerticalLayout->setSpacing(0);
        mainVerticalLayout->setObjectName("mainVerticalLayout");
        mainVerticalLayout->setContentsMargins(0, 0, 0, 0);
        mainStackedWidget = new QStackedWidget(centralwidget);
        mainStackedWidget->setObjectName("mainStackedWidget");
        dashboardPage = new QWidget();
        dashboardPage->setObjectName("dashboardPage");
        horizontalLayout = new QHBoxLayout(dashboardPage);
        horizontalLayout->setSpacing(0);
        horizontalLayout->setObjectName("horizontalLayout");
        horizontalLayout->setContentsMargins(0, 0, 0, 0);
        sidebar = new QFrame(dashboardPage);
        sidebar->setObjectName("sidebar");
        sidebar->setMinimumSize(QSize(250, 0));
        sidebar->setMaximumSize(QSize(250, 16777215));
        verticalLayout = new QVBoxLayout(sidebar);
        verticalLayout->setObjectName("verticalLayout");
        lblTitle = new QLabel(sidebar);
        lblTitle->setObjectName("lblTitle");
        lblTitle->setAlignment(Qt::AlignmentFlag::AlignCenter);

        verticalLayout->addWidget(lblTitle);

        verticalSpacer_2 = new QSpacerItem(20, 30, QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Fixed);

        verticalLayout->addItem(verticalSpacer_2);

        btnEmployes = new QPushButton(sidebar);
        btnEmployes->setObjectName("btnEmployes");
        btnEmployes->setCheckable(true);
        btnEmployes->setChecked(true);
        btnEmployes->setAutoExclusive(true);

        verticalLayout->addWidget(btnEmployes);

        btnPecheurs = new QPushButton(sidebar);
        btnPecheurs->setObjectName("btnPecheurs");
        btnPecheurs->setCheckable(true);
        btnPecheurs->setAutoExclusive(true);

        verticalLayout->addWidget(btnPecheurs);

        btnBateaux = new QPushButton(sidebar);
        btnBateaux->setObjectName("btnBateaux");
        btnBateaux->setCheckable(true);
        btnBateaux->setAutoExclusive(true);

        verticalLayout->addWidget(btnBateaux);

        btnMaintenance = new QPushButton(sidebar);
        btnMaintenance->setObjectName("btnMaintenance");
        btnMaintenance->setCheckable(true);
        btnMaintenance->setAutoExclusive(true);

        verticalLayout->addWidget(btnMaintenance);

        btnQuais = new QPushButton(sidebar);
        btnQuais->setObjectName("btnQuais");
        btnQuais->setCheckable(true);
        btnQuais->setAutoExclusive(true);

        verticalLayout->addWidget(btnQuais);

        btnStock_2 = new QPushButton(sidebar);
        btnStock_2->setObjectName("btnStock_2");
        btnStock_2->setCheckable(true);
        btnStock_2->setAutoExclusive(true);

        verticalLayout->addWidget(btnStock_2);

        verticalSpacer_3 = new QSpacerItem(20, 145, QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Expanding);

        verticalLayout->addItem(verticalSpacer_3);

        verticalSpacer_4 = new QSpacerItem(20, 144, QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Expanding);

        verticalLayout->addItem(verticalSpacer_4);

        btnStock = new QPushButton(sidebar);
        btnStock->setObjectName("btnStock");
        btnStock->setCheckable(true);
        btnStock->setAutoExclusive(true);

        verticalLayout->addWidget(btnStock);

        verticalSpacer = new QSpacerItem(20, 40, QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Expanding);

        verticalLayout->addItem(verticalSpacer);


        horizontalLayout->addWidget(sidebar);

        stackedWidget = new QStackedWidget(dashboardPage);
        stackedWidget->setObjectName("stackedWidget");
        stackedWidget->setStyleSheet(QString::fromUtf8("QMainWindow {\n"
"    background-color: #e8f4f8;\n"
"}\n"
"\n"
"QPushButton {\n"
"    background-color: #2a9d8f;\n"
"    color: white;\n"
"    border: none;\n"
"    padding: 10px;\n"
"    font-weight: bold;\n"
"    border-radius: 5px;\n"
"}\n"
"\n"
"QPushButton:hover {\n"
"    background-color: #21867a;\n"
"}\n"
"\n"
"QPushButton:pressed {\n"
"    background-color: #1e5f74;\n"
"}\n"
"\n"
"QTabWidget::pane {\n"
"    border: 2px solid #2a9d8f;\n"
"    background-color: #2b2b2b;\n"
"    border-radius: 5px;\n"
"}\n"
"\n"
"QTabBar::tab {\n"
"    background-color: #456b77;\n"
"    color: white;\n"
"    padding: 10px 20px;\n"
"    margin-right: 2px;\n"
"    border-top-left-radius: 4px;\n"
"    border-top-right-radius: 4px;\n"
"}\n"
"\n"
"QTabBar::tab:selected {\n"
"    background-color: #2a9d8f;\n"
"}\n"
"\n"
"QLabel {\n"
"    color: white;\n"
"}\n"
"\n"
"QLineEdit, QComboBox, QSpinBox, QDoubleSpinBox, QDateEdit, QTimeEdit {\n"
"    background-color: #3a3a3a;\n"
"    color: white;\n"
"    border: 1px solid #555;\n"
""
                        "    padding: 5px;\n"
"    border-radius: 3px;\n"
"}\n"
"\n"
"QTableWidget {\n"
"    background-color: #2b2b2b;\n"
"    color: white;\n"
"    gridline-color: #555;\n"
"    border: 1px solid #2a9d8f;\n"
"}\n"
"\n"
"QHeaderView::section {\n"
"    background-color: #1e5f74;\n"
"    color: white;\n"
"    padding: 5px;\n"
"    border: none;\n"
"    font-weight: bold;\n"
"}\n"
"\n"
"QGroupBox {\n"
"    color: #2a9d8f;\n"
"    font-weight: bold;\n"
"    border: 2px solid #2a9d8f;\n"
"    border-radius: 5px;\n"
"    margin-top: 10px;\n"
"    padding-top: 10px;\n"
"}\n"
"\n"
"QGroupBox::title {\n"
"    subcontrol-origin: margin;\n"
"    left: 10px;\n"
"    padding: 0 5px;\n"
"}\n"
"\n"
"QCheckBox, QRadioButton {\n"
"    color: white;\n"
"    padding: 5px;\n"
"}\n"
"\n"
"QTextEdit {\n"
"    background-color: #3a3a3a;\n"
"    color: white;\n"
"    border: 1px solid #555;\n"
"}"));
        pageEmployes = new QWidget();
        pageEmployes->setObjectName("pageEmployes");
        verticalLayout_2 = new QVBoxLayout(pageEmployes);
        verticalLayout_2->setSpacing(10);
        verticalLayout_2->setObjectName("verticalLayout_2");
        verticalLayout_2->setContentsMargins(20, 20, 20, -1);
        frame_3 = new QFrame(pageEmployes);
        frame_3->setObjectName("frame_3");
        frame_3->setStyleSheet(QString::fromUtf8(""));
        frame_3->setFrameShape(QFrame::Shape::StyledPanel);
        frame_3->setFrameShadow(QFrame::Shadow::Raised);
        horizontalLayout_4 = new QHBoxLayout(frame_3);
        horizontalLayout_4->setObjectName("horizontalLayout_4");
        title_label_2 = new QLabel(frame_3);
        title_label_2->setObjectName("title_label_2");
        QFont font;
        font.setFamilies({QString::fromUtf8("Script MT Bold")});
        font.setPointSize(30);
        title_label_2->setFont(font);
        title_label_2->setStyleSheet(QString::fromUtf8("background-image: url(:/new/prefix1/17.jpg);\n"
"background-image: url(:/new/prefix1/16.jpg);"));
        title_label_2->setAlignment(Qt::AlignmentFlag::AlignCenter);

        horizontalLayout_4->addWidget(title_label_2);


        verticalLayout_2->addWidget(frame_3);

        subNavBar = new QFrame(pageEmployes);
        subNavBar->setObjectName("subNavBar");
        subNavBar->setFrameShape(QFrame::Shape::NoFrame);
        subNavBarLayout = new QHBoxLayout(subNavBar);
        subNavBarLayout->setSpacing(5);
        subNavBarLayout->setObjectName("subNavBarLayout");
        subNavBarLayout->setContentsMargins(0, 0, 0, 0);
        btnSubEmpl3 = new QPushButton(subNavBar);
        btnSubEmpl3->setObjectName("btnSubEmpl3");
        btnSubEmpl3->setCheckable(true);
        btnSubEmpl3->setAutoExclusive(true);

        subNavBarLayout->addWidget(btnSubEmpl3);

        btnSubEmpl4_2 = new QPushButton(subNavBar);
        btnSubEmpl4_2->setObjectName("btnSubEmpl4_2");
        btnSubEmpl4_2->setCheckable(true);
        btnSubEmpl4_2->setAutoExclusive(true);

        subNavBarLayout->addWidget(btnSubEmpl4_2);

        btnSubEmpl5 = new QPushButton(subNavBar);
        btnSubEmpl5->setObjectName("btnSubEmpl5");
        btnSubEmpl5->setCheckable(true);
        btnSubEmpl5->setAutoExclusive(true);

        subNavBarLayout->addWidget(btnSubEmpl5);

        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);

        subNavBarLayout->addItem(horizontalSpacer);


        verticalLayout_2->addWidget(subNavBar);

        stackedWidgetEmployes = new QStackedWidget(pageEmployes);
        stackedWidgetEmployes->setObjectName("stackedWidgetEmployes");
        stackedWidgetEmployes->setEnabled(true);
        subPageEmpl3 = new QWidget();
        subPageEmpl3->setObjectName("subPageEmpl3");
        info_frame_2 = new QFrame(subPageEmpl3);
        info_frame_2->setObjectName("info_frame_2");
        info_frame_2->setGeometry(QRect(0, 0, 1221, 351));
        info_frame_2->setStyleSheet(QString::fromUtf8("QFrame {\n"
"	background-color: #232323;\n"
"	border-radius: 8px;\n"
"    border: 2px solid #3498db;\n"
"}\n"
"QLabel {\n"
"	color: #ffffff;\n"
"	font-family: \"Segoe UI\";\n"
"	font-weight: bold;\n"
"	font-size: 12px;\n"
"    padding: 2px;\n"
"    border: none;\n"
"}\n"
"QLineEdit, QComboBox, QDateEdit { \n"
"    color: white; \n"
"    background-color: #2d2d2d; \n"
"    border: 1px solid #3d3d3d; \n"
"    padding: 5px; \n"
"    border-radius: 4px; \n"
"    font-family: \"Segoe UI\";\n"
"    font-weight: bold;\n"
"}\n"
"QTabWidget::pane { border: 1px solid #3498db; background-color: #232323; border-radius: 5px; }\n"
"QTabBar::tab { background: #2d2d2d; color: #ffffff; padding: 10px; border-top-left-radius: 5px; border-top-right-radius: 5px; border: 1px solid #3d3d3d; width: 150px;}\n"
"QTabBar::tab:selected { background: #3498db; }"));
        info_frame_2->setFrameShape(QFrame::Shape::StyledPanel);
        info_frame_2->setFrameShadow(QFrame::Shadow::Raised);
        verticalLayout_tab = new QVBoxLayout(info_frame_2);
        verticalLayout_tab->setObjectName("verticalLayout_tab");
        tabWidget_employee = new QTabWidget(info_frame_2);
        tabWidget_employee->setObjectName("tabWidget_employee");
        tab_id = new QWidget();
        tab_id->setObjectName("tab_id");
        gridLayout_id = new QGridLayout(tab_id);
        gridLayout_id->setObjectName("gridLayout_id");
        label_lname = new QLabel(tab_id);
        label_lname->setObjectName("label_lname");

        gridLayout_id->addWidget(label_lname, 3, 0, 1, 1);

        label_id = new QLabel(tab_id);
        label_id->setObjectName("label_id");

        gridLayout_id->addWidget(label_id, 0, 0, 1, 1);

        label_photo = new QLabel(tab_id);
        label_photo->setObjectName("label_photo");
        label_photo->setMinimumSize(QSize(120, 120));
        label_photo->setAutoFillBackground(false);
        label_photo->setStyleSheet(QString::fromUtf8("border: 2px dashed #3498db; color: #7f8c8d;"));
        label_photo->setAlignment(Qt::AlignmentFlag::AlignCenter);

        gridLayout_id->addWidget(label_photo, 0, 2, 4, 1);

        label_cin = new QLabel(tab_id);
        label_cin->setObjectName("label_cin");

        gridLayout_id->addWidget(label_cin, 1, 0, 1, 1);

        label_fname = new QLabel(tab_id);
        label_fname->setObjectName("label_fname");

        gridLayout_id->addWidget(label_fname, 2, 0, 1, 1);

        btn_upload_photo = new QPushButton(tab_id);
        btn_upload_photo->setObjectName("btn_upload_photo");
        btn_upload_photo->setStyleSheet(QString::fromUtf8("background-color: #34495e; color: white; padding: 5px;"));

        gridLayout_id->addWidget(btn_upload_photo, 4, 2, 1, 1);

        lineEdit_cin = new QLineEdit(tab_id);
        lineEdit_cin->setObjectName("lineEdit_cin");

        gridLayout_id->addWidget(lineEdit_cin, 1, 1, 1, 1);

        lineEdit_fname = new QLineEdit(tab_id);
        lineEdit_fname->setObjectName("lineEdit_fname");

        gridLayout_id->addWidget(lineEdit_fname, 2, 1, 1, 1);

        lineEdit_lname = new QLineEdit(tab_id);
        lineEdit_lname->setObjectName("lineEdit_lname");

        gridLayout_id->addWidget(lineEdit_lname, 3, 1, 1, 1);

        lineEdit_id = new QLineEdit(tab_id);
        lineEdit_id->setObjectName("lineEdit_id");

        gridLayout_id->addWidget(lineEdit_id, 0, 1, 1, 1);

        tabWidget_employee->addTab(tab_id, QString());
        tab_pro = new QWidget();
        tab_pro->setObjectName("tab_pro");
        gridLayout_pro = new QGridLayout(tab_pro);
        gridLayout_pro->setObjectName("gridLayout_pro");
        label_job = new QLabel(tab_pro);
        label_job->setObjectName("label_job");

        gridLayout_pro->addWidget(label_job, 0, 0, 1, 1);

        lineEdit_job = new QLineEdit(tab_pro);
        lineEdit_job->setObjectName("lineEdit_job");

        gridLayout_pro->addWidget(lineEdit_job, 0, 1, 1, 1);

        label_dept = new QLabel(tab_pro);
        label_dept->setObjectName("label_dept");

        gridLayout_pro->addWidget(label_dept, 1, 0, 1, 1);

        combo_dept = new QComboBox(tab_pro);
        combo_dept->addItem(QString());
        combo_dept->addItem(QString());
        combo_dept->addItem(QString());
        combo_dept->addItem(QString());
        combo_dept->setObjectName("combo_dept");

        gridLayout_pro->addWidget(combo_dept, 1, 1, 1, 1);

        label_hdate = new QLabel(tab_pro);
        label_hdate->setObjectName("label_hdate");

        gridLayout_pro->addWidget(label_hdate, 2, 0, 1, 1);

        dateEdit_hire = new QDateEdit(tab_pro);
        dateEdit_hire->setObjectName("dateEdit_hire");

        gridLayout_pro->addWidget(dateEdit_hire, 2, 1, 1, 1);

        label_status = new QLabel(tab_pro);
        label_status->setObjectName("label_status");

        gridLayout_pro->addWidget(label_status, 3, 0, 1, 1);

        combo_status = new QComboBox(tab_pro);
        combo_status->addItem(QString());
        combo_status->addItem(QString());
        combo_status->addItem(QString());
        combo_status->setObjectName("combo_status");

        gridLayout_pro->addWidget(combo_status, 3, 1, 1, 1);

        tabWidget_employee->addTab(tab_pro, QString());
        tab_contact = new QWidget();
        tab_contact->setObjectName("tab_contact");
        gridLayout_contact = new QGridLayout(tab_contact);
        gridLayout_contact->setObjectName("gridLayout_contact");
        label_phone = new QLabel(tab_contact);
        label_phone->setObjectName("label_phone");

        gridLayout_contact->addWidget(label_phone, 0, 0, 1, 1);

        lineEdit_phone = new QLineEdit(tab_contact);
        lineEdit_phone->setObjectName("lineEdit_phone");

        gridLayout_contact->addWidget(lineEdit_phone, 0, 1, 1, 1);

        label_email = new QLabel(tab_contact);
        label_email->setObjectName("label_email");

        gridLayout_contact->addWidget(label_email, 1, 0, 1, 1);

        lineEdit_email = new QLineEdit(tab_contact);
        lineEdit_email->setObjectName("lineEdit_email");

        gridLayout_contact->addWidget(lineEdit_email, 1, 1, 1, 1);

        label_pwd = new QLabel(tab_contact);
        label_pwd->setObjectName("label_pwd");

        gridLayout_contact->addWidget(label_pwd, 2, 0, 1, 1);

        lineEdit_pwd = new QLineEdit(tab_contact);
        lineEdit_pwd->setObjectName("lineEdit_pwd");
        lineEdit_pwd->setEchoMode(QLineEdit::EchoMode::Password);

        gridLayout_contact->addWidget(lineEdit_pwd, 2, 1, 1, 1);

        label_city = new QLabel(tab_contact);
        label_city->setObjectName("label_city");

        gridLayout_contact->addWidget(label_city, 3, 0, 1, 1);

        lineEdit_city = new QLineEdit(tab_contact);
        lineEdit_city->setObjectName("lineEdit_city");

        gridLayout_contact->addWidget(lineEdit_city, 3, 1, 1, 1);

        label_addr = new QLabel(tab_contact);
        label_addr->setObjectName("label_addr");

        gridLayout_contact->addWidget(label_addr, 4, 0, 1, 1);

        lineEdit_addr = new QLineEdit(tab_contact);
        lineEdit_addr->setObjectName("lineEdit_addr");

        gridLayout_contact->addWidget(lineEdit_addr, 4, 1, 1, 1);

        tabWidget_employee->addTab(tab_contact, QString());
        tab_skills = new QWidget();
        tab_skills->setObjectName("tab_skills");
        gridLayout_skills = new QGridLayout(tab_skills);
        gridLayout_skills->setObjectName("gridLayout_skills");
        label_certs = new QLabel(tab_skills);
        label_certs->setObjectName("label_certs");

        gridLayout_skills->addWidget(label_certs, 0, 0, 1, 1);

        lineEdit_certs = new QLineEdit(tab_skills);
        lineEdit_certs->setObjectName("lineEdit_certs");

        gridLayout_skills->addWidget(lineEdit_certs, 0, 1, 1, 1);

        label_slevel = new QLabel(tab_skills);
        label_slevel->setObjectName("label_slevel");

        gridLayout_skills->addWidget(label_slevel, 1, 0, 1, 1);

        combo_slevel = new QComboBox(tab_skills);
        combo_slevel->addItem(QString());
        combo_slevel->addItem(QString());
        combo_slevel->addItem(QString());
        combo_slevel->setObjectName("combo_slevel");

        gridLayout_skills->addWidget(combo_slevel, 1, 1, 1, 1);

        label_lic = new QLabel(tab_skills);
        label_lic->setObjectName("label_lic");

        gridLayout_skills->addWidget(label_lic, 2, 0, 1, 1);

        lineEdit_lic = new QLineEdit(tab_skills);
        lineEdit_lic->setObjectName("lineEdit_lic");

        gridLayout_skills->addWidget(lineEdit_lic, 2, 1, 1, 1);

        label_exp = new QLabel(tab_skills);
        label_exp->setObjectName("label_exp");

        gridLayout_skills->addWidget(label_exp, 3, 0, 1, 1);

        dateEdit_exp = new QDateEdit(tab_skills);
        dateEdit_exp->setObjectName("dateEdit_exp");

        gridLayout_skills->addWidget(dateEdit_exp, 3, 1, 1, 1);

        label_shift = new QLabel(tab_skills);
        label_shift->setObjectName("label_shift");

        gridLayout_skills->addWidget(label_shift, 4, 0, 1, 1);

        combo_shift = new QComboBox(tab_skills);
        combo_shift->addItem(QString());
        combo_shift->addItem(QString());
        combo_shift->addItem(QString());
        combo_shift->setObjectName("combo_shift");

        gridLayout_skills->addWidget(combo_shift, 4, 1, 1, 1);

        tabWidget_employee->addTab(tab_skills, QString());

        verticalLayout_tab->addWidget(tabWidget_employee);

        function_frame = new QFrame(subPageEmpl3);
        function_frame->setObjectName("function_frame");
        function_frame->setGeometry(QRect(0, 360, 1221, 61));
        function_frame->setStyleSheet(QString::fromUtf8("QFrame#function_frame { background-color: #34495e; border-radius: 8px; padding: 8px; border: 1px solid #2c3e50; }"));
        function_frame->setFrameShape(QFrame::Shape::StyledPanel);
        function_frame->setFrameShadow(QFrame::Shadow::Raised);
        horizontalLayout_2 = new QHBoxLayout(function_frame);
        horizontalLayout_2->setObjectName("horizontalLayout_2");
        add_btn_2 = new QPushButton(function_frame);
        add_btn_2->setObjectName("add_btn_2");
        QFont font1;
        font1.setFamilies({QString::fromUtf8("Segoe UI")});
        font1.setBold(true);
        add_btn_2->setFont(font1);
        add_btn_2->setStyleSheet(QString::fromUtf8("QPushButton { border: 1px solid #bdc3c7;\n"
"	background-color: #3498db;\n"
"	color: white;\n"
"	font-family: \"Segoe UI\";\n"
"	font-size: 14px;\n"
"	font-weight: bold;\n"
"	border-radius: 5px;\n"
"	padding: 8px 15px;\n"
"	border: none;\n"
"}\n"
"QPushButton:hover {\n"
"	background-color: #2980b9;\n"
"}\n"
"QPushButton:pressed {\n"
"	background-color: #21618c;\n"
"}"));
        QIcon icon;
        icon.addFile(QString::fromUtf8("../PyQt5-Video-Book-main/PyQt5-Video-Book-main/#017_Students information system/icons/add.svg"), QSize(), QIcon::Mode::Normal, QIcon::State::Off);
        add_btn_2->setIcon(icon);
        add_btn_2->setIconSize(QSize(20, 20));

        horizontalLayout_2->addWidget(add_btn_2);

        clear_btn_2 = new QPushButton(function_frame);
        clear_btn_2->setObjectName("clear_btn_2");
        clear_btn_2->setStyleSheet(QString::fromUtf8("QPushButton { border: 1px solid #bdc3c7;\n"
"	background-color: #3498db;\n"
"	color: white;\n"
"	font-family: \"Segoe UI\";\n"
"	font-size: 14px;\n"
"	font-weight: bold;\n"
"	border-radius: 5px;\n"
"	padding: 8px 15px;\n"
"	border: none;\n"
"}\n"
"QPushButton:hover {\n"
"	background-color: #2980b9;\n"
"}\n"
"QPushButton:pressed {\n"
"	background-color: #21618c;\n"
"}"));
        QIcon icon1;
        icon1.addFile(QString::fromUtf8("../PyQt5-Video-Book-main/PyQt5-Video-Book-main/#017_Students information system/icons/clear.svg"), QSize(), QIcon::Mode::Normal, QIcon::State::Off);
        clear_btn_2->setIcon(icon1);
        clear_btn_2->setIconSize(QSize(20, 20));

        horizontalLayout_2->addWidget(clear_btn_2);

        update_btn_2 = new QPushButton(function_frame);
        update_btn_2->setObjectName("update_btn_2");
        update_btn_2->setStyleSheet(QString::fromUtf8("QPushButton { background-color: #3498db; color: white; font-weight: bold; border-radius: 4px; padding: 5px; } QPushButton:hover { background-color: #2980b9; }"));
        QIcon icon2;
        icon2.addFile(QString::fromUtf8("../PyQt5-Video-Book-main/PyQt5-Video-Book-main/#017_Students information system/icons/update.svg"), QSize(), QIcon::Mode::Normal, QIcon::State::Off);
        update_btn_2->setIcon(icon2);
        update_btn_2->setIconSize(QSize(20, 20));

        horizontalLayout_2->addWidget(update_btn_2);

        delete_btn_2 = new QPushButton(function_frame);
        delete_btn_2->setObjectName("delete_btn_2");
        delete_btn_2->setStyleSheet(QString::fromUtf8("QPushButton { border: 1px solid #bdc3c7;\n"
"	background-color: #e74c3c;\n"
"	color: white;\n"
"	font-family: \"Segoe UI\";\n"
"	font-size: 14px;\n"
"	font-weight: bold;\n"
"	border-radius: 5px;\n"
"	padding: 8px 15px;\n"
"	border: none;\n"
"}\n"
"QPushButton:hover {\n"
"	background-color: #c0392b;\n"
"}\n"
"QPushButton:pressed {\n"
"	background-color: #a93226;\n"
"}"));
        QIcon icon3;
        icon3.addFile(QString::fromUtf8("../PyQt5-Video-Book-main/PyQt5-Video-Book-main/#017_Students information system/icons/delete.svg"), QSize(), QIcon::Mode::Normal, QIcon::State::Off);
        delete_btn_2->setIcon(icon3);
        delete_btn_2->setIconSize(QSize(20, 20));

        horizontalLayout_2->addWidget(delete_btn_2);

        search_input_2 = new QLineEdit(function_frame);
        search_input_2->setObjectName("search_input_2");
        search_input_2->setMinimumSize(QSize(200, 0));
        search_input_2->setStyleSheet(QString::fromUtf8("QLineEdit { color: white; background-color: #2d2d2d; border: 1px solid #3498db; border-radius: 4px; padding: 5px; }"));

        horizontalLayout_2->addWidget(search_input_2);

        clear_btn_3 = new QPushButton(function_frame);
        clear_btn_3->setObjectName("clear_btn_3");
        clear_btn_3->setStyleSheet(QString::fromUtf8("QPushButton { border: 1px solid #bdc3c7;\n"
"	background-color: #3498db;\n"
"	color: white;\n"
"	font-family: \"Segoe UI\";\n"
"	font-size: 14px;\n"
"	font-weight: bold;\n"
"	border-radius: 5px;\n"
"	padding: 8px 15px;\n"
"	border: none;\n"
"}\n"
"QPushButton:hover {\n"
"	background-color: #2980b9;\n"
"}\n"
"QPushButton:pressed {\n"
"	background-color: #21618c;\n"
"}"));
        clear_btn_3->setIcon(icon1);
        clear_btn_3->setIconSize(QSize(20, 20));

        horizontalLayout_2->addWidget(clear_btn_3);

        search_input = new QLineEdit(function_frame);
        search_input->setObjectName("search_input");
        search_input->setMinimumSize(QSize(200, 0));
        search_input->setStyleSheet(QString::fromUtf8("QLineEdit { color: white; background-color: #2d2d2d; border: 1px solid #3498db; border-radius: 4px; padding: 5px; }"));

        horizontalLayout_2->addWidget(search_input);

        search_btn_2 = new QPushButton(function_frame);
        search_btn_2->setObjectName("search_btn_2");
        search_btn_2->setStyleSheet(QString::fromUtf8("QPushButton { border: 1px solid #bdc3c7;\n"
"	background-color: #3498db;\n"
"	color: white;\n"
"	font-family: \"Segoe UI\";\n"
"	font-size: 14px;\n"
"	font-weight: bold;\n"
"	border-radius: 5px;\n"
"	padding: 8px 15px;\n"
"	border: none;\n"
"}\n"
"QPushButton:hover {\n"
"	background-color: #2980b9;\n"
"}\n"
"QPushButton:pressed {\n"
"	background-color: #21618c;\n"
"}"));
        QIcon icon4;
        icon4.addFile(QString::fromUtf8("../PyQt5-Video-Book-main/PyQt5-Video-Book-main/#017_Students information system/icons/search.svg"), QSize(), QIcon::Mode::Normal, QIcon::State::Off);
        search_btn_2->setIcon(icon4);
        search_btn_2->setIconSize(QSize(20, 20));

        horizontalLayout_2->addWidget(search_btn_2);

        sort_combo = new QComboBox(function_frame);
        sort_combo->addItem(QString());
        sort_combo->addItem(QString());
        sort_combo->addItem(QString());
        sort_combo->addItem(QString());
        sort_combo->setObjectName("sort_combo");
        sort_combo->setMinimumSize(QSize(150, 0));
        sort_combo->setStyleSheet(QString::fromUtf8("QComboBox { color: white; background-color: #2d2d2d; border: 1px solid #3498db; border-radius: 4px; padding: 5px; }"));

        horizontalLayout_2->addWidget(sort_combo);

        export_excel_btn = new QPushButton(function_frame);
        export_excel_btn->setObjectName("export_excel_btn");
        export_excel_btn->setStyleSheet(QString::fromUtf8("QPushButton { background-color: #f39c12; color: white; border-radius: 5px; padding: 8px; font-weight: bold; border: none; }\n"
"QPushButton:hover { background-color: #e67e22; }"));

        horizontalLayout_2->addWidget(export_excel_btn);

        result_frame = new QFrame(subPageEmpl3);
        result_frame->setObjectName("result_frame");
        result_frame->setGeometry(QRect(0, 440, 1231, 221));
        result_frame->setStyleSheet(QString::fromUtf8(""));
        result_frame->setFrameShape(QFrame::Shape::StyledPanel);
        result_frame->setFrameShadow(QFrame::Shadow::Raised);
        gridLayout_5 = new QGridLayout(result_frame);
        gridLayout_5->setObjectName("gridLayout_5");
        tableWidget_2 = new QTableWidget(result_frame);
        if (tableWidget_2->columnCount() < 17)
            tableWidget_2->setColumnCount(17);
        QFont font2;
        font2.setFamilies({QString::fromUtf8("Segoe UI")});
        font2.setPointSize(10);
        QTableWidgetItem *__qtablewidgetitem = new QTableWidgetItem();
        __qtablewidgetitem->setTextAlignment(Qt::AlignLeading|Qt::AlignVCenter);
        __qtablewidgetitem->setFont(font2);
        tableWidget_2->setHorizontalHeaderItem(0, __qtablewidgetitem);
        QTableWidgetItem *__qtablewidgetitem1 = new QTableWidgetItem();
        tableWidget_2->setHorizontalHeaderItem(1, __qtablewidgetitem1);
        QTableWidgetItem *__qtablewidgetitem2 = new QTableWidgetItem();
        __qtablewidgetitem2->setTextAlignment(Qt::AlignLeading|Qt::AlignVCenter);
        __qtablewidgetitem2->setFont(font2);
        tableWidget_2->setHorizontalHeaderItem(2, __qtablewidgetitem2);
        QTableWidgetItem *__qtablewidgetitem3 = new QTableWidgetItem();
        __qtablewidgetitem3->setTextAlignment(Qt::AlignLeading|Qt::AlignVCenter);
        tableWidget_2->setHorizontalHeaderItem(3, __qtablewidgetitem3);
        QTableWidgetItem *__qtablewidgetitem4 = new QTableWidgetItem();
        __qtablewidgetitem4->setTextAlignment(Qt::AlignLeading|Qt::AlignVCenter);
        tableWidget_2->setHorizontalHeaderItem(4, __qtablewidgetitem4);
        QTableWidgetItem *__qtablewidgetitem5 = new QTableWidgetItem();
        tableWidget_2->setHorizontalHeaderItem(5, __qtablewidgetitem5);
        QTableWidgetItem *__qtablewidgetitem6 = new QTableWidgetItem();
        __qtablewidgetitem6->setTextAlignment(Qt::AlignLeading|Qt::AlignVCenter);
        tableWidget_2->setHorizontalHeaderItem(6, __qtablewidgetitem6);
        QTableWidgetItem *__qtablewidgetitem7 = new QTableWidgetItem();
        __qtablewidgetitem7->setTextAlignment(Qt::AlignLeading|Qt::AlignVCenter);
        tableWidget_2->setHorizontalHeaderItem(7, __qtablewidgetitem7);
        QTableWidgetItem *__qtablewidgetitem8 = new QTableWidgetItem();
        __qtablewidgetitem8->setTextAlignment(Qt::AlignLeading|Qt::AlignVCenter);
        tableWidget_2->setHorizontalHeaderItem(8, __qtablewidgetitem8);
        QTableWidgetItem *__qtablewidgetitem9 = new QTableWidgetItem();
        __qtablewidgetitem9->setTextAlignment(Qt::AlignLeading|Qt::AlignVCenter);
        tableWidget_2->setHorizontalHeaderItem(9, __qtablewidgetitem9);
        QTableWidgetItem *__qtablewidgetitem10 = new QTableWidgetItem();
        __qtablewidgetitem10->setTextAlignment(Qt::AlignLeading|Qt::AlignVCenter);
        tableWidget_2->setHorizontalHeaderItem(10, __qtablewidgetitem10);
        QTableWidgetItem *__qtablewidgetitem11 = new QTableWidgetItem();
        __qtablewidgetitem11->setTextAlignment(Qt::AlignLeading|Qt::AlignVCenter);
        tableWidget_2->setHorizontalHeaderItem(11, __qtablewidgetitem11);
        QTableWidgetItem *__qtablewidgetitem12 = new QTableWidgetItem();
        tableWidget_2->setHorizontalHeaderItem(12, __qtablewidgetitem12);
        QTableWidgetItem *__qtablewidgetitem13 = new QTableWidgetItem();
        tableWidget_2->setHorizontalHeaderItem(13, __qtablewidgetitem13);
        QTableWidgetItem *__qtablewidgetitem14 = new QTableWidgetItem();
        tableWidget_2->setHorizontalHeaderItem(14, __qtablewidgetitem14);
        QTableWidgetItem *__qtablewidgetitem15 = new QTableWidgetItem();
        tableWidget_2->setHorizontalHeaderItem(15, __qtablewidgetitem15);
        QTableWidgetItem *__qtablewidgetitem16 = new QTableWidgetItem();
        tableWidget_2->setHorizontalHeaderItem(16, __qtablewidgetitem16);
        if (tableWidget_2->rowCount() < 3)
            tableWidget_2->setRowCount(3);
        QTableWidgetItem *__qtablewidgetitem17 = new QTableWidgetItem();
        tableWidget_2->setVerticalHeaderItem(0, __qtablewidgetitem17);
        QTableWidgetItem *__qtablewidgetitem18 = new QTableWidgetItem();
        tableWidget_2->setVerticalHeaderItem(1, __qtablewidgetitem18);
        QTableWidgetItem *__qtablewidgetitem19 = new QTableWidgetItem();
        tableWidget_2->setItem(0, 0, __qtablewidgetitem19);
        QTableWidgetItem *__qtablewidgetitem20 = new QTableWidgetItem();
        tableWidget_2->setItem(0, 1, __qtablewidgetitem20);
        QTableWidgetItem *__qtablewidgetitem21 = new QTableWidgetItem();
        tableWidget_2->setItem(0, 2, __qtablewidgetitem21);
        QTableWidgetItem *__qtablewidgetitem22 = new QTableWidgetItem();
        tableWidget_2->setItem(0, 3, __qtablewidgetitem22);
        QTableWidgetItem *__qtablewidgetitem23 = new QTableWidgetItem();
        tableWidget_2->setItem(0, 4, __qtablewidgetitem23);
        QTableWidgetItem *__qtablewidgetitem24 = new QTableWidgetItem();
        tableWidget_2->setItem(0, 5, __qtablewidgetitem24);
        QTableWidgetItem *__qtablewidgetitem25 = new QTableWidgetItem();
        tableWidget_2->setItem(0, 6, __qtablewidgetitem25);
        QTableWidgetItem *__qtablewidgetitem26 = new QTableWidgetItem();
        tableWidget_2->setItem(0, 7, __qtablewidgetitem26);
        QTableWidgetItem *__qtablewidgetitem27 = new QTableWidgetItem();
        tableWidget_2->setItem(0, 8, __qtablewidgetitem27);
        QTableWidgetItem *__qtablewidgetitem28 = new QTableWidgetItem();
        tableWidget_2->setItem(0, 9, __qtablewidgetitem28);
        QTableWidgetItem *__qtablewidgetitem29 = new QTableWidgetItem();
        tableWidget_2->setItem(2, 0, __qtablewidgetitem29);
        tableWidget_2->setObjectName("tableWidget_2");
        tableWidget_2->setFont(font1);
        tableWidget_2->setFocusPolicy(Qt::FocusPolicy::NoFocus);
        tableWidget_2->setStyleSheet(QString::fromUtf8("QTableWidget {\n"
"	background-color: #2d2d2d;\n"
"	gridline-color: #3d3d3d;\n"
"	font-family: \"Segoe UI\";\n"
"	font-size: 12px;\n"
"    font-weight: bold;\n"
"	selection-background-color: #3498db;\n"
"    border-radius: 8px;\n"
"    border: 2px solid #3498db;\n"
"    color: white;\n"
"}\n"
"QHeaderView::section {\n"
"	background-color: #3d3d3d;\n"
"	color: white;\n"
"	padding: 6px;\n"
"	border: 1px solid #232323;\n"
"	font-weight: bold;\n"
"    font-family: \"Segoe UI\";\n"
"}"));
        tableWidget_2->setFrameShape(QFrame::Shape::NoFrame);
        tableWidget_2->setSelectionMode(QAbstractItemView::SelectionMode::ExtendedSelection);
        tableWidget_2->setSelectionBehavior(QAbstractItemView::SelectionBehavior::SelectRows);
        tableWidget_2->setShowGrid(false);
        tableWidget_2->setGridStyle(Qt::PenStyle::NoPen);
        tableWidget_2->setSortingEnabled(true);
        tableWidget_2->setWordWrap(true);
        tableWidget_2->setCornerButtonEnabled(false);
        tableWidget_2->setRowCount(3);
        tableWidget_2->horizontalHeader()->setCascadingSectionResizes(false);
        tableWidget_2->horizontalHeader()->setMinimumSectionSize(50);
        tableWidget_2->horizontalHeader()->setDefaultSectionSize(120);
        tableWidget_2->horizontalHeader()->setStretchLastSection(true);
        tableWidget_2->verticalHeader()->setVisible(false);
        tableWidget_2->verticalHeader()->setCascadingSectionResizes(false);
        tableWidget_2->verticalHeader()->setDefaultSectionSize(28);
        tableWidget_2->verticalHeader()->setHighlightSections(false);
        tableWidget_2->verticalHeader()->setProperty("showSortIndicator", QVariant(true));
        tableWidget_2->verticalHeader()->setStretchLastSection(false);

        gridLayout_5->addWidget(tableWidget_2, 0, 0, 1, 1);

        stackedWidgetEmployes->addWidget(subPageEmpl3);
        subPageEmpl4 = new QWidget();
        subPageEmpl4->setObjectName("subPageEmpl4");
        lblMetierTitle = new QLabel(subPageEmpl4);
        lblMetierTitle->setObjectName("lblMetierTitle");
        lblMetierTitle->setGeometry(QRect(40, -10, 1131, 50));
        QFont font3;
        font3.setFamilies({QString::fromUtf8("Segoe UI")});
        font3.setPointSize(24);
        font3.setBold(true);
        lblMetierTitle->setFont(font3);
        lblMetierTitle->setStyleSheet(QString::fromUtf8("color: #1a3a52; background-color: transparent;\n"
"background-image: url(:/new/prefix1/18.jpg);\n"
"color: rgb(255, 255, 255);"));
        lblMetierTitle->setAlignment(Qt::AlignmentFlag::AlignCenter);
        frameCard1 = new QFrame(subPageEmpl4);
        frameCard1->setObjectName("frameCard1");
        frameCard1->setGeometry(QRect(80, 60, 250, 140));
        frameCard1->setStyleSheet(QString::fromUtf8("QFrame { background-color: #3498db; border-radius: 10px; border: 2px solid #2980b9; }\n"
"QLabel { color: white; border: none; }"));
        frameCard1->setFrameShape(QFrame::Shape::StyledPanel);
        lblTotalEmployees = new QLabel(frameCard1);
        lblTotalEmployees->setObjectName("lblTotalEmployees");
        lblTotalEmployees->setGeometry(QRect(20, 20, 210, 30));
        QFont font4;
        font4.setPointSize(12);
        font4.setBold(true);
        lblTotalEmployees->setFont(font4);
        lblTotalEmployeesValue = new QLabel(frameCard1);
        lblTotalEmployeesValue->setObjectName("lblTotalEmployeesValue");
        lblTotalEmployeesValue->setGeometry(QRect(20, 60, 210, 60));
        QFont font5;
        font5.setPointSize(36);
        font5.setBold(true);
        lblTotalEmployeesValue->setFont(font5);
        lblTotalEmployeesValue->setAlignment(Qt::AlignmentFlag::AlignCenter);
        frameCard2 = new QFrame(subPageEmpl4);
        frameCard2->setObjectName("frameCard2");
        frameCard2->setGeometry(QRect(350, 60, 250, 140));
        frameCard2->setStyleSheet(QString::fromUtf8("QFrame { background-color: #2ecc71; border-radius: 10px; border: 2px solid #27ae60; }\n"
"QLabel { color: white; border: none; }"));
        frameCard2->setFrameShape(QFrame::Shape::StyledPanel);
        lblActiveEmployees = new QLabel(frameCard2);
        lblActiveEmployees->setObjectName("lblActiveEmployees");
        lblActiveEmployees->setGeometry(QRect(20, 20, 210, 30));
        lblActiveEmployees->setFont(font4);
        lblActiveEmployeesValue = new QLabel(frameCard2);
        lblActiveEmployeesValue->setObjectName("lblActiveEmployeesValue");
        lblActiveEmployeesValue->setGeometry(QRect(20, 60, 210, 60));
        lblActiveEmployeesValue->setFont(font5);
        lblActiveEmployeesValue->setAlignment(Qt::AlignmentFlag::AlignCenter);
        frameCard3 = new QFrame(subPageEmpl4);
        frameCard3->setObjectName("frameCard3");
        frameCard3->setGeometry(QRect(620, 60, 250, 140));
        frameCard3->setStyleSheet(QString::fromUtf8("QFrame { background-color: #e74c3c; border-radius: 10px; border: 2px solid #c0392b; }\n"
"QLabel { color: white; border: none; }"));
        frameCard3->setFrameShape(QFrame::Shape::StyledPanel);
        lblOnLeave = new QLabel(frameCard3);
        lblOnLeave->setObjectName("lblOnLeave");
        lblOnLeave->setGeometry(QRect(20, 20, 210, 30));
        lblOnLeave->setFont(font4);
        lblOnLeaveValue = new QLabel(frameCard3);
        lblOnLeaveValue->setObjectName("lblOnLeaveValue");
        lblOnLeaveValue->setGeometry(QRect(20, 60, 210, 60));
        lblOnLeaveValue->setFont(font5);
        lblOnLeaveValue->setAlignment(Qt::AlignmentFlag::AlignCenter);
        frameCard4 = new QFrame(subPageEmpl4);
        frameCard4->setObjectName("frameCard4");
        frameCard4->setGeometry(QRect(890, 60, 250, 140));
        frameCard4->setStyleSheet(QString::fromUtf8("QFrame { background-color: #f39c12; border-radius: 10px; border: 2px solid #e67e22; }\n"
"QLabel { color: white; border: none; }"));
        frameCard4->setFrameShape(QFrame::Shape::StyledPanel);
        lblCertExpiring = new QLabel(frameCard4);
        lblCertExpiring->setObjectName("lblCertExpiring");
        lblCertExpiring->setGeometry(QRect(20, 20, 210, 30));
        lblCertExpiring->setFont(font4);
        lblCertExpiringValue = new QLabel(frameCard4);
        lblCertExpiringValue->setObjectName("lblCertExpiringValue");
        lblCertExpiringValue->setGeometry(QRect(20, 60, 210, 60));
        lblCertExpiringValue->setFont(font5);
        lblCertExpiringValue->setAlignment(Qt::AlignmentFlag::AlignCenter);
        frameDepartments = new QFrame(subPageEmpl4);
        frameDepartments->setObjectName("frameDepartments");
        frameDepartments->setGeometry(QRect(80, 220, 520, 201));
        frameDepartments->setStyleSheet(QString::fromUtf8("QFrame { background-color: #34495e; border-radius: 10px; border: 2px solid #2c3e50; }\n"
"QLabel { color: white; border: none; }\n"
"QProgressBar { background-color: #2c3e50; border: 1px solid #1a252f; border-radius: 5px; text-align: center; color: white; }\n"
"QProgressBar::chunk { background-color: #3498db; border-radius: 5px; }"));
        frameDepartments->setFrameShape(QFrame::Shape::StyledPanel);
        lblDeptTitle = new QLabel(frameDepartments);
        lblDeptTitle->setObjectName("lblDeptTitle");
        lblDeptTitle->setGeometry(QRect(20, 15, 480, 30));
        QFont font6;
        font6.setPointSize(14);
        font6.setBold(true);
        lblDeptTitle->setFont(font6);
        lblDept1 = new QLabel(frameDepartments);
        lblDept1->setObjectName("lblDept1");
        lblDept1->setGeometry(QRect(30, 55, 150, 25));
        progressDept1 = new QProgressBar(frameDepartments);
        progressDept1->setObjectName("progressDept1");
        progressDept1->setGeometry(QRect(190, 55, 300, 25));
        progressDept1->setValue(35);
        lblDept2 = new QLabel(frameDepartments);
        lblDept2->setObjectName("lblDept2");
        lblDept2->setGeometry(QRect(30, 90, 150, 25));
        progressDept2 = new QProgressBar(frameDepartments);
        progressDept2->setObjectName("progressDept2");
        progressDept2->setGeometry(QRect(190, 90, 300, 25));
        progressDept2->setValue(28);
        lblDept3 = new QLabel(frameDepartments);
        lblDept3->setObjectName("lblDept3");
        lblDept3->setGeometry(QRect(30, 125, 150, 25));
        progressDept3 = new QProgressBar(frameDepartments);
        progressDept3->setObjectName("progressDept3");
        progressDept3->setGeometry(QRect(190, 125, 300, 25));
        progressDept3->setValue(22);
        lblDept4 = new QLabel(frameDepartments);
        lblDept4->setObjectName("lblDept4");
        lblDept4->setGeometry(QRect(30, 160, 150, 25));
        progressDept4 = new QProgressBar(frameDepartments);
        progressDept4->setObjectName("progressDept4");
        progressDept4->setGeometry(QRect(190, 160, 300, 25));
        progressDept4->setValue(15);
        frameSkills = new QFrame(subPageEmpl4);
        frameSkills->setObjectName("frameSkills");
        frameSkills->setGeometry(QRect(620, 220, 520, 201));
        frameSkills->setStyleSheet(QString::fromUtf8("QFrame { background-color: #16a085; border-radius: 10px; border: 2px solid #138d75; }\n"
"QLabel { color: white; border: none; }\n"
"QProgressBar { background-color: #138d75; border: 1px solid #0e6655; border-radius: 5px; text-align: center; color: white; }\n"
"QProgressBar::chunk { background-color: #1abc9c; border-radius: 5px; }"));
        frameSkills->setFrameShape(QFrame::Shape::StyledPanel);
        lblSkillsTitle = new QLabel(frameSkills);
        lblSkillsTitle->setObjectName("lblSkillsTitle");
        lblSkillsTitle->setGeometry(QRect(20, 15, 480, 30));
        lblSkillsTitle->setFont(font6);
        lblSkill1 = new QLabel(frameSkills);
        lblSkill1->setObjectName("lblSkill1");
        lblSkill1->setGeometry(QRect(30, 55, 150, 25));
        progressSkill1 = new QProgressBar(frameSkills);
        progressSkill1->setObjectName("progressSkill1");
        progressSkill1->setGeometry(QRect(190, 55, 300, 25));
        progressSkill1->setValue(18);
        lblSkill2 = new QLabel(frameSkills);
        lblSkill2->setObjectName("lblSkill2");
        lblSkill2->setGeometry(QRect(30, 90, 150, 25));
        progressSkill2 = new QProgressBar(frameSkills);
        progressSkill2->setObjectName("progressSkill2");
        progressSkill2->setGeometry(QRect(190, 90, 300, 25));
        progressSkill2->setValue(52);
        lblSkill3 = new QLabel(frameSkills);
        lblSkill3->setObjectName("lblSkill3");
        lblSkill3->setGeometry(QRect(30, 125, 150, 25));
        progressSkill3 = new QProgressBar(frameSkills);
        progressSkill3->setObjectName("progressSkill3");
        progressSkill3->setGeometry(QRect(190, 125, 300, 25));
        progressSkill3->setValue(30);
        frameShifts = new QFrame(subPageEmpl4);
        frameShifts->setObjectName("frameShifts");
        frameShifts->setGeometry(QRect(80, 440, 340, 180));
        frameShifts->setStyleSheet(QString::fromUtf8("QFrame { background-color: #8e44ad; border-radius: 10px; border: 2px solid #7d3c98; }\n"
"QLabel { color: white; border: none; }\n"
"QProgressBar { background-color: #7d3c98; border: 1px solid #6c3483; border-radius: 5px; text-align: center; color: white; }\n"
"QProgressBar::chunk { background-color: #9b59b6; border-radius: 5px; }"));
        frameShifts->setFrameShape(QFrame::Shape::StyledPanel);
        lblShiftsTitle = new QLabel(frameShifts);
        lblShiftsTitle->setObjectName("lblShiftsTitle");
        lblShiftsTitle->setGeometry(QRect(20, 15, 300, 30));
        lblShiftsTitle->setFont(font6);
        lblShift1 = new QLabel(frameShifts);
        lblShift1->setObjectName("lblShift1");
        lblShift1->setGeometry(QRect(30, 55, 100, 25));
        progressShift1 = new QProgressBar(frameShifts);
        progressShift1->setObjectName("progressShift1");
        progressShift1->setGeometry(QRect(140, 55, 180, 25));
        progressShift1->setValue(58);
        lblShift2 = new QLabel(frameShifts);
        lblShift2->setObjectName("lblShift2");
        lblShift2->setGeometry(QRect(30, 90, 100, 25));
        progressShift2 = new QProgressBar(frameShifts);
        progressShift2->setObjectName("progressShift2");
        progressShift2->setGeometry(QRect(140, 90, 180, 25));
        progressShift2->setValue(42);
        frameCities = new QFrame(subPageEmpl4);
        frameCities->setObjectName("frameCities");
        frameCities->setGeometry(QRect(440, 440, 340, 180));
        frameCities->setStyleSheet(QString::fromUtf8("QFrame { background-color: #d35400; border-radius: 10px; border: 2px solid #ba4a00; }\n"
"QLabel { color: white; border: none; }\n"
"QProgressBar { background-color: #ba4a00; border: 1px solid #a04000; border-radius: 5px; text-align: center; color: white; }\n"
"QProgressBar::chunk { background-color: #e67e22; border-radius: 5px; }"));
        frameCities->setFrameShape(QFrame::Shape::StyledPanel);
        lblCitiesTitle = new QLabel(frameCities);
        lblCitiesTitle->setObjectName("lblCitiesTitle");
        lblCitiesTitle->setGeometry(QRect(20, 15, 300, 30));
        lblCitiesTitle->setFont(font6);
        lblCity1 = new QLabel(frameCities);
        lblCity1->setObjectName("lblCity1");
        lblCity1->setGeometry(QRect(30, 55, 100, 25));
        progressCity1 = new QProgressBar(frameCities);
        progressCity1->setObjectName("progressCity1");
        progressCity1->setGeometry(QRect(140, 55, 180, 25));
        progressCity1->setValue(65);
        lblCity2 = new QLabel(frameCities);
        lblCity2->setObjectName("lblCity2");
        lblCity2->setGeometry(QRect(30, 90, 100, 25));
        progressCity2 = new QProgressBar(frameCities);
        progressCity2->setObjectName("progressCity2");
        progressCity2->setGeometry(QRect(140, 90, 180, 25));
        progressCity2->setValue(20);
        lblCity3 = new QLabel(frameCities);
        lblCity3->setObjectName("lblCity3");
        lblCity3->setGeometry(QRect(30, 125, 100, 25));
        progressCity3 = new QProgressBar(frameCities);
        progressCity3->setObjectName("progressCity3");
        progressCity3->setGeometry(QRect(140, 125, 180, 25));
        progressCity3->setValue(15);
        frameAlerts = new QFrame(subPageEmpl4);
        frameAlerts->setObjectName("frameAlerts");
        frameAlerts->setGeometry(QRect(800, 440, 340, 180));
        frameAlerts->setStyleSheet(QString::fromUtf8("QFrame { background-color: #c0392b; border-radius: 10px; border: 2px solid #a93226; }\n"
"QLabel { color: white; border: none; }"));
        frameAlerts->setFrameShape(QFrame::Shape::StyledPanel);
        lblAlertsTitle = new QLabel(frameAlerts);
        lblAlertsTitle->setObjectName("lblAlertsTitle");
        lblAlertsTitle->setGeometry(QRect(20, 15, 300, 30));
        lblAlertsTitle->setFont(font6);
        lblAlert1 = new QLabel(frameAlerts);
        lblAlert1->setObjectName("lblAlert1");
        lblAlert1->setGeometry(QRect(20, 55, 300, 25));
        QFont font7;
        font7.setPointSize(9);
        lblAlert1->setFont(font7);
        lblAlert2 = new QLabel(frameAlerts);
        lblAlert2->setObjectName("lblAlert2");
        lblAlert2->setGeometry(QRect(20, 85, 300, 25));
        lblAlert2->setFont(font7);
        lblAlert3 = new QLabel(frameAlerts);
        lblAlert3->setObjectName("lblAlert3");
        lblAlert3->setGeometry(QRect(20, 115, 300, 25));
        lblAlert3->setFont(font7);
        stackedWidgetEmployes->addWidget(subPageEmpl4);
        subPageEmpl5 = new QWidget();
        subPageEmpl5->setObjectName("subPageEmpl5");
        lblAdvancedMetierTitle = new QLabel(subPageEmpl5);
        lblAdvancedMetierTitle->setObjectName("lblAdvancedMetierTitle");
        lblAdvancedMetierTitle->setGeometry(QRect(70, 0, 1060, 50));
        QFont font8;
        font8.setFamilies({QString::fromUtf8("Segoe UI")});
        font8.setPointSize(22);
        font8.setBold(true);
        lblAdvancedMetierTitle->setFont(font8);
        lblAdvancedMetierTitle->setStyleSheet(QString::fromUtf8("color: #1a3a52; background-color: transparent;\n"
"color: rgb(255, 255, 255);\n"
"background-image: url(:/new/prefix1/15.jpg);"));
        lblAdvancedMetierTitle->setAlignment(Qt::AlignmentFlag::AlignCenter);
        listRoles = new QListWidget(subPageEmpl5);
        listRoles->setObjectName("listRoles");
        listRoles->setGeometry(QRect(20, 70, 250, 580));
        listRoles->setStyleSheet(QString::fromUtf8("QListWidget {\n"
"    background-color: #2c3e50;\n"
"    color: white;\n"
"    border-radius: 10px;\n"
"    padding: 5px;\n"
"}\n"
"QListWidget::item {\n"
"    height: 40px;\n"
"    border-bottom: 1px solid #34495e;\n"
"}\n"
"QListWidget::item:selected {\n"
"    background-color: #3498db;\n"
"    border-radius: 5px;\n"
"}"));
        frameRoleDetail = new QFrame(subPageEmpl5);
        frameRoleDetail->setObjectName("frameRoleDetail");
        frameRoleDetail->setGeometry(QRect(280, 70, 781, 580));
        frameRoleStatus = new QFrame(frameRoleDetail);
        frameRoleStatus->setObjectName("frameRoleStatus");
        frameRoleStatus->setGeometry(QRect(0, 0, 781, 80));
        frameRoleStatus->setStyleSheet(QString::fromUtf8("QFrame { background-color: #2c3e50; border-radius: 10px; border: 2px solid #3498db; }\n"
"QLabel { color: white; border: none; }"));
        lblRoleStatusIcon = new QLabel(frameRoleStatus);
        lblRoleStatusIcon->setObjectName("lblRoleStatusIcon");
        lblRoleStatusIcon->setGeometry(QRect(15, 15, 50, 50));
        QFont font9;
        font9.setPointSize(26);
        font9.setBold(true);
        lblRoleStatusIcon->setFont(font9);
        lblRoleStatusIcon->setAlignment(Qt::AlignmentFlag::AlignCenter);
        lblRoleStatusText = new QLabel(frameRoleStatus);
        lblRoleStatusText->setObjectName("lblRoleStatusText");
        lblRoleStatusText->setGeometry(QRect(75, 15, 680, 50));
        lblRoleStatusText->setFont(font4);
        lblAutoTitle = new QLabel(frameRoleDetail);
        lblAutoTitle->setObjectName("lblAutoTitle");
        lblAutoTitle->setGeometry(QRect(0, 90, 300, 25));
        QFont font10;
        font10.setBold(true);
        lblAutoTitle->setFont(font10);
        lblAutoTitle->setStyleSheet(QString::fromUtf8("color: rgb(0, 0, 0);"));
        tableRoleAutoSuggestions = new QTableWidget(frameRoleDetail);
        if (tableRoleAutoSuggestions->columnCount() < 6)
            tableRoleAutoSuggestions->setColumnCount(6);
        QTableWidgetItem *__qtablewidgetitem30 = new QTableWidgetItem();
        tableRoleAutoSuggestions->setHorizontalHeaderItem(0, __qtablewidgetitem30);
        QTableWidgetItem *__qtablewidgetitem31 = new QTableWidgetItem();
        tableRoleAutoSuggestions->setHorizontalHeaderItem(1, __qtablewidgetitem31);
        QTableWidgetItem *__qtablewidgetitem32 = new QTableWidgetItem();
        tableRoleAutoSuggestions->setHorizontalHeaderItem(2, __qtablewidgetitem32);
        QTableWidgetItem *__qtablewidgetitem33 = new QTableWidgetItem();
        tableRoleAutoSuggestions->setHorizontalHeaderItem(3, __qtablewidgetitem33);
        QTableWidgetItem *__qtablewidgetitem34 = new QTableWidgetItem();
        tableRoleAutoSuggestions->setHorizontalHeaderItem(4, __qtablewidgetitem34);
        QTableWidgetItem *__qtablewidgetitem35 = new QTableWidgetItem();
        tableRoleAutoSuggestions->setHorizontalHeaderItem(5, __qtablewidgetitem35);
        if (tableRoleAutoSuggestions->rowCount() < 4)
            tableRoleAutoSuggestions->setRowCount(4);
        QTableWidgetItem *__qtablewidgetitem36 = new QTableWidgetItem();
        tableRoleAutoSuggestions->setVerticalHeaderItem(0, __qtablewidgetitem36);
        QTableWidgetItem *__qtablewidgetitem37 = new QTableWidgetItem();
        tableRoleAutoSuggestions->setVerticalHeaderItem(1, __qtablewidgetitem37);
        QTableWidgetItem *__qtablewidgetitem38 = new QTableWidgetItem();
        tableRoleAutoSuggestions->setVerticalHeaderItem(2, __qtablewidgetitem38);
        QTableWidgetItem *__qtablewidgetitem39 = new QTableWidgetItem();
        tableRoleAutoSuggestions->setVerticalHeaderItem(3, __qtablewidgetitem39);
        tableRoleAutoSuggestions->setObjectName("tableRoleAutoSuggestions");
        tableRoleAutoSuggestions->setGeometry(QRect(0, 120, 781, 200));
        tableRoleAutoSuggestions->setStyleSheet(QString::fromUtf8("QTableWidget { background-color: #34495e; color: white; border-radius: 8px; }\n"
"QHeaderView::section { background-color: #2c3e50; color: white; }"));
        lblManualTitle = new QLabel(frameRoleDetail);
        lblManualTitle->setObjectName("lblManualTitle");
        lblManualTitle->setGeometry(QRect(0, 330, 300, 25));
        lblManualTitle->setFont(font10);
        lblManualTitle->setStyleSheet(QString::fromUtf8("color: rgb(0, 0, 0);"));
        tableRoleManualSelection = new QTableWidget(frameRoleDetail);
        if (tableRoleManualSelection->columnCount() < 5)
            tableRoleManualSelection->setColumnCount(5);
        QTableWidgetItem *__qtablewidgetitem40 = new QTableWidgetItem();
        tableRoleManualSelection->setHorizontalHeaderItem(0, __qtablewidgetitem40);
        QTableWidgetItem *__qtablewidgetitem41 = new QTableWidgetItem();
        tableRoleManualSelection->setHorizontalHeaderItem(1, __qtablewidgetitem41);
        QTableWidgetItem *__qtablewidgetitem42 = new QTableWidgetItem();
        tableRoleManualSelection->setHorizontalHeaderItem(2, __qtablewidgetitem42);
        QTableWidgetItem *__qtablewidgetitem43 = new QTableWidgetItem();
        tableRoleManualSelection->setHorizontalHeaderItem(3, __qtablewidgetitem43);
        QTableWidgetItem *__qtablewidgetitem44 = new QTableWidgetItem();
        tableRoleManualSelection->setHorizontalHeaderItem(4, __qtablewidgetitem44);
        if (tableRoleManualSelection->rowCount() < 4)
            tableRoleManualSelection->setRowCount(4);
        QTableWidgetItem *__qtablewidgetitem45 = new QTableWidgetItem();
        tableRoleManualSelection->setVerticalHeaderItem(0, __qtablewidgetitem45);
        QTableWidgetItem *__qtablewidgetitem46 = new QTableWidgetItem();
        tableRoleManualSelection->setVerticalHeaderItem(1, __qtablewidgetitem46);
        QTableWidgetItem *__qtablewidgetitem47 = new QTableWidgetItem();
        tableRoleManualSelection->setVerticalHeaderItem(2, __qtablewidgetitem47);
        QTableWidgetItem *__qtablewidgetitem48 = new QTableWidgetItem();
        tableRoleManualSelection->setVerticalHeaderItem(3, __qtablewidgetitem48);
        tableRoleManualSelection->setObjectName("tableRoleManualSelection");
        tableRoleManualSelection->setGeometry(QRect(0, 360, 781, 200));
        tableRoleManualSelection->setStyleSheet(QString::fromUtf8("QTableWidget { background-color: #34495e; color: white; border-radius: 8px; }\n"
"QHeaderView::section { background-color: #2c3e50; color: white; }"));
        tableWidget = new QTableWidget(subPageEmpl5);
        if (tableWidget->columnCount() < 1)
            tableWidget->setColumnCount(1);
        QTableWidgetItem *__qtablewidgetitem49 = new QTableWidgetItem();
        __qtablewidgetitem49->setTextAlignment(Qt::AlignLeading|Qt::AlignVCenter);
        __qtablewidgetitem49->setFont(font10);
        tableWidget->setHorizontalHeaderItem(0, __qtablewidgetitem49);
        if (tableWidget->rowCount() < 6)
            tableWidget->setRowCount(6);
        QTableWidgetItem *__qtablewidgetitem50 = new QTableWidgetItem();
        tableWidget->setVerticalHeaderItem(0, __qtablewidgetitem50);
        QTableWidgetItem *__qtablewidgetitem51 = new QTableWidgetItem();
        tableWidget->setVerticalHeaderItem(1, __qtablewidgetitem51);
        QTableWidgetItem *__qtablewidgetitem52 = new QTableWidgetItem();
        tableWidget->setVerticalHeaderItem(2, __qtablewidgetitem52);
        QTableWidgetItem *__qtablewidgetitem53 = new QTableWidgetItem();
        tableWidget->setVerticalHeaderItem(3, __qtablewidgetitem53);
        QTableWidgetItem *__qtablewidgetitem54 = new QTableWidgetItem();
        tableWidget->setVerticalHeaderItem(4, __qtablewidgetitem54);
        QTableWidgetItem *__qtablewidgetitem55 = new QTableWidgetItem();
        tableWidget->setVerticalHeaderItem(5, __qtablewidgetitem55);
        QTableWidgetItem *__qtablewidgetitem56 = new QTableWidgetItem();
        __qtablewidgetitem56->setTextAlignment(Qt::AlignCenter);
        tableWidget->setItem(0, 0, __qtablewidgetitem56);
        QTableWidgetItem *__qtablewidgetitem57 = new QTableWidgetItem();
        __qtablewidgetitem57->setTextAlignment(Qt::AlignCenter);
        tableWidget->setItem(1, 0, __qtablewidgetitem57);
        QTableWidgetItem *__qtablewidgetitem58 = new QTableWidgetItem();
        __qtablewidgetitem58->setTextAlignment(Qt::AlignCenter);
        tableWidget->setItem(2, 0, __qtablewidgetitem58);
        QTableWidgetItem *__qtablewidgetitem59 = new QTableWidgetItem();
        __qtablewidgetitem59->setTextAlignment(Qt::AlignCenter);
        tableWidget->setItem(3, 0, __qtablewidgetitem59);
        QTableWidgetItem *__qtablewidgetitem60 = new QTableWidgetItem();
        __qtablewidgetitem60->setTextAlignment(Qt::AlignCenter);
        tableWidget->setItem(4, 0, __qtablewidgetitem60);
        QTableWidgetItem *__qtablewidgetitem61 = new QTableWidgetItem();
        __qtablewidgetitem61->setTextAlignment(Qt::AlignCenter);
        tableWidget->setItem(5, 0, __qtablewidgetitem61);
        tableWidget->setObjectName("tableWidget");
        tableWidget->setGeometry(QRect(40, 90, 211, 541));
        stackedWidgetEmployes->addWidget(subPageEmpl5);

        verticalLayout_2->addWidget(stackedWidgetEmployes);

        stackedWidget->addWidget(pageEmployes);
        pagePecheurs = new QWidget();
        pagePecheurs->setObjectName("pagePecheurs");
        verticalLayout_Pecheurs = new QVBoxLayout(pagePecheurs);
        verticalLayout_Pecheurs->setSpacing(0);
        verticalLayout_Pecheurs->setObjectName("verticalLayout_Pecheurs");
        verticalLayout_Pecheurs->setContentsMargins(0, 0, 0, 0);
        frame_Pecheurs_Header = new QFrame(pagePecheurs);
        frame_Pecheurs_Header->setObjectName("frame_Pecheurs_Header");
        frame_Pecheurs_Header->setMinimumSize(QSize(0, 120));
        frame_Pecheurs_Header->setMaximumSize(QSize(16777215, 120));
        frame_Pecheurs_Header->setStyleSheet(QString::fromUtf8("#frame_Pecheurs_Header {\n"
"    border-image: url(:/new/prefix1/img1.jpg);\n"
"}\n"
"#frame_Pecheurs_Header_Overlay {\n"
"    background-color: rgba(0, 0, 0, 0.55);\n"
"}"));
        frame_Pecheurs_Header->setFrameShape(QFrame::Shape::NoFrame);
        verticalLayout_Header = new QVBoxLayout(frame_Pecheurs_Header);
        verticalLayout_Header->setSpacing(0);
        verticalLayout_Header->setObjectName("verticalLayout_Header");
        verticalLayout_Header->setContentsMargins(0, 0, 0, 0);
        frame_Pecheurs_Header_Overlay = new QFrame(frame_Pecheurs_Header);
        frame_Pecheurs_Header_Overlay->setObjectName("frame_Pecheurs_Header_Overlay");
        verticalLayout_Title = new QVBoxLayout(frame_Pecheurs_Header_Overlay);
        verticalLayout_Title->setObjectName("verticalLayout_Title");
        title_label_pecheurs = new QLabel(frame_Pecheurs_Header_Overlay);
        title_label_pecheurs->setObjectName("title_label_pecheurs");
        title_label_pecheurs->setFont(font1);
        title_label_pecheurs->setStyleSheet(QString::fromUtf8("color: white; font-size: 28px; font-weight: bold;"));
        title_label_pecheurs->setAlignment(Qt::AlignmentFlag::AlignCenter);

        verticalLayout_Title->addWidget(title_label_pecheurs);


        verticalLayout_Header->addWidget(frame_Pecheurs_Header_Overlay);


        verticalLayout_Pecheurs->addWidget(frame_Pecheurs_Header);

        subNavBarPecheurs = new QFrame(pagePecheurs);
        subNavBarPecheurs->setObjectName("subNavBarPecheurs");
        subNavBarPecheurs->setMinimumSize(QSize(0, 50));
        horizontalLayout_SubNavPech = new QHBoxLayout(subNavBarPecheurs);
        horizontalLayout_SubNavPech->setObjectName("horizontalLayout_SubNavPech");
        btnSubPech1 = new QPushButton(subNavBarPecheurs);
        btnSubPech1->setObjectName("btnSubPech1");
        btnSubPech1->setCheckable(true);
        btnSubPech1->setChecked(true);
        btnSubPech1->setAutoExclusive(true);

        horizontalLayout_SubNavPech->addWidget(btnSubPech1);

        btnSubPech2 = new QPushButton(subNavBarPecheurs);
        btnSubPech2->setObjectName("btnSubPech2");
        btnSubPech2->setCheckable(true);
        btnSubPech2->setAutoExclusive(true);

        horizontalLayout_SubNavPech->addWidget(btnSubPech2);

        btnSubPech3 = new QPushButton(subNavBarPecheurs);
        btnSubPech3->setObjectName("btnSubPech3");
        btnSubPech3->setCheckable(true);
        btnSubPech3->setAutoExclusive(true);

        horizontalLayout_SubNavPech->addWidget(btnSubPech3);

        spacerSubNav = new QSpacerItem(0, 0, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);

        horizontalLayout_SubNavPech->addItem(spacerSubNav);


        verticalLayout_Pecheurs->addWidget(subNavBarPecheurs);

        stackedWidgetPecheurs = new QStackedWidget(pagePecheurs);
        stackedWidgetPecheurs->setObjectName("stackedWidgetPecheurs");
        subPagePech1 = new QWidget();
        subPagePech1->setObjectName("subPagePech1");
        verticalLayout_sub1 = new QVBoxLayout(subPagePech1);
        verticalLayout_sub1->setObjectName("verticalLayout_sub1");
        groupBox_pecheur_2 = new QGroupBox(subPagePech1);
        groupBox_pecheur_2->setObjectName("groupBox_pecheur_2");
        groupBox_pecheur_2->setStyleSheet(QString::fromUtf8("QGroupBox {\n"
"    background-color: #232323;\n"
"    border-radius: 8px;\n"
"    border: 2px solid #3498db;\n"
"    margin-top: 20px;\n"
"    color: white;\n"
"    font-weight: bold;\n"
"}\n"
"QGroupBox::title {\n"
"    subcontrol-origin: margin;\n"
"    subcontrol-position: top center;\n"
"    padding: 0 3px;\n"
"    color: #3498db;\n"
"}\n"
"QLabel {\n"
"    color: #ffffff;\n"
"    font-family: \"Segoe UI\";\n"
"    font-weight: bold;\n"
"    font-size: 12px;\n"
"    padding: 2px;\n"
"    border: none;\n"
"    background: transparent;\n"
"}\n"
"QLineEdit, QComboBox, QSpinBox {\n"
"    color: white;\n"
"    background-color: #2d2d2d;\n"
"    border: 1px solid #3d3d3d;\n"
"    padding: 5px;\n"
"    border-radius: 4px;\n"
"    font-family: \"Segoe UI\";\n"
"    font-weight: bold;\n"
"}"));
        gridLayout_form_2 = new QGridLayout(groupBox_pecheur_2);
        gridLayout_form_2->setObjectName("gridLayout_form_2");
        l2_2 = new QLabel(groupBox_pecheur_2);
        l2_2->setObjectName("l2_2");

        gridLayout_form_2->addWidget(l2_2, 0, 0, 1, 1);

        le_nom_2 = new QLineEdit(groupBox_pecheur_2);
        le_nom_2->setObjectName("le_nom_2");

        gridLayout_form_2->addWidget(le_nom_2, 0, 1, 1, 1);

        l3_2 = new QLabel(groupBox_pecheur_2);
        l3_2->setObjectName("l3_2");

        gridLayout_form_2->addWidget(l3_2, 0, 2, 1, 1);

        le_prenom_2 = new QLineEdit(groupBox_pecheur_2);
        le_prenom_2->setObjectName("le_prenom_2");

        gridLayout_form_2->addWidget(le_prenom_2, 0, 3, 1, 1);

        l5_2 = new QLabel(groupBox_pecheur_2);
        l5_2->setObjectName("l5_2");

        gridLayout_form_2->addWidget(l5_2, 1, 0, 1, 1);

        cb_role_2 = new QComboBox(groupBox_pecheur_2);
        cb_role_2->addItem(QString());
        cb_role_2->addItem(QString());
        cb_role_2->setObjectName("cb_role_2");

        gridLayout_form_2->addWidget(cb_role_2, 1, 1, 1, 1);

        l7_2 = new QLabel(groupBox_pecheur_2);
        l7_2->setObjectName("l7_2");

        gridLayout_form_2->addWidget(l7_2, 1, 2, 1, 1);

        widget_status_2 = new QWidget(groupBox_pecheur_2);
        widget_status_2->setObjectName("widget_status_2");
        horizontalLayout_status_2 = new QHBoxLayout(widget_status_2);
        horizontalLayout_status_2->setObjectName("horizontalLayout_status_2");
        radio_status_actif_2 = new QRadioButton(widget_status_2);
        radio_status_actif_2->setObjectName("radio_status_actif_2");
        radio_status_actif_2->setChecked(true);

        horizontalLayout_status_2->addWidget(radio_status_actif_2);

        radio_status_inactif_2 = new QRadioButton(widget_status_2);
        radio_status_inactif_2->setObjectName("radio_status_inactif_2");

        horizontalLayout_status_2->addWidget(radio_status_inactif_2);


        gridLayout_form_2->addWidget(widget_status_2, 1, 3, 1, 1);

        l6_2 = new QLabel(groupBox_pecheur_2);
        l6_2->setObjectName("l6_2");

        gridLayout_form_2->addWidget(l6_2, 2, 0, 1, 1);

        sb_experience_2 = new QSpinBox(groupBox_pecheur_2);
        sb_experience_2->setObjectName("sb_experience_2");

        gridLayout_form_2->addWidget(sb_experience_2, 2, 1, 1, 1);

        l_date_limit_2 = new QLabel(groupBox_pecheur_2);
        l_date_limit_2->setObjectName("l_date_limit_2");

        gridLayout_form_2->addWidget(l_date_limit_2, 2, 2, 1, 1);

        dateEdit_limit_2 = new QDateEdit(groupBox_pecheur_2);
        dateEdit_limit_2->setObjectName("dateEdit_limit_2");

        gridLayout_form_2->addWidget(dateEdit_limit_2, 2, 3, 1, 1);

        l_contact_2 = new QLabel(groupBox_pecheur_2);
        l_contact_2->setObjectName("l_contact_2");

        gridLayout_form_2->addWidget(l_contact_2, 3, 0, 1, 1);

        widget_contact_2 = new QWidget(groupBox_pecheur_2);
        widget_contact_2->setObjectName("widget_contact_2");
        horizontalLayout_contact_2 = new QHBoxLayout(widget_contact_2);
        horizontalLayout_contact_2->setSpacing(5);
        horizontalLayout_contact_2->setObjectName("horizontalLayout_contact_2");
        l_prefix_216_2 = new QLabel(widget_contact_2);
        l_prefix_216_2->setObjectName("l_prefix_216_2");

        horizontalLayout_contact_2->addWidget(l_prefix_216_2);

        le_telephone_2 = new QLineEdit(widget_contact_2);
        le_telephone_2->setObjectName("le_telephone_2");

        horizontalLayout_contact_2->addWidget(le_telephone_2);


        gridLayout_form_2->addWidget(widget_contact_2, 3, 1, 1, 3);


        verticalLayout_sub1->addWidget(groupBox_pecheur_2);

        framePecheurToolbar = new QFrame(subPagePech1);
        framePecheurToolbar->setObjectName("framePecheurToolbar");
        framePecheurToolbar->setMinimumSize(QSize(0, 50));
        framePecheurToolbar->setStyleSheet(QString::fromUtf8("QFrame#framePecheurToolbar { background-color: #34495e; border-radius: 8px; padding: 8px; border: 1px solid #2c3e50; }"));
        verticalLayout_pecheur_toolbar = new QVBoxLayout(framePecheurToolbar);
        verticalLayout_pecheur_toolbar->setObjectName("verticalLayout_pecheur_toolbar");
        h_actions_2 = new QHBoxLayout();
        h_actions_2->setObjectName("h_actions_2");
        btn_add_2 = new QPushButton(framePecheurToolbar);
        btn_add_2->setObjectName("btn_add_2");
        btn_add_2->setFont(font1);
        btn_add_2->setStyleSheet(QString::fromUtf8("QPushButton { border: 1px solid #bdc3c7;\n"
"	background-color: #3498db;\n"
"	color: black;\n"
"	font-family: \"Segoe UI\";\n"
"	font-size: 14px;\n"
"	font-weight: bold;\n"
"	border-radius: 5px;\n"
"	padding: 8px 15px;\n"
"	border: none;\n"
"}\n"
"QPushButton:hover {\n"
"	background-color: #2980b9;\n"
"}\n"
"QPushButton:pressed {\n"
"	background-color: #21618c;\n"
"}"));

        h_actions_2->addWidget(btn_add_2);

        btn_edit_2 = new QPushButton(framePecheurToolbar);
        btn_edit_2->setObjectName("btn_edit_2");
        btn_edit_2->setFont(font1);
        btn_edit_2->setStyleSheet(QString::fromUtf8("QPushButton { border: 1px solid #bdc3c7;\n"
"	background-color: #3498db;\n"
"	color: black;\n"
"	font-family: \"Segoe UI\";\n"
"	font-size: 14px;\n"
"	font-weight: bold;\n"
"	border-radius: 5px;\n"
"	padding: 8px 15px;\n"
"	border: none;\n"
"}\n"
"QPushButton:hover {\n"
"	background-color: #2980b9;\n"
"}\n"
"QPushButton:pressed {\n"
"	background-color: #21618c;\n"
"}"));

        h_actions_2->addWidget(btn_edit_2);

        btn_delete_2 = new QPushButton(framePecheurToolbar);
        btn_delete_2->setObjectName("btn_delete_2");
        btn_delete_2->setFont(font1);
        btn_delete_2->setStyleSheet(QString::fromUtf8("QPushButton { border: 1px solid #bdc3c7;\n"
"	background-color: #e74c3c;\n"
"	color: white;\n"
"	font-family: \"Segoe UI\";\n"
"	font-size: 14px;\n"
"	font-weight: bold;\n"
"	border-radius: 5px;\n"
"	padding: 8px 15px;\n"
"	border: none;\n"
"}\n"
"QPushButton:hover {\n"
"	background-color: #c0392b;\n"
"}\n"
"QPushButton:pressed {\n"
"	background-color: #a93226;\n"
"}"));

        h_actions_2->addWidget(btn_delete_2);

        s1_2 = new QSpacerItem(40, 20, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);

        h_actions_2->addItem(s1_2);

        btn_export_2 = new QPushButton(framePecheurToolbar);
        btn_export_2->setObjectName("btn_export_2");
        btn_export_2->setStyleSheet(QString::fromUtf8("QPushButton { background-color: #f1c40f; color: black; font-weight: bold; border-radius: 4px; padding: 5px; } QPushButton:hover { background-color: #f39c12; }"));

        h_actions_2->addWidget(btn_export_2);


        verticalLayout_pecheur_toolbar->addLayout(h_actions_2);

        h_search_sort_2 = new QHBoxLayout();
        h_search_sort_2->setObjectName("h_search_sort_2");
        le_search_2 = new QLineEdit(framePecheurToolbar);
        le_search_2->setObjectName("le_search_2");
        le_search_2->setMinimumSize(QSize(200, 0));
        le_search_2->setStyleSheet(QString::fromUtf8("QLineEdit { color: white; background-color: #2d2d2d; border: 1px solid #3498db; border-radius: 4px; padding: 5px; }"));

        h_search_sort_2->addWidget(le_search_2);

        btn_recherche_2 = new QPushButton(framePecheurToolbar);
        btn_recherche_2->setObjectName("btn_recherche_2");
        btn_recherche_2->setStyleSheet(QString::fromUtf8("QPushButton { border: 1px solid #bdc3c7;\n"
"	background-color: #3498db;\n"
"	color: black;\n"
"	font-family: \"Segoe UI\";\n"
"	font-size: 14px;\n"
"	font-weight: bold;\n"
"	border-radius: 5px;\n"
"	padding: 8px 15px;\n"
"	border: none;\n"
"}\n"
"QPushButton:hover {\n"
"	background-color: #2980b9;\n"
"}\n"
"QPushButton:pressed {\n"
"	background-color: #21618c;\n"
"}"));

        h_search_sort_2->addWidget(btn_recherche_2);

        s2_2 = new QSpacerItem(40, 20, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);

        h_search_sort_2->addItem(s2_2);

        lsort_2 = new QLabel(framePecheurToolbar);
        lsort_2->setObjectName("lsort_2");
        lsort_2->setStyleSheet(QString::fromUtf8("color: #1a3a52; font-weight: bold; background: transparent;"));

        h_search_sort_2->addWidget(lsort_2);

        cb_sort_2 = new QComboBox(framePecheurToolbar);
        cb_sort_2->addItem(QString());
        cb_sort_2->addItem(QString());
        cb_sort_2->addItem(QString());
        cb_sort_2->addItem(QString());
        cb_sort_2->setObjectName("cb_sort_2");

        h_search_sort_2->addWidget(cb_sort_2);


        verticalLayout_pecheur_toolbar->addLayout(h_search_sort_2);


        verticalLayout_sub1->addWidget(framePecheurToolbar);

        tableWidget_4 = new QTableWidget(subPagePech1);
        if (tableWidget_4->columnCount() < 8)
            tableWidget_4->setColumnCount(8);
        QTableWidgetItem *__qtablewidgetitem62 = new QTableWidgetItem();
        tableWidget_4->setHorizontalHeaderItem(0, __qtablewidgetitem62);
        QTableWidgetItem *__qtablewidgetitem63 = new QTableWidgetItem();
        tableWidget_4->setHorizontalHeaderItem(1, __qtablewidgetitem63);
        QTableWidgetItem *__qtablewidgetitem64 = new QTableWidgetItem();
        tableWidget_4->setHorizontalHeaderItem(2, __qtablewidgetitem64);
        QTableWidgetItem *__qtablewidgetitem65 = new QTableWidgetItem();
        tableWidget_4->setHorizontalHeaderItem(3, __qtablewidgetitem65);
        QTableWidgetItem *__qtablewidgetitem66 = new QTableWidgetItem();
        tableWidget_4->setHorizontalHeaderItem(4, __qtablewidgetitem66);
        QTableWidgetItem *__qtablewidgetitem67 = new QTableWidgetItem();
        tableWidget_4->setHorizontalHeaderItem(5, __qtablewidgetitem67);
        QTableWidgetItem *__qtablewidgetitem68 = new QTableWidgetItem();
        tableWidget_4->setHorizontalHeaderItem(6, __qtablewidgetitem68);
        QTableWidgetItem *__qtablewidgetitem69 = new QTableWidgetItem();
        tableWidget_4->setHorizontalHeaderItem(7, __qtablewidgetitem69);
        if (tableWidget_4->rowCount() < 3)
            tableWidget_4->setRowCount(3);
        QTableWidgetItem *__qtablewidgetitem70 = new QTableWidgetItem();
        tableWidget_4->setVerticalHeaderItem(0, __qtablewidgetitem70);
        QTableWidgetItem *__qtablewidgetitem71 = new QTableWidgetItem();
        tableWidget_4->setVerticalHeaderItem(1, __qtablewidgetitem71);
        QTableWidgetItem *__qtablewidgetitem72 = new QTableWidgetItem();
        tableWidget_4->setVerticalHeaderItem(2, __qtablewidgetitem72);
        tableWidget_4->setObjectName("tableWidget_4");
        tableWidget_4->setStyleSheet(QString::fromUtf8("QTableWidget {\n"
"	background-color: #2d2d2d;\n"
"	gridline-color: #3d3d3d;\n"
"	font-family: \"Segoe UI\";\n"
"	font-size: 12px;\n"
"    font-weight: bold;\n"
"	selection-background-color: #3498db;\n"
"    border-radius: 8px;\n"
"    border: 2px solid #3498db;\n"
"    color: white;\n"
"}\n"
"QHeaderView::section {\n"
"	background-color: #3d3d3d;\n"
"	color: white;\n"
"	padding: 6px;\n"
"	border: 1px solid #232323;\n"
"	font-weight: bold;\n"
"    font-family: \"Segoe UI\";\n"
"}"));
        tableWidget_4->setEditTriggers(QAbstractItemView::EditTrigger::NoEditTriggers);
        tableWidget_4->horizontalHeader()->setStretchLastSection(true);

        verticalLayout_sub1->addWidget(tableWidget_4);

        stackedWidgetPecheurs->addWidget(subPagePech1);
        subPagePech2 = new QWidget();
        subPagePech2->setObjectName("subPagePech2");
        verticalLayout_sub2 = new QVBoxLayout(subPagePech2);
        verticalLayout_sub2->setObjectName("verticalLayout_sub2");
        h_charts_2 = new QHBoxLayout();
        h_charts_2->setObjectName("h_charts_2");
        frame_chart_role_2 = new QFrame(subPagePech2);
        frame_chart_role_2->setObjectName("frame_chart_role_2");
        frame_chart_role_2->setMinimumSize(QSize(350, 280));
        frame_chart_role_2->setStyleSheet(QString::fromUtf8("border: 2px solid #3498db; border-radius: 8px; background: rgba(40, 44, 52, 0.5);"));
        frame_chart_role_2->setFrameShape(QFrame::Shape::StyledPanel);
        v_chart_role_2 = new QVBoxLayout(frame_chart_role_2);
        v_chart_role_2->setObjectName("v_chart_role_2");
        label_role_chart_title_2 = new QLabel(frame_chart_role_2);
        label_role_chart_title_2->setObjectName("label_role_chart_title_2");
        label_role_chart_title_2->setStyleSheet(QString::fromUtf8("color: #ffffff; font-family: \"Segoe UI\"; font-weight: bold; font-size: 14px;"));
        label_role_chart_title_2->setAlignment(Qt::AlignmentFlag::AlignCenter);

        v_chart_role_2->addWidget(label_role_chart_title_2);

        chartViewRole_2 = new QWidget(frame_chart_role_2);
        chartViewRole_2->setObjectName("chartViewRole_2");
        chartViewRole_2->setMinimumSize(QSize(220, 220));

        v_chart_role_2->addWidget(chartViewRole_2);


        h_charts_2->addWidget(frame_chart_role_2);

        frame_chart_status_2 = new QFrame(subPagePech2);
        frame_chart_status_2->setObjectName("frame_chart_status_2");
        frame_chart_status_2->setMinimumSize(QSize(350, 280));
        frame_chart_status_2->setStyleSheet(QString::fromUtf8("border: 2px solid #3498db; border-radius: 8px; background: rgba(40, 44, 52, 0.5);"));
        frame_chart_status_2->setFrameShape(QFrame::Shape::StyledPanel);
        v_chart_status_2 = new QVBoxLayout(frame_chart_status_2);
        v_chart_status_2->setObjectName("v_chart_status_2");
        label_status_chart_title_2 = new QLabel(frame_chart_status_2);
        label_status_chart_title_2->setObjectName("label_status_chart_title_2");
        label_status_chart_title_2->setStyleSheet(QString::fromUtf8("color: #ffffff; font-family: \"Segoe UI\"; font-weight: bold; font-size: 14px;"));
        label_status_chart_title_2->setAlignment(Qt::AlignmentFlag::AlignCenter);

        v_chart_status_2->addWidget(label_status_chart_title_2);

        chartViewStatus_2 = new QWidget(frame_chart_status_2);
        chartViewStatus_2->setObjectName("chartViewStatus_2");
        chartViewStatus_2->setMinimumSize(QSize(220, 220));

        v_chart_status_2->addWidget(chartViewStatus_2);


        h_charts_2->addWidget(frame_chart_status_2);


        verticalLayout_sub2->addLayout(h_charts_2);

        stackedWidgetPecheurs->addWidget(subPagePech2);
        subPagePech3 = new QWidget();
        subPagePech3->setObjectName("subPagePech3");
        verticalLayout_sub3 = new QVBoxLayout(subPagePech3);
        verticalLayout_sub3->setObjectName("verticalLayout_sub3");
        gb_mission_setup_2 = new QGroupBox(subPagePech3);
        gb_mission_setup_2->setObjectName("gb_mission_setup_2");
        gb_mission_setup_2->setStyleSheet(QString::fromUtf8("QGroupBox { border: 2px solid #57a5b3; border-radius: 8px; margin-top: 24px; color: #1a3a52; font-weight: bold; background-color: #232323; } QGroupBox::title { subcontrol-origin: margin; subcontrol-position: top center; color: #ffffff; font-family: \"Segoe UI\"; font-weight: bold; font-size: 12px; } QLabel { color: #ffffff; font-family: \"Segoe UI\"; font-weight: bold; font-size: 12px; padding: 2px; border: none; }"));
        grid_mission_setup_2 = new QGridLayout(gb_mission_setup_2);
        grid_mission_setup_2->setObjectName("grid_mission_setup_2");
        lbl_mission_type_2 = new QLabel(gb_mission_setup_2);
        lbl_mission_type_2->setObjectName("lbl_mission_type_2");

        grid_mission_setup_2->addWidget(lbl_mission_type_2, 0, 0, 1, 1);

        cb_mission_type_2 = new QComboBox(gb_mission_setup_2);
        cb_mission_type_2->addItem(QString());
        cb_mission_type_2->addItem(QString());
        cb_mission_type_2->addItem(QString());
        cb_mission_type_2->addItem(QString());
        cb_mission_type_2->setObjectName("cb_mission_type_2");

        grid_mission_setup_2->addWidget(cb_mission_type_2, 0, 1, 1, 1);

        lbl_duration_2 = new QLabel(gb_mission_setup_2);
        lbl_duration_2->setObjectName("lbl_duration_2");

        grid_mission_setup_2->addWidget(lbl_duration_2, 0, 2, 1, 1);

        sb_duration_2 = new QSpinBox(gb_mission_setup_2);
        sb_duration_2->setObjectName("sb_duration_2");
        sb_duration_2->setMinimum(1);
        sb_duration_2->setMaximum(30);
        sb_duration_2->setValue(3);

        grid_mission_setup_2->addWidget(sb_duration_2, 0, 3, 1, 1);

        lbl_difficulty_2 = new QLabel(gb_mission_setup_2);
        lbl_difficulty_2->setObjectName("lbl_difficulty_2");

        grid_mission_setup_2->addWidget(lbl_difficulty_2, 1, 0, 1, 1);

        cb_difficulty_2 = new QComboBox(gb_mission_setup_2);
        cb_difficulty_2->addItem(QString());
        cb_difficulty_2->addItem(QString());
        cb_difficulty_2->addItem(QString());
        cb_difficulty_2->setObjectName("cb_difficulty_2");

        grid_mission_setup_2->addWidget(cb_difficulty_2, 1, 1, 1, 1);

        btn_analyze_mission_2 = new QPushButton(gb_mission_setup_2);
        btn_analyze_mission_2->setObjectName("btn_analyze_mission_2");
        btn_analyze_mission_2->setMinimumSize(QSize(114, 40));
        btn_analyze_mission_2->setCursor(QCursor(Qt::CursorShape::PointingHandCursor));
        btn_analyze_mission_2->setStyleSheet(QString::fromUtf8("QPushButton { background-color: #1a3a52; color: #14ffec; border-radius: 8px; font-weight: bold; } QPushButton:hover { background-color: #2c5f7d; }"));

        grid_mission_setup_2->addWidget(btn_analyze_mission_2, 1, 2, 1, 2);


        verticalLayout_sub3->addWidget(gb_mission_setup_2);

        gb_ai_results_2 = new QGroupBox(subPagePech3);
        gb_ai_results_2->setObjectName("gb_ai_results_2");
        gb_ai_results_2->setStyleSheet(QString::fromUtf8("QGroupBox { border: 2px solid #57a5b3; border-radius: 8px; margin-top: 24px; color: #1a3a52; background-color: #232323; font-weight: bold; } QGroupBox::title { color: #ffffff; font-family: \"Segoe UI\"; font-weight: bold; font-size: 12px; }"));
        v_ai_results_2 = new QVBoxLayout(gb_ai_results_2);
        v_ai_results_2->setObjectName("v_ai_results_2");
        lbl_ai_status_2 = new QLabel(gb_ai_results_2);
        lbl_ai_status_2->setObjectName("lbl_ai_status_2");
        lbl_ai_status_2->setStyleSheet(QString::fromUtf8("color: #ffffff; font-family: \"Segoe UI\"; font-weight: bold; font-size: 12px; padding: 2px; border: none;"));
        lbl_ai_status_2->setAlignment(Qt::AlignmentFlag::AlignCenter);

        v_ai_results_2->addWidget(lbl_ai_status_2);

        table_mission_results_2 = new QTableWidget(gb_ai_results_2);
        if (table_mission_results_2->columnCount() < 4)
            table_mission_results_2->setColumnCount(4);
        QTableWidgetItem *__qtablewidgetitem73 = new QTableWidgetItem();
        table_mission_results_2->setHorizontalHeaderItem(0, __qtablewidgetitem73);
        QTableWidgetItem *__qtablewidgetitem74 = new QTableWidgetItem();
        table_mission_results_2->setHorizontalHeaderItem(1, __qtablewidgetitem74);
        QTableWidgetItem *__qtablewidgetitem75 = new QTableWidgetItem();
        table_mission_results_2->setHorizontalHeaderItem(2, __qtablewidgetitem75);
        QTableWidgetItem *__qtablewidgetitem76 = new QTableWidgetItem();
        table_mission_results_2->setHorizontalHeaderItem(3, __qtablewidgetitem76);
        table_mission_results_2->setObjectName("table_mission_results_2");
        table_mission_results_2->setStyleSheet(QString::fromUtf8("QTableWidget {\n"
"	background-color: #2d2d2d;\n"
"	gridline-color: #3d3d3d;\n"
"	font-family: \"Segoe UI\";\n"
"	font-size: 12px;\n"
"	color: #ffffff;\n"
"	border-radius: 8px;\n"
"	border: 2px solid #57a5b3;\n"
"}\n"
"QTableWidget::item {\n"
"	color: #ffffff;\n"
"	padding: 6px;\n"
"}\n"
"QHeaderView::section {\n"
"	background-color: #3d3d3d;\n"
"	color: #ffffff;\n"
"	padding: 8px;\n"
"	border: 1px solid #232323;\n"
"	font-weight: bold;\n"
"	font-family: \"Segoe UI\";}"));
        table_mission_results_2->setEditTriggers(QAbstractItemView::EditTrigger::NoEditTriggers);
        table_mission_results_2->setAlternatingRowColors(false);
        table_mission_results_2->setSelectionMode(QAbstractItemView::SelectionMode::NoSelection);
        table_mission_results_2->horizontalHeader()->setStretchLastSection(true);

        v_ai_results_2->addWidget(table_mission_results_2);


        verticalLayout_sub3->addWidget(gb_ai_results_2);

        stackedWidgetPecheurs->addWidget(subPagePech3);

        verticalLayout_Pecheurs->addWidget(stackedWidgetPecheurs);

        stackedWidget->addWidget(pagePecheurs);
        pageBateaux = new QWidget();
        pageBateaux->setObjectName("pageBateaux");
        mainContentArea = new QWidget(pageBateaux);
        mainContentArea->setObjectName("mainContentArea");
        mainContentArea->setGeometry(QRect(40, 0, 1180, 825));
        verticalLayout_MainContent_2 = new QVBoxLayout(mainContentArea);
        verticalLayout_MainContent_2->setSpacing(20);
        verticalLayout_MainContent_2->setObjectName("verticalLayout_MainContent_2");
        verticalLayout_MainContent_2->setContentsMargins(20, 35, 20, 20);
        frame_Bateaux_Header = new QFrame(mainContentArea);
        frame_Bateaux_Header->setObjectName("frame_Bateaux_Header");
        frame_Bateaux_Header->setMinimumSize(QSize(0, 120));
        frame_Bateaux_Header->setMaximumSize(QSize(16777215, 120));
        frame_Bateaux_Header->setStyleSheet(QString::fromUtf8("#frame_Bateaux_Header {\n"
"     border-image: url(:/new/prefix1/img1.jpg);\n"
"}\n"
"#frame_Bateaux_Header_Overlay {\n"
"     background-color: rgba(0, 0, 0, 0.55);\n"
"}"));
        frame_Bateaux_Header->setFrameShape(QFrame::Shape::NoFrame);
        verticalLayout_Header_Bateaux = new QVBoxLayout(frame_Bateaux_Header);
        verticalLayout_Header_Bateaux->setSpacing(0);
        verticalLayout_Header_Bateaux->setObjectName("verticalLayout_Header_Bateaux");
        verticalLayout_Header_Bateaux->setContentsMargins(0, 0, 0, 0);
        frame_Bateaux_Header_Overlay = new QFrame(frame_Bateaux_Header);
        frame_Bateaux_Header_Overlay->setObjectName("frame_Bateaux_Header_Overlay");
        verticalLayout_Title_Bateaux = new QVBoxLayout(frame_Bateaux_Header_Overlay);
        verticalLayout_Title_Bateaux->setObjectName("verticalLayout_Title_Bateaux");
        title_label_bateaux = new QLabel(frame_Bateaux_Header_Overlay);
        title_label_bateaux->setObjectName("title_label_bateaux");
        title_label_bateaux->setStyleSheet(QString::fromUtf8("color: white; font-size: 28px; font-weight: bold; font-family: \"Script MT Bold\";"));
        title_label_bateaux->setAlignment(Qt::AlignmentFlag::AlignCenter);

        verticalLayout_Title_Bateaux->addWidget(title_label_bateaux);


        verticalLayout_Header_Bateaux->addWidget(frame_Bateaux_Header_Overlay);


        verticalLayout_MainContent_2->addWidget(frame_Bateaux_Header);

        tabWidget_2 = new QTabWidget(mainContentArea);
        tabWidget_2->setObjectName("tabWidget_2");
        tabWidget_2->setStyleSheet(QString::fromUtf8("QTabWidget::pane {\n"
"    border: 1px solid #dcdde1;\n"
"    border-radius: 10px;\n"
"    background: #ffffff;\n"
"    top: -1px;\n"
"}\n"
"QTabBar::tab {\n"
"    background-color: #2a9d8f;\n"
"    color: #ffffff;\n"
"    padding: 8px 15px;\n"
"    margin: 5px;\n"
"    min-width: 120px;\n"
"    border-radius: 5px;\n"
"    font-weight: bold;\n"
"}\n"
"QTabBar::tab:hover {\n"
"    background-color: #21867a;\n"
"}\n"
"QTabBar::tab:selected {\n"
"    background-color: #1e5f74;\n"
"    font-weight: bold;\n"
"}"));
        tabGestion_8 = new QWidget();
        tabGestion_8->setObjectName("tabGestion_8");
        tabGestion_8->setStyleSheet(QString::fromUtf8("QWidget#tabGestion_8 { background-color: #ffffff; }\n"
"QTableView { background-color: #2b2b2b; color: white; gridline-color: #555; border: 1px solid #dcdde1; }\n"
"QHeaderView::section { background-color: #34495e; color: white; padding: 5px; border: none; font-weight: bold; }"));
        verticalLayout_Gestion_2 = new QVBoxLayout(tabGestion_8);
        verticalLayout_Gestion_2->setObjectName("verticalLayout_Gestion_2");
        groupForm_15 = new QGroupBox(tabGestion_8);
        groupForm_15->setObjectName("groupForm_15");
        groupForm_15->setStyleSheet(QString::fromUtf8("QGroupBox { font-weight: bold; border: 1px solid #dcdde1; border-radius: 6px; background: #232323; padding-top: 35px; margin-top: 20px; color: #ffffff; } QGroupBox::title { subcontrol-origin: margin; left: 16px; padding: 0 8px; color: #ffffff; }\n"
"QLabel { color: #ffffff; }\n"
"QLineEdit, QComboBox, QDateEdit { background-color: #2d2d2d; color: white; border: 1px solid #3d3d3d; padding: 5px; border-radius: 4px; }"));
        gridLayout_Form_2 = new QGridLayout(groupForm_15);
        gridLayout_Form_2->setObjectName("gridLayout_Form_2");
        label_id_16 = new QLabel(groupForm_15);
        label_id_16->setObjectName("label_id_16");

        gridLayout_Form_2->addWidget(label_id_16, 0, 0, 1, 1);

        lineID_15 = new QLineEdit(groupForm_15);
        lineID_15->setObjectName("lineID_15");
        lineID_15->setMinimumSize(QSize(0, 40));

        gridLayout_Form_2->addWidget(lineID_15, 0, 1, 1, 1);

        label_nom_15 = new QLabel(groupForm_15);
        label_nom_15->setObjectName("label_nom_15");

        gridLayout_Form_2->addWidget(label_nom_15, 0, 2, 1, 1);

        lineLicence_15 = new QLineEdit(groupForm_15);
        lineLicence_15->setObjectName("lineLicence_15");
        lineLicence_15->setMinimumSize(QSize(0, 40));

        gridLayout_Form_2->addWidget(lineLicence_15, 0, 3, 1, 1);

        label_exp_51 = new QLabel(groupForm_15);
        label_exp_51->setObjectName("label_exp_51");

        gridLayout_Form_2->addWidget(label_exp_51, 1, 0, 1, 1);

        linePrenom_36 = new QLineEdit(groupForm_15);
        linePrenom_36->setObjectName("linePrenom_36");
        linePrenom_36->setMinimumSize(QSize(0, 40));

        gridLayout_Form_2->addWidget(linePrenom_36, 1, 1, 1, 1);

        label_exp_52 = new QLabel(groupForm_15);
        label_exp_52->setObjectName("label_exp_52");

        gridLayout_Form_2->addWidget(label_exp_52, 1, 2, 1, 1);

        linePrenom_37 = new QLineEdit(groupForm_15);
        linePrenom_37->setObjectName("linePrenom_37");
        linePrenom_37->setMinimumSize(QSize(0, 40));

        gridLayout_Form_2->addWidget(linePrenom_37, 1, 3, 1, 1);

        label_exp_53 = new QLabel(groupForm_15);
        label_exp_53->setObjectName("label_exp_53");

        gridLayout_Form_2->addWidget(label_exp_53, 2, 0, 1, 1);

        linePrenom_38 = new QLineEdit(groupForm_15);
        linePrenom_38->setObjectName("linePrenom_38");
        linePrenom_38->setMinimumSize(QSize(0, 40));

        gridLayout_Form_2->addWidget(linePrenom_38, 2, 1, 1, 1);

        label_status_field_2 = new QLabel(groupForm_15);
        label_status_field_2->setObjectName("label_status_field_2");

        gridLayout_Form_2->addWidget(label_status_field_2, 2, 2, 1, 1);

        comboStatus_2 = new QComboBox(groupForm_15);
        comboStatus_2->addItem(QString());
        comboStatus_2->addItem(QString());
        comboStatus_2->setObjectName("comboStatus_2");
        comboStatus_2->setMinimumSize(QSize(0, 40));

        gridLayout_Form_2->addWidget(comboStatus_2, 2, 3, 1, 1);

        label_license_8 = new QLabel(groupForm_15);
        label_license_8->setObjectName("label_license_8");

        gridLayout_Form_2->addWidget(label_license_8, 0, 4, 1, 1);

        lineLicenseNum_8 = new QLineEdit(groupForm_15);
        lineLicenseNum_8->setObjectName("lineLicenseNum_8");
        lineLicenseNum_8->setMinimumSize(QSize(0, 40));

        gridLayout_Form_2->addWidget(lineLicenseNum_8, 0, 5, 1, 1);

        label_expiry_date_2 = new QLabel(groupForm_15);
        label_expiry_date_2->setObjectName("label_expiry_date_2");

        gridLayout_Form_2->addWidget(label_expiry_date_2, 1, 4, 1, 1);

        dateExpiry_2 = new QDateEdit(groupForm_15);
        dateExpiry_2->setObjectName("dateExpiry_2");
        dateExpiry_2->setMinimumSize(QSize(0, 40));
        dateExpiry_2->setCalendarPopup(true);

        gridLayout_Form_2->addWidget(dateExpiry_2, 1, 5, 1, 1);

        label_exp_54 = new QLabel(groupForm_15);
        label_exp_54->setObjectName("label_exp_54");

        gridLayout_Form_2->addWidget(label_exp_54, 2, 4, 1, 1);

        comboRole_15 = new QComboBox(groupForm_15);
        comboRole_15->addItem(QString());
        comboRole_15->addItem(QString());
        comboRole_15->addItem(QString());
        comboRole_15->addItem(QString());
        comboRole_15->setObjectName("comboRole_15");
        comboRole_15->setMinimumSize(QSize(0, 40));

        gridLayout_Form_2->addWidget(comboRole_15, 2, 5, 1, 1);


        verticalLayout_Gestion_2->addWidget(groupForm_15);

        frameToolbar_3 = new QFrame(tabGestion_8);
        frameToolbar_3->setObjectName("frameToolbar_3");
        frameToolbar_3->setMinimumSize(QSize(0, 50));
        frameToolbar_3->setStyleSheet(QString::fromUtf8("QFrame#frameToolbar_3 { background-color: #34495e; border-radius: 8px; padding: 8px; border: 1px solid #2c3e50; }\n"
"QPushButton { min-width: 80px; padding: 8px; border-radius: 6px; font-weight: bold; } QPushButton#btnAjouter_8 { background: #2ecc71; color: black; } QPushButton#btnModifier_15 { background: #3498db; color: black; } QPushButton#btnSupprimer_15 { background: #e74c3c; color: white; } QPushButton#btnExportPDF_15 { background: #f39c12; color: white; } QLineEdit { background: #2c3e50; color: #ffffff; border: 1px solid #3498db; border-radius: 6px; padding: 5px; }"));
        horizontalLayout_Toolbar_3 = new QHBoxLayout(frameToolbar_3);
        horizontalLayout_Toolbar_3->setObjectName("horizontalLayout_Toolbar_3");
        btnAjouter_8 = new QPushButton(frameToolbar_3);
        btnAjouter_8->setObjectName("btnAjouter_8");
        btnAjouter_8->setFont(font1);
        btnAjouter_8->setStyleSheet(QString::fromUtf8("QPushButton { border: 1px solid #bdc3c7;\n"
"	background-color: #3498db;\n"
"	color: black;\n"
"	font-family: \"Segoe UI\";\n"
"	font-size: 14px;\n"
"	font-weight: bold;\n"
"	border-radius: 5px;\n"
"	padding: 8px 15px;\n"
"	border: none;\n"
"}\n"
"QPushButton:hover {\n"
"	background-color: #2980b9;\n"
"}\n"
"QPushButton:pressed {\n"
"	background-color: #21618c;\n"
"}"));

        horizontalLayout_Toolbar_3->addWidget(btnAjouter_8);

        btnModifier_15 = new QPushButton(frameToolbar_3);
        btnModifier_15->setObjectName("btnModifier_15");
        btnModifier_15->setFont(font1);
        btnModifier_15->setStyleSheet(QString::fromUtf8("QPushButton { border: 1px solid #bdc3c7;\n"
"	background-color: #3498db;\n"
"	color: black;\n"
"	font-family: \"Segoe UI\";\n"
"	font-size: 14px;\n"
"	font-weight: bold;\n"
"	border-radius: 5px;\n"
"	padding: 8px 15px;\n"
"	border: none;\n"
"}\n"
"QPushButton:hover {\n"
"	background-color: #2980b9;\n"
"}\n"
"QPushButton:pressed {\n"
"	background-color: #21618c;\n"
"}"));

        horizontalLayout_Toolbar_3->addWidget(btnModifier_15);

        btnSupprimer_15 = new QPushButton(frameToolbar_3);
        btnSupprimer_15->setObjectName("btnSupprimer_15");
        btnSupprimer_15->setFont(font1);
        btnSupprimer_15->setStyleSheet(QString::fromUtf8("QPushButton { border: 1px solid #bdc3c7;\n"
"	background-color: #e74c3c;\n"
"	color: white;\n"
"	font-family: \"Segoe UI\";\n"
"	font-size: 14px;\n"
"	font-weight: bold;\n"
"	border-radius: 5px;\n"
"	padding: 8px 15px;\n"
"	border: none;\n"
"}\n"
"QPushButton:hover {\n"
"	background-color: #c0392b;\n"
"}\n"
"QPushButton:pressed {\n"
"	background-color: #a93226;\n"
"}"));

        horizontalLayout_Toolbar_3->addWidget(btnSupprimer_15);

        lineRecherche_15 = new QLineEdit(frameToolbar_3);
        lineRecherche_15->setObjectName("lineRecherche_15");

        horizontalLayout_Toolbar_3->addWidget(lineRecherche_15);

        comboTri_15 = new QComboBox(frameToolbar_3);
        comboTri_15->addItem(QString());
        comboTri_15->addItem(QString());
        comboTri_15->addItem(QString());
        comboTri_15->addItem(QString());
        comboTri_15->setObjectName("comboTri_15");
        comboTri_15->setMinimumSize(QSize(120, 0));

        horizontalLayout_Toolbar_3->addWidget(comboTri_15);

        btnExportPDF_15 = new QPushButton(frameToolbar_3);
        btnExportPDF_15->setObjectName("btnExportPDF_15");
        btnExportPDF_15->setStyleSheet(QString::fromUtf8("QPushButton { background-color: #f39c12; color: white; font-weight: bold; border-radius: 4px; padding: 5px; } QPushButton:hover { background-color: #e67e22; }"));

        horizontalLayout_Toolbar_3->addWidget(btnExportPDF_15);


        verticalLayout_Gestion_2->addWidget(frameToolbar_3);

        tableBateaux_2 = new QTableView(tabGestion_8);
        tableBateaux_2->setObjectName("tableBateaux_2");
        tableBateaux_2->setAlternatingRowColors(true);
        tableBateaux_2->setSelectionBehavior(QAbstractItemView::SelectionBehavior::SelectRows);

        verticalLayout_Gestion_2->addWidget(tableBateaux_2);

        tabWidget_2->addTab(tabGestion_8, QString());
        tabStats_8 = new QWidget();
        tabStats_8->setObjectName("tabStats_8");
        verticalLayout_Stats_2 = new QVBoxLayout(tabStats_8);
        verticalLayout_Stats_2->setObjectName("verticalLayout_Stats_2");
        groupForm_16 = new QGroupBox(tabStats_8);
        groupForm_16->setObjectName("groupForm_16");
        groupForm_16->setStyleSheet(QString::fromUtf8("QGroupBox { font-weight: bold; border: 2px solid #5dade2; border-radius: 6px; background: #2c3e50; padding-top: 35px; margin-top: 20px; } QGroupBox::title { subcontrol-origin: margin; left: 16px; padding: 5px 15px; color: #ffffff; background-color: #5dade2; border-radius: 4px; }"));
        gridLayout_3 = new QGridLayout(groupForm_16);
        gridLayout_3->setObjectName("gridLayout_3");
        label_id_17 = new QLabel(groupForm_16);
        label_id_17->setObjectName("label_id_17");

        gridLayout_3->addWidget(label_id_17, 0, 0, 1, 1);

        lineID_16 = new QLineEdit(groupForm_16);
        lineID_16->setObjectName("lineID_16");
        lineID_16->setMinimumSize(QSize(0, 40));

        gridLayout_3->addWidget(lineID_16, 0, 1, 1, 1);

        label_nom_16 = new QLabel(groupForm_16);
        label_nom_16->setObjectName("label_nom_16");

        gridLayout_3->addWidget(label_nom_16, 0, 2, 1, 1);

        lineLicence_16 = new QLineEdit(groupForm_16);
        lineLicence_16->setObjectName("lineLicence_16");
        lineLicence_16->setMinimumSize(QSize(0, 40));

        gridLayout_3->addWidget(lineLicence_16, 0, 3, 1, 1);

        label_exp_55 = new QLabel(groupForm_16);
        label_exp_55->setObjectName("label_exp_55");

        gridLayout_3->addWidget(label_exp_55, 1, 0, 1, 1);

        linePrenom_39 = new QLineEdit(groupForm_16);
        linePrenom_39->setObjectName("linePrenom_39");
        linePrenom_39->setMinimumSize(QSize(0, 40));

        gridLayout_3->addWidget(linePrenom_39, 1, 1, 1, 1);

        label_exp_56 = new QLabel(groupForm_16);
        label_exp_56->setObjectName("label_exp_56");

        gridLayout_3->addWidget(label_exp_56, 1, 2, 1, 1);

        linePrenom_40 = new QLineEdit(groupForm_16);
        linePrenom_40->setObjectName("linePrenom_40");
        linePrenom_40->setMinimumSize(QSize(0, 40));

        gridLayout_3->addWidget(linePrenom_40, 1, 3, 1, 1);

        label_exp_57 = new QLabel(groupForm_16);
        label_exp_57->setObjectName("label_exp_57");

        gridLayout_3->addWidget(label_exp_57, 0, 4, 1, 1);

        comboRole_16 = new QComboBox(groupForm_16);
        comboRole_16->addItem(QString());
        comboRole_16->addItem(QString());
        comboRole_16->addItem(QString());
        comboRole_16->addItem(QString());
        comboRole_16->setObjectName("comboRole_16");
        comboRole_16->setMinimumSize(QSize(0, 40));

        gridLayout_3->addWidget(comboRole_16, 0, 5, 1, 1);


        verticalLayout_Stats_2->addWidget(groupForm_16);

        frameToolbar_4 = new QFrame(tabStats_8);
        frameToolbar_4->setObjectName("frameToolbar_4");
        frameToolbar_4->setMinimumSize(QSize(0, 50));
        frameToolbar_4->setStyleSheet(QString::fromUtf8("QFrame#frameToolbar_4 { background-color: #34495e; border-radius: 8px; padding: 8px; border: 1px solid #2c3e50; }\n"
"QPushButton { min-width: 80px; padding: 8px; border-radius: 6px; font-weight: bold; } QPushButton#btnAjouter_14 { background: #3498db; color: white; } QPushButton#btnModifier_2 { background: #5dade2; color: white; } QPushButton#btnSupprimer_2 { background: #e74c3c; color: white; } QPushButton#btnExportPDF_3 { background: #f39c12; color: white; } QLineEdit { background: #2c3e50; color: #ffffff; border: 1px solid #3498db; border-radius: 6px; padding: 5px; }"));
        horizontalLayout_Toolbar_4 = new QHBoxLayout(frameToolbar_4);
        horizontalLayout_Toolbar_4->setObjectName("horizontalLayout_Toolbar_4");
        btnAjouter_21 = new QPushButton(frameToolbar_4);
        btnAjouter_21->setObjectName("btnAjouter_21");

        horizontalLayout_Toolbar_4->addWidget(btnAjouter_21);

        btnModifier_16 = new QPushButton(frameToolbar_4);
        btnModifier_16->setObjectName("btnModifier_16");

        horizontalLayout_Toolbar_4->addWidget(btnModifier_16);

        btnSupprimer_16 = new QPushButton(frameToolbar_4);
        btnSupprimer_16->setObjectName("btnSupprimer_16");

        horizontalLayout_Toolbar_4->addWidget(btnSupprimer_16);

        lineRecherche_16 = new QLineEdit(frameToolbar_4);
        lineRecherche_16->setObjectName("lineRecherche_16");

        horizontalLayout_Toolbar_4->addWidget(lineRecherche_16);

        comboTri_16 = new QComboBox(frameToolbar_4);
        comboTri_16->addItem(QString());
        comboTri_16->addItem(QString());
        comboTri_16->addItem(QString());
        comboTri_16->addItem(QString());
        comboTri_16->setObjectName("comboTri_16");
        comboTri_16->setMinimumSize(QSize(120, 0));

        horizontalLayout_Toolbar_4->addWidget(comboTri_16);

        btnExportPDF_16 = new QPushButton(frameToolbar_4);
        btnExportPDF_16->setObjectName("btnExportPDF_16");

        horizontalLayout_Toolbar_4->addWidget(btnExportPDF_16);


        verticalLayout_Stats_2->addWidget(frameToolbar_4);

        tableHistorique_3 = new QTableView(tabStats_8);
        tableHistorique_3->setObjectName("tableHistorique_3");
        tableHistorique_3->setAlternatingRowColors(true);
        tableHistorique_3->setSelectionBehavior(QAbstractItemView::SelectionBehavior::SelectRows);

        verticalLayout_Stats_2->addWidget(tableHistorique_3);

        tabWidget_2->addTab(tabStats_8, QString());
        tabStatistiques_8 = new QWidget();
        tabStatistiques_8->setObjectName("tabStatistiques_8");
        gridLayout_Stats_8 = new QGridLayout(tabStatistiques_8);
        gridLayout_Stats_8->setObjectName("gridLayout_Stats_8");
        groupTotal_8 = new QGroupBox(tabStatistiques_8);
        groupTotal_8->setObjectName("groupTotal_8");
        groupTotal_8->setMinimumSize(QSize(0, 150));
        groupTotal_8->setStyleSheet(QString::fromUtf8("QGroupBox { font-weight: bold; border: 3px solid #34495e; border-radius: 12px; background: #2c3e50; margin-top: 12px; padding: 20px; } QGroupBox::title { subcontrol-origin: margin; left: 20px; padding: 5px 15px; color: #ffffff; background-color: #34495e; border-radius: 6px; font-size: 13pt; }"));
        v1_2 = new QVBoxLayout(groupTotal_8);
        v1_2->setObjectName("v1_2");
        lblTotalBateauxCount_8 = new QLabel(groupTotal_8);
        lblTotalBateauxCount_8->setObjectName("lblTotalBateauxCount_8");
        lblTotalBateauxCount_8->setStyleSheet(QString::fromUtf8("font-size: 48px; font-weight: bold; color: #ffffff; padding: 10px;"));
        lblTotalBateauxCount_8->setAlignment(Qt::AlignmentFlag::AlignCenter);

        v1_2->addWidget(lblTotalBateauxCount_8);

        lblTotalBateauxText_8 = new QLabel(groupTotal_8);
        lblTotalBateauxText_8->setObjectName("lblTotalBateauxText_8");
        lblTotalBateauxText_8->setStyleSheet(QString::fromUtf8("color: #778da9; font-size: 11pt;"));
        lblTotalBateauxText_8->setAlignment(Qt::AlignmentFlag::AlignCenter);

        v1_2->addWidget(lblTotalBateauxText_8);


        gridLayout_Stats_8->addWidget(groupTotal_8, 0, 0, 1, 1);

        groupActif_8 = new QGroupBox(tabStatistiques_8);
        groupActif_8->setObjectName("groupActif_8");
        groupActif_8->setMinimumSize(QSize(0, 150));
        groupActif_8->setStyleSheet(QString::fromUtf8("QGroupBox { font-weight: bold; border: 3px solid #3498db; border-radius: 12px; background: #2c3e50; margin-top: 12px; padding: 20px; } QGroupBox::title { subcontrol-origin: margin; left: 20px; padding: 5px 15px; color: #ffffff; background-color: #3498db; border-radius: 6px; font-size: 13pt; }"));
        v2_2 = new QVBoxLayout(groupActif_8);
        v2_2->setObjectName("v2_2");
        lblLicencesActivesCount_8 = new QLabel(groupActif_8);
        lblLicencesActivesCount_8->setObjectName("lblLicencesActivesCount_8");
        lblLicencesActivesCount_8->setStyleSheet(QString::fromUtf8("font-size: 48px; font-weight: bold; color: #3498db; padding: 10px;"));
        lblLicencesActivesCount_8->setAlignment(Qt::AlignmentFlag::AlignCenter);

        v2_2->addWidget(lblLicencesActivesCount_8);

        lblLicencesActivesText_8 = new QLabel(groupActif_8);
        lblLicencesActivesText_8->setObjectName("lblLicencesActivesText_8");
        lblLicencesActivesText_8->setStyleSheet(QString::fromUtf8("color: #778da9; font-size: 11pt;"));
        lblLicencesActivesText_8->setAlignment(Qt::AlignmentFlag::AlignCenter);

        v2_2->addWidget(lblLicencesActivesText_8);


        gridLayout_Stats_8->addWidget(groupActif_8, 0, 1, 1, 1);

        groupMer_8 = new QGroupBox(tabStatistiques_8);
        groupMer_8->setObjectName("groupMer_8");
        groupMer_8->setMinimumSize(QSize(0, 150));
        groupMer_8->setStyleSheet(QString::fromUtf8("QGroupBox { font-weight: bold; border: 3px solid #f39c12; border-radius: 12px; background: #2c3e50; margin-top: 12px; padding: 20px; } QGroupBox::title { subcontrol-origin: margin; left: 20px; padding: 5px 15px; color: #ffffff; background-color: #f39c12; border-radius: 6px; font-size: 13pt; }"));
        verticalLayout_Mer_8 = new QVBoxLayout(groupMer_8);
        verticalLayout_Mer_8->setObjectName("verticalLayout_Mer_8");
        lblBateauxMerCount_8 = new QLabel(groupMer_8);
        lblBateauxMerCount_8->setObjectName("lblBateauxMerCount_8");
        lblBateauxMerCount_8->setStyleSheet(QString::fromUtf8("font-size: 48px; font-weight: bold; color: #f39c12; padding: 10px;"));
        lblBateauxMerCount_8->setAlignment(Qt::AlignmentFlag::AlignCenter);

        verticalLayout_Mer_8->addWidget(lblBateauxMerCount_8);

        lblBateauxMerText_8 = new QLabel(groupMer_8);
        lblBateauxMerText_8->setObjectName("lblBateauxMerText_8");
        lblBateauxMerText_8->setStyleSheet(QString::fromUtf8("color: #778da9; font-size: 11pt;"));
        lblBateauxMerText_8->setAlignment(Qt::AlignmentFlag::AlignCenter);

        verticalLayout_Mer_8->addWidget(lblBateauxMerText_8);


        gridLayout_Stats_8->addWidget(groupMer_8, 0, 2, 1, 1);

        groupExpire_8 = new QGroupBox(tabStatistiques_8);
        groupExpire_8->setObjectName("groupExpire_8");
        groupExpire_8->setMinimumSize(QSize(0, 150));
        groupExpire_8->setStyleSheet(QString::fromUtf8("QGroupBox { font-weight: bold; border: 3px solid #e74c3c; border-radius: 12px; background: #2c3e50; margin-top: 12px; padding: 20px; } QGroupBox::title { subcontrol-origin: margin; left: 20px; padding: 5px 15px; color: #ffffff; background-color: #e74c3c; border-radius: 6px; font-size: 13pt; }"));
        verticalLayout_Expire_8 = new QVBoxLayout(groupExpire_8);
        verticalLayout_Expire_8->setObjectName("verticalLayout_Expire_8");
        lblLicencesExpireesCount_8 = new QLabel(groupExpire_8);
        lblLicencesExpireesCount_8->setObjectName("lblLicencesExpireesCount_8");
        lblLicencesExpireesCount_8->setStyleSheet(QString::fromUtf8("font-size: 48px; font-weight: bold; color: #e74c3c; padding: 10px;"));
        lblLicencesExpireesCount_8->setAlignment(Qt::AlignmentFlag::AlignCenter);

        verticalLayout_Expire_8->addWidget(lblLicencesExpireesCount_8);

        lblLicencesExpireesText_8 = new QLabel(groupExpire_8);
        lblLicencesExpireesText_8->setObjectName("lblLicencesExpireesText_8");
        lblLicencesExpireesText_8->setStyleSheet(QString::fromUtf8("color: #778da9; font-size: 11pt;"));
        lblLicencesExpireesText_8->setAlignment(Qt::AlignmentFlag::AlignCenter);

        verticalLayout_Expire_8->addWidget(lblLicencesExpireesText_8);


        gridLayout_Stats_8->addWidget(groupExpire_8, 0, 3, 1, 1);

        groupNotifications_8 = new QGroupBox(tabStatistiques_8);
        groupNotifications_8->setObjectName("groupNotifications_8");
        groupNotifications_8->setStyleSheet(QString::fromUtf8("QGroupBox { font-weight: bold; border: 2px solid #34495e; border-radius: 10px; background: #2c3e50; margin-top: 12px; padding: 15px; } QGroupBox::title { subcontrol-origin: margin; left: 20px; padding: 5px 15px; color: #ffffff; background-color: #34495e; border-radius: 6px; font-size: 12pt; }"));
        verticalLayout_Notifications_2 = new QVBoxLayout(groupNotifications_8);
        verticalLayout_Notifications_2->setObjectName("verticalLayout_Notifications_2");
        textNotifications_8 = new QTextEdit(groupNotifications_8);
        textNotifications_8->setObjectName("textNotifications_8");
        textNotifications_8->setMinimumSize(QSize(0, 100));
        textNotifications_8->setStyleSheet(QString::fromUtf8("background: #1a252f; color: #ffffff; border: 1px solid #34495e; border-radius: 6px; padding: 10px; font-size: 10pt;"));
        textNotifications_8->setReadOnly(true);

        verticalLayout_Notifications_2->addWidget(textNotifications_8);


        gridLayout_Stats_8->addWidget(groupNotifications_8, 1, 0, 1, 4);

        btnAIAssistant_8 = new QPushButton(tabStatistiques_8);
        btnAIAssistant_8->setObjectName("btnAIAssistant_8");
        btnAIAssistant_8->setCursor(QCursor(Qt::CursorShape::PointingHandCursor));
        btnAIAssistant_8->setStyleSheet(QString::fromUtf8("\n"
"          QPushButton {\n"
"           background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #6d5dfc, stop:1 #2b95ce);\n"
"           color: white;\n"
"           border-radius: 20px;\n"
"           font-weight: bold;\n"
"           font-size: 14px;\n"
"           border: 2px solid rgba(255,255,255,0.2);\n"
"          }\n"
"          QPushButton:hover {\n"
"           background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #2b95ce, stop:1 #6d5dfc);\n"
"          }\n"
"         "));

        gridLayout_Stats_8->addWidget(btnAIAssistant_8, 2, 0, 1, 4);

        tabWidget_2->addTab(tabStatistiques_8, QString());

        verticalLayout_MainContent_2->addWidget(tabWidget_2);

        stackedWidget->addWidget(pageBateaux);
        pageMaintenance = new QWidget();
        pageMaintenance->setObjectName("pageMaintenance");
        verticalLayout_4 = new QVBoxLayout(pageMaintenance);
        verticalLayout_4->setSpacing(0);
        verticalLayout_4->setObjectName("verticalLayout_4");
        verticalLayout_4->setContentsMargins(0, 0, 0, 0);
        frame_Equipement_Header = new QFrame(pageMaintenance);
        frame_Equipement_Header->setObjectName("frame_Equipement_Header");
        frame_Equipement_Header->setMinimumSize(QSize(0, 120));
        frame_Equipement_Header->setMaximumSize(QSize(16777215, 120));
        frame_Equipement_Header->setStyleSheet(QString::fromUtf8("#frame_Equipement_Header {\n"
"     border-image: url(:/new/prefix1/img1.jpg);\n"
"}\n"
"#frame_Equipement_Header_Overlay {\n"
"     background-color: rgba(0, 0, 0, 0.55);\n"
"}"));
        frame_Equipement_Header->setFrameShape(QFrame::Shape::NoFrame);
        verticalLayout_Header_Equipement = new QVBoxLayout(frame_Equipement_Header);
        verticalLayout_Header_Equipement->setSpacing(0);
        verticalLayout_Header_Equipement->setObjectName("verticalLayout_Header_Equipement");
        verticalLayout_Header_Equipement->setContentsMargins(0, 0, 0, 0);
        frame_Equipement_Header_Overlay = new QFrame(frame_Equipement_Header);
        frame_Equipement_Header_Overlay->setObjectName("frame_Equipement_Header_Overlay");
        verticalLayout_Title_Equipement = new QVBoxLayout(frame_Equipement_Header_Overlay);
        verticalLayout_Title_Equipement->setObjectName("verticalLayout_Title_Equipement");
        title_label_equipement = new QLabel(frame_Equipement_Header_Overlay);
        title_label_equipement->setObjectName("title_label_equipement");
        title_label_equipement->setStyleSheet(QString::fromUtf8("color: white; font-size: 28px; font-weight: bold; font-family: \"Script MT Bold\";"));
        title_label_equipement->setAlignment(Qt::AlignmentFlag::AlignCenter);

        verticalLayout_Title_Equipement->addWidget(title_label_equipement);


        verticalLayout_Header_Equipement->addWidget(frame_Equipement_Header_Overlay);


        verticalLayout_4->addWidget(frame_Equipement_Header);

        tabWidget = new QTabWidget(pageMaintenance);
        tabWidget->setObjectName("tabWidget");
        tabWidget->setStyleSheet(QString::fromUtf8("/* Page background white (same as P\303\252cheur) */\n"
"QTabWidget::pane {\n"
"    border: 1px solid #3498db;\n"
"    background-color: #ffffff;\n"
"    border-radius: 5px;\n"
"}\n"
"/* Tab bar - same look as P\303\252cheur buttons (Liste, Statistiques, Missions IA): teal pill-shaped */\n"
"QTabWidget#tabWidget QTabBar {\n"
"    background-color: transparent;\n"
"}\n"
"QTabWidget#tabWidget QTabBar::tab {\n"
"    background-color: #3CA99C;\n"
"    color: #ffffff;\n"
"    padding: 6px 14px;\n"
"    margin: 4px 6px;\n"
"    min-width: 90px;\n"
"    border: none;\n"
"    border-radius: 8px;\n"
"    font-family: \"Segoe UI\", Arial, sans-serif;\n"
"    font-size: 12px;\n"
"    font-weight: bold;\n"
"    text-align: center;\n"
"}\n"
"QTabWidget#tabWidget QTabBar::tab:hover {\n"
"    background-color: #36a399;\n"
"}\n"
"QTabWidget#tabWidget QTabBar::tab:selected {\n"
"    background-color: #2d8f85;\n"
"    font-weight: bold;\n"
"}\n"
"/* Global Widget Styles for this Tab */\n"
"QWidget {\n"
"    font-family: \"Segoe"
                        " UI\";\n"
"    background-color: #ffffff;\n"
"}\n"
"/* Attribute labels inside form - white text, transparent background (same as P\303\252cheur/Employ\303\251) */\n"
"QGroupBox QLabel {\n"
"    color: #ffffff;\n"
"    background: transparent;\n"
"    border: none;\n"
"    font-family: \"Segoe UI\";\n"
"    font-weight: bold;\n"
"    font-size: 12px;\n"
"    padding: 2px;\n"
"}\n"
"/* Widgets inside form (e.g. Disponibilit\303\251 row) - same background as form */\n"
"QGroupBox QWidget {\n"
"    background-color: transparent;\n"
"}\n"
"\n"
"/* Radio Buttons */\n"
"QRadioButton {\n"
"    background-color: transparent;\n"
"    color: #e0e0e0;\n"
"    font-size: 13px;\n"
"}\n"
"QRadioButton::indicator {\n"
"    width: 14px;\n"
"    height: 14px;\n"
"    border-radius: 8px;\n"
"    border: 2px solid #3b9ddd;\n"
"    background-color: transparent;\n"
"}\n"
"QRadioButton::indicator:checked {\n"
"    background-color: #3b9ddd;\n"
"    border: 2px solid #3b9ddd;\n"
"}\n"
"\n"
"/* GroupBox (Form) - black/dark like P\303"
                        "\252cheur */\n"
"QGroupBox {\n"
"    border: 2px solid #3498db;\n"
"    border-radius: 8px;\n"
"    margin-top: 10px;\n"
"    background-color: #232323;\n"
"    font-weight: bold;\n"
"    color: #3498db;\n"
"}\n"
"QGroupBox::title {\n"
"    subcontrol-origin: margin;\n"
"    subcontrol-position: top center;\n"
"    padding: 0 10px;\n"
"    background-color: #232323;\n"
"    color: #3498db;\n"
"}\n"
"\n"
"/* Inputs inside form */\n"
"QLineEdit, QComboBox, QDateEdit, QSpinBox, QPlainTextEdit {\n"
"    background-color: #2d2d2d;\n"
"    border: 1px solid #3d3d3d;\n"
"    border-radius: 6px;\n"
"    padding: 8px;\n"
"    color: #ffffff;\n"
"    min-height: 25px;\n"
"}\n"
"QLineEdit:focus, QComboBox:focus, QDateEdit:focus {\n"
"    border: 1px solid #3498db;\n"
"    background-color: #353535;\n"
"}\n"
"\n"
"/* Dropdown list (options) - black background, white text */\n"
"QComboBox QAbstractItemView {\n"
"    background-color: #232323;\n"
"    color: #ffffff;\n"
"    border: 1px solid #3498db;\n"
"    border-radius:"
                        " 4px;\n"
"    padding: 4px;\n"
"    selection-background-color: #3498db;\n"
"}\n"
"QComboBox QAbstractItemView::item {\n"
"    min-height: 24px;\n"
"}\n"
"\n"
"/* Tri par label and sort combo - dark text on white page so they are clear */\n"
"QLabel#label_10 {\n"
"    color: #1a3a52;\n"
"    background: transparent;\n"
"    font-weight: bold;\n"
"    font-size: 13px;\n"
"}\n"
"QComboBox#comboBox_3 {\n"
"    color: #1a3a52;\n"
"    background-color: #f0f4f8;\n"
"    border: 1px solid #3498db;\n"
"    border-radius: 6px;\n"
"    padding: 8px;\n"
"    min-height: 25px;\n"
"}\n"
"\n"
"/* Buttons - Ajouter/Modifier: blue + black text; Supprimer: red + white (flat, rounded) */\n"
"QPushButton {\n"
"    background-color: #3498db;\n"
"    color: black;\n"
"    border-radius: 6px;\n"
"    padding: 8px 16px;\n"
"    font-weight: bold;\n"
"    border: none;\n"
"}\n"
"QPushButton:hover {\n"
"    background-color: #2980b9;\n"
"}\n"
"QPushButton:pressed {\n"
"    background-color: #21618c;\n"
"}\n"
"\n"
"/* Supprimer - red "
                        "*/\n"
"QPushButton#pushButton_5 {\n"
"    background-color: #e74c3c;\n"
"    color: white;\n"
"}\n"
"QPushButton#pushButton_5:hover {\n"
"    background-color: #c0392b;\n"
"}\n"
"QPushButton#pushButton_5:pressed {\n"
"    background-color: #a93226;\n"
"}\n"
"\n"
"/* Exporter en PDF - orange */\n"
"QPushButton#pushButton_7 {\n"
"    background-color: #f39c12;\n"
"    color: white;\n"
"}\n"
"QPushButton#pushButton_7:hover {\n"
"    background-color: #f5b041;\n"
"}\n"
"\n"
"/* Table Widget (list) - black/dark like P\303\252cheur */\n"
"QTableWidget {\n"
"    background-color: #232323;\n"
"    alternate-background-color: #2d2d2d;\n"
"    gridline-color: #3d3d3d;\n"
"    border: 2px solid #3498db;\n"
"    border-radius: 8px;\n"
"    color: #e0e0e0;\n"
"}\n"
"QTableWidget::item:selected {\n"
"    background-color: #3498db;\n"
"    color: white;\n"
"}\n"
"QHeaderView::section {\n"
"    background-color: #2d2d2d;\n"
"    color: #ffffff;\n"
"    padding: 8px;\n"
"    border: none;\n"
"    border-bottom: 2px solid #3498"
                        "db;\n"
"    font-weight: bold;\n"
"}\n"
"\n"
"/* Frames (table container) - black/dark */\n"
"QFrame#chartContainer, QFrame#chartContainer_2 {\n"
"    border: 2px solid #3498db;\n"
"    border-radius: 8px;\n"
"    background-color: #232323;\n"
"}\n"
"\n"
"/* Chart View */\n"
"QChartView {\n"
"    background-color: transparent;\n"
"}\n"
"\n"
"/* Menus */\n"
"QMenuBar {\n"
"    background-color: #1a1a1a;\n"
"    color: #e0e0e0;\n"
"}\n"
"QMenuBar::item:selected {\n"
"    background-color: #3b9ddd;\n"
"    color: white;\n"
"}\n"
"QStatusBar {\n"
"    background-color: #1a1a1a;\n"
"    color: #e0e0e0;\n"
"}"));
        tab_gestion = new QWidget();
        tab_gestion->setObjectName("tab_gestion");
        verticalLayout_equip_content = new QVBoxLayout(tab_gestion);
        verticalLayout_equip_content->setSpacing(12);
        verticalLayout_equip_content->setObjectName("verticalLayout_equip_content");
        verticalLayout_equip_content->setContentsMargins(20, 15, 20, 20);
        scrollArea_equip_form = new QScrollArea(tab_gestion);
        scrollArea_equip_form->setObjectName("scrollArea_equip_form");
        QSizePolicy sizePolicy(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Expanding);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(1);
        sizePolicy.setHeightForWidth(scrollArea_equip_form->sizePolicy().hasHeightForWidth());
        scrollArea_equip_form->setSizePolicy(sizePolicy);
        scrollArea_equip_form->setMinimumSize(QSize(0, 0));
        scrollArea_equip_form->setFrameShape(QFrame::Shape::NoFrame);
        scrollArea_equip_form->setVerticalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAlwaysOff);
        scrollArea_equip_form->setHorizontalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAlwaysOff);
        scrollArea_equip_form->setWidgetResizable(true);
        scrollAreaWidgetContents_equip = new QWidget();
        scrollAreaWidgetContents_equip->setObjectName("scrollAreaWidgetContents_equip");
        scrollAreaWidgetContents_equip->setGeometry(QRect(0, 0, 60, 16));
        scrollAreaWidgetContents_equip->setMinimumSize(QSize(0, 0));
        groupBox = new QGroupBox(scrollAreaWidgetContents_equip);
        groupBox->setObjectName("groupBox");
        groupBox->setGeometry(QRect(0, 0, 1181, 401));
        groupBox->setStyleSheet(QString::fromUtf8("QGroupBox {\n"
"    background-color: #232323;\n"
"    border-radius: 8px;\n"
"    border: 2px solid #3498db;\n"
"    margin-top: 20px;\n"
"    color: white;\n"
"    font-weight: bold;\n"
"}\n"
"QGroupBox::title {\n"
"    subcontrol-origin: margin;\n"
"    subcontrol-position: top center;\n"
"    padding: 0 3px;\n"
"    color: #3498db;\n"
"}\n"
"QLabel {\n"
"    color: #ffffff;\n"
"    font-family: \"Segoe UI\";\n"
"    font-weight: bold;\n"
"    font-size: 12px;\n"
"    padding: 2px;\n"
"    border: none;\n"
"    background: transparent;\n"
"}\n"
"QLineEdit, QComboBox, QDateEdit {\n"
"    color: white;\n"
"    background-color: #2d2d2d;\n"
"    border: 1px solid #3d3d3d;\n"
"    padding: 5px;\n"
"    border-radius: 4px;\n"
"    font-family: \"Segoe UI\";\n"
"    font-weight: bold;\n"
"}"));
        gridLayout_equip_form = new QGridLayout(groupBox);
        gridLayout_equip_form->setObjectName("gridLayout_equip_form");
        gridLayout_equip_form->setHorizontalSpacing(12);
        gridLayout_equip_form->setVerticalSpacing(8);
        label = new QLabel(groupBox);
        label->setObjectName("label");

        gridLayout_equip_form->addWidget(label, 0, 0, 1, 1);

        lineEdit = new QLineEdit(groupBox);
        lineEdit->setObjectName("lineEdit");
        lineEdit->setMinimumSize(QSize(0, 37));

        gridLayout_equip_form->addWidget(lineEdit, 0, 1, 1, 1);

        label_2 = new QLabel(groupBox);
        label_2->setObjectName("label_2");

        gridLayout_equip_form->addWidget(label_2, 0, 2, 1, 1);

        lineEdit_2 = new QLineEdit(groupBox);
        lineEdit_2->setObjectName("lineEdit_2");
        lineEdit_2->setMinimumSize(QSize(0, 37));

        gridLayout_equip_form->addWidget(lineEdit_2, 0, 3, 1, 1);

        label_3 = new QLabel(groupBox);
        label_3->setObjectName("label_3");

        gridLayout_equip_form->addWidget(label_3, 1, 0, 1, 1);

        lineEdit_3 = new QLineEdit(groupBox);
        lineEdit_3->setObjectName("lineEdit_3");
        lineEdit_3->setMinimumSize(QSize(0, 37));

        gridLayout_equip_form->addWidget(lineEdit_3, 1, 1, 1, 1);

        label_4 = new QLabel(groupBox);
        label_4->setObjectName("label_4");

        gridLayout_equip_form->addWidget(label_4, 1, 2, 1, 1);

        comboBox = new QComboBox(groupBox);
        comboBox->addItem(QString());
        comboBox->addItem(QString());
        comboBox->addItem(QString());
        comboBox->setObjectName("comboBox");
        comboBox->setMinimumSize(QSize(0, 37));

        gridLayout_equip_form->addWidget(comboBox, 1, 3, 1, 1);

        label_5 = new QLabel(groupBox);
        label_5->setObjectName("label_5");

        gridLayout_equip_form->addWidget(label_5, 2, 0, 1, 1);

        dateEdit = new QDateEdit(groupBox);
        dateEdit->setObjectName("dateEdit");
        dateEdit->setMinimumSize(QSize(0, 37));

        gridLayout_equip_form->addWidget(dateEdit, 2, 1, 1, 1);

        label_6 = new QLabel(groupBox);
        label_6->setObjectName("label_6");

        gridLayout_equip_form->addWidget(label_6, 2, 2, 1, 1);

        comboBox_2 = new QComboBox(groupBox);
        comboBox_2->addItem(QString());
        comboBox_2->addItem(QString());
        comboBox_2->addItem(QString());
        comboBox_2->setObjectName("comboBox_2");
        comboBox_2->setMinimumSize(QSize(0, 37));

        gridLayout_equip_form->addWidget(comboBox_2, 2, 3, 1, 1);

        label_7 = new QLabel(groupBox);
        label_7->setObjectName("label_7");

        gridLayout_equip_form->addWidget(label_7, 3, 0, 1, 1);

        lineEdit_4 = new QLineEdit(groupBox);
        lineEdit_4->setObjectName("lineEdit_4");
        lineEdit_4->setMinimumSize(QSize(0, 37));

        gridLayout_equip_form->addWidget(lineEdit_4, 3, 1, 1, 1);

        label_8 = new QLabel(groupBox);
        label_8->setObjectName("label_8");

        gridLayout_equip_form->addWidget(label_8, 3, 2, 1, 1);

        lineEdit_11 = new QLineEdit(groupBox);
        lineEdit_11->setObjectName("lineEdit_11");
        lineEdit_11->setMinimumSize(QSize(0, 37));

        gridLayout_equip_form->addWidget(lineEdit_11, 3, 3, 1, 1);

        label_9 = new QLabel(groupBox);
        label_9->setObjectName("label_9");

        gridLayout_equip_form->addWidget(label_9, 4, 0, 1, 1);

        widget_disp_equip = new QWidget(groupBox);
        widget_disp_equip->setObjectName("widget_disp_equip");
        horizontalLayout_disp = new QHBoxLayout(widget_disp_equip);
        horizontalLayout_disp->setSpacing(15);
        horizontalLayout_disp->setObjectName("horizontalLayout_disp");
        radioButton = new QRadioButton(widget_disp_equip);
        radioButton->setObjectName("radioButton");
        radioButton->setChecked(true);

        horizontalLayout_disp->addWidget(radioButton);

        radioButton_2 = new QRadioButton(widget_disp_equip);
        radioButton_2->setObjectName("radioButton_2");

        horizontalLayout_disp->addWidget(radioButton_2);

        spacerDisp = new QSpacerItem(40, 20, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);

        horizontalLayout_disp->addItem(spacerDisp);


        gridLayout_equip_form->addWidget(widget_disp_equip, 4, 1, 1, 3);

        scrollArea_equip_form->setWidget(scrollAreaWidgetContents_equip);

        verticalLayout_equip_content->addWidget(scrollArea_equip_form);

        frameEquipToolbar = new QFrame(tab_gestion);
        frameEquipToolbar->setObjectName("frameEquipToolbar");
        frameEquipToolbar->setMinimumSize(QSize(0, 50));
        frameEquipToolbar->setStyleSheet(QString::fromUtf8("QFrame#frameEquipToolbar { background-color: #34495e; border-radius: 8px; padding: 8px; border: 1px solid #2c3e50; }"));
        h_buttons_equip = new QHBoxLayout(frameEquipToolbar);
        h_buttons_equip->setSpacing(10);
        h_buttons_equip->setObjectName("h_buttons_equip");
        pushButton_2 = new QPushButton(frameEquipToolbar);
        pushButton_2->setObjectName("pushButton_2");
        pushButton_2->setStyleSheet(QString::fromUtf8(""));

        h_buttons_equip->addWidget(pushButton_2);

        pushButton_4 = new QPushButton(frameEquipToolbar);
        pushButton_4->setObjectName("pushButton_4");

        h_buttons_equip->addWidget(pushButton_4);

        pushButton_5 = new QPushButton(frameEquipToolbar);
        pushButton_5->setObjectName("pushButton_5");

        h_buttons_equip->addWidget(pushButton_5);

        spacerEquipBtn = new QSpacerItem(40, 20, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);

        h_buttons_equip->addItem(spacerEquipBtn);

        pushButton_7 = new QPushButton(frameEquipToolbar);
        pushButton_7->setObjectName("pushButton_7");
        pushButton_7->setStyleSheet(QString::fromUtf8(""));

        h_buttons_equip->addWidget(pushButton_7);


        verticalLayout_equip_content->addWidget(frameEquipToolbar);

        h_search_equip = new QHBoxLayout();
        h_search_equip->setSpacing(10);
        h_search_equip->setObjectName("h_search_equip");
        lineEdit_8 = new QLineEdit(tab_gestion);
        lineEdit_8->setObjectName("lineEdit_8");
        lineEdit_8->setMinimumSize(QSize(200, 43));
        lineEdit_8->setClearButtonEnabled(true);

        h_search_equip->addWidget(lineEdit_8);

        spacerEquipSearch = new QSpacerItem(40, 20, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);

        h_search_equip->addItem(spacerEquipSearch);

        label_10 = new QLabel(tab_gestion);
        label_10->setObjectName("label_10");

        h_search_equip->addWidget(label_10);

        comboBox_3 = new QComboBox(tab_gestion);
        comboBox_3->addItem(QString());
        comboBox_3->addItem(QString());
        comboBox_3->addItem(QString());
        comboBox_3->addItem(QString());
        comboBox_3->setObjectName("comboBox_3");
        comboBox_3->setMinimumSize(QSize(120, 43));

        h_search_equip->addWidget(comboBox_3);


        verticalLayout_equip_content->addLayout(h_search_equip);

        chartContainer_2 = new QFrame(tab_gestion);
        chartContainer_2->setObjectName("chartContainer_2");
        chartContainer_2->setFrameShape(QFrame::Shape::StyledPanel);
        chartContainer_2->setFrameShadow(QFrame::Shadow::Raised);
        verticalLayout_table_equip = new QVBoxLayout(chartContainer_2);
        verticalLayout_table_equip->setSpacing(0);
        verticalLayout_table_equip->setObjectName("verticalLayout_table_equip");
        verticalLayout_table_equip->setContentsMargins(0, 0, 0, 0);
        tableWidget_3 = new QTableWidget(chartContainer_2);
        if (tableWidget_3->columnCount() < 4)
            tableWidget_3->setColumnCount(4);
        QTableWidgetItem *__qtablewidgetitem77 = new QTableWidgetItem();
        tableWidget_3->setHorizontalHeaderItem(0, __qtablewidgetitem77);
        QTableWidgetItem *__qtablewidgetitem78 = new QTableWidgetItem();
        tableWidget_3->setHorizontalHeaderItem(1, __qtablewidgetitem78);
        QTableWidgetItem *__qtablewidgetitem79 = new QTableWidgetItem();
        tableWidget_3->setHorizontalHeaderItem(2, __qtablewidgetitem79);
        QTableWidgetItem *__qtablewidgetitem80 = new QTableWidgetItem();
        tableWidget_3->setHorizontalHeaderItem(3, __qtablewidgetitem80);
        tableWidget_3->setObjectName("tableWidget_3");
        tableWidget_3->setStyleSheet(QString::fromUtf8(""));
        tableWidget_3->setSortingEnabled(true);
        tableWidget_3->horizontalHeader()->setStretchLastSection(true);

        verticalLayout_table_equip->addWidget(tableWidget_3);


        verticalLayout_equip_content->addWidget(chartContainer_2);

        tabWidget->addTab(tab_gestion, QString());
        tab_statistique = new QWidget();
        tab_statistique->setObjectName("tab_statistique");
        frame_stat = new QFrame(tab_statistique);
        frame_stat->setObjectName("frame_stat");
        frame_stat->setGeometry(QRect(50, 50, 1031, 601));
        frame_stat->setStyleSheet(QString::fromUtf8("background-color: #212121;\n"
"border: 2px solid #3b9ddd;\n"
"border-radius: 10px;"));
        frame_stat->setFrameShape(QFrame::Shape::StyledPanel);
        frame_stat->setFrameShadow(QFrame::Shadow::Raised);
        verticalLayout_3 = new QVBoxLayout(frame_stat);
        verticalLayout_3->setObjectName("verticalLayout_3");
        label_stat = new QLabel(frame_stat);
        label_stat->setObjectName("label_stat");
        label_stat->setStyleSheet(QString::fromUtf8("font-size: 24px; color: #3b9ddd; font-weight: bold; border: none; background: transparent;"));
        label_stat->setAlignment(Qt::AlignmentFlag::AlignCenter);

        verticalLayout_3->addWidget(label_stat);

        tabWidget->addTab(tab_statistique, QString());
        tab_smart_location = new QWidget();
        tab_smart_location->setObjectName("tab_smart_location");
        frame_smart_location = new QFrame(tab_smart_location);
        frame_smart_location->setObjectName("frame_smart_location");
        frame_smart_location->setGeometry(QRect(50, 50, 1031, 601));
        frame_smart_location->setStyleSheet(QString::fromUtf8("background-color: #212121;\n"
"border: 2px solid #3b9ddd;\n"
"border-radius: 10px;"));
        frame_smart_location->setFrameShape(QFrame::Shape::StyledPanel);
        frame_smart_location->setFrameShadow(QFrame::Shadow::Raised);
        verticalLayout_smart = new QVBoxLayout(frame_smart_location);
        verticalLayout_smart->setObjectName("verticalLayout_smart");
        label_smart_location = new QLabel(frame_smart_location);
        label_smart_location->setObjectName("label_smart_location");
        label_smart_location->setStyleSheet(QString::fromUtf8("font-size: 24px; color: #3b9ddd; font-weight: bold; border: none; background: transparent;"));
        label_smart_location->setAlignment(Qt::AlignmentFlag::AlignCenter);

        verticalLayout_smart->addWidget(label_smart_location);

        tabWidget->addTab(tab_smart_location, QString());
        tab_maintenance = new QWidget();
        tab_maintenance->setObjectName("tab_maintenance");
        frame_maintenance = new QFrame(tab_maintenance);
        frame_maintenance->setObjectName("frame_maintenance");
        frame_maintenance->setGeometry(QRect(50, 50, 1031, 601));
        frame_maintenance->setStyleSheet(QString::fromUtf8("background-color: #212121;\n"
"border: 2px solid #3b9ddd;\n"
"border-radius: 10px;"));
        frame_maintenance->setFrameShape(QFrame::Shape::StyledPanel);
        frame_maintenance->setFrameShadow(QFrame::Shadow::Raised);
        verticalLayout_maintenance = new QVBoxLayout(frame_maintenance);
        verticalLayout_maintenance->setObjectName("verticalLayout_maintenance");
        label_maintenance = new QLabel(frame_maintenance);
        label_maintenance->setObjectName("label_maintenance");
        label_maintenance->setStyleSheet(QString::fromUtf8("font-size: 24px; color: #3b9ddd; font-weight: bold; border: none; background: transparent;"));
        label_maintenance->setAlignment(Qt::AlignmentFlag::AlignCenter);

        verticalLayout_maintenance->addWidget(label_maintenance);

        tabWidget->addTab(tab_maintenance, QString());

        verticalLayout_4->addWidget(tabWidget);

        stackedWidget->addWidget(pageMaintenance);
        pageQuais = new QWidget();
        pageQuais->setObjectName("pageQuais");
        pageQuais->setStyleSheet(QString::fromUtf8("/* Quais module \342\200\223 professional, teal-themed */\n"
"#pageQuais QLabel { color: #ffffff; font-weight: bold; }\n"
"#pageQuais QLineEdit, \n"
"#pageQuais QComboBox, \n"
"#pageQuais QDateEdit, \n"
"#pageQuais QSpinBox, \n"
"#pageQuais QDoubleSpinBox, \n"
"#pageQuais QTimeEdit {\n"
"    background-color: #ffffff;\n"
"    color: #2b2b2b;\n"
"    border: 1px solid #b7d1db;\n"
"    border-radius: 6px;\n"
"    padding: 6px 8px;\n"
"    min-height: 4px;\n"
"    font-size: 13px;\n"
"}\n"
"#pageQuais QSpinBox, #pageQuais QDoubleSpinBox {\n"
"    min-width: 100px;\n"
"    padding-left: 8px;\n"
"}\n"
"#pageQuais QSpinBox::up-button, #pageQuais QSpinBox::down-button,\n"
"#pageQuais QDoubleSpinBox::up-button, #pageQuais QDoubleSpinBox::down-button {\n"
"    background-color: #e8e8e8;\n"
"    border: 1px solid #b7d1db;\n"
"    width: 18px;\n"
"}\n"
"#pageQuais QLineEdit:focus, #pageQuais QComboBox:focus, #pageQuais QSpinBox:focus, #pageQuais QDoubleSpinBox:focus, #pageQuais QDateEdit:focus { border: 1px solid #2a9d8f"
                        "; }\n"
"#pageQuais QHeaderView::section {\n"
"    background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #3a3a3a, stop:1 #252525);\n"
"    color: #ffffff;\n"
"    padding: 12px 14px;\n"
"    border: none;\n"
"    border-right: 1px solid #454545;\n"
"    border-bottom: 2px solid #353535;\n"
"    font-weight: bold;\n"
"    font-size: 12px;\n"
"}\n"
"#pageQuais QHeaderView::section:last { border-right: none; }\n"
"#pageQuais QTableWidget {\n"
"    border-radius: 10px;\n"
"    border: 2px solid #2a9d8f;\n"
"    background-color: #2b2b2b;\n"
"    gridline-color: #3d3d3d;\n"
"    padding: 2px;\n"
"}\n"
"#pageQuais QTableWidget::item { padding: 10px 12px; font-size: 12px; }\n"
"#pageQuais QTableWidget::item:alternate { background-color: #323232; }\n"
"#pageQuais QTableWidget::item:selected { background-color: #2a9d8f; color: #ffffff; }\n"
"#pageQuais QGroupBox {\n"
"    color: #2a9d8f;\n"
"    font-weight: bold;\n"
"    border: 2px solid #2a9d8f;\n"
"    border-radius: 8px;\n"
"    margin-top: 12px;\n"
"    paddi"
                        "ng: 16px;\n"
"    padding-top: 14px;\n"
"}\n"
"#pageQuais QGroupBox::title {\n"
"    subcontrol-origin: margin;\n"
"    left: 12px;\n"
"    padding: 0 8px;\n"
"    font-size: 13px;\n"
"}\n"
"/* Radio buttons \342\200\223 white text, light gray ring, blue fill when checked (same size as Pecheur Statut) */\n"
"#pageQuais QRadioButton {\n"
"    color: #ffffff;\n"
"    font-size: 12px;\n"
"    spacing: 6px;\n"
"}\n"
"#pageQuais QRadioButton::indicator {\n"
"    width: 14px;\n"
"    height: 14px;\n"
"    border-radius: 7px;\n"
"    border: 2px solid #b0b0b0;\n"
"    background-color: transparent;\n"
"}\n"
"#pageQuais QRadioButton::indicator:checked {\n"
"    background-color: #3598DB;\n"
"    border-color: #b0b0b0;\n"
"}\n"
"#pageQuais QRadioButton::indicator:hover {\n"
"    border-color: #d0d0d0;\n"
"}\n"
"/* Toolbar \342\200\223 compact, professional */\n"
"#pageQuais QPushButton {\n"
"    background-color: #2a9d8f;\n"
"    color: #ffffff;\n"
"    border: none;\n"
"    min-width: 72px;\n"
"    min-height: 28px;\n"
""
                        "    padding: 5px 10px;\n"
"    border-radius: 4px;\n"
"    font-weight: 600;\n"
"    font-size: 12px;\n"
"}\n"
"#pageQuais QPushButton:hover { background-color: #21867a; }\n"
"#pageQuais QPushButton:pressed { background-color: #1a6b60; }\n"
"#pageQuais QPushButton#btnAdd, #pageQuais QPushButton#btnUpdate { background-color: #3598DB; color: #000000; border: none; min-width: 78px; min-height: 28px; padding: 6px 12px; border-radius: 6px; font-size: 12px; font-weight: bold; }\n"
"#pageQuais QPushButton#btnAdd:hover, #pageQuais QPushButton#btnUpdate:hover { background-color: #2980b9; }\n"
"#pageQuais QPushButton#btnAdd:pressed, #pageQuais QPushButton#btnUpdate:pressed { background-color: #2471a3; }\n"
"#pageQuais QPushButton#btnDelete { background-color: #E74C3C; color: #FFFFFF; border: none; min-width: 78px; min-height: 28px; padding: 6px 12px; border-radius: 6px; font-size: 12px; font-weight: bold; }\n"
"#pageQuais QPushButton#btnDelete:hover { background-color: #c0392b; }\n"
"#pageQuais QPushButton#btnDelete:pre"
                        "ssed { background-color: #a93226; }\n"
"#pageQuais QPushButton#btnClear, #pageQuais QPushButton#btnSearch { background-color: #3598DB; color: #000000; border: none; min-width: 78px; min-height: 28px; padding: 6px 12px; border-radius: 6px; font-size: 12px; font-weight: bold; }\n"
"#pageQuais QPushButton#btnClear:hover, #pageQuais QPushButton#btnSearch:hover { background-color: #2980b9; }\n"
"#pageQuais QPushButton#btnClear:pressed, #pageQuais QPushButton#btnSearch:pressed { background-color: #2471a3; }\n"
"#pageQuais QPushButton#btnExport { background-color: #d68910; min-width: 78px; min-height: 28px; padding: 5px 10px; border-radius: 4px; font-size: 12px; }\n"
"#pageQuais QPushButton#btnExport:hover { background-color: #b9770e; }\n"
"#pageQuais QLineEdit#lineEditSearch { min-width: 140px; padding: 5px 10px; border-radius: 4px; font-size: 12px; }\n"
"#pageQuais #labelSmartSort { color: #1e5f74; font-size: 12px; font-weight: 600; }\n"
"#pageQuais #comboSmartSort { background-color: #2a2a2a; color: #ffffff; borde"
                        "r: 1px solid #3d3d3d; border-radius: 4px; padding: 5px 10px; padding-right: 22px; min-width: 100px; max-width: 160px; font-size: 12px; }\n"
"#pageQuais #comboSmartSort:hover { border-color: #2a9d8f; }\n"
"#pageQuais #comboSmartSort::drop-down { subcontrol-origin: padding; subcontrol-position: right center; width: 20px; border: none; border-left: 1px solid #3d3d3d; background-color: #252525; border-radius: 0 3px 3px 0; }\n"
"/* Supervision / Statistiques tables and cards */\n"
"#pageQuais #tableOccupationByType { border-radius: 8px; border: 1px solid #3d3d3d; }\n"
"            "));
        pageQuaisLayout = new QVBoxLayout(pageQuais);
        pageQuaisLayout->setObjectName("pageQuaisLayout");
        pageQuaisLayout->setContentsMargins(30, 30, 30, 30);
        contentFrame = new QFrame(pageQuais);
        contentFrame->setObjectName("contentFrame");
        contentLayout = new QVBoxLayout(contentFrame);
        contentLayout->setSpacing(0);
        contentLayout->setObjectName("contentLayout");
        contentLayout->setContentsMargins(0, 0, 0, 0);
        frame_Quais_Header = new QFrame(contentFrame);
        frame_Quais_Header->setObjectName("frame_Quais_Header");
        frame_Quais_Header->setMinimumSize(QSize(0, 72));
        frame_Quais_Header->setMaximumSize(QSize(16777215, 72));
        frame_Quais_Header->setStyleSheet(QString::fromUtf8("#frame_Quais_Header {\n"
"    background-color: #2F5C6A;\n"
"    border: none;\n"
"    border-bottom: 3px solid #44B9AC;\n"
"}\n"
"#frame_Quais_Header_Overlay {\n"
"    background-color: transparent;\n"
"}\n"
"#title_label_quais {\n"
"    color: #ffffff;\n"
"    font-size: 22px;\n"
"    font-weight: bold;\n"
"    font-family: \"Segoe UI\", sans-serif;\n"
"    letter-spacing: 0.5px;\n"
"}"));
        frame_Quais_Header->setFrameShape(QFrame::Shape::NoFrame);
        verticalLayout_Header_Quais = new QVBoxLayout(frame_Quais_Header);
        verticalLayout_Header_Quais->setSpacing(0);
        verticalLayout_Header_Quais->setObjectName("verticalLayout_Header_Quais");
        verticalLayout_Header_Quais->setContentsMargins(0, 0, 0, 0);
        frame_Quais_Header_Overlay = new QFrame(frame_Quais_Header);
        frame_Quais_Header_Overlay->setObjectName("frame_Quais_Header_Overlay");
        verticalLayout_Title_Quais = new QVBoxLayout(frame_Quais_Header_Overlay);
        verticalLayout_Title_Quais->setObjectName("verticalLayout_Title_Quais");
        title_label_quais = new QLabel(frame_Quais_Header_Overlay);
        title_label_quais->setObjectName("title_label_quais");
        title_label_quais->setStyleSheet(QString::fromUtf8("color: #ffffff; font-size: 22px; font-weight: bold; font-family: \"Segoe UI\", sans-serif; letter-spacing: 0.5px;"));
        title_label_quais->setAlignment(Qt::AlignmentFlag::AlignCenter);

        verticalLayout_Title_Quais->addWidget(title_label_quais);


        verticalLayout_Header_Quais->addWidget(frame_Quais_Header_Overlay);


        contentLayout->addWidget(frame_Quais_Header);

        mainTabWidget = new QTabWidget(contentFrame);
        mainTabWidget->setObjectName("mainTabWidget");
        mainTabWidget->setStyleSheet(QString::fromUtf8("/* Quais main tabs \342\200\223 professional, like Employ\303\251s */\n"
"QTabWidget::pane { border: 2px solid #2a9d8f; background-color: #e5ebf0; border-radius: 8px; margin-top: -1px; padding: 14px; }\n"
"QTabBar::tab { background-color: #456b77; color: #ffffff; padding: 12px 22px; margin-right: 2px; border-top-left-radius: 6px; border-top-right-radius: 6px; font-weight: 600; font-size: 13px; border-left: 3px solid transparent; }\n"
"QTabBar::tab:selected { background-color: #2a9d8f; border-left: 3px solid #14ffec; }\n"
"QTabBar::tab:hover:!selected { background-color: #3e7a9e; }\n"
"/* Tables inside */\n"
"QTableWidget { background-color: #2b2b2b; color: #ffffff; gridline-color: #444; border: 1px solid #2a9d8f; border-radius: 8px; }\n"
"                "));
        mainTabWidget->setDocumentMode(false);
        tabCRUD = new QWidget();
        tabCRUD->setObjectName("tabCRUD");
        verticalLayout_5 = new QVBoxLayout(tabCRUD);
        verticalLayout_5->setObjectName("verticalLayout_5");
        frame_Quais_SectionTitle = new QFrame(tabCRUD);
        frame_Quais_SectionTitle->setObjectName("frame_Quais_SectionTitle");
        frame_Quais_SectionTitle->setMinimumSize(QSize(0, 44));
        frame_Quais_SectionTitle->setMaximumSize(QSize(16777215, 44));
        frame_Quais_SectionTitle->setStyleSheet(QString::fromUtf8("#frame_Quais_SectionTitle { background-color: #1e5f74; border-radius: 6px; border: none; }\n"
"#frame_Quais_SectionTitle QLabel { color: #ffffff; font-size: 15px; font-weight: bold; }"));
        frame_Quais_SectionTitle->setFrameShape(QFrame::Shape::NoFrame);
        sectionTitleLayout = new QHBoxLayout(frame_Quais_SectionTitle);
        sectionTitleLayout->setObjectName("sectionTitleLayout");
        sectionTitleLayout->setContentsMargins(16, 0, 16, 0);
        label_Quais_SectionTitle = new QLabel(frame_Quais_SectionTitle);
        label_Quais_SectionTitle->setObjectName("label_Quais_SectionTitle");

        sectionTitleLayout->addWidget(label_Quais_SectionTitle);


        verticalLayout_5->addWidget(frame_Quais_SectionTitle);

        formAndPhotoLayout = new QHBoxLayout();
        formAndPhotoLayout->setObjectName("formAndPhotoLayout");
        groupBoxQuaiInfo = new QGroupBox(tabCRUD);
        groupBoxQuaiInfo->setObjectName("groupBoxQuaiInfo");
        groupBoxQuaiInfo->setStyleSheet(QString::fromUtf8("QGroupBox {\n"
"    background-color: #262626;\n"
"    color: #ffffff;\n"
"    padding: 18px;\n"
"    border-radius: 8px;\n"
"    border: 1px solid #3d3d3d;\n"
"}\n"
"QGroupBox QRadioButton {\n"
"    color: #ffffff;\n"
"    font-size: 12px;\n"
"    spacing: 6px;\n"
"}\n"
"QGroupBox QRadioButton::indicator {\n"
"    width: 14px;\n"
"    height: 14px;\n"
"    border-radius: 7px;\n"
"    border: 2px solid #b0b0b0;\n"
"    background-color: transparent;\n"
"}\n"
"QGroupBox QRadioButton::indicator:checked {\n"
"    background-color: #3598DB;\n"
"    border-color: #b0b0b0;\n"
"}\n"
"QGroupBox QRadioButton::indicator:hover {\n"
"    border-color: #d0d0d0;\n"
"}\n"
"QGroupBox::title {\n"
"    subcontrol-origin: margin;\n"
"    left: 20px;\n"
"    padding: 5px 14px;\n"
"    color: #ffffff;\n"
"    background-color: #3498db;\n"
"    border-radius: 6px;\n"
"    font-weight: bold;\n"
"    font-size: 13px;\n"
"}"));
        gridLayoutQuaiInfo = new QGridLayout(groupBoxQuaiInfo);
        gridLayoutQuaiInfo->setObjectName("gridLayoutQuaiInfo");
        gridLayoutQuaiInfo->setHorizontalSpacing(20);
        gridLayoutQuaiInfo->setVerticalSpacing(12);
        gridLayoutQuaiInfo->setContentsMargins(15, 15, 15, 15);
        statutRadioContainer = new QWidget(groupBoxQuaiInfo);
        statutRadioContainer->setObjectName("statutRadioContainer");
        statutRadioLayout = new QHBoxLayout(statutRadioContainer);
        statutRadioLayout->setSpacing(8);
        statutRadioLayout->setObjectName("statutRadioLayout");
        statutRadioLayout->setContentsMargins(0, 0, 0, 0);
        radioStatutActif = new QRadioButton(statutRadioContainer);
        radioStatutActif->setObjectName("radioStatutActif");
        radioStatutActif->setChecked(true);

        statutRadioLayout->addWidget(radioStatutActif);

        radioStatutInactif = new QRadioButton(statutRadioContainer);
        radioStatutInactif->setObjectName("radioStatutInactif");

        statutRadioLayout->addWidget(radioStatutInactif);

        radioStatutMaintenance = new QRadioButton(statutRadioContainer);
        radioStatutMaintenance->setObjectName("radioStatutMaintenance");

        statutRadioLayout->addWidget(radioStatutMaintenance);


        gridLayoutQuaiInfo->addWidget(statutRadioContainer, 4, 3, 1, 1);

        doubleSpinBoxOccupation = new QDoubleSpinBox(groupBoxQuaiInfo);
        doubleSpinBoxOccupation->setObjectName("doubleSpinBoxOccupation");
        doubleSpinBoxOccupation->setMinimumSize(QSize(118, 18));
        doubleSpinBoxOccupation->setMaximum(100.000000000000000);

        gridLayoutQuaiInfo->addWidget(doubleSpinBoxOccupation, 1, 3, 1, 1);

        labelCapacite = new QLabel(groupBoxQuaiInfo);
        labelCapacite->setObjectName("labelCapacite");

        gridLayoutQuaiInfo->addWidget(labelCapacite, 4, 0, 1, 1);

        labelType = new QLabel(groupBoxQuaiInfo);
        labelType->setObjectName("labelType");

        gridLayoutQuaiInfo->addWidget(labelType, 2, 0, 1, 1);

        doubleSpinBoxLongueur = new QDoubleSpinBox(groupBoxQuaiInfo);
        doubleSpinBoxLongueur->setObjectName("doubleSpinBoxLongueur");
        doubleSpinBoxLongueur->setMinimumSize(QSize(118, 18));
        doubleSpinBoxLongueur->setMaximum(500.000000000000000);

        gridLayoutQuaiInfo->addWidget(doubleSpinBoxLongueur, 3, 1, 1, 1);

        eclairageRadioContainer = new QWidget(groupBoxQuaiInfo);
        eclairageRadioContainer->setObjectName("eclairageRadioContainer");
        eclairageRadioLayout = new QHBoxLayout(eclairageRadioContainer);
        eclairageRadioLayout->setSpacing(8);
        eclairageRadioLayout->setObjectName("eclairageRadioLayout");
        eclairageRadioLayout->setContentsMargins(0, 0, 0, 0);
        radioEclairageAllume = new QRadioButton(eclairageRadioContainer);
        radioEclairageAllume->setObjectName("radioEclairageAllume");
        radioEclairageAllume->setChecked(true);

        eclairageRadioLayout->addWidget(radioEclairageAllume);

        radioEclairageEteint = new QRadioButton(eclairageRadioContainer);
        radioEclairageEteint->setObjectName("radioEclairageEteint");

        eclairageRadioLayout->addWidget(radioEclairageEteint);

        radioEclairageDefaillant = new QRadioButton(eclairageRadioContainer);
        radioEclairageDefaillant->setObjectName("radioEclairageDefaillant");

        eclairageRadioLayout->addWidget(radioEclairageDefaillant);


        gridLayoutQuaiInfo->addWidget(eclairageRadioContainer, 6, 3, 1, 1);

        labelLongueur = new QLabel(groupBoxQuaiInfo);
        labelLongueur->setObjectName("labelLongueur");

        gridLayoutQuaiInfo->addWidget(labelLongueur, 3, 0, 1, 1);

        labelCurrentUsage = new QLabel(groupBoxQuaiInfo);
        labelCurrentUsage->setObjectName("labelCurrentUsage");

        gridLayoutQuaiInfo->addWidget(labelCurrentUsage, 2, 2, 1, 1);

        prioriteRadioContainer = new QWidget(groupBoxQuaiInfo);
        prioriteRadioContainer->setObjectName("prioriteRadioContainer");
        prioriteRadioLayout = new QHBoxLayout(prioriteRadioContainer);
        prioriteRadioLayout->setSpacing(12);
        prioriteRadioLayout->setObjectName("prioriteRadioLayout");
        prioriteRadioLayout->setContentsMargins(0, 0, 0, 0);
        radioPrioriteNormale = new QRadioButton(prioriteRadioContainer);
        radioPrioriteNormale->setObjectName("radioPrioriteNormale");

        prioriteRadioLayout->addWidget(radioPrioriteNormale);

        radioPrioriteHaute = new QRadioButton(prioriteRadioContainer);
        radioPrioriteHaute->setObjectName("radioPrioriteHaute");

        prioriteRadioLayout->addWidget(radioPrioriteHaute);

        radioPrioriteBasse = new QRadioButton(prioriteRadioContainer);
        radioPrioriteBasse->setObjectName("radioPrioriteBasse");
        radioPrioriteBasse->setChecked(true);

        prioriteRadioLayout->addWidget(radioPrioriteBasse);


        gridLayoutQuaiInfo->addWidget(prioriteRadioContainer, 0, 3, 1, 1);

        spinBoxCurrentUsage = new QSpinBox(groupBoxQuaiInfo);
        spinBoxCurrentUsage->setObjectName("spinBoxCurrentUsage");
        spinBoxCurrentUsage->setMinimumSize(QSize(118, 18));
        spinBoxCurrentUsage->setMinimum(0);
        spinBoxCurrentUsage->setMaximum(9999);

        gridLayoutQuaiInfo->addWidget(spinBoxCurrentUsage, 2, 3, 1, 1);

        spinBoxCapacite = new QSpinBox(groupBoxQuaiInfo);
        spinBoxCapacite->setObjectName("spinBoxCapacite");
        spinBoxCapacite->setMinimumSize(QSize(118, 18));
        spinBoxCapacite->setMaximum(9999);

        gridLayoutQuaiInfo->addWidget(spinBoxCapacite, 4, 1, 2, 1);

        labelNom = new QLabel(groupBoxQuaiInfo);
        labelNom->setObjectName("labelNom");

        gridLayoutQuaiInfo->addWidget(labelNom, 1, 0, 1, 1);

        labelSecurite = new QLabel(groupBoxQuaiInfo);
        labelSecurite->setObjectName("labelSecurite");

        gridLayoutQuaiInfo->addWidget(labelSecurite, 6, 0, 1, 1);

        labelOccupation = new QLabel(groupBoxQuaiInfo);
        labelOccupation->setObjectName("labelOccupation");

        gridLayoutQuaiInfo->addWidget(labelOccupation, 1, 2, 1, 1);

        labelQuaiID = new QLabel(groupBoxQuaiInfo);
        labelQuaiID->setObjectName("labelQuaiID");

        gridLayoutQuaiInfo->addWidget(labelQuaiID, 0, 0, 1, 1);

        typeRadioContainer = new QWidget(groupBoxQuaiInfo);
        typeRadioContainer->setObjectName("typeRadioContainer");
        typeRadioLayout = new QHBoxLayout(typeRadioContainer);
        typeRadioLayout->setSpacing(8);
        typeRadioLayout->setObjectName("typeRadioLayout");
        typeRadioLayout->setContentsMargins(0, 0, 0, 0);
        radioTypePeche = new QRadioButton(typeRadioContainer);
        radioTypePeche->setObjectName("radioTypePeche");
        radioTypePeche->setChecked(true);

        typeRadioLayout->addWidget(radioTypePeche);

        radioTypeCommerce = new QRadioButton(typeRadioContainer);
        radioTypeCommerce->setObjectName("radioTypeCommerce");

        typeRadioLayout->addWidget(radioTypeCommerce);

        radioTypeMaintenance = new QRadioButton(typeRadioContainer);
        radioTypeMaintenance->setObjectName("radioTypeMaintenance");

        typeRadioLayout->addWidget(radioTypeMaintenance);

        radioTypeMixte = new QRadioButton(typeRadioContainer);
        radioTypeMixte->setObjectName("radioTypeMixte");

        typeRadioLayout->addWidget(radioTypeMixte);


        gridLayoutQuaiInfo->addWidget(typeRadioContainer, 2, 1, 1, 1);

        labelProfondeur = new QLabel(groupBoxQuaiInfo);
        labelProfondeur->setObjectName("labelProfondeur");

        gridLayoutQuaiInfo->addWidget(labelProfondeur, 3, 2, 1, 1);

        labelPriorite = new QLabel(groupBoxQuaiInfo);
        labelPriorite->setObjectName("labelPriorite");

        gridLayoutQuaiInfo->addWidget(labelPriorite, 0, 2, 1, 1);

        labelLightingStatus = new QLabel(groupBoxQuaiInfo);
        labelLightingStatus->setObjectName("labelLightingStatus");

        gridLayoutQuaiInfo->addWidget(labelLightingStatus, 6, 2, 1, 1);

        lineEditNom = new QLineEdit(groupBoxQuaiInfo);
        lineEditNom->setObjectName("lineEditNom");
        lineEditNom->setMinimumSize(QSize(160, 18));

        gridLayoutQuaiInfo->addWidget(lineEditNom, 1, 1, 1, 1);

        securiteRadioContainer = new QWidget(groupBoxQuaiInfo);
        securiteRadioContainer->setObjectName("securiteRadioContainer");
        securiteRadioLayout = new QHBoxLayout(securiteRadioContainer);
        securiteRadioLayout->setSpacing(12);
        securiteRadioLayout->setObjectName("securiteRadioLayout");
        securiteRadioLayout->setContentsMargins(0, 0, 0, 0);
        radioSecurite1 = new QRadioButton(securiteRadioContainer);
        radioSecurite1->setObjectName("radioSecurite1");
        radioSecurite1->setChecked(true);

        securiteRadioLayout->addWidget(radioSecurite1);

        radioSecurite2 = new QRadioButton(securiteRadioContainer);
        radioSecurite2->setObjectName("radioSecurite2");

        securiteRadioLayout->addWidget(radioSecurite2);

        radioSecurite3 = new QRadioButton(securiteRadioContainer);
        radioSecurite3->setObjectName("radioSecurite3");

        securiteRadioLayout->addWidget(radioSecurite3);


        gridLayoutQuaiInfo->addWidget(securiteRadioContainer, 6, 1, 1, 1);

        labelStatut = new QLabel(groupBoxQuaiInfo);
        labelStatut->setObjectName("labelStatut");

        gridLayoutQuaiInfo->addWidget(labelStatut, 4, 2, 1, 1);

        lineEditQuaiID = new QLineEdit(groupBoxQuaiInfo);
        lineEditQuaiID->setObjectName("lineEditQuaiID");
        lineEditQuaiID->setMinimumSize(QSize(160, 18));
        lineEditQuaiID->setReadOnly(true);

        gridLayoutQuaiInfo->addWidget(lineEditQuaiID, 0, 1, 1, 1);

        doubleSpinBoxProfondeur = new QDoubleSpinBox(groupBoxQuaiInfo);
        doubleSpinBoxProfondeur->setObjectName("doubleSpinBoxProfondeur");
        doubleSpinBoxProfondeur->setMinimumSize(QSize(118, 18));
        doubleSpinBoxProfondeur->setMaximum(50.000000000000000);

        gridLayoutQuaiInfo->addWidget(doubleSpinBoxProfondeur, 3, 3, 1, 1);


        formAndPhotoLayout->addWidget(groupBoxQuaiInfo);


        verticalLayout_5->addLayout(formAndPhotoLayout);

        frameQuaiToolbar = new QFrame(tabCRUD);
        frameQuaiToolbar->setObjectName("frameQuaiToolbar");
        frameQuaiToolbar->setMinimumSize(QSize(0, 50));
        frameQuaiToolbar->setStyleSheet(QString::fromUtf8("QFrame#frameQuaiToolbar { background-color: #34495e; border-radius: 8px; padding: 8px; border: 1px solid #2c3e50; }"));
        crudButtonLayout = new QHBoxLayout(frameQuaiToolbar);
        crudButtonLayout->setSpacing(10);
        crudButtonLayout->setObjectName("crudButtonLayout");
        btnAdd = new QPushButton(frameQuaiToolbar);
        btnAdd->setObjectName("btnAdd");
        QSizePolicy sizePolicy1(QSizePolicy::Policy::Fixed, QSizePolicy::Policy::Fixed);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(btnAdd->sizePolicy().hasHeightForWidth());
        btnAdd->setSizePolicy(sizePolicy1);
        btnAdd->setStyleSheet(QString::fromUtf8("QPushButton { background-color: #3598DB; color: #000000; border: none; border-radius: 6px; font-weight: bold; padding: 6px 12px; }\n"
"QPushButton:hover { background-color: #2980b9; }\n"
"QPushButton:pressed { background-color: #2471a3; }"));
        btnAdd->setIcon(icon);
        btnAdd->setIconSize(QSize(16, 16));

        crudButtonLayout->addWidget(btnAdd);

        btnUpdate = new QPushButton(frameQuaiToolbar);
        btnUpdate->setObjectName("btnUpdate");
        sizePolicy1.setHeightForWidth(btnUpdate->sizePolicy().hasHeightForWidth());
        btnUpdate->setSizePolicy(sizePolicy1);
        btnUpdate->setStyleSheet(QString::fromUtf8("QPushButton { background-color: #3598DB; color: #000000; border: none; border-radius: 6px; font-weight: bold; padding: 6px 12px; }\n"
"QPushButton:hover { background-color: #2980b9; }\n"
"QPushButton:pressed { background-color: #2471a3; }"));
        btnUpdate->setIcon(icon2);
        btnUpdate->setIconSize(QSize(16, 16));

        crudButtonLayout->addWidget(btnUpdate);

        btnDelete = new QPushButton(frameQuaiToolbar);
        btnDelete->setObjectName("btnDelete");
        sizePolicy1.setHeightForWidth(btnDelete->sizePolicy().hasHeightForWidth());
        btnDelete->setSizePolicy(sizePolicy1);
        btnDelete->setStyleSheet(QString::fromUtf8("QPushButton { background-color: #E74C3C; color: #FFFFFF; border: none; border-radius: 6px; font-weight: bold; padding: 6px 12px; }\n"
"QPushButton:hover { background-color: #c0392b; }\n"
"QPushButton:pressed { background-color: #a93226; }"));
        btnDelete->setIcon(icon3);
        btnDelete->setIconSize(QSize(16, 16));

        crudButtonLayout->addWidget(btnDelete);

        btnClear = new QPushButton(frameQuaiToolbar);
        btnClear->setObjectName("btnClear");
        sizePolicy1.setHeightForWidth(btnClear->sizePolicy().hasHeightForWidth());
        btnClear->setSizePolicy(sizePolicy1);
        btnClear->setStyleSheet(QString::fromUtf8("QPushButton { background-color: #3598DB; color: #000000; border: none; border-radius: 6px; font-weight: bold; padding: 6px 12px; }\n"
"QPushButton:hover { background-color: #2980b9; }\n"
"QPushButton:pressed { background-color: #2471a3; }"));
        btnClear->setIcon(icon1);
        btnClear->setIconSize(QSize(16, 16));

        crudButtonLayout->addWidget(btnClear);

        spacerBeforeFilter = new QSpacerItem(24, 20, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);

        crudButtonLayout->addItem(spacerBeforeFilter);

        labelFilterBy = new QLabel(frameQuaiToolbar);
        labelFilterBy->setObjectName("labelFilterBy");
        labelFilterBy->setStyleSheet(QString::fromUtf8("color: #1e5f74; font-weight: 600; font-size: 11px;"));

        crudButtonLayout->addWidget(labelFilterBy);

        filterComboType = new QComboBox(frameQuaiToolbar);
        filterComboType->addItem(QString());
        filterComboType->addItem(QString());
        filterComboType->addItem(QString());
        filterComboType->addItem(QString());
        filterComboType->addItem(QString());
        filterComboType->setObjectName("filterComboType");
        sizePolicy1.setHeightForWidth(filterComboType->sizePolicy().hasHeightForWidth());
        filterComboType->setSizePolicy(sizePolicy1);
        filterComboType->setMinimumSize(QSize(108, 14));
        filterComboType->setStyleSheet(QString::fromUtf8("QComboBox { background-color: #2d2d2d; color: #e0e0e0; border: 1px solid #3d3d3d; border-radius: 4px; padding: 4px 8px; min-width: 90px; }\n"
"QComboBox:focus { border-color: #2a9d8f; }\n"
"QComboBox:hover { border-color: #2a9d8f; }"));

        crudButtonLayout->addWidget(filterComboType);

        filterComboStatut = new QComboBox(frameQuaiToolbar);
        filterComboStatut->addItem(QString());
        filterComboStatut->addItem(QString());
        filterComboStatut->addItem(QString());
        filterComboStatut->setObjectName("filterComboStatut");
        sizePolicy1.setHeightForWidth(filterComboStatut->sizePolicy().hasHeightForWidth());
        filterComboStatut->setSizePolicy(sizePolicy1);
        filterComboStatut->setMinimumSize(QSize(98, 14));
        filterComboStatut->setStyleSheet(QString::fromUtf8("QComboBox { background-color: #2d2d2d; color: #e0e0e0; border: 1px solid #3d3d3d; border-radius: 4px; padding: 4px 8px; min-width: 80px; }\n"
"QComboBox:focus { border-color: #2a9d8f; }\n"
"QComboBox:hover { border-color: #2a9d8f; }"));

        crudButtonLayout->addWidget(filterComboStatut);

        filterComboSecurite = new QComboBox(frameQuaiToolbar);
        filterComboSecurite->addItem(QString());
        filterComboSecurite->addItem(QString());
        filterComboSecurite->addItem(QString());
        filterComboSecurite->addItem(QString());
        filterComboSecurite->setObjectName("filterComboSecurite");
        sizePolicy1.setHeightForWidth(filterComboSecurite->sizePolicy().hasHeightForWidth());
        filterComboSecurite->setSizePolicy(sizePolicy1);
        filterComboSecurite->setMinimumSize(QSize(103, 14));
        filterComboSecurite->setStyleSheet(QString::fromUtf8("QComboBox { background-color: #2d2d2d; color: #e0e0e0; border: 1px solid #3d3d3d; border-radius: 4px; padding: 4px 8px; min-width: 85px; }\n"
"QComboBox:focus { border-color: #2a9d8f; }\n"
"QComboBox:hover { border-color: #2a9d8f; }"));

        crudButtonLayout->addWidget(filterComboSecurite);

        horizontalSpacer_2 = new QSpacerItem(20, 20, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);

        crudButtonLayout->addItem(horizontalSpacer_2);

        lineEditSearch = new QLineEdit(frameQuaiToolbar);
        lineEditSearch->setObjectName("lineEditSearch");
        lineEditSearch->setMinimumSize(QSize(120, 0));
        lineEditSearch->setMaximumSize(QSize(320, 16777215));
        QSizePolicy sizePolicy2(QSizePolicy::Policy::Maximum, QSizePolicy::Policy::Fixed);
        sizePolicy2.setHorizontalStretch(0);
        sizePolicy2.setVerticalStretch(0);
        sizePolicy2.setHeightForWidth(lineEditSearch->sizePolicy().hasHeightForWidth());
        lineEditSearch->setSizePolicy(sizePolicy2);
        lineEditSearch->setStyleSheet(QString::fromUtf8("QLineEdit { color: #e0e0e0; background-color: #2d2d2d; border: 1px solid #3d3d3d; border-radius: 4px; padding: 5px 10px; font-size: 12px; }\n"
"QLineEdit:focus { border-color: #2a9d8f; }"));

        crudButtonLayout->addWidget(lineEditSearch);

        btnSearch = new QPushButton(frameQuaiToolbar);
        btnSearch->setObjectName("btnSearch");
        sizePolicy1.setHeightForWidth(btnSearch->sizePolicy().hasHeightForWidth());
        btnSearch->setSizePolicy(sizePolicy1);
        btnSearch->setStyleSheet(QString::fromUtf8("QPushButton { background-color: #3598DB; color: #000000; border: none; border-radius: 6px; font-weight: bold; padding: 6px 12px; }\n"
"QPushButton:hover { background-color: #2980b9; }\n"
"QPushButton:pressed { background-color: #2471a3; }"));
        btnSearch->setIcon(icon4);
        btnSearch->setIconSize(QSize(16, 16));

        crudButtonLayout->addWidget(btnSearch);

        spacerBeforeSort = new QSpacerItem(24, 20, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);

        crudButtonLayout->addItem(spacerBeforeSort);

        labelSmartSort = new QLabel(frameQuaiToolbar);
        labelSmartSort->setObjectName("labelSmartSort");

        crudButtonLayout->addWidget(labelSmartSort);

        comboSmartSort = new QComboBox(frameQuaiToolbar);
        comboSmartSort->addItem(QString());
        comboSmartSort->addItem(QString());
        comboSmartSort->addItem(QString());
        comboSmartSort->addItem(QString());
        comboSmartSort->setObjectName("comboSmartSort");
        sizePolicy1.setHeightForWidth(comboSmartSort->sizePolicy().hasHeightForWidth());
        comboSmartSort->setSizePolicy(sizePolicy1);

        crudButtonLayout->addWidget(comboSmartSort);

        btnExport = new QPushButton(frameQuaiToolbar);
        btnExport->setObjectName("btnExport");
        sizePolicy1.setHeightForWidth(btnExport->sizePolicy().hasHeightForWidth());
        btnExport->setSizePolicy(sizePolicy1);
        btnExport->setStyleSheet(QString::fromUtf8("QPushButton { background-color: #d68910; color: white; border: none; }\n"
"QPushButton:hover { background-color: #b9770e; }\n"
"QPushButton:pressed { background-color: #9c650c; }"));

        crudButtonLayout->addWidget(btnExport);


        verticalLayout_5->addWidget(frameQuaiToolbar);

        tableQuais = new QTableWidget(tabCRUD);
        if (tableQuais->columnCount() < 12)
            tableQuais->setColumnCount(12);
        QTableWidgetItem *__qtablewidgetitem81 = new QTableWidgetItem();
        tableQuais->setHorizontalHeaderItem(0, __qtablewidgetitem81);
        QTableWidgetItem *__qtablewidgetitem82 = new QTableWidgetItem();
        tableQuais->setHorizontalHeaderItem(1, __qtablewidgetitem82);
        QTableWidgetItem *__qtablewidgetitem83 = new QTableWidgetItem();
        tableQuais->setHorizontalHeaderItem(2, __qtablewidgetitem83);
        QTableWidgetItem *__qtablewidgetitem84 = new QTableWidgetItem();
        tableQuais->setHorizontalHeaderItem(3, __qtablewidgetitem84);
        QTableWidgetItem *__qtablewidgetitem85 = new QTableWidgetItem();
        tableQuais->setHorizontalHeaderItem(4, __qtablewidgetitem85);
        QTableWidgetItem *__qtablewidgetitem86 = new QTableWidgetItem();
        tableQuais->setHorizontalHeaderItem(5, __qtablewidgetitem86);
        QTableWidgetItem *__qtablewidgetitem87 = new QTableWidgetItem();
        tableQuais->setHorizontalHeaderItem(6, __qtablewidgetitem87);
        QTableWidgetItem *__qtablewidgetitem88 = new QTableWidgetItem();
        tableQuais->setHorizontalHeaderItem(7, __qtablewidgetitem88);
        QTableWidgetItem *__qtablewidgetitem89 = new QTableWidgetItem();
        tableQuais->setHorizontalHeaderItem(8, __qtablewidgetitem89);
        QTableWidgetItem *__qtablewidgetitem90 = new QTableWidgetItem();
        tableQuais->setHorizontalHeaderItem(9, __qtablewidgetitem90);
        QTableWidgetItem *__qtablewidgetitem91 = new QTableWidgetItem();
        tableQuais->setHorizontalHeaderItem(10, __qtablewidgetitem91);
        QTableWidgetItem *__qtablewidgetitem92 = new QTableWidgetItem();
        tableQuais->setHorizontalHeaderItem(11, __qtablewidgetitem92);
        tableQuais->setObjectName("tableQuais");
        tableQuais->setStyleSheet(QString::fromUtf8("QTableWidget {\n"
"    background-color: #2b2b2b;\n"
"    gridline-color: #3d3d3d;\n"
"    border-radius: 10px;\n"
"    border: 2px solid #2a9d8f;\n"
"    color: #ffffff;\n"
"    font-size: 12px;\n"
"}\n"
"QHeaderView::section {\n"
"    background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #3a3a3a, stop:1 #252525);\n"
"    color: #ffffff;\n"
"    padding: 10px 16px;\n"
"    border: none;\n"
"    border-right: 1px solid #454545;\n"
"    font-weight: bold;\n"
"    min-width: 72px;\n"
"}\n"
"QTableWidget::item { padding: 10px 12px; }\n"
"QTableWidget::item:alternate { background-color: #323232; }\n"
"QTableWidget::item:selected { background-color: #2a9d8f; }"));
        tableQuais->setAlternatingRowColors(true);
        tableQuais->setSelectionBehavior(QAbstractItemView::SelectionBehavior::SelectRows);
        tableQuais->horizontalHeader()->setCascadingSectionResizes(false);
        tableQuais->horizontalHeader()->setMinimumSectionSize(72);
        tableQuais->horizontalHeader()->setDefaultSectionSize(100);
        tableQuais->horizontalHeader()->setStretchLastSection(true);

        verticalLayout_5->addWidget(tableQuais);

        mainTabWidget->addTab(tabCRUD, QString());
        tabSupervision = new QWidget();
        tabSupervision->setObjectName("tabSupervision");
        supervisionLayout = new QVBoxLayout(tabSupervision);
        supervisionLayout->setSpacing(24);
        supervisionLayout->setObjectName("supervisionLayout");
        supervisionLayout->setContentsMargins(8, 8, 8, 8);
        frame_Supervision_SectionTitle = new QFrame(tabSupervision);
        frame_Supervision_SectionTitle->setObjectName("frame_Supervision_SectionTitle");
        frame_Supervision_SectionTitle->setMinimumSize(QSize(0, 44));
        frame_Supervision_SectionTitle->setMaximumSize(QSize(16777215, 44));
        frame_Supervision_SectionTitle->setStyleSheet(QString::fromUtf8("#frame_Supervision_SectionTitle { background-color: #1e5f74; border-radius: 6px; border: none; }\n"
"#frame_Supervision_SectionTitle QLabel { color: #ffffff; font-size: 15px; font-weight: bold; }"));
        frame_Supervision_SectionTitle->setFrameShape(QFrame::Shape::NoFrame);
        supervisionSectionTitleLayout = new QHBoxLayout(frame_Supervision_SectionTitle);
        supervisionSectionTitleLayout->setObjectName("supervisionSectionTitleLayout");
        supervisionSectionTitleLayout->setContentsMargins(16, -1, -1, -1);
        label_Supervision_SectionTitle = new QLabel(frame_Supervision_SectionTitle);
        label_Supervision_SectionTitle->setObjectName("label_Supervision_SectionTitle");

        supervisionSectionTitleLayout->addWidget(label_Supervision_SectionTitle);


        supervisionLayout->addWidget(frame_Supervision_SectionTitle);

        kpiCardsLayout = new QHBoxLayout();
        kpiCardsLayout->setSpacing(20);
        kpiCardsLayout->setObjectName("kpiCardsLayout");
        kpiCardTotal = new QFrame(tabSupervision);
        kpiCardTotal->setObjectName("kpiCardTotal");
        kpiCardTotal->setMinimumSize(QSize(180, 100));
        kpiCardTotal->setStyleSheet(QString::fromUtf8("#kpiCardTotal { background: qlineargradient(x1:0,y1:0,x2:0,y2:1, stop:0 #3498db, stop:1 #2980b9); border-radius: 10px; border: 2px solid #2a9d8f; }\n"
"#kpiCardTotal QLabel { color: #ffffff; }"));
        kpiTotalLayout = new QVBoxLayout(kpiCardTotal);
        kpiTotalLayout->setObjectName("kpiTotalLayout");
        lblKpiTotalTitle = new QLabel(kpiCardTotal);
        lblKpiTotalTitle->setObjectName("lblKpiTotalTitle");
        lblKpiTotalTitle->setStyleSheet(QString::fromUtf8("font-size: 12px; font-weight: bold;"));

        kpiTotalLayout->addWidget(lblKpiTotalTitle);

        lblKpiTotalValue = new QLabel(kpiCardTotal);
        lblKpiTotalValue->setObjectName("lblKpiTotalValue");
        lblKpiTotalValue->setStyleSheet(QString::fromUtf8("font-size: 28px; font-weight: bold;"));

        kpiTotalLayout->addWidget(lblKpiTotalValue);


        kpiCardsLayout->addWidget(kpiCardTotal);

        kpiCardLibres = new QFrame(tabSupervision);
        kpiCardLibres->setObjectName("kpiCardLibres");
        kpiCardLibres->setMinimumSize(QSize(180, 100));
        kpiCardLibres->setStyleSheet(QString::fromUtf8("#kpiCardLibres { background: qlineargradient(x1:0,y1:0,x2:0,y2:1, stop:0 #2ecc71, stop:1 #27ae60); border-radius: 10px; border: 2px solid #2a9d8f; }\n"
"#kpiCardLibres QLabel { color: #ffffff; }"));
        kpiLibresLayout = new QVBoxLayout(kpiCardLibres);
        kpiLibresLayout->setObjectName("kpiLibresLayout");
        lblKpiLibresTitle = new QLabel(kpiCardLibres);
        lblKpiLibresTitle->setObjectName("lblKpiLibresTitle");
        lblKpiLibresTitle->setStyleSheet(QString::fromUtf8("font-size: 12px; font-weight: bold;"));

        kpiLibresLayout->addWidget(lblKpiLibresTitle);

        lblKpiLibresValue = new QLabel(kpiCardLibres);
        lblKpiLibresValue->setObjectName("lblKpiLibresValue");
        lblKpiLibresValue->setStyleSheet(QString::fromUtf8("font-size: 28px; font-weight: bold;"));

        kpiLibresLayout->addWidget(lblKpiLibresValue);


        kpiCardsLayout->addWidget(kpiCardLibres);

        kpiCardOccupes = new QFrame(tabSupervision);
        kpiCardOccupes->setObjectName("kpiCardOccupes");
        kpiCardOccupes->setMinimumSize(QSize(180, 100));
        kpiCardOccupes->setStyleSheet(QString::fromUtf8("#kpiCardOccupes { background: qlineargradient(x1:0,y1:0,x2:0,y2:1, stop:0 #e74c3c, stop:1 #c0392b); border-radius: 10px; border: 2px solid #2a9d8f; }\n"
"#kpiCardOccupes QLabel { color: #ffffff; }"));
        kpiOccupesLayout = new QVBoxLayout(kpiCardOccupes);
        kpiOccupesLayout->setObjectName("kpiOccupesLayout");
        lblKpiOccupesTitle = new QLabel(kpiCardOccupes);
        lblKpiOccupesTitle->setObjectName("lblKpiOccupesTitle");
        lblKpiOccupesTitle->setStyleSheet(QString::fromUtf8("font-size: 12px; font-weight: bold;"));

        kpiOccupesLayout->addWidget(lblKpiOccupesTitle);

        lblKpiOccupesValue = new QLabel(kpiCardOccupes);
        lblKpiOccupesValue->setObjectName("lblKpiOccupesValue");
        lblKpiOccupesValue->setStyleSheet(QString::fromUtf8("font-size: 28px; font-weight: bold;"));

        kpiOccupesLayout->addWidget(lblKpiOccupesValue);


        kpiCardsLayout->addWidget(kpiCardOccupes);

        kpiCardMaintenance = new QFrame(tabSupervision);
        kpiCardMaintenance->setObjectName("kpiCardMaintenance");
        kpiCardMaintenance->setMinimumSize(QSize(180, 100));
        kpiCardMaintenance->setStyleSheet(QString::fromUtf8("#kpiCardMaintenance { background: qlineargradient(x1:0,y1:0,x2:0,y2:1, stop:0 #f39c12, stop:1 #e67e22); border-radius: 10px; border: 2px solid #2a9d8f; }\n"
"#kpiCardMaintenance QLabel { color: #ffffff; }"));
        kpiMaintenanceLayout = new QVBoxLayout(kpiCardMaintenance);
        kpiMaintenanceLayout->setObjectName("kpiMaintenanceLayout");
        lblKpiMaintenanceTitle = new QLabel(kpiCardMaintenance);
        lblKpiMaintenanceTitle->setObjectName("lblKpiMaintenanceTitle");
        lblKpiMaintenanceTitle->setStyleSheet(QString::fromUtf8("font-size: 12px; font-weight: bold;"));

        kpiMaintenanceLayout->addWidget(lblKpiMaintenanceTitle);

        lblKpiMaintenanceValue = new QLabel(kpiCardMaintenance);
        lblKpiMaintenanceValue->setObjectName("lblKpiMaintenanceValue");
        lblKpiMaintenanceValue->setStyleSheet(QString::fromUtf8("font-size: 28px; font-weight: bold;"));

        kpiMaintenanceLayout->addWidget(lblKpiMaintenanceValue);


        kpiCardsLayout->addWidget(kpiCardMaintenance);


        supervisionLayout->addLayout(kpiCardsLayout);

        supervisionSplitLayout = new QHBoxLayout();
        supervisionSplitLayout->setSpacing(20);
        supervisionSplitLayout->setObjectName("supervisionSplitLayout");
        superLeftLayout = new QVBoxLayout();
        superLeftLayout->setSpacing(20);
        superLeftLayout->setObjectName("superLeftLayout");
        groupBoxOccupation = new QGroupBox(tabSupervision);
        groupBoxOccupation->setObjectName("groupBoxOccupation");
        groupBoxOccupation->setMinimumSize(QSize(0, 420));
        groupBoxOccupation->setStyleSheet(QString::fromUtf8("QGroupBox {\n"
"    background-color: #262626;\n"
"    color: #ffffff;\n"
"    padding: 18px;\n"
"    border-radius: 8px;\n"
"    border: 1px solid #3d3d3d;\n"
"}\n"
"QGroupBox::title {\n"
"    subcontrol-origin: margin;\n"
"    left: 20px;\n"
"    padding: 5px 14px;\n"
"    color: #ffffff;\n"
"    background-color: #3498db;\n"
"    border-radius: 6px;\n"
"    font-weight: bold;\n"
"    font-size: 13px;\n"
"}"));
        groupBoxOccupation->setFlat(false);
        occupationLayout = new QHBoxLayout(groupBoxOccupation);
        occupationLayout->setSpacing(20);
        occupationLayout->setObjectName("occupationLayout");
        tableOccupationByType = new QTableWidget(groupBoxOccupation);
        if (tableOccupationByType->columnCount() < 4)
            tableOccupationByType->setColumnCount(4);
        QTableWidgetItem *__qtablewidgetitem93 = new QTableWidgetItem();
        tableOccupationByType->setHorizontalHeaderItem(0, __qtablewidgetitem93);
        QTableWidgetItem *__qtablewidgetitem94 = new QTableWidgetItem();
        tableOccupationByType->setHorizontalHeaderItem(1, __qtablewidgetitem94);
        QTableWidgetItem *__qtablewidgetitem95 = new QTableWidgetItem();
        tableOccupationByType->setHorizontalHeaderItem(2, __qtablewidgetitem95);
        QTableWidgetItem *__qtablewidgetitem96 = new QTableWidgetItem();
        tableOccupationByType->setHorizontalHeaderItem(3, __qtablewidgetitem96);
        tableOccupationByType->setObjectName("tableOccupationByType");
        tableOccupationByType->setMinimumSize(QSize(400, 320));
        tableOccupationByType->setAlternatingRowColors(true);

        occupationLayout->addWidget(tableOccupationByType);

        pieContainerLayout = new QVBoxLayout();
        pieContainerLayout->setSpacing(16);
        pieContainerLayout->setObjectName("pieContainerLayout");
        chartTitle = new QLabel(groupBoxOccupation);
        chartTitle->setObjectName("chartTitle");
        chartTitle->setStyleSheet(QString::fromUtf8("font-weight: bold; color: #2a9d8f; font-size: 16px;"));
        chartTitle->setAlignment(Qt::AlignmentFlag::AlignCenter);

        pieContainerLayout->addWidget(chartTitle);

        chartAndLegendLayout = new QHBoxLayout();
        chartAndLegendLayout->setSpacing(24);
        chartAndLegendLayout->setObjectName("chartAndLegendLayout");
        pieChartFrame = new QFrame(groupBoxOccupation);
        pieChartFrame->setObjectName("pieChartFrame");
        pieChartFrame->setMinimumSize(QSize(320, 320));
        pieChartFrame->setMaximumSize(QSize(400, 400));
        pieChartFrame->setStyleSheet(QString::fromUtf8("\n"
"                    border: 4px solid #1e5f74;\n"
"                    border-radius: 160px;\n"
"                    background-color: qconicalgradient(cx:0.5, cy:0.5, angle:90, \n"
"                        stop:0 #264653, stop:0.4 #264653, \n"
"                        stop:0.4001 #2a9d8f, stop:0.65 #2a9d8f, \n"
"                        stop:0.6501 #e9c46a, stop:0.85 #e9c46a, \n"
"                        stop:0.8501 #e76f51, stop:1 #e76f51);\n"
"                   "));
        pieChartFrame->setFrameShape(QFrame::Shape::NoFrame);

        chartAndLegendLayout->addWidget(pieChartFrame);

        legendFrame = new QFrame(groupBoxOccupation);
        legendFrame->setObjectName("legendFrame");
        legendLayout = new QVBoxLayout(legendFrame);
        legendLayout->setSpacing(6);
        legendLayout->setObjectName("legendLayout");
        legend1 = new QLabel(legendFrame);
        legend1->setObjectName("legend1");
        legend1->setStyleSheet(QString::fromUtf8("color: #264653; font-weight: bold; font-size: 14px;"));

        legendLayout->addWidget(legend1);

        legend2 = new QLabel(legendFrame);
        legend2->setObjectName("legend2");
        legend2->setStyleSheet(QString::fromUtf8("color: #2a9d8f; font-weight: bold; font-size: 14px;"));

        legendLayout->addWidget(legend2);

        legend3 = new QLabel(legendFrame);
        legend3->setObjectName("legend3");
        legend3->setStyleSheet(QString::fromUtf8("color: #e9c46a; font-weight: bold; font-size: 14px;"));

        legendLayout->addWidget(legend3);

        legend4 = new QLabel(legendFrame);
        legend4->setObjectName("legend4");
        legend4->setStyleSheet(QString::fromUtf8("color: #e76f51; font-weight: bold; font-size: 14px;"));

        legendLayout->addWidget(legend4);


        chartAndLegendLayout->addWidget(legendFrame);


        pieContainerLayout->addLayout(chartAndLegendLayout);


        occupationLayout->addLayout(pieContainerLayout);


        superLeftLayout->addWidget(groupBoxOccupation);


        supervisionSplitLayout->addLayout(superLeftLayout);

        superRightLayout = new QVBoxLayout();
        superRightLayout->setObjectName("superRightLayout");
        incidentButtonsInner = new QHBoxLayout();
        incidentButtonsInner->setObjectName("incidentButtonsInner");
        btnAddIncident = new QPushButton(tabSupervision);
        btnAddIncident->setObjectName("btnAddIncident");
        btnAddIncident->setStyleSheet(QString::fromUtf8("background-color: #2196F3;"));

        incidentButtonsInner->addWidget(btnAddIncident);

        horizontalSpacer_3 = new QSpacerItem(40, 20, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);

        incidentButtonsInner->addItem(horizontalSpacer_3);


        superRightLayout->addLayout(incidentButtonsInner);

        tableSupervisionCombined = new QTableWidget(tabSupervision);
        if (tableSupervisionCombined->columnCount() < 7)
            tableSupervisionCombined->setColumnCount(7);
        QTableWidgetItem *__qtablewidgetitem97 = new QTableWidgetItem();
        tableSupervisionCombined->setHorizontalHeaderItem(0, __qtablewidgetitem97);
        QTableWidgetItem *__qtablewidgetitem98 = new QTableWidgetItem();
        tableSupervisionCombined->setHorizontalHeaderItem(1, __qtablewidgetitem98);
        QTableWidgetItem *__qtablewidgetitem99 = new QTableWidgetItem();
        tableSupervisionCombined->setHorizontalHeaderItem(2, __qtablewidgetitem99);
        QTableWidgetItem *__qtablewidgetitem100 = new QTableWidgetItem();
        tableSupervisionCombined->setHorizontalHeaderItem(3, __qtablewidgetitem100);
        QTableWidgetItem *__qtablewidgetitem101 = new QTableWidgetItem();
        tableSupervisionCombined->setHorizontalHeaderItem(4, __qtablewidgetitem101);
        QTableWidgetItem *__qtablewidgetitem102 = new QTableWidgetItem();
        tableSupervisionCombined->setHorizontalHeaderItem(5, __qtablewidgetitem102);
        QTableWidgetItem *__qtablewidgetitem103 = new QTableWidgetItem();
        tableSupervisionCombined->setHorizontalHeaderItem(6, __qtablewidgetitem103);
        tableSupervisionCombined->setObjectName("tableSupervisionCombined");
        tableSupervisionCombined->setMinimumSize(QSize(0, 120));
        tableSupervisionCombined->setAlternatingRowColors(true);

        superRightLayout->addWidget(tableSupervisionCombined);

        chartCardAlertes = new QFrame(tabSupervision);
        chartCardAlertes->setObjectName("chartCardAlertes");
        chartCardAlertes->setMinimumSize(QSize(0, 160));
        chartCardAlertes->setStyleSheet(QString::fromUtf8("#chartCardAlertes { background-color: #4a2c2c; border-radius: 10px; border: 2px solid #e74c3c; padding: 12px; }\n"
"#chartCardAlertes QLabel { color: #ffffff; }"));
        chartAlertesLayout = new QVBoxLayout(chartCardAlertes);
        chartAlertesLayout->setObjectName("chartAlertesLayout");
        lblChartAlertesTitle = new QLabel(chartCardAlertes);
        lblChartAlertesTitle->setObjectName("lblChartAlertesTitle");
        lblChartAlertesTitle->setStyleSheet(QString::fromUtf8("font-size: 14px; font-weight: bold; color: #e74c3c;"));

        chartAlertesLayout->addWidget(lblChartAlertesTitle);

        lblAlertesContent = new QLabel(chartCardAlertes);
        lblAlertesContent->setObjectName("lblAlertesContent");
        lblAlertesContent->setStyleSheet(QString::fromUtf8("font-size: 12px; color: #e0e0e0;"));
        lblAlertesContent->setWordWrap(true);

        chartAlertesLayout->addWidget(lblAlertesContent);


        superRightLayout->addWidget(chartCardAlertes);


        supervisionSplitLayout->addLayout(superRightLayout);


        supervisionLayout->addLayout(supervisionSplitLayout);

        frame_Supervision_Resume = new QFrame(tabSupervision);
        frame_Supervision_Resume->setObjectName("frame_Supervision_Resume");
        frame_Supervision_Resume->setMinimumSize(QSize(0, 48));
        frame_Supervision_Resume->setMaximumSize(QSize(16777215, 48));
        frame_Supervision_Resume->setStyleSheet(QString::fromUtf8("#frame_Supervision_Resume { background-color: #2c3e50; border-radius: 6px; border: 1px solid #34495e; }\n"
"#frame_Supervision_Resume QLabel { color: #b0b0b0; font-size: 12px; }"));
        resumeLayout = new QHBoxLayout(frame_Supervision_Resume);
        resumeLayout->setSpacing(24);
        resumeLayout->setObjectName("resumeLayout");
        resumeLayout->setContentsMargins(16, -1, 16, -1);
        lblResumeQuais = new QLabel(frame_Supervision_Resume);
        lblResumeQuais->setObjectName("lblResumeQuais");

        resumeLayout->addWidget(lblResumeQuais);

        spacerResume = new QSpacerItem(40, 20, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);

        resumeLayout->addItem(spacerResume);

        lblResumeMAJ = new QLabel(frame_Supervision_Resume);
        lblResumeMAJ->setObjectName("lblResumeMAJ");

        resumeLayout->addWidget(lblResumeMAJ);


        supervisionLayout->addWidget(frame_Supervision_Resume);

        mainTabWidget->addTab(tabSupervision, QString());
        tabAideDecision = new QWidget();
        tabAideDecision->setObjectName("tabAideDecision");
        aideDecisionLayout = new QVBoxLayout(tabAideDecision);
        aideDecisionLayout->setObjectName("aideDecisionLayout");
        groupBoxBoatInfo = new QGroupBox(tabAideDecision);
        groupBoxBoatInfo->setObjectName("groupBoxBoatInfo");
        groupBoxBoatInfo->setStyleSheet(QString::fromUtf8("QGroupBox {\n"
"    background-color: #262626;\n"
"    color: #ffffff;\n"
"    padding: 14px;\n"
"}\n"
"QGroupBox::title {\n"
"    subcontrol-origin: margin;\n"
"    left: 20px;\n"
"    padding: 4px 12px;\n"
"    color: #ffffff;\n"
"    background-color: #3498db;\n"
"    border-radius: 6px;\n"
"}"));
        boatFormLayout = new QFormLayout(groupBoxBoatInfo);
        boatFormLayout->setObjectName("boatFormLayout");
        labelTypeBateau = new QLabel(groupBoxBoatInfo);
        labelTypeBateau->setObjectName("labelTypeBateau");

        boatFormLayout->setWidget(0, QFormLayout::LabelRole, labelTypeBateau);

        comboBoxTypeBateau = new QComboBox(groupBoxBoatInfo);
        comboBoxTypeBateau->addItem(QString());
        comboBoxTypeBateau->addItem(QString());
        comboBoxTypeBateau->addItem(QString());
        comboBoxTypeBateau->setObjectName("comboBoxTypeBateau");

        boatFormLayout->setWidget(0, QFormLayout::FieldRole, comboBoxTypeBateau);

        labelLongueurBateau = new QLabel(groupBoxBoatInfo);
        labelLongueurBateau->setObjectName("labelLongueurBateau");

        boatFormLayout->setWidget(1, QFormLayout::LabelRole, labelLongueurBateau);

        spinBoxLongueurBateau = new QSpinBox(groupBoxBoatInfo);
        spinBoxLongueurBateau->setObjectName("spinBoxLongueurBateau");
        spinBoxLongueurBateau->setMaximum(300);

        boatFormLayout->setWidget(1, QFormLayout::FieldRole, spinBoxLongueurBateau);

        labelTonnage = new QLabel(groupBoxBoatInfo);
        labelTonnage->setObjectName("labelTonnage");

        boatFormLayout->setWidget(2, QFormLayout::LabelRole, labelTonnage);

        spinBoxTonnage = new QSpinBox(groupBoxBoatInfo);
        spinBoxTonnage->setObjectName("spinBoxTonnage");
        spinBoxTonnage->setMaximum(50000);

        boatFormLayout->setWidget(2, QFormLayout::FieldRole, spinBoxTonnage);

        btnAnalyze = new QPushButton(groupBoxBoatInfo);
        btnAnalyze->setObjectName("btnAnalyze");

        boatFormLayout->setWidget(3, QFormLayout::SpanningRole, btnAnalyze);


        aideDecisionLayout->addWidget(groupBoxBoatInfo);

        tableRecommendations = new QTableWidget(tabAideDecision);
        if (tableRecommendations->columnCount() < 5)
            tableRecommendations->setColumnCount(5);
        QTableWidgetItem *__qtablewidgetitem104 = new QTableWidgetItem();
        tableRecommendations->setHorizontalHeaderItem(0, __qtablewidgetitem104);
        QTableWidgetItem *__qtablewidgetitem105 = new QTableWidgetItem();
        tableRecommendations->setHorizontalHeaderItem(1, __qtablewidgetitem105);
        QTableWidgetItem *__qtablewidgetitem106 = new QTableWidgetItem();
        tableRecommendations->setHorizontalHeaderItem(2, __qtablewidgetitem106);
        QTableWidgetItem *__qtablewidgetitem107 = new QTableWidgetItem();
        tableRecommendations->setHorizontalHeaderItem(3, __qtablewidgetitem107);
        QTableWidgetItem *__qtablewidgetitem108 = new QTableWidgetItem();
        tableRecommendations->setHorizontalHeaderItem(4, __qtablewidgetitem108);
        tableRecommendations->setObjectName("tableRecommendations");
        tableRecommendations->setAlternatingRowColors(true);

        aideDecisionLayout->addWidget(tableRecommendations);

        mainTabWidget->addTab(tabAideDecision, QString());
        tabParametres = new QWidget();
        tabParametres->setObjectName("tabParametres");
        paramsLayout = new QVBoxLayout(tabParametres);
        paramsLayout->setObjectName("paramsLayout");
        paramsTabs = new QTabWidget(tabParametres);
        paramsTabs->setObjectName("paramsTabs");
        paramsTabs->setStyleSheet(QString::fromUtf8("QTabWidget::pane { border: 2px solid #2a9d8f; background-color: #e5ebf0; border-radius: 8px; margin-top: -1px; padding: 12px; }\n"
"QTabBar::tab { background-color: #456b77; color: #ffffff; padding: 10px 18px; margin-right: 2px; border-top-left-radius: 6px; border-top-right-radius: 6px; font-weight: 600; font-size: 12px; border-left: 3px solid transparent; }\n"
"QTabBar::tab:selected { background-color: #2a9d8f; border-left: 3px solid #14ffec; }\n"
"QTabBar::tab:hover:!selected { background-color: #3e7a9e; }"));
        tabSyntheseVocale = new QWidget();
        tabSyntheseVocale->setObjectName("tabSyntheseVocale");
        ttsInnerLayout = new QVBoxLayout(tabSyntheseVocale);
        ttsInnerLayout->setObjectName("ttsInnerLayout");
        groupBoxTTSSettings = new QGroupBox(tabSyntheseVocale);
        groupBoxTTSSettings->setObjectName("groupBoxTTSSettings");
        groupBoxTTSSettings->setStyleSheet(QString::fromUtf8("QGroupBox {\n"
"    background-color: #262626;\n"
"    color: #ffffff;\n"
"    padding: 18px;\n"
"    border-radius: 8px;\n"
"    border: 1px solid #3d3d3d;\n"
"}\n"
"QGroupBox::title {\n"
"    subcontrol-origin: margin;\n"
"    left: 20px;\n"
"    padding: 5px 14px;\n"
"    color: #ffffff;\n"
"    background-color: #3498db;\n"
"    border-radius: 6px;\n"
"    font-weight: bold;\n"
"    font-size: 13px;\n"
"}"));
        ttsFormLayout = new QFormLayout(groupBoxTTSSettings);
        ttsFormLayout->setObjectName("ttsFormLayout");
        checkBoxEnableTTS = new QCheckBox(groupBoxTTSSettings);
        checkBoxEnableTTS->setObjectName("checkBoxEnableTTS");

        ttsFormLayout->setWidget(0, QFormLayout::SpanningRole, checkBoxEnableTTS);

        labelLanguage = new QLabel(groupBoxTTSSettings);
        labelLanguage->setObjectName("labelLanguage");

        ttsFormLayout->setWidget(1, QFormLayout::LabelRole, labelLanguage);

        comboBoxLanguage = new QComboBox(groupBoxTTSSettings);
        comboBoxLanguage->addItem(QString());
        comboBoxLanguage->addItem(QString());
        comboBoxLanguage->addItem(QString());
        comboBoxLanguage->setObjectName("comboBoxLanguage");

        ttsFormLayout->setWidget(1, QFormLayout::FieldRole, comboBoxLanguage);

        checkBoxReadSelected = new QCheckBox(groupBoxTTSSettings);
        checkBoxReadSelected->setObjectName("checkBoxReadSelected");

        ttsFormLayout->setWidget(2, QFormLayout::SpanningRole, checkBoxReadSelected);

        checkBoxCriticalAlerts = new QCheckBox(groupBoxTTSSettings);
        checkBoxCriticalAlerts->setObjectName("checkBoxCriticalAlerts");

        ttsFormLayout->setWidget(3, QFormLayout::SpanningRole, checkBoxCriticalAlerts);

        checkBoxDailySummary = new QCheckBox(groupBoxTTSSettings);
        checkBoxDailySummary->setObjectName("checkBoxDailySummary");

        ttsFormLayout->setWidget(4, QFormLayout::SpanningRole, checkBoxDailySummary);

        labelSummaryTime = new QLabel(groupBoxTTSSettings);
        labelSummaryTime->setObjectName("labelSummaryTime");

        ttsFormLayout->setWidget(5, QFormLayout::LabelRole, labelSummaryTime);

        timeEditSummary = new QTimeEdit(groupBoxTTSSettings);
        timeEditSummary->setObjectName("timeEditSummary");
        timeEditSummary->setTime(QTime(8, 0, 0));

        ttsFormLayout->setWidget(5, QFormLayout::FieldRole, timeEditSummary);

        btnTestTTS = new QPushButton(groupBoxTTSSettings);
        btnTestTTS->setObjectName("btnTestTTS");
        btnTestTTS->setStyleSheet(QString::fromUtf8("background-color: #2196F3;"));

        ttsFormLayout->setWidget(6, QFormLayout::SpanningRole, btnTestTTS);


        ttsInnerLayout->addWidget(groupBoxTTSSettings);

        tableTTSLog = new QTableWidget(tabSyntheseVocale);
        if (tableTTSLog->columnCount() < 3)
            tableTTSLog->setColumnCount(3);
        QTableWidgetItem *__qtablewidgetitem109 = new QTableWidgetItem();
        tableTTSLog->setHorizontalHeaderItem(0, __qtablewidgetitem109);
        QTableWidgetItem *__qtablewidgetitem110 = new QTableWidgetItem();
        tableTTSLog->setHorizontalHeaderItem(1, __qtablewidgetitem110);
        QTableWidgetItem *__qtablewidgetitem111 = new QTableWidgetItem();
        tableTTSLog->setHorizontalHeaderItem(2, __qtablewidgetitem111);
        tableTTSLog->setObjectName("tableTTSLog");
        tableTTSLog->setAlternatingRowColors(true);

        ttsInnerLayout->addWidget(tableTTSLog);

        paramsTabs->addTab(tabSyntheseVocale, QString());

        paramsLayout->addWidget(paramsTabs);

        mainTabWidget->addTab(tabParametres, QString());

        contentLayout->addWidget(mainTabWidget);


        pageQuaisLayout->addWidget(contentFrame);

        stackedWidget->addWidget(pageQuais);
        pageStock = new QWidget();
        pageStock->setObjectName("pageStock");
        pageStock->setStyleSheet(QString::fromUtf8("QWidget#pageStock { background-color: #ffffff; }\n"
"QTabWidget::pane {\n"
"    border: 1px solid #dcdde1;\n"
"    border-radius: 10px;\n"
"    background: #ffffff;\n"
"    top: -1px;\n"
"}\n"
"QTabBar::tab {\n"
"    background-color: #2a9d8f;\n"
"    color: #ffffff;\n"
"    padding: 8px 15px;\n"
"    margin: 5px;\n"
"    min-width: 120px;\n"
"    border-radius: 5px;\n"
"    font-weight: bold;\n"
"}\n"
"QTabBar::tab:hover { background-color: #21867a; }\n"
"QTabBar::tab:selected { background-color: #1e5f74; font-weight: bold; }"));
        verticalLayout_stock = new QVBoxLayout(pageStock);
        verticalLayout_stock->setSpacing(20);
        verticalLayout_stock->setObjectName("verticalLayout_stock");
        verticalLayout_stock->setContentsMargins(20, 35, 20, 20);
        frame_Stock_Header = new QFrame(pageStock);
        frame_Stock_Header->setObjectName("frame_Stock_Header");
        frame_Stock_Header->setMinimumSize(QSize(0, 120));
        frame_Stock_Header->setMaximumSize(QSize(16777215, 120));
        frame_Stock_Header->setStyleSheet(QString::fromUtf8("#frame_Stock_Header {\n"
"     border-image: url(:/new/prefix1/img1.jpg);\n"
"}\n"
"#frame_Stock_Header_Overlay {\n"
"     background-color: rgba(0, 0, 0, 0.55);\n"
"}"));
        frame_Stock_Header->setFrameShape(QFrame::Shape::NoFrame);
        verticalLayout_Header_Stock = new QVBoxLayout(frame_Stock_Header);
        verticalLayout_Header_Stock->setSpacing(0);
        verticalLayout_Header_Stock->setObjectName("verticalLayout_Header_Stock");
        verticalLayout_Header_Stock->setContentsMargins(0, 0, 0, 0);
        frame_Stock_Header_Overlay = new QFrame(frame_Stock_Header);
        frame_Stock_Header_Overlay->setObjectName("frame_Stock_Header_Overlay");
        verticalLayout_Title_Stock = new QVBoxLayout(frame_Stock_Header_Overlay);
        verticalLayout_Title_Stock->setObjectName("verticalLayout_Title_Stock");
        title_label_stock = new QLabel(frame_Stock_Header_Overlay);
        title_label_stock->setObjectName("title_label_stock");
        title_label_stock->setStyleSheet(QString::fromUtf8("color: white; font-size: 28px; font-weight: bold; font-family: \"Script MT Bold\";"));
        title_label_stock->setAlignment(Qt::AlignmentFlag::AlignCenter);

        verticalLayout_Title_Stock->addWidget(title_label_stock);


        verticalLayout_Header_Stock->addWidget(frame_Stock_Header_Overlay);


        verticalLayout_stock->addWidget(frame_Stock_Header);

        tabWidgetStockVentes = new QTabWidget(pageStock);
        tabWidgetStockVentes->setObjectName("tabWidgetStockVentes");
        tabWidgetStockVentes->setStyleSheet(QString::fromUtf8("QTabWidget::pane {\n"
"    border: 2px solid #2a9d8f;\n"
"    background-color: #2b2b2b;\n"
"    border-radius: 5px;\n"
"}\n"
"QTabBar::tab {\n"
"    background-color: #456b77;\n"
"    color: white;\n"
"    padding: 10px 20px;\n"
"    margin-right: 2px;\n"
"    border-top-left-radius: 4px;\n"
"    border-top-right-radius: 4px;\n"
"    width: 150px;\n"
"}\n"
"QTabBar::tab:selected {\n"
"    background-color: #2a9d8f;\n"
"}\n"
"QWidget {\n"
"    font-family: \"Segoe UI\";\n"
"}\n"
"QLabel {\n"
"    color: #ffffff;\n"
"    font-weight: bold;\n"
"    font-size: 12px;\n"
"}\n"
"QLineEdit, QComboBox, QDateEdit, QSpinBox, QDoubleSpinBox {\n"
"    background-color: #3a3a3a;\n"
"    color: white;\n"
"    border: 1px solid #555;\n"
"    padding: 5px;\n"
"    border-radius: 3px;\n"
"}\n"
"QPushButton {\n"
"    background-color: #2a9d8f;\n"
"    color: white;\n"
"    border: none;\n"
"    padding: 10px;\n"
"    font-weight: bold;\n"
"    border-radius: 5px;\n"
"}\n"
"QPushButton:hover {\n"
"    background-color: #21867a;\n"
""
                        "}\n"
"QPushButton:pressed {\n"
"    background-color: #1e5f74;\n"
"}\n"
"QTableWidget, QTableView {\n"
"    background-color: #2b2b2b;\n"
"    color: white;\n"
"    gridline-color: #555;\n"
"    border: 1px solid #2a9d8f;\n"
"}\n"
"QHeaderView::section {\n"
"    background-color: #1e5f74;\n"
"    color: white;\n"
"    padding: 5px;\n"
"    border: none;\n"
"    font-weight: bold;\n"
"}"));
        tabStock = new QWidget();
        tabStock->setObjectName("tabStock");
        tabStock->setStyleSheet(QString::fromUtf8("QWidget#tabStock { background-color: #ffffff; }\n"
"QTableWidget { background-color: #2b2b2b; color: white; gridline-color: #555; border: 1px solid #dcdde1; }\n"
"QHeaderView::section { background-color: #34495e; color: white; padding: 5px; border: none; font-weight: bold; }"));
        verticalLayout_Stock_Gestion = new QVBoxLayout(tabStock);
        verticalLayout_Stock_Gestion->setSpacing(12);
        verticalLayout_Stock_Gestion->setObjectName("verticalLayout_Stock_Gestion");
        verticalLayout_Stock_Gestion->setContentsMargins(20, 15, 20, 15);
        groupBoxStockForm = new QGroupBox(tabStock);
        groupBoxStockForm->setObjectName("groupBoxStockForm");
        groupBoxStockForm->setStyleSheet(QString::fromUtf8("QGroupBox { font-weight: bold; border: 1px solid #dcdde1; border-radius: 6px; background: #232323; padding-top: 35px; margin-top: 20px; color: #ffffff; }\n"
"QGroupBox::title { subcontrol-origin: margin; left: 16px; padding: 0 8px; color: #ffffff; }\n"
"QLabel { color: #ffffff; }\n"
"QLineEdit, QComboBox, QDateEdit { background-color: #2d2d2d; color: white; border: 1px solid #3d3d3d; padding: 5px; border-radius: 4px; }"));
        gridLayout_Stock_Form = new QGridLayout(groupBoxStockForm);
        gridLayout_Stock_Form->setObjectName("gridLayout_Stock_Form");
        stockIdLabel = new QLabel(groupBoxStockForm);
        stockIdLabel->setObjectName("stockIdLabel");

        gridLayout_Stock_Form->addWidget(stockIdLabel, 0, 0, 1, 1);

        stockIdEdit = new QLineEdit(groupBoxStockForm);
        stockIdEdit->setObjectName("stockIdEdit");

        gridLayout_Stock_Form->addWidget(stockIdEdit, 0, 1, 1, 1);

        stockEspeceLabel = new QLabel(groupBoxStockForm);
        stockEspeceLabel->setObjectName("stockEspeceLabel");

        gridLayout_Stock_Form->addWidget(stockEspeceLabel, 0, 2, 1, 1);

        stockEspeceEdit = new QLineEdit(groupBoxStockForm);
        stockEspeceEdit->setObjectName("stockEspeceEdit");

        gridLayout_Stock_Form->addWidget(stockEspeceEdit, 0, 3, 1, 1);

        stockQuantiteLabel = new QLabel(groupBoxStockForm);
        stockQuantiteLabel->setObjectName("stockQuantiteLabel");

        gridLayout_Stock_Form->addWidget(stockQuantiteLabel, 0, 4, 1, 1);

        stockQuantiteEdit = new QLineEdit(groupBoxStockForm);
        stockQuantiteEdit->setObjectName("stockQuantiteEdit");

        gridLayout_Stock_Form->addWidget(stockQuantiteEdit, 0, 5, 1, 1);

        stockEtatLabel = new QLabel(groupBoxStockForm);
        stockEtatLabel->setObjectName("stockEtatLabel");

        gridLayout_Stock_Form->addWidget(stockEtatLabel, 1, 0, 1, 1);

        stockEtatCombo = new QComboBox(groupBoxStockForm);
        stockEtatCombo->addItem(QString());
        stockEtatCombo->addItem(QString());
        stockEtatCombo->addItem(QString());
        stockEtatCombo->setObjectName("stockEtatCombo");

        gridLayout_Stock_Form->addWidget(stockEtatCombo, 1, 1, 1, 1);

        stockSeuilMinLabel = new QLabel(groupBoxStockForm);
        stockSeuilMinLabel->setObjectName("stockSeuilMinLabel");

        gridLayout_Stock_Form->addWidget(stockSeuilMinLabel, 1, 2, 1, 1);

        stockSeuilMinEdit = new QLineEdit(groupBoxStockForm);
        stockSeuilMinEdit->setObjectName("stockSeuilMinEdit");

        gridLayout_Stock_Form->addWidget(stockSeuilMinEdit, 1, 3, 1, 1);

        stockSeuilMaxLabel = new QLabel(groupBoxStockForm);
        stockSeuilMaxLabel->setObjectName("stockSeuilMaxLabel");

        gridLayout_Stock_Form->addWidget(stockSeuilMaxLabel, 1, 4, 1, 1);

        stockSeuilMaxEdit = new QLineEdit(groupBoxStockForm);
        stockSeuilMaxEdit->setObjectName("stockSeuilMaxEdit");

        gridLayout_Stock_Form->addWidget(stockSeuilMaxEdit, 1, 5, 1, 1);

        stockDateAjoutLabel = new QLabel(groupBoxStockForm);
        stockDateAjoutLabel->setObjectName("stockDateAjoutLabel");

        gridLayout_Stock_Form->addWidget(stockDateAjoutLabel, 2, 0, 1, 1);

        stockDateAjoutEdit = new QLineEdit(groupBoxStockForm);
        stockDateAjoutEdit->setObjectName("stockDateAjoutEdit");

        gridLayout_Stock_Form->addWidget(stockDateAjoutEdit, 2, 1, 1, 1);

        stockCINLabel = new QLabel(groupBoxStockForm);
        stockCINLabel->setObjectName("stockCINLabel");

        gridLayout_Stock_Form->addWidget(stockCINLabel, 2, 2, 1, 1);

        stockCIN = new QLineEdit(groupBoxStockForm);
        stockCIN->setObjectName("stockCIN");

        gridLayout_Stock_Form->addWidget(stockCIN, 2, 3, 1, 1);

        stockCMDLabel = new QLabel(groupBoxStockForm);
        stockCMDLabel->setObjectName("stockCMDLabel");

        gridLayout_Stock_Form->addWidget(stockCMDLabel, 2, 4, 1, 1);

        stockCMD = new QLineEdit(groupBoxStockForm);
        stockCMD->setObjectName("stockCMD");

        gridLayout_Stock_Form->addWidget(stockCMD, 2, 5, 1, 1);

        stockDateVenteLabel = new QLabel(groupBoxStockForm);
        stockDateVenteLabel->setObjectName("stockDateVenteLabel");

        gridLayout_Stock_Form->addWidget(stockDateVenteLabel, 3, 0, 1, 1);

        stockDateVente = new QLineEdit(groupBoxStockForm);
        stockDateVente->setObjectName("stockDateVente");

        gridLayout_Stock_Form->addWidget(stockDateVente, 3, 1, 1, 1);


        verticalLayout_Stock_Gestion->addWidget(groupBoxStockForm);

        frameStockToolbar = new QFrame(tabStock);
        frameStockToolbar->setObjectName("frameStockToolbar");
        frameStockToolbar->setMinimumSize(QSize(0, 50));
        frameStockToolbar->setStyleSheet(QString::fromUtf8("QFrame#frameStockToolbar { background-color: #34495e; border-radius: 8px; padding: 8px; border: 1px solid #2c3e50; }\n"
"QPushButton { min-width: 80px; padding: 8px; border-radius: 6px; font-weight: bold; }\n"
"QPushButton#btnStockAjouter, QPushButton#btnStockModifier { background: #3498db; color: black; }\n"
"QPushButton#btnStockSupprimer { background: #e74c3c; color: white; }\n"
"QPushButton#btnStockSave { background: #2a9d8f; color: white; }\n"
"QPushButton#btnStockAnnuler { background: #456b77; color: white; }\n"
"QLineEdit { background: #2c3e50; color: #ffffff; border: 1px solid #3498db; border-radius: 6px; padding: 5px; }"));
        horizontalLayout_Stock_Toolbar = new QHBoxLayout(frameStockToolbar);
        horizontalLayout_Stock_Toolbar->setObjectName("horizontalLayout_Stock_Toolbar");
        btnStockAjouter = new QPushButton(frameStockToolbar);
        btnStockAjouter->setObjectName("btnStockAjouter");
        btnStockAjouter->setFont(font1);

        horizontalLayout_Stock_Toolbar->addWidget(btnStockAjouter);

        btnStockModifier = new QPushButton(frameStockToolbar);
        btnStockModifier->setObjectName("btnStockModifier");
        btnStockModifier->setFont(font1);

        horizontalLayout_Stock_Toolbar->addWidget(btnStockModifier);

        btnStockSupprimer = new QPushButton(frameStockToolbar);
        btnStockSupprimer->setObjectName("btnStockSupprimer");
        btnStockSupprimer->setFont(font1);

        horizontalLayout_Stock_Toolbar->addWidget(btnStockSupprimer);

        btnStockSave = new QPushButton(frameStockToolbar);
        btnStockSave->setObjectName("btnStockSave");
        btnStockSave->setVisible(false);

        horizontalLayout_Stock_Toolbar->addWidget(btnStockSave);

        btnStockAnnuler = new QPushButton(frameStockToolbar);
        btnStockAnnuler->setObjectName("btnStockAnnuler");
        btnStockAnnuler->setVisible(false);

        horizontalLayout_Stock_Toolbar->addWidget(btnStockAnnuler);

        stockToolbarSpacer = new QSpacerItem(40, 20, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);

        horizontalLayout_Stock_Toolbar->addItem(stockToolbarSpacer);

        stockSearchEdit = new QLineEdit(frameStockToolbar);
        stockSearchEdit->setObjectName("stockSearchEdit");
        stockSearchEdit->setMinimumWidth(200);

        horizontalLayout_Stock_Toolbar->addWidget(stockSearchEdit);

        stockSortLabel = new QLabel(frameStockToolbar);
        stockSortLabel->setObjectName("stockSortLabel");

        horizontalLayout_Stock_Toolbar->addWidget(stockSortLabel);

        stockSortCombo = new QComboBox(frameStockToolbar);
        stockSortCombo->addItem(QString());
        stockSortCombo->addItem(QString());
        stockSortCombo->addItem(QString());
        stockSortCombo->setObjectName("stockSortCombo");
        stockSortCombo->setMinimumSize(QSize(120, 0));

        horizontalLayout_Stock_Toolbar->addWidget(stockSortCombo);


        verticalLayout_Stock_Gestion->addWidget(frameStockToolbar);

        tableStock = new QTableWidget(tabStock);
        if (tableStock->columnCount() < 10)
            tableStock->setColumnCount(10);
        QTableWidgetItem *__qtablewidgetitem112 = new QTableWidgetItem();
        tableStock->setHorizontalHeaderItem(0, __qtablewidgetitem112);
        QTableWidgetItem *__qtablewidgetitem113 = new QTableWidgetItem();
        tableStock->setHorizontalHeaderItem(1, __qtablewidgetitem113);
        QTableWidgetItem *__qtablewidgetitem114 = new QTableWidgetItem();
        tableStock->setHorizontalHeaderItem(2, __qtablewidgetitem114);
        QTableWidgetItem *__qtablewidgetitem115 = new QTableWidgetItem();
        tableStock->setHorizontalHeaderItem(3, __qtablewidgetitem115);
        QTableWidgetItem *__qtablewidgetitem116 = new QTableWidgetItem();
        tableStock->setHorizontalHeaderItem(4, __qtablewidgetitem116);
        QTableWidgetItem *__qtablewidgetitem117 = new QTableWidgetItem();
        tableStock->setHorizontalHeaderItem(5, __qtablewidgetitem117);
        QTableWidgetItem *__qtablewidgetitem118 = new QTableWidgetItem();
        tableStock->setHorizontalHeaderItem(6, __qtablewidgetitem118);
        QTableWidgetItem *__qtablewidgetitem119 = new QTableWidgetItem();
        tableStock->setHorizontalHeaderItem(7, __qtablewidgetitem119);
        QTableWidgetItem *__qtablewidgetitem120 = new QTableWidgetItem();
        tableStock->setHorizontalHeaderItem(8, __qtablewidgetitem120);
        QTableWidgetItem *__qtablewidgetitem121 = new QTableWidgetItem();
        tableStock->setHorizontalHeaderItem(9, __qtablewidgetitem121);
        tableStock->setObjectName("tableStock");
        tableStock->setAlternatingRowColors(true);
        tableStock->setSelectionBehavior(QAbstractItemView::SelectionBehavior::SelectRows);
        tableStock->setStyleSheet(QString::fromUtf8("QTableWidget { background-color: #2d2d2d; gridline-color: #3d3d3d; selection-background-color: #3498db; border-radius: 8px; border: 2px solid #3498db; color: white; }\n"
"QHeaderView::section { background-color: #3d3d3d; color: white; padding: 6px; border: 1px solid #232323; font-weight: bold; }"));

        verticalLayout_Stock_Gestion->addWidget(tableStock);

        tabWidgetStockVentes->addTab(tabStock, QString());
        tabVentes = new QWidget();
        tabVentes->setObjectName("tabVentes");
        frameVentesForm = new QFrame(tabVentes);
        frameVentesForm->setObjectName("frameVentesForm");
        frameVentesForm->setGeometry(QRect(120, 100, 1010, 230));
        frameVentesForm->setFrameShape(QFrame::Shape::StyledPanel);
        frameVentesForm->setFrameShadow(QFrame::Shadow::Raised);
        venteAcheteurEdit = new QLineEdit(frameVentesForm);
        venteAcheteurEdit->setObjectName("venteAcheteurEdit");
        venteAcheteurEdit->setGeometry(QRect(90, 20, 160, 24));
        venteAcheteurEdit->setStyleSheet(QString::fromUtf8("QFrame {\n"
"    background-color: #232323;\n"
"    border-radius: 8px;\n"
"    border: 2px solid #3498db;\n"
"}\n"
"QLabel {\n"
"    color: #ffffff;\n"
"    font-family: \"Segoe UI\";\n"
"    font-weight: bold;\n"
"    font-size: 12px;\n"
"    padding: 2px;\n"
"    border: none;\n"
"}\n"
"QLineEdit, QComboBox {\n"
"    color: white;\n"
"    background-color: #2d2d2d;\n"
"    border: 1px solid #3d3d3d;\n"
"    padding: 5px;\n"
"    border-radius: 4px;\n"
"    font-family: \"Segoe UI\";\n"
"    font-weight: bold;\n"
"}"));
        venteAcheteurLabel = new QLabel(frameVentesForm);
        venteAcheteurLabel->setObjectName("venteAcheteurLabel");
        venteAcheteurLabel->setGeometry(QRect(15, 22, 80, 20));
        venteCinLabel = new QLabel(frameVentesForm);
        venteCinLabel->setObjectName("venteCinLabel");
        venteCinLabel->setGeometry(QRect(15, 60, 80, 20));
        venteCinEdit = new QLineEdit(frameVentesForm);
        venteCinEdit->setObjectName("venteCinEdit");
        venteCinEdit->setGeometry(QRect(90, 58, 160, 24));
        venteQuantiteLabel = new QLabel(frameVentesForm);
        venteQuantiteLabel->setObjectName("venteQuantiteLabel");
        venteQuantiteLabel->setGeometry(QRect(15, 100, 80, 20));
        venteQuantiteEdit = new QLineEdit(frameVentesForm);
        venteQuantiteEdit->setObjectName("venteQuantiteEdit");
        venteQuantiteEdit->setGeometry(QRect(90, 98, 160, 24));
        venteIdLabel = new QLabel(frameVentesForm);
        venteIdLabel->setObjectName("venteIdLabel");
        venteIdLabel->setGeometry(QRect(15, 140, 80, 20));
        venteIdEdit = new QLineEdit(frameVentesForm);
        venteIdEdit->setObjectName("venteIdEdit");
        venteIdEdit->setGeometry(QRect(90, 138, 160, 24));
        venteStockIdLabel = new QLabel(frameVentesForm);
        venteStockIdLabel->setObjectName("venteStockIdLabel");
        venteStockIdLabel->setGeometry(QRect(290, 22, 110, 20));
        venteStockIdEdit = new QLineEdit(frameVentesForm);
        venteStockIdEdit->setObjectName("venteStockIdEdit");
        venteStockIdEdit->setGeometry(QRect(410, 20, 160, 24));
        venteSeuilMaxLabel = new QLabel(frameVentesForm);
        venteSeuilMaxLabel->setObjectName("venteSeuilMaxLabel");
        venteSeuilMaxLabel->setGeometry(QRect(290, 60, 110, 20));
        venteSeuilMaxEdit = new QLineEdit(frameVentesForm);
        venteSeuilMaxEdit->setObjectName("venteSeuilMaxEdit");
        venteSeuilMaxEdit->setGeometry(QRect(410, 58, 160, 24));
        venteDateLabel = new QLabel(frameVentesForm);
        venteDateLabel->setObjectName("venteDateLabel");
        venteDateLabel->setGeometry(QRect(290, 100, 110, 20));
        venteDateEdit = new QLineEdit(frameVentesForm);
        venteDateEdit->setObjectName("venteDateEdit");
        venteDateEdit->setGeometry(QRect(410, 98, 160, 24));
        frameVentesToolbar = new QFrame(frameVentesForm);
        frameVentesToolbar->setObjectName("frameVentesToolbar");
        frameVentesToolbar->setGeometry(QRect(0, 132, 1010, 40));
        frameVentesToolbar->setStyleSheet(QString::fromUtf8("QFrame#frameVentesToolbar { background-color: #34495e; border-radius: 8px; padding: 8px; border: 1px solid #2c3e50; }"));
        frameVentesToolbar->setFrameShape(QFrame::Shape::StyledPanel);
        frameVentesToolbar->setFrameShadow(QFrame::Shadow::Plain);
        btnVenteAjouter = new QPushButton(frameVentesForm);
        btnVenteAjouter->setObjectName("btnVenteAjouter");
        btnVenteAjouter->setGeometry(QRect(489, 140, 81, 24));
        btnVenteAjouter->setStyleSheet(QString::fromUtf8("QPushButton { background-color: #2ecc71; color: black; font-weight: bold; border-radius: 4px; padding: 5px; } QPushButton:hover { background-color: #27ae60; }"));
        btnVenteModifier = new QPushButton(frameVentesForm);
        btnVenteModifier->setObjectName("btnVenteModifier");
        btnVenteModifier->setGeometry(QRect(280, 140, 81, 24));
        btnVenteModifier->setStyleSheet(QString::fromUtf8("QPushButton { background-color: #3498db; color: black; font-weight: bold; border-radius: 4px; padding: 5px; } QPushButton:hover { background-color: #2980b9; }"));
        btnVenteSupprimer = new QPushButton(frameVentesForm);
        btnVenteSupprimer->setObjectName("btnVenteSupprimer");
        btnVenteSupprimer->setGeometry(QRect(370, 140, 81, 24));
        btnVenteSupprimer->setStyleSheet(QString::fromUtf8("QPushButton { background-color: #e74c3c; color: white; font-weight: bold; border-radius: 4px; padding: 5px; } QPushButton:hover { background-color: #c0392b; }"));
        tableVentes = new QTableWidget(tabVentes);
        if (tableVentes->columnCount() < 7)
            tableVentes->setColumnCount(7);
        QTableWidgetItem *__qtablewidgetitem122 = new QTableWidgetItem();
        tableVentes->setHorizontalHeaderItem(0, __qtablewidgetitem122);
        QTableWidgetItem *__qtablewidgetitem123 = new QTableWidgetItem();
        tableVentes->setHorizontalHeaderItem(1, __qtablewidgetitem123);
        QTableWidgetItem *__qtablewidgetitem124 = new QTableWidgetItem();
        tableVentes->setHorizontalHeaderItem(2, __qtablewidgetitem124);
        QTableWidgetItem *__qtablewidgetitem125 = new QTableWidgetItem();
        tableVentes->setHorizontalHeaderItem(3, __qtablewidgetitem125);
        QTableWidgetItem *__qtablewidgetitem126 = new QTableWidgetItem();
        tableVentes->setHorizontalHeaderItem(4, __qtablewidgetitem126);
        QTableWidgetItem *__qtablewidgetitem127 = new QTableWidgetItem();
        tableVentes->setHorizontalHeaderItem(5, __qtablewidgetitem127);
        QTableWidgetItem *__qtablewidgetitem128 = new QTableWidgetItem();
        tableVentes->setHorizontalHeaderItem(6, __qtablewidgetitem128);
        tableVentes->setObjectName("tableVentes");
        tableVentes->setGeometry(QRect(120, 390, 1010, 320));
        tableVentes->setStyleSheet(QString::fromUtf8("QTableWidget {\n"
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
"}"));
        venteSearchEdit = new QLineEdit(tabVentes);
        venteSearchEdit->setObjectName("venteSearchEdit");
        venteSearchEdit->setGeometry(QRect(140, 350, 360, 26));
        venteSearchEdit->setStyleSheet(QString::fromUtf8("background-color: #2d2d2d; color: #ffffff; border-radius: 4px; padding: 2px 6px; border: 1px solid #3d3d3d;"));
        venteSortLabel = new QLabel(tabVentes);
        venteSortLabel->setObjectName("venteSortLabel");
        venteSortLabel->setGeometry(QRect(520, 352, 80, 20));
        venteSortLabel->setStyleSheet(QString::fromUtf8("color: #1a3a52; font-weight: bold; background: transparent;"));
        venteSortCombo = new QComboBox(tabVentes);
        venteSortCombo->addItem(QString());
        venteSortCombo->addItem(QString());
        venteSortCombo->addItem(QString());
        venteSortCombo->setObjectName("venteSortCombo");
        venteSortCombo->setGeometry(QRect(610, 350, 150, 24));
        venteSortCombo->setStyleSheet(QString::fromUtf8("QComboBox { background-color: #2d2d2d; color: #ffffff; border-radius: 4px; padding: 2px 6px; border: 1px solid #3d3d3d; }"));
        tabWidgetStockVentes->addTab(tabVentes, QString());
        tabHistorique = new QWidget();
        tabHistorique->setObjectName("tabHistorique");
        frameHistorique = new QFrame(tabHistorique);
        frameHistorique->setObjectName("frameHistorique");
        frameHistorique->setGeometry(QRect(20, 110, 1180, 450));
        frameHistorique->setStyleSheet(QString::fromUtf8("border-radius: 10px; background-color: #1a3a52; border: 2px solid #3498db;"));
        frameHistorique->setFrameShape(QFrame::Shape::StyledPanel);
        frameHistorique->setFrameShadow(QFrame::Shadow::Raised);
        tableHistorique = new QTableWidget(frameHistorique);
        tableHistorique->setObjectName("tableHistorique");
        tableHistorique->setGeometry(QRect(15, 15, 1150, 510));
        tableHistorique->setStyleSheet(QString::fromUtf8("QTableWidget {\n"
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
"}"));
        btnExportHistoriquePdf = new QPushButton(tabHistorique);
        btnExportHistoriquePdf->setObjectName("btnExportHistoriquePdf");
        btnExportHistoriquePdf->setGeometry(QRect(1080, 570, 121, 32));
        btnExportHistoriquePdf->setStyleSheet(QString::fromUtf8("QPushButton { background-color: #e74c3c; color: white; font-weight: bold; border-radius: 4px; padding: 5px 10px; } QPushButton:hover { background-color: #c0392b; }"));
        tabWidgetStockVentes->addTab(tabHistorique, QString());
        tabAssistantIA = new QWidget();
        tabAssistantIA->setObjectName("tabAssistantIA");
        frameAssistantIA = new QFrame(tabAssistantIA);
        frameAssistantIA->setObjectName("frameAssistantIA");
        frameAssistantIA->setGeometry(QRect(20, 110, 1180, 450));
        frameAssistantIA->setStyleSheet(QString::fromUtf8("QFrame {\n"
"    background-color: #232323;\n"
"    border-radius: 8px;\n"
"    border: 2px solid #3498db;\n"
"}\n"
"QTextEdit {\n"
"    background-color: #2d2d2d;\n"
"    color: #ffffff;\n"
"    border: 1px solid #3d3d3d;\n"
"    border-radius: 4px;\n"
"    font-family: \"Segoe UI\";\n"
"    font-size: 12px;\n"
"}"));
        frameAssistantIA->setFrameShape(QFrame::Shape::StyledPanel);
        frameAssistantIA->setFrameShadow(QFrame::Shadow::Raised);
        textEditAssistant = new QTextEdit(frameAssistantIA);
        textEditAssistant->setObjectName("textEditAssistant");
        textEditAssistant->setGeometry(QRect(10, 10, 1160, 520));
        textEditAssistant->setReadOnly(true);
        lineEditAssistant = new QLineEdit(tabAssistantIA);
        lineEditAssistant->setObjectName("lineEditAssistant");
        lineEditAssistant->setGeometry(QRect(20, 570, 1040, 32));
        lineEditAssistant->setStyleSheet(QString::fromUtf8("background-color: #2d2d2d; color: #ffffff; border-radius: 6px; padding: 4px 8px; border: 1px solid #3d3d3d;"));
        btnAssistantGo = new QPushButton(tabAssistantIA);
        btnAssistantGo->setObjectName("btnAssistantGo");
        btnAssistantGo->setGeometry(QRect(1070, 570, 120, 32));
        btnAssistantGo->setStyleSheet(QString::fromUtf8("QPushButton { background-color: #3498db; color: white; font-weight: bold; border-radius: 4px; padding: 5px 10px; } QPushButton:hover { background-color: #2980b9; }"));
        tabWidgetStockVentes->addTab(tabAssistantIA, QString());
        tabSource = new QWidget();
        tabSource->setObjectName("tabSource");
        frameSourceMain = new QFrame(tabSource);
        frameSourceMain->setObjectName("frameSourceMain");
        frameSourceMain->setGeometry(QRect(40, 120, 1160, 420));
        frameSourceMain->setStyleSheet(QString::fromUtf8("QFrame {\n"
"    background-color: #232323;\n"
"    border-radius: 8px;\n"
"    border: 2px solid #3498db;\n"
"}\n"
"QPushButton {\n"
"    background-color: #3498db;\n"
"    color: white;\n"
"    font-weight: bold;\n"
"    border-radius: 8px;\n"
"    padding: 8px 12px;\n"
"}\n"
"QPushButton:hover {\n"
"    background-color: #2980b9;\n"
"}"));
        frameSourceMain->setFrameShape(QFrame::Shape::StyledPanel);
        frameSourceMain->setFrameShadow(QFrame::Shadow::Raised);
        btnSourceAny = new QPushButton(frameSourceMain);
        btnSourceAny->setObjectName("btnSourceAny");
        btnSourceAny->setGeometry(QRect(200, 210, 761, 61));
        btnSourceAny->setFont(font1);
        btnSourceAny->setStyleSheet(QString::fromUtf8("QPushButton { background-color: #3498db; color: white; font-weight: bold; border-radius: 8px; padding: 8px 12px; } QPushButton:hover { background-color: #2980b9; }"));
        tabWidgetStockVentes->addTab(tabSource, QString());

        verticalLayout_stock->addWidget(tabWidgetStockVentes);

        stackedWidget->addWidget(pageStock);

        horizontalLayout->addWidget(stackedWidget);

        mainStackedWidget->addWidget(dashboardPage);
        welcomePage = new QWidget();
        welcomePage->setObjectName("welcomePage");
        label_welcome = new QLabel(welcomePage);
        label_welcome->setObjectName("label_welcome");
        label_welcome->setGeometry(QRect(11, 90, 1419, 894));
        QFont font11;
        font11.setFamilies({QString::fromUtf8("Segoe UI")});
        font11.setPointSize(28);
        font11.setBold(true);
        label_welcome->setFont(font11);
        label_welcome->setAlignment(Qt::AlignmentFlag::AlignCenter);
        widget_5 = new QWidget(welcomePage);
        widget_5->setObjectName("widget_5");
        widget_5->setGeometry(QRect(490, 180, 551, 531));
        widget_5->setStyleSheet(QString::fromUtf8("QPushButton#pushButton_6, #pushButton_3{\n"
"	background-color: qlineargradient(spread:pad, x1:0, y1:0.505682, x2:1, y2:0.477, stop:0 rgba(11, 131, 120, 219), stop:1 rgba(85, 98, 112, 226));\n"
"	color:rgba(255, 255, 255, 210);\n"
"	border-radius:5px;\n"
"}\n"
"\n"
"QPushButton#pushButton_6:hover, #pushButton_3:hover{\n"
"	background-color: qlineargradient(spread:pad, x1:0, y1:0.505682, x2:1, y2:0.477, stop:0 rgba(150, 123, 111, 219), stop:1 rgba(85, 81, 84, 226));\n"
"}\n"
"\n"
"QPushButton#pushButton_6:pressed, #pushButton_3:pressed{\n"
"	padding-left:5px;\n"
"	padding-top:5px;\n"
"	background-color:rgba(150, 123, 111, 255);\n"
"}\n"
"\n"
"QPushButton#pushButton_2, #pushButton_4{\n"
"	background-color: rgba(0, 0, 0, 0);\n"
"	color:rgba(85, 98, 112, 255);\n"
"}\n"
"\n"
"QPushButton#pushButton_2:hover,  #pushButton_4:hover{\n"
"	color: rgba(131, 96, 53, 255);\n"
"}\n"
"\n"
"QPushButton#pushButton_2:pressed, #pushButton_4:pressed, #pushButton_5:pressed{\n"
"	padding-left:5px;\n"
"	padding-top:5px;\n"
"	color:rg"
                        "ba(91, 88, 53, 255);\n"
"}\n"
"\n"
""));
        label_20 = new QLabel(widget_5);
        label_20->setObjectName("label_20");
        label_20->setGeometry(QRect(40, 30, 280, 430));
        label_20->setStyleSheet(QString::fromUtf8("border-top-left-radius: 50px;\n"
"border-image: url(:/new/prefix1/background.jpg);"));
        label_28 = new QLabel(widget_5);
        label_28->setObjectName("label_28");
        label_28->setGeometry(QRect(40, 30, 280, 430));
        label_28->setStyleSheet(QString::fromUtf8("background-color:rgba(0, 0, 0, 80);\n"
"border-top-left-radius: 50px;"));
        label_32 = new QLabel(widget_5);
        label_32->setObjectName("label_32");
        label_32->setGeometry(QRect(270, 30, 240, 430));
        label_32->setStyleSheet(QString::fromUtf8("background-color:rgba(255, 255, 255, 255);\n"
"border-bottom-right-radius: 50px;"));
        label_33 = new QLabel(widget_5);
        label_33->setObjectName("label_33");
        label_33->setGeometry(QRect(340, 70, 100, 71));
        QFont font12;
        font12.setPointSize(20);
        font12.setBold(true);
        label_33->setFont(font12);
        label_33->setStyleSheet(QString::fromUtf8("color:rgba(0, 0, 0, 200);"));
        lineEdit_5 = new QLineEdit(widget_5);
        lineEdit_5->setObjectName("lineEdit_5");
        lineEdit_5->setGeometry(QRect(295, 150, 190, 40));
        QFont font13;
        font13.setPointSize(10);
        lineEdit_5->setFont(font13);
        lineEdit_5->setStyleSheet(QString::fromUtf8("background-color:rgba(0, 0, 0, 0);\n"
"border:none;\n"
"border-bottom:2px solid rgba(46, 82, 101, 200);\n"
"color:rgba(0, 0, 0, 240);\n"
"padding-bottom:7px;"));
        lineEdit_6 = new QLineEdit(widget_5);
        lineEdit_6->setObjectName("lineEdit_6");
        lineEdit_6->setGeometry(QRect(295, 215, 190, 40));
        lineEdit_6->setFont(font13);
        lineEdit_6->setStyleSheet(QString::fromUtf8("background-color:rgba(0, 0, 0, 0);\n"
"border:none;\n"
"border-bottom:2px solid rgba(46, 82, 101, 200);\n"
"color:rgba(0, 0, 0, 240);\n"
"padding-bottom:7px;"));
        lineEdit_6->setEchoMode(QLineEdit::EchoMode::Password);
        pushButton_3 = new QPushButton(widget_5);
        pushButton_3->setObjectName("pushButton_3");
        pushButton_3->setGeometry(QRect(295, 295, 190, 40));
        QFont font14;
        font14.setWeight(QFont::Medium);
        pushButton_3->setFont(font14);
        label_35 = new QLabel(widget_5);
        label_35->setObjectName("label_35");
        label_35->setGeometry(QRect(40, 80, 230, 130));
        label_35->setStyleSheet(QString::fromUtf8("background-color:rgba(0, 0, 0, 75);"));
        label_36 = new QLabel(widget_5);
        label_36->setObjectName("label_36");
        label_36->setGeometry(QRect(50, 80, 180, 40));
        QFont font15;
        font15.setPointSize(22);
        font15.setBold(true);
        label_36->setFont(font15);
        label_36->setStyleSheet(QString::fromUtf8("color:rgba(255, 255, 255, 200);"));
        label_37 = new QLabel(widget_5);
        label_37->setObjectName("label_37");
        label_37->setGeometry(QRect(50, 145, 220, 50));
        QFont font16;
        font16.setPointSize(10);
        font16.setBold(true);
        label_37->setFont(font16);
        label_37->setStyleSheet(QString::fromUtf8("color:rgba(255, 255, 255, 170);"));
        pushButton_6 = new QPushButton(widget_5);
        pushButton_6->setObjectName("pushButton_6");
        pushButton_6->setGeometry(QRect(295, 350, 190, 40));
        pushButton_6->setFont(font14);
        pushButton = new QPushButton(widget_5);
        pushButton->setObjectName("pushButton");
        pushButton->setGeometry(QRect(300, 400, 171, 29));
        pushButton->setStyleSheet(QString::fromUtf8("color: rgb(0, 0, 0);"));
        label_38 = new QLabel(welcomePage);
        label_38->setObjectName("label_38");
        label_38->setGeometry(QRect(280, 120, 1041, 611));
        label_38->setStyleSheet(QString::fromUtf8("border-image: url(:/new/prefix1/background.jpg);\n"
"border-top-left-radius: 50px;\n"
"border-bottom-right-radius: 50px;"));
        mainStackedWidget->addWidget(welcomePage);
        label_38->raise();
        label_welcome->raise();
        widget_5->raise();
        placeholderPage1 = new QWidget();
        placeholderPage1->setObjectName("placeholderPage1");
        label_p1 = new QLabel(placeholderPage1);
        label_p1->setObjectName("label_p1");
        label_p1->setGeometry(QRect(11, 11, 1419, 894));
        label_p1->setFont(font11);
        label_p1->setAlignment(Qt::AlignmentFlag::AlignCenter);
        label_39 = new QLabel(placeholderPage1);
        label_39->setObjectName("label_39");
        label_39->setGeometry(QRect(280, 120, 1041, 611));
        label_39->setStyleSheet(QString::fromUtf8("border-image: url(:/new/prefix1/background.jpg);\n"
"border-top-left-radius: 50px;\n"
"border-bottom-right-radius: 50px;"));
        widget_6 = new QWidget(placeholderPage1);
        widget_6->setObjectName("widget_6");
        widget_6->setGeometry(QRect(490, 180, 550, 500));
        widget_6->setStyleSheet(QString::fromUtf8("QPushButton#pushButton, #pushButton_8{\n"
"	background-color: qlineargradient(spread:pad, x1:0, y1:0.505682, x2:1, y2:0.477, stop:0 rgba(11, 131, 120, 219), stop:1 rgba(85, 98, 112, 226));\n"
"	color:rgba(255, 255, 255, 210);\n"
"	border-radius:5px;\n"
"}\n"
"\n"
"QPushButton#pushButton:hover, #pushButton_8:hover{\n"
"	background-color: qlineargradient(spread:pad, x1:0, y1:0.505682, x2:1, y2:0.477, stop:0 rgba(150, 123, 111, 219), stop:1 rgba(85, 81, 84, 226));\n"
"}\n"
"\n"
"QPushButton#pushButton:pressed, #pushButton_8:pressed{\n"
"	padding-left:5px;\n"
"	padding-top:5px;\n"
"	background-color:rgba(150, 123, 111, 255);\n"
"}\n"
"\n"
"QPushButton#pushButton_2, #pushButton_3, #pushButton_4, #pushButton_5{\n"
"	background-color: rgba(0, 0, 0, 0);\n"
"	color:rgba(85, 98, 112, 255);\n"
"}\n"
"\n"
"QPushButton#pushButton_2:hover, #pushButton_3:hover, #pushButton_4:hover, #pushButton_5:hover{\n"
"	color: rgba(131, 96, 53, 255);\n"
"}\n"
"\n"
"QPushButton#pushButton_2:pressed, #pushButton_3:pressed, #pushButton_4:pr"
                        "essed, #pushButton_5:pressed{\n"
"	padding-left:5px;\n"
"	padding-top:5px;\n"
"	color:rgba(91, 88, 53, 255);\n"
"}\n"
"\n"
""));
        label_40 = new QLabel(widget_6);
        label_40->setObjectName("label_40");
        label_40->setGeometry(QRect(40, 30, 280, 430));
        label_40->setStyleSheet(QString::fromUtf8("border-top-left-radius: 50px;\n"
"border-image: url(:/new/prefix1/background.jpg);"));
        label_41 = new QLabel(widget_6);
        label_41->setObjectName("label_41");
        label_41->setGeometry(QRect(40, 30, 280, 430));
        label_41->setStyleSheet(QString::fromUtf8("background-color:rgba(0, 0, 0, 80);\n"
"border-top-left-radius: 50px;"));
        label_42 = new QLabel(widget_6);
        label_42->setObjectName("label_42");
        label_42->setGeometry(QRect(270, 30, 240, 430));
        label_42->setStyleSheet(QString::fromUtf8("background-color:rgba(255, 255, 255, 255);\n"
"border-bottom-right-radius: 50px;"));
        label_43 = new QLabel(widget_6);
        label_43->setObjectName("label_43");
        label_43->setGeometry(QRect(330, 70, 131, 71));
        label_43->setFont(font12);
        label_43->setStyleSheet(QString::fromUtf8("color:rgba(0, 0, 0, 200);"));
        label_44 = new QLabel(widget_6);
        label_44->setObjectName("label_44");
        label_44->setGeometry(QRect(40, 80, 230, 130));
        label_44->setStyleSheet(QString::fromUtf8("background-color:rgba(0, 0, 0, 75);"));
        label_45 = new QLabel(widget_6);
        label_45->setObjectName("label_45");
        label_45->setGeometry(QRect(50, 80, 180, 40));
        label_45->setFont(font15);
        label_45->setStyleSheet(QString::fromUtf8("color:rgba(255, 255, 255, 200);"));
        label_46 = new QLabel(widget_6);
        label_46->setObjectName("label_46");
        label_46->setGeometry(QRect(50, 145, 220, 50));
        label_46->setFont(font16);
        label_46->setStyleSheet(QString::fromUtf8("color:rgba(255, 255, 255, 170);"));
        pushButton_8 = new QPushButton(widget_6);
        pushButton_8->setObjectName("pushButton_8");
        pushButton_8->setGeometry(QRect(295, 350, 190, 40));
        pushButton_8->setFont(font14);
        label_photo_3 = new QLabel(widget_6);
        label_photo_3->setObjectName("label_photo_3");
        label_photo_3->setGeometry(QRect(310, 140, 171, 187));
        label_photo_3->setMinimumSize(QSize(120, 120));
        label_photo_3->setAutoFillBackground(false);
        label_photo_3->setStyleSheet(QString::fromUtf8("border: 2px dashed #3498db; color: #7f8c8d;"));
        label_photo_3->setAlignment(Qt::AlignmentFlag::AlignCenter);
        mainStackedWidget->addWidget(placeholderPage1);
        placeholderPage2 = new QWidget();
        placeholderPage2->setObjectName("placeholderPage2");
        label_p2 = new QLabel(placeholderPage2);
        label_p2->setObjectName("label_p2");
        label_p2->setGeometry(QRect(11, 11, 1419, 894));
        label_p2->setFont(font11);
        label_p2->setAlignment(Qt::AlignmentFlag::AlignCenter);
        label_47 = new QLabel(placeholderPage2);
        label_47->setObjectName("label_47");
        label_47->setGeometry(QRect(280, 120, 1041, 611));
        label_47->setStyleSheet(QString::fromUtf8("border-image: url(:/new/prefix1/background.jpg);\n"
"border-top-left-radius: 50px;\n"
"border-bottom-right-radius: 50px;"));
        widget_7 = new QWidget(placeholderPage2);
        widget_7->setObjectName("widget_7");
        widget_7->setGeometry(QRect(490, 180, 550, 500));
        widget_7->setStyleSheet(QString::fromUtf8("QPushButton#pushButton_9,#pushButton_11{\n"
"	background-color: qlineargradient(spread:pad, x1:0, y1:0.505682, x2:1, y2:0.477, stop:0 rgba(11, 131, 120, 219), stop:1 rgba(85, 98, 112, 226));\n"
"	color:rgba(255, 255, 255, 210);\n"
"	border-radius:5px;\n"
"}\n"
"\n"
"QPushButton#pushButton_9:hover,#pushButton_11:hover{\n"
"	background-color: qlineargradient(spread:pad, x1:0, y1:0.505682, x2:1, y2:0.477, stop:0 rgba(150, 123, 111, 219), stop:1 rgba(85, 81, 84, 226));\n"
"}\n"
"\n"
"QPushButton#pushButton_9:pressed,#pushButton_11:pressed{\n"
"	padding-left:5px;\n"
"	padding-top:5px;\n"
"	background-color:rgba(150, 123, 111, 255);\n"
"}\n"
"\n"
"QPushButton #pushButton_3, #pushButton_4, #pushButton_5{\n"
"	background-color: rgba(0, 0, 0, 0);\n"
"	color:rgba(85, 98, 112, 255);\n"
"}\n"
"\n"
"QPushButton#pushButton_3:hover, #pushButton_4:hover, #pushButton_5:hover{\n"
"	color: rgba(131, 96, 53, 255);\n"
"}\n"
"\n"
"QPushButton#pushButton_3:pressed, #pushButton_4:pressed, #pushButton_5:pressed{\n"
"	padding-left:5px;"
                        "\n"
"	padding-top:5px;\n"
"	color:rgba(91, 88, 53, 255);\n"
"}\n"
"\n"
""));
        label_48 = new QLabel(widget_7);
        label_48->setObjectName("label_48");
        label_48->setGeometry(QRect(40, 30, 280, 430));
        label_48->setStyleSheet(QString::fromUtf8("border-top-left-radius: 50px;\n"
"border-image: url(:/new/prefix1/background.jpg);"));
        label_49 = new QLabel(widget_7);
        label_49->setObjectName("label_49");
        label_49->setGeometry(QRect(40, 30, 280, 430));
        label_49->setStyleSheet(QString::fromUtf8("background-color:rgba(0, 0, 0, 80);\n"
"border-top-left-radius: 50px;"));
        label_50 = new QLabel(widget_7);
        label_50->setObjectName("label_50");
        label_50->setGeometry(QRect(270, 30, 240, 430));
        label_50->setStyleSheet(QString::fromUtf8("background-color:rgba(255, 255, 255, 255);\n"
"border-bottom-right-radius: 50px;"));
        label_51 = new QLabel(widget_7);
        label_51->setObjectName("label_51");
        label_51->setGeometry(QRect(270, 70, 351, 71));
        QFont font17;
        font17.setPointSize(15);
        font17.setBold(true);
        label_51->setFont(font17);
        label_51->setStyleSheet(QString::fromUtf8("color:rgba(0, 0, 0, 200);"));
        lineEdit_7 = new QLineEdit(widget_7);
        lineEdit_7->setObjectName("lineEdit_7");
        lineEdit_7->setGeometry(QRect(295, 150, 190, 40));
        lineEdit_7->setFont(font13);
        lineEdit_7->setStyleSheet(QString::fromUtf8("background-color:rgba(0, 0, 0, 0);\n"
"border:none;\n"
"border-bottom:2px solid rgba(46, 82, 101, 200);\n"
"color:rgba(0, 0, 0, 240);\n"
"padding-bottom:7px;"));
        lineEdit_7->setEchoMode(QLineEdit::EchoMode::Password);
        lineEdit_9 = new QLineEdit(widget_7);
        lineEdit_9->setObjectName("lineEdit_9");
        lineEdit_9->setGeometry(QRect(295, 215, 190, 40));
        lineEdit_9->setFont(font13);
        lineEdit_9->setStyleSheet(QString::fromUtf8("background-color:rgba(0, 0, 0, 0);\n"
"border:none;\n"
"border-bottom:2px solid rgba(46, 82, 101, 200);\n"
"color:rgba(0, 0, 0, 240);\n"
"padding-bottom:7px;"));
        lineEdit_9->setEchoMode(QLineEdit::EchoMode::Password);
        pushButton_9 = new QPushButton(widget_7);
        pushButton_9->setObjectName("pushButton_9");
        pushButton_9->setGeometry(QRect(295, 295, 190, 40));
        pushButton_9->setFont(font14);
        label_52 = new QLabel(widget_7);
        label_52->setObjectName("label_52");
        label_52->setGeometry(QRect(40, 80, 230, 130));
        label_52->setStyleSheet(QString::fromUtf8("background-color:rgba(0, 0, 0, 75);"));
        label_53 = new QLabel(widget_7);
        label_53->setObjectName("label_53");
        label_53->setGeometry(QRect(50, 80, 180, 40));
        label_53->setFont(font15);
        label_53->setStyleSheet(QString::fromUtf8("color:rgba(255, 255, 255, 200);"));
        label_54 = new QLabel(widget_7);
        label_54->setObjectName("label_54");
        label_54->setGeometry(QRect(50, 145, 220, 50));
        label_54->setFont(font16);
        label_54->setStyleSheet(QString::fromUtf8("color:rgba(255, 255, 255, 170);"));
        pushButton_11 = new QPushButton(widget_7);
        pushButton_11->setObjectName("pushButton_11");
        pushButton_11->setGeometry(QRect(295, 350, 190, 40));
        pushButton_11->setFont(font14);
        mainStackedWidget->addWidget(placeholderPage2);
        placeholderPage3 = new QWidget();
        placeholderPage3->setObjectName("placeholderPage3");
        label_p3 = new QLabel(placeholderPage3);
        label_p3->setObjectName("label_p3");
        label_p3->setGeometry(QRect(11, 11, 1419, 894));
        label_p3->setFont(font11);
        label_p3->setAlignment(Qt::AlignmentFlag::AlignCenter);
        label_55 = new QLabel(placeholderPage3);
        label_55->setObjectName("label_55");
        label_55->setGeometry(QRect(280, 120, 1041, 611));
        label_55->setStyleSheet(QString::fromUtf8("border-image: url(:/new/prefix1/background.jpg);\n"
"border-top-left-radius: 50px;\n"
"border-bottom-right-radius: 50px;"));
        widget_8 = new QWidget(placeholderPage3);
        widget_8->setObjectName("widget_8");
        widget_8->setGeometry(QRect(490, 180, 550, 500));
        widget_8->setStyleSheet(QString::fromUtf8("QPushButton#pushButton, #pushButton_10{\n"
"	background-color: qlineargradient(spread:pad, x1:0, y1:0.505682, x2:1, y2:0.477, stop:0 rgba(11, 131, 120, 219), stop:1 rgba(85, 98, 112, 226));\n"
"	color:rgba(255, 255, 255, 210);\n"
"	border-radius:5px;\n"
"}\n"
"\n"
"QPushButton#pushButton:hover, #pushButton_10:hover{\n"
"	background-color: qlineargradient(spread:pad, x1:0, y1:0.505682, x2:1, y2:0.477, stop:0 rgba(150, 123, 111, 219), stop:1 rgba(85, 81, 84, 226));\n"
"}\n"
"\n"
"QPushButton#pushButton:pressed,#pushButton_10:pressed{\n"
"	padding-left:5px;\n"
"	padding-top:5px;\n"
"	background-color:rgba(150, 123, 111, 255);\n"
"}\n"
"\n"
"QPushButton#pushButton_2, #pushButton_3, #pushButton_5{\n"
"	background-color: rgba(0, 0, 0, 0);\n"
"	color:rgba(85, 98, 112, 255);\n"
"}\n"
"\n"
"QPushButton#pushButton_2:hover, #pushButton_3:hover, #pushButton_5:hover{\n"
"	color: rgba(131, 96, 53, 255);\n"
"}\n"
"\n"
"QPushButton#pushButton_2:pressed, #pushButton_3:pressed, #pushButton_5:pressed{\n"
"	padding-left:5px;\n"
""
                        "	padding-top:5px;\n"
"	color:rgba(91, 88, 53, 255);\n"
"}\n"
"\n"
""));
        label_56 = new QLabel(widget_8);
        label_56->setObjectName("label_56");
        label_56->setGeometry(QRect(40, 30, 280, 430));
        label_56->setStyleSheet(QString::fromUtf8("border-top-left-radius: 50px;\n"
"border-image: url(:/new/prefix1/background.jpg);"));
        label_57 = new QLabel(widget_8);
        label_57->setObjectName("label_57");
        label_57->setGeometry(QRect(40, 30, 280, 430));
        label_57->setStyleSheet(QString::fromUtf8("background-color:rgba(0, 0, 0, 80);\n"
"border-top-left-radius: 50px;"));
        label_58 = new QLabel(widget_8);
        label_58->setObjectName("label_58");
        label_58->setGeometry(QRect(270, 30, 240, 430));
        label_58->setStyleSheet(QString::fromUtf8("background-color:rgba(255, 255, 255, 255);\n"
"border-bottom-right-radius: 50px;"));
        label_59 = new QLabel(widget_8);
        label_59->setObjectName("label_59");
        label_59->setGeometry(QRect(340, 70, 100, 71));
        label_59->setFont(font12);
        label_59->setStyleSheet(QString::fromUtf8("color:rgba(0, 0, 0, 200);"));
        lineEdit_10 = new QLineEdit(widget_8);
        lineEdit_10->setObjectName("lineEdit_10");
        lineEdit_10->setGeometry(QRect(295, 215, 190, 40));
        lineEdit_10->setFont(font13);
        lineEdit_10->setStyleSheet(QString::fromUtf8("background-color:rgba(0, 0, 0, 0);\n"
"border:none;\n"
"border-bottom:2px solid rgba(46, 82, 101, 200);\n"
"color:rgba(0, 0, 0, 240);\n"
"padding-bottom:7px;"));
        lineEdit_10->setEchoMode(QLineEdit::EchoMode::Password);
        pushButton_10 = new QPushButton(widget_8);
        pushButton_10->setObjectName("pushButton_10");
        pushButton_10->setGeometry(QRect(295, 295, 190, 40));
        pushButton_10->setFont(font14);
        label_60 = new QLabel(widget_8);
        label_60->setObjectName("label_60");
        label_60->setGeometry(QRect(40, 80, 230, 130));
        label_60->setStyleSheet(QString::fromUtf8("background-color:rgba(0, 0, 0, 75);"));
        label_61 = new QLabel(widget_8);
        label_61->setObjectName("label_61");
        label_61->setGeometry(QRect(50, 80, 180, 40));
        label_61->setFont(font15);
        label_61->setStyleSheet(QString::fromUtf8("color:rgba(255, 255, 255, 200);"));
        label_62 = new QLabel(widget_8);
        label_62->setObjectName("label_62");
        label_62->setGeometry(QRect(50, 145, 220, 50));
        label_62->setFont(font16);
        label_62->setStyleSheet(QString::fromUtf8("color:rgba(255, 255, 255, 170);"));
        mainStackedWidget->addWidget(placeholderPage3);

        mainVerticalLayout->addWidget(mainStackedWidget);

        MainWindow->setCentralWidget(centralwidget);

        retranslateUi(MainWindow);

        mainStackedWidget->setCurrentIndex(0);
        stackedWidget->setCurrentIndex(4);
        stackedWidgetEmployes->setCurrentIndex(2);
        tabWidget_employee->setCurrentIndex(3);
        stackedWidgetPecheurs->setCurrentIndex(2);
        tabWidget_2->setCurrentIndex(1);
        tabWidget->setCurrentIndex(1);
        mainTabWidget->setCurrentIndex(0);
        tabWidgetStockVentes->setCurrentIndex(2);


        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QCoreApplication::translate("MainWindow", "Smart Fishing & Port Management", nullptr));
        lblTitle->setText(QCoreApplication::translate("MainWindow", " ATLAS  ", nullptr));
        btnEmployes->setText(QCoreApplication::translate("MainWindow", " Employ\303\251s", nullptr));
        btnPecheurs->setText(QCoreApplication::translate("MainWindow", " P\303\252cheurs", nullptr));
        btnBateaux->setText(QCoreApplication::translate("MainWindow", " Bateaux", nullptr));
        btnMaintenance->setText(QCoreApplication::translate("MainWindow", "Equipement", nullptr));
        btnQuais->setText(QCoreApplication::translate("MainWindow", " Quais", nullptr));
        btnStock_2->setText(QCoreApplication::translate("MainWindow", " Stock", nullptr));
        btnStock->setText(QCoreApplication::translate("MainWindow", "Quiter", nullptr));
        title_label_2->setText(QCoreApplication::translate("MainWindow", "Systeme D'information D'employ\303\251e", nullptr));
        btnSubEmpl3->setText(QCoreApplication::translate("MainWindow", "Liste", nullptr));
        btnSubEmpl4_2->setText(QCoreApplication::translate("MainWindow", "Statistique", nullptr));
        btnSubEmpl5->setText(QCoreApplication::translate("MainWindow", "Disponibilit\303\251", nullptr));
        label_lname->setText(QCoreApplication::translate("MainWindow", "Nom", nullptr));
        label_id->setText(QCoreApplication::translate("MainWindow", "Employee ID (Unique)", nullptr));
        label_photo->setText(QCoreApplication::translate("MainWindow", "Photo Placeholder", nullptr));
        label_cin->setText(QCoreApplication::translate("MainWindow", "CIN (National ID)", nullptr));
        label_fname->setText(QCoreApplication::translate("MainWindow", "Prenom", nullptr));
        btn_upload_photo->setText(QCoreApplication::translate("MainWindow", "Upload Photo", nullptr));
        tabWidget_employee->setTabText(tabWidget_employee->indexOf(tab_id), QCoreApplication::translate("MainWindow", "\360\237\224\271 Identification", nullptr));
        label_job->setText(QCoreApplication::translate("MainWindow", "Occupation", nullptr));
        label_dept->setText(QCoreApplication::translate("MainWindow", "Departement", nullptr));
        combo_dept->setItemText(0, QCoreApplication::translate("MainWindow", "Port", nullptr));
        combo_dept->setItemText(1, QCoreApplication::translate("MainWindow", "S\303\251curit\303\251", nullptr));
        combo_dept->setItemText(2, QCoreApplication::translate("MainWindow", "Maintenance", nullptr));
        combo_dept->setItemText(3, QCoreApplication::translate("MainWindow", "Administration", nullptr));

        label_hdate->setText(QCoreApplication::translate("MainWindow", "Date d\342\200\231embauchement", nullptr));
        label_status->setText(QCoreApplication::translate("MainWindow", "Status", nullptr));
        combo_status->setItemText(0, QCoreApplication::translate("MainWindow", "Actif", nullptr));
        combo_status->setItemText(1, QCoreApplication::translate("MainWindow", "Inactif", nullptr));
        combo_status->setItemText(2, QCoreApplication::translate("MainWindow", "En cong\303\251", nullptr));

        tabWidget_employee->setTabText(tabWidget_employee->indexOf(tab_pro), QCoreApplication::translate("MainWindow", "\360\237\224\271 Info Personnelle", nullptr));
        label_phone->setText(QCoreApplication::translate("MainWindow", "Numero de telephone", nullptr));
        label_email->setText(QCoreApplication::translate("MainWindow", "Adresse Email", nullptr));
        label_pwd->setText(QCoreApplication::translate("MainWindow", "Mot de passe", nullptr));
        label_city->setText(QCoreApplication::translate("MainWindow", "Ville", nullptr));
        label_addr->setText(QCoreApplication::translate("MainWindow", "Addresse", nullptr));
        tabWidget_employee->setTabText(tabWidget_employee->indexOf(tab_contact), QCoreApplication::translate("MainWindow", "\360\237\224\271 Coordonn\303\251es", nullptr));
        label_certs->setText(QCoreApplication::translate("MainWindow", "Certifications", nullptr));
        lineEdit_certs->setPlaceholderText(QCoreApplication::translate("MainWindow", "e.g. Navigation, S\303\251curit\303\251, Secours", nullptr));
        label_slevel->setText(QCoreApplication::translate("MainWindow", "Niveaux", nullptr));
        combo_slevel->setItemText(0, QCoreApplication::translate("MainWindow", "D\303\251butant", nullptr));
        combo_slevel->setItemText(1, QCoreApplication::translate("MainWindow", "Interm\303\251diaire", nullptr));
        combo_slevel->setItemText(2, QCoreApplication::translate("MainWindow", "Avanc\303\251", nullptr));

        label_lic->setText(QCoreApplication::translate("MainWindow", "Numero de License ", nullptr));
        label_exp->setText(QCoreApplication::translate("MainWindow", "Date d'expiration", nullptr));
        label_shift->setText(QCoreApplication::translate("MainWindow", "Quart", nullptr));
        combo_shift->setItemText(0, QCoreApplication::translate("MainWindow", "Matin", nullptr));
        combo_shift->setItemText(1, QCoreApplication::translate("MainWindow", "Nuit", nullptr));
        combo_shift->setItemText(2, QCoreApplication::translate("MainWindow", "Rotation", nullptr));

        tabWidget_employee->setTabText(tabWidget_employee->indexOf(tab_skills), QCoreApplication::translate("MainWindow", "\360\237\224\271 Certification", nullptr));
        add_btn_2->setText(QCoreApplication::translate("MainWindow", "Ajouter", nullptr));
        clear_btn_2->setText(QCoreApplication::translate("MainWindow", "Nettoyer", nullptr));
        update_btn_2->setText(QCoreApplication::translate("MainWindow", "Modifier", nullptr));
        delete_btn_2->setText(QCoreApplication::translate("MainWindow", "Suprimer", nullptr));
        search_input_2->setPlaceholderText(QCoreApplication::translate("MainWindow", "talck hear", nullptr));
        clear_btn_3->setText(QCoreApplication::translate("MainWindow", "Reconnaissance vocale", nullptr));
        search_input->setPlaceholderText(QCoreApplication::translate("MainWindow", "Search by Name, CIN, Dept...", nullptr));
        search_btn_2->setText(QCoreApplication::translate("MainWindow", "Recherche", nullptr));
        sort_combo->setItemText(0, QCoreApplication::translate("MainWindow", "ID (Asc)", nullptr));
        sort_combo->setItemText(1, QCoreApplication::translate("MainWindow", "ID (Desc)", nullptr));
        sort_combo->setItemText(2, QCoreApplication::translate("MainWindow", "Name (A-Z)", nullptr));
        sort_combo->setItemText(3, QCoreApplication::translate("MainWindow", "Name (Z-A)", nullptr));

        sort_combo->setPlaceholderText(QCoreApplication::translate("MainWindow", "Trie", nullptr));
        export_excel_btn->setText(QCoreApplication::translate("MainWindow", "Exporter Excel", nullptr));
        QTableWidgetItem *___qtablewidgetitem = tableWidget_2->horizontalHeaderItem(0);
        ___qtablewidgetitem->setText(QCoreApplication::translate("MainWindow", "Employee ID (Unique)", nullptr));
        QTableWidgetItem *___qtablewidgetitem1 = tableWidget_2->horizontalHeaderItem(1);
        ___qtablewidgetitem1->setText(QCoreApplication::translate("MainWindow", "CIN (National ID)", nullptr));
        QTableWidgetItem *___qtablewidgetitem2 = tableWidget_2->horizontalHeaderItem(2);
        ___qtablewidgetitem2->setText(QCoreApplication::translate("MainWindow", "Prenom", nullptr));
        QTableWidgetItem *___qtablewidgetitem3 = tableWidget_2->horizontalHeaderItem(3);
        ___qtablewidgetitem3->setText(QCoreApplication::translate("MainWindow", "Nom", nullptr));
        QTableWidgetItem *___qtablewidgetitem4 = tableWidget_2->horizontalHeaderItem(4);
        ___qtablewidgetitem4->setText(QCoreApplication::translate("MainWindow", "Occupation", nullptr));
        QTableWidgetItem *___qtablewidgetitem5 = tableWidget_2->horizontalHeaderItem(5);
        ___qtablewidgetitem5->setText(QCoreApplication::translate("MainWindow", "Departement", nullptr));
        QTableWidgetItem *___qtablewidgetitem6 = tableWidget_2->horizontalHeaderItem(6);
        ___qtablewidgetitem6->setText(QCoreApplication::translate("MainWindow", "Date d\342\200\231embauchement", nullptr));
        QTableWidgetItem *___qtablewidgetitem7 = tableWidget_2->horizontalHeaderItem(7);
        ___qtablewidgetitem7->setText(QCoreApplication::translate("MainWindow", "Status", nullptr));
        QTableWidgetItem *___qtablewidgetitem8 = tableWidget_2->horizontalHeaderItem(8);
        ___qtablewidgetitem8->setText(QCoreApplication::translate("MainWindow", "Numero de telephone", nullptr));
        QTableWidgetItem *___qtablewidgetitem9 = tableWidget_2->horizontalHeaderItem(9);
        ___qtablewidgetitem9->setText(QCoreApplication::translate("MainWindow", "Adresse Email", nullptr));
        QTableWidgetItem *___qtablewidgetitem10 = tableWidget_2->horizontalHeaderItem(10);
        ___qtablewidgetitem10->setText(QCoreApplication::translate("MainWindow", "Mot de passe", nullptr));
        QTableWidgetItem *___qtablewidgetitem11 = tableWidget_2->horizontalHeaderItem(11);
        ___qtablewidgetitem11->setText(QCoreApplication::translate("MainWindow", "Ville", nullptr));
        QTableWidgetItem *___qtablewidgetitem12 = tableWidget_2->horizontalHeaderItem(12);
        ___qtablewidgetitem12->setText(QCoreApplication::translate("MainWindow", "Certifications", nullptr));
        QTableWidgetItem *___qtablewidgetitem13 = tableWidget_2->horizontalHeaderItem(13);
        ___qtablewidgetitem13->setText(QCoreApplication::translate("MainWindow", "Niveaux", nullptr));
        QTableWidgetItem *___qtablewidgetitem14 = tableWidget_2->horizontalHeaderItem(14);
        ___qtablewidgetitem14->setText(QCoreApplication::translate("MainWindow", "Numero de License ", nullptr));
        QTableWidgetItem *___qtablewidgetitem15 = tableWidget_2->horizontalHeaderItem(15);
        ___qtablewidgetitem15->setText(QCoreApplication::translate("MainWindow", "Date d'expiration", nullptr));
        QTableWidgetItem *___qtablewidgetitem16 = tableWidget_2->horizontalHeaderItem(16);
        ___qtablewidgetitem16->setText(QCoreApplication::translate("MainWindow", "Quart", nullptr));
        QTableWidgetItem *___qtablewidgetitem17 = tableWidget_2->verticalHeaderItem(0);
        ___qtablewidgetitem17->setText(QCoreApplication::translate("MainWindow", "New Row", nullptr));
        QTableWidgetItem *___qtablewidgetitem18 = tableWidget_2->verticalHeaderItem(1);
        ___qtablewidgetitem18->setText(QCoreApplication::translate("MainWindow", "New Row", nullptr));

        const bool __sortingEnabled = tableWidget_2->isSortingEnabled();
        tableWidget_2->setSortingEnabled(false);
        QTableWidgetItem *___qtablewidgetitem19 = tableWidget_2->item(0, 0);
        ___qtablewidgetitem19->setText(QCoreApplication::translate("MainWindow", "33070", nullptr));
        QTableWidgetItem *___qtablewidgetitem20 = tableWidget_2->item(0, 1);
        ___qtablewidgetitem20->setText(QCoreApplication::translate("MainWindow", "hamza", nullptr));
        QTableWidgetItem *___qtablewidgetitem21 = tableWidget_2->item(0, 2);
        ___qtablewidgetitem21->setText(QCoreApplication::translate("MainWindow", "saidane", nullptr));
        QTableWidgetItem *___qtablewidgetitem22 = tableWidget_2->item(0, 3);
        ___qtablewidgetitem22->setText(QCoreApplication::translate("MainWindow", "admin", nullptr));
        QTableWidgetItem *___qtablewidgetitem23 = tableWidget_2->item(0, 4);
        ___qtablewidgetitem23->setText(QCoreApplication::translate("MainWindow", "94962714", nullptr));
        QTableWidgetItem *___qtablewidgetitem24 = tableWidget_2->item(0, 5);
        ___qtablewidgetitem24->setText(QCoreApplication::translate("MainWindow", "tunis", nullptr));
        QTableWidgetItem *___qtablewidgetitem25 = tableWidget_2->item(0, 6);
        ___qtablewidgetitem25->setText(QCoreApplication::translate("MainWindow", "actif", nullptr));
        QTableWidgetItem *___qtablewidgetitem26 = tableWidget_2->item(0, 7);
        ___qtablewidgetitem26->setText(QCoreApplication::translate("MainWindow", "2026", nullptr));
        QTableWidgetItem *___qtablewidgetitem27 = tableWidget_2->item(0, 8);
        ___qtablewidgetitem27->setText(QCoreApplication::translate("MainWindow", "hh", nullptr));
        QTableWidgetItem *___qtablewidgetitem28 = tableWidget_2->item(0, 9);
        ___qtablewidgetitem28->setText(QCoreApplication::translate("MainWindow", "saidanehamza87@gmail.com", nullptr));
        tableWidget_2->setSortingEnabled(__sortingEnabled);

        lblMetierTitle->setText(QCoreApplication::translate("MainWindow", " Statistiques des Employ\303\251s", nullptr));
        lblTotalEmployees->setText(QCoreApplication::translate("MainWindow", " Total Employ\303\251s", nullptr));
        lblTotalEmployeesValue->setText(QCoreApplication::translate("MainWindow", "154", nullptr));
        lblActiveEmployees->setText(QCoreApplication::translate("MainWindow", " Actifs", nullptr));
        lblActiveEmployeesValue->setText(QCoreApplication::translate("MainWindow", "142", nullptr));
        lblOnLeave->setText(QCoreApplication::translate("MainWindow", " En Cong\303\251", nullptr));
        lblOnLeaveValue->setText(QCoreApplication::translate("MainWindow", "12", nullptr));
        lblCertExpiring->setText(QCoreApplication::translate("MainWindow", " Certif. Expirant", nullptr));
        lblCertExpiringValue->setText(QCoreApplication::translate("MainWindow", "3", nullptr));
        lblDeptTitle->setText(QCoreApplication::translate("MainWindow", " Distribution par D\303\251partement", nullptr));
        lblDept1->setText(QCoreApplication::translate("MainWindow", "Port", nullptr));
        lblDept2->setText(QCoreApplication::translate("MainWindow", "Maintenance", nullptr));
        lblDept3->setText(QCoreApplication::translate("MainWindow", "S\303\251curit\303\251", nullptr));
        lblDept4->setText(QCoreApplication::translate("MainWindow", "Administration", nullptr));
        lblSkillsTitle->setText(QCoreApplication::translate("MainWindow", " Niveau de Comp\303\251tences", nullptr));
        lblSkill1->setText(QCoreApplication::translate("MainWindow", "D\303\251butant", nullptr));
        lblSkill2->setText(QCoreApplication::translate("MainWindow", "Interm\303\251diaire", nullptr));
        lblSkill3->setText(QCoreApplication::translate("MainWindow", "Avanc\303\251", nullptr));
        lblShiftsTitle->setText(QCoreApplication::translate("MainWindow", " Horaires de Travail", nullptr));
        lblShift1->setText(QCoreApplication::translate("MainWindow", "Matin", nullptr));
        lblShift2->setText(QCoreApplication::translate("MainWindow", "Nuit", nullptr));
        lblCitiesTitle->setText(QCoreApplication::translate("MainWindow", " R\303\251partition G\303\251ographique", nullptr));
        lblCity1->setText(QCoreApplication::translate("MainWindow", "Tunis", nullptr));
        lblCity2->setText(QCoreApplication::translate("MainWindow", "Sfax", nullptr));
        lblCity3->setText(QCoreApplication::translate("MainWindow", "Autres", nullptr));
        lblAlertsTitle->setText(QCoreApplication::translate("MainWindow", " Alertes Importantes", nullptr));
        lblAlert1->setText(QCoreApplication::translate("MainWindow", "\342\200\242 3 certifications expirent dans 30 jours", nullptr));
        lblAlert2->setText(QCoreApplication::translate("MainWindow", "\342\200\242 12 employ\303\251s en cong\303\251 ce mois", nullptr));
        lblAlert3->setText(QCoreApplication::translate("MainWindow", "\342\200\242 Taux de pr\303\251sence: 92%", nullptr));
        lblAdvancedMetierTitle->setText(QCoreApplication::translate("MainWindow", " Moniteur de Dotation & Remplacements", nullptr));
        lblRoleStatusIcon->setText(QCoreApplication::translate("MainWindow", "\360\237\232\250", nullptr));
        lblRoleStatusText->setText(QCoreApplication::translate("MainWindow", "S\303\251lectionnez une date pour analyser ", nullptr));
        lblAutoTitle->setText(QCoreApplication::translate("MainWindow", "Suggestions Automatiques (Certifi\303\251s)", nullptr));
        QTableWidgetItem *___qtablewidgetitem29 = tableRoleAutoSuggestions->horizontalHeaderItem(0);
        ___qtablewidgetitem29->setText(QCoreApplication::translate("MainWindow", "ID", nullptr));
        QTableWidgetItem *___qtablewidgetitem30 = tableRoleAutoSuggestions->horizontalHeaderItem(1);
        ___qtablewidgetitem30->setText(QCoreApplication::translate("MainWindow", "Nom", nullptr));
        QTableWidgetItem *___qtablewidgetitem31 = tableRoleAutoSuggestions->horizontalHeaderItem(2);
        ___qtablewidgetitem31->setText(QCoreApplication::translate("MainWindow", "Pr\303\251nom", nullptr));
        QTableWidgetItem *___qtablewidgetitem32 = tableRoleAutoSuggestions->horizontalHeaderItem(3);
        ___qtablewidgetitem32->setText(QCoreApplication::translate("MainWindow", "Poste", nullptr));
        QTableWidgetItem *___qtablewidgetitem33 = tableRoleAutoSuggestions->horizontalHeaderItem(4);
        ___qtablewidgetitem33->setText(QCoreApplication::translate("MainWindow", "Certificats", nullptr));
        QTableWidgetItem *___qtablewidgetitem34 = tableRoleAutoSuggestions->horizontalHeaderItem(5);
        ___qtablewidgetitem34->setText(QCoreApplication::translate("MainWindow", "Action", nullptr));
        QTableWidgetItem *___qtablewidgetitem35 = tableRoleAutoSuggestions->verticalHeaderItem(0);
        ___qtablewidgetitem35->setText(QCoreApplication::translate("MainWindow", "dddddd", nullptr));
        QTableWidgetItem *___qtablewidgetitem36 = tableRoleAutoSuggestions->verticalHeaderItem(1);
        ___qtablewidgetitem36->setText(QCoreApplication::translate("MainWindow", "dddddd", nullptr));
        QTableWidgetItem *___qtablewidgetitem37 = tableRoleAutoSuggestions->verticalHeaderItem(2);
        ___qtablewidgetitem37->setText(QCoreApplication::translate("MainWindow", "dddddddd", nullptr));
        QTableWidgetItem *___qtablewidgetitem38 = tableRoleAutoSuggestions->verticalHeaderItem(3);
        ___qtablewidgetitem38->setText(QCoreApplication::translate("MainWindow", "ddddd", nullptr));
        lblManualTitle->setText(QCoreApplication::translate("MainWindow", "Remplacement Manuel (Tous les Actifs)", nullptr));
        QTableWidgetItem *___qtablewidgetitem39 = tableRoleManualSelection->horizontalHeaderItem(0);
        ___qtablewidgetitem39->setText(QCoreApplication::translate("MainWindow", "ID", nullptr));
        QTableWidgetItem *___qtablewidgetitem40 = tableRoleManualSelection->horizontalHeaderItem(1);
        ___qtablewidgetitem40->setText(QCoreApplication::translate("MainWindow", "Nom", nullptr));
        QTableWidgetItem *___qtablewidgetitem41 = tableRoleManualSelection->horizontalHeaderItem(2);
        ___qtablewidgetitem41->setText(QCoreApplication::translate("MainWindow", "Pr\303\251nom", nullptr));
        QTableWidgetItem *___qtablewidgetitem42 = tableRoleManualSelection->horizontalHeaderItem(3);
        ___qtablewidgetitem42->setText(QCoreApplication::translate("MainWindow", "Poste", nullptr));
        QTableWidgetItem *___qtablewidgetitem43 = tableRoleManualSelection->horizontalHeaderItem(4);
        ___qtablewidgetitem43->setText(QCoreApplication::translate("MainWindow", "Certificats", nullptr));
        QTableWidgetItem *___qtablewidgetitem44 = tableRoleManualSelection->verticalHeaderItem(0);
        ___qtablewidgetitem44->setText(QCoreApplication::translate("MainWindow", "qqqqqqq", nullptr));
        QTableWidgetItem *___qtablewidgetitem45 = tableRoleManualSelection->verticalHeaderItem(1);
        ___qtablewidgetitem45->setText(QCoreApplication::translate("MainWindow", "qqqqq", nullptr));
        QTableWidgetItem *___qtablewidgetitem46 = tableRoleManualSelection->verticalHeaderItem(2);
        ___qtablewidgetitem46->setText(QCoreApplication::translate("MainWindow", "qqqqq", nullptr));
        QTableWidgetItem *___qtablewidgetitem47 = tableRoleManualSelection->verticalHeaderItem(3);
        ___qtablewidgetitem47->setText(QCoreApplication::translate("MainWindow", "qqqqq", nullptr));
        QTableWidgetItem *___qtablewidgetitem48 = tableWidget->horizontalHeaderItem(0);
        ___qtablewidgetitem48->setText(QCoreApplication::translate("MainWindow", "date", nullptr));
        QTableWidgetItem *___qtablewidgetitem49 = tableWidget->verticalHeaderItem(0);
        ___qtablewidgetitem49->setText(QCoreApplication::translate("MainWindow", "09/02/2026", nullptr));
        QTableWidgetItem *___qtablewidgetitem50 = tableWidget->verticalHeaderItem(1);
        ___qtablewidgetitem50->setText(QCoreApplication::translate("MainWindow", "10/02/2026", nullptr));
        QTableWidgetItem *___qtablewidgetitem51 = tableWidget->verticalHeaderItem(2);
        ___qtablewidgetitem51->setText(QCoreApplication::translate("MainWindow", "11/02/2026", nullptr));
        QTableWidgetItem *___qtablewidgetitem52 = tableWidget->verticalHeaderItem(3);
        ___qtablewidgetitem52->setText(QCoreApplication::translate("MainWindow", "12/02/2026", nullptr));
        QTableWidgetItem *___qtablewidgetitem53 = tableWidget->verticalHeaderItem(4);
        ___qtablewidgetitem53->setText(QCoreApplication::translate("MainWindow", "13/02/2026", nullptr));
        QTableWidgetItem *___qtablewidgetitem54 = tableWidget->verticalHeaderItem(5);
        ___qtablewidgetitem54->setText(QCoreApplication::translate("MainWindow", "15/02/2026", nullptr));

        const bool __sortingEnabled1 = tableWidget->isSortingEnabled();
        tableWidget->setSortingEnabled(false);
        QTableWidgetItem *___qtablewidgetitem55 = tableWidget->item(0, 0);
        ___qtablewidgetitem55->setText(QCoreApplication::translate("MainWindow", "1", nullptr));
        QTableWidgetItem *___qtablewidgetitem56 = tableWidget->item(1, 0);
        ___qtablewidgetitem56->setText(QCoreApplication::translate("MainWindow", "0", nullptr));
        QTableWidgetItem *___qtablewidgetitem57 = tableWidget->item(2, 0);
        ___qtablewidgetitem57->setText(QCoreApplication::translate("MainWindow", "0", nullptr));
        QTableWidgetItem *___qtablewidgetitem58 = tableWidget->item(3, 0);
        ___qtablewidgetitem58->setText(QCoreApplication::translate("MainWindow", "2", nullptr));
        QTableWidgetItem *___qtablewidgetitem59 = tableWidget->item(4, 0);
        ___qtablewidgetitem59->setText(QCoreApplication::translate("MainWindow", "0", nullptr));
        QTableWidgetItem *___qtablewidgetitem60 = tableWidget->item(5, 0);
        ___qtablewidgetitem60->setText(QCoreApplication::translate("MainWindow", "0", nullptr));
        tableWidget->setSortingEnabled(__sortingEnabled1);

        title_label_pecheurs->setText(QCoreApplication::translate("MainWindow", "GESTION DES P\303\212CHEURS", nullptr));
        btnSubPech1->setText(QCoreApplication::translate("MainWindow", "Liste", nullptr));
        btnSubPech2->setText(QCoreApplication::translate("MainWindow", "Statistiques", nullptr));
        btnSubPech3->setText(QCoreApplication::translate("MainWindow", "Missions IA", nullptr));
        groupBox_pecheur_2->setTitle(QCoreApplication::translate("MainWindow", "Formulaire Pecheur", nullptr));
        l2_2->setText(QCoreApplication::translate("MainWindow", "Nom:", nullptr));
        l3_2->setText(QCoreApplication::translate("MainWindow", "Prenom:", nullptr));
        l5_2->setText(QCoreApplication::translate("MainWindow", "Role:", nullptr));
        cb_role_2->setItemText(0, QCoreApplication::translate("MainWindow", "Capitaine", nullptr));
        cb_role_2->setItemText(1, QCoreApplication::translate("MainWindow", "Matelot", nullptr));

        l7_2->setText(QCoreApplication::translate("MainWindow", "Statut:", nullptr));
        radio_status_actif_2->setText(QCoreApplication::translate("MainWindow", "Actif", nullptr));
        radio_status_inactif_2->setText(QCoreApplication::translate("MainWindow", "Inactif", nullptr));
        l6_2->setText(QCoreApplication::translate("MainWindow", "Experience:", nullptr));
        l_date_limit_2->setText(QCoreApplication::translate("MainWindow", "Date limite:", nullptr));
        l_contact_2->setText(QCoreApplication::translate("MainWindow", "Contact:", nullptr));
        l_prefix_216_2->setText(QCoreApplication::translate("MainWindow", "+216", nullptr));
        le_telephone_2->setPlaceholderText(QCoreApplication::translate("MainWindow", "Num\303\251ro de t\303\251l\303\251phone", nullptr));
        btn_add_2->setText(QCoreApplication::translate("MainWindow", "Ajouter", nullptr));
        btn_edit_2->setText(QCoreApplication::translate("MainWindow", "Modifier", nullptr));
        btn_delete_2->setText(QCoreApplication::translate("MainWindow", "Supprimer", nullptr));
        btn_export_2->setText(QCoreApplication::translate("MainWindow", "Exporter PDF", nullptr));
        le_search_2->setPlaceholderText(QCoreApplication::translate("MainWindow", "Rechercher par nom, ID...", nullptr));
        btn_recherche_2->setText(QCoreApplication::translate("MainWindow", "Recherche", nullptr));
        lsort_2->setText(QCoreApplication::translate("MainWindow", "Trier par:", nullptr));
        cb_sort_2->setItemText(0, QCoreApplication::translate("MainWindow", "Nom", nullptr));
        cb_sort_2->setItemText(1, QCoreApplication::translate("MainWindow", "Pr\303\251nom", nullptr));
        cb_sort_2->setItemText(2, QCoreApplication::translate("MainWindow", "Experience", nullptr));
        cb_sort_2->setItemText(3, QCoreApplication::translate("MainWindow", "Date limite", nullptr));

        QTableWidgetItem *___qtablewidgetitem61 = tableWidget_4->horizontalHeaderItem(0);
        ___qtablewidgetitem61->setText(QCoreApplication::translate("MainWindow", "ID", nullptr));
        QTableWidgetItem *___qtablewidgetitem62 = tableWidget_4->horizontalHeaderItem(1);
        ___qtablewidgetitem62->setText(QCoreApplication::translate("MainWindow", "Nom", nullptr));
        QTableWidgetItem *___qtablewidgetitem63 = tableWidget_4->horizontalHeaderItem(2);
        ___qtablewidgetitem63->setText(QCoreApplication::translate("MainWindow", "Prenom", nullptr));
        QTableWidgetItem *___qtablewidgetitem64 = tableWidget_4->horizontalHeaderItem(3);
        ___qtablewidgetitem64->setText(QCoreApplication::translate("MainWindow", "Role", nullptr));
        QTableWidgetItem *___qtablewidgetitem65 = tableWidget_4->horizontalHeaderItem(4);
        ___qtablewidgetitem65->setText(QCoreApplication::translate("MainWindow", "Experience", nullptr));
        QTableWidgetItem *___qtablewidgetitem66 = tableWidget_4->horizontalHeaderItem(5);
        ___qtablewidgetitem66->setText(QCoreApplication::translate("MainWindow", "Statut", nullptr));
        QTableWidgetItem *___qtablewidgetitem67 = tableWidget_4->horizontalHeaderItem(6);
        ___qtablewidgetitem67->setText(QCoreApplication::translate("MainWindow", "CIN", nullptr));
        QTableWidgetItem *___qtablewidgetitem68 = tableWidget_4->horizontalHeaderItem(7);
        ___qtablewidgetitem68->setText(QCoreApplication::translate("MainWindow", "Date limite", nullptr));
        QTableWidgetItem *___qtablewidgetitem69 = tableWidget_4->verticalHeaderItem(0);
        ___qtablewidgetitem69->setText(QCoreApplication::translate("MainWindow", "1", nullptr));
        QTableWidgetItem *___qtablewidgetitem70 = tableWidget_4->verticalHeaderItem(1);
        ___qtablewidgetitem70->setText(QCoreApplication::translate("MainWindow", "2", nullptr));
        QTableWidgetItem *___qtablewidgetitem71 = tableWidget_4->verticalHeaderItem(2);
        ___qtablewidgetitem71->setText(QCoreApplication::translate("MainWindow", "3", nullptr));
        label_role_chart_title_2->setText(QCoreApplication::translate("MainWindow", "Statistique par r\303\264le", nullptr));
        label_status_chart_title_2->setText(QCoreApplication::translate("MainWindow", "Statistique par statut (Actif / Inactif)", nullptr));
        gb_mission_setup_2->setTitle(QCoreApplication::translate("MainWindow", "Configuration de la Mission", nullptr));
        lbl_mission_type_2->setText(QCoreApplication::translate("MainWindow", "Type de Mission:", nullptr));
        cb_mission_type_2->setItemText(0, QCoreApplication::translate("MainWindow", "P\303\252che C\303\264ti\303\250re", nullptr));
        cb_mission_type_2->setItemText(1, QCoreApplication::translate("MainWindow", "Haute Mer (Deep Sea)", nullptr));
        cb_mission_type_2->setItemText(2, QCoreApplication::translate("MainWindow", "Maintenance Navale", nullptr));
        cb_mission_type_2->setItemText(3, QCoreApplication::translate("MainWindow", "Sauvetage / Urgence", nullptr));

        lbl_duration_2->setText(QCoreApplication::translate("MainWindow", "Dur\303\251e (Jours):", nullptr));
        lbl_difficulty_2->setText(QCoreApplication::translate("MainWindow", "Difficult\303\251 / Risque:", nullptr));
        cb_difficulty_2->setItemText(0, QCoreApplication::translate("MainWindow", "Routine (Faible)", nullptr));
        cb_difficulty_2->setItemText(1, QCoreApplication::translate("MainWindow", "Moyenne", nullptr));
        cb_difficulty_2->setItemText(2, QCoreApplication::translate("MainWindow", "Extr\303\252me (Temp\303\252te/Nuit)", nullptr));

        btn_analyze_mission_2->setText(QCoreApplication::translate("MainWindow", "\360\237\244\226 Analyser & Former l'\303\211quipe", nullptr));
        gb_ai_results_2->setTitle(QCoreApplication::translate("MainWindow", "Recommandation IA", nullptr));
        lbl_ai_status_2->setText(QString());
        QTableWidgetItem *___qtablewidgetitem72 = table_mission_results_2->horizontalHeaderItem(0);
        ___qtablewidgetitem72->setText(QCoreApplication::translate("MainWindow", "R\303\264le", nullptr));
        QTableWidgetItem *___qtablewidgetitem73 = table_mission_results_2->horizontalHeaderItem(1);
        ___qtablewidgetitem73->setText(QCoreApplication::translate("MainWindow", "Candidat Sugger\303\251", nullptr));
        QTableWidgetItem *___qtablewidgetitem74 = table_mission_results_2->horizontalHeaderItem(2);
        ___qtablewidgetitem74->setText(QCoreApplication::translate("MainWindow", "Score (Match %)", nullptr));
        QTableWidgetItem *___qtablewidgetitem75 = table_mission_results_2->horizontalHeaderItem(3);
        ___qtablewidgetitem75->setText(QCoreApplication::translate("MainWindow", "Raisonnement IA", nullptr));
        title_label_bateaux->setText(QCoreApplication::translate("MainWindow", "GESTION DES BATEAUX", nullptr));
        groupForm_15->setTitle(QString());
        label_id_16->setText(QCoreApplication::translate("MainWindow", "ID Bateau:", nullptr));
        label_nom_15->setText(QCoreApplication::translate("MainWindow", "Nom:", nullptr));
        label_exp_51->setText(QCoreApplication::translate("MainWindow", "Immat:", nullptr));
        label_exp_52->setText(QCoreApplication::translate("MainWindow", "Moteur:", nullptr));
        label_exp_53->setText(QCoreApplication::translate("MainWindow", "Propri\303\251taire:", nullptr));
        label_status_field_2->setText(QCoreApplication::translate("MainWindow", "Statut:", nullptr));
        comboStatus_2->setItemText(0, QCoreApplication::translate("MainWindow", "\342\232\223 AU PORT", nullptr));
        comboStatus_2->setItemText(1, QCoreApplication::translate("MainWindow", "\360\237\214\212 EN MER", nullptr));

        label_license_8->setText(QCoreApplication::translate("MainWindow", "Num. Licence:", nullptr));
        label_expiry_date_2->setText(QCoreApplication::translate("MainWindow", "Visite Technique:", nullptr));
        label_exp_54->setText(QCoreApplication::translate("MainWindow", "Type du bateau:", nullptr));
        comboRole_15->setItemText(0, QCoreApplication::translate("MainWindow", "Commerce", nullptr));
        comboRole_15->setItemText(1, QCoreApplication::translate("MainWindow", "P\303\252che mois de 12 m ", nullptr));
        comboRole_15->setItemText(2, QCoreApplication::translate("MainWindow", "P\303\252che plus de 12 m", nullptr));
        comboRole_15->setItemText(3, QCoreApplication::translate("MainWindow", "Voilier", nullptr));

        btnAjouter_8->setText(QCoreApplication::translate("MainWindow", "Ajouter", nullptr));
        btnModifier_15->setText(QCoreApplication::translate("MainWindow", "Modifier", nullptr));
        btnSupprimer_15->setText(QCoreApplication::translate("MainWindow", "Supprimer", nullptr));
        lineRecherche_15->setPlaceholderText(QCoreApplication::translate("MainWindow", "Search by Name, ID...", nullptr));
        comboTri_15->setItemText(0, QCoreApplication::translate("MainWindow", "ID bateau", nullptr));
        comboTri_15->setItemText(1, QCoreApplication::translate("MainWindow", "Type", nullptr));
        comboTri_15->setItemText(2, QCoreApplication::translate("MainWindow", "Propri\303\251taire", nullptr));
        comboTri_15->setItemText(3, QCoreApplication::translate("MainWindow", "Moteur", nullptr));

        btnExportPDF_15->setText(QCoreApplication::translate("MainWindow", "Export PDF", nullptr));
        tabWidget_2->setTabText(tabWidget_2->indexOf(tabGestion_8), QCoreApplication::translate("MainWindow", "Information Bateaux", nullptr));
        groupForm_16->setTitle(QString());
        label_id_17->setText(QCoreApplication::translate("MainWindow", "ID du bateau:", nullptr));
        label_nom_16->setText(QCoreApplication::translate("MainWindow", "Dates d'entr\303\251es:", nullptr));
        label_exp_55->setText(QCoreApplication::translate("MainWindow", "Dates des sorties:", nullptr));
        label_exp_56->setText(QCoreApplication::translate("MainWindow", "S\303\251jours d'occupation:", nullptr));
        label_exp_57->setText(QCoreApplication::translate("MainWindow", "Type du bateau:", nullptr));
        comboRole_16->setItemText(0, QCoreApplication::translate("MainWindow", "Commerce", nullptr));
        comboRole_16->setItemText(1, QCoreApplication::translate("MainWindow", "P\303\252che mois de 12 m ", nullptr));
        comboRole_16->setItemText(2, QCoreApplication::translate("MainWindow", "P\303\252che plus de 12 m", nullptr));
        comboRole_16->setItemText(3, QCoreApplication::translate("MainWindow", "Voilier", nullptr));

        btnAjouter_21->setText(QCoreApplication::translate("MainWindow", "Add", nullptr));
        btnModifier_16->setText(QCoreApplication::translate("MainWindow", "Update", nullptr));
        btnSupprimer_16->setText(QCoreApplication::translate("MainWindow", "Delete", nullptr));
        lineRecherche_16->setPlaceholderText(QCoreApplication::translate("MainWindow", "Search by Name, ID...", nullptr));
        comboTri_16->setItemText(0, QCoreApplication::translate("MainWindow", "ID bateau", nullptr));
        comboTri_16->setItemText(1, QCoreApplication::translate("MainWindow", "Date d'entr\303\251es", nullptr));
        comboTri_16->setItemText(2, QCoreApplication::translate("MainWindow", "Date des sorties", nullptr));
        comboTri_16->setItemText(3, QString());

        btnExportPDF_16->setText(QCoreApplication::translate("MainWindow", "Export PDF", nullptr));
        tabWidget_2->setTabText(tabWidget_2->indexOf(tabStats_8), QCoreApplication::translate("MainWindow", "Historique", nullptr));
        groupTotal_8->setTitle(QCoreApplication::translate("MainWindow", "\360\237\232\242 Total Bateaux", nullptr));
        lblTotalBateauxCount_8->setText(QCoreApplication::translate("MainWindow", "0", nullptr));
        lblTotalBateauxText_8->setText(QCoreApplication::translate("MainWindow", "Bateaux enregistr\303\251s", nullptr));
        groupActif_8->setTitle(QCoreApplication::translate("MainWindow", "\342\234\205 Licences Actives", nullptr));
        lblLicencesActivesCount_8->setText(QCoreApplication::translate("MainWindow", "0", nullptr));
        lblLicencesActivesText_8->setText(QCoreApplication::translate("MainWindow", "Actuellement valides", nullptr));
        groupMer_8->setTitle(QCoreApplication::translate("MainWindow", "\360\237\214\212 Bateaux en Mer", nullptr));
        lblBateauxMerCount_8->setText(QCoreApplication::translate("MainWindow", "0", nullptr));
        lblBateauxMerText_8->setText(QCoreApplication::translate("MainWindow", "Actuellement sortis", nullptr));
        groupExpire_8->setTitle(QCoreApplication::translate("MainWindow", "\342\232\240\357\270\217 Licences Expir\303\251es", nullptr));
        lblLicencesExpireesCount_8->setText(QCoreApplication::translate("MainWindow", "0", nullptr));
        lblLicencesExpireesText_8->setText(QCoreApplication::translate("MainWindow", "N\303\251cessitent renouvellement", nullptr));
        groupNotifications_8->setTitle(QCoreApplication::translate("MainWindow", "\360\237\224\224 Notifications R\303\251centes", nullptr));
        textNotifications_8->setPlaceholderText(QCoreApplication::translate("MainWindow", "Les notifications appara\303\256tront ici...", nullptr));
        btnAIAssistant_8->setText(QCoreApplication::translate("MainWindow", "\360\237\244\226 Assistant IA SmartPort", nullptr));
        tabWidget_2->setTabText(tabWidget_2->indexOf(tabStatistiques_8), QCoreApplication::translate("MainWindow", "Statistiques", nullptr));
        title_label_equipement->setText(QCoreApplication::translate("MainWindow", "GESTION DES EQUIPEMENTS", nullptr));
        groupBox->setTitle(QCoreApplication::translate("MainWindow", "Formulaire Equipement", nullptr));
        label->setText(QCoreApplication::translate("MainWindow", "Id_equipement:", nullptr));
        label_2->setText(QCoreApplication::translate("MainWindow", "Nom:", nullptr));
        label_3->setText(QCoreApplication::translate("MainWindow", "Type:", nullptr));
        label_4->setText(QCoreApplication::translate("MainWindow", "Etat:", nullptr));
        comboBox->setItemText(0, QCoreApplication::translate("MainWindow", "Disponible", nullptr));
        comboBox->setItemText(1, QCoreApplication::translate("MainWindow", "En panne", nullptr));
        comboBox->setItemText(2, QCoreApplication::translate("MainWindow", "Maintenance", nullptr));

        label_5->setText(QCoreApplication::translate("MainWindow", "Date_acquisition:", nullptr));
        label_6->setText(QCoreApplication::translate("MainWindow", "Localisation:", nullptr));
        comboBox_2->setItemText(0, QCoreApplication::translate("MainWindow", "Quai", nullptr));
        comboBox_2->setItemText(1, QCoreApplication::translate("MainWindow", "Bateau", nullptr));
        comboBox_2->setItemText(2, QCoreApplication::translate("MainWindow", "entrep\303\264t", nullptr));

        label_7->setText(QCoreApplication::translate("MainWindow", "Co\303\273t estim\303\251:", nullptr));
        label_8->setText(QCoreApplication::translate("MainWindow", "Fournisseur:", nullptr));
        label_9->setText(QCoreApplication::translate("MainWindow", "Disponibilit\303\251:", nullptr));
        radioButton->setText(QCoreApplication::translate("MainWindow", "Oui", nullptr));
        radioButton_2->setText(QCoreApplication::translate("MainWindow", "Non", nullptr));
        pushButton_2->setText(QCoreApplication::translate("MainWindow", "Ajouter", nullptr));
        pushButton_4->setText(QCoreApplication::translate("MainWindow", "Modifier", nullptr));
        pushButton_5->setText(QCoreApplication::translate("MainWindow", "Supprimer", nullptr));
        pushButton_7->setText(QCoreApplication::translate("MainWindow", "Exporter en PDF", nullptr));
        lineEdit_8->setPlaceholderText(QCoreApplication::translate("MainWindow", "Rechercher un \303\251quipement", nullptr));
        label_10->setText(QCoreApplication::translate("MainWindow", "Tri par:", nullptr));
        comboBox_3->setItemText(0, QCoreApplication::translate("MainWindow", "Cout", nullptr));
        comboBox_3->setItemText(1, QCoreApplication::translate("MainWindow", "Nom", nullptr));
        comboBox_3->setItemText(2, QCoreApplication::translate("MainWindow", "Disp", nullptr));
        comboBox_3->setItemText(3, QCoreApplication::translate("MainWindow", "Id", nullptr));

        QTableWidgetItem *___qtablewidgetitem76 = tableWidget_3->horizontalHeaderItem(0);
        ___qtablewidgetitem76->setText(QCoreApplication::translate("MainWindow", "Id", nullptr));
        QTableWidgetItem *___qtablewidgetitem77 = tableWidget_3->horizontalHeaderItem(1);
        ___qtablewidgetitem77->setText(QCoreApplication::translate("MainWindow", "Nom", nullptr));
        QTableWidgetItem *___qtablewidgetitem78 = tableWidget_3->horizontalHeaderItem(2);
        ___qtablewidgetitem78->setText(QCoreApplication::translate("MainWindow", "Type", nullptr));
        QTableWidgetItem *___qtablewidgetitem79 = tableWidget_3->horizontalHeaderItem(3);
        ___qtablewidgetitem79->setText(QCoreApplication::translate("MainWindow", "Etat", nullptr));
        tabWidget->setTabText(tabWidget->indexOf(tab_gestion), QCoreApplication::translate("MainWindow", "Gestion Equipements", nullptr));
        label_stat->setText(QCoreApplication::translate("MainWindow", "Statistique", nullptr));
        tabWidget->setTabText(tabWidget->indexOf(tab_statistique), QCoreApplication::translate("MainWindow", "Statistique", nullptr));
        label_smart_location->setText(QCoreApplication::translate("MainWindow", "Smart Location", nullptr));
        tabWidget->setTabText(tabWidget->indexOf(tab_smart_location), QCoreApplication::translate("MainWindow", "Smart Location", nullptr));
        label_maintenance->setText(QCoreApplication::translate("MainWindow", "Maintenance intelligente", nullptr));
        tabWidget->setTabText(tabWidget->indexOf(tab_maintenance), QCoreApplication::translate("MainWindow", "Maintenance intelligente", nullptr));
        title_label_quais->setText(QCoreApplication::translate("MainWindow", "Syst\303\250me de Gestion des Quais", nullptr));
        label_Quais_SectionTitle->setText(QCoreApplication::translate("MainWindow", "Liste des Quais \342\200\224 Gestion et CRUD", nullptr));
        groupBoxQuaiInfo->setTitle(QCoreApplication::translate("MainWindow", "\342\227\206 Informations du Quai", nullptr));
        radioStatutActif->setText(QCoreApplication::translate("MainWindow", "Actif", nullptr));
        radioStatutInactif->setText(QCoreApplication::translate("MainWindow", "Inactif", nullptr));
        radioStatutMaintenance->setText(QCoreApplication::translate("MainWindow", "En Maintenance", nullptr));
        labelCapacite->setText(QCoreApplication::translate("MainWindow", "Capacit\303\251:", nullptr));
        labelType->setText(QCoreApplication::translate("MainWindow", "Type:", nullptr));
        radioEclairageAllume->setText(QCoreApplication::translate("MainWindow", "Allum\303\251", nullptr));
        radioEclairageEteint->setText(QCoreApplication::translate("MainWindow", "\303\211teint", nullptr));
        radioEclairageDefaillant->setText(QCoreApplication::translate("MainWindow", "D\303\251faillant", nullptr));
        labelLongueur->setText(QCoreApplication::translate("MainWindow", "Longueur (m):", nullptr));
        labelCurrentUsage->setText(QCoreApplication::translate("MainWindow", "Usage actuel:", nullptr));
        radioPrioriteNormale->setText(QCoreApplication::translate("MainWindow", "Normale", nullptr));
        radioPrioriteHaute->setText(QCoreApplication::translate("MainWindow", "Haute", nullptr));
        radioPrioriteBasse->setText(QCoreApplication::translate("MainWindow", "Basse", nullptr));
        labelNom->setText(QCoreApplication::translate("MainWindow", "Nom du Quai:", nullptr));
        labelSecurite->setText(QCoreApplication::translate("MainWindow", "Niveau S\303\251curit\303\251:", nullptr));
        labelOccupation->setText(QCoreApplication::translate("MainWindow", "Taux d'occupation %:", nullptr));
        labelQuaiID->setText(QCoreApplication::translate("MainWindow", "Quai ID:", nullptr));
        radioTypePeche->setText(QCoreApplication::translate("MainWindow", "P\303\252che", nullptr));
        radioTypeCommerce->setText(QCoreApplication::translate("MainWindow", "Commerce", nullptr));
        radioTypeMaintenance->setText(QCoreApplication::translate("MainWindow", "Maintenance", nullptr));
        radioTypeMixte->setText(QCoreApplication::translate("MainWindow", "Mixte", nullptr));
        labelProfondeur->setText(QCoreApplication::translate("MainWindow", "Profondeur (m):", nullptr));
        labelPriorite->setText(QCoreApplication::translate("MainWindow", "Priorit\303\251:", nullptr));
        labelLightingStatus->setText(QCoreApplication::translate("MainWindow", "\303\211tat \303\251clairage:", nullptr));
        radioSecurite1->setText(QCoreApplication::translate("MainWindow", "Niveau 1", nullptr));
        radioSecurite2->setText(QCoreApplication::translate("MainWindow", "Niveau 2", nullptr));
        radioSecurite3->setText(QCoreApplication::translate("MainWindow", "Niveau 3", nullptr));
        labelStatut->setText(QCoreApplication::translate("MainWindow", "Statut:", nullptr));
        lineEditQuaiID->setPlaceholderText(QCoreApplication::translate("MainWindow", "Auto-g\303\251n\303\251r\303\251", nullptr));
        btnAdd->setText(QCoreApplication::translate("MainWindow", " Ajouter", nullptr));
        btnUpdate->setText(QCoreApplication::translate("MainWindow", " Modifier", nullptr));
        btnDelete->setText(QCoreApplication::translate("MainWindow", " Supprimer", nullptr));
        btnClear->setText(QCoreApplication::translate("MainWindow", " Effacer", nullptr));
        labelFilterBy->setText(QCoreApplication::translate("MainWindow", "Filtrer:", nullptr));
        filterComboType->setItemText(0, QCoreApplication::translate("MainWindow", "Tous (type)", nullptr));
        filterComboType->setItemText(1, QCoreApplication::translate("MainWindow", "P\303\252che", nullptr));
        filterComboType->setItemText(2, QCoreApplication::translate("MainWindow", "Commerce", nullptr));
        filterComboType->setItemText(3, QCoreApplication::translate("MainWindow", "Maintenance", nullptr));
        filterComboType->setItemText(4, QCoreApplication::translate("MainWindow", "Mixte", nullptr));

        filterComboType->setPlaceholderText(QCoreApplication::translate("MainWindow", "Type", nullptr));
        filterComboStatut->setItemText(0, QCoreApplication::translate("MainWindow", "Tous (statut)", nullptr));
        filterComboStatut->setItemText(1, QCoreApplication::translate("MainWindow", "Actif", nullptr));
        filterComboStatut->setItemText(2, QCoreApplication::translate("MainWindow", "Inactif", nullptr));

        filterComboSecurite->setItemText(0, QCoreApplication::translate("MainWindow", "Tous (s\303\251c.)", nullptr));
        filterComboSecurite->setItemText(1, QCoreApplication::translate("MainWindow", "Niveau 1", nullptr));
        filterComboSecurite->setItemText(2, QCoreApplication::translate("MainWindow", "Niveau 2", nullptr));
        filterComboSecurite->setItemText(3, QCoreApplication::translate("MainWindow", "Niveau 3", nullptr));

        lineEditSearch->setPlaceholderText(QCoreApplication::translate("MainWindow", "Rechercher (nom, type, statut, s\303\251curit\303\251)", nullptr));
        btnSearch->setText(QCoreApplication::translate("MainWindow", " Rechercher", nullptr));
        labelSmartSort->setText(QCoreApplication::translate("MainWindow", "Trier par", nullptr));
        comboSmartSort->setItemText(0, QCoreApplication::translate("MainWindow", "Capacit\303\251", nullptr));
        comboSmartSort->setItemText(1, QCoreApplication::translate("MainWindow", "Taux d'occupation", nullptr));
        comboSmartSort->setItemText(2, QCoreApplication::translate("MainWindow", "Disponibilit\303\251", nullptr));
        comboSmartSort->setItemText(3, QCoreApplication::translate("MainWindow", "Priorit\303\251", nullptr));

        btnExport->setText(QCoreApplication::translate("MainWindow", "\360\237\223\212 Exporter", nullptr));
        QTableWidgetItem *___qtablewidgetitem80 = tableQuais->horizontalHeaderItem(0);
        ___qtablewidgetitem80->setText(QCoreApplication::translate("MainWindow", "ID", nullptr));
        QTableWidgetItem *___qtablewidgetitem81 = tableQuais->horizontalHeaderItem(1);
        ___qtablewidgetitem81->setText(QCoreApplication::translate("MainWindow", "Nom", nullptr));
        QTableWidgetItem *___qtablewidgetitem82 = tableQuais->horizontalHeaderItem(2);
        ___qtablewidgetitem82->setText(QCoreApplication::translate("MainWindow", "Type", nullptr));
        QTableWidgetItem *___qtablewidgetitem83 = tableQuais->horizontalHeaderItem(3);
        ___qtablewidgetitem83->setText(QCoreApplication::translate("MainWindow", "Long. (m)", nullptr));
        QTableWidgetItem *___qtablewidgetitem84 = tableQuais->horizontalHeaderItem(4);
        ___qtablewidgetitem84->setText(QCoreApplication::translate("MainWindow", "Prof. (m)", nullptr));
        QTableWidgetItem *___qtablewidgetitem85 = tableQuais->horizontalHeaderItem(5);
        ___qtablewidgetitem85->setText(QCoreApplication::translate("MainWindow", "Capacit\303\251", nullptr));
        QTableWidgetItem *___qtablewidgetitem86 = tableQuais->horizontalHeaderItem(6);
        ___qtablewidgetitem86->setText(QCoreApplication::translate("MainWindow", "Statut", nullptr));
        QTableWidgetItem *___qtablewidgetitem87 = tableQuais->horizontalHeaderItem(7);
        ___qtablewidgetitem87->setText(QCoreApplication::translate("MainWindow", "Usage", nullptr));
        QTableWidgetItem *___qtablewidgetitem88 = tableQuais->horizontalHeaderItem(8);
        ___qtablewidgetitem88->setText(QCoreApplication::translate("MainWindow", "Taux occup.%", nullptr));
        QTableWidgetItem *___qtablewidgetitem89 = tableQuais->horizontalHeaderItem(9);
        ___qtablewidgetitem89->setText(QCoreApplication::translate("MainWindow", "Priorit\303\251", nullptr));
        QTableWidgetItem *___qtablewidgetitem90 = tableQuais->horizontalHeaderItem(10);
        ___qtablewidgetitem90->setText(QCoreApplication::translate("MainWindow", "Niveau s\303\251c.", nullptr));
        QTableWidgetItem *___qtablewidgetitem91 = tableQuais->horizontalHeaderItem(11);
        ___qtablewidgetitem91->setText(QCoreApplication::translate("MainWindow", "\303\211clairage", nullptr));
        mainTabWidget->setTabText(mainTabWidget->indexOf(tabCRUD), QCoreApplication::translate("MainWindow", " LISTE", nullptr));
        label_Supervision_SectionTitle->setText(QCoreApplication::translate("MainWindow", "Statistiques et Supervision des Quais", nullptr));
        lblKpiTotalTitle->setText(QCoreApplication::translate("MainWindow", "Total Quais", nullptr));
        lblKpiTotalValue->setText(QCoreApplication::translate("MainWindow", "0", nullptr));
        lblKpiLibresTitle->setText(QCoreApplication::translate("MainWindow", "Quais Libres", nullptr));
        lblKpiLibresValue->setText(QCoreApplication::translate("MainWindow", "0 (0%)", nullptr));
        lblKpiOccupesTitle->setText(QCoreApplication::translate("MainWindow", "Quais Occup\303\251s", nullptr));
        lblKpiOccupesValue->setText(QCoreApplication::translate("MainWindow", "0 (0%)", nullptr));
        lblKpiMaintenanceTitle->setText(QCoreApplication::translate("MainWindow", "En Maintenance", nullptr));
        lblKpiMaintenanceValue->setText(QCoreApplication::translate("MainWindow", "0 (0%)", nullptr));
        groupBoxOccupation->setTitle(QCoreApplication::translate("MainWindow", "\342\227\206 Statistiques des quais (r\303\251f\303\251rence par graphique)", nullptr));
        QTableWidgetItem *___qtablewidgetitem92 = tableOccupationByType->horizontalHeaderItem(0);
        ___qtablewidgetitem92->setText(QCoreApplication::translate("MainWindow", "Type de Quai", nullptr));
        QTableWidgetItem *___qtablewidgetitem93 = tableOccupationByType->horizontalHeaderItem(1);
        ___qtablewidgetitem93->setText(QCoreApplication::translate("MainWindow", "Nombre", nullptr));
        QTableWidgetItem *___qtablewidgetitem94 = tableOccupationByType->horizontalHeaderItem(2);
        ___qtablewidgetitem94->setText(QCoreApplication::translate("MainWindow", "Occupation %", nullptr));
        QTableWidgetItem *___qtablewidgetitem95 = tableOccupationByType->horizontalHeaderItem(3);
        ___qtablewidgetitem95->setText(QCoreApplication::translate("MainWindow", "Statut", nullptr));
        chartTitle->setText(QCoreApplication::translate("MainWindow", "R\303\251partition des types de quais", nullptr));
        legend1->setText(QCoreApplication::translate("MainWindow", "\342\226\240 P\303\252che (40%)", nullptr));
        legend2->setText(QCoreApplication::translate("MainWindow", "\342\226\240 Commerce (25%)", nullptr));
        legend3->setText(QCoreApplication::translate("MainWindow", "\342\226\240 Maintenance (20%)", nullptr));
        legend4->setText(QCoreApplication::translate("MainWindow", "\342\226\240 Mixte (15%)", nullptr));
        btnAddIncident->setText(QCoreApplication::translate("MainWindow", "\342\236\225 Ajouter Incident", nullptr));
        QTableWidgetItem *___qtablewidgetitem96 = tableSupervisionCombined->horizontalHeaderItem(0);
        ___qtablewidgetitem96->setText(QCoreApplication::translate("MainWindow", "Source", nullptr));
        QTableWidgetItem *___qtablewidgetitem97 = tableSupervisionCombined->horizontalHeaderItem(1);
        ___qtablewidgetitem97->setText(QCoreApplication::translate("MainWindow", "Quai", nullptr));
        QTableWidgetItem *___qtablewidgetitem98 = tableSupervisionCombined->horizontalHeaderItem(2);
        ___qtablewidgetitem98->setText(QCoreApplication::translate("MainWindow", "Zone", nullptr));
        QTableWidgetItem *___qtablewidgetitem99 = tableSupervisionCombined->horizontalHeaderItem(3);
        ___qtablewidgetitem99->setText(QCoreApplication::translate("MainWindow", "Type", nullptr));
        QTableWidgetItem *___qtablewidgetitem100 = tableSupervisionCombined->horizontalHeaderItem(4);
        ___qtablewidgetitem100->setText(QCoreApplication::translate("MainWindow", "Statut", nullptr));
        QTableWidgetItem *___qtablewidgetitem101 = tableSupervisionCombined->horizontalHeaderItem(5);
        ___qtablewidgetitem101->setText(QCoreApplication::translate("MainWindow", "D\303\251tail", nullptr));
        QTableWidgetItem *___qtablewidgetitem102 = tableSupervisionCombined->horizontalHeaderItem(6);
        ___qtablewidgetitem102->setText(QCoreApplication::translate("MainWindow", "Date", nullptr));
        lblChartAlertesTitle->setText(QCoreApplication::translate("MainWindow", "Alertes importantes", nullptr));
        lblAlertesContent->setText(QCoreApplication::translate("MainWindow", "\342\200\242 Taux d'occupation (aujourd'hui): \342\200\224%\n"
"\342\200\242 Quais en maintenance: 0\n"
"\342\200\242 Derni\303\250re MAJ: \342\200\224", nullptr));
        lblResumeQuais->setText(QCoreApplication::translate("MainWindow", "Quais actifs: \342\200\224 | Capacit\303\251 totale: \342\200\224 | Zones: \342\200\224", nullptr));
        lblResumeMAJ->setText(QCoreApplication::translate("MainWindow", "Derni\303\250re synchro: \342\200\224", nullptr));
        mainTabWidget->setTabText(mainTabWidget->indexOf(tabSupervision), QCoreApplication::translate("MainWindow", " Supervision", nullptr));
        groupBoxBoatInfo->setTitle(QCoreApplication::translate("MainWindow", "\342\227\206 Informations Bateau", nullptr));
        labelTypeBateau->setText(QCoreApplication::translate("MainWindow", "Type:", nullptr));
        comboBoxTypeBateau->setItemText(0, QCoreApplication::translate("MainWindow", "Chalutier", nullptr));
        comboBoxTypeBateau->setItemText(1, QCoreApplication::translate("MainWindow", "Cargo", nullptr));
        comboBoxTypeBateau->setItemText(2, QCoreApplication::translate("MainWindow", "Tanker", nullptr));

        labelLongueurBateau->setText(QCoreApplication::translate("MainWindow", "Longueur (m):", nullptr));
        labelTonnage->setText(QCoreApplication::translate("MainWindow", "Tonnage:", nullptr));
        btnAnalyze->setText(QCoreApplication::translate("MainWindow", "\360\237\224\215 Analyser", nullptr));
        QTableWidgetItem *___qtablewidgetitem103 = tableRecommendations->horizontalHeaderItem(0);
        ___qtablewidgetitem103->setText(QCoreApplication::translate("MainWindow", "Rang", nullptr));
        QTableWidgetItem *___qtablewidgetitem104 = tableRecommendations->horizontalHeaderItem(1);
        ___qtablewidgetitem104->setText(QCoreApplication::translate("MainWindow", "Quai", nullptr));
        QTableWidgetItem *___qtablewidgetitem105 = tableRecommendations->horizontalHeaderItem(2);
        ___qtablewidgetitem105->setText(QCoreApplication::translate("MainWindow", "Score", nullptr));
        QTableWidgetItem *___qtablewidgetitem106 = tableRecommendations->horizontalHeaderItem(3);
        ___qtablewidgetitem106->setText(QCoreApplication::translate("MainWindow", "Disponibilit\303\251", nullptr));
        QTableWidgetItem *___qtablewidgetitem107 = tableRecommendations->horizontalHeaderItem(4);
        ___qtablewidgetitem107->setText(QCoreApplication::translate("MainWindow", "Raison", nullptr));
        mainTabWidget->setTabText(mainTabWidget->indexOf(tabAideDecision), QCoreApplication::translate("MainWindow", " Aide D\303\251cision", nullptr));
        groupBoxTTSSettings->setTitle(QCoreApplication::translate("MainWindow", "\342\227\206 Param\303\250tres Synth\303\250se Vocale", nullptr));
        checkBoxEnableTTS->setText(QCoreApplication::translate("MainWindow", "Activer TTS", nullptr));
        labelLanguage->setText(QCoreApplication::translate("MainWindow", "Langue:", nullptr));
        comboBoxLanguage->setItemText(0, QCoreApplication::translate("MainWindow", "Fran\303\247ais", nullptr));
        comboBoxLanguage->setItemText(1, QCoreApplication::translate("MainWindow", "Arabe", nullptr));
        comboBoxLanguage->setItemText(2, QCoreApplication::translate("MainWindow", "Anglais", nullptr));

        checkBoxReadSelected->setText(QCoreApplication::translate("MainWindow", "Lire \303\251tat quai s\303\251lectionn\303\251", nullptr));
        checkBoxCriticalAlerts->setText(QCoreApplication::translate("MainWindow", "Annonce alertes critiques", nullptr));
        checkBoxDailySummary->setText(QCoreApplication::translate("MainWindow", "R\303\251sum\303\251 vocal quotidien", nullptr));
        labelSummaryTime->setText(QCoreApplication::translate("MainWindow", "Heure r\303\251sum\303\251:", nullptr));
        btnTestTTS->setText(QCoreApplication::translate("MainWindow", "\360\237\216\244 Test TTS", nullptr));
        QTableWidgetItem *___qtablewidgetitem108 = tableTTSLog->horizontalHeaderItem(0);
        ___qtablewidgetitem108->setText(QCoreApplication::translate("MainWindow", "Heure", nullptr));
        QTableWidgetItem *___qtablewidgetitem109 = tableTTSLog->horizontalHeaderItem(1);
        ___qtablewidgetitem109->setText(QCoreApplication::translate("MainWindow", "Type", nullptr));
        QTableWidgetItem *___qtablewidgetitem110 = tableTTSLog->horizontalHeaderItem(2);
        ___qtablewidgetitem110->setText(QCoreApplication::translate("MainWindow", "Message", nullptr));
        paramsTabs->setTabText(paramsTabs->indexOf(tabSyntheseVocale), QCoreApplication::translate("MainWindow", " Synth\303\250se Vocale", nullptr));
        mainTabWidget->setTabText(mainTabWidget->indexOf(tabParametres), QCoreApplication::translate("MainWindow", " Param\303\250tres", nullptr));
        title_label_stock->setText(QCoreApplication::translate("MainWindow", "GESTION DU STOCK", nullptr));
        groupBoxStockForm->setTitle(QCoreApplication::translate("MainWindow", "Formulaire Stock", nullptr));
        stockIdLabel->setText(QCoreApplication::translate("MainWindow", "Stock ID:", nullptr));
        stockEspeceLabel->setText(QCoreApplication::translate("MainWindow", "Esp\303\250ce:", nullptr));
        stockQuantiteLabel->setText(QCoreApplication::translate("MainWindow", "Quantit\303\251:", nullptr));
        stockQuantiteEdit->setPlaceholderText(QCoreApplication::translate("MainWindow", "En KG", nullptr));
        stockEtatLabel->setText(QCoreApplication::translate("MainWindow", "\303\211tat:", nullptr));
        stockEtatCombo->setItemText(0, QCoreApplication::translate("MainWindow", "Vendu", nullptr));
        stockEtatCombo->setItemText(1, QCoreApplication::translate("MainWindow", "Frais", nullptr));
        stockEtatCombo->setItemText(2, QCoreApplication::translate("MainWindow", "Stock\303\251", nullptr));

        stockSeuilMinLabel->setText(QCoreApplication::translate("MainWindow", "Seuil minimum:", nullptr));
        stockSeuilMaxLabel->setText(QCoreApplication::translate("MainWindow", "Seuil maximum:", nullptr));
        stockDateAjoutLabel->setText(QCoreApplication::translate("MainWindow", "Date d'ajout:", nullptr));
        stockDateAjoutEdit->setPlaceholderText(QCoreApplication::translate("MainWindow", "YYYY-MM-DD", nullptr));
        stockCINLabel->setText(QCoreApplication::translate("MainWindow", "CIN vente:", nullptr));
        stockCMDLabel->setText(QCoreApplication::translate("MainWindow", "N\302\260 commande:", nullptr));
        stockDateVenteLabel->setText(QCoreApplication::translate("MainWindow", "Date vente:", nullptr));
        stockDateVente->setPlaceholderText(QCoreApplication::translate("MainWindow", "YYYY-MM-DD", nullptr));
        btnStockAjouter->setText(QCoreApplication::translate("MainWindow", "Ajouter", nullptr));
        btnStockModifier->setText(QCoreApplication::translate("MainWindow", "Modifier", nullptr));
        btnStockSupprimer->setText(QCoreApplication::translate("MainWindow", "Supprimer", nullptr));
        btnStockSave->setText(QCoreApplication::translate("MainWindow", "Enregistrer", nullptr));
        btnStockAnnuler->setText(QCoreApplication::translate("MainWindow", "Annuler", nullptr));
        stockSearchEdit->setPlaceholderText(QCoreApplication::translate("MainWindow", "Recherchez...", nullptr));
        stockSortLabel->setText(QCoreApplication::translate("MainWindow", "Trier par:", nullptr));
        stockSortCombo->setItemText(0, QCoreApplication::translate("MainWindow", "Esp\303\250ce", nullptr));
        stockSortCombo->setItemText(1, QCoreApplication::translate("MainWindow", "\303\211tat", nullptr));
        stockSortCombo->setItemText(2, QCoreApplication::translate("MainWindow", "Quantit\303\251", nullptr));

        QTableWidgetItem *___qtablewidgetitem111 = tableStock->horizontalHeaderItem(0);
        ___qtablewidgetitem111->setText(QCoreApplication::translate("MainWindow", "Stock ID", nullptr));
        QTableWidgetItem *___qtablewidgetitem112 = tableStock->horizontalHeaderItem(1);
        ___qtablewidgetitem112->setText(QCoreApplication::translate("MainWindow", "Esp\303\250ce", nullptr));
        QTableWidgetItem *___qtablewidgetitem113 = tableStock->horizontalHeaderItem(2);
        ___qtablewidgetitem113->setText(QCoreApplication::translate("MainWindow", "Quantit\303\251", nullptr));
        QTableWidgetItem *___qtablewidgetitem114 = tableStock->horizontalHeaderItem(3);
        ___qtablewidgetitem114->setText(QCoreApplication::translate("MainWindow", "\303\211tat", nullptr));
        QTableWidgetItem *___qtablewidgetitem115 = tableStock->horizontalHeaderItem(4);
        ___qtablewidgetitem115->setText(QCoreApplication::translate("MainWindow", "Seuil min", nullptr));
        QTableWidgetItem *___qtablewidgetitem116 = tableStock->horizontalHeaderItem(5);
        ___qtablewidgetitem116->setText(QCoreApplication::translate("MainWindow", "Seuil max", nullptr));
        QTableWidgetItem *___qtablewidgetitem117 = tableStock->horizontalHeaderItem(6);
        ___qtablewidgetitem117->setText(QCoreApplication::translate("MainWindow", "Date d'ajout", nullptr));
        QTableWidgetItem *___qtablewidgetitem118 = tableStock->horizontalHeaderItem(7);
        ___qtablewidgetitem118->setText(QCoreApplication::translate("MainWindow", "CIN vente", nullptr));
        QTableWidgetItem *___qtablewidgetitem119 = tableStock->horizontalHeaderItem(8);
        ___qtablewidgetitem119->setText(QCoreApplication::translate("MainWindow", "N\302\260 commande", nullptr));
        QTableWidgetItem *___qtablewidgetitem120 = tableStock->horizontalHeaderItem(9);
        ___qtablewidgetitem120->setText(QCoreApplication::translate("MainWindow", "Date vente", nullptr));
        tabWidgetStockVentes->setTabText(tabWidgetStockVentes->indexOf(tabStock), QCoreApplication::translate("MainWindow", "Stock", nullptr));
        venteAcheteurLabel->setText(QCoreApplication::translate("MainWindow", "Acheteur:", nullptr));
        venteCinLabel->setText(QCoreApplication::translate("MainWindow", "CIN:", nullptr));
        venteQuantiteLabel->setText(QCoreApplication::translate("MainWindow", "Quantit\303\251:", nullptr));
        venteQuantiteEdit->setPlaceholderText(QCoreApplication::translate("MainWindow", "En KG", nullptr));
        venteIdLabel->setText(QCoreApplication::translate("MainWindow", "ID Vente:", nullptr));
        venteStockIdLabel->setText(QCoreApplication::translate("MainWindow", "Stock ID:", nullptr));
        venteSeuilMaxLabel->setText(QCoreApplication::translate("MainWindow", "Seuil maximum:", nullptr));
        venteDateLabel->setText(QCoreApplication::translate("MainWindow", "Date de vente:", nullptr));
        venteDateEdit->setPlaceholderText(QCoreApplication::translate("MainWindow", "JJ/MM/AA", nullptr));
        btnVenteAjouter->setText(QCoreApplication::translate("MainWindow", "Ajouter", nullptr));
        btnVenteModifier->setText(QCoreApplication::translate("MainWindow", "Modifier", nullptr));
        btnVenteSupprimer->setText(QCoreApplication::translate("MainWindow", "Supprimer", nullptr));
        QTableWidgetItem *___qtablewidgetitem121 = tableVentes->horizontalHeaderItem(0);
        ___qtablewidgetitem121->setText(QCoreApplication::translate("MainWindow", "ID Vente", nullptr));
        QTableWidgetItem *___qtablewidgetitem122 = tableVentes->horizontalHeaderItem(1);
        ___qtablewidgetitem122->setText(QCoreApplication::translate("MainWindow", "Acheteur", nullptr));
        QTableWidgetItem *___qtablewidgetitem123 = tableVentes->horizontalHeaderItem(2);
        ___qtablewidgetitem123->setText(QCoreApplication::translate("MainWindow", "CIN", nullptr));
        QTableWidgetItem *___qtablewidgetitem124 = tableVentes->horizontalHeaderItem(3);
        ___qtablewidgetitem124->setText(QCoreApplication::translate("MainWindow", "Quantit\303\251", nullptr));
        QTableWidgetItem *___qtablewidgetitem125 = tableVentes->horizontalHeaderItem(4);
        ___qtablewidgetitem125->setText(QCoreApplication::translate("MainWindow", "Stock ID", nullptr));
        QTableWidgetItem *___qtablewidgetitem126 = tableVentes->horizontalHeaderItem(5);
        ___qtablewidgetitem126->setText(QCoreApplication::translate("MainWindow", "Seuil max", nullptr));
        QTableWidgetItem *___qtablewidgetitem127 = tableVentes->horizontalHeaderItem(6);
        ___qtablewidgetitem127->setText(QCoreApplication::translate("MainWindow", "Date de vente", nullptr));
        venteSearchEdit->setPlaceholderText(QCoreApplication::translate("MainWindow", "Recherchez...", nullptr));
        venteSortLabel->setText(QCoreApplication::translate("MainWindow", "Trier par:", nullptr));
        venteSortCombo->setItemText(0, QCoreApplication::translate("MainWindow", "Acheteur", nullptr));
        venteSortCombo->setItemText(1, QCoreApplication::translate("MainWindow", "CIN", nullptr));
        venteSortCombo->setItemText(2, QCoreApplication::translate("MainWindow", "Quantit\303\251", nullptr));

        tabWidgetStockVentes->setTabText(tabWidgetStockVentes->indexOf(tabVentes), QCoreApplication::translate("MainWindow", "Ventes", nullptr));
        btnExportHistoriquePdf->setText(QCoreApplication::translate("MainWindow", "Export PDF", nullptr));
        tabWidgetStockVentes->setTabText(tabWidgetStockVentes->indexOf(tabHistorique), QCoreApplication::translate("MainWindow", "Histoire", nullptr));
        lineEditAssistant->setPlaceholderText(QCoreApplication::translate("MainWindow", "Tapez ici...", nullptr));
        btnAssistantGo->setText(QCoreApplication::translate("MainWindow", "GO", nullptr));
        tabWidgetStockVentes->setTabText(tabWidgetStockVentes->indexOf(tabAssistantIA), QCoreApplication::translate("MainWindow", "Assistant IA", nullptr));
        btnSourceAny->setText(QCoreApplication::translate("MainWindow", "Source de tout type", nullptr));
        tabWidgetStockVentes->setTabText(tabWidgetStockVentes->indexOf(tabSource), QCoreApplication::translate("MainWindow", "Source", nullptr));
        label_welcome->setText(QString());
        label_20->setText(QString());
        label_28->setText(QString());
        label_32->setText(QString());
        label_33->setText(QCoreApplication::translate("MainWindow", "Log In", nullptr));
        lineEdit_5->setPlaceholderText(QCoreApplication::translate("MainWindow", "  Nom d'etulisateur", nullptr));
        lineEdit_6->setPlaceholderText(QCoreApplication::translate("MainWindow", "  Mot de passe", nullptr));
        pushButton_3->setText(QCoreApplication::translate("MainWindow", "L o g  I n", nullptr));
        label_35->setText(QString());
        label_36->setText(QCoreApplication::translate("MainWindow", "ATLAS", nullptr));
        label_37->setText(QCoreApplication::translate("MainWindow", "Advanced Technology for\n"
"Logistics & Aquatic Systems", nullptr));
        pushButton_6->setText(QCoreApplication::translate("MainWindow", "Reconesance Faciale", nullptr));
        pushButton->setText(QCoreApplication::translate("MainWindow", "Mot de passe oublier", nullptr));
        label_38->setText(QString());
        label_p1->setText(QString());
        label_39->setText(QString());
        label_40->setText(QString());
        label_41->setText(QString());
        label_42->setText(QString());
        label_43->setText(QCoreApplication::translate("MainWindow", "Reconesance Faciale", nullptr));
        label_44->setText(QString());
        label_45->setText(QCoreApplication::translate("MainWindow", "ATLAS", nullptr));
        label_46->setText(QCoreApplication::translate("MainWindow", "Advanced Technology for\n"
"Logistics & Aquatic Systems", nullptr));
        pushButton_8->setText(QCoreApplication::translate("MainWindow", "Retourner", nullptr));
        label_photo_3->setText(QCoreApplication::translate("MainWindow", "Photo Placeholder", nullptr));
        label_p2->setText(QString());
        label_47->setText(QString());
        label_48->setText(QString());
        label_49->setText(QString());
        label_50->setText(QString());
        label_51->setText(QCoreApplication::translate("MainWindow", "changer le mot de passe ", nullptr));
        lineEdit_7->setPlaceholderText(QCoreApplication::translate("MainWindow", "    Mot de passe", nullptr));
        lineEdit_9->setPlaceholderText(QCoreApplication::translate("MainWindow", "  Comfirmer   Mot de passe", nullptr));
        pushButton_9->setText(QCoreApplication::translate("MainWindow", "Confirmer", nullptr));
        label_52->setText(QString());
        label_53->setText(QCoreApplication::translate("MainWindow", "ATLAS", nullptr));
        label_54->setText(QCoreApplication::translate("MainWindow", "Advanced Technology for\n"
"Logistics & Aquatic Systems", nullptr));
        pushButton_11->setText(QCoreApplication::translate("MainWindow", "Retourner", nullptr));
        label_p3->setText(QString());
        label_55->setText(QString());
        label_56->setText(QString());
        label_57->setText(QString());
        label_58->setText(QString());
        label_59->setText(QCoreApplication::translate("MainWindow", "Mail verification ", nullptr));
        lineEdit_10->setPlaceholderText(QCoreApplication::translate("MainWindow", "  Code", nullptr));
        pushButton_10->setText(QCoreApplication::translate("MainWindow", "Verifier", nullptr));
        label_60->setText(QString());
        label_61->setText(QCoreApplication::translate("MainWindow", "ATLAS", nullptr));
        label_62->setText(QCoreApplication::translate("MainWindow", "Advanced Technology for\n"
"Logistics & Aquatic Systems", nullptr));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_TEST_H
