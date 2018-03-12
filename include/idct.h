#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "jpeg_reader.h"
#include "bitstream.h"
#include "huffman.h"

#define PI 3.14159265358979323846

void affiche_tableau_float(float_t tab[], uint32_t taille);

void iDCT(int32_t tab[]);
