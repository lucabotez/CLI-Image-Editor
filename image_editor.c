// Copyright @lucabotez

#include "image_editor.h"


// function used to copy the values from one matrix to another
// type == 0 => pixel_matrix = new_matrix
// type == 1 => new_matrix = pixel_matrix
void copy_values(struct photo *p, int type)
{
	int i, j;
	if (type == 0)
		for (i = 0; i < p->height; i++)
			for (j = 0; j < p->width; j++) {
				p->pixel_matrix[i][j].gray = p->new_matrix[i][j].gray;
				p->pixel_matrix[i][j].red = p->new_matrix[i][j].red;
				p->pixel_matrix[i][j].green = p->new_matrix[i][j].green;
				p->pixel_matrix[i][j].blue = p->new_matrix[i][j].blue;
			}
	else if (type == 1)
		for (i = 0; i < p->height; i++)
			for (j = 0; j < p->width; j++) {
				p->new_matrix[i][j].gray = p->pixel_matrix[i][j].gray;
				p->new_matrix[i][j].red = p->pixel_matrix[i][j].red;
				p->new_matrix[i][j].green = p->pixel_matrix[i][j].green;
				p->new_matrix[i][j].blue = p->pixel_matrix[i][j].blue;
			}
}

// function used to ensure that a number does not exceed the [0 - 255]
// interval
double clamp(double number)
{
	if (number > 255)
		return 255;
	if (number < 0)
		return 0;
	return number;
}

// function that checks if a number is a power of 2, used in HISTOGRAM
int is_pow(int number)
{
	while (number != 1) {
		if (number % 2 != 0)
			return 0;
		number = number / 2;
	}
	return 1;
}

// function that resets the current selection to the initial size of the picture
void reset_selection(struct photo *p)
{
	p->sel_h_d = p->init_h_d;
	p->sel_h_u = p->init_h_u;
	p->sel_w_l = p->init_w_l;
	p->sel_w_r = p->init_w_r;
}

// function that frees the pixel matrix
void free_photo(struct photo p)
{
	int i;
	for (i = p.init_h_d; i < p.init_h_u; i++)
		free(p.pixel_matrix[i]);
	free(p.pixel_matrix);
}

// function that allocates the pixel matrix
void alloc_photo(struct photo *p)
{
	int i, j;
	p->pixel_matrix = (struct pixels **)malloc(p->height *
	sizeof(struct pixels *));
	if (!p->pixel_matrix)
		exit(-1);

	for (i = 0; i < p->height; i++) {
		p->pixel_matrix[i] = (struct pixels *)malloc(p->width *
		sizeof(struct pixels));
		if (!p->pixel_matrix) {
			for (j = 0; j < i; j++)
				free(p->pixel_matrix[j]);
			free(p->pixel_matrix);
			exit(-1);
		}
	}
}

// function that allocates the temp matrix with a given height and width
void alloc_new_matrix(struct photo *p, int height, int width)
{
	int i, j;
	p->new_matrix = (struct pixels **)malloc(height *
	sizeof(struct pixels *));
	if (!p->new_matrix) {
		free_photo(*p);
		exit(-1);
	}

	for (i = 0; i < height; i++) {
		p->new_matrix[i] = (struct pixels *)malloc(width *
		sizeof(struct pixels));
		if (!p->new_matrix) {
			for (j = 0; j < i; j++)
				free(p->new_matrix[j]);
			free(p->new_matrix);
			free_photo(*p);
			exit(-1);
		}
	}
}

// function that skips the commented lines in the photo file
void skip_comments(FILE *in)
{
	char c;
	fpos_t curr_position;
	fgetpos(in, &curr_position);
	fscanf(in, "%c", &c);

	if (c == '\n')
		fscanf(in, "%c", &c);

	// if '#' is met, read characters until '\n' is met
	if (c == '#')
		while (c != '\n')
			fscanf(in, "%c", &c);
	else
		// if '#' is not met, go back to the initial position to not lose values
		fsetpos(in, &curr_position);
}

// function that loads an ascii-formatted photo in the pixel matrix
void ascii(struct photo *p, FILE *in)
{
	int i, j;
	if (p->type == 0) {
		for (i = 0; i < p->height; i++)
			for (j = 0; j < p->width; j++) {
				int gray_value;
				fscanf(in, "%d", &gray_value);
				p->pixel_matrix[i][j].gray = (double)gray_value;
			}
	} else {
		for (i = 0; i < p->height; i++)
			for (j = 0; j < p->width; j++) {
				int red_value, green_value, blue_value;
				fscanf(in, "%d%d%d", &red_value, &green_value, &blue_value);
				p->pixel_matrix[i][j].red = (double)red_value;
				p->pixel_matrix[i][j].green = (double)green_value;
				p->pixel_matrix[i][j].blue = (double)blue_value;
			}
	}
}

