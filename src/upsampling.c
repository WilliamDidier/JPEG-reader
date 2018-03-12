#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdbool.h>

#include "jpeg_reader.h"
#include "bitstream.h"
#include "huffman.h"
#include "upsampling.h"

/*Fonction principale du module : elle appelle les fonctions
secondaires tour à tour en fonction du contexte*/
void upsampling(uint8_t mcu[], uint8_t sampling_factor_h, uint8_t sampling_factor_v){
  if (sampling_factor_v > 1){
      if (sampling_factor_h > 1) {
          upsampling_horizontal(mcu);
          upsampling_double(mcu, sampling_factor_h);
      } else {
          upsampling_vertical(mcu, sampling_factor_v, sampling_factor_h);
      }
  } else {
      if (sampling_factor_h > 1){
          upsampling_horizontal(mcu);
      }
  }
}

/*Fonction qui permet d'effectuer un upsampling vertical sur un tableau de
composante Cr ou Cb. L'espace nécessaire pour placer les coefficient dupliqué
doit être disponible dans le tableau passé en paramètre de la fonction*/
void upsampling_vertical(uint8_t tab[], uint8_t facteur_v, uint8_t facteur_h){
    for (int count=64*facteur_h-1 ; count > -1 ; count--){
        for (int cnt_fact=facteur_v-1 ; cnt_fact > -1 ; cnt_fact--){
            tab[facteur_v*count+cnt_fact]=tab[count];
        }
    }
}

/*Fonction qui permet de faire un upsampling vertical si un upsampling
horizontal a été fait auparavant. On fait une permutation des lignes
du bloc préalablement upsamplé pour le traiter dans le bon sens */
void upsampling_double(uint8_t tab[], uint8_t facteur_h){
    uint8_t tab_original[64*facteur_h];
    memcpy(tab_original, tab, 64*facteur_h);
    uint8_t tab_indice[16] = {0, 1, 2, 3, 8, 9, 10, 11, 4, 5, 6, 7, 12, 13, 14, 15};
    uint8_t indice = 0;
    for (uint8_t compteur_1=0 ; compteur_1 < 64*facteur_h ; compteur_1+=8){

        for (int compteur_2 = 0; compteur_2 < 8; compteur_2++){
          tab[compteur_1+indice*8+compteur_2] = tab_original[tab_indice[indice]*8+compteur_2];
          tab[compteur_1+indice*8+compteur_2+8] = tab_original[tab_indice[indice]*8+compteur_2];
        }
        indice++;
    }

}

/*Fonction qui permet d'effectuer un upsampling horizontal sur un tableau de
composante Cr ou Cb. L'espace nécessaire pour placer les coefficient dupliqué
doit être disponible dans le tableau passé en paramètre de la fonction*/
void upsampling_horizontal(uint8_t tab[]){

    for (int count=63 ; count > -1 ; count--){
        if ((count)%8 > 3){
              tab[64+count-4+(count-4)%8]=tab[count];
              tab[64+count-4+(count-4)%8 +1]=tab[count];
        } else {
              tab[count+count%8]=tab[count];
              tab[count+count%8 +1]=tab[count];
        }
    }
}
