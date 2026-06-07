#include "employee.h"

#include <QSqlError>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QSslSocket>
#include <QByteArray>
#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QMap>
#include <QSettings>
#include <QRandomGenerator>
#include <QRegularExpression>
#include <cmath>

static QString normalizedLike(const QString &s)
{
    return "%" + s.trimmed() + "%";
}

static QString pickRecordField(const QSqlRecord &rec, const QStringList &candidates)
{
    for (const QString &want : candidates) {
        const int i = rec.indexOf(want);
        if (i >= 0)
            return rec.fieldName(i);
    }
    for (int i = 0; i < rec.count(); ++i) {
        const QString fn = rec.fieldName(i);
        for (const QString &want : candidates) {
            if (QString::compare(fn, want, Qt::CaseInsensitive) == 0)
                return fn;
        }
    }
    return QString();
}

/** Oracle (QOCI, QODBC→Oracle) n'accepte pas LIMIT 1 (erreur ORA-00933). */
static QString sqlSelectOneRowSuffix(const QSqlDatabase &db)
{
    const QString drv = db.driverName();
    if (drv.contains(QStringLiteral("OCI"), Qt::CaseInsensitive)
        || drv.contains(QStringLiteral("ODBC"), Qt::CaseInsensitive)) {
        return QStringLiteral(" AND ROWNUM <= 1");
    }
    return QStringLiteral(" LIMIT 1");
}

static QDate toDateAny(const QVariant &v)
{
    const QDate d = v.toDate();
    if (d.isValid())
        return d;
    const QString s = v.toString().trimmed();
    if (s.isEmpty())
        return {};
    // Common ODBC string formats
    const QDate d1 = QDate::fromString(s, "yyyy-MM-dd");
    if (d1.isValid())
        return d1;
    const QDate d2 = QDate::fromString(s, "dd/MM/yyyy");
    if (d2.isValid())
        return d2;
    const QDate d3 = QDate::fromString(s.left(10), "yyyy-MM-dd");
    if (d3.isValid())
        return d3;
    return {};
}

bool EmployeeCrud::ensureTable(QSqlDatabase &db, QString *errorMessage)
{
    if (!db.isValid() || !db.isOpen()) {
        if (errorMessage) *errorMessage = "Connexion base de données indisponible.";
        return false;
    }
    QSqlQuery q(db);
    if (q.exec("SELECT 1 FROM employe")) {
        return true;
    }
    if (errorMessage)
        *errorMessage = q.lastError().text();
    return false;
}

static const QString kDefaultLoginEmail = QStringLiteral("rayan@port.local");
static const QString kDefaultLoginPassword = QStringLiteral("test123");

QString EmployeeCrud::defaultLoginEmail()
{
    return kDefaultLoginEmail;
}

QString EmployeeCrud::defaultLoginPassword()
{
    return kDefaultLoginPassword;
}

bool EmployeeCrud::ensureDefaultLoginUser(QSqlDatabase &db, QString *errorMessage)
{
    if (!ensureTable(db, errorMessage))
        return false;

    QSqlQuery q(db);
    q.prepare("SELECT 1 FROM employe WHERE UPPER(adresse_email)=UPPER(?)");
    q.addBindValue(kDefaultLoginEmail);
    if (!q.exec()) {
        if (errorMessage) *errorMessage = q.lastError().text();
        return false;
    }
    if (q.next())
        return true;

    EmployeeRow e;
    e.prenom = QStringLiteral("Rayan");
    e.nom = QStringLiteral("Admin");
    e.photo_path = QString();
    e.occupation = QStringLiteral("Directeur");
    e.departement = QStringLiteral("Direction");
    e.date_embauche = QDate::currentDate();
    e.statut = QStringLiteral("Actif");
    e.telephone = QStringLiteral("+21612345678");
    e.email = kDefaultLoginEmail;
    e.motdepasse = kDefaultLoginPassword;
    e.ville = QStringLiteral("Tunis");
    e.adresse = QStringLiteral("Port");
    e.certifications = QString();
    e.niveaux = QString();
    e.num_licence = QString();
    e.date_expiration = QDate::currentDate().addYears(1);
    e.quart = QStringLiteral("Matin");

    for (int attempt = 0; attempt < 100; ++attempt) {
        e.cin = QString::number(90000001 + attempt).rightJustified(8, QLatin1Char('0'));
        bool exists = false;
        if (!cinExists(db, e.cin, QString(), &exists, errorMessage))
            return false;
        if (!exists)
            return insert(db, e, nullptr, errorMessage);
    }
    if (errorMessage)
        *errorMessage = QStringLiteral("Aucun CIN libre dans la plage 90000001–90000100 pour le compte par défaut.");
    return false;
}

// ---------------- SMTP (moved from smtpclient.cpp) ----------------

static QByteArray b64(const QString &s)
{
    return s.toUtf8().toBase64();
}

static bool waitRead(QSslSocket &sock, int timeoutMs, QString *err)
{
    if (!sock.waitForReadyRead(timeoutMs)) {
        if (err) *err = QStringLiteral("SMTP: timeout (%1)").arg(sock.errorString());
        return false;
    }
    return true;
}

static QString readResponse(QSslSocket &sock, int timeoutMs, QString *err)
{
    if (!waitRead(sock, timeoutMs, err))
        return {};
    QByteArray all;
    // Read available lines; SMTP multiline uses "xyz-" prefix, end with "xyz "
    while (sock.canReadLine() || sock.bytesAvailable() > 0) {
        all += sock.readAll();
        if (!sock.waitForReadyRead(20))
            break;
    }
    return QString::fromUtf8(all);
}

static bool expectCode(const QString &resp, int code)
{
    return resp.trimmed().startsWith(QString::number(code));
}

static bool sendLine(QSslSocket &sock, const QByteArray &line, QString *err)
{
    const qint64 n = sock.write(line + "\r\n");
    if (n < 0) {
        if (err) *err = QStringLiteral("SMTP: write failed (%1)").arg(sock.errorString());
        return false;
    }
    if (!sock.waitForBytesWritten(5000)) {
        if (err) *err = QStringLiteral("SMTP: write timeout (%1)").arg(sock.errorString());
        return false;
    }
    return true;
}

