#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "jpeg_reader.h"
#include "bitstream.h"
#include "huffman.h"
#include "inverse_zigzag.h"

/*Fonction qui réarrange les coefficients du tableau selon
la transformation zig-zag inverse. Ne travaille pas en place */
void inverse_zigzag(int32_t tab[]){
  int32_t tab2[64];
  memcpy(tab2, tab, 64*sizeof(int32_t));
  uint8_t indice_zigzag[64] = {0,1,5,6,14,15,27,28,
                              2,4,7,13,16,26,29,42,
                              3,8,12,17,25,30,41,43,
                              9,11,18,24,31,40,44,53,
                              10,19,23,32,39,45,52,54,
                              20,22,33,38,46,51,55,60,
                              21,34,37,47,50,56,59,61,
                              35,36,48,49,57,58,62,63};
  for (int i = 0; i < 64; i++){
    tab[i] = tab2[indice_zigzag[i]];
  }
}
