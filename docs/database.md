# Documentation base de donnees

## Objectif

Cette section documente la structure de base de donnees utilisee par l'application ATLAS Smart Port. Le projet utilise Oracle avec une connexion ODBC depuis l'application Qt/C++.

## Fichiers SQL disponibles

| Fichier | Role |
| --- | --- |
| `Source_Projet2A.sql` | Script principal contenant la structure Oracle et des donnees de demonstration. |
| `database_full_27_02_2026.sql` | Export complet anterieur de la base. |
| `oracle_alter_bateau_localisation.sql` | Script de migration pour la localisation des bateaux. |
| `oracle_alter_pecheur_rfid.sql` | Script de migration pour l'association RFID des pecheurs. |

## Utilisation recommandee

Pour initialiser une base locale de demonstration :

1. creer un utilisateur ou schema Oracle dedie au projet ;
2. ouvrir SQL Developer ou un client Oracle equivalent ;
3. se connecter avec l'utilisateur proprietaire du schema ;
4. executer le script principal `Source_Projet2A.sql` ;
5. executer les scripts `oracle_alter_*.sql` si la version du schema le necessite ;
6. configurer une source ODBC pointant vers cette base ;
7. renseigner le DSN, l'utilisateur et le mot de passe dans la configuration locale.

## Configuration ODBC

L'application Qt utilise le driver SQL ODBC. La configuration attendue est documentee dans `.env.example`.

Variables de reference :
```text
DB_DSN=Source_Projet2A
DB_USER=your_oracle_user
DB_PASSWORD=your_oracle_password
```