// function that loads an binary-formatted photo in the pixel matrix
void binary(struct photo *p, FILE *in)
{
	int i, j;
	if (p->type == 0) {
		for (i = 0; i < p->height; i++)
			for (j = 0; j < p->width; j++) {
				unsigned char gray_value;
				fread(&gray_value, sizeof(unsigned char), 1, in);
				p->pixel_matrix[i][j].gray = (double)gray_value;
			}
	} else {
		for (i = 0; i < p->height; i++)
			for (j = 0; j < p->width; j++) {
				unsigned char red_value, green_value, blue_value;

				fread(&red_value, sizeof(unsigned char), 1, in);
				p->pixel_matrix[i][j].red = (double)red_value;
				fread(&green_value, sizeof(unsigned char), 1, in);
				p->pixel_matrix[i][j].green = (double)green_value;
				fread(&blue_value, sizeof(unsigned char), 1, in);
				p->pixel_matrix[i][j].blue = (double)blue_value;
			}
	}
}

// function that loads a photo in the pixel matrix by using the ascii / binary
// helper function accordingly
void load_photo(struct photo *p, char file[], int *counter)
{
	FILE *in = fopen(file, "rt");
	if (!in) {
		printf("Failed to load %s\n", file);

		// if an image is already loaded, it is freed and a new one takes its place
		if (*counter == 1)
			free_photo(*p);
		*counter = 0;
		return;
	}

	int max_value; // color intensity max color (255)

	skip_comments(in);

	fscanf(in, "%s", p->magic_word);
	if (strcmp(p->magic_word, "P2") == 0 || (strcmp(p->magic_word, "P5") == 0))
		p->type = 0; // grayscale picture
	else
		p->type = 1; // color picture

	skip_comments(in);
	fscanf(in, "%d%d", &p->width, &p->height);

	skip_comments(in);
	fscanf(in, "%d ", &max_value);

	skip_comments(in);

	// initial photo dimensions
	p->init_h_d = 0;
	p->init_h_u = p->height;
	p->init_w_l = 0;
	p->init_w_r = p->width;

	// photo selection dimensions; initially no selection is made
	p->sel_h_d = 0;
	p->sel_h_u = p->height;
	p->sel_w_l = 0;
	p->sel_w_r = p->width;

	alloc_photo(p);
	if (strcmp(p->magic_word, "P2") == 0 || strcmp(p->magic_word, "P3") == 0) {
		ascii(p, in);
		fclose(in);
	} else {
		// reading from the current position in the binary file
		// (skipped the comments)
		FILE *binary_in = fopen(file, "rb");

		if (!binary_in) {
			fclose(in);
			free_photo(*p);
			exit(-1);
		}
		fpos_t position;
		fgetpos(in, &position);
		fsetpos(binary_in, &position);
		binary(p, binary_in);
		fclose(in);
		fclose(binary_in);
	}
	printf("Loaded %s\n", file);
	*counter = 1; // an image is loaded
}

// function that checks if a string represents a number
int check_selected_value(char *value)
{
	if (isdigit(value[0])) {
		return 1;
	} else if (value[0] == '-') { // negative number
		if (isdigit(value[1]))
			return 1;
	}
	return 0;
}

