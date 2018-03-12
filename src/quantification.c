#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "jpeg_reader.h"
#include "bitstream.h"
#include "huffman.h"
#include "quantification.h"

/*Fonction qui effectue l'opération de quantification inverse
sur les coefficients du bloc */
void quantification_inverse(const struct jpeg_desc *jdesc, int32_t tab[], uint8_t comp_id){
  uint8_t *quantization_table = get_quantization_table(jdesc, get_frame_component_quant_index(jdesc, comp_id));
  for (int i = 0; i<64; i++){
    tab[i] *= quantization_table[i];
  }
}