bool SmtpClient::sendMail(const SmtpConfig &cfg,
                          const QString &to,
                          const QString &subject,
                          const QString &body,
                          QString *errorMessage)
{
    QString err;
    if (cfg.host.trimmed().isEmpty()) {
        if (errorMessage) *errorMessage = "SMTP: host vide (configure smtp.ini).";
        return false;
    }
    if (cfg.username.trimmed().isEmpty() || cfg.password.isEmpty() || cfg.from.trimmed().isEmpty()) {
        if (errorMessage) *errorMessage = "SMTP: username/password/from manquants (configure smtp.ini).";
        return false;
    }
    if (to.trimmed().isEmpty()) {
        if (errorMessage) *errorMessage = "SMTP: destinataire vide.";
        return false;
    }

    QSslSocket sock;
    sock.setPeerVerifyMode(QSslSocket::VerifyNone); // for dev; production should verify certs

    if (cfg.ssl) {
        sock.connectToHostEncrypted(cfg.host, cfg.port);
        if (!sock.waitForEncrypted(10000)) {
            if (errorMessage) *errorMessage = QStringLiteral("SMTP: TLS failed (%1)").arg(sock.errorString());
            return false;
        }
    } else {
        sock.connectToHost(cfg.host, cfg.port);
        if (!sock.waitForConnected(10000)) {
            if (errorMessage) *errorMessage = QStringLiteral("SMTP: connect failed (%1)").arg(sock.errorString());
            return false;
        }
    }

    QString resp = readResponse(sock, 10000, &err);
    if (!expectCode(resp, 220)) {
        if (errorMessage) *errorMessage = QStringLiteral("SMTP: greeting unexpected:\n%1").arg(resp.trimmed());
        return false;
    }

    if (!sendLine(sock, "EHLO atlas", &err)) { if (errorMessage) *errorMessage = err; return false; }
    resp = readResponse(sock, 10000, &err);
    if (!expectCode(resp, 250)) {
        // fallback HELO
        if (!sendLine(sock, "HELO atlas", &err)) { if (errorMessage) *errorMessage = err; return false; }
        resp = readResponse(sock, 10000, &err);
        if (!expectCode(resp, 250)) {
            if (errorMessage) *errorMessage = QStringLiteral("SMTP: EHLO/HELO failed:\n%1").arg(resp.trimmed());
            return false;
        }
    }

    if (!cfg.ssl && cfg.startTls) {
        if (!sendLine(sock, "STARTTLS", &err)) { if (errorMessage) *errorMessage = err; return false; }
        resp = readResponse(sock, 10000, &err);
        if (!expectCode(resp, 220)) {
            if (errorMessage) *errorMessage = QStringLiteral("SMTP: STARTTLS failed:\n%1").arg(resp.trimmed());
            return false;
        }
        sock.startClientEncryption();
        if (!sock.waitForEncrypted(10000)) {
            if (errorMessage) *errorMessage = QStringLiteral("SMTP: TLS handshake failed (%1)").arg(sock.errorString());
            return false;
        }
        if (!sendLine(sock, "EHLO atlas", &err)) { if (errorMessage) *errorMessage = err; return false; }
        resp = readResponse(sock, 10000, &err);
        if (!expectCode(resp, 250)) {
            if (errorMessage) *errorMessage = QStringLiteral("SMTP: EHLO after STARTTLS failed:\n%1").arg(resp.trimmed());
            return false;
        }
    }

    if (!sendLine(sock, "AUTH LOGIN", &err)) { if (errorMessage) *errorMessage = err; return false; }
    resp = readResponse(sock, 10000, &err);
    if (!expectCode(resp, 334)) {
        if (errorMessage) *errorMessage = QStringLiteral("SMTP: AUTH LOGIN rejected:\n%1").arg(resp.trimmed());
        return false;
    }
    if (!sendLine(sock, b64(cfg.username), &err)) { if (errorMessage) *errorMessage = err; return false; }
    resp = readResponse(sock, 10000, &err);
    if (!expectCode(resp, 334)) {
        if (errorMessage) *errorMessage = QStringLiteral("SMTP: username rejected:\n%1").arg(resp.trimmed());
        return false;
    }
    if (!sendLine(sock, b64(cfg.password), &err)) { if (errorMessage) *errorMessage = err; return false; }
    resp = readResponse(sock, 10000, &err);
    if (!expectCode(resp, 235)) {
        if (errorMessage) *errorMessage = QStringLiteral("SMTP: password rejected:\n%1").arg(resp.trimmed());
        return false;
    }

    if (!sendLine(sock, QByteArray("MAIL FROM:<") + cfg.from.toUtf8() + ">", &err)) { if (errorMessage) *errorMessage = err; return false; }
    resp = readResponse(sock, 10000, &err);
    if (!expectCode(resp, 250)) {
        if (errorMessage) *errorMessage = QStringLiteral("SMTP: MAIL FROM failed:\n%1").arg(resp.trimmed());
        return false;
    }

    if (!sendLine(sock, QByteArray("RCPT TO:<") + to.toUtf8() + ">", &err)) { if (errorMessage) *errorMessage = err; return false; }
    resp = readResponse(sock, 10000, &err);
    if (!(expectCode(resp, 250) || expectCode(resp, 251))) {
        if (errorMessage) *errorMessage = QStringLiteral("SMTP: RCPT TO failed:\n%1").arg(resp.trimmed());
        return false;
    }

    if (!sendLine(sock, "DATA", &err)) { if (errorMessage) *errorMessage = err; return false; }
    resp = readResponse(sock, 10000, &err);
    if (!expectCode(resp, 354)) {
        if (errorMessage) *errorMessage = QStringLiteral("SMTP: DATA failed:\n%1").arg(resp.trimmed());
        return false;
    }

    QString msg;
    msg += "From: <" + cfg.from + ">\r\n";
    msg += "To: <" + to + ">\r\n";
    msg += "Subject: " + subject + "\r\n";
    msg += "MIME-Version: 1.0\r\n";
    msg += "Content-Type: text/plain; charset=\"utf-8\"\r\n";
    msg += "Content-Transfer-Encoding: 8bit\r\n";
    msg += "\r\n";
    msg += body + "\r\n";
    msg += ".\r\n";

    const qint64 wn = sock.write(msg.toUtf8());
    if (wn < 0 || !sock.waitForBytesWritten(10000)) {
        if (errorMessage) *errorMessage = QStringLiteral("SMTP: failed writing body (%1)").arg(sock.errorString());
        return false;
    }
    resp = readResponse(sock, 15000, &err);
    if (!expectCode(resp, 250)) {
        if (errorMessage) *errorMessage = QStringLiteral("SMTP: message rejected:\n%1").arg(resp.trimmed());
        return false;
    }

    sendLine(sock, "QUIT", &err);
    sock.disconnectFromHost();
    if (errorMessage) *errorMessage = QString();
    return true;
}

// ---------------- Forgot password (email) ----------------

static QString stripUtf8Bom(QString s)
{
    if (!s.isEmpty() && s.at(0) == QChar(0xFEFF))
        s.remove(0, 1);
    return s;
}

