#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "jpeg_reader.h"
#include "bitstream.h"
#include "huffman.h"

void quantification_inverse(const struct jpeg_desc *jdesc, int32_t tab[], uint8_t comp_id);
