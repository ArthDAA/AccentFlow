
const ACCENTS_MAP = {
    'e': ['é', 'è', 'ê', 'ë'], 'E': ['É', 'È', 'Ê', 'Ë'],
    'a': ['à', 'â', 'æ', 'ä'], 'A': ['À', 'Â', 'Æ', 'Ä'],
    'u': ['ù', 'û', 'ü'], 'U': ['Ù', 'Û', 'Ü'],
    'i': ['î', 'ï'], 'I': ['Î', 'Ï'],
    'o': ['ô', 'œ', 'ö'], 'O': ['Ô', 'Œ', 'Ö'],
    'c': ['ç'], 'C': ['Ç'],
    'y': ['ÿ'], 'Y': ['Ÿ']
};

const ALL_ACCENT_KEYS = Object.keys(ACCENTS_MAP);

class AccentFlow {
    constructor() {
        this.isAltHeld = false;
        this.popupElement = null;
        this.accentState = {
            indices: {},
            lastBaseKey: null,
            activeElement: null,
            activeNode: null,
            originalValue: null
        };
        this.init();
    }

    init() {
        this.createStyles();
        this.attachEventListeners();
        this.createPopup();
    }

    createStyles() {
        if (document.getElementById('accentflow-styles')) return;
        
        const styles = document.createElement('style');
        styles.id = 'accentflow-styles';
        styles.textContent = `
            .accentflow-highlight {
                background-color: #111827 !important;
                color: white !important;
                border-radius: 3px;
                padding: 0 1px;
                display: inline;
            }
            
            .accentflow-popup {
                position: absolute;
                background: white;
                border: 2px solid #e5e7eb;
                border-radius: 8px;
                padding: 8px;
                box-shadow: 0 20px 25px -5px rgba(0, 0, 0, 0.1), 0 10px 10px -5px rgba(0, 0, 0, 0.04);
                z-index: 999999;
                display: none;
                pointer-events: none;
            }
            
            .accentflow-popup-options {
                display: flex;
                gap: 4px;
            }
            
            .accentflow-option {
                width: 32px;
                height: 32px;
                display: flex;
                align-items: center;
                justify-content: center;
                font-size: 18px;
                font-weight: bold;
                border-radius: 6px;
                transition: all 0.2s;
                background: #f9fafb;
                color: #374151;
            }
            
            .accentflow-option.selected {
                background: #3b82f6;
                color: white;
            }
            
            .accentflow-popup::after {
                content: '';
                position: absolute;
                top: 100%;
                left: 50%;
                transform: translateX(-50%);
                width: 0;
                height: 0;
                border-left: 8px solid transparent;
                border-right: 8px solid transparent;
                border-top: 8px solid #e5e7eb;
            }
            
            .accentflow-badge {
                position: fixed;
                top: 20px;
                right: 20px;
                background: #6b7280;
                color: white;
                padding: 6px 12px;
                border-radius: 16px;
                font-size: 12px;
                font-weight: 500;
                z-index: 999999;
                display: none;
                box-shadow: 0 4px 6px -1px rgba(0, 0, 0, 0.1);
            }
            
            .accentflow-badge.active {
                background: #dc2626;
            }
        `;
        document.head.appendChild(styles);
    }

    createPopup() {
        this.popupElement = document.createElement('div');
        this.popupElement.className = 'accentflow-popup';
        this.popupElement.innerHTML = '<div class="accentflow-popup-options"></div>';
        document.body.appendChild(this.popupElement);

        this.badgeElement = document.createElement('div');
        this.badgeElement.className = 'accentflow-badge';
        this.badgeElement.textContent = 'Mode Accent';
        document.body.appendChild(this.badgeElement);
    }

    attachEventListeners() {
        document.addEventListener('keydown', this.handleKeyDown.bind(this));
        document.addEventListener('keyup', this.handleKeyUp.bind(this));
        window.addEventListener('blur', this.resetState.bind(this));
    }

    handleKeyDown(event) {
        if (event.key === 'Alt') {
            this.isAltHeld = true;
            this.showBadge();
            return;
        }

        if (this.isAltHeld && ALL_ACCENT_KEYS.includes(event.key)) {
            event.preventDefault();
            event.stopPropagation();
            this.handleAccentKey(event.key, event.target);
        }
    }

    handleKeyUp(event) {
        if (event.key === 'Alt') {
            this.isAltHeld = false;
            this.hideBadge();
            this.commitAccent();
        }
    }

    handleAccentKey(baseKey, target) {
        const isNewLetter = this.accentState.lastBaseKey !== baseKey;

        if (isNewLetter) {
            this.resetAccentState();
            this.accentState.lastBaseKey = baseKey;
            this.accentState.indices[baseKey] = 0;
            this.accentState.activeElement = target;
            
            this.insertAccentedChar(baseKey, target);
        } else {
            this.accentState.indices[baseKey] = (this.accentState.indices[baseKey] + 1) % ACCENTS_MAP[baseKey].length;
            this.updateAccentedChar(baseKey, target);
        }

        this.showPopup(baseKey, target);
    }

