Copyright Botez Luca 2022-2023

## Image Editor

This program performs various operations on images formatted as PPM or PGM, saved in ASCII or binary format.

## COMMANDS:

### LOAD
Loads the image into memory. Depending on the file type (binary or ASCII), different functions are called to store all relevant information such as dimensions, image type (grayscale or color), and color intensity of each pixel in a structure. Memory is dynamically allocated based on the dimensions of the image.

### SELECT
Defines a specific region of the image to apply operations to. The selection is made using coordinates provided by the user. If the selection exceeds the image boundaries, an error is displayed.

### ROTATE
Rotates the selected region or the entire image by a given angle (multiples of 90 degrees). If the selection does not form a square, an error is displayed.

### CROP
Cuts the selected area, reducing the image to that region.

### APPLY
Applies filters to the image, such as edge detection, sharpening, or blurring. Filters work only on color images.

### HISTOGRAM
Generates a histogram for grayscale images, providing a graphical representation of pixel intensity distribution.

### EQUALIZE
Equalizes the histogram of grayscale images to enhance contrast.

### SAVE
Saves the image in the chosen format (ASCII or binary). The binary format allows more efficient storage.

### EXIT
Frees allocated memory and exits the program.

## Notes:
- The program is optimized for performance using dynamic memory allocation.
- Supports both grayscale and color images.
- Provides error handling for invalid operations or selections.


