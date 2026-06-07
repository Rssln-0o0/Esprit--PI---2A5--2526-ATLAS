#include "equipement.h"
#include <QDateTime>
#include <QDebug>
#include <QLocale>
#include <QMarginsF>
#include <QPageLayout>
#include <QPageSize>
#include <QPrinter>
#include <QTextDocument>
#include <QVariant>

Equipement::Equipement()
{
    db = QSqlDatabase::database(Connection::connectionName());
    m_lastError.clear();
}

bool Equipement::ajouter(int idEquip, const QString &nom, const QString &type, const QString &etat,
                         const QDate &dateAcquisition, const QString &localisation, double coutEstime,
                         const QString &fournisseur, const QString &disponibilite, int idBateau)
{
    QSqlQuery query(db);

    if (idEquip <= 0) {
        if (!query.exec("SELECT SEQ_EQUIPEMENT_ID.NEXTVAL FROM DUAL") || !query.next()) {
            m_lastError = QString("Impossible d'obtenir un ID (séquence SEQ_EQUIPEMENT_ID).");
            return false;
        }
        idEquip = query.value(0).toInt();
    }

    QString idBateauVal = idBateau > 0 ? ":id_bateau" : "NULL";
    bool hasDate = dateAcquisition.isValid();
    QString dateVal = hasDate ? "TO_DATE(:date_acq,'YYYY-MM-DD')" : "NULL";
    query.prepare(
        QString("INSERT INTO EQUIPEMENT "
                "(ID_EQUIPEMENT, NOM, TYPE, ETAT, DATE_ACQUISITION, LOCALISATION, "
                "COUT_ESTIME, FOURNISSEUR, DISPONIBILITE, ID_BATEAU) "
                "VALUES (:id, :nom, :type, :etat, %1, :localisation, "
                ":cout, :fournisseur, :disponibilite, %2)").arg(dateVal, idBateauVal));
    query.bindValue(":id", QString::number(idEquip));
    query.bindValue(":nom", QVariant::fromValue(nom.trimmed()));
    query.bindValue(":type", QVariant::fromValue(type.trimmed()));
    query.bindValue(":etat", QVariant::fromValue(etat.trimmed()));
    if (hasDate) query.bindValue(":date_acq", QVariant::fromValue(dateAcquisition.toString(QStringLiteral("yyyy-MM-dd"))));
    query.bindValue(":localisation", QVariant::fromValue(localisation.trimmed()));
    query.bindValue(":cout", QString::number(coutEstime));
    query.bindValue(":fournisseur", QVariant::fromValue(fournisseur.trimmed()));
    query.bindValue(":disponibilite", QVariant::fromValue(disponibilite.trimmed()));
    if (idBateau > 0) query.bindValue(":id_bateau", QString::number(idBateau));

    if (query.exec()) {
        m_lastError.clear();
        return true;
    }
    m_lastError = query.lastError().text();
    qDebug() << "Equipement::ajouter erreur:" << m_lastError;
    return false;
}

bool Equipement::supprimer(int idEquip)
{
    QSqlQuery query(db);
    query.prepare("DELETE FROM EQUIPEMENT WHERE ID_EQUIPEMENT = :id");
    query.bindValue(":id", QString::number(idEquip));
    if (query.exec()) {
        m_lastError.clear();
        return true;
    }
    m_lastError = query.lastError().text();
    qDebug() << "Equipement::supprimer erreur:" << m_lastError;
    return false;
}

bool Equipement::modifier(int idEquip, const QString &nom, const QString &type, const QString &etat,
                         const QDate &dateAcquisition, const QString &localisation, double coutEstime,
                         const QString &fournisseur, const QString &disponibilite, int idBateau)
{
    QSqlQuery query(db);
    QString idBateauVal = idBateau > 0 ? ":id_bateau" : "NULL";
    bool hasDate = dateAcquisition.isValid();
    QString dateVal = hasDate ? "TO_DATE(:date_acq,'YYYY-MM-DD')" : "NULL";
    query.prepare(
        QString("UPDATE EQUIPEMENT SET NOM = :nom, TYPE = :type, ETAT = :etat, "
                "DATE_ACQUISITION = %1, LOCALISATION = :localisation, "
                "COUT_ESTIME = :cout, FOURNISSEUR = :fournisseur, DISPONIBILITE = :disponibilite, "
                "ID_BATEAU = %2 WHERE ID_EQUIPEMENT = :id").arg(dateVal, idBateauVal));
    query.bindValue(":id", QString::number(idEquip));
    query.bindValue(":nom", QVariant::fromValue(nom.trimmed()));
    query.bindValue(":type", QVariant::fromValue(type.trimmed()));
    query.bindValue(":etat", QVariant::fromValue(etat.trimmed()));
    if (hasDate) query.bindValue(":date_acq", QVariant::fromValue(dateAcquisition.toString(QStringLiteral("yyyy-MM-dd"))));
    query.bindValue(":localisation", QVariant::fromValue(localisation.trimmed()));
    query.bindValue(":cout", QString::number(coutEstime));
    query.bindValue(":fournisseur", QVariant::fromValue(fournisseur.trimmed()));
    query.bindValue(":disponibilite", QVariant::fromValue(disponibilite.trimmed()));
    if (idBateau > 0) query.bindValue(":id_bateau", QString::number(idBateau));

    if (query.exec()) {
        m_lastError.clear();
        return true;
    }
    m_lastError = query.lastError().text();
    qDebug() << "Equipement::modifier erreur:" << m_lastError;
    return false;
}

