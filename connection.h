#ifndef CONNECTION_H
#define CONNECTION_H

#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>

/**
 * Implémentation du pattern Singleton pour gérer une connexion unique
 * à la base de données.
 */
class Connection
{
public:
    static Connection& createInstance();
    bool createConnection();

    /** Nom de la connexion (pour QSqlDatabase::database(name)) */
    static const char* connectionName();

    /** Retourne la base de données pour exécuter des requêtes (INSERT, SELECT, etc.) */
    QSqlDatabase& getDatabase() { return db; }

    Connection(const Connection&) = delete;
    Connection& operator=(const Connection&) = delete;

private:
    Connection();
    ~Connection();

    QSqlDatabase db;
};

#endif // CONNECTION_H
