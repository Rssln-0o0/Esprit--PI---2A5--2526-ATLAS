#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Reconnaissance vocale pour l'application - écrit chaque phrase reconnue dans le fichier donné.
Utilise SpeechRecognition : Google (gratuit, internet) ou Sphinx (hors ligne).
"""
import sys
import os

def main():
    if len(sys.argv) < 2:
        sys.exit(1)
    out_file = os.path.normpath(os.path.abspath(sys.argv[1]))
    if not out_file:
        sys.exit(1)
    out_dir = os.path.dirname(out_file)
    if out_dir:
        try:
            os.makedirs(out_dir, exist_ok=True)
        except OSError:
            pass

    try:
        import speech_recognition as sr
    except ImportError:
        sys.exit(10)  # module non installé

    r = sr.Recognizer()
    r.energy_threshold = 180
    r.dynamic_energy_threshold = True
    r.pause_threshold = 0.9

    def is_valid_command(text):
        """Ignore le bruit et les fausses reconnaissances (ex: 'tu tu tu...')."""
        t = text.strip()
        if not t:
            return False
        tl = t.lower()
        # Mots courts utiles pour les confirmations (oui/non/ok)
        if tl in ("no", "ok", "oui", "non"):
            return True
        single_word_ok = {
            "liste", "stats", "rechercher", "modifier", "rafraîchir", "rafraichir",
            "recharger", "actualiser", "employés", "employes", "paramètres", "parametres",
            "disponibilité", "disponibilite", "smartport",
            "tri", "alerte", "alertes", "photo", "esp32", "pdf", "excel", "notification",
        }
        words = t.split()
        if len(words) == 1 and tl in single_word_ok:
            return True
        if len(t) < 3:
            return False
        if len(words) == 1 and len(t) < 4:
            return False  # un seul mot très court souvent du bruit
        # une seule forme répétée = probablement du bruit
        if len(words) >= 3 and len(set(w.lower() for w in words)) == 1:
            return False
        return True

    def write_line(text):
        if not text or not text.strip():
            return
        if not is_valid_command(text):
            return
        try:
            # Ligne unique \n : l’app Qt découpe avec [\r\n]+ ; évite verrouillage \r\n Windows.
            with open(out_file, 'a', encoding='utf-8', newline='\n') as f:
                f.write(text.strip() + '\n')
                f.flush()
                try:
                    os.fsync(f.fileno())
                except (OSError, AttributeError):
                    pass
        except OSError:
            pass

    def open_microphone():
        """Ouvre le micro par défaut ; sous Windows essaie les index 0..3 en secours."""
        try:
            return sr.Microphone()
        except OSError:
            pass
        for idx in (0, 1, 2, 3):
            try:
                return sr.Microphone(device_index=idx)
            except OSError:
                continue
        raise OSError("Aucun micro disponible")

    try:
        mic = open_microphone()
    except OSError:
        sys.exit(3)  # micro inaccessible

    print("Micro OK. Calibration du bruit ambiant...", flush=True)
    calibrated = False
    while True:
        try:
            with mic as source:
                if not calibrated:
                    r.adjust_for_ambient_noise(source, duration=0.45)
                    calibrated = True
                    print("Ecoute en cours. Parlez (ex: 'liste employes', 'smart port')...", flush=True)
                try:
                    audio = r.listen(source, timeout=25, phrase_time_limit=12)
                except sr.WaitTimeoutError:
                    continue
        except OSError as e:
            if "Microphone" in str(e) or "micro" in str(e).lower():
                sys.exit(3)
            continue

        text = None
        for lang in ("fr-FR", "fr_CA", "fr_BE"):
            try:
                text = r.recognize_google(audio, language=lang)
                if text:
                    break
            except sr.UnknownValueError:
                pass
            except sr.RequestError:
                break  # pas de réseau : inutile de réessayer d’autres variantes FR
            except Exception:
                pass

        if not text:
            try:
                text = r.recognize_sphinx(audio, language="fr-fr")
            except sr.UnknownValueError:
                pass
            except OSError:
                pass
            except Exception:
                pass

        if text:
            if is_valid_command(text):
                print("Reconnu:", text, flush=True)
                write_line(text)


if __name__ == "__main__":
    try:
        main()
    except Exception as e:
        import traceback
        traceback.print_exc(file=sys.stderr)
        sys.stderr.flush()
        sys.exit(1)