QString EmployeePassword::normalizeEmailInput(const QString &email, QString *correctedFrom)
{
    QString cleaned = email.trimmed().toLower();
    cleaned.remove(' ');
    if (correctedFrom)
        correctedFrom->clear();

    const int atPos = cleaned.indexOf('@');
    if (atPos <= 0 || atPos >= cleaned.size() - 1)
        return cleaned;

    const QString localPart = cleaned.left(atPos);
    QString domain = cleaned.mid(atPos + 1);
    static const QMap<QString, QString> domainFixups = {
        {QStringLiteral("gamil.com"), QStringLiteral("gmail.com")},
        {QStringLiteral("gmial.com"), QStringLiteral("gmail.com")},
        {QStringLiteral("gmai.com"), QStringLiteral("gmail.com")},
        {QStringLiteral("gmail.fr"), QStringLiteral("gmail.com")},
        {QStringLiteral("hotnail.com"), QStringLiteral("hotmail.com")},
        {QStringLiteral("hotmai.com"), QStringLiteral("hotmail.com")},
        {QStringLiteral("outlok.com"), QStringLiteral("outlook.com")},
        {QStringLiteral("outllok.com"), QStringLiteral("outlook.com")},
        {QStringLiteral("yaho.com"), QStringLiteral("yahoo.com")},
        {QStringLiteral("yahho.com"), QStringLiteral("yahoo.com")},
        {QStringLiteral("iclod.com"), QStringLiteral("icloud.com")}
    };
    const QString fixedDomain = domainFixups.value(domain, domain);
    if (fixedDomain != domain && correctedFrom)
        *correctedFrom = cleaned;
    domain = fixedDomain;
    return localPart + QStringLiteral("@") + domain;
}

bool EmployeePassword::isEmailFormatValid(const QString &email)
{
    static const QRegularExpression rx(QStringLiteral(R"(^[^@\s]+@[^@\s]+\.[^@\s]+$)"));
    return rx.match(email).hasMatch();
}

QString EmployeePassword::generateResetCode(int digits)
{
    if (digits < 4) digits = 4;
    if (digits > 10) digits = 10;
    const int max = int(std::pow(10.0, digits));
    const int v = int(QRandomGenerator::global()->bounded(max));
    return QString("%1").arg(v, digits, 10, QLatin1Char('0'));
}

bool EmployeePassword::emailExists(QSqlDatabase &db, const QString &email, bool *existsOut, QString *errorMessage)
{
    if (existsOut) *existsOut = false;
    if (!db.isValid() || !db.isOpen()) {
        if (errorMessage) *errorMessage = "Connexion base de données indisponible.";
        return false;
    }
    const QString raw = email.trimmed();
    const QString normalized = normalizeEmailInput(raw);
    auto queryExists = [&](const QString &candidate, bool *found) -> bool {
        if (found) *found = false;
        QSqlQuery q(db);
        q.prepare("SELECT 1 FROM employe WHERE UPPER(adresse_email)=UPPER(?)");
        q.addBindValue(candidate);
        if (!q.exec()) {
            if (errorMessage) *errorMessage = q.lastError().text();
            return false;
        }
        if (found) *found = q.next();
        return true;
    };

    bool found = false;
    if (!queryExists(normalized, &found))
        return false;
    if (!found && !raw.isEmpty() && QString::compare(raw, normalized, Qt::CaseInsensitive) != 0) {
        if (!queryExists(raw, &found))
            return false;
    }
    if (existsOut) *existsOut = found;
    return true;
}

bool EmployeePassword::updatePasswordByEmail(QSqlDatabase &db, const QString &email, const QString &newPassword, QString *errorMessage)
{
    if (!db.isValid() || !db.isOpen()) {
        if (errorMessage) *errorMessage = "Connexion base de données indisponible.";
        return false;
    }
    const QString raw = email.trimmed();
    const QString normalized = normalizeEmailInput(raw);
    auto updateByEmail = [&](const QString &candidate, int *rowsOut) -> bool {
        if (rowsOut) *rowsOut = 0;
        QSqlQuery q(db);
        q.prepare("UPDATE employe SET mot_de_passe=? WHERE UPPER(adresse_email)=UPPER(?)");
        q.addBindValue(newPassword);
        q.addBindValue(candidate);
        if (!q.exec()) {
            if (errorMessage) *errorMessage = q.lastError().text();
            return false;
        }
        if (rowsOut) *rowsOut = q.numRowsAffected();
        return true;
    };

    int rows = 0;
    if (!updateByEmail(normalized, &rows))
        return false;
    if (rows <= 0 && !raw.isEmpty() && QString::compare(raw, normalized, Qt::CaseInsensitive) != 0) {
        if (!updateByEmail(raw, &rows))
            return false;
    }
    if (errorMessage) *errorMessage = QString();
    return true;
}

QStringList EmployeePassword::smtpIniSearchPaths()
{
    QStringList out;

    QSettings appSettings("ATLAS", "ATLAS");
    const QString saved = appSettings.value("smtp/iniPath").toString().trimmed();
    if (!saved.isEmpty())
        out << saved;

    out << (QCoreApplication::applicationDirPath() + "/smtp.ini");
    out << (QDir::currentPath() + "/smtp.ini");
    return out;
}

static bool manualParseSmtpIni(const QString &path, SmtpConfig *cfg)
{
    if (!cfg) return false;
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text))
        return false;
    QString section;
    QMap<QString, QString> kv;
    while (!f.atEnd()) {
        QString line = stripUtf8Bom(QString::fromUtf8(f.readLine()).trimmed());
        if (line.isEmpty() || line.startsWith('#') || line.startsWith(';'))
            continue;
        if (line.startsWith('[') && line.endsWith(']')) {
            section = line.mid(1, line.size() - 2).trimmed().toLower();
            continue;
        }
        if (section != "smtp")
            continue;
        const int eq = line.indexOf('=');
        if (eq <= 0)
            continue;
        const QString key = line.left(eq).trimmed().toLower();
        const QString val = line.mid(eq + 1).trimmed();
        kv.insert(key, val);
    }
    if (kv.isEmpty())
        return false;

    if (cfg->host.isEmpty()) cfg->host = kv.value("host").trimmed();
    if (!kv.value("port").isEmpty()) cfg->port = kv.value("port").toInt();
    if (!kv.value("ssl").isEmpty()) cfg->ssl = (kv.value("ssl").trimmed().toLower() == "true" || kv.value("ssl") == "1");
    if (!kv.value("starttls").isEmpty()) cfg->startTls = (kv.value("starttls").trimmed().toLower() == "true" || kv.value("starttls") == "1");
    if (cfg->username.isEmpty()) cfg->username = kv.value("username").trimmed();
    if (cfg->password.isEmpty()) cfg->password = kv.value("password");
    if (cfg->from.isEmpty()) cfg->from = kv.value("from").trimmed();
    return true;
}

