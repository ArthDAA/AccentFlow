import React, { useState, useEffect, useCallback, useRef } from 'react';
import { Card, CardContent, CardHeader, CardTitle, CardDescription } from "@/components/ui/card";
import { Badge } from "@/components/ui/badge";
import { Keyboard, Sparkles, Lightbulb } from 'lucide-react';
import { motion, AnimatePresence } from 'framer-motion';

const ACCENTS_MAP = {
    'e': ['é', 'è', 'ê', 'ë'], 'E': ['É', 'È', 'Ê', 'Ë'],
    'a': ['à', 'â', 'æ', 'ä'], 'A': ['À', 'Â', 'Æ', 'Ä'],
    'u': ['ù', 'û', 'ü'], 'U': ['Ù', 'Û', 'Ü'],
    'i': ['î', 'ï'], 'I': ['Î', 'Ï'],
    'o': ['ô', 'œ', 'ö'], 'O': ['Ô', 'Œ', 'Ö'],
    'c': ['ç'], 'C': ['Ç'],
    'y': ['ÿ'], 'Y': ['Ÿ'],
};

const ALL_ACCENT_KEYS = Object.keys(ACCENTS_MAP);

export default function ClavierIntelligentPage() {
    const [isAltHeld, setIsAltHeld] = useState(false);
    const [popupConfig, setPopupConfig] = useState(null);
    
    const editorRef = useRef(null);
    const accentState = useRef({ indices: {}, lastBaseKey: null, activeNode: null });
    const popupIndexRef = useRef(0); // Référence pour éviter les re-renders

    const resetAccentState = useCallback(() => {
        // Nettoyer le style négatif
        const node = accentState.current.activeNode;
        if (node && node.parentNode && node.parentNode.contains(node)) {
            const textNode = document.createTextNode(node.textContent);
            node.parentNode.replaceChild(textNode, node);
            
            // Remettre le curseur à la bonne position
            const selection = window.getSelection();
            const range = document.createRange();
            range.setStartAfter(textNode);
            range.collapse(true);
            selection.removeAllRanges();
            selection.addRange(range);
        }
        accentState.current = { indices: {}, lastBaseKey: null, activeNode: null };
        setPopupConfig(null);
        popupIndexRef.current = 0;
    }, []);

    const handleKeyDown = useCallback((event) => {
        if (event.altKey && ALL_ACCENT_KEYS.includes(event.key)) {
            event.preventDefault();
            
            const baseKey = event.key;
            const isNewLetter = accentState.current.lastBaseKey !== baseKey;
            
            if (isNewLetter) {
                // Nettoyer l'état précédent
                resetAccentState();
                
                // Ne pas effacer la lettre, mais la remplacer par un span stylé
                const selection = window.getSelection();
                const range = selection.getRangeAt(0);
                
                // Créer le span avec la première variante
                const span = document.createElement('span');
                span.className = 'accent-in-progress';
                const currentIndex = 0;
                const newChar = ACCENTS_MAP[baseKey][currentIndex];
                span.textContent = newChar;
                
                // Supprimer la lettre de base et insérer le span
                range.deleteContents();
                range.insertNode(span);
                
                // Positionner le curseur après le span
                range.setStartAfter(span);
                range.collapse(true);
                selection.removeAllRanges();
                selection.addRange(range);
                
                // Sauvegarder l'état
                accentState.current.activeNode = span;
                accentState.current.lastBaseKey = baseKey;
                accentState.current.indices[baseKey] = (currentIndex + 1) % ACCENTS_MAP[baseKey].length;

                // Créer l'infobulle UNE SEULE FOIS avec position fixe
                const editorRect = editorRef.current.getBoundingClientRect();
                const spanRect = span.getBoundingClientRect();
                setPopupConfig({
                    letter: baseKey,
                    position: {
                        x: spanRect.left - editorRect.left,
                        y: spanRect.top - editorRect.top - 60,
                    }
                });
                popupIndexRef.current = currentIndex;

            } else {
                // Continuer la séquence sur la même lettre
                const node = accentState.current.activeNode;
                if (node && node.parentNode && node.parentNode.contains(node)) {
                    const currentIndex = accentState.current.indices[baseKey];
                    const newChar = ACCENTS_MAP[baseKey][currentIndex];
                    node.textContent = newChar;
                    accentState.current.indices[baseKey] = (currentIndex + 1) % ACCENTS_MAP[baseKey].length;
                    
                    // CORRECTION: Juste mettre à jour la référence, pas l'état
                    popupIndexRef.current = currentIndex;
                }
            }
        } 
        else if (event.altKey && event.key.length === 1) {
            resetAccentState();
        }
        else if (event.altKey) {
            // Alt pressé avec une autre touche spéciale - ne pas faire de reset
        } else {
            // Autre touche sans Alt - reset
            resetAccentState();
        }
    }, [resetAccentState]);

    useEffect(() => {
        const handleAltDown = (e) => e.key === 'Alt' && setIsAltHeld(true);
        const handleAltUp = (e) => {
            if (e.key === 'Alt') {
                setIsAltHeld(false);
                resetAccentState();
            }
        };

        const editorNode = editorRef.current;

        window.addEventListener('keydown', handleAltDown);
        window.addEventListener('keyup', handleAltUp);
        editorNode?.addEventListener('keydown', handleKeyDown);

        const handleBlur = () => {
            setIsAltHeld(false);
            resetAccentState();
        };
        window.addEventListener('blur', handleBlur);

        return () => {
            window.removeEventListener('keydown', handleAltDown);
            window.removeEventListener('keyup', handleAltUp);
            editorNode?.removeEventListener('keydown', handleKeyDown);
            window.removeEventListener('blur', handleBlur);
        };
    }, [handleKeyDown, resetAccentState]);

    // Infobulle réactive qui ne se recrée jamais
    const AccentPopup = ({ config }) => {
        const [selectedIndex, setSelectedIndex] = useState(0);
        
        useEffect(() => {
            // Observer les changements d'index via la référence
            const checkIndexChange = () => {
                if (popupIndexRef.current !== selectedIndex) {
                    setSelectedIndex(popupIndexRef.current);
                }
            };
            
            const interval = setInterval(checkIndexChange, 50); // Vérification rapide
            return () => clearInterval(interval);
        }, [selectedIndex]);

        if (!config) return null;
        const { letter, position } = config;
        const variants = ACCENTS_MAP[letter];

        return (
            <motion.div
                initial={{ opacity: 0, scale: 0.8, y: 10 }} 
                animate={{ opacity: 1, scale: 1, y: 0 }} 
                exit={{ opacity: 0, scale: 0.8, y: 10 }}
                style={{ position: 'absolute', left: position.x, top: position.y, zIndex: 50 }}
                className="bg-white rounded-lg shadow-2xl border-2 border-gray-200 p-2"
            >
                <div className="flex items-center gap-1">
                    {variants.map((variant, index) => (
                        <motion.div 
                            key={variant}
                            animate={{ 
                                backgroundColor: index === selectedIndex ? '#3B82F6' : '#F9FAFB',
                                color: index === selectedIndex ? '#FFFFFF' : '#374151'
                            }}
                            transition={{ duration: 0.2 }}
                            className="w-8 h-8 flex items-center justify-center text-lg font-bold rounded-md"
                        >
                            {variant}
                        </motion.div>
                    ))}
                </div>
                <div className="absolute top-full left-1/2 transform -translate-x-1/2 w-0 h-0 border-l-4 border-r-4 border-t-4 border-l-transparent border-r-transparent border-t-gray-200"></div>
            </motion.div>
        );
    };

    return (
        <>
            <style>{`
                .accent-in-progress {
                    background-color: #111827;
                    color: white;
                    border-radius: 3px;
                    padding: 0 1px;
                    display: inline;
                }
                .editable-div:focus-visible {
                    outline: 2px solid #8B5CF6;
                    outline-offset: 2px;
                }
            `}</style>
            <div className="min-h-screen bg-gray-50 flex items-center justify-center p-4">
                <Card className="w-full max-w-2xl shadow-2xl shadow-gray-200">
                    <CardHeader>
                        <div className="flex items-center gap-3">
                            <div className="w-12 h-12 bg-gradient-to-br from-blue-500 to-purple-600 rounded-lg flex items-center justify-center">
                                <Keyboard className="text-white" />
                            </div>
                            <div>
                                <CardTitle className="text-2xl font-bold text-gray-800">Clavier Intelligent</CardTitle>
                                <CardDescription className="text-gray-500">Accents contextuels sur le caractère.</CardDescription>
                            </div>
                        </div>
                    </CardHeader>
                    <CardContent>
                        <div className="relative">
                            <AnimatePresence>
                            {isAltHeld && (
                                <motion.div initial={{ opacity: 0, scale: 0.9 }} animate={{ opacity: 1, scale: 1 }} exit={{ opacity: 0, scale: 0.9 }}
                                    className="absolute -top-3 -right-3 z-10"
                                >
                                    <Badge variant={popupConfig ? "destructive" : "secondary"} className="flex items-center gap-1.5 shadow-lg">
                                        <Sparkles className="w-4 h-4" />
                                        {popupConfig ? "Sélection" : "Mode Prêt"}
                                    </Badge>
                                </motion.div>
                            )}
                            </AnimatePresence>
                            
                            <div
                                ref={editorRef}
                                contentEditable="true"
                                suppressContentEditableWarning={true}
                                className="min-h-[250px] text-lg p-6 rounded-xl border bg-background editable-div"
                            >
                                Essayez ici ! L'infobulle reste maintenant vraiment stable.
                            </div>

                            <AnimatePresence>
                                {popupConfig && <AccentPopup config={popupConfig} />}
                            </AnimatePresence>
                        </div>
                        <Card className="mt-6 bg-blue-50 border-blue-200">
                            <CardContent className="p-4 flex items-start gap-4">
                                <Lightbulb className="w-6 h-6 text-blue-500 mt-1 flex-shrink-0" />
                                <div>
                                    <h4 className="font-semibold text-blue-800">Comment ça marche ?</h4>
                                    <p className="text-sm text-blue-700 mt-1">
                                        1. Maintenez <Badge variant="secondary">Alt</Badge> pour activer le mode.
                                        <br/>
                                        2. Tapez une lettre avec accents ('e', 'a'...) - elle passe en négatif avec options au-dessus.
                                        <br/>
                                        3. Les lettres sans accents ('j', 'k'...) s'écrivent normalement.
                                        <br/>
                                        4. Relâchez <Badge variant="secondary">Alt</Badge> pour valider le choix.
                                    </p>
                                </div>
                            </CardContent>
                        </Card>
                    </CardContent>
                </Card>
            </div>
        </>
    );
}