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


void tailles(uint16_t *size_h,
              uint16_t *size_v,
              uint8_t sampling_factor_h,
              uint8_t sampling_factor_v,
              struct jpeg_desc *jdesc);

uint32_t analyse_bloc(int32_t *prev_DC,
                      struct jpeg_desc *jdesc, uint8_t ligne[],
                      struct bitstream *stream, bool verbeux,
                      uint8_t comp_id, FILE* file);

void fermeture(FILE* fichier, FILE* fichier2, struct jpeg_desc *jdesc, uint8_t *p1,
               uint8_t *p2, uint8_t *p3, uint8_t *p4, uint8_t *p5,
               uint8_t *p6, int32_t *p7, int32_t *p8, int32_t *p9);

void reorganise(uint8_t bloc[], uint8_t ligne[], uint32_t j,
                uint16_t size_h, uint8_t facteur_v);
