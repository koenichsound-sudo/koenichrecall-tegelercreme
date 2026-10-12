# Prüfbericht – Crème V1.0.1

12. Oktober 2026. Release-Build mit Apple Clang 17, CMake und JUCE 8.0.15. Ziel: macOS 12+, Apple Silicon ARM64.

## Erfolgreiche Prüfungen

- CMake-Projekt kompiliert und VST3-Bundle erzeugt.
- Alle zwölf diskreten Parameter mit sämtlichen Stufen und den korrekten Host-Anzeigewerten geprüft.
- Output und Threshold: jeweils 51 Stufen von 0 bis 10 in 0,2-Schritten.
- Gegenläufige Threshold-Skala; Output und Threshold stehen bei 5 auf 12 Uhr.
- Release enthält ausdrücklich die vom Nutzer vorgegebene Stufe 0,8 s.
- Alle Reglerstufen gespeichert und in einer zweiten Prozessorinstanz wiederhergestellt.
- Beschädigte Zustände, NaN, unlesbare, negative, überhöhte und nicht ganzzahlige Stufenindizes verworfen.
- GUI-Regler, dynamische Hover-Werte und Hostparameter synchron; keine Werteselectoren mehr vorhanden; Restore bei offenem Editor geprüft.
- Programmatisch erzeugte Maus-Drag-Ereignisse ändern die Regler und erzeugen ausschließlich ganze Stufenindizes.
- Threshold-Linksziehen erhöht den Wert und bewegt den Zeiger nach links; Rechtsziehen verringert ihn. Beide Richtungen automatisiert geprüft.
- Jede Position der drei Schalter überprüft.
- Float/Double-Audio bitgenau unverändert bei 1/2/8/64 Kanälen, 44,1/48/96 kHz, Blockgrößen 0/1/64/1024 und IN/BYPASS. Enthalten sind negatives Null, ein Subnormalwert und Unendlich.
- Fertige VST3 im JUCE-VST3-Testhost gescannt, zwei Instanzen geladen, Zustand übertragen, Stereo-Audio verarbeitet und nativen Editor erzeugt.
- Finale GUI aus dem echten Editor gerendert und auf überlappende Beschriftungen geprüft.
- ARM64-Architektur und ad-hoc-Bundlesignatur überprüft.

Das Laufprotokoll steht in `TESTERGEBNIS.txt`, die Tests unter `Tests/CremeTests.cpp`.

## Grenzen

Kein manueller Ende-zu-Ende-Test mit Speichern, Schließen und Wiederöffnen eines REAPER-Projekts. Der Ladetest erfolgte im JUCE-VST3-Testhost. Die Mausereignisse wurden automatisiert erzeugt; ein manueller Bedienungstest in REAPER steht aus.

Keine Apple-Notarisierung oder Developer-ID-Signatur. Das Plugin wird nicht automatisch installiert. Die Oberfläche ist eine vergrößerte Vektorumsetzung der bereitgestellten Vorlagen, keine fotografische Kopie.

Das Plugin verwendet eine eigene ID; Zustände des bestehenden Tegeler-x86-Plugins werden nicht automatisch übernommen.
