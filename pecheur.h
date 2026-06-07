#ifndef PECHEUR_H
#define PECHEUR_H

#include <QString>
#include <QList>
#include <QPair>
#include <QSqlQuery>
#include <QSqlDatabase>
#include <QSqlError>
#include <QDate>
#include "connection.h"

class Pecheur
{
public:
    Pecheur();
    
    // CRUD Operations (table PECHEUR: id_pecheur, nomprenom, role, contact, annees_experience, statut, date_limite, cin)
    bool ajouter(QString id, QString nom, QString prenom, QString role,
                  QString statut, int experience, QDate dateLimite, QString telephone,
                  QString rfid = QString(), QString cin = QString());
    
    bool supprimer(QString id);
    
    bool modifier(QString id, QString nom, QString prenom, QString role,
                  QString statut, int experience, QDate dateLimite, QString telephone,
                  QString rfid = QString(), QString cin = QString());
    
    QSqlQuery afficher();
    QSqlQuery afficherAvecFiltres(const QString &searchText, int sortIndex);
    QList<QPair<QString, int>> statistiquesParRole(const QString &searchText = QString());
    QList<QPair<QString, int>> statistiquesParStatut(const QString &searchText = QString());

    QString lastError() const { return m_lastError; }

private:
    QSqlDatabase db;
    QString m_lastError;
};

#endif // PECHEUR_H
