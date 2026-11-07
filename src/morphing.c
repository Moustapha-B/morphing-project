/*
=========================================================
FICHIER : morphing.c
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

#include "morphing.h"
#include "ppm.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

/* Solve P = A + lambda*(B-A) + mu*(C-A) */
static int barycentric_lambda_mu(Point a, Point b, Point c, Point p, double *lambda, double *mu) {
    double abx = b.x - a.x, aby = b.y - a.y;
    double acx = c.x - a.x, acy = c.y - a.y;
    double apx = p.x - a.x, apy = p.y - a.y;

    double det = abx * acy - aby * acx;
    if (fabs(det) < 1e-12) return 0;

    *lambda = (apx * acy - apy * acx) / det;
    *mu     = (abx * apy - aby * apx) / det;
    return 1;
}

static Pixel sample_nn(const Image *img, Point p) {
    int x = (int)llround(p.x);
    int y = (int)llround(p.y);
    if (x < 0) x = 0;
    if (y < 0) y = 0;
    if (x >= img->w) x = img->w - 1;
    if (y >= img->h) y = img->h - 1;
    return *image_at_const(img, x, y);
}

int morph_intermediate_points(const PairPoint *pairs, size_t n, double alpha, Point **out_pts, size_t *out_n) {
    if (!pairs || !out_pts || !out_n) return 0;
    Point *pts = (Point*)malloc(n * sizeof(Point));
    if (!pts) return 0;
    for (size_t i = 0; i < n; i++) {
        pts[i].x = (1.0 - alpha) * pairs[i].dep.x + alpha * pairs[i].arr.x;
        pts[i].y = (1.0 - alpha) * pairs[i].dep.y + alpha * pairs[i].arr.y;
    }
    *out_pts = pts;
    *out_n = n;
    return 1;
}

int morph_render_frame(const Image *dep, const Image *arr,
                       const PairPoint *pairs, size_t n,
                       const Point *pts_mid,
                       const Triangle *tris, size_t nb_tris,
                       double alpha, Image *out) {
    (void)n;
    if (!dep || !arr || !pairs || !pts_mid || !tris || !out) return 0;
    if (dep->w != arr->w || dep->h != arr->h) return 0;

    *out = image_create(dep->w, dep->h, 255);

    for (int y = 0; y < out->h; y++) {
        for (int x = 0; x < out->w; x++) {
            Point P = {(double)x, (double)y};

            /* trouver le triangle contenant P */
            int tid = tri_find_containing(pts_mid, tris, nb_tris, P);
            if (tid < 0) {
                /* fallback : simple cross-dissolve */
                Pixel pd = *image_at_const(dep, x, y);
                Pixel pa = *image_at_const(arr, x, y);
                Pixel pr;
                pr.r = (unsigned char)llround((1-alpha)*pd.r + alpha*pa.r);
                pr.g = (unsigned char)llround((1-alpha)*pd.g + alpha*pa.g);
                pr.b = (unsigned char)llround((1-alpha)*pd.b + alpha*pa.b);
                *image_at(out, x, y) = pr;
                continue;
            }

            Triangle t = tris[tid];
            Point A = pts_mid[t.a], B = pts_mid[t.b], C = pts_mid[t.c];
            double lambda, mu;
            if (!barycentric_lambda_mu(A, B, C, P, &lambda, &mu)) {
                Pixel pd = *image_at_const(dep, x, y);
                *image_at(out, x, y) = pd;
                continue;
            }

            /* points correspondants dans départ et arrivée */
            Point AD = pairs[t.a].dep, BD = pairs[t.b].dep, CD = pairs[t.c].dep;
            Point AA = pairs[t.a].arr, BA = pairs[t.b].arr, CA = pairs[t.c].arr;

            Point PD = { AD.x + lambda*(BD.x-AD.x) + mu*(CD.x-AD.x),
                         AD.y + lambda*(BD.y-AD.y) + mu*(CD.y-AD.y) };
            Point PA = { AA.x + lambda*(BA.x-AA.x) + mu*(CA.x-AA.x),
                         AA.y + lambda*(BA.y-AA.y) + mu*(CA.y-AA.y) };

            Pixel cD = sample_nn(dep, PD);
            Pixel cA = sample_nn(arr, PA);

            Pixel c;
            c.r = (unsigned char)llround((1.0-alpha)*cD.r + alpha*cA.r);
            c.g = (unsigned char)llround((1.0-alpha)*cD.g + alpha*cA.g);
            c.b = (unsigned char)llround((1.0-alpha)*cD.b + alpha*cA.b);
            *image_at(out, x, y) = c;
        }
    }
    return 1;
}