QSqlQuery Equipement::afficher(const QString &orderBy)
{
    QSqlQuery query(db);
    QString sql = "SELECT ID_EQUIPEMENT, NOM, TYPE, ETAT, DATE_ACQUISITION, LOCALISATION, "
                  "COUT_ESTIME, FOURNISSEUR, DISPONIBILITE, ID_BATEAU FROM EQUIPEMENT";
    
    // Security: Only allow valid sort columns
    QStringList validCols = {"ID_EQUIPEMENT", "NOM", "TYPE", "ETAT", "COUT_ESTIME", "DATE_ACQUISITION"};
    if (validCols.contains(orderBy)) {
        sql += " ORDER BY " + orderBy;
    } else {
        sql += " ORDER BY ID_EQUIPEMENT";
    }

    query.prepare(sql);
    if (!query.exec())
        qDebug() << "Equipement::afficher erreur:" << query.lastError().text();
    return query;
}

QSqlQuery Equipement::rechercher(const QString &motCle, const QString &orderBy)
{
    QSqlQuery query(db);
    QString mc = "%" + motCle.trimmed().toLower() + "%";
    QString sql = "SELECT ID_EQUIPEMENT, NOM, TYPE, ETAT, DATE_ACQUISITION, LOCALISATION, "
                  "COUT_ESTIME, FOURNISSEUR, DISPONIBILITE, ID_BATEAU FROM EQUIPEMENT "
                  "WHERE LOWER(NOM) LIKE :mc OR LOWER(TYPE) LIKE :mc OR LOWER(FOURNISSEUR) LIKE :mc "
                  "OR LOWER(LOCALISATION) LIKE :mc";
    
    QStringList validCols = {"ID_EQUIPEMENT", "NOM", "TYPE", "ETAT", "COUT_ESTIME", "DATE_ACQUISITION"};
    if (validCols.contains(orderBy)) {
        sql += " ORDER BY " + orderBy;
    } else {
        sql += " ORDER BY ID_EQUIPEMENT";
    }

    query.prepare(sql);
    query.bindValue(":mc", mc);
    if (!query.exec())
        qDebug() << "Equipement::rechercher erreur:" << query.lastError().text();
    return query;
}

