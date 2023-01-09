# SY5-Projet-2022-2023

## Compilation et execution

- Pour __compiler__ le projet, il faut utiliser la commande `make` dans le dossier `sy5-projet-2022-2023`.
- Pour __executer__ le projet, il faut utiliser la commande `./slash` dans le dossier `sy5-projet-2022-2023`.

## Utilisation de Slash

Slash s'utilise comme un shell classique. Il suffit d'écrire les commandes souhaiter et de les executer avec la touche `Entrée`.

## Commandes internes

- `exit [val]` : 

Termine le processus avec comme valeur de retour val (ou par défaut la valeur de retour de la dernière commande exécutée).

- `pwd [-L | -P]`: 

Affiche la référence absolue du répertoire courant. 

- avec l'option `-P`, sa référence absolue *physique*, c'est-à-dire ne faisant intervenir aucun lien symbolique;
- avec l'option `-L`, sa référence absolue *logique*, déduite des paramètres des précédents changements de répertoire courant, et contenant éventuellement des liens symboliques.

La valeur de retour est 0 en cas de succès, 1 en cas d'echec.

- `cd [-L | -P] [ref | -]`: 

Change de répertoire de travail courant en le répertoire ref (s'il s'agit d'une référence valide), le précédent répertoire de travail si le paramètre est -, ou $HOME en l'absence de paramètre. 

- Avec l'option -P, ref (et en particulier ses composantes ..) est interprétée au regard de la structure physique de l'arborescence. 
- Avec l'option -L (option par défaut), ref (et en particulier ses composantes ..) est interprétée de manière logique (a/../b est interprétée comme b) si cela a du sens, et de manière physique sinon. 

La valeur de retour est 0 en cas de succès, 1 en cas d'échec.

## Commandes externes

Slash peut exécuter toutes les commandes externes avec ou sans arguments.