// function that saves the selection dimensions or resets it
void selection(struct photo *p)
{
	char *c;
	int x1, y1, x2, y2; // edges of the selection interval

	// selection parameters
	c = strtok(NULL, "\n ");

	if (!c) { // checks for valid parameters
		printf("Invalid command\n");
		return;
	} else if (strcmp(c, "ALL") == 0) {
		printf("Selected ALL\n");
		reset_selection(p);
	} else {
		if (check_selected_value(c)) {
			x1 = atoi(c);
		} else {
			printf("Invalid command\n");
			return;
		}
		c = strtok(NULL, "\n ");
		if (!c) {
			printf("Invalid command\n");
			return;
		} else if (check_selected_value(c)) {
			y1 = atoi(c);
		} else {
			printf("Invalid command\n");
			return;
		}
		c = strtok(NULL, "\n ");
		if (!c) {
			printf("Invalid command\n");
			return;
		} else if (check_selected_value(c)) {
			x2 = atoi(c);
		} else {
			printf("Invalid command\n");
			return;
		}
		c = strtok(NULL, "\n ");
		if (!c) {
			printf("Invalid command\n");
			return;
		} else if (check_selected_value(c)) {
			y2 = atoi(c);
		} else {
			printf("Invalid command\n");
			return;
		}
		
		// checks if the numbers are included in the given intervals
		if ((x1 < p->init_w_l || x1 > p->init_w_r) ||
		    (x2 < p->init_w_l || x2 > p->init_w_r)) {
			printf("Invalid set of coordinates\n");
			return;
		}
		if ((y1 < p->init_h_d || y1 > p->init_h_u) ||
		    (y2 < p->init_h_d || y2 > p->init_h_u)) {
			printf("Invalid set of coordinates\n");
			return;
		}
		if (x1 == x2 || y1 == y2) {
			printf("Invalid set of coordinates\n");
			return;
		}
		if (x1 > x2) { // assuring the correct order of the interval edges
			int aux = x1;
			x1 = x2;
			x2 = aux;
		}
		if (y1 > y2) {
			int aux = y1;
			y1 = y2;
			y2 = aux;
		}

		// saving the selection
		p->sel_w_l = x1;
		p->sel_w_r = x2;
		p->sel_h_d = y1;
		p->sel_h_u = y2;
		printf("Selected %d %d %d %d\n", x1, y1, x2, y2);
	}
}

// function that crops the dimensions of the loaded photo based on the current
// selection
void crop(struct photo *p)
{
	int i, j;
	p->height = p->sel_h_u - p->sel_h_d;
	p->width = p->sel_w_r - p->sel_w_l;

	// matrix with the newly cropped image
	alloc_new_matrix(p, p->height, p->width);

	for (i = 0; i < p->height; i++)
		for (j = 0; j < p->width; j++) {
			p->new_matrix[i][j].gray = p->pixel_matrix[i + p->sel_h_d]
			[j + p->sel_w_l].gray;
			p->new_matrix[i][j].red = p->pixel_matrix[i + p->sel_h_d]
			[j + p->sel_w_l].red;
			p->new_matrix[i][j].green = p->pixel_matrix[i + p->sel_h_d]
			[j + p->sel_w_l].green;
			p->new_matrix[i][j].blue = p->pixel_matrix[i + p->sel_h_d]
			[j + p->sel_w_l].blue;
		}

	free_photo(*p);
	alloc_photo(p);

	copy_values(p, 0); // initial matrix values

	for (i = 0; i < p->height; i++)
		free(p->new_matrix[i]);
	free(p->new_matrix);

	// modifying the dimension, after the CROP operation the initial ones are
	// changed too
	p->init_h_d = 0;
	p->init_h_u = p->height;
	p->init_w_l = 0;
	p->init_w_r = p->width;

	p->sel_h_d = 0;
	p->sel_h_u = p->height;
	p->sel_w_l = 0;
	p->sel_w_r = p->width;

	printf("Image cropped\n");
}

// function that applies an image kernel that creates an edge detection effect
void edge(struct photo *p)
{
	if (p->type == 0) { // se aplica doar pe imagini color
		printf("Easy, Charlie Chaplin\n");
		return;
	}
	int i, j, k, l;

	// corresponding filter kernel
	double matrix[3][3] = {{-1, -1, -1}, {-1, 8, -1}, {-1, -1, -1}};
	double s_r = 0, s_g = 0, s_b = 0;

	alloc_new_matrix(p, p->height, p->width);

	for (i = p->sel_h_d; i < p->sel_h_u; i++)
		for (j = p->sel_w_l; j < p->sel_w_r; j++) {
			// unchanged photo edge case
			if ((i == 0 || j == 0) ||
			    (i == p->height - 1 || j == p->width - 1)) {
				p->new_matrix[i][j].red = p->pixel_matrix[i][j].red;
				p->new_matrix[i][j].green = p->pixel_matrix[i][j].green;
				p->new_matrix[i][j].blue = p->pixel_matrix[i][j].blue;
			} else {
				// new values for each color channel
				s_r = 0;
				s_g = 0;
				s_b = 0;

				// iterating through all the neighbors along with the pixel itself
				for (k = -1; k < 2; k++) {
					for (l = -1; l < 2; l++) {
						s_r = s_r + p->pixel_matrix[i + k][j + l].red *
						matrix[k + 1][l + 1];
						s_g = s_g + p->pixel_matrix[i + k][j + l].green *
						matrix[k + 1][l + 1];
						s_b = s_b + p->pixel_matrix[i + k][j + l].blue *
						matrix[k + 1][l + 1];
					}
				}

				// saving the values
				p->new_matrix[i][j].red = clamp(s_r);
				p->new_matrix[i][j].green = clamp(s_g);
				p->new_matrix[i][j].blue = clamp(s_b);
			}
		}

	for (i = p->sel_h_d; i < p->sel_h_u; i++) {
		for (j = p->sel_w_l; j < p->sel_w_r; j++) {
			p->pixel_matrix[i][j].red = p->new_matrix[i][j].red;
			p->pixel_matrix[i][j].green = p->new_matrix[i][j].green;
			p->pixel_matrix[i][j].blue = p->new_matrix[i][j].blue;
		}
	}

	for (i = 0; i < p->height; i++)
		free(p->new_matrix[i]);
	free(p->new_matrix);

	printf("APPLY EDGE done\n");
}

