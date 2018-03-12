#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "jpeg_reader.h"
#include "bitstream.h"
#include "huffman.h"
#include "write_ppm.h"

int indice_sans_extension(const struct jpeg_desc *jdesc){
    const char *filename = get_filename(jdesc);
    int i = 0;
    while (filename[i] != '.'){
        i++;
    }
    return i;
}

FILE* entete_fichier(const struct jpeg_desc *jdesc, uint32_t hauteur,
                     uint32_t largeur, bool couleur){
    /* Traduit le tableau en un fichier PGM ou PPM */
    const char *filename = get_filename(jdesc);
    int i = indice_sans_extension(jdesc);
    char *new_filename = NULL;
    new_filename = calloc(i+5, sizeof(int));
    memcpy(new_filename, filename, i);
    FILE* fichier = NULL;

    if (!couleur){
        strcat(new_filename, ".pgm");
        fichier = fopen(new_filename, "wb");
        entete_PGM(fichier, hauteur, largeur);
    } else {
        strcat(new_filename, ".ppm");
        fichier = fopen(new_filename, "wb");
        entete_PPM(fichier, hauteur, largeur);
    }
      return fichier;
  }


void entete_PGM(FILE* fichier, uint32_t hauteur, uint32_t largeur){
    /* Ecris dans un fichier l'entête d'un fichier PGM */
    fputs("P5\n", fichier);
    fprintf(fichier, "%u %u\n", hauteur, largeur);
    fputs("255\n", fichier);
}


void entete_PPM(FILE* fichier, uint32_t hauteur, uint32_t largeur){
    /* Ecris dans un fichier l'entête d'un fichier PPM */
    fputs("P6\n", fichier);
    fprintf(fichier, "%u %u\n", hauteur, largeur);
    fputs("255\n", fichier);
}


uint8_t conversion_pixel_RGB(float tmp_Y, float tmp_Cb, float tmp_Cr, int couleur){
    /* Passage de la représentation (Y, Cb, Cr) à (R, G, B) pour une couleur */

    /* Calcul de la composante rouge : */
    if (couleur == 0){
        float red_float = tmp_Y - 0.0009267*(tmp_Cb - 128.0) + 1.4016868*(tmp_Cr - 128.0);
        uint8_t red;
        if (red_float< 0.0){
            red = 0;
        } else if (red_float > 255.0){
            red = 255;
        } else {
            red = (uint8_t) (red_float + 0.5);
        }
        return red;

    /* Calcul de la composante verte : */
    } else if (couleur == 1){
        float green_float = tmp_Y - 0.3436954*(tmp_Cb - 128.0) - 0.7141690*(tmp_Cr - 128.0);
        uint8_t green;
        if (green_float< 0.0){
            green = 0;
        } else if (green_float > 255.0){
            green = 255;
        } else {
            green = (uint8_t) (green_float + 0.5);
        }
        return green;

    /* Calcul de la composante bleue : */
    } else if (couleur == 2){
        float blue_float = tmp_Y + 1.7721604*(tmp_Cb - 128.0) + 0.0009902*(tmp_Cr - 128.0);
        uint8_t blue;
        if (blue_float< 0.0){
            blue = 0;
        } else if (blue_float > 255.0){
            blue = 255;
        } else {
            blue = (uint8_t) (blue_float + 0.5);
        }
        return blue;

    } else {
       fprintf(stderr, "Mauvais indice de couleur\n");
       return EXIT_FAILURE;
    }
  }


void write_tab(FILE* fichier, uint8_t tab_Y[], uint8_t tab_Cb[],
               uint8_t tab_Cr[], bool couleur, uint16_t size_h,
               uint8_t facteur_h, uint8_t facteur_v){
    /* Ecris une ligne de bloc dans un fichier PGM ou PPM */
    if (!couleur){
        JPEG_into_PGM(fichier, tab_Y, size_h);
    } else {
        JPEG_into_PPM(fichier, tab_Y, tab_Cb, tab_Cr, size_h, facteur_h, facteur_v);
    }
}