bool EmployeePassword::loadSmtpConfigFromIni(const QString &iniPath, SmtpConfig *out, QString *errorMessage)
{
    if (!out) {
        if (errorMessage) *errorMessage = "SMTP: config out nullptr.";
        return false;
    }
    *out = SmtpConfig{};
    if (iniPath.trimmed().isEmpty() || !QFileInfo::exists(iniPath)) {
        if (errorMessage) *errorMessage = "SMTP: smtp.ini introuvable.";
        return false;
    }

    // Try QSettings first
    QSettings ini(iniPath, QSettings::IniFormat);
    ini.beginGroup("smtp");
    out->host = ini.value("host").toString().trimmed();
    out->port = ini.value("port", 465).toInt();
    out->ssl = ini.value("ssl", true).toBool();
    out->startTls = ini.value("startTls", false).toBool();
    out->username = ini.value("username").toString().trimmed();
    out->password = ini.value("password").toString();
    out->from = ini.value("from").toString().trimmed();
    ini.endGroup();

    // Fallback manual parse if needed
    if (out->host.isEmpty() || out->username.isEmpty() || out->password.isEmpty() || out->from.isEmpty())
        manualParseSmtpIni(iniPath, out);

    const bool ok = !(out->host.isEmpty() || out->username.isEmpty() || out->password.isEmpty() || out->from.isEmpty());
    if (!ok && errorMessage)
        *errorMessage = "SMTP: champs manquants dans smtp.ini.";
    return ok;
}

bool EmployeePassword::loadSmtpConfigAuto(SmtpConfig *out, QString *usedIniPath, QStringList *searchedPaths, QString *errorMessage)
{
    const QStringList paths = smtpIniSearchPaths();
    if (searchedPaths) *searchedPaths = paths;
    QString lastErr;
    for (const QString &p : paths) {
        QString e;
        SmtpConfig cfg;
        if (loadSmtpConfigFromIni(p, &cfg, &e)) {
            if (out) *out = cfg;
            if (usedIniPath) *usedIniPath = p;
            if (errorMessage) *errorMessage = QString();
            return true;
        }
        lastErr = e;
    }
    if (usedIniPath) usedIniPath->clear();
    if (errorMessage) *errorMessage = lastErr.isEmpty() ? "SMTP: configuration introuvable." : lastErr;
    return false;
}

bool EmployeePassword::sendResetCodeEmail(const QString &toEmail, const QString &code, QString *errorMessage)
{
    const QString normalizedTo = normalizeEmailInput(toEmail);
    if (!isEmailFormatValid(normalizedTo)) {
        if (errorMessage) *errorMessage = QStringLiteral("SMTP: adresse destinataire invalide.");
        return false;
    }

    SmtpConfig cfg;
    QString err;
    if (!loadSmtpConfigAuto(&cfg, nullptr, nullptr, &err)) {
        if (errorMessage) *errorMessage = err;
        return false;
    }

    const QString subject = "ATLAS - Code de réinitialisation";
    const QString body =
        "Bonjour,\n\n"
        "Voici votre code de réinitialisation du mot de passe:\n\n"
        "CODE: " + code + "\n\n"
        "Si vous n'êtes pas à l'origine de cette demande, ignorez ce mail.\n";

    QString sendErr;
    const bool ok = SmtpClient::sendMail(cfg, normalizedTo, subject, body, &sendErr);
    if (!ok) {
        if (errorMessage) *errorMessage = sendErr;
        return false;
    }
    if (errorMessage) *errorMessage = QString();
    return true;
}

bool EmployeeCrud::cinExists(QSqlDatabase &db, const QString &cin, const QString &excludeCin, bool *existsOut, QString *errorMessage)
{
    if (existsOut) *existsOut = false;
    if (!db.isValid() || !db.isOpen())
        return false;

    QSqlQuery q(db);
    if (!excludeCin.trimmed().isEmpty()) {
        q.prepare("SELECT COUNT(*) FROM employe WHERE cin=? AND cin<>?");
        q.addBindValue(cin);
        q.addBindValue(excludeCin.trimmed());
    } else {
        q.prepare("SELECT COUNT(*) FROM employe WHERE cin=?");
        q.addBindValue(cin);
    }

    if (!q.exec() || !q.next()) {
        if (errorMessage) *errorMessage = q.lastError().text();
        return false;
    }
    if (existsOut) *existsOut = (q.value(0).toInt() > 0);
    return true;
}

bool EmployeeCrud::insert(QSqlDatabase &db, const EmployeeRow &e, int *newId, QString *errorMessage)
{
    if (newId) *newId = -1;
    if (!db.isValid() || !db.isOpen()) {
        if (errorMessage) *errorMessage = "Connexion base de données indisponible.";
        return false;
    }

    QSqlQuery q(db);
    q.prepare(
        "INSERT INTO employe (cin, prenom, nom, photo, poste, service, date_embauche, statut_professionnel, numero_telephone, adresse_email, mot_de_passe, ville, adresse, certifications, niveau_competence, numero_license, date_expiration_certification, horaire_travail, rfid, code) "
        "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)"
    );
    q.addBindValue(e.cin);
    q.addBindValue(e.prenom);
    q.addBindValue(e.nom);
    q.addBindValue(e.photo_path);
    q.addBindValue(e.occupation);
    q.addBindValue(e.departement);
    q.addBindValue(e.date_embauche);
    q.addBindValue(e.statut);
    q.addBindValue(e.telephone);
    q.addBindValue(EmployeePassword::normalizeEmailInput(e.email));
    q.addBindValue(e.motdepasse);
    q.addBindValue(e.ville);
    q.addBindValue(e.adresse);
    q.addBindValue(e.certifications);
    q.addBindValue(e.niveaux);
    q.addBindValue(e.num_licence);
    q.addBindValue(e.date_expiration);
    q.addBindValue(e.quart);
    q.addBindValue(e.rfid);
    q.addBindValue(e.emp_code);

    if (!q.exec()) {
        if (errorMessage) *errorMessage = q.lastError().text();
        return false;
    }
    db.commit();
    return true;
}

