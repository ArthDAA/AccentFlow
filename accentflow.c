/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   accent_editor.c                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: adarthus <adarthus@student.42nice.fr>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/14 00:00:00 by adarthus          #+#    #+#             */
/*   Updated: 2025/01/14 00:00:00 by adarthus         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <ncurses.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#define MAX_LINES 1000
#define MAX_LINE_LENGTH 1024
#define MAX_ACCENT_VARIANTS 8
#define ACCENT_COUNT 8
#define COLOR_POPUP_BG 1
#define COLOR_SELECTED 2
#define COLOR_PENDING 3#include <ncurses.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#define MAX_LINES 1000
#define MAX_LINE_LENGTH 1024
#define MAX_ACCENT_VARIANTS 8

typedef struct {
    char base;
    char variants[MAX_ACCENT_VARIANTS][4]; // UTF-8 jusqu'à 3 bytes + \0
    int count;
} AccentConfig;

typedef struct {
    char lines[MAX_LINES][MAX_LINE_LENGTH];
    int num_lines;
    int cursor_row;
    int cursor_col;
    
    // État des accents
    int accent_active;
    char accent_base;
    int accent_index;
    
    // Sauvegarde de ce qui est sous l'infobulle
    char saved_chars[MAX_ACCENT_VARIANTS];
    int saved_attrs[MAX_ACCENT_VARIANTS];
    int popup_row;
    int popup_col;
    int popup_width;
} Editor;

Editor editor = {0};

AccentConfig accents[] = {
    {'e', {"é", "è", "ê", "ë"}, 4},
    {'a', {"à", "â", "ä", "æ"}, 4},
    {'u', {"ù", "û", "ü"}, 3},
    {'i', {"î", "ï"}, 2},
    {'o', {"ô", "ö", "œ"}, 3},
    {'c', {"ç"}, 1},
    {'y', {"ÿ"}, 1},
    {'n', {"ñ"}, 1}
};

#define ACCENT_COUNT (sizeof(accents)/sizeof(accents[0]))

// Couleurs
#define COLOR_POPUP_BG 1
#define COLOR_SELECTED 2
#define COLOR_PENDING 3#include <ncurses.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#define MAX_LINES 1000
#define MAX_LINE_LENGTH 1024
#define MAX_ACCENT_VARIANTS 8

typedef struct {
    char base;
    char variants[MAX_ACCENT_VARIANTS][4]; // UTF-8 jusqu'à 3 bytes + \0
    int count;
} AccentConfig;

typedef struct {
    char lines[MAX_LINES][MAX_LINE_LENGTH];
    int num_lines;
    int cursor_row;
    int cursor_col;
    
    // État des accents
    int accent_active;
    char accent_base;
    int accent_index;
    
    // Sauvegarde de ce qui est sous l'infobulle
    char saved_chars[MAX_ACCENT_VARIANTS];
    int saved_attrs[MAX_ACCENT_VARIANTS];
    int popup_row;
    int popup_col;
    int popup_width;
} Editor;

Editor editor = {0};

AccentConfig accents[] = {
    {'e', {"é", "è", "ê", "ë"}, 4},
    {'a', {"à", "â", "ä", "æ"}, 4},
    {'u', {"ù", "û", "ü"}, 3},
    {'i', {"î", "ï"}, 2},
    {'o', {"ô", "ö", "œ"}, 3},
    {'c', {"ç"}, 1},
    {'y', {"ÿ"}, 1},
    {'n', {"ñ"}, 1}
};

#define ACCENT_COUNT (sizeof(accents)/sizeof(accents[0]))

// Couleurs
#define COLOR_POPUP_BG 1
#define COLOR_SELECTED 2
#define COLOR_PENDING 3

AccentConfig* find_accent_config(char ch) {
    for (int i = 0; i < ACCENT_COUNT; i++) {
        if (accents[i].base == tolower(ch)) {
            return &accents[i];
        }
    }
    return NULL;
}

void init_colors() {
    start_color();
    init_pair(COLOR_POPUP_BG, COLOR_WHITE, COLOR_BLUE);      // Blanc sur bleu marine
    init_pair(COLOR_SELECTED, COLOR_YELLOW, COLOR_BLUE);     // Jaune sur bleu marine
    init_pair(COLOR_PENDING, COLOR_WHITE, COLOR_BLACK);      // Blanc sur noir
}

void init_editor() {
    editor.num_lines = 1;
    strcpy(editor.lines[0], "");
    editor.cursor_row = 0;
    editor.cursor_col = 0;
    editor.accent_active = 0;
}

void save_popup_area(int row, int col, int width) {
    editor.popup_row = row;
    editor.popup_col = col;
    editor.popup_width = width;
    
    for (int i = 0; i < width; i++) {
        if (col + i < COLS && row >= 0 && row < LINES) {
            editor.saved_chars[i] = mvinch(row, col + i) & A_CHARTEXT;
            editor.saved_attrs[i] = mvinch(row, col + i) & A_ATTRIBUTES;
        } else {
            editor.saved_chars[i] = ' ';
            editor.saved_attrs[i] = A_NORMAL;
        }
    }
}

void restore_popup_area() {
    if (editor.popup_width > 0) {
        for (int i = 0; i < editor.popup_width; i++) {
            if (editor.popup_col + i < COLS && editor.popup_row >= 0 && editor.popup_row < LINES) {
                mvaddch(editor.popup_row, editor.popup_col + i, 
                        editor.saved_chars[i] | editor.saved_attrs[i]);
            }
        }
        editor.popup_width = 0;
    }
}

void display_accent_popup(AccentConfig* config, int selected_index) {
    if (!config || editor.cursor_row == 0) return; // Pas assez de place en haut
    
    int popup_row = editor.cursor_row - 1;
    int popup_col = editor.cursor_col;
    int popup_width = config->count;
    
    // Ajuster si on dépasse l'écran
    if (popup_col + popup_width >= COLS) {
        popup_col = COLS - popup_width;
    }
    if (popup_col < 0) popup_col = 0;
    
    // Sauvegarder ce qui est sous la popup
    save_popup_area(popup_row, popup_col, popup_width);
    
    // Afficher les options
    for (int i = 0; i < config->count; i++) {
        if (popup_col + i < COLS) {
            if (i == selected_index) {
                attron(COLOR_PAIR(COLOR_SELECTED));
            } else {
                attron(COLOR_PAIR(COLOR_POPUP_BG));
            }
            
            mvaddch(popup_row, popup_col + i, config->variants[i][0]);
            
            if (i == selected_index) {
                attroff(COLOR_PAIR(COLOR_SELECTED));
            } else {
                attroff(COLOR_PAIR(COLOR_POPUP_BG));
            }
        }
    }
}

void display_editor() {
    clear();
    
    // Afficher les lignes
    for (int i = 0; i < editor.num_lines && i < LINES - 1; i++) {
        mvprintw(i, 0, "%s", editor.lines[i]);
    }
    
    // Si un accent est en cours, afficher le caractère en attente
    if (editor.accent_active) {
        AccentConfig* config = find_accent_config(editor.accent_base);
        if (config) {
            // Afficher le caractère sélectionné en blanc sur noir au curseur
            attron(COLOR_PAIR(COLOR_PENDING));
            mvaddch(editor.cursor_row, editor.cursor_col, config->variants[editor.accent_index][0]);
            attroff(COLOR_PAIR(COLOR_PENDING));
            
            // Afficher la popup avec les options
            display_accent_popup(config, editor.accent_index);
        }
    }
    
    // Afficher les infos en bas
    mvprintw(LINES - 1, 0, "Ligne %d, Col %d | Ctrl+Q: quitter | Alt+lettre: accents", 
             editor.cursor_row + 1, editor.cursor_col + 1);
    
    if (editor.accent_active) {
        mvprintw(LINES - 1, 50, "MODE ACCENT: %c", editor.accent_base);
    }
    
    // Positionner le curseur
    move(editor.cursor_row, editor.cursor_col);
    refresh();
}

void insert_char(char ch) {
    char *line = editor.lines[editor.cursor_row];
    int len = strlen(line);
    
    if (len < MAX_LINE_LENGTH - 1) {
        memmove(&line[editor.cursor_col + 1], &line[editor.cursor_col], 
                len - editor.cursor_col + 1);
        line[editor.cursor_col] = ch;
        editor.cursor_col++;
    }
}

void commit_accent() {
    if (editor.accent_active) {
        AccentConfig* config = find_accent_config(editor.accent_base);
        if (config) {
            // Insérer le caractère accentué sélectionné
            insert_char(config->variants[editor.accent_index][0]);
            // Note: simplification - on prend juste le premier byte UTF-8
        }
        
        // Nettoyer l'état des accents
        restore_popup_area();
        editor.accent_active = 0;
        editor.accent_base = 0;
        editor.accent_index = 0;
    }
}

void handle_accent_key(char ch) {
    AccentConfig* config = find_accent_config(ch);
    if (!config) return;
    
    if (editor.accent_active && editor.accent_base == ch) {
        // Même lettre - cycler
        editor.accent_index = (editor.accent_index + 1) % config->count;
    } else {
        // Nouvelle lettre ou première fois
        if (editor.accent_active) {
            restore_popup_area(); // Nettoyer l'ancienne popup
        }
        editor.accent_active = 1;
        editor.accent_base = ch;
        editor.accent_index = 0;
    }
}

void delete_char() {
    // Annuler le mode accent si actif
    if (editor.accent_active) {
        restore_popup_area();
        editor.accent_active = 0;
        return;
    }
    
    char *line = editor.lines[editor.cursor_row];
    int len = strlen(line);
    
    if (editor.cursor_col > 0) {
        memmove(&line[editor.cursor_col - 1], &line[editor.cursor_col], 
                len - editor.cursor_col + 1);
        editor.cursor_col--;
    } else if (editor.cursor_row > 0) {
        int prev_len = strlen(editor.lines[editor.cursor_row - 1]);
        
        if (prev_len + len < MAX_LINE_LENGTH - 1) {
            strcat(editor.lines[editor.cursor_row - 1], line);
            
            for (int i = editor.cursor_row; i < editor.num_lines - 1; i++) {
                strcpy(editor.lines[i], editor.lines[i + 1]);
            }
            
            editor.num_lines--;
            editor.cursor_row--;
            editor.cursor_col = prev_len;
        }
    }
}

void new_line() {
    // Valider l'accent en cours
    if (editor.accent_active) {
        commit_accent();
        editor.cursor_col--; // Corriger la position après insertion
    }
    
    if (editor.num_lines < MAX_LINES - 1) {
        char *current_line = editor.lines[editor.cursor_row];
        
        for (int i = editor.num_lines; i > editor.cursor_row + 1; i--) {
            strcpy(editor.lines[i], editor.lines[i - 1]);
        }
        
        strcpy(editor.lines[editor.cursor_row + 1], &current_line[editor.cursor_col]);
        current_line[editor.cursor_col] = '\0';
        
        editor.num_lines++;
        editor.cursor_row++;
        editor.cursor_col = 0;
    }
}

void move_cursor(int direction) {
    // Valider l'accent en cours avant de bouger
    if (editor.accent_active) {
        commit_accent();
        editor.cursor_col--; // Corriger la position après insertion
    }
    
    switch (direction) {
        case KEY_LEFT:
            if (editor.cursor_col > 0) {
                editor.cursor_col--;
            } else if (editor.cursor_row > 0) {
                editor.cursor_row--;
                editor.cursor_col = strlen(editor.lines[editor.cursor_row]);
            }
            break;
            
        case KEY_RIGHT: {
            int line_len = strlen(editor.lines[editor.cursor_row]);
            if (editor.cursor_col < line_len) {
                editor.cursor_col++;
            } else if (editor.cursor_row < editor.num_lines - 1) {
                editor.cursor_row++;
                editor.cursor_col = 0;
            }
            break;
        }
        
        case KEY_UP:
            if (editor.cursor_row > 0) {
                editor.cursor_row--;
                int line_len = strlen(editor.lines[editor.cursor_row]);
                if (editor.cursor_col > line_len) {
                    editor.cursor_col = line_len;
                }
            }
            break;
            
        case KEY_DOWN:
            if (editor.cursor_row < editor.num_lines - 1) {
                editor.cursor_row++;
                int line_len = strlen(editor.lines[editor.cursor_row]);
                if (editor.cursor_col > line_len) {
                    editor.cursor_col = line_len;
                }
            }
            break;
    }
}

int main() {
    initscr();
    noecho();
    cbreak();
    keypad(stdscr, TRUE);
    
    init_colors();
    init_editor();
    
    int ch;
    int alt_pressed = 0;
    
    while ((ch = getch()) != 17) { // Ctrl+Q
        // Détecter Alt (ESC suivi d'une touche)
        if (ch == 27) {
            alt_pressed = 1;
            continue;
        }
        
        if (alt_pressed && isalpha(ch)) {
            // Alt + lettre
            handle_accent_key(tolower(ch));
            alt_pressed = 0;
        } else {
            alt_pressed = 0;
            
            switch (ch) {
                case KEY_LEFT:
                case KEY_RIGHT:
                case KEY_UP:
                case KEY_DOWN:
                    move_cursor(ch);
                    break;
                    
                case KEY_BACKSPACE:
                case 127:
                case 8:
                    delete_char();
                    break;
                    
                case '\n':
                case '\r':
                case KEY_ENTER:
                    new_line();
                    break;
                    
                case ' ':
                    if (editor.accent_active) {
                        commit_accent();
                    }
                    insert_char(' ');
                    break;
                    
                default:
                    if (ch >= 32 && ch < 127) {
                        if (editor.accent_active) {
                            commit_accent();
                        }
                        insert_char(ch);
                    }
                    break;
            }
        }
        
        display_editor();
    }
    
    endwin();
    return 0;
}
#include <ncurses.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#define MAX_LINES 1000
#define MAX_LINE_LENGTH 1024
#define MAX_ACCENT_VARIANTS 8

typedef struct {
    char base;
    char variants[MAX_ACCENT_VARIANTS][4]; // UTF-8 jusqu'à 3 bytes + \0
    int count;
} AccentConfig;
#include <ncurses.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#define MAX_LINES 1000
#define MAX_LINE_LENGTH 1024
#define MAX_ACCENT_VARIANTS 8

typedef struct {
    char base;
    char variants[MAX_ACCENT_VARIANTS][4]; // UTF-8 jusqu'à 3 bytes + \0
    int count;
} AccentConfig;

typedef struct {
    char lines[MAX_LINES][MAX_LINE_LENGTH];
    int num_lines;
    int cursor_row;
    int cursor_col;
    
    // État des accents
    int accent_active;
    char accent_base;
    int accent_index;
    
    // Sauvegarde de ce qui est sous l'infobulle
    char saved_chars[MAX_ACCENT_VARIANTS];
    int saved_attrs[MAX_ACCENT_VARIANTS];
    int popup_row;
    int popup_col;
    int popup_width;
} Editor;

Editor editor = {0};

AccentConfig accents[] = {
    {'e', {"é", "è", "ê", "ë"}, 4},
    {'a', {"à", "â", "ä", "æ"}, 4},
    {'u', {"ù", "û", "ü"}, 3},
    {'i', {"î", "ï"}, 2},
    {'o', {"ô", "ö", "œ"}, 3},
    {'c', {"ç"}, 1},
    {'y', {"ÿ"}, 1},
    {'n', {"ñ"}, 1}
};

#define ACCENT_COUNT (sizeof(accents)/sizeof(accents[0]))

// Couleurs
#define COLOR_POPUP_BG 1
#define COLOR_SELECTED 2
#define COLOR_PENDING 3

AccentConfig* find_accent_config(char ch) {
    for (int i = 0; i < ACCENT_COUNT; i++) {
        if (accents[i].base == tolower(ch)) {
            return &accents[i];
        }
    }
    return NULL;
}

void init_colors() {
    start_color();
    init_pair(COLOR_POPUP_BG, COLOR_WHITE, COLOR_BLUE);      // Blanc sur bleu marine
    init_pair(COLOR_SELECTED, COLOR_YELLOW, COLOR_BLUE);     // Jaune sur bleu marine
    init_pair(COLOR_PENDING, COLOR_WHITE, COLOR_BLACK);      // Blanc sur noir
}

void init_editor() {
    editor.num_lines = 1;
    strcpy(editor.lines[0], "");
    editor.cursor_row = 0;
    editor.cursor_col = 0;
    editor.accent_active = 0;
}

void save_popup_area(int row, int col, int width) {
    editor.popup_row = row;
    editor.popup_col = col;
    editor.popup_width = width;
    
    for (int i = 0; i < width; i++) {
        if (col + i < COLS && row >= 0 && row < LINES) {
            editor.saved_chars[i] = mvinch(row, col + i) & A_CHARTEXT;
            editor.saved_attrs[i] = mvinch(row, col + i) & A_ATTRIBUTES;
        } else {
            editor.saved_chars[i] = ' ';
            editor.saved_attrs[i] = A_NORMAL;
        }
    }
}

void restore_popup_area() {
    if (editor.popup_width > 0) {
        for (int i = 0; i < editor.popup_width; i++) {
            if (editor.popup_col + i < COLS && editor.popup_row >= 0 && editor.popup_row < LINES) {
                mvaddch(editor.popup_row, editor.popup_col + i, 
                        editor.saved_chars[i] | editor.saved_attrs[i]);
            }
        }
        editor.popup_width = 0;
    }
}

void display_accent_popup(AccentConfig* config, int selected_index) {
    if (!config || editor.cursor_row == 0) return; // Pas assez de place en haut
    
    int popup_row = editor.cursor_row - 1;
    int popup_col = editor.cursor_col;
    int popup_width = config->count;
    
    // Ajuster si on dépasse l'écran
    if (popup_col + popup_width >= COLS) {
        popup_col = COLS - popup_width;
    }
    if (popup_col < 0) popup_col = 0;
    
    // Sauvegarder ce qui est sous la popup
    save_popup_area(popup_row, popup_col, popup_width);
    
    // Afficher les options
    for (int i = 0; i < config->count; i++) {
        if (popup_col + i < COLS) {
            if (i == selected_index) {
                attron(COLOR_PAIR(COLOR_SELECTED));
            } else {
                attron(COLOR_PAIR(COLOR_POPUP_BG));
            }
            
            mvaddch(popup_row, popup_col + i, config->variants[i][0]);
            
            if (i == selected_index) {
                attroff(COLOR_PAIR(COLOR_SELECTED));
            } else {
                attroff(COLOR_PAIR(COLOR_POPUP_BG));
            }
        }
    }
}

void display_editor() {
    clear();
    
    // Afficher les lignes
    for (int i = 0; i < editor.num_lines && i < LINES - 1; i++) {
        mvprintw(i, 0, "%s", editor.lines[i]);
    }
    
    // Si un accent est en cours, afficher le caractère en attente
    if (editor.accent_active) {
        AccentConfig* config = find_accent_config(editor.accent_base);
        if (config) {
            // Afficher le caractère sélectionné en blanc sur noir au curseur
            attron(COLOR_PAIR(COLOR_PENDING));
            mvaddch(editor.cursor_row, editor.cursor_col, config->variants[editor.accent_index][0]);
            attroff(COLOR_PAIR(COLOR_PENDING));
            
            // Afficher la popup avec les options
            display_accent_popup(config, editor.accent_index);
        }
    }
    
    // Afficher les infos en bas
    mvprintw(LINES - 1, 0, "Ligne %d, Col %d | Ctrl+Q: quitter | Alt+lettre: accents", 
             editor.cursor_row + 1, editor.cursor_col + 1);
    
    if (editor.accent_active) {
        mvprintw(LINES - 1, 50, "MODE ACCENT: %c", editor.accent_base);
    }
    
    // Positionner le curseur
    move(editor.cursor_row, editor.cursor_col);
    refresh();
}

void insert_char(char ch) {
    char *line = editor.lines[editor.cursor_row];
    int len = strlen(line);
    
    if (len < MAX_LINE_LENGTH - 1) {
        memmove(&line[editor.cursor_col + 1], &line[editor.cursor_col], 
                len - editor.cursor_col + 1);
        line[editor.cursor_col] = ch;
        editor.cursor_col++;
    }
}

void commit_accent() {
    if (editor.accent_active) {
        AccentConfig* config = find_accent_config(editor.accent_base);
        if (config) {
            // Insérer le caractère accentué sélectionné
            insert_char(config->variants[editor.accent_index][0]);
            // Note: simplification - on prend juste le premier byte UTF-8
        }
        
        // Nettoyer l'état des accents
        restore_popup_area();
        editor.accent_active = 0;
        editor.accent_base = 0;
        editor.accent_index = 0;
    }
}

void handle_accent_key(char ch) {
    AccentConfig* config = find_accent_config(ch);
    if (!config) return;
    
    if (editor.accent_active && editor.accent_base == ch) {
        // Même lettre - cycler
        editor.accent_index = (editor.accent_index + 1) % config->count;
    } else {
        // Nouvelle lettre ou première fois
        if (editor.accent_active) {
            restore_popup_area(); // Nettoyer l'ancienne popup
        }
        editor.accent_active = 1;
        editor.accent_base = ch;
        editor.accent_index = 0;
    }
}

void delete_char() {
    // Annuler le mode accent si actif
    if (editor.accent_active) {
        restore_popup_area();
        editor.accent_active = 0;
        return;
    }
    
    char *line = editor.lines[editor.cursor_row];
    int len = strlen(line);
    
    if (editor.cursor_col > 0) {
        memmove(&line[editor.cursor_col - 1], &line[editor.cursor_col], 
                len - editor.cursor_col + 1);
        editor.cursor_col--;
    } else if (editor.cursor_row > 0) {
        int prev_len = strlen(editor.lines[editor.cursor_row - 1]);
        
        if (prev_len + len < MAX_LINE_LENGTH - 1) {
            strcat(editor.lines[editor.cursor_row - 1], line);
            
            for (int i = editor.cursor_row; i < editor.num_lines - 1; i++) {
                strcpy(editor.lines[i], editor.lines[i + 1]);
            }
            
            editor.num_lines--;
            editor.cursor_row--;
            editor.cursor_col = prev_len;
        }
    }
}

void new_line() {
    // Valider l'accent en cours
    if (editor.accent_active) {
        commit_accent();
        editor.cursor_col--; // Corriger la position après insertion
    }
    
    if (editor.num_lines < MAX_LINES - 1) {
        char *current_line = editor.lines[editor.cursor_row];
        
        for (int i = editor.num_lines; i > editor.cursor_row + 1; i--) {
            strcpy(editor.lines[i], editor.lines[i - 1]);
        }
        
        strcpy(editor.lines[editor.cursor_row + 1], &current_line[editor.cursor_col]);
        current_line[editor.cursor_col] = '\0';
        
        editor.num_lines++;
        editor.cursor_row++;
        editor.cursor_col = 0;
    }
}

void move_cursor(int direction) {
    // Valider l'accent en cours avant de bouger
    if (editor.accent_active) {
        commit_accent();
        editor.cursor_col--; // Corriger la position après insertion
    }
    
    switch (direction) {
        case KEY_LEFT:
            if (editor.cursor_col > 0) {
                editor.cursor_col--;
            } else if (editor.cursor_row > 0) {
                editor.cursor_row--;
                editor.cursor_col = strlen(editor.lines[editor.cursor_row]);
            }
            break;
            
        case KEY_RIGHT: {
            int line_len = strlen(editor.lines[editor.cursor_row]);
            if (editor.cursor_col < line_len) {
                editor.cursor_col++;
            } else if (editor.cursor_row < editor.num_lines - 1) {
                editor.cursor_row++;
                editor.cursor_col = 0;
            }
            break;
        }
        
        case KEY_UP:
            if (editor.cursor_row > 0) {
                editor.cursor_row--;
                int line_len = strlen(editor.lines[editor.cursor_row]);
                if (editor.cursor_col > line_len) {
                    editor.cursor_col = line_len;
                }
            }
            break;
            
        case KEY_DOWN:
            if (editor.cursor_row < editor.num_lines - 1) {
                editor.cursor_row++;
                int line_len = strlen(editor.lines[editor.cursor_row]);
                if (editor.cursor_col > line_len) {
                    editor.cursor_col = line_len;
                }
            }
            break;
    }
}

int main() {
    initscr();
    noecho();
    cbreak();
    keypad(stdscr, TRUE);
    
    init_colors();
    init_editor();
    
    int ch;
    int alt_pressed = 0;
    
    while ((ch = getch()) != 17) { // Ctrl+Q
        // Détecter Alt (ESC suivi d'une touche)
        if (ch == 27) {
            alt_pressed = 1;
            continue;
        }
        
        if (alt_pressed && isalpha(ch)) {
            // Alt + lettre
            handle_accent_key(tolower(ch));
            alt_pressed = 0;
        } else {
            alt_pressed = 0;
            
            switch (ch) {
                case KEY_LEFT:
                case KEY_RIGHT:
                case KEY_UP:
                case KEY_DOWN:
                    move_cursor(ch);
                    break;
                    
                case KEY_BACKSPACE:
                case 127:
                case 8:
                    delete_char();
                    break;
                    
                case '\n':
                case '\r':
                case KEY_ENTER:
                    new_line();
                    break;
                    
                case ' ':
                    if (editor.accent_active) {
                        commit_accent();
                    }
                    insert_char(' ');
                    break;
                    
                default:
                    if (ch >= 32 && ch < 127) {
                        if (editor.accent_active) {
                            commit_accent();
                        }
                        insert_char(ch);
                    }
                    break;
            }
        }
        
        display_editor();
    }
    
    endwin();
    return 0;
}
typedef struct {
    char lines[MAX_LINES][MAX_LINE_LENGTH];
    int num_lines;
    int cursor_row;
    int cursor_col;
    
    // État des accents
    int accent_active;
    char accent_base;
    int accent_index;
    
    // Sauvegarde de ce qui est sous l'infobulle
    char saved_chars[MAX_ACCENT_VARIANTS];
    int saved_attrs[MAX_ACCENT_VARIANTS];
    int popup_row;
    int popup_col;
    int popup_width;
} Editor;

Editor editor = {0};

AccentConfig accents[] = {
    {'e', {"é", "è", "ê", "ë"}, 4},
    {'a', {"à", "â", "ä", "æ"}, 4},
    {'u', {"ù", "û", "ü"}, 3},
    {'i', {"î", "ï"}, 2},
    {'o', {"ô", "ö", "œ"}, 3},
    {'c', {"ç"}, 1},
    {'y', {"ÿ"}, 1},
    {'n', {"ñ"}, 1}
};

#define ACCENT_COUNT (sizeof(accents)/sizeof(accents[0]))

// Couleurs
#define COLOR_POPUP_BG 1
#define COLOR_SELECTED 2
#define COLOR_PENDING 3

AccentConfig* find_accent_config(char ch) {
    for (int i = 0; i < ACCENT_COUNT; i++) {
        if (accents[i].base == tolower(ch)) {
            return &accents[i];
        }
    }
    return NULL;
}

void init_colors() {
    start_color();
    init_pair(COLOR_POPUP_BG, COLOR_WHITE, COLOR_BLUE);      // Blanc sur bleu marine
    init_pair(COLOR_SELECTED, COLOR_YELLOW, COLOR_BLUE);     // Jaune sur bleu marine
    init_pair(COLOR_PENDING, COLOR_WHITE, COLOR_BLACK);      // Blanc sur noir
}

void init_editor() {
    editor.num_lines = 1;
    strcpy(editor.lines[0], "");
    editor.cursor_row = 0;
    editor.cursor_col = 0;
    editor.accent_active = 0;
}

void save_popup_area(int row, int col, int width) {
    editor.popup_row = row;
    editor.popup_col = col;
    editor.popup_width = width;
    
    for (int i = 0; i < width; i++) {
        if (col + i < COLS && row >= 0 && row < LINES) {
            editor.saved_chars[i] = mvinch(row, col + i) & A_CHARTEXT;
            editor.saved_attrs[i] = mvinch(row, col + i) & A_ATTRIBUTES;
        } else {
            editor.saved_chars[i] = ' ';
            editor.saved_attrs[i] = A_NORMAL;
        }
    }
}

void restore_popup_area() {
    if (editor.popup_width > 0) {
        for (int i = 0; i < editor.popup_width; i++) {
            if (editor.popup_col + i < COLS && editor.popup_row >= 0 && editor.popup_row < LINES) {
                mvaddch(editor.popup_row, editor.popup_col + i, 
                        editor.saved_chars[i] | editor.saved_attrs[i]);
            }
        }
        editor.popup_width = 0;
    }
}

void display_accent_popup(AccentConfig* config, int selected_index) {
    if (!config || editor.cursor_row == 0) return; // Pas assez de place en haut
    
    int popup_row = editor.cursor_row - 1;
    int popup_col = editor.cursor_col;
    int popup_width = config->count;
    
    // Ajuster si on dépasse l'écran
    if (popup_col + popup_width >= COLS) {
        popup_col = COLS - popup_width;
    }
    if (popup_col < 0) popup_col = 0;
    
    // Sauvegarder ce qui est sous la popup
    save_popup_area(popup_row, popup_col, popup_width);
    
    // Afficher les options
    for (int i = 0; i < config->count; i++) {
        if (popup_col + i < COLS) {
            if (i == selected_index) {
                attron(COLOR_PAIR(COLOR_SELECTED));
            } else {
                attron(COLOR_PAIR(COLOR_POPUP_BG));
            }
            
            mvaddch(popup_row, popup_col + i, config->variants[i][0]);
            
            if (i == selected_index) {
                attroff(COLOR_PAIR(COLOR_SELECTED));
            } else {
                attroff(COLOR_PAIR(COLOR_POPUP_BG));
            }
        }
    }
}

void display_editor() {
    clear();
    
    // Afficher les lignes
    for (int i = 0; i < editor.num_lines && i < LINES - 1; i++) {
        mvprintw(i, 0, "%s", editor.lines[i]);
    }
    
    // Si un accent est en cours, afficher le caractère en attente
    if (editor.accent_active) {
        AccentConfig* config = find_accent_config(editor.accent_base);
        if (config) {
            // Afficher le caractère sélectionné en blanc sur noir au curseur
            attron(COLOR_PAIR(COLOR_PENDING));
            mvaddch(editor.cursor_row, editor.cursor_col, config->variants[editor.accent_index][0]);
            attroff(COLOR_PAIR(COLOR_PENDING));
            
            // Afficher la popup avec les options
            display_accent_popup(config, editor.accent_index);
        }
    }
    
    // Afficher les infos en bas
    mvprintw(LINES - 1, 0, "Ligne %d, Col %d | Ctrl+Q: quitter | Alt+lettre: accents", 
             editor.cursor_row + 1, editor.cursor_col + 1);
    
    if (editor.accent_active) {
        mvprintw(LINES - 1, 50, "MODE ACCENT: %c", editor.accent_base);
    }
    
    // Positionner le curseur
    move(editor.cursor_row, editor.cursor_col);
    refresh();
}

void insert_char(char ch) {
    char *line = editor.lines[editor.cursor_row];
    int len = strlen(line);
    
    if (len < MAX_LINE_LENGTH - 1) {
        memmove(&line[editor.cursor_col + 1], &line[editor.cursor_col], 
                len - editor.cursor_col + 1);
        line[editor.cursor_col] = ch;
        editor.cursor_col++;
    }
}

void commit_accent() {
    if (editor.accent_active) {
        AccentConfig* config = find_accent_config(editor.accent_base);
        if (config) {
            // Insérer le caractère accentué sélectionné
            insert_char(config->variants[editor.accent_index][0]);
            // Note: simplification - on prend juste le premier byte UTF-8
        }
        
        // Nettoyer l'état des accents
        restore_popup_area();
        editor.accent_active = 0;
        editor.accent_base = 0;
        editor.accent_index = 0;
    }
}

void handle_accent_key(char ch) {
    AccentConfig* config = find_accent_config(ch);
    if (!config) return;
    
    if (editor.accent_active && editor.accent_base == ch) {
        // Même lettre - cycler
        editor.accent_index = (editor.accent_index + 1) % config->count;
    } else {
        // Nouvelle lettre ou première fois
        if (editor.accent_active) {
            restore_popup_area(); // Nettoyer l'ancienne popup
        }
        editor.accent_active = 1;
        editor.accent_base = ch;
        editor.accent_index = 0;
    }
}

void delete_char() {
    // Annuler le mode accent si actif
    if (editor.accent_active) {
        restore_popup_area();
        editor.accent_active = 0;
        return;
    }
    
    char *line = editor.lines[editor.cursor_row];
    int len = strlen(line);
    
    if (editor.cursor_col > 0) {
        memmove(&line[editor.cursor_col - 1], &line[editor.cursor_col], 
                len - editor.cursor_col + 1);
        editor.cursor_col--;
    } else if (editor.cursor_row > 0) {
        int prev_len = strlen(editor.lines[editor.cursor_row - 1]);
        
        if (prev_len + len < MAX_LINE_LENGTH - 1) {
            strcat(editor.lines[editor.cursor_row - 1], line);
            
            for (int i = editor.cursor_row; i < editor.num_lines - 1; i++) {
                strcpy(editor.lines[i], editor.lines[i + 1]);
            }
            
            editor.num_lines--;
            editor.cursor_row--;
            editor.cursor_col = prev_len;
        }
    }
}

void new_line() {
    // Valider l'accent en cours
    if (editor.accent_active) {
        commit_accent();
        editor.cursor_col--; // Corriger la position après insertion
    }
    
    if (editor.num_lines < MAX_LINES - 1) {
        char *current_line = editor.lines[editor.cursor_row];
        
        for (int i = editor.num_lines; i > editor.cursor_row + 1; i--) {
            strcpy(editor.lines[i], editor.lines[i - 1]);
        }
        
        strcpy(editor.lines[editor.cursor_row + 1], &current_line[editor.cursor_col]);
        current_line[editor.cursor_col] = '\0';
        
        editor.num_lines++;
        editor.cursor_row++;
        editor.cursor_col = 0;
    }
}

void move_cursor(int direction) {
    // Valider l'accent en cours avant de bouger
    if (editor.accent_active) {
        commit_accent();
        editor.cursor_col--; // Corriger la position après insertion
    }
    
    switch (direction) {
        case KEY_LEFT:
            if (editor.cursor_col > 0) {
                editor.cursor_col--;
            } else if (editor.cursor_row > 0) {
                editor.cursor_row--;
                editor.cursor_col = strlen(editor.lines[editor.cursor_row]);
            }
            break;
            
        case KEY_RIGHT: {
            int line_len = strlen(editor.lines[editor.cursor_row]);
            if (editor.cursor_col < line_len) {
                editor.cursor_col++;
            } else if (editor.cursor_row < editor.num_lines - 1) {
                editor.cursor_row++;
                editor.cursor_col = 0;
            }
            break;
        }
        
        case KEY_UP:
            if (editor.cursor_row > 0) {
                editor.cursor_row--;
                int line_len = strlen(editor.lines[editor.cursor_row]);
                if (editor.cursor_col > line_len) {
                    editor.cursor_col = line_len;
                }
            }
            break;
            
        case KEY_DOWN:
            if (editor.cursor_row < editor.num_lines - 1) {
                editor.cursor_row++;
                int line_len = strlen(editor.lines[editor.cursor_row]);
                if (editor.cursor_col > line_len) {
                    editor.cursor_col = line_len;
                }
            }
            break;
    }
}

int main() {
    initscr();
    noecho();
    cbreak();
    keypad(stdscr, TRUE);
    
    init_colors();
    init_editor();
    
    int ch;
    int alt_pressed = 0;
    
    while ((ch = getch()) != 17) { // Ctrl+Q
        // Détecter Alt (ESC suivi d'une touche)
        if (ch == 27) {
            alt_pressed = 1;
            continue;
        }
        
        if (alt_pressed && isalpha(ch)) {
            // Alt + lettre
            handle_accent_key(tolower(ch));
            alt_pressed = 0;
        } else {
            alt_pressed = 0;
            
            switch (ch) {
                case KEY_LEFT:
                case KEY_RIGHT:
                case KEY_UP:
                case KEY_DOWN:
                    move_cursor(ch);
                    break;
                    
                case KEY_BACKSPACE:
                case 127:
                case 8:
                    delete_char();
                    break;
                    
                case '\n':
                case '\r':
                case KEY_ENTER:
                    new_line();
                    break;
                    
                case ' ':
                    if (editor.accent_active) {
                        commit_accent();
                    }
                    insert_char(' ');
                    break;
                    
                default:
                    if (ch >= 32 && ch < 127) {
                        if (editor.accent_active) {
                            commit_accent();
                        }
                        insert_char(ch);
                    }
                    break;
            }
        }
        
        display_editor();
    }
    
    endwin();
    return 0;
}

AccentConfig* find_accent_config(char ch) {
    for (int i = 0; i < ACCENT_COUNT; i++) {
        if (accents[i].base == tolower(ch)) {
            return &accents[i];
        }
    }
    return NULL;
}

void init_colors() {
    start_color();
    init_pair(COLOR_POPUP_BG, COLOR_WHITE, COLOR_BLUE);      // Blanc sur bleu marine
    init_pair(COLOR_SELECTED, COLOR_YELLOW, COLOR_BLUE);     // Jaune sur bleu marine
    init_pair(COLOR_PENDING, COLOR_WHITE, COLOR_BLACK);      // Blanc sur noir
}

void init_editor() {
    editor.num_lines = 1;
    strcpy(editor.lines[0], "");
    editor.cursor_row = 0;
    editor.cursor_col = 0;
    editor.accent_active = 0;
}

void save_popup_area(int row, int col, int width) {
    editor.popup_row = row;
    editor.popup_col = col;
    editor.popup_width = width;
    
    for (int i = 0; i < width; i++) {
        if (col + i < COLS && row >= 0 && row < LINES) {
            editor.saved_chars[i] = mvinch(row, col + i) & A_CHARTEXT;
            editor.saved_attrs[i] = mvinch(row, col + i) & A_ATTRIBUTES;
        } else {
            editor.saved_chars[i] = ' ';
            editor.saved_attrs[i] = A_NORMAL;
        }
    }
}

void restore_popup_area() {
    if (editor.popup_width > 0) {
        for (int i = 0; i < editor.popup_width; i++) {
            if (editor.popup_col + i < COLS && editor.popup_row >= 0 && editor.popup_row < LINES) {
                mvaddch(editor.popup_row, editor.popup_col + i, 
                        editor.saved_chars[i] | editor.saved_attrs[i]);
            }
        }
        editor.popup_width = 0;
    }
}

void display_accent_popup(AccentConfig* config, int selected_index) {
    if (!config || editor.cursor_row == 0) return; // Pas assez de place en haut
    
    int popup_row = editor.cursor_row - 1;
    int popup_col = editor.cursor_col;
    int popup_width = config->count;
    
    // Ajuster si on dépasse l'écran
    if (popup_col + popup_width >= COLS) {
        popup_col = COLS - popup_width;
    }
    if (popup_col < 0) popup_col = 0;
    
    // Sauvegarder ce qui est sous la popup
    save_popup_area(popup_row, popup_col, popup_width);
    
    // Afficher les options
    for (int i = 0; i < config->count; i++) {
        if (popup_col + i < COLS) {
            if (i == selected_index) {
                attron(COLOR_PAIR(COLOR_SELECTED));
            } else {
                attron(COLOR_PAIR(COLOR_POPUP_BG));
            }
            
            mvaddch(popup_row, popup_col + i, config->variants[i][0]);
            
            if (i == selected_index) {
                attroff(COLOR_PAIR(COLOR_SELECTED));
            } else {
                attroff(COLOR_PAIR(COLOR_POPUP_BG));
            }
        }
    }
}

void display_editor() {
    clear();
    
    // Afficher les lignes
    for (int i = 0; i < editor.num_lines && i < LINES - 1; i++) {
        mvprintw(i, 0, "%s", editor.lines[i]);
    }
    
    // Si un accent est en cours, afficher le caractère en attente
    if (editor.accent_active) {
        AccentConfig* config = find_accent_config(editor.accent_base);
        if (config) {
            // Afficher le caractère sélectionné en blanc sur noir au curseur
            attron(COLOR_PAIR(COLOR_PENDING));
            mvaddch(editor.cursor_row, editor.cursor_col, config->variants[editor.accent_index][0]);
            attroff(COLOR_PAIR(COLOR_PENDING));
            
            // Afficher la popup avec les options
            display_accent_popup(config, editor.accent_index);
        }
    }
    
    // Afficher les infos en bas
    mvprintw(LINES - 1, 0, "Ligne %d, Col %d | Ctrl+Q: quitter | Alt+lettre: accents", 
             editor.cursor_row + 1, editor.cursor_col + 1);
    
    if (editor.accent_active) {
        mvprintw(LINES - 1, 50, "MODE ACCENT: %c", editor.accent_base);
    }
    
    // Positionner le curseur
    move(editor.cursor_row, editor.cursor_col);
    refresh();
}

void insert_char(char ch) {
    char *line = editor.lines[editor.cursor_row];
    int len = strlen(line);
    
    if (len < MAX_LINE_LENGTH - 1) {
        memmove(&line[editor.cursor_col + 1], &line[editor.cursor_col], 
                len - editor.cursor_col + 1);
        line[editor.cursor_col] = ch;
        editor.cursor_col++;
    }
}

void commit_accent() {
    if (editor.accent_active) {
        AccentConfig* config = find_accent_config(editor.accent_base);
        if (config) {
            // Insérer le caractère accentué sélectionné
            insert_char(config->variants[editor.accent_index][0]);
            // Note: simplification - on prend juste le premier byte UTF-8
        }
        
        // Nettoyer l'état des accents
        restore_popup_area();
        editor.accent_active = 0;
        editor.accent_base = 0;
        editor.accent_index = 0;
    }
}

void handle_accent_key(char ch) {
    AccentConfig* config = find_accent_config(ch);
    if (!config) return;
    
    if (editor.accent_active && editor.accent_base == ch) {
        // Même lettre - cycler
        editor.accent_index = (editor.accent_index + 1) % config->count;
    } else {
        // Nouvelle lettre ou première fois
        if (editor.accent_active) {
            restore_popup_area(); // Nettoyer l'ancienne popup
        }
        editor.accent_active = 1;
        editor.accent_base = ch;
        editor.accent_index = 0;
    }
}

void delete_char() {
    // Annuler le mode accent si actif
    if (editor.accent_active) {
        restore_popup_area();
        editor.accent_active = 0;
        return;
    }
    
    char *line = editor.lines[editor.cursor_row];
    int len = strlen(line);
    
    if (editor.cursor_col > 0) {
        memmove(&line[editor.cursor_col - 1], &line[editor.cursor_col], 
                len - editor.cursor_col + 1);
        editor.cursor_col--;
    } else if (editor.cursor_row > 0) {
        int prev_len = strlen(editor.lines[editor.cursor_row - 1]);
        
        if (prev_len + len < MAX_LINE_LENGTH - 1) {
            strcat(editor.lines[editor.cursor_row - 1], line);
            
            for (int i = editor.cursor_row; i < editor.num_lines - 1; i++) {
                strcpy(editor.lines[i], editor.lines[i + 1]);
            }
            
            editor.num_lines--;
            editor.cursor_row--;
            editor.cursor_col = prev_len;
        }
    }
}

void new_line() {
    // Valider l'accent en cours
    if (editor.accent_active) {
        commit_accent();
        editor.cursor_col--; // Corriger la position après insertion
    }
    
    if (editor.num_lines < MAX_LINES - 1) {
        char *current_line = editor.lines[editor.cursor_row];
        
        for (int i = editor.num_lines; i > editor.cursor_row + 1; i--) {
            strcpy(editor.lines[i], editor.lines[i - 1]);
        }
        
        strcpy(editor.lines[editor.cursor_row + 1], &current_line[editor.cursor_col]);
        current_line[editor.cursor_col] = '\0';
        
        editor.num_lines++;
        editor.cursor_row++;
        editor.cursor_col = 0;
    }
}

void move_cursor(int direction) {
    // Valider l'accent en cours avant de bouger
    if (editor.accent_active) {
        commit_accent();
        editor.cursor_col--; // Corriger la position après insertion
    }
    
    switch (direction) {
        case KEY_LEFT:
            if (editor.cursor_col > 0) {
                editor.cursor_col--;
            } else if (editor.cursor_row > 0) {
                editor.cursor_row--;
                editor.cursor_col = strlen(editor.lines[editor.cursor_row]);
            }
            break;
            
        case KEY_RIGHT: {
            int line_len = strlen(editor.lines[editor.cursor_row]);
            if (editor.cursor_col < line_len) {
                editor.cursor_col++;
            } else if (editor.cursor_row < editor.num_lines - 1) {
                editor.cursor_row++;
                editor.cursor_col = 0;
            }
            break;
        }
        
        case KEY_UP:
            if (editor.cursor_row > 0) {
                editor.cursor_row--;
                int line_len = strlen(editor.lines[editor.cursor_row]);
                if (editor.cursor_col > line_len) {
                    editor.cursor_col = line_len;
                }
            }
            break;
            
        case KEY_DOWN:
            if (editor.cursor_row < editor.num_lines - 1) {
                editor.cursor_row++;
                int line_len = strlen(editor.lines[editor.cursor_row]);
                if (editor.cursor_col > line_len) {
                    editor.cursor_col = line_len;
                }
            }
            break;
    }
}

int main() {
    initscr();
    noecho();
    cbreak();
    keypad(stdscr, TRUE);
    
    init_colors();
    init_editor();
    
    int ch;
    int alt_pressed = 0;
    
    while ((ch = getch()) != 17) { // Ctrl+Q
        // Détecter Alt (ESC suivi d'une touche)
        if (ch == 27) {
            alt_pressed = 1;
            continue;
        }
        
        if (alt_pressed && isalpha(ch)) {
            // Alt + lettre
            handle_accent_key(tolower(ch));
            alt_pressed = 0;
        } else {
            alt_pressed = 0;
            
            switch (ch) {
                case KEY_LEFT:
                case KEY_RIGHT:
                case KEY_UP:
                case KEY_DOWN:
                    move_cursor(ch);
                    break;
                    
                case KEY_BACKSPACE:
                case 127:
                case 8:
                    delete_char();
                    break;
                    
                case '\n':
                case '\r':
                case KEY_ENTER:
                    new_line();
                    break;
                    
                case ' ':
                    if (editor.accent_active) {
                        commit_accent();
                    }
                    insert_char(' ');
                    break;
                    
                default:
                    if (ch >= 32 && ch < 127) {
                        if (editor.accent_active) {
                            commit_accent();
                        }
                        insert_char(ch);
                    }
                    break;
            }
        }
        
        display_editor();
    }
    
    endwin();
    return 0;
}

typedef struct s_accent_config
{
	char	base;
	char	variants[MAX_ACCENT_VARIANTS][4];
	int		count;
}	t_accent_config;

typedef struct s_editor
{
	char	lines[MAX_LINES][MAX_LINE_LENGTH];
	int		num_lines;
	int		cursor_row;
	int		cursor_col;
	int		accent_active;
	char	accent_base;
	int		accent_index;
	char	saved_chars[MAX_ACCENT_VARIANTS];
	int		saved_attrs[MAX_ACCENT_VARIANTS];
	int		popup_row;
	int		popup_col;
	int		popup_width;
}	t_editor;

t_editor		g_editor = {0};

t_accent_config	g_accents[ACCENT_COUNT] = {
	{'e', {"é", "è", "ê", "ë"}, 4},
	{'a', {"à", "â", "ä", "æ"}, 4},
	{'u', {"ù", "û", "ü"}, 3},
	{'i', {"î", "ï"}, 2},
	{'o', {"ô", "ö", "œ"}, 3},
	{'c', {"ç"}, 1},
	{'y', {"ÿ"}, 1},
	{'n', {"ñ"}, 1}
};

t_accent_config	*ft_find_accent_config(char ch)
{
	int	i;

	i = 0;
	while (i < ACCENT_COUNT)
	{
		if (g_accents[i].base == tolower(ch))
			return (&g_accents[i]);
		i++;
	}
	return (NULL);
}

void	ft_init_colors(void)
{
	start_color();
	init_pair(COLOR_POPUP_BG, COLOR_WHITE, COLOR_BLUE);
	init_pair(COLOR_SELECTED, COLOR_YELLOW, COLOR_BLUE);
	init_pair(COLOR_PENDING, COLOR_WHITE, COLOR_BLACK);
}

void	ft_init_editor(void)
{
	g_editor.num_lines = 1;
	strcpy(g_editor.lines[0], "");
	g_editor.cursor_row = 0;
	g_editor.cursor_col = 0;
	g_editor.accent_active = 0;
}

void	ft_save_popup_area(int row, int col, int width)
{
	int	i;

	g_editor.popup_row = row;
	g_editor.popup_col = col;
	g_editor.popup_width = width;
	i = 0;
	while (i < width)
	{
		if (col + i < COLS && row >= 0 && row < LINES)
		{
			g_editor.saved_chars[i] = mvinch(row, col + i) & A_CHARTEXT;
			g_editor.saved_attrs[i] = mvinch(row, col + i) & A_ATTRIBUTES;
		}
		else
		{
			g_editor.saved_chars[i] = ' ';
			g_editor.saved_attrs[i] = A_NORMAL;
		}
		i++;
	}
}

void	ft_restore_popup_area(void)
{
	int	i;

	if (g_editor.popup_width > 0)
	{
		i = 0;
		while (i < g_editor.popup_width)
		{
			if (g_editor.popup_col + i < COLS && g_editor.popup_row >= 0
				&& g_editor.popup_row < LINES)
			{
				mvaddch(g_editor.popup_row, g_editor.popup_col + i,
					g_editor.saved_chars[i] | g_editor.saved_attrs[i]);
			}
			i++;
		}
		g_editor.popup_width = 0;
	}
}

void	ft_display_accent_popup(t_accent_config *config, int selected_index)
{
	int	popup_row;
	int	popup_col;
	int	popup_width;
	int	i;

	if (!config || g_editor.cursor_row == 0)
		return ;
	popup_row = g_editor.cursor_row - 1;
	popup_col = g_editor.cursor_col;
	popup_width = config->count;
	if (popup_col + popup_width >= COLS)
		popup_col = COLS - popup_width;
	if (popup_col < 0)
		popup_col = 0;
	ft_save_popup_area(popup_row, popup_col, popup_width);
	i = 0;
	while (i < config->count)
	{
		if (popup_col + i < COLS)
		{
			if (i == selected_index)
				attron(COLOR_PAIR(COLOR_SELECTED));
			else
				attron(COLOR_PAIR(COLOR_POPUP_BG));
			mvaddch(popup_row, popup_col + i, config->variants[i][0]);
			if (i == selected_index)
				attroff(COLOR_PAIR(COLOR_SELECTED));
			else
				attroff(COLOR_PAIR(COLOR_POPUP_BG));
		}
		i++;
	}
}

void	ft_display_editor(void)
{
	int				i;
	t_accent_config	*config;

	clear();
	i = 0;
	while (i < g_editor.num_lines && i < LINES - 1)
	{
		mvprintw(i, 0, "%s", g_editor.lines[i]);
		i++;
	}
	if (g_editor.accent_active)
	{
		config = ft_find_accent_config(g_editor.accent_base);
		if (config)
		{
			attron(COLOR_PAIR(COLOR_PENDING));
			mvaddch(g_editor.cursor_row, g_editor.cursor_col,
				config->variants[g_editor.accent_index][0]);
			attroff(COLOR_PAIR(COLOR_PENDING));
			ft_display_accent_popup(config, g_editor.accent_index);
		}
	}
	mvprintw(LINES - 1, 0, "Ligne %d, Col %d | Ctrl+Q: quitter | Alt+lettre: accents",
		g_editor.cursor_row + 1, g_editor.cursor_col + 1);
	if (g_editor.accent_active)
		mvprintw(LINES - 1, 50, "MODE ACCENT: %c", g_editor.accent_base);
	move(g_editor.cursor_row, g_editor.cursor_col);
	refresh();
}

void	ft_insert_char(char ch)
{
	char	*line;
	int		len;

	line = g_editor.lines[g_editor.cursor_row];
	len = strlen(line);
	if (len < MAX_LINE_LENGTH - 1)
	{
		memmove(&line[g_editor.cursor_col + 1], &line[g_editor.cursor_col],
			len - g_editor.cursor_col + 1);
		line[g_editor.cursor_col] = ch;
		g_editor.cursor_col++;
	}
}

void	ft_commit_accent(void)
{
	t_accent_config	*config;

	if (g_editor.accent_active)
	{
		config = ft_find_accent_config(g_editor.accent_base);
		if (config)
			ft_insert_char(config->variants[g_editor.accent_index][0]);
		ft_restore_popup_area();
		g_editor.accent_active = 0;
		g_editor.accent_base = 0;
		g_editor.accent_index = 0;
	}
}

void	ft_handle_accent_key(char ch)
{
	t_accent_config	*config;

	config = ft_find_accent_config(ch);
	if (!config)
		return ;
	if (g_editor.accent_active && g_editor.accent_base == ch)
		g_editor.accent_index = (g_editor.accent_index + 1) % config->count;
	else
	{
		if (g_editor.accent_active)
			ft_restore_popup_area();
		g_editor.accent_active = 1;
		g_editor.accent_base = ch;
		g_editor.accent_index = 0;
	}
}

void	ft_delete_char(void)
{
	char	*line;
	int		len;
	int		prev_len;
	int		i;

	if (g_editor.accent_active)
	{
		ft_restore_popup_area();
		g_editor.accent_active = 0;
		return ;
	}
	line = g_editor.lines[g_editor.cursor_row];
	len = strlen(line);
	if (g_editor.cursor_col > 0)
	{
		memmove(&line[g_editor.cursor_col - 1], &line[g_editor.cursor_col],
			len - g_editor.cursor_col + 1);
		g_editor.cursor_col--;
	}
	else if (g_editor.cursor_row > 0)
	{
		prev_len = strlen(g_editor.lines[g_editor.cursor_row - 1]);
		if (prev_len + len < MAX_LINE_LENGTH - 1)
		{
			strcat(g_editor.lines[g_editor.cursor_row - 1], line);
			i = g_editor.cursor_row;
			while (i < g_editor.num_lines - 1)
			{
				strcpy(g_editor.lines[i], g_editor.lines[i + 1]);
				i++;
			}
			g_editor.num_lines--;
			g_editor.cursor_row--;
			g_editor.cursor_col = prev_len;
		}
	}
}

void	ft_new_line(void)
{
	char	*current_line;
	int		i;

	if (g_editor.accent_active)
	{
		ft_commit_accent();
		g_editor.cursor_col--;
	}
	if (g_editor.num_lines < MAX_LINES - 1)
	{
		current_line = g_editor.lines[g_editor.cursor_row];
		i = g_editor.num_lines;
		while (i > g_editor.cursor_row + 1)
		{
			strcpy(g_editor.lines[i], g_editor.lines[i - 1]);
			i--;
		}
		strcpy(g_editor.lines[g_editor.cursor_row + 1],
			&current_line[g_editor.cursor_col]);
		current_line[g_editor.cursor_col] = '\0';
		g_editor.num_lines++;
		g_editor.cursor_row++;
		g_editor.cursor_col = 0;
	}
}

void	ft_move_cursor(int direction)
{
	int	line_len;

	if (g_editor.accent_active)
	{
		ft_commit_accent();
		g_editor.cursor_col--;
	}
	if (direction == KEY_LEFT)
	{
		if (g_editor.cursor_col > 0)
			g_editor.cursor_col--;
		else if (g_editor.cursor_row > 0)
		{
			g_editor.cursor_row--;
			g_editor.cursor_col = strlen(g_editor.lines[g_editor.cursor_row]);
		}
	}
	else if (direction == KEY_RIGHT)
	{
		line_len = strlen(g_editor.lines[g_editor.cursor_row]);
		if (g_editor.cursor_col < line_len)
			g_editor.cursor_col++;
		else if (g_editor.cursor_row < g_editor.num_lines - 1)
		{
			g_editor.cursor_row++;
			g_editor.cursor_col = 0;
		}
	}
	else if (direction == KEY_UP)
	{
		if (g_editor.cursor_row > 0)
		{
			g_editor.cursor_row--;
			line_len = strlen(g_editor.lines[g_editor.cursor_row]);
			if (g_editor.cursor_col > line_len)
				g_editor.cursor_col = line_len;
		}
	}
	else if (direction == KEY_DOWN)
	{
		if (g_editor.cursor_row < g_editor.num_lines - 1)
		{
			g_editor.cursor_row++;
			line_len = strlen(g_editor.lines[g_editor.cursor_row]);
			if (g_editor.cursor_col > line_len)
				g_editor.cursor_col = line_len;
		}
	}
}

int	main(void)
{
	int	ch;
	int	alt_pressed;

	initscr();
	noecho();
	cbreak();
	keypad(stdscr, 1);
	ft_init_colors();
	ft_init_editor();
	alt_pressed = 0;
	while ((ch = getch()) != 17)
	{
		if (ch == 27)
		{
			alt_pressed = 1;
			continue ;
		}
		if (alt_pressed && isalpha(ch))
		{
			ft_handle_accent_key(tolower(ch));
			alt_pressed = 0;
		}
		else
		{
			alt_pressed = 0;
			if (ch == KEY_LEFT || ch == KEY_RIGHT || ch == KEY_UP || ch == KEY_DOWN)
				ft_move_cursor(ch);
			else if (ch == KEY_BACKSPACE || ch == 127 || ch == 8)
				ft_delete_char();
			else if (ch == '\n' || ch == '\r' || ch == KEY_ENTER)
				ft_new_line();
			else if (ch == ' ')
			{
				if (g_editor.accent_active)
					ft_commit_accent();
				ft_insert_char(' ');
			}
			else if (ch >= 32 && ch < 127)
			{
				if (g_editor.accent_active)
					ft_commit_accent();
				ft_insert_char(ch);
			}
		}
		ft_display_editor();
	}
	endwin();
	return (0);
}