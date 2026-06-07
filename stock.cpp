#include "stock.h"
#include "connection.h"
#include <QDate>
#include <QDateTime>
#include <QDebug>
#include <QLocale>
#include <QMetaType>
#include <QPageLayout>
#include <QPageSize>
#include <QPrinter>
#include <QSqlDatabase>
#include <QSqlError>
#include <QTextDocument>
#include <QVariant>

QString Stock::s_lastError;

namespace
{
    QSqlDatabase stockDatabase()
    {
        QSqlDatabase db = QSqlDatabase::database(Connection::connectionName());
        if (db.isValid() && db.isOpen())
            return db;

        QSqlDatabase fallback = QSqlDatabase::database();
        return fallback;
    }

    QDate parseStockDate(const QString &value)
    {
        const QString trimmed = value.trimmed();
        const QStringList dateFormats = {"yyyy-MM-dd", "dd/MM/yyyy", "dd/MM/yy"};
        const QStringList dateTimeFormats = {
            "yyyy-MM-dd hh:mm:ss",
            "yyyy-MM-dd HH:mm:ss",
            "dd/MM/yyyy hh:mm:ss",
            "dd/MM/yyyy HH:mm:ss",
            "dd/MM/yy hh:mm:ss",
            "dd/MM/yy HH:mm:ss",
            "yyyy-MM-ddTHH:mm:ss",
            "yyyy-MM-ddTHH:mm:ss.zzz"};

        if (trimmed.isEmpty())
            return QDate();

        for (const QString &format : dateFormats)
        {
            const QDate parsed = QDate::fromString(trimmed, format);
            if (parsed.isValid())
                return parsed;
        }

        const QDateTime isoDateTime = QDateTime::fromString(trimmed, Qt::ISODate);
        if (isoDateTime.isValid())
            return isoDateTime.date();

        for (const QString &format : dateTimeFormats)
        {
            const QDateTime parsed = QDateTime::fromString(trimmed, format);
            if (parsed.isValid())
                return parsed.date();
        }

        const int splitIndex = qMax(trimmed.indexOf(' '), trimmed.indexOf('T'));
        if (splitIndex > 0)
        {
            const QString datePart = trimmed.left(splitIndex).trimmed();
            for (const QString &format : dateFormats)
            {
                const QDate parsed = QDate::fromString(datePart, format);
                if (parsed.isValid())
                    return parsed;
            }
        }

        return QDate();
    }

    QString stockSelectColumns()
    {
        return QStringLiteral(
            "STOCK_ID, ESPECE, QUANTITE, ETAT, S_MIN, S_MAX, "
            "TO_CHAR(DATE_AJOUT, 'YYYY-MM-DD') AS DATE_AJOUT, "
            "CIN_VENT, N_CMD, TO_CHAR(DATE_VENT, 'YYYY-MM-DD') AS DATE_VENT");
    }
}

Stock::Stock() {}

Stock::Stock(int stock_id, QString espece, int quantite, QString etat,
             int s_min, int s_max, QString date_ajout,
             int cin_vent, int n_cmd, QString date_vent)
{
    this->stock_id = stock_id;
    this->espece = espece;
    this->quantite = quantite;
    this->etat = etat;
    this->s_min = s_min;
    this->s_max = s_max;
    this->date_ajout = date_ajout;
    this->cin_vent = cin_vent;
    this->n_cmd = n_cmd;
    this->date_vent = date_vent;
}

bool Stock::ajouter()
{
    QSqlDatabase db = stockDatabase();
    QSqlQuery query(db);

    const QDate ajout = parseStockDate(date_ajout);
    const QDate vente = parseStockDate(date_vent);
    if (!ajout.isValid() || !vente.isValid())
    {
        qWarning() << "Stock::ajouter invalid date format" << date_ajout << date_vent;
        return false;
    }

    query.prepare("INSERT INTO STOCK "
                  "(STOCK_ID, ESPECE, QUANTITE, ETAT, S_MIN, S_MAX, DATE_AJOUT, CIN_VENT, N_CMD, DATE_VENT) "
                  "VALUES (:stock_id, :espece, :quantite, :etat, :s_min, :s_max, "
                  "TO_DATE(:date_ajout, 'YYYY-MM-DD'), :cin_vent, :n_cmd, TO_DATE(:date_vent, 'YYYY-MM-DD'))");

    query.bindValue(":stock_id", stock_id);
    query.bindValue(":espece", espece);
    query.bindValue(":quantite", quantite);
    query.bindValue(":etat", etat);
    query.bindValue(":s_min", s_min);
    query.bindValue(":s_max", s_max);
    query.bindValue(":date_ajout", ajout.toString("yyyy-MM-dd"));
    query.bindValue(":cin_vent", QString::number(cin_vent));
    query.bindValue(":n_cmd", QString::number(n_cmd));
    query.bindValue(":date_vent", vente.toString("yyyy-MM-dd"));
    
    const bool ok = query.exec();
    if (!ok)
        qWarning() << "Stock::ajouter failed:" << query.lastError().text();

    return ok;
}

