/*
=========================================================
FICHIER : points.h
---------------------------------------------------------
Ce fichier fait partie du projet de morphing d’images
réalisé par Ba Moustapha.

Le code ci-dessous n’a PAS été modifié.
Seuls des commentaires explicatifs ont été ajoutés
afin de faciliter la compréhension du fonctionnement
lors de la soutenance.

Ce fichier participe au bon déroulement global du
morphing : lecture d’images, sélection de points,
triangulation, interpolation ou affichage graphique
selon son rôle spécifique.
=========================================================
*/

#ifndef POINTS_H
#define POINTS_H

#include <stddef.h>

typedef struct { double x, y; } Point;

typedef struct {
    Point dep;
    Point arr;
} PairPoint;

/* Sauvegarde au format texte :
   - première ligne : nb_points_total (incluant les 4 coins ajoutés)
   - puis : dx dy ax ay
   La fonction ajoute automatiquement les 4 coins (en tête).
*/
int points_save_with_corners(const char *path, const PairPoint *pairs, size_t n, int w, int h);

int points_load(const char *path, PairPoint **out_pairs, size_t *out_n);

#endif
