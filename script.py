from pynput import keyboard
from pynput.keyboard import Controller

# Initialisation du contrôleur clavier
controller = Controller()

# Table des accents
accents = {
    'e': ['é', 'è', 'ê', 'ë'],
    'a': ['à', 'â', 'ä', 'æ'],
    'i': ['î', 'ï'],
    'o': ['ô', 'ö', 'œ'],
    'u': ['ù', 'û', 'ü'],
    'y': ['ÿ'],
    'c': ['ç'],
    'E': ['É', 'È', 'Ê', 'Ë'],
    'A': ['À', 'Â', 'Ä', 'Æ'],
    'I': ['Î', 'Ï'],
    'O': ['Ô', 'Ö', 'Œ'],
    'U': ['Ù', 'Û', 'Ü'],
    'Y': ['Ÿ'],
    'C': ['Ç']
}

# États des lettres pour suivre les accents
accent_states = {letter: 0 for letter in accents}
lettre_pressee = '\0'  # Stocke la dernière touche pressée
active_letter = None
accent_mode = False  # Mode verrouillé/déverrouillé avec shift_r


def delete_characters(count):
    """Supprime un nombre donné de caractères."""
    for _ in range(count):
        controller.press(keyboard.Key.backspace)
        controller.release(keyboard.Key.backspace)


def handle_accent(letter):
    """Insère la lettre accentuée et met à jour l'état."""
    accented_char = accents[letter][accent_states[letter]]
    controller.type(accented_char)
    accent_states[letter] = (accent_states[letter] + 1) % len(accents[letter])


def reset_accent_states():
    """Réinitialise les états des accents."""
    global active_letter, accent_states, lettre_pressee
    active_letter = None
    accent_states = {letter: 0 for letter in accents}
    lettre_pressee = '\0'  # Réinitialisation ici


def on_press(key):
    global active_letter, accent_mode, lettre_pressee

    # Activation/désactivation avec shift_r
    if key == keyboard.Key.shift_r:
        accent_mode = not accent_mode
        print(f"Accent mode {'activated' if accent_mode else 'deactivated'}")
        if not accent_mode:
            reset_accent_states()
        return

    if accent_mode:
        try:
            touche_clavier = key.char

            # Gestion des backspaces selon l'état de la touche pressée
            if touche_clavier in accents:
                delete_characters(1)  # Suppression par défaut

                if touche_clavier == lettre_pressee:
                    delete_characters(1)  # Suppression supplémentaire si répétition
                else:
                    lettre_pressee = touche_clavier

                handle_accent(touche_clavier)

        except (AttributeError, TypeError):
            pass


def on_release(key):
    if key == keyboard.Key.esc:
        return False


# Lancer l'écoute
with keyboard.Listener(on_press=on_press, on_release=on_release) as listener:
    listener.join()
