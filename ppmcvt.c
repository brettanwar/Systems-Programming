/* THIS CODE WAS MY OWN WORK , IT WAS WRITTEN WITHOUT CONSULTING ANY
SOURCES OUTSIDE OF THOSE APPROVED BY THE INSTRUCTOR . Brett Anwar */

#include "pbm.h"
#include <stdio.h>
#include <getopt.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

//declaration of transformation functions 
PBMImage * PBM_Conversion(const PPMImage *inputImage, int ppmMax);
PGMImage * PGM_Conversion(const PPMImage *inputImage, int ppmMax, int pgmMax);
void Isolation(PPMImage *image, const char *color);
void Removal(PPMImage *image, const char *color);
void Sepia(PPMImage *image);
void Mirror(PPMImage *image);
PPMImage* Thumbnail(PPMImage *image, int n);
PPMImage* N_Tiles(PPMImage *image, int n);


int main( int argc, char *argv[] )
{
    //flags to represent which processes to use 
    bool pbm_flag = false;
    bool pgm_flag = false;
    bool isolate_flag = false;
    bool remove_flag = false;
    bool sepia_flag = false;
    bool mirror_flag = false;
    bool thumbnail_flag = false;
    bool n_tile_flag = false;
    bool output_flag = false;

    //values used to store string/int of flags to be passed to functions later on 
    char remove[1000];
    int grayscale = 0;
    int thumbnail_scaler = 0;
    int n_tile_scaler = 0;
    char isolate[1000];
    char output_filename[1000];
    char input_filename[1000];
    int x = 0;

    //loop through arguments and update flags and parse errors
    while ((x = getopt(argc, argv, ":g:i:r:t:n:o:bsm")) != -1){
        switch(x){
        case 'g':
            //check if grayscale is in allowed range
            if ((atoi(optarg) > 0 && atoi(optarg) <= 65536) && isdigit(optarg[0])){
                pgm_flag = true;
                grayscale = atoi(optarg);
            }else{
                fprintf(stderr, "Error: Invalid max grayscale pixel value: %s; must be less than 65,536\n", optarg);
                exit(1);
            }
            break;
        case 'i':
            //check if valid channel (red, green or blue)
            if (strcmp(optarg, "red") == 0 || strcmp(optarg, "green") == 0 || strcmp(optarg, "blue") == 0) {
                strcpy(isolate, optarg);
                isolate_flag = true;
            } else{
                strcpy(isolate, optarg);
                fprintf(stderr, "Error: Invalid channel specification: (%s); should be 'red', 'green', or 'blue'\n", isolate);
                exit(1);
            }
            break;
        case 'r':
            //check if valid channel (red, green or blue)
            if (strcmp(optarg, "red") == 0 || strcmp(optarg, "green") == 0 || strcmp(optarg, "blue") == 0) {
                strcpy(remove, optarg);
                remove_flag = true;
            } else{
                fprintf(stderr, "Error: Invalid channel specification: %s; should be ‘red’, ‘green’, or ‘blue’\n", remove);
                exit(1);
            }
            break;
        case 't':
            //check if thumnail size given is within range
            if (atoi(optarg) >= 0 && atoi(optarg) <= 8){
                thumbnail_flag = true;
                thumbnail_scaler = atoi(optarg);
            }else{
                fprintf(stderr, "Error: Invalid scale factor: %d; must be 1-8\n", atoi(optarg));
                exit(1);
            }
            break;
        case 'n':
            //check if number of tiles given is within range
            if (atoi(optarg) >= 0 && atoi(optarg) <= 8){
                n_tile_flag = true;
                n_tile_scaler = atoi(optarg);
            }else{
                fprintf(stderr, "Error: Invalid scale factor: %d; must be 1-8\n", atoi(optarg));
                exit(1);
            }
            break;
        case 'o':
            //copies name of output file
            if (optarg){
                strcpy(output_filename, optarg);
                output_flag = true;
            }
            else{
                fprintf(stderr, "Error: No output file specified\n");
                exit(1);
            }
            break;
        case 'b':
            pbm_flag = true;
            break;
        case 's':
            sepia_flag = true;
            break;
        case 'm':
            mirror_flag = true;
            break;
        case '?': //means uncreognized option
            fprintf(stderr, "Usage: ppmcvt [-bgirsmtno] [FILE]\n");
            exit(1);
            break;
        case ':': //means missing argument
            fprintf(stderr, "Missing argument for option -%c\n", optopt);
            exit(1);
            break;
        default:
            break;
        }
        //check at end of each iteration of the  loop to ensure only one transformation is given
        if (pbm_flag + pgm_flag + isolate_flag + remove_flag + sepia_flag + mirror_flag + thumbnail_flag + n_tile_flag > 1){
            fprintf(stderr, "Error: Multiple transformations specified\n");
            exit(1);
        }
    }

    //if no transformation is specified we default to pbm
    if (pbm_flag + pgm_flag + isolate_flag + remove_flag + sepia_flag + mirror_flag + thumbnail_flag + n_tile_flag == 0){
            pbm_flag = true;
        }

    //checks to ensure that the mandatory -o argument is included 
    if (output_flag == 0){
        fprintf(stderr, "Error: No output file specified\n");
        exit(1);
    }

     //checks for an argument that is not associated to a flag to ensure there is input
    if (optind + 1 > argc){
        fprintf(stderr, "Error: No input file specified\n");
        exit(1);
    }

    //copies input file into input_filename
    strcpy(input_filename, argv[optind]);

    //reads input file into *inputImage
    PPMImage *inputImage = read_ppmfile(input_filename);


    //cases for each flag being true
    /*
    General structure:
    if flag:
        call function associated to given flag
            if output file is required allocatememory for it and store result there
            if no output is required then simply modify input 
        call write function to write input/output file to specified file from -o
        call delete on input file to free memory
            if output file: do the same on that
    */

    if (pbm_flag){
        PBMImage *outputImage = PBM_Conversion(inputImage, inputImage->max);
        write_pbmfile(outputImage, output_filename);
        del_ppmimage(inputImage);
        del_pbmimage(outputImage);
    }
    else if (pgm_flag){
        PGMImage *outputImage = PGM_Conversion(inputImage, inputImage->max, grayscale);
        write_pgmfile(outputImage, output_filename);
        del_pgmimage(outputImage);
        del_ppmimage(inputImage);
    }
    else if(isolate_flag){
        Isolation(inputImage, isolate);
        write_ppmfile(inputImage, output_filename);
        del_ppmimage(inputImage);
    }
    else if(remove_flag){
        Removal(inputImage, remove);
        write_ppmfile(inputImage, output_filename);
        del_ppmimage(inputImage);
    }
    else if(sepia_flag){
        Sepia(inputImage);
        write_ppmfile(inputImage, output_filename);
        del_ppmimage(inputImage);
    }
    else if(mirror_flag){
        Mirror(inputImage);
        write_ppmfile(inputImage, output_filename);
        del_ppmimage(inputImage);
    }
    else if(thumbnail_flag){
        PPMImage *thumbnailImage = Thumbnail(inputImage, thumbnail_scaler);
        write_ppmfile(thumbnailImage, output_filename);
        del_ppmimage(inputImage);
        del_ppmimage(thumbnailImage);
    }
    else if(n_tile_flag){
        PPMImage *tiledImage = N_Tiles(inputImage, n_tile_scaler);
        write_ppmfile(tiledImage, output_filename);
        del_ppmimage(inputImage);
        del_ppmimage(tiledImage);
    }


    return 0;
}