bool EmployeeCrud::update(QSqlDatabase &db, const QString &cin, const EmployeeRow &e, QString *errorMessage)
{
    if (cin.trimmed().isEmpty()) {
        if (errorMessage) *errorMessage = "CIN invalide.";
        return false;
    }
    if (!db.isValid() || !db.isOpen()) {
        if (errorMessage) *errorMessage = "Connexion base de données indisponible.";
        return false;
    }

    const QString oldCin = cin.trimmed();
    const QString newCin = e.cin.trimmed();

    // Si le CIN change : 1) rompre FK (NULL), 2) mettre à jour employe, 3) rétablir FK
    // (On ne peut pas mettre PECHEUR.CIN=new avant que new existe dans employe)
    QList<int> pecheurIdsToRelink;
    if (!newCin.isEmpty() && newCin != oldCin) {
        QSqlQuery sel(db);
        sel.prepare("SELECT ID_PECHEUR FROM PECHEUR WHERE CIN = ?");
        sel.addBindValue(oldCin);
        if (sel.exec()) {
            while (sel.next())
                pecheurIdsToRelink.append(sel.value(0).toInt());
        }
        QSqlQuery q0(db);
        q0.prepare("UPDATE PECHEUR SET CIN = NULL WHERE CIN = ?");
        q0.addBindValue(oldCin);
        q0.exec();
    }

    QSqlQuery q(db);
    q.prepare(
        "UPDATE employe SET cin=?, prenom=?, nom=?, photo=?, poste=?, service=?, date_embauche=?, statut_professionnel=?, numero_telephone=?, adresse_email=?, mot_de_passe=?, ville=?, adresse=?, certifications=?, niveau_competence=?, numero_license=?, date_expiration_certification=?, horaire_travail=?, rfid=?, code=? "
        "WHERE cin=?"
    );
    q.addBindValue(newCin.isEmpty() ? oldCin : newCin);
    q.addBindValue(e.prenom);
    q.addBindValue(e.nom);
    q.addBindValue(e.photo_path);
    q.addBindValue(e.occupation);
    q.addBindValue(e.departement);
    q.addBindValue(e.date_embauche);
    q.addBindValue(e.statut);
    q.addBindValue(e.telephone);
    q.addBindValue(EmployeePassword::normalizeEmailInput(e.email));
    q.addBindValue(e.motdepasse);
    q.addBindValue(e.ville);
    q.addBindValue(e.adresse);
    q.addBindValue(e.certifications);
    q.addBindValue(e.niveaux);
    q.addBindValue(e.num_licence);
    q.addBindValue(e.date_expiration);
    q.addBindValue(e.quart);
    q.addBindValue(e.rfid);
    q.addBindValue(e.emp_code);
    q.addBindValue(oldCin);

    if (!q.exec()) {
        if (errorMessage) *errorMessage = q.lastError().text();
        return false;
    }

    // Rétablir la FK PECHEUR.CIN pour les pêcheurs qui référençaient l'ancien CIN
    if (!pecheurIdsToRelink.isEmpty()) {
        QSqlQuery q2(db);
        q2.prepare("UPDATE PECHEUR SET CIN = ? WHERE ID_PECHEUR = ?");
        for (int id : pecheurIdsToRelink) {
            q2.bindValue(0, newCin);
            q2.bindValue(1, id);
            q2.exec();
        }
    }

    db.commit();
    return true;
}

bool EmployeeCrud::remove(QSqlDatabase &db, const QString &cin, QString *errorMessage)
{
    if (cin.trimmed().isEmpty()) {
        if (errorMessage) *errorMessage = "CIN invalide.";
        return false;
    }
    if (!db.isValid() || !db.isOpen()) {
        if (errorMessage) *errorMessage = "Connexion base de données indisponible.";
        return false;
    }

    // Libérer la FK PECHEUR.CIN → EMPLOYE.CIN avant suppression (comme module Pêcheur)
    QSqlQuery q1(db);
    q1.prepare("UPDATE PECHEUR SET CIN = NULL WHERE CIN = ?");
    q1.addBindValue(cin.trimmed());
    if (!q1.exec() && errorMessage)
        *errorMessage = q1.lastError().text();

    QSqlQuery q(db);
    q.prepare("DELETE FROM employe WHERE cin=?");
    q.addBindValue(cin.trimmed());
    if (!q.exec()) {
        if (errorMessage) *errorMessage = q.lastError().text();
        return false;
    }
    db.commit();
    return true;
}

