/******************************************************************************
 * Projekt - Zaklady pocitacove grafiky - IZG
 * spanel@fit.vutbr.cz
 *
 * $Id:$
 */

#include "student.h"
#include "transform.h"
#include "fragment.h"

#include <memory.h>
#include <math.h>


/*****************************************************************************
 * Globalni promenne a konstanty
 */

/* Typ/ID rendereru (nemenit) */
const int           STUDENT_RENDERER = 1;

// v MAT_RED bola cervena zlozka nastavena na 0.8 - nastavit to tiez na 0.8 ci na 1.0?
const S_Material    MAT_WHITE_AMBIENT  = { 1.0, 1.0, 1.0, 1.0 };
const S_Material    MAT_WHITE_DIFFUSE  = { 1.0, 1.0, 1.0, 1.0 };
const S_Material    MAT_WHITE_SPECULAR = { 1.0, 1.0, 1.0, 1.0 };


/*****************************************************************************
 * Funkce vytvori vas renderer a nainicializuje jej
 */

S_Renderer * studrenCreate()
{
    S_StudentRenderer * renderer = (S_StudentRenderer *)malloc(sizeof(S_StudentRenderer));
    IZG_CHECK(renderer, "Cannot allocate enough memory");

    /* inicializace default rendereru */
    renderer->base.type = STUDENT_RENDERER;
    renInit(&renderer->base);

    /* nastaveni ukazatelu na upravene funkce */
    /* napr. renderer->base.releaseFunc = studrenRelease; */
    renderer->base.releaseFunc = studrenRelease;
    renderer->base.projectTriangleFunc = studrenProjectTriangle;

    /* inicializace nove pridanych casti */
    renderer->width = 0;
    renderer->height = 0;
    //S_RGBA *p = loadBitmap(TEXTURE_FILENAME, &(renderer->width), &(renderer->height));
    renderer->texture = loadBitmap(TEXTURE_FILENAME, &(renderer->width), &(renderer->height));

    return (S_Renderer *)renderer;
}

/*****************************************************************************
 * Funkce korektne zrusi renderer a uvolni pamet
 */

void studrenRelease(S_Renderer **ppRenderer)
{
    S_StudentRenderer * renderer;

    if( ppRenderer && *ppRenderer )
    {
        /* ukazatel na studentsky renderer */
        renderer = (S_StudentRenderer *)(*ppRenderer);

        /* pripadne uvolneni pameti */
        free(renderer->texture);
        
        /* fce default rendereru */
        renRelease(ppRenderer);
    }
}

/******************************************************************************
 * Nova fce pro rasterizaci trojuhelniku s podporou texturovani
 * Upravte tak, aby se trojuhelnik kreslil s texturami
 * (doplnte i potrebne parametry funkce - texturovaci souradnice, ...)
 * v1, v2, v3 - ukazatele na vrcholy trojuhelniku ve 3D pred projekci
 * n1, n2, n3 - ukazatele na normaly ve vrcholech ve 3D pred projekci
 * x1, y1, ... - vrcholy trojuhelniku po projekci do roviny obrazovky
 */