// function that applies an image kernel that creates a sharpen effect
void sharpen(struct photo *p)
{
	if (p->type == 0) { // only applied on colored images
		printf("Easy, Charlie Chaplin\n");
		return;
	}
	int i, j, k, l;
	// corresponding filter kernel
	double matrix[3][3] = {{0, -1, 0}, {-1, 5, -1}, {0, -1, 0}};
	double s_r = 0, s_g = 0, s_b = 0;

	alloc_new_matrix(p, p->height, p->width);

	for (i = p->sel_h_d; i < p->sel_h_u; i++)
		for (j = p->sel_w_l; j < p->sel_w_r; j++) {
			if ((i == 0 || j == 0) ||
			    (i == p->height - 1 || j == p->width - 1)) {
				p->new_matrix[i][j].red = p->pixel_matrix[i][j].red;
				p->new_matrix[i][j].green = p->pixel_matrix[i][j].green;
				p->new_matrix[i][j].blue = p->pixel_matrix[i][j].blue;
			} else {
				s_r = 0;
				s_g = 0;
				s_b = 0;

				for (k = -1; k < 2; k++) {
					for (l = -1; l < 2; l++) {
						s_r = s_r + p->pixel_matrix[i + k][j + l].red *
						matrix[k + 1][l + 1];
						s_g = s_g + p->pixel_matrix[i + k][j + l].green *
						matrix[k + 1][l + 1];
						s_b = s_b + p->pixel_matrix[i + k][j + l].blue *
						matrix[k + 1][l + 1];
					}
				}

				p->new_matrix[i][j].red = clamp(s_r);
				p->new_matrix[i][j].green = clamp(s_g);
				p->new_matrix[i][j].blue = clamp(s_b);
			}
		}

	for (i = p->sel_h_d; i < p->sel_h_u; i++) {
		for (j = p->sel_w_l; j < p->sel_w_r; j++) {
			p->pixel_matrix[i][j].red = p->new_matrix[i][j].red;
			p->pixel_matrix[i][j].green = p->new_matrix[i][j].green;
			p->pixel_matrix[i][j].blue = p->new_matrix[i][j].blue;
		}
	}

	for (i = 0; i < p->height; i++)
		free(p->new_matrix[i]);
	free(p->new_matrix);

	printf("APPLY SHARPEN done\n");
}

// function that applies an image kernel that creates a blur effect
void blur(struct photo *p)
{
	if (p->type == 0) { // only applied on colored images
		printf("Easy, Charlie Chaplin\n");
		return;
	}
	int i, j, k, l;
	double nr = (double)1 / 9;
	// corresponding filter kernel
	double matrix[3][3] = {{nr, nr, nr}, {nr, nr, nr}, {nr, nr, nr}};
	double s_r = 0, s_g = 0, s_b = 0;

	alloc_new_matrix(p, p->height, p->width);

	for (i = p->sel_h_d; i < p->sel_h_u; i++)
		for (j = p->sel_w_l; j < p->sel_w_r; j++) {
			if ((i == 0 || j == 0) ||
			    (i == p->height - 1 || j == p->width - 1)) {
				p->new_matrix[i][j].red = p->pixel_matrix[i][j].red;
				p->new_matrix[i][j].green = p->pixel_matrix[i][j].green;
				p->new_matrix[i][j].blue = p->pixel_matrix[i][j].blue;
			} else {
				s_r = 0;
				s_g = 0;
				s_b = 0;

				for (k = -1; k < 2; k++) {
					for (l = -1; l < 2; l++) {
						s_r = s_r + p->pixel_matrix[i + k][j + l].red *
						matrix[k + 1][l + 1];
						s_g = s_g + p->pixel_matrix[i + k][j + l].green *
						matrix[k + 1][l + 1];
						s_b = s_b + p->pixel_matrix[i + k][j + l].blue *
						matrix[k + 1][l + 1];
					}
				}

				p->new_matrix[i][j].red = clamp(round(s_r));
				p->new_matrix[i][j].green = clamp(round(s_g));
				p->new_matrix[i][j].blue = clamp(round(s_b));
			}
		}

	for (i = p->sel_h_d; i < p->sel_h_u; i++) {
		for (j = p->sel_w_l; j < p->sel_w_r; j++) {
			p->pixel_matrix[i][j].red = p->new_matrix[i][j].red;
			p->pixel_matrix[i][j].green = p->new_matrix[i][j].green;
			p->pixel_matrix[i][j].blue = p->new_matrix[i][j].blue;
		}
	}

	for (i = 0; i < p->height; i++)
		free(p->new_matrix[i]);
	free(p->new_matrix);

	printf("APPLY BLUR done\n");
}

