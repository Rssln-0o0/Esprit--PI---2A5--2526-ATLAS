# ATLAS Smart Port

ATLAS Smart Port est une application desktop destinée à la gestion d'un environnement portuaire maritime connecté. Elle regroupe une interface Qt/C++, une connexion à une base de données Oracle, plusieurs modules de gestion métier, des scripts Python d'assistance et des firmwares pour cartes Arduino et ESP32.

Ce dépôt présente le code source, les scripts et les éléments embarqués nécessaires à la compréhension, à la compilation et à la démonstration du projet.

## Présentation

Le projet ATLAS vise à centraliser plusieurs fonctionnalités liées à la gestion d'un smart port :

- gestion des employés, pêcheurs, bateaux, quais, équipements et stocks ;
- suivi des opérations à travers une interface graphique Qt ;
- communication avec des cartes Arduino et ESP32 via port série ;
- intégration de capteurs et d'actionneurs pour la partie IoT ;
- génération de QR codes pour certains workflows applicatifs ;
- détection de visage avec OpenCV ;
- reconnaissance vocale en français ;
- assistant de stock avec réponses locales et support optionnel de l'API Gemini.

## Technologies utilisées

| Couche | Technologies |
| --- | --- |
| Application desktop | C++17, Qt Widgets, Qt Designer |
| Modules Qt | Core, GUI, Charts, SQL, PrintSupport, Network, Multimedia, SerialPort, TextToSpeech, Quick, Location |
| Base de données | Oracle via Qt SQL ODBC |
| Embarqué et IoT | Arduino Uno, ESP32, ESP32-CAM, RFID RC522, GPS, capteurs, actionneurs |
| Scripts Python | Python 3, qrcode, OpenCV, SpeechRecognition, API Google Gemini  |

## Structure du dépôt

```text
.
|-- arduino/                         Firmwares Arduino et ESP32
|-- esp32_cam/                       Firmware série ESP32-CAM
|-- demo/                            Captures, vidéos, GIFs ou liens de démonstration
|-- docs/                            Documentation technique, schémas et notes de configuration
|-- images/                          Images et ressources utilisées par l'interface
|-- musique/                         Ressources audio
|-- python/                          Scripts Python additionnels
|-- *.cpp, *.h, *.ui                 Code source de l'application Qt/C++
|-- test.pro.pro                     Fichier projet qmake
|-- .env.example                     Modèle public des variables d'environnement
|-- .gitignore                       Fichiers locaux exclus du versionnement
```

## Prérequis

L'environnement suivant est recommandé pour compiler et exécuter le projet :

- Windows 10 ou Windows 11 ;
- Qt 5.15+ ou Qt 6.x avec les modules indiqués dans `test.pro.pro` ;
- MinGW, MSVC 2022 ou un compilateur C++17 compatible avec Qt ;
- driver Oracle ODBC installé ;
- source de données ODBC configurée pour la base Oracle ;
- Python 3.10+ pour les scripts d'assistance ;
- Arduino IDE ou `arduino-cli` pour téléverser les firmwares ;
- matériel IoT compatible avec les sketches fournis : Arduino Uno ou Nano, ESP32, ESP32-CAM, module RFID RC522, GPS, capteurs et actionneurs.

## Variables d'environnement

Le fichier `.env.example` sert de modèle public pour la configuration locale.

Créer un fichier `.env` local à partir du modèle :

```bash
cp .env.example .env
```

Sous Windows PowerShell :

```powershell
Copy-Item .env.example .env
```

Notes de configuration :

- `atlas_ai_stock.py` lit `GEMINI_API_KEY` ou `STOCK_GEMINI_API_KEY`.
- `.env.example` documente les variables attendues pour faciliter une future migration vers une configuration chargée au runtime.

## Configuration de la base de données

L'application utilise Oracle via une connexion ODBC Qt SQL. Avant le lancement :