QList<EmployeeRow> EmployeeCrud::list(QSqlDatabase &db, const QString &searchTerm, QString *errorMessage)
{
    QList<EmployeeRow> out;
    if (!db.isValid() || !db.isOpen())
        return out;

    QSqlRecord rec = db.record("employe");
    if (rec.count() == 0) rec = db.record("EMPLOYE");
    auto hasCol = [&](const QString &c) { return rec.indexOf(c) != -1; };
    const bool hasId = hasCol("id") || hasCol("ID");
    QStringList selectCols = {"cin","prenom","nom","poste","service","date_embauche","statut_professionnel","numero_telephone","adresse_email","mot_de_passe","ville","certifications","niveau_competence","numero_license","date_expiration_certification","horaire_travail"};
    if (hasCol("rfid") || hasCol("RFID")) selectCols << "rfid";
    else selectCols << "NULL AS rfid";
    if (hasCol("code") || hasCol("CODE")) selectCols << "code";
    else selectCols << "NULL AS code";
    if (hasCol("photo") || hasCol("PHOTO")) selectCols << "photo";
    else selectCols << "NULL AS photo";
    {
        const QString colDe = pickRecordField(rec, {QStringLiteral("date_entree"), QStringLiteral("DATE_ENTREE")});
        const QString colDs = pickRecordField(rec, {QStringLiteral("date_sortie"), QStringLiteral("DATE_SORTIE")});
        if (!colDe.isEmpty())
            selectCols << colDe;
        else
            selectCols << QStringLiteral("NULL AS date_entree");
        if (!colDs.isEmpty())
            selectCols << colDs;
        else
            selectCols << QStringLiteral("NULL AS date_sortie");
    }
    if (hasId) selectCols.prepend("id");
    const QString baseSql = "SELECT " + selectCols.join(", ") + " FROM employe";

    QSqlQuery q(db);
    const QString term = searchTerm.trimmed();
    if (!term.isEmpty()) {
        const QString like = normalizedLike(term);

        QStringList textCols = {
            "cin","prenom","nom","poste","service","statut_professionnel","numero_telephone","adresse_email","ville",
            "certifications","niveau_competence","numero_license","horaire_travail"
        };
        if (hasCol("adresse"))
            textCols << "adresse";
        if (hasCol("rfid") || hasCol("RFID")) textCols << "rfid";
        if (hasCol("code") || hasCol("CODE")) textCols << "code";

        QStringList dateCols;
        if (hasCol("date_embauche")) dateCols << "date_embauche";
        if (hasCol("date_expiration_certification")) dateCols << "date_expiration_certification";

        auto tryExecSearch = [&](const QString &textExprFn, bool useUpper, bool useToCharDates) -> bool {
            QStringList parts;
            parts << "cin = ?";
            for (const QString &c : textCols) {
                if (c == "cin") continue;
                const QString colExpr = textExprFn.arg(c);
                if (useUpper)
                    parts << QString("UPPER(%1) LIKE UPPER(?)").arg(colExpr);
                else
                    parts << QString("%1 LIKE ?").arg(colExpr);
            }

            if (useToCharDates) {
                for (const QString &dc : dateCols)
                    parts << QString("TO_CHAR(%1,'DD/MM/YYYY') LIKE ?").arg(dc);
            }

            q.prepare(baseSql + " WHERE (" + parts.join(" OR ") + ")");
            q.addBindValue(term);
            for (const QString &c : textCols) {
                if (c == "cin") continue;
                q.addBindValue(like);
            }
            if (useToCharDates) {
                for (int i = 0; i < dateCols.size(); ++i)
                    q.addBindValue(like);
            }
            return q.exec();
        };

        if (!tryExecSearch("NVL(TO_CHAR(%1),'')", true, true)) {
            if (!tryExecSearch("COALESCE(%1,'')", true, true)) {
                if (!tryExecSearch("COALESCE(%1,'')", true, false)) {
                    if (!tryExecSearch("NVL(%1,'')", true, false)) {
                        if (!tryExecSearch("%1", false, false)) {
                            if (errorMessage) *errorMessage = q.lastError().text();
                            return out;
                        }
                    }
                }
            }
        }
    } else {
        if (!q.prepare(baseSql) || !q.exec()) {
            const QString baseSqlAlt = hasId
                ? "SELECT id, cin, prenom, nom, poste, service, date_embauche, statut_professionnel, numero_telephone, adresse_email, mot_de_passe, ville, certifications, niveau_competence, numero_license, date_expiration_certification, horaire_travail, NULL AS rfid, NULL AS code, NULL AS photo, NULL AS date_entree, NULL AS date_sortie FROM employe"
                : "SELECT cin, prenom, nom, poste, service, date_embauche, statut_professionnel, numero_telephone, adresse_email, mot_de_passe, ville, certifications, niveau_competence, numero_license, date_expiration_certification, horaire_travail, NULL AS rfid, NULL AS code, NULL AS photo, NULL AS date_entree, NULL AS date_sortie FROM employe";
            q.clear();
            if (!q.prepare(baseSqlAlt) || !q.exec()) {
                if (errorMessage) *errorMessage = q.lastError().text();
                return out;
            }
        }
    }

    const int idOffset = hasId ? 1 : 0;
    while (q.next()) {
        EmployeeRow e;
        e.id = hasId ? q.value(0).toInt() : -1;
        e.cin = q.value(0 + idOffset).toString();
        e.prenom = q.value(1 + idOffset).toString();
        e.nom = q.value(2 + idOffset).toString();
        e.occupation = q.value(3 + idOffset).toString();
        e.departement = q.value(4 + idOffset).toString();
        e.date_embauche = toDateAny(q.value(5 + idOffset));
        e.statut = q.value(6 + idOffset).toString();
        e.telephone = q.value(7 + idOffset).toString();
        e.email = q.value(8 + idOffset).toString();
        e.motdepasse = q.value(9 + idOffset).toString();
        e.ville = q.value(10 + idOffset).toString();
        e.certifications = q.value(11 + idOffset).toString();
        e.niveaux = q.value(12 + idOffset).toString();
        e.num_licence = q.value(13 + idOffset).toString();
        e.date_expiration = toDateAny(q.value(14 + idOffset));
        e.quart = q.value(15 + idOffset).toString();
        e.rfid = q.value(16 + idOffset).toString();
        e.emp_code = q.value(17 + idOffset).toString();
        e.photo_path = q.value(18 + idOffset).toString();
        e.port_date_entree = q.value(19 + idOffset).toDateTime();
        e.port_date_sortie = q.value(20 + idOffset).toDateTime();
        out.append(e);
    }

    return out;
}

