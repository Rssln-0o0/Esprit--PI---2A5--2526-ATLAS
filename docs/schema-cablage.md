# Schema de cablage

## Objectif

Ce document decrit le cablage attendu pour la partie IoT du projet ATLAS Smart Port. Il complete les firmwares presents dans le dossier `arduino/`.

Un schema visuel annote est recommande pour la soumission finale. Le fichier attendu peut etre ajoute sous le nom :

```text
docs/schema-cablage.png
```

## Montage Smart Port capteurs et actionneurs

Firmware associe :

```text
arduino/smart_port_sensors_actuators/smart_port_sensors_actuators.ino
```

| Broche | Composant | Description |
| --- | --- | --- |
| D2 | Capteur gaz | Entree digitale, actif LOW. |
| D3 | Capteur humidite ou pluie | Entree digitale, actif LOW. |
| D4 | Capteur flamme | Entree digitale, actif LOW. |
| D5 | Capteur pluie | Entree digitale, actif LOW. |
| D6 | Capteur PIR | Detection de mouvement. |
| D7 | Bouton tactile | Commande locale. |
| D8 | Ventilateur | Sortie vers transistor ou module relais. |
| D9 | Buzzer | Sortie sonore avec `tone()`. |
| D10 | Pompe a eau | Sortie vers transistor ou module relais. |
| D12 | LED eclairage | Sortie de controle de l'eclairage. |
| D13 | Contact magnetique | Detection d'ouverture ou fermeture. |
| A0 | LED gaz | Indicateur visuel. |
| A1 | LED humidite | Indicateur visuel. |
| A2 | LED flamme | Indicateur visuel. |
| A3 | LED mouvement | Indicateur visuel. |
| A4 | HC-SR04 TRIG | Declenchement ultrason. |
| A5 | HC-SR04 ECHO | Retour ultrason. |

## Recommandations de cablage

- Utiliser une masse commune entre la carte et les modules externes.
- Alimenter les actionneurs avec une source adaptee si le courant USB est insuffisant.
- Utiliser un transistor, module relais ou driver adapte pour les charges comme pompe et ventilateur.
- Verifier la polarite des capteurs actifs LOW ou actifs HIGH avant la demonstration.
- Isoler les circuits de puissance des circuits logiques lorsque cela est necessaire.
