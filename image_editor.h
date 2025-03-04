// Copyright @lucabotez

#ifndef IMAGE_EDITOR_H
#define IMAGE_EDITOR_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <ctype.h>

#define MAX 100

// the structure stores the color information for each pixel
struct pixels {
    double red, green, blue, gray;
};

// the structure stores information about the loaded photo
struct photo {
	char magic_word[2]; // P2, P3, P5, P6
	int type, height, width;
	int init_h_u, init_h_d, init_w_l, init_w_r;
	// initial dimensions of the picture; modified only after the CROP operation

	int sel_h_u, sel_h_d, sel_w_l, sel_w_r;
	// photo selection dimensions; modified only after the SELECT operation

	struct pixels **pixel_matrix, **new_matrix;
	// pixel_matrix = the matrix in which the photo is loaded
	// new_matrix = temporary matrix
};

// function headers
void copy_values(struct photo *p, int type);
double clamp(double number);
int is_pow(int number);
void reset_selection(struct photo *p);
void free_photo(struct photo p);
void alloc_photo(struct photo *p);
void alloc_new_matrix(struct photo *p, int height, int width);
void skip_comments(FILE *in);
void ascii(struct photo *p, FILE *in);
void binary(struct photo *p, FILE *in);
void load_photo(struct photo *p, char file[], int *counter);
int check_selected_value(char *value);
void selection(struct photo *p);
void crop(struct photo *p);
void edge(struct photo *p);
void sharpen(struct photo *p);
void blur(struct photo *p);
void gaussian_blur(struct photo *p);
void apply_effect(struct photo *p);
void histogram(struct photo p);
void equalize(struct photo *p);
void rotate_square(struct photo *p, int angle, int is_negative);
void rotate_picture(struct photo *p, int angle, int is_negative);
void rotate(struct photo *p);
void save(struct photo p);

#endif // IMAGE_EDITOR_H
