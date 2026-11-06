/*
=========================================================
FICHIER : triangulation.h
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

#ifndef TRIANGULATION_H
#define TRIANGULATION_H

#include <stddef.h>
#include "points.h"

typedef struct { int a, b, c; } Triangle;

int triangulate_incremental(const Point *pts, size_t nb_pts, Triangle **out_tris, size_t *out_nb_tris);

int tri_find_containing(const Point *pts, const Triangle *tris, size_t nb_tris, Point p);

#endif
