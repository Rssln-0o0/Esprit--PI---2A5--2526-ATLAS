# Architecture systeme

## Vue d'ensemble

ATLAS Smart Port est organise autour de trois blocs principaux :

1. une application desktop Qt/C++ ;
2. une base de donnees Oracle accessible via ODBC ;
3. des modules embarques Arduino et ESP32 connectes par port serie.

## Composants

| Composant | Role |
| --- | --- |
| Application Qt/C++ | Interface principale, gestion des modules metier et interaction utilisateur. |
| Oracle Database | Persistance des donnees metier : employes, pecheurs, bateaux, quais, equipements et stocks. |
| ODBC | Couche de connexion entre Qt SQL et Oracle. |
| Scripts Python | Fonctions annexes : QR code, detection de visage, reconnaissance vocale et assistant stock. |
| Arduino Uno ou Nano | Controle de capteurs et actionneurs locaux. |
| ESP32 | Gestion GPS, WiFi ou modules embarques complementaires. |
| ESP32-CAM | Capture d'image et transmission vers l'application. |

## Flux fonctionnels

### Gestion metier

L'utilisateur interagit avec l'interface Qt. Les operations CRUD sont executees depuis les modules de l'application, puis enregistrees dans la base Oracle via Qt SQL et ODBC.

### Communication embarquee

Les cartes Arduino et ESP32 communiquent avec l'application via port serie. Les firmwares envoient des informations de capteurs et recoivent des commandes de controle pour certains actionneurs.

### Scripts d'assistance

Les scripts Python sont appeles pour des fonctionnalites specialisees :

- generation de QR code ;
- detection de visage ;
- reconnaissance vocale ;
- assistant de stock local ou connecte a Gemini.

## Diagramme logique

```text
Utilisateur
    |
    v
Application Qt/C++
    |----------------------|
    v                      v
Oracle via ODBC            Scripts Python
                            |
                            v
                 QR, visage, voix, assistant stock

Application Qt/C++
    |
    v
Ports serie USB
    |
    v
Arduino / ESP32 / ESP32-CAM
    |
    v
Capteurs, actionneurs, RFID, GPS
```
