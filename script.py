from pynput import keyboard

# Table des accents : index [lettre][type d'accent]
accents = {
    'e': ['é', 'è', 'ê', 'ë'],
    'a': ['à', 'â', 'ä'],
    'i': ['î', 'ï'],
    'o': ['ô', 'ö'],
    'u': ['ù', 'û', 'ü'],
    'y': ['ÿ'],
    'c': ['ç']
}

current_letter = None
current_index = 0


def on_press(key):
    global current_letter, current_index

    try:
        # Gérer les lettres directement
        if hasattr(key, 'char') and key.char in accents:
            current_letter = key.char
            print(f"Accent sélectionné : {accents[current_letter][current_index]}")

    except AttributeError:
        pass


def on_release(key):
    global current_letter, current_index

    # Gérer la rotation des accents
    if current_letter:
        current_index = (current_index + 1) % len(accents[current_letter])
        print(f"Prochain accent : {accents[current_letter][current_index]}")


# Démarrage de l'écoute clavier
with keyboard.Listener(on_press=on_press, on_release=on_release) as listener:
    listener.join()