// UI -> DB: query employees with optional WHERE (searchTerm) and ORDER BY (sortIndex).
// sortIndex: 0=ID ASC, 1=ID DESC, 2=Nom+Prenom ASC, 3=Nom+Prenom DESC
QList<EmployeeRow> EmployeeCrud::listWithSort(QSqlDatabase &db, const QString &searchTerm, int sortIndex, QString *errorMessage)
{
    QList<EmployeeRow> out;
    if (!db.isValid() || !db.isOpen())
        return out;

    QSqlRecord rec = db.record("employe");
    if (rec.count() == 0) rec = db.record("EMPLOYE");
    auto hasCol = [&](const QString &c) { return rec.indexOf(c) != -1; };
    const bool hasId = hasCol("id") || hasCol("ID");
    QStringList selectCols = {"cin","prenom","nom","poste","service","date_embauche","statut_professionnel","numero_telephone","adresse_email","mot_de_passe","ville","certifications","niveau_competence","numero_license","date_expiration_certification","horaire_travail"};
    if (hasCol("rfid") || hasCol("RFID")) selectCols << "rfid";
    else selectCols << "NULL AS rfid";
    if (hasCol("code") || hasCol("CODE")) selectCols << "code";
    else selectCols << "NULL AS code";
    if (hasCol("photo") || hasCol("PHOTO")) selectCols << "photo";
    else selectCols << "NULL AS photo";
    {
        const QString colDe = pickRecordField(rec, {QStringLiteral("date_entree"), QStringLiteral("DATE_ENTREE")});
        const QString colDs = pickRecordField(rec, {QStringLiteral("date_sortie"), QStringLiteral("DATE_SORTIE")});
        if (!colDe.isEmpty())
            selectCols << colDe;
        else
            selectCols << QStringLiteral("NULL AS date_entree");
        if (!colDs.isEmpty())
            selectCols << colDs;
        else
            selectCols << QStringLiteral("NULL AS date_sortie");
    }
    if (hasId) selectCols.prepend("id");
    QString baseSql = "SELECT " + selectCols.join(", ") + " FROM employe";

    // Build ORDER BY clause based on sortIndex.
    QString orderBySql;
    switch (sortIndex) {
    case 1: orderBySql = hasId ? " ORDER BY id DESC" : " ORDER BY cin DESC"; break;
    case 2: orderBySql = " ORDER BY nom ASC, prenom ASC"; break;
    case 3: orderBySql = " ORDER BY nom DESC, prenom DESC"; break;
    case 0:
    default: orderBySql = hasId ? " ORDER BY id ASC" : " ORDER BY cin ASC"; break;
    }

    QSqlQuery q(db);
    const QString term = searchTerm.trimmed();
    if (!term.isEmpty()) {
        const QString like = normalizedLike(term);

        QStringList textCols = {
            "cin","prenom","nom","poste","service","statut_professionnel","numero_telephone","adresse_email","ville",
            "certifications","niveau_competence","numero_license","horaire_travail"
        };
        if (hasCol("adresse"))
            textCols << "adresse";
        if (hasCol("rfid") || hasCol("RFID")) textCols << "rfid";
        if (hasCol("code") || hasCol("CODE")) textCols << "code";

        QStringList dateCols;
        if (hasCol("date_embauche")) dateCols << "date_embauche";
        if (hasCol("date_expiration_certification")) dateCols << "date_expiration_certification";

        auto tryExecSearch = [&](const QString &textExprFn, bool useUpper, bool useToCharDates) -> bool {
            QStringList parts;
            parts << "cin = ?";
            for (const QString &c : textCols) {
                if (c == "cin") continue;
                const QString colExpr = textExprFn.arg(c);
                if (useUpper)
                    parts << QString("UPPER(%1) LIKE UPPER(?)").arg(colExpr);
                else
                    parts << QString("%1 LIKE ?").arg(colExpr);
            }

            if (useToCharDates) {
                for (const QString &dc : dateCols)
                    parts << QString("TO_CHAR(%1,'DD/MM/YYYY') LIKE ?").arg(dc);
            }

            q.prepare(baseSql + " WHERE (" + parts.join(" OR ") + ")" + orderBySql);
            q.addBindValue(term);
            for (const QString &c : textCols) {
                if (c == "cin") continue;
                q.addBindValue(like);
            }
            if (useToCharDates) {
                for (int i = 0; i < dateCols.size(); ++i)
                    q.addBindValue(like);
            }
            return q.exec();
        };

        if (!tryExecSearch("NVL(TO_CHAR(%1),'')", true, true)) {
            if (!tryExecSearch("COALESCE(%1,'')", true, true)) {
                if (!tryExecSearch("COALESCE(%1,'')", true, false)) {
                    if (!tryExecSearch("NVL(%1,'')", true, false)) {
                        if (!tryExecSearch("%1", false, false)) {
                            if (errorMessage) *errorMessage = q.lastError().text();
                            return out;
                        }
                    }
                }
            }
        }
    } else {
        if (!q.prepare(baseSql + orderBySql) || !q.exec()) {
            const QString baseSqlAlt = hasId
                ? "SELECT id, cin, prenom, nom, poste, service, date_embauche, statut_professionnel, numero_telephone, adresse_email, mot_de_passe, ville, certifications, niveau_competence, numero_license, date_expiration_certification, horaire_travail, NULL AS rfid, NULL AS code, NULL AS photo, NULL AS date_entree, NULL AS date_sortie FROM employe"
                : "SELECT cin, prenom, nom, poste, service, date_embauche, statut_professionnel, numero_telephone, adresse_email, mot_de_passe, ville, certifications, niveau_competence, numero_license, date_expiration_certification, horaire_travail, NULL AS rfid, NULL AS code, NULL AS photo, NULL AS date_entree, NULL AS date_sortie FROM employe";
            q.clear();
            if (!q.prepare(baseSqlAlt + orderBySql) || !q.exec()) {
                if (errorMessage) *errorMessage = q.lastError().text();
                return out;
            }
        }
    }

    const int idOffset = hasId ? 1 : 0;
    while (q.next()) {
        EmployeeRow e;
        e.id = hasId ? q.value(0).toInt() : -1;
        e.cin = q.value(0 + idOffset).toString();
        e.prenom = q.value(1 + idOffset).toString();
        e.nom = q.value(2 + idOffset).toString();
        e.occupation = q.value(3 + idOffset).toString();
        e.departement = q.value(4 + idOffset).toString();
        e.date_embauche = toDateAny(q.value(5 + idOffset));
        e.statut = q.value(6 + idOffset).toString();
        e.telephone = q.value(7 + idOffset).toString();
        e.email = q.value(8 + idOffset).toString();
        e.motdepasse = q.value(9 + idOffset).toString();
        e.ville = q.value(10 + idOffset).toString();
        e.certifications = q.value(11 + idOffset).toString();
        e.niveaux = q.value(12 + idOffset).toString();
        e.num_licence = q.value(13 + idOffset).toString();
        e.date_expiration = toDateAny(q.value(14 + idOffset));
        e.quart = q.value(15 + idOffset).toString();
        e.rfid = q.value(16 + idOffset).toString();
        e.emp_code = q.value(17 + idOffset).toString();
        e.photo_path = q.value(18 + idOffset).toString();
        e.port_date_entree = q.value(19 + idOffset).toDateTime();
        e.port_date_sortie = q.value(20 + idOffset).toDateTime();
        out.append(e);
    }

    return out;
}

bool EmployeeCrud::getByCin(QSqlDatabase &db, const QString &cin, EmployeeRow *out, QString *errorMessage)
{
    if (!out) {
        if (errorMessage) *errorMessage = "Destination invalide.";
        return false;
    }
    *out = {};

    if (cin.trimmed().isEmpty()) {
        if (errorMessage) *errorMessage = "CIN invalide.";
        return false;
    }
    if (!db.isValid() || !db.isOpen()) {
        if (errorMessage) *errorMessage = "Connexion base de données indisponible.";
        return false;
    }

    QSqlRecord rec = db.record("employe");
    if (rec.count() == 0) rec = db.record("EMPLOYE");
    const bool hasId = rec.indexOf("id") != -1 || rec.indexOf("ID") != -1;
    QString selectList = hasId
        ? "id, cin, prenom, nom, poste, service, date_embauche, statut_professionnel, numero_telephone, adresse_email, mot_de_passe, ville, adresse, certifications, niveau_competence, numero_license, date_expiration_certification, horaire_travail, rfid, code, photo"
        : "cin, prenom, nom, poste, service, date_embauche, statut_professionnel, numero_telephone, adresse_email, mot_de_passe, ville, adresse, certifications, niveau_competence, numero_license, date_expiration_certification, horaire_travail, rfid, code, photo";
    {
        const QString colDe = pickRecordField(rec, {QStringLiteral("date_entree"), QStringLiteral("DATE_ENTREE")});
        const QString colDs = pickRecordField(rec, {QStringLiteral("date_sortie"), QStringLiteral("DATE_SORTIE")});
        if (!colDe.isEmpty())
            selectList += QStringLiteral(", ") + colDe;
        else
            selectList += QStringLiteral(", NULL AS date_entree");
        if (!colDs.isEmpty())
            selectList += QStringLiteral(", ") + colDs;
        else
            selectList += QStringLiteral(", NULL AS date_sortie");
    }
    QSqlQuery q(db);
    q.prepare("SELECT " + selectList + " FROM employe WHERE cin=?");
    q.addBindValue(cin.trimmed());
    if (!q.exec() || !q.next()) {
        if (errorMessage) *errorMessage = q.lastError().text();
        return false;
    }

    EmployeeRow e;
    const int off = hasId ? 1 : 0;
    e.id = hasId ? q.value(0).toInt() : -1;
    e.cin = q.value(0 + off).toString();
    e.prenom = q.value(1 + off).toString();
    e.nom = q.value(2 + off).toString();
    e.occupation = q.value(3 + off).toString();
    e.departement = q.value(4 + off).toString();
    e.date_embauche = toDateAny(q.value(5 + off));
    e.statut = q.value(6 + off).toString();
    e.telephone = q.value(7 + off).toString();
    e.email = q.value(8 + off).toString();
    e.motdepasse = q.value(9 + off).toString();
    e.ville = q.value(10 + off).toString();
    e.adresse = q.value(11 + off).toString();
    e.certifications = q.value(12 + off).toString();
    e.niveaux = q.value(13 + off).toString();
    e.num_licence = q.value(14 + off).toString();
    e.date_expiration = toDateAny(q.value(15 + off));
    e.quart = q.value(16 + off).toString();
    e.rfid = q.value(17 + off).toString();
    e.emp_code = q.value(18 + off).toString();
    e.photo_path = q.value(19 + off).toString();
    e.port_date_entree = q.value(20 + off).toDateTime();
    e.port_date_sortie = q.value(21 + off).toDateTime();
    *out = e;
    return true;
}

