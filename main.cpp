#include "mainwindow.h"
#include "connection.h"
#include "employee.h"
#include <QApplication>
#include <QMessageBox>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    
    // Établir la connexion AVANT de créer MainWindow
    // (car le constructeur appelle afficherPecheurs() qui nécessite la connexion)
    bool connected = Connection::createInstance().createConnection();
    if (connected) {
        QString seedErr;
        if (!EmployeeCrud::ensureDefaultLoginUser(Connection::createInstance().getDatabase(), &seedErr)) {
            QMessageBox::warning(nullptr, QObject::tr("Compte de connexion"),
                QObject::tr("Connexion OK, mais impossible de garantir un compte employé par défaut:\n%1").arg(seedErr));
        }
        QMessageBox::information(nullptr, QObject::tr("Connexion"),
            QObject::tr("Connexion à la base de données réussie."));
    } else {
        QMessageBox::critical(nullptr, QObject::tr("Connexion"),
            QObject::tr("Échec de la connexion à la base de données."));
        return -1; // Quitter l'application si la connexion échoue
    }
    
    MainWindow w;
    w.show();
    return a.exec();
}
