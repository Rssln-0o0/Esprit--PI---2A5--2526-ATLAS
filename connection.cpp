#include "connection.h"
#include <QDebug>

Connection::Connection()
{
    // db sera configuré dans createConnection()
}

Connection::~Connection()
{
    if (db.isOpen()) {
        db.close();
    }
}

Connection& Connection::createInstance()
{
    static Connection instance; // Vivante pendant toute la durée du programme (thread-safe en C++11)
    return instance;
}

static const char* CONNECTION_NAME = "oracle_conn";

const char* Connection::connectionName()
{
    return CONNECTION_NAME;
}

bool Connection::createConnection()
{
    if (!QSqlDatabase::contains(CONNECTION_NAME)) {
        db = QSqlDatabase::addDatabase("QODBC", CONNECTION_NAME);
    } else {
        db = QSqlDatabase::database(CONNECTION_NAME);
    }

    // Replace with your ODBC DSN / project name, username and password
    db.setDatabaseName("Source_Projet2A");
    db.setUserName("projet");
    db.setPassword("projet");
    // ODBC 3.x pour Oracle : gestion correcte des transactions (commit/rollback)
    db.setConnectOptions("SQL_ATTR_ODBC_VERSION=SQL_OV_ODBC3");

    if (db.open()) {
        qDebug() << "Connexion établie";
        return true;
    } else {
        qDebug() << "Échec de la connexion:" << db.lastError().text();
        return false;
    }
}