bool Equipement::exportListePdf(const QString &pdfFilePath,
                                const QString &searchMotCle,
                                const QString &orderBy,
                                int *rowCountOut)
{
    m_lastError.clear();
    if (rowCountOut)
        *rowCountOut = 0;
    if (pdfFilePath.trimmed().isEmpty()) {
        m_lastError = QStringLiteral("Chemin PDF vide.");
        return false;
    }
    if (!db.isOpen()) {
        m_lastError = QStringLiteral("Base fermée.");
        return false;
    }

    QSqlQuery q = searchMotCle.trimmed().isEmpty() ? afficher(orderBy) : rechercher(searchMotCle, orderBy);
    if (!q.isActive()) {
        m_lastError = q.lastError().text();
        if (m_lastError.isEmpty())
            m_lastError = QStringLiteral("Échec exécution requête EQUIPEMENT.");
        return false;
    }

    const QStringList headers = {
        QStringLiteral("ID"),
        QStringLiteral("Nom"),
        QStringLiteral("Type"),
        QStringLiteral("État"),
        QStringLiteral("Date acq."),
        QStringLiteral("Localisation"),
        QStringLiteral("Coût"),
        QStringLiteral("Fournisseur"),
        QStringLiteral("Disponibilité"),
    };

    QString html;
    html += QStringLiteral("<h1 style='text-align:center;color:#2c3e50;'>%1</h1>")
                .arg(QStringLiteral("Liste des équipements — SmartPort").toHtmlEscaped());
    html += QStringLiteral("<p style='text-align:center;color:#7f8c8d;font-size:10pt;'>%1</p>")
                .arg(QStringLiteral("Données issues de la table EQUIPEMENT. %1")
                         .arg(QLocale::system().toString(QDateTime::currentDateTime(), QLocale::ShortFormat))
                         .toHtmlEscaped());

    html += QStringLiteral(
        "<table border='1' cellpadding='8' cellspacing='0' "
        "style='width:100%;border-collapse:collapse;font-size:9pt;'>");
    html += QStringLiteral("<tr style='background:#3498db;color:white;'>");
    for (const QString &h : headers)
        html += QStringLiteral("<th>%1</th>").arg(h.toHtmlEscaped());
    html += QStringLiteral("</tr>");

    int rowCount = 0;
    while (q.next()) {
        ++rowCount;
        const QDate d = q.value(4).toDate();
        const QString dateStr = d.isValid() ? d.toString(QStringLiteral("dd/MM/yyyy")) : QString();
        html += QStringLiteral("<tr>");
        html += QStringLiteral("<td>%1</td>").arg(q.value(0).toString().toHtmlEscaped());
        html += QStringLiteral("<td>%1</td>").arg(q.value(1).toString().trimmed().toHtmlEscaped());
        html += QStringLiteral("<td>%1</td>").arg(q.value(2).toString().trimmed().toHtmlEscaped());
        html += QStringLiteral("<td>%1</td>").arg(q.value(3).toString().trimmed().toHtmlEscaped());
        html += QStringLiteral("<td>%1</td>").arg(dateStr.toHtmlEscaped());
        html += QStringLiteral("<td>%1</td>").arg(q.value(5).toString().trimmed().toHtmlEscaped());
        html += QStringLiteral("<td>%1</td>")
                    .arg(QString::number(q.value(6).toDouble(), 'f', 2).toHtmlEscaped());
        html += QStringLiteral("<td>%1</td>").arg(q.value(7).toString().trimmed().toHtmlEscaped());
        html += QStringLiteral("<td>%1</td>").arg(q.value(8).toString().trimmed().toHtmlEscaped());
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
    m_lastError.clear();
    return true;
}

QMap<QString, int> Equipement::getCountByType()
{
    QMap<QString, int> stats;
    QSqlQuery query(db);
    if (query.exec("SELECT TYPE, COUNT(*) FROM EQUIPEMENT GROUP BY TYPE")) {
        while (query.next()) {
            stats.insert(query.value(0).toString(), query.value(1).toInt());
        }
    } else {
        qDebug() << "Equipement::getCountByType erreur:" << query.lastError().text();
    }
    return stats;
}

QMap<QString, int> Equipement::getCountByStatus()
{
    QMap<QString, int> stats;
    QSqlQuery query(db);
    // On groupe par ETAT
    if (query.exec("SELECT ETAT, COUNT(*) FROM EQUIPEMENT GROUP BY ETAT")) {
        while (query.next()) {
            stats.insert(query.value(0).toString(), query.value(1).toInt());
        }
    } else {
        qDebug() << "Equipement::getCountByStatus erreur:" << query.lastError().text();
    }
    return stats;
}

double Equipement::getTotalValue()
{
    QSqlQuery query(db);
    if (query.exec("SELECT SUM(COUT_ESTIME) FROM EQUIPEMENT") && query.next()) {
        return query.value(0).toDouble();
    }
    return 0.0;
}

int Equipement::getTotalCount()
{
    QSqlQuery query(db);
    if (query.exec("SELECT COUNT(*) FROM EQUIPEMENT") && query.next()) {
        return query.value(0).toInt();
    }
    return 0;
}

QSqlQuery Equipement::getMaintenanceAlerts()
{
    QSqlQuery query(db);
    // Smart Alert: Older than 12 months OR State is 'Mauvais' or 'En Maintenance'
    query.prepare("SELECT ID_EQUIPEMENT, NOM, TYPE, ETAT, DATE_ACQUISITION, LOCALISATION "
                  "FROM EQUIPEMENT "
                  "WHERE LOWER(ETAT) LIKE '%mauvais%' "
                  "OR LOWER(ETAT) LIKE '%maintenance%' "
                  "OR DATE_ACQUISITION < ADD_MONTHS(SYSDATE, -12) "
                  "ORDER BY DATE_ACQUISITION ASC");
    if (!query.exec())
        qDebug() << "Equipement::getMaintenanceAlerts erreur:" << query.lastError().text();
    return query;
}

bool Equipement::updateStatus(int idEquip, const QString &newStatus)
{
    QSqlQuery query(db);
    query.prepare("UPDATE EQUIPEMENT SET ETAT = :etat WHERE ID_EQUIPEMENT = :id");
    query.bindValue(":etat", newStatus);
    query.bindValue(":id", idEquip);
    
    if (query.exec()) return true;
    m_lastError = query.lastError().text();
    return false;
}

int Equipement::getAlertCount()
{
    QSqlQuery query(db);
    if (query.exec("SELECT COUNT(*) FROM EQUIPEMENT WHERE LOWER(ETAT) LIKE '%mauvais%' OR DATE_ACQUISITION < ADD_MONTHS(SYSDATE, -12)")) {
        if (query.next()) return query.value(0).toInt();
    }
    return 0;
}