// function that applies an image kernel that creates a gaussian-blur effect
void gaussian_blur(struct photo *p)
{
	if (p->type == 0) { // only applied on colored images
		printf("Easy, Charlie Chaplin\n");
		return;
	}
	int i, j, k, l;
	// corresponding filter kernel
	double nr1 = (double)1 / 16, nr2 = (double)2 / 16, nr3 = (double)4 / 16;
	double matrix[3][3] = {{nr1, nr2, nr1}, {nr2, nr3, nr2}, {nr1, nr2, nr1}};
	double s_r = 0, s_g = 0, s_b = 0;

	alloc_new_matrix(p, p->height, p->width);

	for (i = p->sel_h_d; i < p->sel_h_u; i++)
		for (j = p->sel_w_l; j < p->sel_w_r; j++) {
			if ((i == 0 || j == 0) ||
			    (i == p->height - 1 || j == p->width - 1)) {
				p->new_matrix[i][j].red = p->pixel_matrix[i][j].red;
				p->new_matrix[i][j].green = p->pixel_matrix[i][j].green;
				p->new_matrix[i][j].blue = p->pixel_matrix[i][j].blue;
			} else {
				s_r = 0;
				s_g = 0;
				s_b = 0;

				for (k = -1; k < 2; k++) {
					for (l = -1; l < 2; l++) {
						s_r = s_r + p->pixel_matrix[i + k][j + l].red *
						matrix[k + 1][l + 1];
						s_g = s_g + p->pixel_matrix[i + k][j + l].green *
						matrix[k + 1][l + 1];
						s_b = s_b + p->pixel_matrix[i + k][j + l].blue *
						matrix[k + 1][l + 1];
					}
				}

				p->new_matrix[i][j].red = clamp(round(s_r));
				p->new_matrix[i][j].green = clamp(round(s_g));
				p->new_matrix[i][j].blue = clamp(round(s_b));
			}
		}

	for (i = p->sel_h_d; i < p->sel_h_u; i++) {
		for (j = p->sel_w_l; j < p->sel_w_r; j++) {
			p->pixel_matrix[i][j].red = p->new_matrix[i][j].red;
			p->pixel_matrix[i][j].green = p->new_matrix[i][j].green;
			p->pixel_matrix[i][j].blue = p->new_matrix[i][j].blue;
		}
	}

	for (i = 0; i < p->height; i++)
		free(p->new_matrix[i]);
	free(p->new_matrix);

	printf("APPLY GAUSSIAN_BLUR done\n");
}

// function that verifies the parameters received by the APPLY command and uses
// the helper function accordingly
void apply_effect(struct photo *p)
{
	char *effect;
	effect = strtok(NULL, "\n ");
	if (!effect) {
		printf("Invalid command\n");
		return;
	} else if (strcmp(effect, "EDGE") == 0) {
		edge(p);
	} else if (strcmp(effect, "SHARPEN") == 0) {
		sharpen(p);
	} else if (strcmp(effect, "BLUR") == 0) {
		blur(p);
	} else if (strcmp(effect, "GAUSSIAN_BLUR") == 0) {
		gaussian_blur(p);
	} else {
		printf("APPLY parameter invalid\n");
	}
}