//function to convert from PPM to PBM
PBMImage * PBM_Conversion(const PPMImage *inputImage, int ppmMax) {
    //allocate memory for Pbm image to output  
    PBMImage *outputImage = new_pbmimage(inputImage->width, inputImage->height);
    float fppmMax = ppmMax / 2.0;
    //loop through the pixmap and collect rgb values 
    for (int y = 0; y < inputImage->height; y++) {
        for (int x = 0; x < inputImage->width; x++) {
            int red = inputImage->pixmap[0][y][x];
            int green = inputImage->pixmap[1][y][x];
            int blue = inputImage->pixmap[2][y][x];
            
            //compute average and assign 1 or 0 based on avg
            float average = (red + green + blue) / 3.0;
            if (average < fppmMax) {
                outputImage->pixmap[y][x] = 1; 
            } else {
                outputImage->pixmap[y][x] = 0; 
            }
        }
    }
    return outputImage;
}

//function to convert fromm PPM to PGM 
PGMImage * PGM_Conversion(const PPMImage *inputImage, int ppmMax, int pgmMax) {
    //allocate memory for Pgm image to output 
    PGMImage *outputImage = new_pgmimage(inputImage->width, inputImage->height, pgmMax);
    float fppmMax = (float)ppmMax;
    float fpgmMax = (float)pgmMax;
    //loops through pixmap to calculate average 
    for (int y = 0; y < inputImage->height; y++) {
        for (int x = 0; x < inputImage->width; x++) {
            int red = inputImage->pixmap[0][y][x];
            int green = inputImage->pixmap[1][y][x];
            int blue = inputImage->pixmap[2][y][x];
            float average = (red + green + blue) / 3.0;
            //scales average using given maximum values 
            outputImage->pixmap[y][x] = (int)((average * fpgmMax) / fppmMax);
        }
    }
    return outputImage;
}