void studrenDrawTriangle(S_Renderer *pRenderer,
                         S_Coords *v1, S_Coords *v2, S_Coords *v3,
                         S_Coords *n1, S_Coords *n2, S_Coords *n3,
                         int x1, int y1,
                         int x2, int y2,
                         int x3, int y3,
                         S_Coords *t, double ha, double hb, double hc)
{

    int         minx, miny, maxx, maxy;
    int         a1, a2, a3, b1, b2, b3, c1, c2, c3;
    int         s1, s2, s3;
    int         x, y, e1, e2, e3;
    double      alpha, beta, gamma, w1, w2, w3, z, tX, tY;
    S_RGBA      col1, col2, col3, color, tColor;

    IZG_ASSERT(pRenderer && v1 && v2 && v3 && n1 && n2 && n3);

    /* vypocet barev ve vrcholech */
    col1 = pRenderer->calcReflectanceFunc(pRenderer, v1, n1);
    col2 = pRenderer->calcReflectanceFunc(pRenderer, v2, n2);
    col3 = pRenderer->calcReflectanceFunc(pRenderer, v3, n3);

    /* obalka trojuhleniku */
    minx = MIN(x1, MIN(x2, x3));
    maxx = MAX(x1, MAX(x2, x3));
    miny = MIN(y1, MIN(y2, y3));
    maxy = MAX(y1, MAX(y2, y3));

    /* oriznuti podle rozmeru okna */
    miny = MAX(miny, 0);
    maxy = MIN(maxy, pRenderer->frame_h - 1);
    minx = MAX(minx, 0);
    maxx = MIN(maxx, pRenderer->frame_w - 1);

    /* Pineduv alg. rasterizace troj.
       hranova fce je obecna rovnice primky Ax + By + C = 0
       primku prochazejici body (x1, y1) a (x2, y2) urcime jako
       (y1 - y2)x + (x2 - x1)y + x1y2 - x2y1 = 0 */

    /* normala primek - vektor kolmy k vektoru mezi dvema vrcholy, tedy (-dy, dx) */
    a1 = y1 - y2;
    a2 = y2 - y3;
    a3 = y3 - y1;
    b1 = x2 - x1;
    b2 = x3 - x2;
    b3 = x1 - x3;

    /* koeficient C */
    c1 = x1 * y2 - x2 * y1;
    c2 = x2 * y3 - x3 * y2;
    c3 = x3 * y1 - x1 * y3;

    /* vypocet hranove fce (vzdalenost od primky) pro protejsi body */
    s1 = a1 * x3 + b1 * y3 + c1;
    s2 = a2 * x1 + b2 * y1 + c2;
    s3 = a3 * x2 + b3 * y2 + c3;

    if ( !s1 || !s2 || !s3 )
    {
        return;
    }

    /* normalizace, aby vzdalenost od primky byla kladna uvnitr trojuhelniku */
    if( s1 < 0 )
    {
        a1 *= -1;
        b1 *= -1;
        c1 *= -1;
    }
    if( s2 < 0 )
    {
        a2 *= -1;
        b2 *= -1;
        c2 *= -1;
    }
    if( s3 < 0 )
    {
        a3 *= -1;
        b3 *= -1;
        c3 *= -1;
    }

    /* koeficienty pro barycentricke souradnice */
    alpha = 1.0 / ABS(s2);
    beta = 1.0 / ABS(s3);
    gamma = 1.0 / ABS(s1);

    /* vyplnovani... */
    for( y = miny; y <= maxy; ++y )
    {
        /* inicilizace hranove fce v bode (minx, y) */
        e1 = a1 * minx + b1 * y + c1;
        e2 = a2 * minx + b2 * y + c2;
        e3 = a3 * minx + b3 * y + c3;

        for( x = minx; x <= maxx; ++x )
        {
            if( e1 >= 0 && e2 >= 0 && e3 >= 0 )
            {
                /* interpolace pomoci barycentrickych souradnic
                   e1, e2, e3 je aktualni vzdalenost bodu (x, y) od primek */
                w1 = alpha * e2;
                w2 = beta * e3;
                w3 = gamma * e1;

                /* interpolace z-souradnice */
                z = w1 * v1->z + w2 * v2->z + w3 * v3->z;

                /* Vypocet texturovacich suradnic bez perspektivnej korekcie
                tX = w1 * t[0].x + w2 * t[1].x + w3 * t[2].x;
                tY = w1 * t[0].y + w2 * t[1].y + w3 * t[2].y;
                */
                /* Vypocet texturovacich suradnic s vyuzitim perspektivnej korekcie */
                tX = (w1*t[0].x/ha + w2*t[1].x/hb + w3*t[2].x/hc)/(w1/ha + w2/hb + w3/hc);
                tY = (w1*t[0].y/ha + w2*t[1].y/hb + w3*t[2].y/hc)/(w1/ha + w2/hb + w3/hc);

                /* interpolace barvy */
                color.red = ROUND2BYTE(w1 * col1.red + w2 * col2.red + w3 * col3.red);
                color.green = ROUND2BYTE(w1 * col1.green + w2 * col2.green + w3 * col3.green);
                color.blue = ROUND2BYTE(w1 * col1.blue + w2 * col2.blue + w3 * col3.blue);

                /* Ziskanie farby z textury */
                tColor = studrenTextureValue((S_StudentRenderer *) pRenderer, tX, tY);

                /* Miesanie farieb */
                color.red = (color.red * tColor.red)/256;
                color.green = (color.green * tColor.green)/256;
                color.blue = (color.blue * tColor.blue)/256;
                color.alpha = 255;

                /* vykresleni bodu */
                if( z < DEPTH(pRenderer, x, y) )
                {
                    PIXEL(pRenderer, x, y) = color;
                    DEPTH(pRenderer, x, y) = z;
                }
            }

            /* hranova fce o pixel vedle */
            e1 += a1;
            e2 += a2;
            e3 += a3;
        }
    }
}