// function that shows the histogram of a grayscale image, with a preset number
// of bins and a max column height
void histogram(struct photo p)
{
	int bins, max_stars, i, j, max_value = 0;
	char *c;
	c = strtok(NULL, " ");
	if (!c) {
		printf("Invalid command\n");
		return;
	}

	max_stars = atoi(c);

	c = strtok(NULL, " ");
	if (!c) {
		printf("Invalid command\n");
		return;
	}

	bins = atoi(c);
	if (is_pow(bins) == 0) { // verifies if bin is a power of 2
		printf("Invalid set of parameters\n");
		return;
	}

	c = strtok(NULL, " ");
	if (c) {
		printf("Invalid command\n");
		return;
	}

	if (p.type == 1) { // only applied on grayscale images
		printf("Black and white image needed\n");
		return;
	}

	// frequency array used for storing the number of values for the searched intervals
	int *values;
	values = (int *)calloc(bins, sizeof(int));
	if (!values) {
		free_photo(p);
		exit(-1);
	}
	for (i = 0; i < p.height; i++)
		for (j = 0; j < p.width; j++)
			values[(int)p.pixel_matrix[i][j].gray / (256 / bins)]++;

	for (i = 0; i < bins; i++)
		if (values[i] > max_value)
			max_value = values[i]; // max element

	for (i = 0; i < bins; i++) {
		// number of stars shown after the following formula
		int aux = (int)((double)values[i] / max_value * max_stars);
		printf("%d%c%c", aux, 9, 124);
		if (aux != 0) {
			printf("%c", 9);
			for (j = 0; j < aux; j++)
				printf("*");
		}
		printf("\n");
	}

	free(values);
}

// function that equalizez a grayscale image based on its histogram
void equalize(struct photo *p)
{
	if (p->type == 1) { // only applied on grayscale images
		printf("Black and white image needed\n");
		return;
	}

	int *values, i, j, k;

	// value that is used for multiplying with the histogram sum of elements
	double aux = (double)255 / (p->height * p->width);
	double sum;

	// frequency array used for storing the number of values for the searched intervals
	values = (int *)calloc(256, sizeof(int));
	if (!values) {
		free_photo(*p);
		exit(-1);
	}

	for (i = 0; i < p->height; i++)
		for (j = 0; j < p->width; j++) {
			int aux2 = (int)p->pixel_matrix[i][j].gray;
			values[aux2]++;
		}

	for (i = 0; i < p->height; i++)
		for (j = 0; j < p->width; j++) {
			// sum of elements from the frequency array up to a value
			sum = 0;
			for (k = 0; k <= p->pixel_matrix[i][j].gray; k++)
				sum += values[k];

			// new pixel value
			p->pixel_matrix[i][j].gray = clamp(round(aux * sum));
		}

	free(values);
	printf("Equalize done\n");
}

// function that rotates a square selection of an image by a given angle
void rotate_square(struct photo *p, int angle, int is_negative)
{
	int i, j;
	int dim = p->sel_h_u - p->sel_h_d; // dimension of the selection

	alloc_new_matrix(p, dim, dim);

	while (angle != 0 && is_negative == 0) {
		// saving the rotated selection in new_matrix
		for (i = p->sel_h_d; i < p->sel_h_u; i++)
			for (j = p->sel_w_l; j < p->sel_w_r; j++) {
				p->new_matrix[j - p->sel_w_l][p->sel_h_u - i - 1].gray =
				p->pixel_matrix[i][j].gray;
				p->new_matrix[j - p->sel_w_l][p->sel_h_u - i - 1].red =
				p->pixel_matrix[i][j].red;
				p->new_matrix[j - p->sel_w_l][p->sel_h_u - i - 1].green =
				p->pixel_matrix[i][j].green;
				p->new_matrix[j - p->sel_w_l][p->sel_h_u - i - 1].blue =
				p->pixel_matrix[i][j].blue;
			}
		for (i = 0; i < dim; i++)
			for (j = 0; j < dim; j++) {
				// copying the selection from new_matrix to the original image
				p->pixel_matrix[i + p->sel_h_d][j + p->sel_w_l].gray =
				p->new_matrix[i][j].gray;
				p->pixel_matrix[i + p->sel_h_d][j + p->sel_w_l].red =
				p->new_matrix[i][j].red;
				p->pixel_matrix[i + p->sel_h_d][j + p->sel_w_l].green =
				p->new_matrix[i][j].green;
				p->pixel_matrix[i + p->sel_h_d][j + p->sel_w_l].blue =
				p->new_matrix[i][j].blue;
			}

		angle = angle - 90;
	}

	while (angle != 0 && is_negative == 1) {
		// saving the rotated selection in new_matrix
		for (i = p->sel_h_d; i < p->sel_h_u; i++)
			for (j = p->sel_w_l; j < p->sel_w_r; j++) {
				p->new_matrix[p->sel_w_r - j - 1][i - p->sel_h_d].gray =
				p->pixel_matrix[i][j].gray;
				p->new_matrix[p->sel_w_r - j - 1][i - p->sel_h_d].red =
				p->pixel_matrix[i][j].red;
				p->new_matrix[p->sel_w_r - j - 1][i - p->sel_h_d].green =
				p->pixel_matrix[i][j].green;
				p->new_matrix[p->sel_w_r - j - 1][i - p->sel_h_d].blue =
				p->pixel_matrix[i][j].blue;
			}
		for (i = 0; i < dim; i++)
			for (j = 0; j < dim; j++) {
				// copying the selection from new_matrix to the original image
				p->pixel_matrix[i + p->sel_h_d][j + p->sel_w_l].gray =
				p->new_matrix[i][j].gray;
				p->pixel_matrix[i + p->sel_h_d][j + p->sel_w_l].red =
				p->new_matrix[i][j].red;
				p->pixel_matrix[i + p->sel_h_d][j + p->sel_w_l].green =
				p->new_matrix[i][j].green;
				p->pixel_matrix[i + p->sel_h_d][j + p->sel_w_l].blue =
				p->new_matrix[i][j].blue;
			}

		angle = angle - 90;
	}

	for (i = 0; i < dim; i++)
		free(p->new_matrix[i]);
	free(p->new_matrix);
}

