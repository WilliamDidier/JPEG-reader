#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "jpeg_reader.h"
#include "bitstream.h"
#include "huffman.h"
#include "idct.h"

/*Fonction analogue aux deux autres fonctions d'affichage du module bloc.
Permet d'afficher un tableau de valeurs flottantes en hexa */
void affiche_tableau_float(float_t tab[], uint32_t taille){
  for (uint32_t i = 0; i < taille; i++){
    printf("%hx ", tab[i]);
  }
  printf("\n");
}

/*Fontion qui fait la transformée en cosinus discrets de manière naïve */
void iDCT(int32_t tab[]){
  float_t tab_float[64]={0.0};
  int32_t tab2[64];
  memcpy(tab2, tab, 64*sizeof(int32_t));
  for (int x = 0; x < 8; x++){
    for (int y = 0; y < 8; y++){
      int indice = 8*x+y;
      for (int lambda = 0; lambda < 8; lambda++){
        for (int mu = 0; mu < 8; mu++){
          float clambda = 1.0;
          if (lambda == 0){
            clambda = 1.0/sqrt(2);
          }
          float cmu = 1.0;
          if (mu == 0){
            cmu = 1.0/sqrt(2);
          }
          tab_float[indice] += cos(((2.0*(float) x+1.0)*(float) lambda*PI)/16.0)
                       *cos(((2.0*(float) y+1.0)*(float) mu*PI)/16.0)
                       *(float) tab2[8*lambda+mu]*clambda*cmu;
        }
      }
      tab_float[indice] *= 1.0/sqrt(16);
      tab_float[indice] += 128.0;
      if (tab_float[indice] < 0.0){
        tab_float[indice] = 0.0;
      }
      else if (tab_float[indice] > 255.0){
        tab_float[indice] = 255.0;
      }
      tab[indice] = (uint8_t) (tab_float[indice]+0.5);
    }
  }
}
