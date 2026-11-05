/*
=========================================================
FICHIER : points.c
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

#include "points.h"
#include <stdio.h>
#include <stdlib.h>

int points_save_with_corners(const char *path, const PairPoint *pairs, size_t n, int w, int h) {
    FILE *f = fopen(path, "w");
    if (!f) return 0;

    /* 4 coins ajoutés */
    size_t total = n + 4;
    fprintf(f, "%zu\n", total);

    /* coins (dep == arr) */
    PairPoint corners[4] = {
        {{0, 0}, {0, 0}},
        {{w-1, 0}, {w-1, 0}},
        {{w-1, h-1}, {w-1, h-1}},
        {{0, h-1}, {0, h-1}},
    };
    for (int i = 0; i < 4; i++) {
        fprintf(f, "%.10g %.10g %.10g %.10g\n",
            corners[i].dep.x, corners[i].dep.y,
            corners[i].arr.x, corners[i].arr.y);
    }

    for (size_t i = 0; i < n; i++) {
        fprintf(f, "%.10g %.10g %.10g %.10g\n",
            pairs[i].dep.x, pairs[i].dep.y,
            pairs[i].arr.x, pairs[i].arr.y);
    }

    fclose(f);
    return 1;
}

int points_load(const char *path, PairPoint **out_pairs, size_t *out_n) {
    if (!out_pairs || !out_n) return 0;
    *out_pairs = NULL;
    *out_n = 0;

    FILE *f = fopen(path, "r");
    if (!f) return 0;

    size_t n;
    if (fscanf(f, "%zu", &n) != 1 || n < 4) { fclose(f); return 0; }

    PairPoint *pairs = (PairPoint*)malloc(n * sizeof(PairPoint));
    if (!pairs) { fclose(f); return 0; }

    for (size_t i = 0; i < n; i++) {
        if (fscanf(f, "%lf %lf %lf %lf",
                   &pairs[i].dep.x, &pairs[i].dep.y,
                   &pairs[i].arr.x, &pairs[i].arr.y) != 4) {
            free(pairs);
            fclose(f);
            return 0;
        }
    }

    fclose(f);
    *out_pairs = pairs;
    *out_n = n;
    return 1;
}
