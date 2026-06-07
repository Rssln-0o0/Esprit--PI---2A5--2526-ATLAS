#ifndef EMPLOYEE_H
#define EMPLOYEE_H

#include <QDate>
#include <QDateTime>
#include <QList>
#include <QSqlDatabase>
#include <QString>
#include <QStringList>

struct EmployeeRow
{
    int id = -1;
    QString cin;
    QString prenom;
    QString nom;
    QString occupation;
    QString departement;
    QDate date_embauche;
    QString statut;
    QString telephone;
    QString email;
    QString motdepasse;
    QString ville;
    QString adresse;
    QString certifications;
    QString niveaux;
    QString num_licence;
    QDate date_expiration;
    QString quart;
    QString rfid;
    QString emp_code; // colonne BDD "CODE"
    QString photo_path;
    /** RFID / Smart Port — colonnes DATE_ENTREE, DATE_SORTIE (nullable). */
    QDateTime port_date_entree;
    QDateTime port_date_sortie;
};

// SMTP helper (used by "Mot de passe oublié")
struct SmtpConfig
{
    QString host;
    int port = 465;
    bool ssl = true;          // direct TLS (SMTPS)
    bool startTls = false;    // for port 587
    QString username;
    QString password;
    QString from;
};

class SmtpClient
{
public:
    static bool sendMail(const SmtpConfig &cfg,
                         const QString &to,
                         const QString &subject,
                         const QString &body,
                         QString *errorMessage = nullptr);
};

namespace EmployeePassword {

QString normalizeEmailInput(const QString &email, QString *correctedFrom = nullptr);
bool isEmailFormatValid(const QString &email);

QString generateResetCode(int digits = 6);

bool emailExists(QSqlDatabase &db, const QString &email, bool *existsOut, QString *errorMessage = nullptr);

bool updatePasswordByEmail(QSqlDatabase &db, const QString &email, const QString &newPassword, QString *errorMessage = nullptr);

QStringList smtpIniSearchPaths();
bool loadSmtpConfigFromIni(const QString &iniPath, SmtpConfig *out, QString *errorMessage = nullptr);

bool loadSmtpConfigAuto(SmtpConfig *out,
                        QString *usedIniPath = nullptr,
                        QStringList *searchedPaths = nullptr,
                        QString *errorMessage = nullptr);

bool sendResetCodeEmail(const QString &toEmail,
                        const QString &code,
                        QString *errorMessage = nullptr);

}

namespace EmployeeCrud {

bool ensureTable(QSqlDatabase &db, QString *errorMessage = nullptr);

/** Email / password used for the login screen when aucun compte n'existe encore (créé au démarrage). */
QString defaultLoginEmail();
QString defaultLoginPassword();

/** Insère un employé par défaut si aucune ligne avec defaultLoginEmail() n'existe (pour pouvoir se connecter). */
bool ensureDefaultLoginUser(QSqlDatabase &db, QString *errorMessage = nullptr);

bool cinExists(QSqlDatabase &db, const QString &cin, const QString &excludeCin, bool *existsOut, QString *errorMessage = nullptr);

bool insert(QSqlDatabase &db, const EmployeeRow &e, int *newId = nullptr, QString *errorMessage = nullptr);
bool update(QSqlDatabase &db, const QString &cin, const EmployeeRow &e, QString *errorMessage = nullptr);
bool remove(QSqlDatabase &db, const QString &cin, QString *errorMessage = nullptr);

// UI -> DB: query with optional WHERE and ORDER BY for dynamic search/sort.
QList<EmployeeRow> listWithSort(QSqlDatabase &db, const QString &searchTerm = QString(), int sortIndex = 0, QString *errorMessage = nullptr);

// Legacy list method (kept for compatibility).
QList<EmployeeRow> list(QSqlDatabase &db, const QString &searchTerm = QString(), QString *errorMessage = nullptr);
bool getByCin(QSqlDatabase &db, const QString &cin, EmployeeRow *out, QString *errorMessage = nullptr);
/** uidHex : UID sans espaces (ex. 630A1FA8), insensible à la casse. Cherche dans rfid et/ou code (même valeur normalisée). */
bool getByRfid(QSqlDatabase &db, const QString &uidHex, EmployeeRow *out, QString *errorMessage = nullptr);

/** Horodatage passage entrée porte (colonne DATE_ENTREE / TIMESTAMP). */
bool recordSmartPortEntree(QSqlDatabase &db, const QString &cin, QString *errorMessage = nullptr);
/** Horodatage passage sortie porte (colonne DATE_SORTIE / TIMESTAMP). */
bool recordSmartPortSortie(QSqlDatabase &db, const QString &cin, QString *errorMessage = nullptr);

}

#endif // EMPLOYEE_H
