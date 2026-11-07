/*
=========================================================
FICHIER : morphing.h
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

#ifndef MORPHING_H
#define MORPHING_H

#include "image.h"
#include "points.h"
#include "triangulation.h"

/* Calcule les points intermédiaires (même nombre que pairs) */
int morph_intermediate_points(const PairPoint *pairs, size_t n, double alpha, Point **out_pts, size_t *out_n);

/* Rend une frame intermédiaire à alpha.
   - pts_mid: points intermédiaires (taille n)
   - tris: triangulation sur pts_mid
*/
int morph_render_frame(const Image *dep, const Image *arr,
                       const PairPoint *pairs, size_t n,
                       const Point *pts_mid,
                       const Triangle *tris, size_t nb_tris,
                       double alpha, Image *out);

/* Génère N+1 frames + (optionnel) vidéo via ffmpeg. */
int generate_morphing_sequence(const char *img1_ppm, const char *img2_ppm, const char *points_file,
                               int N, int fps, const char *out_video_path);

#endif
