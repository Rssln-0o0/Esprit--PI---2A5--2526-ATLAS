# Validation du depot

## Objectif

Ce document sert de checklist de validation avant publication du projet ATLAS Smart Port sur GitHub. Il doit etre mis a jour apres les tests finaux.

## Etat de validation

| Point de controle | Statut | Commentaire |
| --- | --- | --- |
| Compilation Qt | A verifier | Compiler le projet depuis `test.pro.pro` avec le kit Qt cible. |
| Lancement application | A verifier | Tester le lancement depuis Qt Creator ou depuis l'executable genere. |
| Connexion Oracle ODBC | A verifier | Verifier que le DSN local pointe vers la base attendue. |
| Scripts Python | A verifier | Installer `docs/requirements.txt` et tester les scripts principaux. |
| Firmwares Arduino et ESP32 | A verifier | Compiler et televerser les sketches utilises dans la demonstration. |
| Demonstration video | Present | Lien YouTube disponible dans `demo/Video Demo Link.txt`. |
| Documentation materiel | Present | Voir `docs/liste-materiel.md`. Completer les references exactes si necessaire. |
| Documentation cablage | Present | Voir `docs/schema-cablage.md`. Ajouter une image annotee si disponible. |
| Documentation base de donnees | Present | Voir `docs/database.md` et les scripts SQL dans `docs/`. |
| Secrets et fichiers locaux | A verifier | Aucun `.env`, mot de passe reel, cle API ou dump prive ne doit etre versionne. |

## Commandes de verification recommandees

Depuis la racine du depot :

```bash
git status --short
```

```bash
git diff --check
```

Verification Python :

```bash
python -m venv .venv
pip install -r docs/requirements.txt
python qr_generator.py "ATLAS" atlas_qr_test.png
```

Verification Qt :

```bash
qmake test.pro.pro
make
```

Sous Windows avec MinGW :

```powershell
qmake .\test.pro.pro
mingw32-make
```
