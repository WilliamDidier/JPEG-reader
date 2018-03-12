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

/*Fonction qui renvoie la taille encodée de l'image, en fonction de la "vraie"*
 taille et des facteurs d'échantillonnage*/
void tailles(uint16_t *size_h, uint16_t *size_v,
             uint8_t sampling_factor_h, uint8_t sampling_factor_v,
             struct jpeg_desc *jdesc){

    uint16_t size_h_i = get_image_size(jdesc, DIR_H);
    *size_h = size_h_i;
    if (size_h_i%(8*sampling_factor_h) != 0){
        *size_h = size_h_i - size_h_i % (8*sampling_factor_h) + (8*sampling_factor_h);
    }

    uint16_t size_v_i = get_image_size(jdesc, DIR_V);
    *size_v = size_v_i;
    if (size_v_i%(8*sampling_factor_v) != 0){
        *size_v = size_v_i - size_v_i % (8*sampling_factor_v) + (8*sampling_factor_v);
    }
}



/*Fonction qui appelle tour à tour les fonction d'analyse de bloc.
Retourne le coefficient DC obtenu après la première étapê,
car il sera utile au prochain appel.*/
uint32_t analyse_bloc(int32_t *prev_DC,
                      struct jpeg_desc *jdesc, uint8_t mcu[],
                      struct bitstream *stream, bool verbeux,
                      uint8_t comp_id, FILE* file){
    int32_t tab[64] = {0};
    get_DC(jdesc, stream, tab, prev_DC, comp_id);
    *prev_DC = tab[0];
    get_AC(jdesc, stream, tab, comp_id);


    if (verbeux){
    fprintf(file, "[  bloc] ");
    affiche_tableau(tab, 64, file);
    }

    quantification_inverse(jdesc, tab, comp_id);

    if (verbeux){
    fprintf(file, "[iquant] ");
    affiche_tableau(tab, 64, file);
    }

    inverse_zigzag(tab);

    if (verbeux){
    fprintf(file, "[   izz] ");
    affiche_tableau(tab, 64, file);
    }

    iDCT(tab);

    if (verbeux){
    fprintf(file, "[  idct] ");
    affiche_tableau(tab, 64, file);
    fprintf(file, "\n");
    }
    for (int k = 0; k<64; k++){
      mcu[k] = tab[k];
    }
    return *prev_DC;
  }


/* Fonction qui permet de placer les éléments d'un bloc au bon endroit
d'un tableau qui représente une ligne de MCU de l'image, en fonction de
l'indice du bloc dans la ligne de MCU */
  void reorganise(uint8_t bloc[], uint8_t ligne[],
                  uint32_t j, uint16_t size_h, uint8_t kv){

      for (uint32_t k = 0; k < 8; k++){
          for (uint32_t l = 0; l < 8; l++){
              ligne[kv*size_h*8 + size_h*k + 8*j + l] = bloc[8*k + l];
          }
      }
  }


/*Fonction qui permet de libérer l'espace alloué à tous les tableaux
encore ouverts en fin d'exécution du programme.*/
void fermeture(FILE* fichier, FILE* fichier2, struct jpeg_desc *jdesc, uint8_t *p1, uint8_t *p2,
                             uint8_t *p3, uint8_t *p4, uint8_t *p5, uint8_t *p6,
                             int32_t *p7, int32_t *p8, int32_t *p9){
  fclose(fichier);
  if (fichier2 != NULL){
      fclose(fichier2);
  }
  free(p1);
  free(p2);
  free(p3);
  free(p4);
  free(p5);
  free(p6);
  free(p7);
  free(p8);
  free(p9);

  close_jpeg(jdesc);
}