QSqlQueryModel *Stock::afficher()
{
    QSqlQueryModel *model = new QSqlQueryModel();
    QSqlDatabase db = stockDatabase();

    model->setQuery("SELECT " + stockSelectColumns() + " FROM STOCK", db);
    if (model->lastError().isValid())
        qWarning() << "Stock::afficher failed:" << model->lastError().text();

    return model;
}

namespace
{
    QString stockOrderBy(const QString &critere)
    {
        if (critere == "État" || critere == "Etat" || critere.toLower() == "etat")
        {
            return "CASE "
                   "WHEN UPPER(ETAT) = 'VENDUE' THEN 1 "
                   "WHEN UPPER(ETAT) = 'FRAIS' THEN 2 "
                   "WHEN UPPER(ETAT) IN ('STOCKE', 'STOCKÉ') THEN 3 "
                   "ELSE 4 END";
        }

        if (critere == "Quantité" || critere == "Quantite" || critere.toLower() == "quantite")
        {
            return "QUANTITE";
        }

        return "ESPECE";
    }
}

QSqlQueryModel *Stock::afficherTrie(const QString &critere)
{
    QString colonne = stockOrderBy(critere);

    QSqlQueryModel *model = new QSqlQueryModel();
    QSqlDatabase db = stockDatabase();
    model->setQuery("SELECT " + stockSelectColumns() + " FROM STOCK ORDER BY " + colonne, db);
    if (model->lastError().isValid())
        qWarning() << "Stock::afficherTrie failed:" << model->lastError().text();

    return model;
}

QSqlQueryModel *Stock::rechercherParEspece(const QString &texte, const QString &critere)
{
    QString colonne = stockOrderBy(critere);
    QString texteRecherche = "%" + texte + "%";

    QSqlDatabase db = stockDatabase();
    QSqlQuery query(db);
    query.prepare("SELECT " + stockSelectColumns() + " FROM STOCK "
                                                     "WHERE UPPER(ESPECE) LIKE UPPER(:texte_espece) "
                                                     "OR TO_CHAR(CIN_VENT) LIKE :texte_cin_vent "
                                                     "ORDER BY " +
                  colonne);
    query.bindValue(":texte_espece", texteRecherche);
    query.bindValue(":texte_cin_vent", texteRecherche);
    if (!query.exec())
        qWarning() << "Stock::rechercherParEspece failed:" << query.lastError().text();

    QSqlQueryModel *model = new QSqlQueryModel();
    model->setQuery(query);
    if (model->lastError().isValid())
        qWarning() << "Stock::rechercherParEspece model failed:" << model->lastError().text();

    return model;
}

bool Stock::supprimer(int id)
{
    QSqlDatabase db = stockDatabase();
    QSqlQuery query(db);

    query.prepare("DELETE FROM STOCK WHERE STOCK_ID = :id");
    query.bindValue(":id", id);

    const bool ok = query.exec();
    if (!ok)
        qWarning() << "Stock::supprimer failed:" << query.lastError().text();

    return ok;
}

bool Stock::modifier()
{
    QSqlDatabase db = stockDatabase();
    QSqlQuery query(db);

    const QDate ajout = parseStockDate(date_ajout);
    const QDate vente = parseStockDate(date_vent);
    if (!ajout.isValid() || !vente.isValid())
    {
        qWarning() << "Stock::modifier invalid date format" << date_ajout << date_vent;
        return false;
    }

    query.prepare("UPDATE STOCK SET "
                  "ESPECE = :espece, "
                  "QUANTITE = :quantite, "
                  "ETAT = :etat, "
                  "S_MIN = :s_min, "
                  "S_MAX = :s_max, "
                  "DATE_AJOUT = TO_DATE(:date_ajout, 'YYYY-MM-DD'), "
                  "CIN_VENT = :cin_vent, "
                  "N_CMD = :n_cmd, "
                  "DATE_VENT = TO_DATE(:date_vent, 'YYYY-MM-DD') "
                  "WHERE STOCK_ID = :stock_id");

    query.bindValue(":espece", espece);
    query.bindValue(":quantite", quantite);
    query.bindValue(":etat", etat);
    query.bindValue(":s_min", s_min);
    query.bindValue(":s_max", s_max);
    query.bindValue(":date_ajout", ajout.toString("yyyy-MM-dd"));
    query.bindValue(":cin_vent", QString::number(cin_vent));
    query.bindValue(":n_cmd", QString::number(n_cmd));
    query.bindValue(":date_vent", vente.toString("yyyy-MM-dd"));
    query.bindValue(":stock_id", stock_id);

    const bool ok = query.exec();
    if (!ok)
        qWarning() << "Stock::modifier failed:" << query.lastError().text();

    return ok;
}

