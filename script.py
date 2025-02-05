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
special_mode = False


def on_press(key):
    global current_letter, current_index, special_mode

    try:
        if key == keyboard.Key.shift_r:
            special_mode = True
            print("[Mode accents activé]")
        
        # Gérer les lettres
        if special_mode and hasattr(key, 'char') and key.char in accents:
            current_letter = key.char
            print(f"Accent sélectionné : {accents[current_letter][current_index]}")

    except AttributeError:
        pass


def on_release(key):
    global current_letter, current_index, special_mode

    if key == keyboard.Key.f24:
        special_mode = False
        print("[Mode accents désactivé]")

    # Gérer la rotation des accents
    if special_mode and current_letter:
        current_index = (current_index + 1) % len(accents[current_letter])
        print(f"Prochain accent : {accents[current_letter][current_index]}")


# Démarrage de l'écoute clavier
with keyboard.Listener(on_press=on_press, on_release=on_release) as listener:
    listener.join()