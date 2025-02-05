from pynput import keyboard
from pynput.keyboard import Controller

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

# État des lettres (initialement à 0)
letter_states = {letter: 0 for letter in accents}
current_indices = {letter: 0 for letter in accents}

controller = Controller()


def reset_states():
    """Remet tous les états de lettre à 0."""
    for letter in letter_states:
        letter_states[letter] = 0


def handle_accent(letter):
    """Supprime la lettre précédente et insère la version accentuée."""
    # Supprimer la lettre de base + accent précédent
    controller.press(keyboard.Key.backspace)
    controller.release(keyboard.Key.backspace)
    controller.press(keyboard.Key.backspace)
    controller.release(keyboard.Key.backspace)

    # Insère la lettre accentuée
    accent = accents[letter][current_indices[letter]]
    controller.type(accent)


def on_press(key):
    try:
        # Vérifie si c'est une lettre avec accents possibles
        if hasattr(key, 'char') and key.char in accents:
            letter = key.char

            # Si la lettre est déjà active, change l'accent
            if letter_states[letter] == 1:
                current_indices[letter] = (current_indices[letter] + 1) % len(accents[letter])
                handle_accent(letter)
            else:
                # Activer cette lettre, désactiver les autres
                reset_states()
                letter_states[letter] = 1
                current_indices[letter] = 0

    except Exception as e:
        print(f"Erreur : {e}")


def on_release(key):
    # Permet de quitter avec la touche ESC
    if key == keyboard.Key.esc:
        return False


# Écoute du clavier
with keyboard.Listener(on_press=on_press, on_release=on_release) as listener:
    listener.join()