/******************************************************************************
 * Vykresli i-ty trojuhelnik n-teho klicoveho snimku modelu
 * pomoci nove fce studrenDrawTriangle()
 * Pred vykreslenim aplikuje na vrcholy a normaly trojuhelniku
 * aktualne nastavene transformacni matice!
 * Upravte tak, aby se model vykreslil interpolovane dle parametru n
 * (cela cast n udava klicovy snimek, desetinna cast n parametr interpolace
 * mezi snimkem n a n + 1)
 * i - index trojuhelniku
 * n - index klicoveho snimku (float pro pozdejsi interpolaci mezi snimky)
 */

void studrenProjectTriangle(S_Renderer *pRenderer, S_Model *pModel, int i, float n)
{

    S_Coords    aa, bb, cc;             /* souradnice vrcholu po transformaci */
    S_Coords    n_aa, n_bb, n_cc;
    S_Coords    naa, nbb, ncc;          /* normaly ve vrcholech po transformaci */
    S_Coords    n_naa, n_nbb, n_ncc;
    S_Coords    nn;                     /* normala trojuhelniku po transformaci */
    S_Coords    n_nn;
    double      ha, hb, hc;
    double      nFrac;                  /* desatinna cast n */
    int         u1, v1, u2, v2, u3, v3; /* souradnice vrcholu po projekci do roviny obrazovky */
    S_Triangle  * triangle;
    int         vertexOffset, normalOffset; /* offset pro vrcholy a normalove vektory trojuhelniku */
    int         n_vertexOffset, n_normalOffset;
    int         i0, i1, i2, in;             /* indexy vrcholu a normaly pro i-ty trojuhelnik n-teho snimku */
    int         n_i0, n_i1, n_i2, n_in;

    IZG_ASSERT(pRenderer && pModel && i >= 0 && i < trivecSize(pModel->triangles) && n >= 0 );

    nFrac = n - (long) n;

    /* z modelu si vytahneme i-ty trojuhelnik */
    triangle = trivecGetPtr(pModel->triangles, i);

    /* ziskame offset pro vrcholy n-teho snimku */
    vertexOffset = (((int) n) % pModel->frames) * pModel->verticesPerFrame;
    n_vertexOffset = (((int) (n + 1.0)) % pModel->frames) * pModel->verticesPerFrame;

    /* ziskame offset pro normaly trojuhelniku n-teho snimku */
    normalOffset = (((int) n) % pModel->frames) * pModel->triangles->size;
    n_normalOffset = (((int) (n + 1.0)) % pModel->frames) * pModel->triangles->size;

    /* indexy vrcholu pro i-ty trojuhelnik n-teho snimku - pricteni offsetu */
      i0 = triangle->v[ 0 ] + vertexOffset;
      i1 = triangle->v[ 1 ] + vertexOffset;
      i2 = triangle->v[ 2 ] + vertexOffset;
    n_i0 = triangle->v[ 0 ] + n_vertexOffset;
    n_i1 = triangle->v[ 1 ] + n_vertexOffset;
    n_i2 = triangle->v[ 2 ] + n_vertexOffset;

    /* index normaloveho vektoru pro i-ty trojuhelnik n-teho snimku - pricteni offsetu */
    in = triangle->n + normalOffset;
    n_in = triangle->n + n_normalOffset;

    /* transformace vrcholu matici model */
    trTransformVertex(&aa, cvecGetPtr(pModel->vertices, i0));
    trTransformVertex(&bb, cvecGetPtr(pModel->vertices, i1));
    trTransformVertex(&cc, cvecGetPtr(pModel->vertices, i2));

    trTransformVertex(&n_aa, cvecGetPtr(pModel->vertices, n_i0));
    trTransformVertex(&n_bb, cvecGetPtr(pModel->vertices, n_i1));
    trTransformVertex(&n_cc, cvecGetPtr(pModel->vertices, n_i2));

    aa.x += (n_aa.x - aa.x) * nFrac;
    aa.y += (n_aa.y - aa.y) * nFrac;
    aa.z += (n_aa.z - aa.z) * nFrac;

    bb.x += (n_bb.x - bb.x) * nFrac;
    bb.y += (n_bb.y - bb.y) * nFrac;
    bb.z += (n_bb.z - bb.z) * nFrac;

    cc.x += (n_cc.x - cc.x) * nFrac;
    cc.y += (n_cc.y - cc.y) * nFrac;
    cc.z += (n_cc.z - cc.z) * nFrac;

    /* promitneme vrcholy trojuhelniku na obrazovku */
    ha = trProjectVertex(&u1, &v1, &aa);
    hb = trProjectVertex(&u2, &v2, &bb);
    hc = trProjectVertex(&u3, &v3, &cc);

    /* pro osvetlovaci model transformujeme take normaly ve vrcholech */
    trTransformVector(&naa, cvecGetPtr(pModel->normals, i0));
    trTransformVector(&nbb, cvecGetPtr(pModel->normals, i1));
    trTransformVector(&ncc, cvecGetPtr(pModel->normals, i2));

    trTransformVector(&n_naa, cvecGetPtr(pModel->normals, n_i0));
    trTransformVector(&n_nbb, cvecGetPtr(pModel->normals, n_i1));
    trTransformVector(&n_ncc, cvecGetPtr(pModel->normals, n_i2));

    naa.x += (n_naa.x - naa.x) * nFrac;
    naa.y += (n_naa.y - naa.y) * nFrac;
    naa.z += (n_naa.z - naa.z) * nFrac;

    nbb.x += (n_nbb.x - nbb.x) * nFrac;
    nbb.y += (n_nbb.y - nbb.y) * nFrac;
    nbb.z += (n_nbb.z - nbb.z) * nFrac;
    
    ncc.x += (n_ncc.x - ncc.x) * nFrac;
    ncc.y += (n_ncc.y - ncc.y) * nFrac;
    ncc.z += (n_ncc.z - ncc.z) * nFrac;

    /* normalizace normal */
    coordsNormalize(&naa);
    coordsNormalize(&nbb);
    coordsNormalize(&ncc);

    /* transformace normaly trojuhelniku matici model */
    trTransformVector(&nn, cvecGetPtr(pModel->trinormals, in));
    trTransformVector(&n_nn, cvecGetPtr(pModel->trinormals, n_in));

    nn.x += (n_nn.x - nn.x) * nFrac;
    nn.y += (n_nn.y - nn.y) * nFrac;
    nn.z += (n_nn.z - nn.z) * nFrac;

    
    /* normalizace normaly */
    coordsNormalize(&nn);

    /* je troj. privraceny ke kamere, tudiz viditelny? */
    if( !renCalcVisibility(pRenderer, &aa, &nn) )
    {
        /* odvracene troj. vubec nekreslime */
        return;
    }

    /* rasterizace trojuhelniku */
    studrenDrawTriangle(pRenderer,
                    &aa, &bb, &cc,
                    &naa, &nbb, &ncc,
                    u1, v1, u2, v2, u3, v3,
                    triangle->t, ha, hb, hc);
}

