#ifndef QUAI_ARDUINO_WINDOW_H
#define QUAI_ARDUINO_WINDOW_H

#include <QWidget>

QT_BEGIN_NAMESPACE
namespace Ui {
class QuaiArduinoWindow;
}
QT_END_NAMESPACE

/**
 * Fenêtre indépendante pour la maquette Quai_sys (porte capteurs + base).
 * Ne modifie pas les autres écrans du module Quais.
 */
class QuaiArduinoWindow : public QWidget
{
    Q_OBJECT

public:
    explicit QuaiArduinoWindow(QWidget *parent = nullptr);
    ~QuaiArduinoWindow() override;

public slots:
    void refreshActiveQuaisCombo();
    /** Horodaté ; tronque si le journal dépasse ~500 lignes. */
    void appendQuaiGateLog(const QString &message);

signals:
    /** Émis après mise à jour réussie en base (statut → Inactif). */
    void assignmentCompleted(int quaiId);
    void forceOpenQuaiGateRequested();
    void forceCloseQuaiGateRequested();

private slots:
    void onValidateClicked();
    void onRefreshClicked();
    void onForceOpenClicked();
    void onForceCloseClicked();
    void onClearLogClicked();

private:
    void refreshStatBoxes();

    Ui::QuaiArduinoWindow *ui;
};

#endif // QUAI_ARDUINO_WINDOW_H