int generate_morphing_sequence(const char *img1_ppm, const char *img2_ppm, const char *points_file,
                               int N, int fps, const char *out_video_path) {
    if (N < 1) return 0;
    if (fps <= 0) fps = 10;

    Image dep, arr;
    if (!ppm_read(img1_ppm, &dep)) { fprintf(stderr, "Erreur lecture %s\n", img1_ppm); return 0; }
    if (!ppm_read(img2_ppm, &arr)) { fprintf(stderr, "Erreur lecture %s\n", img2_ppm); image_free(&dep); return 0; }
    if (dep.w != arr.w || dep.h != arr.h) {
        fprintf(stderr, "Erreur: images de tailles différentes (%dx%d vs %dx%d)\n", dep.w, dep.h, arr.w, arr.h);
        image_free(&dep); image_free(&arr);
        return 0;
    }

    PairPoint *pairs = NULL;
    size_t n = 0;
    if (!points_load(points_file, &pairs, &n)) {
        fprintf(stderr, "Erreur lecture points %s\n", points_file);
        image_free(&dep); image_free(&arr);
        return 0;
    }
    if (n < 5) {
        fprintf(stderr, "Erreur: il faut au moins 5 points (coins inclus).\n");
        free(pairs); image_free(&dep); image_free(&arr);
        return 0;
    }

    /* frames/ */
    system("mkdir -p frames");

    for (int k = 0; k <= N; k++) {
        double alpha = (double)k / (double)N;

        Point *mid = NULL; size_t mid_n = 0;
        if (!morph_intermediate_points(pairs, n, alpha, &mid, &mid_n)) {
            fprintf(stderr, "Erreur points intermédiaires\n");
            free(pairs); image_free(&dep); image_free(&arr);
            return 0;
        }

        Triangle *tris = NULL; size_t nb_tris = 0;
        if (!triangulate_incremental(mid, mid_n, &tris, &nb_tris)) {
            fprintf(stderr, "Erreur triangulation (vérifie l'ordre: coins puis points)\n");
            free(mid); free(pairs); image_free(&dep); image_free(&arr);
            return 0;
        }

        Image frame;
        if (!morph_render_frame(&dep, &arr, pairs, n, mid, tris, nb_tris, alpha, &frame)) {
            fprintf(stderr, "Erreur render frame\n");
            free(tris); free(mid); free(pairs); image_free(&dep); image_free(&arr);
            return 0;
        }

        char path[256];
        snprintf(path, sizeof(path), "frames/morph_%03d.ppm", k);
        /* P6 plus rapide + léger */
        ppm_write(path, &frame, PPM_P6);

        image_free(&frame);
        free(tris);
        free(mid);
    }

    /* vidéo optionnelle */
    if (out_video_path && out_video_path[0] != '\0') {
        char cmd[1024];
        /* -pattern_type glob ne marche pas partout; on reste sur morph_%03d.ppm */
        snprintf(cmd, sizeof(cmd),
            "ffmpeg -y -framerate %d -i frames/morph_%%03d.ppm "
            "-c:v libx264 -pix_fmt yuv420p -preset medium -crf 22 %s",
            fps, out_video_path);
        int r = system(cmd);
        if (r != 0) fprintf(stderr, "Attention: ffmpeg a retourné un code %d (ffmpeg installé ?)\n", r);
    }

    free(pairs);
    image_free(&dep);
    image_free(&arr);
    return 1;
}
