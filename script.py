from pynput import keyboard
from pynput.keyboard import Controller

# Initialisation du contrôleur
controller = Controller()

# Table des accents
accents = {
    'e': ['é', 'è', 'ê', 'ë'],
    'a': ['à', 'â', 'ä'],
    'i': ['î', 'ï'],
    'o': ['ô', 'ö'],
    'u': ['ù', 'û', 'ü'],
    'y': ['ÿ'],
    'c': ['ç']
}

# États des lettres pour suivre les accents
accent_states = {letter: 0 for letter in accents}
active_letter = None
accent_mode = False  # Désactivé par défaut


def delete_previous_character():
    """Supprime la lettre précédente."""
    controller.press(keyboard.Key.backspace)
    controller.release(keyboard.Key.backspace)
    controller.press(keyboard.Key.backspace)
    controller.release(keyboard.Key.backspace)


def handle_accent(letter):
    """Insère la lettre accentuée et met à jour l'état."""
    accented_char = accents[letter][accent_states[letter]]
    controller.type(accented_char)

    # Passe à l'accent suivant
    accent_states[letter] = (accent_states[letter] + 1) % len(accents[letter])


def on_press(key):
    global active_letter, accent_mode

    try:
        # Activer le mode accentuation avec Shift droit (F24)
        if key == keyboard.Key.shift_r:
            accent_mode = True

        if accent_mode and key.char in accents:
            if key.char == active_letter:
                # Supprimer la lettre précédente et ajouter un nouvel accent
                delete_previous_character()
                handle_accent(active_letter)
            else:
                # Nouvelle lettre, réinitialiser l'état
                controller.press(keyboard.Key.backspace)
                controller.release(keyboard.Key.backspace)
                active_letter = key.char
                accent_states[active_letter] = 0
                handle_accent(active_letter)
    except (AttributeError, TypeError):
        pass


def on_release(key):
    global active_letter, accent_mode

    # Désactiver le mode accentuation lorsque Shift droit est relâché
    if key == keyboard.Key.shift_r:
        accent_mode = False

    if key == keyboard.Key.esc:  # Escape pour quitter proprement
        return False


# Lancer l'écoute
with keyboard.Listener(on_press=on_press, on_release=on_release) as listener:
    listener.join()