bool EmployeeCrud::getByRfid(QSqlDatabase &db, const QString &uidHex, EmployeeRow *out, QString *errorMessage)
{
    if (!out) {
        if (errorMessage)
            *errorMessage = QStringLiteral("Destination invalide.");
        return false;
    }
    *out = {};

    QString norm;
    for (const QChar &c : uidHex) {
        if (c.isLetterOrNumber())
            norm.append(c.toUpper());
    }
    if (norm.size() < 4)
        return false;
    if (!db.isValid() || !db.isOpen()) {
        if (errorMessage)
            *errorMessage = QStringLiteral("Connexion base de données indisponible.");
        return false;
    }

    QSqlRecord rec = db.record(QStringLiteral("employe"));
    QString tbl = QStringLiteral("employe");
    if (rec.count() == 0) {
        rec = db.record(QStringLiteral("EMPLOYE"));
        tbl = QStringLiteral("EMPLOYE");
    }
    const QString rfidCol = pickRecordField(rec, {QStringLiteral("rfid"), QStringLiteral("RFID")});
    const QString codeCol = pickRecordField(rec, {QStringLiteral("code"), QStringLiteral("CODE")});
    if (rfidCol.isEmpty() && codeCol.isEmpty())
        return false;
    const QString colHoraire = pickRecordField(rec, {QStringLiteral("horaire_travail"), QStringLiteral("HORAIRE_TRAVAIL")});
    const QString horaireExpr = colHoraire.isEmpty() ? QStringLiteral("NULL AS horaire_travail") : colHoraire;

    auto normSql = [](const QString &col) {
        return QStringLiteral("UPPER(REPLACE(REPLACE(REPLACE(%1,' ',''),'-',''),':',''))").arg(col);
    };
    QString whereClause;
    if (!rfidCol.isEmpty() && !codeCol.isEmpty())
        whereClause = QStringLiteral("(%1 = ? OR %2 = ?)").arg(normSql(rfidCol), normSql(codeCol));
    else if (!rfidCol.isEmpty())
        whereClause = normSql(rfidCol) + QStringLiteral(" = ?");
    else
        whereClause = normSql(codeCol) + QStringLiteral(" = ?");

    QSqlQuery q(db);
    const QString sql = QStringLiteral("SELECT cin, prenom, nom, poste, service, %1 FROM %2 WHERE %3%4")
                            .arg(horaireExpr, tbl, whereClause, sqlSelectOneRowSuffix(db));
    if (!q.prepare(sql)) {
        if (errorMessage)
            *errorMessage = q.lastError().text();
        return false;
    }
    if (!rfidCol.isEmpty() && !codeCol.isEmpty()) {
        q.addBindValue(norm);
        q.addBindValue(norm);
    } else {
        q.addBindValue(norm);
    }
    if (!q.exec()) {
        if (errorMessage)
            *errorMessage = q.lastError().text();
        return false;
    }
    if (!q.next())
        return false;

    out->cin = q.value(0).toString();
    out->prenom = q.value(1).toString();
    out->nom = q.value(2).toString();
    out->occupation = q.value(3).toString();
    out->departement = q.value(4).toString();
    out->quart = q.value(5).toString();
    return true;
}

static QString employeTableSqlName(const QSqlDatabase &db)
{
    QSqlRecord r = db.record(QStringLiteral("employe"));
    if (r.count() > 0)
        return QStringLiteral("employe");
    return QStringLiteral("EMPLOYE");
}

static bool updateEmployePortTimestamp(QSqlDatabase &db,
                                       const QString &cin,
                                       const QStringList &colCandidates,
                                       QString *errorMessage)
{
    if (cin.trimmed().isEmpty()) {
        if (errorMessage)
            *errorMessage = QStringLiteral("CIN invalide.");
        return false;
    }
    if (!db.isValid() || !db.isOpen()) {
        if (errorMessage)
            *errorMessage = QStringLiteral("Connexion base de données indisponible.");
        return false;
    }
    QSqlRecord rec = db.record(QStringLiteral("employe"));
    if (rec.count() == 0)
        rec = db.record(QStringLiteral("EMPLOYE"));
    const QString col = pickRecordField(rec, colCandidates);
    if (col.isEmpty()) {
        if (errorMessage)
            *errorMessage = QStringLiteral("Colonne Smart Port absente en base.");
        return false;
    }
    const QString tbl = employeTableSqlName(db);
    const QString cinCol = pickRecordField(rec, {QStringLiteral("cin"), QStringLiteral("CIN")});
    const QString cinKey = cinCol.isEmpty() ? QStringLiteral("cin") : cinCol;

    QSqlQuery q(db);
    const QString sql = QStringLiteral("UPDATE %1 SET %2 = ? WHERE %3 = ?").arg(tbl, col, cinKey);
    if (!q.prepare(sql)) {
        if (errorMessage)
            *errorMessage = q.lastError().text();
        return false;
    }
    q.addBindValue(QDateTime::currentDateTime());
    q.addBindValue(cin.trimmed());
    if (!q.exec()) {
        if (errorMessage)
            *errorMessage = q.lastError().text();
        return false;
    }
    db.commit();
    return true;
}

bool EmployeeCrud::recordSmartPortEntree(QSqlDatabase &db, const QString &cin, QString *errorMessage)
{
    return updateEmployePortTimestamp(db, cin,
                                      {QStringLiteral("date_entree"), QStringLiteral("DATE_ENTREE")},
                                      errorMessage);
}

bool EmployeeCrud::recordSmartPortSortie(QSqlDatabase &db, const QString &cin, QString *errorMessage)
{
    return updateEmployePortTimestamp(db, cin,
                                      {QStringLiteral("date_sortie"), QStringLiteral("DATE_SORTIE")},
                                      errorMessage);
}
