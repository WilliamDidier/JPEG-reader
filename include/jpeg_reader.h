#ifndef __JPEG_DESC_H__
#define __JPEG_DESC_H__

#include <stdint.h>
#include <stdbool.h>

#include "bitstream.h"
#include "huffman.h"


enum component {
    COMP_Y,
    COMP_Cb,
    COMP_Cr,
    /* sentinelle */
    COMP_NB
};

enum direction {
    DIR_H,
    DIR_V,
    /* sentinelle */
    DIR_NB
};

enum acdc {
    DC = 0,
    AC = 1,
    /* sentinelle */
    ACDC_NB
};

struct jpeg_desc{
  const char *filename;
  struct bitstream *stream;
  uint8_t nb_quantization_table;
  uint8_t *quantization_table_luminance;
  uint8_t *quantization_table_chrominance;
  uint32_t hauteur;
  uint32_t largeur;
  uint32_t nb_composantes;
  uint32_t frame_comp_index_y;
  uint32_t frame_comp_index_cb;
  uint32_t frame_comp_index_cr;
  uint32_t sampling_factor_h_y;
  uint32_t sampling_factor_h_cb;
  uint32_t sampling_factor_h_cr;
  uint32_t sampling_factor_v_y;
  uint32_t sampling_factor_v_cb;
  uint32_t sampling_factor_v_cr;
  uint32_t quantization_index_y;
  uint32_t quantization_index_cb;
  uint32_t quantization_index_cr;
  uint8_t nb_huffman_table_DC;
  uint8_t nb_huffman_table_AC;
  struct huff_table *huff_table_DC_luminance;
  struct huff_table *huff_table_DC_chrominance;
  struct huff_table *huff_table_AC_luminance;
  struct huff_table *huff_table_AC_chrominance;
  uint32_t id_composante_Y;
  uint32_t id_composante_Cb;
  uint32_t id_composante_Cr;
  uint32_t indice_huffman_DC_Y;
  uint32_t indice_huffman_AC_Y;
  uint32_t indice_huffman_DC_Cb;
  uint32_t indice_huffman_AC_Cb;
  uint32_t indice_huffman_DC_Cr;
  uint32_t indice_huffman_AC_Cr;

};

// general
struct jpeg_desc *read_jpeg(const char *filename);
void close_jpeg(struct jpeg_desc *jpeg);
const char *get_filename(const struct jpeg_desc *jpeg);

// access to stream, placed just at the beginning of the scan raw data
struct bitstream *get_bitstream(const struct jpeg_desc *jpeg);

// from DQT
uint8_t get_nb_quantization_tables(const struct jpeg_desc *jpeg);
uint8_t *get_quantization_table(const struct jpeg_desc *jpeg,
                                       uint8_t index);

// from DHT
uint8_t get_nb_huffman_tables(const struct jpeg_desc *jpeg,
                                     enum acdc acdc);
struct huff_table *get_huffman_table(const struct jpeg_desc *jpeg,
                                            enum acdc acdc, uint8_t index);

// from Frame Header SOF0
uint16_t get_image_size(struct jpeg_desc *jpeg, enum direction dir);
uint8_t get_nb_components(const struct jpeg_desc *jpeg);

uint8_t get_frame_component_id(const struct jpeg_desc *jpeg,
                                      uint8_t frame_comp_index);
uint8_t get_frame_component_sampling_factor(const struct jpeg_desc *jpeg,
                                                   enum direction dir,
                                                   uint8_t frame_comp_index);
uint8_t get_frame_component_quant_index(const struct jpeg_desc *jpeg,
                                               uint8_t frame_comp_index);

// from Scan Header SOS
uint8_t get_scan_component_id(const struct jpeg_desc *jpeg,
                                     uint8_t scan_comp_index);
uint8_t get_scan_component_huffman_index(const struct jpeg_desc *jpeg,
                                                enum acdc,
                                                uint8_t scan_comp_index);

#endif
