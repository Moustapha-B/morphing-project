/*
=========================================================
FICHIER : triangulation.c
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

#include "triangulation.h"
#include <stdlib.h>
#include <math.h>

static double cross(Point a, Point b, Point c) {
    /* cross(AB, AC) */
    double abx = b.x - a.x, aby = b.y - a.y;
    double acx = c.x - a.x, acy = c.y - a.y;
    return abx * acy - aby * acx;
}

static int point_in_triangle(Point p, Point a, Point b, Point c) {
    /* même côté (tolérance) */
    const double eps = 1e-9;
    double c1 = cross(a, b, p);
    double c2 = cross(b, c, p);
    double c3 = cross(c, a, p);

    int has_neg = (c1 < -eps) || (c2 < -eps) || (c3 < -eps);
    int has_pos = (c1 > eps) || (c2 > eps) || (c3 > eps);
    return !(has_neg && has_pos);
}

int tri_find_containing(const Point *pts, const Triangle *tris, size_t nb_tris, Point p) {
    for (size_t i = 0; i < nb_tris; i++) {
        Point a = pts[tris[i].a];
        Point b = pts[tris[i].b];
        Point c = pts[tris[i].c];
        if (point_in_triangle(p, a, b, c)) return (int)i;
    }
    return -1;
}

int triangulate_incremental(const Point *pts, size_t nb_pts, Triangle **out_tris, size_t *out_nb_tris) {
    if (!pts || !out_tris || !out_nb_tris) return 0;
    if (nb_pts < 5) return 0;

    /* On sait que le résultat doit être 2N-6 triangles */
    size_t cap = 2 * nb_pts;
    Triangle *tris = (Triangle*)malloc(cap * sizeof(Triangle));
    if (!tris) return 0;

    /* 4 premiers points = rectangle (0,1,2,3) dans cet ordre */
    /* 5e point (index 4) crée 4 triangles autour du rectangle */
    size_t nb = 0;
    int P = 4;
    tris[nb++] = (Triangle){0, 1, P};
    tris[nb++] = (Triangle){1, 2, P};
    tris[nb++] = (Triangle){2, 3, P};
    tris[nb++] = (Triangle){3, 0, P};

    for (size_t i = 5; i < nb_pts; i++) {
        Point p = pts[i];
        int idx = tri_find_containing(pts, tris, nb, p);
        if (idx < 0) {
            /* Si pas trouvé (rare), on échoue proprement */
            free(tris);
            return 0;
        }

        Triangle old = tris[idx];

        /* supprimer old : on remplace par le dernier */
        tris[idx] = tris[nb - 1];
        nb--;

        /* ajouter 3 triangles */
        if (nb + 3 > cap) {
            cap *= 2;
            Triangle *tmp = (Triangle*)realloc(tris, cap * sizeof(Triangle));
            if (!tmp) { free(tris); return 0; }
            tris = tmp;
        }
        tris[nb++] = (Triangle){old.a, old.b, (int)i};
        tris[nb++] = (Triangle){old.b, old.c, (int)i};
        tris[nb++] = (Triangle){old.c, old.a, (int)i};
    }

    *out_tris = tris;
    *out_nb_tris = nb;
    return 1;
}
