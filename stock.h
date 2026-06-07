#ifndef STOCK_H
#define STOCK_H

#include <QString>
#include <QSqlQuery>
#include <QSqlQueryModel>

class Stock
{
public:
    Stock();
    Stock(int stock_id, QString espece, int quantite, QString etat,
          int s_min, int s_max, QString date_ajout,
          int cin_vent, int n_cmd, QString date_vent);

    bool ajouter();
    static QSqlQueryModel *afficher();
    static QSqlQueryModel *afficherTrie(const QString &critere);
    static QSqlQueryModel *rechercherParEspece(const QString &texte, const QString &critere);
    bool supprimer(int id);
    bool modifier();
    static QString lastErrorMessage();
    bool exportStock_Data_Pdf(const QString &pdfFilePath,
                              const QString &whereAndSuffix,
                              const QString &orderBySuffix,
                              int *rowCountOut);

private:
    int stock_id, quantite, s_min, s_max, cin_vent, n_cmd;
    QString espece, etat, date_ajout, date_vent;
    static QString s_lastError;
};

#endif
