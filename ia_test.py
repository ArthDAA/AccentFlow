#!/usr/bin/env python3

from pynput import keyboard
from pynput.keyboard import Controller

# Initialisation
controller = Controller()

# Table des accents
accents = {
    'e': ['é', 'è', 'ê', 'ë'], 'E': ['É', 'È', 'Ê', 'Ë'],
    'a': ['à', 'â', 'ä', 'æ'], 'A': ['À', 'Â', 'Ä', 'Æ'],
    'i': ['î', 'ï'], 'I': ['Î', 'Ï'],
    'o': ['ô', 'ö', 'œ'], 'O': ['Ô', 'Ö', 'Œ'],
    'u': ['ù', 'û', 'ü'], 'U': ['Ù', 'Û', 'Ü'],
    'y': ['ÿ'], 'Y': ['Ÿ'],
    'c': ['ç'], 'C': ['Ç']
}

# États
accent_mode = False
accent_indices = {}   # index courant par lettre
last_base_key = None  # dernière lettre pressée
last_output = ""      # dernier caractère réellement inséré


def delete_last_output():
    """Efface exactement ce qui a été inséré précédemment."""
    global last_output
    if last_output:
        for _ in last_output:
            controller.press(keyboard.Key.backspace)
            controller.release(keyboard.Key.backspace)
        last_output = ""


def on_press(key):
    global accent_mode, accent_indices, last_base_key, last_output

    # Toggle accent mode
    if key == keyboard.Key.shift_r:
        accent_mode = not accent_mode
        print(f"[INFO] Accent mode {'ON' if accent_mode else 'OFF'}")
        if not accent_mode:
            accent_indices = {}
            last_base_key = None
            last_output = ""
        return

    # Mode accent actif
    if accent_mode:
        try:
            base_key = key.char
        except AttributeError:
            return  # touches spéciales ignorées

        if base_key not in accents:
            return  # touche non concernée → ignorée

        # Nouvelle lettre ?
        is_new_letter = (last_base_key != base_key)

        if is_new_letter:
            # Reset des indices uniquement pour cette lettre
            accent_indices[base_key] = 0
            last_base_key = base_key
            delete_last_output()
        else:
            delete_last_output()

        # Récupérer l’index courant
        current_index = accent_indices.get(base_key, 0)
        accented_char = accents[base_key][current_index]

        # Écrire le caractère accentué
        controller.type(accented_char)
        last_output = accented_char

        # Avancer l’index
        accent_indices[base_key] = (current_index + 1) % len(accents[base_key])

    # Mode accent OFF → comportement normal (laisser passer la touche)


def on_release(key):
    if key == keyboard.Key.esc:
        print("[INFO] Listener arrêté.")
        return False


# Lancer l’écoute
with keyboard.Listener(on_press=on_press, on_release=on_release) as listener:
    listener.join()