/******************************************************************************
* Vraci hodnotu v aktualne nastavene texture na zadanych
* texturovacich souradnicich u, v
* Pro urceni hodnoty pouziva bilinearni interpolaci
* Pro otestovani vraci ve vychozim stavu barevnou sachovnici dle uv souradnic
* u, v - texturovaci souradnice v intervalu 0..1, ktery odpovida sirce/vysce textury
*/

S_RGBA studrenTextureValue( S_StudentRenderer * pRenderer, double u, double v )
{
    double uFil = u * pRenderer->height - 0.5;
    int x = floor(uFil);
    double uR = uFil - x;
    double uRN = 1.0 - uR;

    double vFil = v * pRenderer->width - 0.5;
    int y = floor(vFil);
    double vR = vFil - y;
    double vRN = 1.0 - vR;
    
    S_RGBA p0 = pRenderer->texture[(x*(pRenderer->width) + y)%(pRenderer->width*pRenderer->height)];
    S_RGBA p1 = pRenderer->texture[(x*(pRenderer->width) + y + 1)%(pRenderer->width*pRenderer->height)];
    S_RGBA p2 = pRenderer->texture[((x + 1) * (pRenderer->width) + y + 1)%(pRenderer->width*pRenderer->height)];
    S_RGBA p3 = pRenderer->texture[((x + 1) * (pRenderer->width) + y)%(pRenderer->width*pRenderer->height)];

    S_RGBA col;
    col.red = (p0.red*uRN + p3.red*uR)*vRN + (p1.red*uRN + p2.red*uR)*vR;
    col.green = (p0.green*uRN + p3.green*uR)*vRN + (p1.green*uRN + p2.green*uR)*vR;
    col.blue = (p0.blue*uRN + p3.blue*uR)*vRN + (p1.blue*uRN + p2.blue*uR)*vR;
    col.alpha = 255;

    return col;
}