// function that rotates the entire image by a given angle
void rotate_picture(struct photo *p, int angle, int is_negative)
{
	int i, j, aux;

	while (angle != 0 && is_negative == 0) {
		alloc_new_matrix(p, p->width, p->height);
		// saving the rotated image in new_matrix
		for (i = 0; i < p->height; i++)
			for (j = 0; j < p->width; j++) {
				p->new_matrix[j][p->height - i - 1].gray =
				p->pixel_matrix[i][j].gray;
				p->new_matrix[j][p->height - i - 1].red =
				p->pixel_matrix[i][j].red;
				p->new_matrix[j][p->height - i - 1].green =
				p->pixel_matrix[i][j].green;
				p->new_matrix[j][p->height - i - 1].blue =
				p->pixel_matrix[i][j].blue;
			}

		free_photo(*p);

		// interchanging the dimension of the image, through rotation they
		// are inverted
		aux = p->height;
		p->height = p->width;
		p->width = aux;
		alloc_photo(p);

		// copying the image in the pixel_matrix
		copy_values(p, 0);

		angle = angle - 90;
		for (i = 0; i < p->height; i++)
			free(p->new_matrix[i]);
		free(p->new_matrix);

		aux = p->init_h_u;
		p->init_h_u = p->init_w_r;
		p->init_w_r = aux;
	}

	while (angle != 0 && is_negative == 1) {
		alloc_new_matrix(p, p->width, p->height);

		for (i = 0; i < p->height; i++)
			for (j = 0; j < p->width; j++) {
				p->new_matrix[p->width - j - 1][i].gray =
				p->pixel_matrix[i][j].gray;
				p->new_matrix[p->width - j - 1][i].red =
				p->pixel_matrix[i][j].red;
				p->new_matrix[p->width - j - 1][i].green =
				p->pixel_matrix[i][j].green;
				p->new_matrix[p->width - j - 1][i].blue =
				p->pixel_matrix[i][j].blue;
			}

		free_photo(*p);
		int aux = p->height;
		p->height = p->width;
		p->width = aux;
		alloc_photo(p);

		copy_values(p, 0);

		angle = angle - 90;
		for (i = 0; i < p->height; i++)
			free(p->new_matrix[i]);
		free(p->new_matrix);

		aux = p->init_h_u;
		p->init_h_u = p->init_w_r;
		p->init_w_r = aux;
	}
	p->sel_h_d = 0;
	p->sel_h_u = p->init_h_u;
	p->sel_w_l = 0;
	p->sel_w_r = p->init_w_r;
}

// function that checks the validity of an angle (90, 180, 270, 360, ...)
// and calls a helper function by the nature of the rotation
void rotate(struct photo *p)
{
	int angle; // rotation angle
	char *c;
	c = strtok(NULL, "\n ");
	angle = atoi(c);
	int neg = 0;

	if (angle % 90) {
		printf("Unsupported rotation angle\n");
		return;
	}

	if (angle == 0) {
		printf("Rotated %d\n", angle);
		return;
	}

	printf("Rotated %d\n", angle);

	if (angle != abs(angle))
		neg = 1; // negative angle
	angle = abs(angle);

	// checks if the selection is square
	if (p->sel_h_u - p->sel_h_d == p->sel_w_r - p->sel_w_l)
		rotate_square(p, angle, neg);
	else
		rotate_picture(p, angle, neg);
}