//function to isolate given color in image
void Isolation(PPMImage *image, const char *color) {
    for (int y = 0; y < image->height; y++) {
        for (int x = 0; x < image->width; x++) {
            //set all other RGB values to 0 based on given input color
            if (strcmp(color, "red") == 0) {
                image->pixmap[1][y][x] = 0;  
                image->pixmap[2][y][x] = 0;  
            } else if (strcmp(color, "green") == 0) {
                image->pixmap[0][y][x] = 0;  
                image->pixmap[2][y][x] = 0;  
            } else if (strcmp(color, "blue") == 0) {
                image->pixmap[0][y][x] = 0;  
                image->pixmap[1][y][x] = 0; 
            }
        }
    }
}

//function to remove a given channel 
void Removal(PPMImage *image, const char *color) {
    for (int y = 0; y < image->height; y++) {
        for (int x = 0; x < image->width; x++) {
            //sets all values of specified color to 0
            if (strcmp(color, "red") == 0) {
                image->pixmap[0][y][x] = 0;  
            } else if (strcmp(color, "green") == 0) {
                image->pixmap[1][y][x] = 0;  
            } else if (strcmp(color, "blue") == 0) {
                image->pixmap[2][y][x] = 0;
            }
        }
    }
}

//function to apply sepia filter to a PPM image
void Sepia(PPMImage *image) {
    for (int y = 0; y < image->height; y++) {
        for (int x = 0; x < image->width; x++) {
            int oldR = image->pixmap[0][y][x];
            int oldG = image->pixmap[1][y][x];
            int oldB = image->pixmap[2][y][x];

            // update RGB values by applyign Sepia formula 
            int newR = (int)(0.393 * oldR + 0.769 * oldG + 0.189 * oldB);
            int newG = (int)(0.349 * oldR + 0.686 * oldG + 0.168 * oldB);
            int newB = (int)(0.272 * oldR + 0.534 * oldG + 0.131 * oldB);

            // Max value for RGB is 255
            //if any values are over 255 we lower them to 255 using ternary operator
            image->pixmap[0][y][x] = (newR > 255) ? 255 : newR;
            image->pixmap[1][y][x] = (newG > 255) ? 255 : newG;
            image->pixmap[2][y][x] = (newB > 255) ? 255 : newB;
        }
    }
}

//function to vertically mirror image onto itsellf 
void Mirror(PPMImage *image) {
    int mid = image->width / 2;

    for (int y = 0; y < image->height; y++) {
        for (int x = 0; x < mid; x++) {
            //copy RGB values from the left to right side of given image
            image->pixmap[0][y][image->width - x - 1] = image->pixmap[0][y][x];
            image->pixmap[1][y][image->width - x - 1] = image->pixmap[1][y][x];
            image->pixmap[2][y][image->width - x - 1] = image->pixmap[2][y][x];
        }
    }
}

//function to create thumbnail of given image
PPMImage* Thumbnail(PPMImage *image, int n) {
    int newWidth = image->width / n;
    int newHeight = image->height / n;
    // create new ppm for thumbnail
    PPMImage *thumbnail = new_ppmimage(newWidth, newHeight, image->max);

    for (int y = 0; y < newHeight; y++) {
        for (int x = 0; x < newWidth; x++) {
            //transfer over values from image to thumbnail
            thumbnail->pixmap[0][y][x] = image->pixmap[0][y * n][x * n];
            thumbnail->pixmap[1][y][x] = image->pixmap[1][y * n][x * n];
            thumbnail->pixmap[2][y][x] = image->pixmap[2][y * n][x * n];
        }
    }
    return thumbnail;
}

//function to create n-tiles of thumbnails 
PPMImage* N_Tiles(PPMImage *image, int n) {
    PPMImage *tiledImage = new_ppmimage(image->width, image->height, image->max);
    //calls thumbnail function
    PPMImage *thumbnail = Thumbnail(image, n);

    int thumbnailWidth = image->width / n;
    int thumbnailHeight = image->height / n;

    for (int y = 0; y < image->height; y++) {
        for (int x = 0; x < image->width; x++) {
            // gets corresponding pixel from thumbnail
            int thumbnailY = y % thumbnailHeight;
            int thumbnailX = x % thumbnailWidth;

            tiledImage->pixmap[0][y][x] = thumbnail->pixmap[0][thumbnailY][thumbnailX];
            tiledImage->pixmap[1][y][x] = thumbnail->pixmap[1][thumbnailY][thumbnailX];
            tiledImage->pixmap[2][y][x] = thumbnail->pixmap[2][thumbnailY][thumbnailX];
        }
    }

    // Free the memory allocated for the thumbnail
    del_ppmimage(thumbnail);

    return tiledImage;
}