1. installer le driver Oracle ODBC ;
2. créer une source de données ODBC avec le DSN attendu par le projet ;
3. créer ou importer le schéma de base de données nécessaire ;
4. placer les scripts SQL publiables dans `docs/` ;
5. éviter tout dump contenant des données privées ou des identifiants réels.

Les informations de connexion à renseigner localement sont indiquées dans `.env.example`.

## Installation des scripts Python

Créer un environnement virtuel :

```bash
python -m venv .venv
```

Activer l'environnement sous Windows PowerShell :

```powershell
.\.venv\Scripts\Activate.ps1
```

Installer les dépendances utilisées par les scripts :

```bash
pip install -r docs/requirements.txt
```

Selon la version de Python utilisée sous Windows, `PyAudio` peut nécessiter l'installation préalable d'une wheel compatible.

## Compilation et exécution de l'application Qt

### Avec Qt Creator

1. Ouvrir `test.pro.pro` dans Qt Creator.
2. Sélectionner un kit Desktop contenant les modules Qt requis.
3. Exécuter qmake.
4. Compiler le projet.
5. Lancer l'application depuis Qt Creator.

### Avec qmake en ligne de commande

Depuis la racine du dépôt :

```bash
qmake test.pro.pro
make
```

Sous Windows avec MinGW :

```powershell
qmake .\test.pro.pro
mingw32-make
```

L'exécutable généré se trouve dans le dossier de build configuré par Qt.

## Lancement

Après compilation, lancer l'exécutable généré depuis le dossier de build Qt.

Sous Windows, l'application peut aussi être lancée directement depuis Qt Creator avec le bouton Run après configuration du kit Desktop.

Avant le lancement, vérifier que :
- la source ODBC Oracle est configurée ;
- le fichier `.env` local existe ;
- les ressources nécessaires sont présentes dans les dossiers `images/`, `musique/` et `python/`.

## Firmwares embarqués

Les firmwares sont fournis sous forme de sketches Arduino. Chaque sketch doit être ouvert dans Arduino IDE ou compilé avec `arduino-cli`, en sélectionnant la carte et le port série correspondant au matériel utilisé.

| Firmware | Chemin |
| --- | --- |
| Capteurs et actionneurs Smart Port | `arduino/smart_port_sensors_actuators/smart_port_sensors_actuators.ino` |
| Porte RFID avec servos et bouton tactile | `arduino/rfid_porte_2servos_touch/rfid_porte_2servos_touch.ino` |
| RFID RC522 Uno | `arduino/rfid_rc522_uno/rfid_rc522_uno.ino` |
| ESP32 GPS | `arduino/esp32_gps/esp32_gps.ino` |
| ESP32 GPS WiFi | `arduino/esp32_gps_wifi/esp32_gps_wifi.ino` |
| ESP32-CAM série | `esp32 cam/esp32_cam_serial/esp32_cam_serial.ino` |
| Système Quai | `arduino/Quai_system/Quai_sys/Quai_sys.ino` |

Exemple de compilation et de téléversement avec `arduino-cli` :

```bash
arduino-cli compile --fqbn arduino:avr:uno arduino/smart_port_sensors_actuators
arduino-cli upload -p COM3 --fqbn arduino:avr:uno arduino/smart_port_sensors_actuators
```

Le port série et le FQBN doivent être adaptés à la carte utilisée.

## Démonstration

Les éléments de démonstration sont disponibles dans les dossiers `demo/` et `docs/` :

- schéma de câblage : `docs/schema-cablage.png`
- liste du matériel : `docs/liste-materiel.md`
- architecture système : `docs/architecture-systeme.png`
- vidéo de démonstration : `demo/demo.mp4`
- captures d'écran : `demo/screenshots/`
- scripts SQL et notes de configuration : `docs/`

## Auteurs

- Équipe : ATLAS
- Classe : 2A5
- Année universitaire : 2025-2026
- École : ESPRIT School of Engineering

