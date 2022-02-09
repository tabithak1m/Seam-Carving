#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "seamcarving.h"


void calc_energy(struct rgb_img *im, struct rgb_img **grad){
    int width = im->width;
    int height = im->height;
    create_img(grad, height, width);
    for(int x = 0; x < width; x++){
        for(int y = 0; y < height; y++){
            int x_left = x-1;
            int x_right = x+1;
            if (x==0){
                x_left = width-1;
            }
            if (x==width-1){
                x_right = 0;
            }
            int Rx = get_pixel(im, y, x_right, 0) - get_pixel(im, y, x_left, 0);
            int Gx = get_pixel(im, y, x_right, 1) - get_pixel(im, y, x_left, 1);
            int Bx = get_pixel(im, y, x_right, 2) - get_pixel(im, y, x_left, 2);

            int y_up = y-1;
            int y_down = y+1;
            if (y==0){
                y_up = height-1;
            }
            if (y==height-1){
                y_down = 0;
            }
            int Ry = get_pixel(im, y_down, x, 0) - get_pixel(im, y_up, x, 0);
            int Gy = get_pixel(im, y_down, x, 1) - get_pixel(im, y_up, x, 1);
            int By = get_pixel(im, y_down, x, 2) - get_pixel(im, y_up, x, 2);

            int x2 = Rx*Rx + Gx*Gx + Bx*Bx;
            int y2 = Ry*Ry + Gy*Gy + By*By;

            uint8_t energy = (uint8_t)(sqrt(x2+y2)/10);
            set_pixel(*grad, y, x, energy, energy, energy);
        }
    }
}

void dynamic_seam(struct rgb_img *grad, double **best_arr){
    int width = grad->width;
    int height = grad->height;
    *best_arr = (double *) malloc(sizeof(double) * width * height);
    for(int y = 0; y < height; y++) {
        for(int x = 0; x < width; x++){
            double min = 0;
            if (y > 0){
                double up = (*best_arr)[(y-1)*width+x];
                min = up;
                if (x>0) {
                    double upleft = (*best_arr)[(y-1)*width+x-1];
                    if (upleft < min){ min = upleft;}
                }
                if (x<width-1) {
                    double upright = (*best_arr)[(y-1)*width+x+1];
                    if (upright < min){ min = upright;}
                }
            }
            (*best_arr)[y*width+x] = (get_pixel(grad,y,x,0) + min);
        }
    }
}

void recover_path(double *best, int height, int width, int **path){
    int *path1 = (int*) malloc(sizeof(int) * height);
    *path = path1;
    for(int y = height-1; y >= 0; y--) {
        int min = 0;
        if (y==height-1) {
            for (int x = 0; x < width; x++) {
                if (best[y*width+x] < best[y*width+min]){ min = x;}
            }

        } else {
            for (int x = -1; x < 2; x++) {
                int offset = (*path)[y+1];
                if (offset+x < 0 || offset+x >= width){continue;}
                if (best[y*width+offset+x] < best[y*width+offset+min]){ min = x;}
            }
            min += (*path)[y+1];
        }
        (*path)[y] = min;
    }
}

void remove_seam(struct rgb_img *src, struct rgb_img **dest, int *path){
    int width = src->width;
    int height = src->height;
    create_img(dest, src->height, src->width-1);
    for(int y = 0; y < height; y++) {
        for(int j = 0; j < width; j++) {
            int x = j;
            if (x==path[y]){
                continue;
            }
            if (x>path[y]){
               x-=1;
            }
            set_pixel(*dest,y,x,get_pixel(src,y,j,0),get_pixel(src,y,j,1),get_pixel(src,y,j,2));
        }
    }
}
