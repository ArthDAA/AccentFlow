
import React, { useState, useEffect, useCallback, useRef } from 'react';
import { Textarea } from "@/components/ui/textarea";
import { Card, CardContent, CardHeader, CardTitle, CardDescription } from "@/components/ui/card";
import { Badge } from "@/components/ui/badge";
import { Keyboard, Sparkles, Lightbulb } from 'lucide-react';
import { motion, AnimatePresence } from 'framer-motion';

// Définition de la table des accents, similaire à votre code Python.
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
    const [text, setText] = useState("Essayez ici ! Maintenez la touche 'Alt' et appuyez sur 'e', 'a', 'c'...");
    const [accentMode, setAccentMode] = useState(false);
    const [activeLetter, setActiveLetter] = useState(null);
    const accentState = useRef({ indices: {}, lastBaseKey: null });
    const textareaRef = useRef(null);

    const resetAccentState = useCallback(() => {
        accentState.current = { indices: {}, lastBaseKey: null };
        setActiveLetter(null);
    }, []);

    const handleKeyUp = useCallback((event) => {
        if (event.key === 'Alt') {
            setAccentMode(false);
            resetAccentState();
            event.preventDefault();
        }
    }, [resetAccentState]);
    
    const handleKeyDown = useCallback((event) => {
        if (event.key === 'Alt') {
            setAccentMode(true);
            event.preventDefault();
            return;
        }

        if (!accentMode) {
            return;
        }

        const baseKey = event.key;
        if (ALL_ACCENT_KEYS.includes(baseKey)) {
            event.preventDefault();

            let target = event.target;
            // On s'assure que la logigue ne s'applique que dans notre zone de texte
            if (!textareaRef.current || !textareaRef.current.contains(target)) {
                 if(target.tagName.toLowerCase() !== 'textarea'){
                    return;
                 }
            }
            
            const isNewLetter = accentState.current.lastBaseKey !== baseKey;
            
            if (isNewLetter) {
                resetAccentState();
                accentState.current.lastBaseKey = baseKey;
                setActiveLetter(baseKey);
            }

            const currentIndex = accentState.current.indices[baseKey] || 0;
            const newChar = ACCENTS_MAP[baseKey][currentIndex];
            
            const start = target.selectionStart;
            const end = target.selectionEnd;

            // Remplacer le caractère précédent si c'est la même lettre de base
            const textToInsertBefore = isNewLetter ? 0 : 1;
            
            target.setSelectionRange(start - textToInsertBefore, end);
            
            // Simule l'insertion de texte pour supporter l'annulation (undo)
            document.execCommand("insertText", false, newChar);

            // Mise à jour de l'index pour la prochaine pression
            accentState.current.indices[baseKey] = (currentIndex + 1) % ACCENTS_MAP[baseKey].length;
        }

    }, [accentMode, resetAccentState]);

    useEffect(() => {
        // On attache les listeners à la fenêtre pour capturer les évènements partout
        window.addEventListener('keydown', handleKeyDown);
        window.addEventListener('keyup', handleKeyUp);

        return () => {
            window.removeEventListener('keydown', handleKeyDown);
            window.removeEventListener('keyup', handleKeyUp);
        };
    }, [handleKeyDown, handleKeyUp]);

    const AccentVisualizer = ({ letter }) => {
        if (!letter || !ACCENTS_MAP[letter]) return null;

        const variants = ACCENTS_MAP[letter];
        const currentIndex = accentState.current.indices[letter] || 0;

        return (
            <motion.div
                initial={{ opacity: 0, y: 10 }}
                animate={{ opacity: 1, y: 0 }}
                exit={{ opacity: 0, y: -10 }}
                className="flex items-center gap-2 mt-4"
            >
                <span className="font-semibold text-gray-600">{letter}:</span>
                <div className="flex items-center gap-2 rounded-full bg-gray-100 p-1">
                    {variants.map((variant, index) => (
                        <motion.div
                            key={variant}
                            animate={{ 
                                scale: index === currentIndex ? 1.2 : 1,
                                color: index === currentIndex ? '#10B981' : '#4B5563',
                            }}
                            transition={{ type: 'spring', stiffness: 500, damping: 20 }}
                            className="text-xl font-bold w-8 h-8 flex items-center justify-center"
                        >
                            {variant}
                        </motion.div>
                    ))}
                </div>
            </motion.div>
        );
    };

    return (
        <div className="min-h-screen bg-gray-50 flex items-center justify-center p-4">
            <Card className="w-full max-w-2xl shadow-2xl shadow-gray-200">
                <CardHeader>
                    <div className="flex items-center gap-3">
                        <div className="w-12 h-12 bg-gradient-to-br from-blue-500 to-purple-600 rounded-lg flex items-center justify-center">
                            <Keyboard className="text-white" />
                        </div>
                        <div>
                            <CardTitle className="text-2xl font-bold text-gray-800">Clavier Intelligent</CardTitle>
                            <CardDescription className="text-gray-500">Une nouvelle façon de saisir les accents.</CardDescription>
                        </div>
                    </div>
                </CardHeader>
                <CardContent>
                    <div className="relative">
                        <AnimatePresence>
                        {accentMode && (
                            <motion.div
                                initial={{ opacity: 0, scale: 0.9 }}
                                animate={{ opacity: 1, scale: 1 }}
                                exit={{ opacity: 0, scale: 0.9 }}
                                className="absolute -top-3 -right-3"
                            >
                                <Badge variant="destructive" className="flex items-center gap-1.5 shadow-lg">
                                    <Sparkles className="w-4 h-4" />
                                    Mode Accent
                                </Badge>
                            </motion.div>
                        )}
                        </AnimatePresence>
                        <Textarea
                            ref={textareaRef}
                            value={text}
                            onChange={(e) => setText(e.target.value)}
                            className="min-h-[250px] text-lg p-6 rounded-xl focus-visible:ring-purple-500"
                            placeholder="Commencez à écrire..."
                        />
                    </div>

                    <AnimatePresence mode="wait">
                        <AccentVisualizer key={activeLetter} letter={activeLetter} />
                    </AnimatePresence>

                    <Card className="mt-6 bg-blue-50 border-blue-200">
                        <CardContent className="p-4 flex items-start gap-4">
                            <Lightbulb className="w-6 h-6 text-blue-500 mt-1 flex-shrink-0" />
                            <div>
                                <h4 className="font-semibold text-blue-800">Comment ça marche ?</h4>
                                <p className="text-sm text-blue-700 mt-1">
                                    1. Maintenez la touche <Badge variant="secondary">Alt</Badge> enfoncée pour activer le mode accent.
                                    <br />
                                    2. Appuyez plusieurs fois sur une lettre ('e', 'a', etc.) pour faire défiler les accents.
                                    <br />
                                    3. Relâchez la touche <Badge variant="secondary">Alt</Badge> pour continuer à écrire normalement.
                                </p>
                            </div>
                        </CardContent>
                    </Card>
                </CardContent>
            </Card>
        </div>
    );
}