    insertAccentedChar(baseKey, target) {
        const variants = ACCENTS_MAP[baseKey];
        const accentChar = variants[this.accentState.indices[baseKey]];

        if (this.isContentEditable(target)) {
            this.insertInContentEditable(accentChar, target);
        } else if (this.isInputField(target)) {
            this.insertInInputField(accentChar, target);
        }
    }

    updateAccentedChar(baseKey, target) {
        const variants = ACCENTS_MAP[baseKey];
        const accentChar = variants[this.accentState.indices[baseKey]];

        if (this.isContentEditable(target)) {
            this.updateContentEditable(accentChar);
        } else if (this.isInputField(target)) {
            this.updateInputField(accentChar, target);
        }
    }

    insertInContentEditable(char, target) {
        const selection = window.getSelection();
        const range = selection.getRangeAt(0);
        
        const span = document.createElement('span');
        span.className = 'accentflow-highlight';
        span.textContent = char;
        
        range.deleteContents();
        range.insertNode(span);
        
        range.setStartAfter(span);
        range.collapse(true);
        selection.removeAllRanges();
        selection.addRange(range);
        
        this.accentState.activeNode = span;
    }

    updateContentEditable(char) {
        if (this.accentState.activeNode) {
            this.accentState.activeNode.textContent = char;
        }
    }

    insertInInputField(char, target) {
        const start = target.selectionStart;
        const end = target.selectionEnd;
        const value = target.value;
        
        this.accentState.originalValue = {
            value: value,
            start: start,
            end: end
        };
        
        target.value = value.slice(0, start) + char + value.slice(end);
        target.selectionStart = target.selectionEnd = start + 1;
        
        target.style.backgroundColor = '#111827';
        target.style.color = 'white';
    }

    updateInputField(char, target) {
        if (this.accentState.originalValue) {
            const { value, start, end } = this.accentState.originalValue;
            target.value = value.slice(0, start) + char + value.slice(end);
            target.selectionStart = target.selectionEnd = start + 1;
        }
    }

    showPopup(baseKey, target) {
        const variants = ACCENTS_MAP[baseKey];
        const currentIndex = this.accentState.indices[baseKey];
        
        const optionsHtml = variants.map((variant, index) => 
            `<div class="accentflow-option ${index === currentIndex ? 'selected' : ''}">${variant}</div>`
        ).join('');
        
        this.popupElement.querySelector('.accentflow-popup-options').innerHTML = optionsHtml;
        
        const rect = target.getBoundingClientRect();
        this.popupElement.style.left = `${rect.left + window.scrollX}px`;
        this.popupElement.style.top = `${rect.top + window.scrollY - 60}px`;
        this.popupElement.style.display = 'block';

        this.badgeElement.textContent = 'Sélection';
        this.badgeElement.classList.add('active');
    }

    hidePopup() {
        if (this.popupElement) {
            this.popupElement.style.display = 'none';
        }
    }

    showBadge() {
        this.badgeElement.style.display = 'block';
        this.badgeElement.textContent = 'Mode Prêt';
        this.badgeElement.classList.remove('active');
    }

    hideBadge() {
        this.badgeElement.style.display = 'none';
    }

    commitAccent() {
        if (this.accentState.activeNode) {
            // Pour contentEditable, remplacer le span par du texte normal
            const textNode = document.createTextNode(this.accentState.activeNode.textContent);
            this.accentState.activeNode.parentNode.replaceChild(textNode, this.accentState.activeNode);
            
            // Repositionner le curseur
            const selection = window.getSelection();
            const range = document.createRange();
            range.setStartAfter(textNode);
            range.collapse(true);
            selection.removeAllRanges();
            selection.addRange(range);
        }
        
        if (this.accentState.activeElement && this.isInputField(this.accentState.activeElement)) {
            // Pour les input fields, restaurer les styles
            this.accentState.activeElement.style.backgroundColor = '';
            this.accentState.activeElement.style.color = '';
        }
        
        this.hidePopup();
        this.resetAccentState();
    }

    resetAccentState() {
        this.accentState = {
            indices: {},
            lastBaseKey: null,
            activeElement: null,
            activeNode: null,
            originalValue: null
        };
    }

    resetState() {
        this.isAltHeld = false;
        this.hideBadge();
        this.hidePopup();
        this.resetAccentState();
    }

    isContentEditable(element) {
        return element.contentEditable === 'true' || element.isContentEditable;
    }

    isInputField(element) {
        return element.tagName === 'INPUT' || element.tagName === 'TEXTAREA';
    }
}

// Initialiser AccentFlow quand le DOM est prêt
if (document.readyState === 'loading') {
    document.addEventListener('DOMContentLoaded', () => new AccentFlow());
} else {
    new AccentFlow();
}
