#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "jpeg_reader.h"
#include "bitstream.h"
#include "huffman.h"

int indice_sans_extension(const struct jpeg_desc *jdesc);

FILE* entete_fichier(const struct jpeg_desc *jdesc,
                    uint32_t hauteur,
                    uint32_t largeur,
                    bool couleur);

void entete_PGM(FILE* fichier,
                uint32_t hauteur,
                uint32_t largeur);

void entete_PPM(FILE* fichier,
                uint32_t hauteur,
                uint32_t largeur);

void write_tab(FILE* fichier,
               uint8_t *tab_Y,
               uint8_t *tab_Cb,
               uint8_t *tab_Cr,
               bool couleur,
               uint16_t size_h,
               uint8_t facteur_h,
               uint8_t facteur_v);

void write_last_tab(FILE* fichier,
                    uint8_t tab_Y[],
                    uint8_t tab_Cb[],
                    uint8_t tab_Cr[],
                    bool couleur,
                    uint16_t size_h,
                    uint16_t size_v,
                    uint8_t facteur_h,
                    uint8_t facteur_v);

void JPEG_into_PGM(FILE* fichier,
                   uint8_t tab[],
                   uint16_t size_h);

void JPEG_into_PGM_last(FILE* fichier,
                        uint8_t tab[],
                        uint16_t size_h,
                        uint16_t size_v);

void JPEG_into_PPM(FILE* fichier,
                   uint8_t tab_Y[],
                   uint8_t tab_Cb[],
                   uint8_t tab_Cr[],
                   uint16_t size_h,
                   uint8_t facteur_h,
                   uint8_t facteur_v);

 void JPEG_into_PPM_last(FILE* fichier,
                         uint8_t tab_Y[],
                         uint8_t tab_Cb[],
                         uint8_t tab_Cr[],
                         uint16_t size_h,
                         uint16_t size_v,
                         uint8_t facteur_h,
                         uint8_t facteur_v);

 uint8_t conversion_pixel_RGB(float tmp_Y,
                              float tmp_Cb,
                              float tmp_Cr,
                              int couleur);
