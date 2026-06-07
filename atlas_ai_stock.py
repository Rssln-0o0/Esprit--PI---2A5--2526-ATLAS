import os
import re
import sys
import time
import random
import unicodedata
from datetime import datetime

try:
    import google.generativeai as genai
except Exception:
    genai = None


DEFAULT_GEMINI_KEY = ""


STOCK_QA = {
    "comment ajouter un stock": "Dans l'onglet Stock, remplissez le formulaire puis cliquez sur Ajouter.",
    "comment modifier un stock": "Selectionnez une ligne, cliquez sur Modifier, ajustez les champs puis cliquez sur Save.",
    "comment supprimer un stock": "Selectionnez une ligne du tableau puis cliquez sur Supprimer et confirmez.",
    "a quoi sert seuil minimum": "Le seuil minimum indique le niveau d'alerte bas du stock.",
    "a quoi sert seuil maximum": "Le seuil maximum indique la capacite haute recommandee du stock.",
    "pourquoi quantite invalide": "La quantite doit etre numerique et comprise entre S_MIN et S_MAX.",
    "comment filtrer le tableau": "Utilisez la barre de recherche et la liste de tri dans la section Stock.",
    "comment rafraichir le tableau": "Cliquez sur le bouton Rafraichir pour recharger les donnees.",
    "comment voir l historique": "Allez sur l'onglet Histoire du module Stock pour voir les operations CRUD.",
    "comment nettoyer historique": "Utilisez le bouton Nettoyer Histoire puis confirmez la suppression.",
    "comment exporter historique pdf": "Cliquez sur Export PDF dans l'onglet Histoire puis choisissez l'emplacement du fichier.",
    "que contient history csv": "Le fichier history.csv contient Date/Heure, Action, Stock ID, Espece, Quantite et Etat.",
}

FALLBACK_MESSAGES = [
    "Je n'ai pas bien compris. Reformulez votre question sur le module Stock.",
    "Je peux vous aider sur le CRUD Stock, les seuils, l'historique CSV et l'export PDF.",
    "Question hors scope Stock. Essayez une question sur les operations stock.",
]

LOOP_MESSAGES = [
    "Souhaitez-vous une autre aide sur le module Stock ?",
    "Je peux aussi aider sur l'historique et l'export PDF.",
    "Besoin d'une precision supplementaire ?",
]


def get_greeting() -> str:
    hour = datetime.now().hour
    if 5 <= hour < 12:
        return "☀️ Bonjour, ATLAS est a votre service."
    if 12 <= hour < 18:
        return "🌤️ Bon apres-midi, ATLAS est a votre service."
    if 18 <= hour < 22:
        return "🌙 Bonsoir, ATLAS est a votre service."
    return "🌌 Bonne nuit, ATLAS est a votre service."


def type_writer(text: str, delay: float = 0.02) -> None:
    if delay <= 0:
        sys.stdout.write(text + "\n")
        sys.stdout.flush()
        return

    for char in text:
        sys.stdout.write(char)
        sys.stdout.flush()
        time.sleep(delay)
    sys.stdout.write("\n")
    sys.stdout.flush()


def normalize(text: str) -> str:
    text = text.strip().lower()
    text = unicodedata.normalize("NFKD", text)
    text = "".join(ch for ch in text if not unicodedata.combining(ch))
    text = re.sub(r"[^a-z0-9\s]", " ", text)
    text = re.sub(r"\s+", " ", text)
    return text


def lookup_local_answer(user_input: str) -> str:
    q = normalize(user_input)
    for key, value in STOCK_QA.items():
        if key in q or q in key:
            return value
    return ""


def gemini_stock_answer(user_input: str) -> str:
    if genai is None:
        return (
            "Je peux vous aider sur le module Stock (CRUD, seuils, historique, export PDF). "
            "Le module Gemini n'est pas disponible dans cet environnement Python."
        )

    api_key = (
        os.getenv("GEMINI_API_KEY", "").strip()
        or os.getenv("STOCK_GEMINI_API_KEY", "").strip()
        or DEFAULT_GEMINI_KEY
    )
    if not api_key:
        return (
            "Je peux vous aider sur le module Stock (CRUD, seuils, historique, export PDF). "
            "Aucun modele Gemini n'est configure sur cette machine."
        )

    genai.configure(api_key=api_key)
    model = genai.GenerativeModel("gemini-2.5-flash-lite")

    prompt = (
        "Vous etes ATLAS, un assistant pour le module Stock d'une application Qt. "
        "Repondez en francais, de maniere concise et pratique. "
        "Vous ne traitez que les sujets stock: ajout, modification, suppression, seuils, tri, recherche, historique CSV, export PDF.\n\n"
        "Question utilisateur: " + user_input
    )

    try:
        response = model.generate_content(prompt)
        text = (response.text or "").strip()
        if text:
            return text
        return random.choice(FALLBACK_MESSAGES)
    except Exception:
        return "Le service ATLAS est temporairement indisponible. Reessayez dans un instant."


def answer(user_input: str) -> str:
    if not user_input.strip():
        return "Veuillez saisir une question sur le module Stock."

    local = lookup_local_answer(user_input)
    if local:
        return f"{local}\n{random.choice(LOOP_MESSAGES)}"

    model_reply = gemini_stock_answer(user_input)
    if not model_reply.strip():
        return random.choice(FALLBACK_MESSAGES)
    return f"{model_reply}\n{random.choice(LOOP_MESSAGES)}"


def main() -> int:
    if len(sys.argv) > 1 and sys.argv[1].strip().lower() == "__atlas_greeting__":
        type_writer(get_greeting(), delay=0)
        return 0

    if len(sys.argv) > 1:
        user_input = " ".join(sys.argv[1:]).strip()
    else:
        user_input = sys.stdin.read().strip()

    type_writer(answer(user_input), delay=0)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
