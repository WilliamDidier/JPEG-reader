# JPEG-reader
JPEG files reader coded in C

Compile using make
Test the programe with ./bin/jpeg2ppm image/image_of_your_choice
The output will be writen in the image folder as a ppm or pgm file with the same name as the input.

This JPEG reader only works in some specific cases and cannot read a random image from the internet. Also, some module have some serious problems of memory leakage that we did not have the time to fix before the due date.