void JPEG_into_PGM(FILE* fichier, uint8_t tab[], uint16_t size_h){
      /* Ecris une ligne de bloc dans un PGM, en enlevant les duplications
         faites aux bords */
      uint32_t fausse_largeur = size_h + 8 - size_h%8;
      if (size_h % 8 == 0){
          fausse_largeur = size_h;
      }
      for (uint32_t j = 0; j < 8; j++){
          for (uint32_t i = 0; i < fausse_largeur; i++){
              if (i < size_h){
                  uint8_t tmp = tab[fausse_largeur*j+i];
                  fwrite(&tmp, sizeof(int8_t), 1, fichier);
              }
          }
      }
  }


  void JPEG_into_PPM(FILE* fichier, uint8_t tab_Y[], uint8_t tab_Cb[],
                     uint8_t tab_Cr[], uint16_t size_h, uint8_t facteur_h,
                     uint8_t facteur_v){
      /* Ecris une ligne de bloc dans un PPM, en enlevant les duplications
         sur les bords */
      uint8_t size_MCU = 8*facteur_h;
      uint32_t fausse_largeur = size_h - size_h%size_MCU + size_MCU;
      if (size_h % size_MCU == 0){
          fausse_largeur = size_h;
      }
      for (uint32_t j = 0; j < 8*facteur_v; j++){
          for (uint32_t i = 0; i < fausse_largeur; i++){
              if (i < size_h){
                  float tmp_Y = tab_Y[fausse_largeur*j+i];
                  float tmp_Cb = tab_Cb[fausse_largeur*j+i];
                  float tmp_Cr = tab_Cr[fausse_largeur*j+i];
                  // Traitement du rouge :
                  uint8_t red = conversion_pixel_RGB(tmp_Y, tmp_Cb, tmp_Cr, 0);
                  fwrite(&red, sizeof(int8_t), 1, fichier);
                  // Traitement du vert :
                  uint8_t green = conversion_pixel_RGB(tmp_Y, tmp_Cb, tmp_Cr, 1);
                  fwrite(&green, sizeof(int8_t), 1, fichier);
                  // Traitement du bleu :
                  uint8_t blue = conversion_pixel_RGB(tmp_Y, tmp_Cb, tmp_Cr, 2);
                  fwrite(&blue, sizeof(int8_t), 1, fichier);
              }
          }
      }
}


void write_last_tab(FILE* fichier, uint8_t tab_Y[], uint8_t tab_Cb[],
                    uint8_t tab_Cr[], bool couleur, uint16_t size_h,
                    uint16_t size_v, uint8_t facteur_h, uint8_t facteur_v){
    /* Ecris la dernière ligne de bloc d'une image dans un fichier PGM ou PPM */
    if (!couleur){
        JPEG_into_PGM_last(fichier, tab_Y, size_h, size_v);
    } else {
        JPEG_into_PPM_last(fichier, tab_Y, tab_Cb, tab_Cr, size_h,
                           size_v, facteur_h, facteur_v);
    }
}


void JPEG_into_PGM_last(FILE* fichier, uint8_t tab[], uint16_t size_h,
                        uint16_t size_v){
    /* Ecris la dernière ligne de bloc dans un PGM, en enlevant les duplications
       faites aux bords, y compris la duplication de ligne */
    if (size_v%8 != 0){
        uint32_t fausse_largeur = size_h + 8 - size_h%8;
        if (size_h % 8 == 0){
            fausse_largeur = size_h;
        }
        for (uint32_t j = 0; j < size_v%8; j++){
            for (uint32_t i = 0; i < fausse_largeur; i++){
                if (i < size_h){
                    uint8_t tmp = tab[fausse_largeur*j+i];
                    fwrite(&tmp, sizeof(int8_t), 1, fichier);
                }
            }
        }
    } else {
        JPEG_into_PGM(fichier, tab, size_h);
    }
}



void JPEG_into_PPM_last(FILE* fichier, uint8_t tab_Y[], uint8_t tab_Cb[],
                        uint8_t tab_Cr[], uint16_t size_h, uint16_t size_v,
                        uint8_t facteur_h, uint8_t facteur_v){
    /* Ecris la dernière ligne de bloc dans un PGM, en enlevant les duplications
       faites aux bords, y compris la duplication de ligne */
    if (size_v%8 != 0){
        uint8_t size_MCU = 8*facteur_h;
        uint32_t fausse_largeur = size_h + size_MCU - size_h%size_MCU;
        if (size_h % size_MCU == 0){
            fausse_largeur = size_h;
        }
        for (uint32_t j = 0; j < (facteur_v-1)*8+size_v%8; j++){
            for (uint32_t i = 0; i < fausse_largeur; i++){
                if (i < size_h){
                    float tmp_Y = tab_Y[fausse_largeur*j+i]*1.0;
                    float tmp_Cb = tab_Cb[fausse_largeur*j+i]*1.0;
                    float tmp_Cr = tab_Cr[fausse_largeur*j+i]*1.0;
                    // Traitement du rouge :
                    uint8_t red = conversion_pixel_RGB(tmp_Y, tmp_Cb, tmp_Cr, 0);
                    fwrite(&red, sizeof(int8_t), 1, fichier);
                    // Traitement du vert :
                    uint8_t green = conversion_pixel_RGB(tmp_Y, tmp_Cb, tmp_Cr, 1);
                    fwrite(&green, sizeof(int8_t), 1, fichier);
                    // Traitement du bleu :
                    uint8_t blue = conversion_pixel_RGB(tmp_Y, tmp_Cb, tmp_Cr, 2);
                    fwrite(&blue, sizeof(int8_t), 1, fichier);
                }
            }
        }
    } else {
        JPEG_into_PPM(fichier, tab_Y, tab_Cb, tab_Cr, size_h, facteur_h, facteur_v);
    }
}
