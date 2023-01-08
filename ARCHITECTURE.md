# Architecture

Ce document décrit l'architecture de notre projet Slash de SY5.

## Vue d'ensemble

|-- Dossier courant               (slash)
|   |-- cmd                       (gère les commandes)
|       |-- cmd_externe           (gère les commandes externes)
|           |-- cmd_externe.c
|           |-- cmd_externe.h
|       |-- cmd_interne           (gère les commandes internes)
|           |-- cmd_interne.c
|           |-- cmd_interne.h

## Détails



### Dossier courant

Le dossier courant contient les fichiers principaux du projet. Il contient notamment le fichier `slash.c` qui est le point d'entrée du programme. Il contient également le fichier `Makefile` qui permet de compiler le projet.

### cmd

Le dossier `cmd` contient les fichiers qui gèrent les commandes. Il contient deux sous-dossiers : `cmd_externe` et `cmd_interne`. Le dossier `cmd_externe` contient les fichiers qui gèrent les commandes externes. Le dossier `cmd_interne` contient les fichiers qui gèrent les commandes internes.