/******************************************************************************
 ******************************************************************************
 * Funkce pro vyrenderovani sceny, tj. vykresleni modelu
 * Upravte tak, aby se model vykreslil animovane
 * (volani renderModel s aktualizovanym parametrem n)
 */

void renderStudentScene(S_Renderer *pRenderer, S_Model *pModel)
{

    /* test existence frame bufferu a modelu */
    IZG_ASSERT(pModel && pRenderer);

    /* nastavit projekcni matici */
    trProjectionPerspective(pRenderer->camera_dist, pRenderer->frame_w, pRenderer->frame_h);

    /* vycistit model matici */
    trLoadIdentity();

    /* nejprve nastavime posuv cele sceny od/ke kamere */
    trTranslate(0.0, 0.0, pRenderer->scene_move_z);

    /* nejprve nastavime posuv cele sceny v rovine XY */
    trTranslate(pRenderer->scene_move_x, pRenderer->scene_move_y, 0.0);

    /* natoceni cele sceny - jen ve dvou smerech - mys je jen 2D... :( */
    trRotateX(pRenderer->scene_rot_x);
    trRotateY(pRenderer->scene_rot_y);

    /* nastavime material */
    renMatAmbient(pRenderer, &MAT_WHITE_AMBIENT);
    renMatDiffuse(pRenderer, &MAT_WHITE_DIFFUSE);
    renMatSpecular(pRenderer, &MAT_WHITE_SPECULAR);

    /* a vykreslime nas model (ve vychozim stavu kreslime pouze snimek 0) */
    renderModel(pRenderer, pModel, t);
}

/* Callback funkce volana pri tiknuti casovace
 * ticks - pocet milisekund od inicializace */
void onTimer( int ticks )
{
    /* uprava parametru pouzivaneho pro vyber klicoveho snimku
     * a pro interpolaci mezi snimky */
    t = (float) ticks / 100;
}


/*****************************************************************************
 *****************************************************************************/
