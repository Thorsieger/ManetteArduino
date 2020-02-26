# ManetteArduino
Projet de tutorat à Polytech Lille dans le but de transformer un Arduino en une manette de jeux reconnu automatiquement par un ordinateur.

Fichier ReadMe du tutorat de système du groupe Mertz/Hassenforder

Compilation et exécution du code :

- Code Arduino :
NB : Si l'ATMega16u2 n'est plus configuré en TFDI repasser à l'état d'origine en suivant le guide présent plus bas dans ce Fichier
Se placer dans le dossier : ./Code_Arduino
Compiler le code : make 	(avr-gcc doit etre installé)
Téléverser (avec la carte arduino connectée à son pc) : make upload

- Code 16u2 :
Se placer dans le dossier : ./lufa-LUFA-170418/PolytechLille/PAD
Compiler le code : make
Téléversement :
    1) court-circuiter les lignes reset et masse du 16u2
    2) lancer les commandes : dfu-programmer atmega16u2 erase
                              dfu-programmer atmega16u2 flash PAD.hex
                              dfu-programmer atmega16u2 reset
    3) Débrancher et rebrancher la carte Arduino

(Pour revenir à l'état d'origine, remplacer PAD.hex par rollback.hex)

- Code USB :
NB : L'ATMega16u2 doit avoir été flashé
Se placer dans le dossier : ./ConfigUSB
Compiler le code : make 	(la library usb doit avoir été téléchargé)
Exécuter le code (droit admin nécessaire) : sudo ./tutorat 
Utilisation : Pour allumer une led, envoyer une lettre entre a et f (A-F pour l'éteindre). La lettre a correspond à la led 1, b à la led 2, etc.
              x pour quitter le programme


Ce qui fonctionne :

Le code Arduino est fonctionnel, il envoie à chaque changement la valeur de ses boutons et du joystick. Il allume bien les led quand le message correspondant lui est envoyé. (Test avant flash de l'ATMega16u2)
Le code USB fonctionne également, on récupère correctement le périphériques, ses interfaces et ses endpoints. Les interfaces sont libéré à la fin du programme.
Le code 16u2 : la configuration des EndPoints est correcte et le 16u2 fait bien passer les données recu du pc vers l'ATMega328p et inversement (lsusb -vvv -d 1111:2222)

Ce qui ne fonctionne pas :

On affiche en permanence la valeurs des boutons/joystick et pas uniquement à chaque changement.
On utlise une fonction getChar() qui est bloquant pour changer l'état des leds, on ne voit donc pas bien les changements des boutons/joystick.
On ne quitte pas le programme lors de l'appuie sur le bouton d'arret.
Les leds ne s'éteignent pas à la fin du programme.
Le code n'a pas été relié au jeu space invaders.
