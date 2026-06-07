# ATLAS Smart Port

ATLAS Smart Port est une application desktop de gestion pour un environnement portuaire maritime connecte. Le projet combine une interface Qt/C++, une connexion a une base de donnees Oracle, des firmwares Arduino et ESP32, ainsi que des scripts Python pour la generation de QR codes, la detection de visage, les commandes vocales et l'assistant de stock ATLAS.

Ce depot est prepare pour une publication sur l'organisation GitHub ESPRIT. Une personne externe au projet doit pouvoir le cloner, installer les outils documentes, configurer l'environnement, compiler l'application et comprendre le materiel necessaire.

## Fonctionnalites

- Tableau de bord desktop construit avec Qt Widgets et des fichiers Qt Designer
- Modules CRUD pour les employes, pecheurs, bateaux, quais, equipements et stocks
- Acces a une base Oracle via une source de donnees ODBC
- Assistant de stock avec reponses locales et support optionnel de l'API Gemini
- Generation de QR codes pour les workflows de l'application
- Script de detection de visage avec OpenCV
- Script de reconnaissance vocale pour les commandes en francais
- Communication serie avec Arduino Uno, ESP32-CAM, RFID, GPS et firmwares Smart Port
- Dossiers `demo/` et `docs/` reserves aux captures, videos, schemas et notes techniques

## Technologies

| Couche | Outils |
| --- | --- |
| Application desktop | C++17, Qt Widgets, Qt Designer |
| Modules Qt | Core, GUI, Charts, SQL, PrintSupport, Network, Multimedia, SerialPort, TextToSpeech, Quick, Location |
| Base de donnees | Oracle via Qt SQL ODBC |
| Embarque et IoT | Arduino Uno, ESP32, ESP32-CAM, RFID RC522, GPS, capteurs, actionneurs |
| Scripts Python | Python 3, qrcode, OpenCV, SpeechRecognition, Google Gemini optionnel |

## Structure du depot

```text
.
|-- arduino/                         Firmwares Arduino et ESP32
|-- esp32 cam/                       Firmware serie ESP32-CAM
|-- demo/                            Captures, GIFs, APKs, videos ou liens de demo publics
|-- docs/                            Architecture, schemas de cablage, notes base de donnees et API
|-- images/                          Images et ressources de l'interface
|-- musique/                         Ressources audio
|-- python/                          Scripts Python additionnels
|-- *.cpp, *.h, *.ui                 Code source de l'application Qt/C++
|-- test.pro.pro                     Fichier projet qmake Qt
|-- .env.example                     Modele des variables d'environnement
|-- .gitignore                       Fichiers locaux exclus de Git
```

## Prerequis

- OS cible teste : Windows 10 ou Windows 11
- Qt : Qt 5.15+ ou Qt 6.x avec les modules listes dans `test.pro.pro`
- Compilateur : MinGW, MSVC 2022 ou un autre compilateur C++17 compatible avec Qt
- Driver Oracle ODBC et DSN configure
- Python 3.10+ pour les scripts d'aide
- Arduino IDE ou `arduino-cli` pour televerser les firmwares
- Materiel physique pour la partie IoT, dont Arduino Uno ou Nano, ESP32, ESP32-CAM, RFID RC522, GPS, capteurs et actionneurs utilises par les sketches Smart Port

## Variables d'environnement

Copier le fichier d'exemple avant d'executer les outils locaux :

```bash
cp .env.example .env
```

Sur Windows PowerShell :

```powershell
Copy-Item .env.example .env
```

Remplir uniquement le fichier local `.env`. Ne jamais commiter de vraies cles API, mots de passe de base de donnees, identifiants Oracle ou secrets lies aux appareils.

Notes sur le code actuel :

- `atlas_ai_stock.py` lit `GEMINI_API_KEY` ou `STOCK_GEMINI_API_KEY`.
- `connection.cpp` contient actuellement le DSN Oracle, le nom d'utilisateur et le mot de passe directement dans le code. Garder `.env.example` comme reference publique de configuration et mettre a jour la couche de connexion C++ si un chargement runtime des variables d'environnement est souhaite.

## Configuration de la base de donnees

1. Installer le driver Oracle ODBC.
2. Creer une source de donnees ODBC avec le DSN utilise par le projet.
3. Creer ou importer le schema de base de donnees requis.
4. Placer les scripts SQL ou scripts de seed dans `docs/` s'ils peuvent etre publies sans risque.
5. Ne jamais commiter un dump reel de base de donnees contenant des donnees privees.