QString Stock::lastErrorMessage()
{
    return s_lastError;
}

bool Stock::exportStock_Data_Pdf(const QString &pdfFilePath,
                                 const QString &whereAndSuffix,
                                 const QString &orderBySuffix,
                                 int *rowCountOut)
{
    s_lastError.clear();
    if (rowCountOut)
        *rowCountOut = 0;
    if (pdfFilePath.trimmed().isEmpty())
    {
        s_lastError = QStringLiteral("Chemin PDF vide.");
        return false;
    }

    QSqlDatabase db = QSqlDatabase::database(Connection::connectionName());
    if (!db.isOpen())
    {
        s_lastError = QStringLiteral("Base fermée.");
        return false;
    }

    const QString sql = QStringLiteral("SELECT %1 FROM STOCK WHERE 1=1")
                            .arg(stockSelectColumns()) +
                        whereAndSuffix + orderBySuffix;

    QSqlQuery q(db);
    if (!q.exec(sql))
    {
        s_lastError = q.lastError().text();
        return false;
    }

    constexpr int kStockExportCols = 10;
    const QStringList headers = {
        QStringLiteral("ID"),
        QStringLiteral("ESPÈCE"),
        QStringLiteral("Quantité"),
        QStringLiteral("État"),
        QStringLiteral("Seuil minimum"),
        QStringLiteral("Seuil maximum"),
        QStringLiteral("Date d'ajout"),
        QStringLiteral("CIN du vendeur"),
        QStringLiteral("Numéro de commande"),
        QStringLiteral("Date de vente"),
    };

    auto cellText = [](const QVariant &v) -> QString
    {
        if (v.isNull())
            return QString();
        if (v.typeId() == QMetaType::Double)
            return QString::number(v.toDouble(), 'f', 2);
        return v.toString();
    };

    QString html;
    html += QStringLiteral("<h2 style='text-align:center;color:#2c3e50;'>%1</h2>")
                .arg(QStringLiteral("Liste des stocks").toHtmlEscaped());
    html += QStringLiteral("<p style='text-align:center;color:#7f8c8d;font-size:10pt;'>%1</p>")
                .arg(QStringLiteral("Données issues de la table STOCK %1")
                         .arg(QLocale::system().toString(QDateTime::currentDateTime(), QLocale::ShortFormat))
                         .toHtmlEscaped());

    html += QStringLiteral(
        "<table border='1' cellpadding='6' cellspacing='0' "
        "style='width:100%;border-collapse:collapse;font-size:9pt;'>");

    html += QStringLiteral("<tr style='background-color:#3498db;color:white;'>");
    for (const QString &h : headers)
        html += QStringLiteral("<th>%1</th>").arg(h.toHtmlEscaped());
    html += QStringLiteral("</tr>");

    int rowCount = 0;
    while (q.next())
    {
        ++rowCount;
        html += QStringLiteral("<tr>");
        for (int c = 0; c < kStockExportCols; ++c)
            html += QStringLiteral("<td>%1</td>").arg(cellText(q.value(c)).toHtmlEscaped());
        html += QStringLiteral("</tr>");
    }
    html += QStringLiteral("</table>");

    QPrinter printer(QPrinter::HighResolution);
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setOutputFileName(pdfFilePath);
    printer.setPageSize(QPageSize(QPageSize::A4));
    printer.setPageOrientation(QPageLayout::Landscape);
    printer.setPageMargins(QMarginsF(9, 9, 10, 10), QPageLayout::Millimeter);
    printer.setResolution(150);

    QTextDocument doc;
    doc.setHtml(html);
    doc.print(&printer);

    if (rowCountOut)
        *rowCountOut = rowCount;
    return true;
}
