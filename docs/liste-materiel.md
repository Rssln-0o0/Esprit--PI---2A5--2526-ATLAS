# Liste du materiel

## Objectif

Ce document recense le materiel necessaire pour reproduire la partie embarquee et IoT du projet ATLAS Smart Port. Les references exactes peuvent etre adaptees selon le montage final, mais chaque composant utilise lors de la demonstration doit etre identifiable.

## Cartes et modules principaux

| Element | Reference recommandee | Role dans le projet |
| --- | --- | --- |
| Arduino Uno ou Nano | Arduino Uno R3 ou equivalent compatible | Controle des capteurs, actionneurs et communication serie. |
| ESP32 | ESP32 DevKit ou equivalent | Communication GPS, WiFi ou extensions embarquees. |
| ESP32-CAM | AI Thinker ESP32-CAM ou equivalent | Capture d'image et transmission serie vers l'application Qt. |
| Module RFID | RC522 | Lecture des badges RFID. |
| Module GPS | NEO-6M ou equivalent | Recuperation des coordonnees GPS. |

## Capteurs

| Capteur | Role |
| --- | --- |
| Capteur de gaz | Detection d'un niveau de gaz anormal. |
| Capteur de flamme | Detection d'un risque d'incendie. |
| Capteur d'humidite ou pluie | Detection d'humidite ou de presence d'eau. |
| Capteur PIR | Detection de mouvement en mode securite. |
| Interrupteur magnetique | Detection d'ouverture ou de fermeture. |
| Capteur ultrason HC-SR04 | Mesure de distance ou niveau d'eau. |

## Actionneurs

| Actionneur | Role |
| --- | --- |
| Ventilateur | Reaction automatique en cas de detection de gaz. |
| Pompe a eau | Reaction automatique en cas de detection de flamme ou besoin de pompage. |
| Buzzer | Signal sonore d'alerte. |
| LEDs d'etat | Signalisation visuelle des capteurs et alertes. |
| Servomoteurs | Controle mecanique pour les scenarios de porte RFID. |
| Bouton tactile | Commande locale de l'eclairage ou d'un mode. |

## Connexion et alimentation

| Element | Utilisation |
| --- | --- |
| Cables USB | Programmation et communication serie entre le PC et les cartes. |
| Breadboard | Prototypage du circuit. |
| Jumpers male-male et male-femelle | Connexion des capteurs et actionneurs. |
| Resistance adaptee | Protection des LEDs et adaptation des signaux si necessaire. |
| Alimentation externe | Alimentation stable pour les actionneurs si le courant USB est insuffisant. |

## Informations a completer avant soumission

Pour une documentation totalement reproductible, ajouter :

- les references exactes achetees ou utilisees ;
- les tensions d'alimentation ;
- les ports utilises pendant la demonstration ;
- les bibliotheques Arduino necessaires ;
- les photos du montage final si disponibles.