Les valeurs de connexion locales attendues sont documentees dans `.env.example`.

## Installation des scripts Python

Creer un environnement virtuel :

```bash
python -m venv .venv
```

L'activer sur Windows PowerShell :

```powershell
.\.venv\Scripts\Activate.ps1
```

Installer les dependances des scripts :

```bash
pip install google-generativeai opencv-python qrcode SpeechRecognition pocketsphinx PyAudio
```

Si l'installation de `PyAudio` echoue sous Windows, installer une wheel compatible avec votre version de Python, puis relancer la commande.

## Compilation et lancement de l'application Qt

### Qt Creator

1. Ouvrir `test.pro.pro` dans Qt Creator.
2. Selectionner un kit Desktop avec les modules Qt requis.
3. Executer qmake.
4. Compiler le projet.
5. Lancer l'application depuis Qt Creator.

### Ligne de commande avec qmake

Depuis la racine du depot :

```bash
qmake test.pro.pro
make
```

Sur Windows avec MinGW :

```powershell
qmake .\test.pro.pro
mingw32-make
```

Lancer ensuite l'executable genere depuis le dossier de build.

## Televersement des firmwares

Ouvrir le fichier `.ino` necessaire dans Arduino IDE, selectionner la bonne carte et le bon port, puis utiliser Verifier et Televerser.

Principaux sketches :

| Firmware | Chemin |
| --- | --- |
| Capteurs et actionneurs Smart Port | `arduino/smart_port_sensors_actuators/smart_port_sensors_actuators.ino` |
| Porte RFID avec servos et bouton tactile | `arduino/rfid_porte_2servos_touch/rfid_porte_2servos_touch.ino` |
| RFID RC522 Uno | `arduino/rfid_rc522_uno/rfid_rc522_uno.ino` |
| ESP32 GPS | `arduino/esp32_gps/esp32_gps.ino` |
| ESP32 GPS WiFi | `arduino/esp32_gps_wifi/esp32_gps_wifi.ino` |
| ESP32-CAM serie | `esp32 cam/esp32_cam_serial/esp32_cam_serial.ino` |
| Systeme Quai | `arduino/Quai_system/Quai_sys/Quai_sys.ino` |

Exemple avec `arduino-cli` :

```bash
arduino-cli compile --fqbn arduino:avr:uno arduino/smart_port_sensors_actuators
arduino-cli upload -p COM3 --fqbn arduino:avr:uno arduino/smart_port_sensors_actuators
```

Remplacer `COM3` et le FQBN de la carte par les valeurs correspondant a votre materiel.

## Checklist demo et documentation

Le guide de publication ESPRIT demande aux projets IoT d'inclure une preuve externe du bon fonctionnement du materiel. Avant la soumission, ajouter :

- `docs/schema-cablage.png` ou un autre schema de cablage annote
- `docs/liste-materiel.md` avec les references exactes du materiel
- `docs/architecture-systeme.png` ou un schema d'architecture equivalent
- `demo/demo.mp4` ou un lien video public montrant l'application et le materiel
- Des captures d'ecran dans `demo/screenshots/` si disponibles
- Le schema de base de donnees ou les notes de configuration dans `docs/`

## Test de lancement externe

Avant de soumettre le depot, tester le projet dans un dossier propre :

```bash
git clone https://github.com/USERNAME/Esprit-[PI]-[2A5]-2526-ATLAS.git
cd Esprit-[PI]-[2A5]-2526-ATLAS
cp .env.example .env
qmake test.pro.pro
make
```

Verifier que :

- Le projet Qt compile sans fichier manquant.
- Les commandes du README suffisent pour lancer le projet.
- Les etapes de connexion a la base de donnees sont documentees.
- Le materiel requis, le cablage et les firmwares sont documentes.
- Aucun `.env`, cle API, mot de passe, dump de base de donnees, dossier de build ou parametre IDE n'est commite.

## Auteurs

- Equipe : ATLAS
- Classe : 2A5
- Annee universitaire : 2025-2026
- Ecole : ESPRIT School of Engineering

Ajouter les noms des membres de l'equipe, le nom de l'encadrant, le lien de demo et le lien de deploiement avant la soumission finale.