// function that saves the image in the chosen format in a specified file
void save(struct photo p)
{
	char *format, *filename;
	int i, j;
	filename = strtok(NULL, "\n ");
	format = strtok(NULL, "\n ");
	if (!format || strcmp(format, "ascii")) { // binary format
		FILE *out = fopen(filename, "wb");
		int value;
		if (p.type == 0) {
			fprintf(out, "P5\n");
			fprintf(out, "%d %d\n", p.width, p.height);
			fprintf(out, "255\n");
			for (i = 0; i < p.height; i++)
				for (j = 0; j < p.width; j++) {
					value = (int)p.pixel_matrix[i][j].gray;
					fwrite(&value, sizeof(unsigned char), 1, out);
				}
		} else {
			fprintf(out, "P6\n");
			fprintf(out, "%d %d\n", p.width, p.height);
			fprintf(out, "255\n");
			for (i = 0; i < p.height; i++)
				for (j = 0; j < p.width; j++) {
					value = (int)p.pixel_matrix[i][j].red;
					fwrite(&value, sizeof(unsigned char), 1, out);
					value = (int)p.pixel_matrix[i][j].green;
					fwrite(&value, sizeof(unsigned char), 1, out);
					value = (int)p.pixel_matrix[i][j].blue;
					fwrite(&value, sizeof(unsigned char), 1, out);
				}
		}
		fclose(out);
	} else { // ascii format
		FILE *out = fopen(filename, "wt");
		if (p.type == 0) {
			fprintf(out, "P2\n");
			fprintf(out, "%d %d\n", p.width, p.height);
			fprintf(out, "255\n");
			for (i = 0; i < p.height; i++) {
				for (j = 0; j < p.width; j++)
					fprintf(out, "%d ", (int)p.pixel_matrix[i][j].gray);
				fprintf(out, "\n");
			}
		} else {
			fprintf(out, "P3\n");
			fprintf(out, "%d %d\n", p.width, p.height);
			fprintf(out, "255\n");
			for (i = 0; i < p.height; i++) {
				for (j = 0; j < p.width; j++) {
					fprintf(out, "%d ", (int)p.pixel_matrix[i][j].red);
					fprintf(out, "%d ", (int)p.pixel_matrix[i][j].green);
					fprintf(out, "%d ", (int)p.pixel_matrix[i][j].blue);
				}
				fprintf(out, "\n");
			}
		}
		fclose(out);
	}

	printf("Saved %s\n", filename);
}

int main(void)
{
	int is_loaded = 0; // variable that stores if we have an image loaded
	char command[MAX];
	struct photo p1;

	while (fgets(command, MAX, stdin)) {  // receiving the command from terminal
		char *first_word;
		first_word = strtok(command, "\n ");
		if (strcmp(first_word, "LOAD") == 0) {
			if (is_loaded == 1) {
				free_photo(p1);
				is_loaded = 0;
			}
			char *file_name;
			file_name = strtok(NULL, "\n");
			load_photo(&p1, file_name, &is_loaded);
		} else if (strcmp(first_word, "SELECT") == 0) {
			if (is_loaded == 0)
				printf("No image loaded\n");
			else
				selection(&p1);
		} else if (strcmp(first_word, "CROP") == 0) {
			if (is_loaded == 0)
				printf("No image loaded\n");
			else
				crop(&p1);
		} else if (strcmp(first_word, "APPLY") == 0) {
			if (is_loaded == 0)
				printf("No image loaded\n");
			else
				apply_effect(&p1);
		} else if (strcmp(first_word, "HISTOGRAM") == 0) {
			if (is_loaded == 0)
				printf("No image loaded\n");
			else
				histogram(p1);
		} else if (strcmp(first_word, "EQUALIZE") == 0) {
			if (is_loaded == 0)
				printf("No image loaded\n");
			else
				equalize(&p1);
		} else if (strcmp(first_word, "ROTATE") == 0) {
			if (is_loaded == 0)
				printf("No image loaded\n");
			else
				rotate(&p1);
		} else if (strcmp(first_word, "SAVE") == 0) {
			if (is_loaded == 0)
				printf("No image loaded\n");
			else
				save(p1);
		} else if (strcmp(first_word, "EXIT") == 0) {
			if (is_loaded == 1) {
				free_photo(p1);
				return 0;
			}

			printf("No image loaded\n");
			return 0;
		} else {
			printf("Invalid command\n");
		}
	}
	return 0;
}
