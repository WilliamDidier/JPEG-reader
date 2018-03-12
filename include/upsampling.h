#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdbool.h>

#include "jpeg_reader.h"
#include "bitstream.h"
#include "huffman.h"
#include "bloc.h"
#include "quantification.h"
#include "inverse_zigzag.h"
#include "idct.h"
#include "write_ppm.h"
#include "traitement_bloc.h"


void upsampling(uint8_t mcu[], uint8_t sampling_factor_h,
                uint8_t sampling_factor_v);

void upsampling_vertical(uint8_t tab[], uint8_t facteur_v, uint8_t facteur_h);

void upsampling_horizontal(uint8_t tab[]);

void upsampling_double(uint8_t tab[], uint8_t facteur_h);
