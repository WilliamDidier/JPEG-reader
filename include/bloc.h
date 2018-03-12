#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "jpeg_reader.h"
#include "bitstream.h"
#include "huffman.h"

int32_t valeur_magnitude(uint8_t magnitude, uint32_t indice_m);

void get_DC(const struct jpeg_desc *jdesc,
            struct bitstream *stream,
            int32_t tab[],
            int32_t *prev_DC,
            uint8_t comp_id);

int get_AC(const struct jpeg_desc *jdesc,
           struct bitstream *stream,
           int32_t tab[],
           uint8_t comp_id);

void affiche_tableau(int32_t tab[], uint32_t taille, FILE* file);

void affiche_tableau_8(uint8_t tab[], uint32_t taille, FILE* file);
