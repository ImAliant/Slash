#ifndef _CMD_INTERNE_H
#define _CMD_INTERNE_H

#define MAX_ARGS_NUMBER 4096
#define MAX_ARG_STRLEN 4096

/* Termine le processus slash avec comme valeur de retour val.
 * @return val ou par défaut la derniere valeur de retour de la dernière commande exécutée.
 */
int cmd_exit(int val);

/* Change de répertoire de travail courant en le répertoire ref 
 * Si arg est "-L", on suit les liens symboliques
 * Si arg est "-P", on suit l'interpretation physique des liens symboliques
 * @return 0 en cas de succès, 1 sinon
 */
int cmd_cd(char *arg, char *ref);

/* Affiche une référence absolue du répertoire de travail courant.
 * Si arg est "-P", on affiche sa référence absolue physique, c'est à dire ne faisant intervenir aucun lien symbolique.
 * Si arg est "-L", on affiche sa référence absolue logique, déduite des paramètres des précédents changements de répertoire
 * courant, et contenant éventuellement des liens symboliques.
 * @return 0 en cas de succès, 1 sinon
 */
int cmd_pwd(char *arg);

#endif