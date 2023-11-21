/* THIS CODE WAS MY OWN WORK , IT WAS WRITTEN WITHOUT CONSULTING ANY
SOURCES OUTSIDE OF THOSE APPROVED BY THE INSTRUCTOR . Brett Anwar */
#include "pbm.h"
#include <stdlib.h>

//function creates new PPM image with given width, height and max color val
PPMImage * new_ppmimage( unsigned int w, unsigned int h, unsigned int m )
{
    //allocates memory of size PPMImage
    PPMImage * img = malloc(sizeof(PPMImage));

    img->width = w;
    img->height = h;
    img->max = m;

    //allocates memory for each RGB of the image (rows of pointers)
    img->pixmap[0] = malloc(sizeof(int*) * h);
    img->pixmap[1] = malloc(sizeof(int*) * h);
    img->pixmap[2] = malloc(sizeof(int*) * h);
    
    //allocate memory for columns for each RGB channel
    for(int i=0; i<h; i++){
        img->pixmap[0][i] = malloc(w * sizeof(int));
        img->pixmap[1][i] = malloc(w * sizeof(int));
        img->pixmap[2][i] = malloc(w * sizeof(int));
    }

    return img;
}

//function creates PBM image with given width and height
PBMImage * new_pbmimage( unsigned int w, unsigned int h )
{
    //allocates memory for PMB image
    PBMImage * img = malloc(sizeof(PBMImage));

    img->width = w;
    img->height = h;
    //allocates memory for pixmap (rows of pointers)
    img->pixmap = malloc(sizeof(int*) * h);

    //allocates memory for columns of the pixmap
    for(int i=0; i<h; i++){
        img->pixmap[i] = malloc(w * sizeof(int));
    }

    return img;
}

//function creates PGM image with given width, height and max color val
PGMImage * new_pgmimage( unsigned int w, unsigned int h, unsigned int m )
{
    //allocates memory for PGM image
    PGMImage * img = malloc(sizeof(PGMImage));

    img->width = w;
    img->height = h;
    img->max = m;
    //allocate memory for pixmap (rows of pointers)
    img->pixmap = malloc(sizeof(int*) * h);

    //allocates memory for columns of pixmap
    for(int i=0; i<h; i++){
        img->pixmap[i] = malloc(w * sizeof(unsigned int));
    }

    return img;
}

//function to free memory of PPM image
//we free in the opposite order we allocate in 
void del_ppmimage( PPMImage * p )
{
    //free memory allocated for columns of each RGB channel
    for(int i=0; i<p->height; i++){
        free(p->pixmap[0][i]);
        free(p->pixmap[1][i]);
        free(p->pixmap[2][i]);
    }
    //frees memory allocated for rows in each channel
    free(p->pixmap[0]);
    free(p->pixmap[1]);
    free(p->pixmap[2]);
    //frees memory of PPM image structure
    free(p);
}

//frees memory allocated for PGM image
void del_pgmimage( PGMImage * p )
{
    //frees memory allocated for pixmaps columns 
    for(int i=0; i<p->height; i++){
        free(p->pixmap[i]);
    }
    //frees memory allocated for rows of pixmap
    free(p->pixmap);

    //frees memory of PGM image structure
    free(p);
}

//frees memory allocated for PBM image
void del_pbmimage( PBMImage * p )
{
    //frees memory allocated for columns of pixmap
    for(int i=0; i<p->height; i++){
        free(p->pixmap[i]);
    }
    //frees memory allocated for rows of pixmap
    free(p->pixmap);

    //frees memory of PBM image structure
    free(p);
}

