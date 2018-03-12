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
#include "upsampling.h"

/* int next_bit_stream(const struct jpeg_desc *jdesc, struct bitstream *next_stream){
  uint32_t byte = 0;
  uint8_t nb_read = read_bitstream(next_stream, 8, &byte, false);
  if (nb_read != 8) {
    fprintf(stderr, "Erreur de lecture! ");
    return EXIT_FAILURE;
  }
  else {
    return byte;
  }
}
*/


int main(int argc, char **argv){

    if (argc < 2) {
    	  fprintf(stderr, "Usage: %s fichier.jpeg [-v]\n", argv[0]);
    	  return EXIT_FAILURE;
    }
    char *filename = argv[1];
    /* On cree un jpeg_desc qui permettra de lire ce fichier. */
    struct jpeg_desc *jdesc = read_jpeg(filename);
    struct bitstream *stream = get_bitstream(jdesc);

    bool verbeux = false;
    FILE* file_blabla = NULL;
    //Gestion des options d'appel -v et -h
    if (argc == 3){
        char *second_argument = argv[2];
        if (strcmp(second_argument, "-v") || strcmp(second_argument, "-verbose")){
            verbeux = true;
            int i = indice_sans_extension(jdesc);
            char *new_filename = NULL;
            new_filename = calloc(i+8, sizeof(int));
            memcpy(new_filename, filename, i);
            strcat(new_filename, ".blabla");
            file_blabla = fopen(new_filename, "w");

        } else if (strcmp(second_argument, "-h") || strcmp(second_argument, "-help")){
            fprintf(file_blabla, "Usage: %s fichier.jpeg [-v]\n", argv[0]);
            return EXIT_SUCCESS;
        } else {
            fprintf(stderr, "Usage: %s fichier.jpeg [-v]\n", argv[0]);
            return EXIT_FAILURE;
        }
    }


    /*On commence par regarder si l'image est en couleur ou non.  */
    bool en_couleur = false;
    if (get_nb_huffman_tables(jdesc, DC) > 1){
        en_couleur = true;
    }


    uint8_t sampling_factor_h = 1;
    uint8_t sampling_factor_v = 1;
    //On calcule la taille d'une MCU (en nombre de blocs)
    uint8_t size_MCU = get_frame_component_sampling_factor(jdesc, DIR_V, 0)*
                       get_frame_component_sampling_factor(jdesc, DIR_H, 0);

    if (en_couleur){
        sampling_factor_h = get_frame_component_sampling_factor(jdesc, DIR_H, 0)/
                            get_frame_component_sampling_factor(jdesc, DIR_H, 1);
        sampling_factor_v = get_frame_component_sampling_factor(jdesc, DIR_V, 0)/
                            get_frame_component_sampling_factor(jdesc, DIR_V, 1);
    }

    uint16_t size_h_i = get_image_size(jdesc, DIR_H);
    uint16_t size_v_i = get_image_size(jdesc, DIR_V);
    uint16_t size_v, size_h;
    /*Les variables se terminant par un i sont les tailles initiales.
    Dans size_h et size_v ont met les tailles effectivement encodées, de sorte
    que le nombre de MCu soit rond. Cf. partie 2.9 du sujet */
    tailles(&size_h, &size_v, sampling_factor_h, sampling_factor_v, jdesc);

    /*On commence par ouvrir le fichier de destination, et on y écrit déjà
    l'entete*/
    FILE* fichier = entete_fichier(jdesc, size_h_i, size_v_i, en_couleur);


    int32_t *prev_DC_Y = calloc(1, sizeof(int32_t));
    int32_t *prev_DC_Cb = calloc(1, sizeof(int32_t));
    int32_t *prev_DC_Cr = calloc(1, sizeof(int32_t));

    uint8_t *mcu_Y = calloc(64*size_MCU, sizeof(uint8_t));
    uint8_t *mcu_Cb = calloc(64*size_MCU, sizeof(uint8_t));
    uint8_t *mcu_Cr = calloc(64*size_MCU, sizeof(uint8_t));

    uint8_t *ligne_Y = calloc(size_h*8*sampling_factor_v, sizeof(uint8_t));
    uint8_t *ligne_Cb = calloc(size_h*8*sampling_factor_v, sizeof(uint8_t));
    uint8_t *ligne_Cr = calloc(size_h*8*sampling_factor_v, sizeof(uint8_t));
/* Ici on a besoin des prev_DC du fait de la façon dont les coefficients sont
encodés. Les variables ligne représentent des lignes de MCU, que l'on construit
au fur et à mesure du décodage de l'image*/

    for (uint32_t i = 0; i < (size_v/(8*sampling_factor_v))-1; i++){
      /*On parcourt l'image sur les lignes de MCU. On différencie la dernière
       ligne car elle sera traitée différemment */
        for (uint32_t j = 0; j < size_h/(8*sampling_factor_h); j++){
          //On parcourt par MCU au sein d'une ligne de MCU
            if (verbeux){
                fprintf(file_blabla, "**************************************************************\n");
                fprintf(file_blabla, "*** mcu %d\n", ((size_h*i)/(8*sampling_factor_h)) + j);
                fprintf(file_blabla, "** component Y\n");
            }
            for (int indice_bloc_v=0; indice_bloc_v < sampling_factor_v;
                      indice_bloc_v++){
                for (int indice_bloc_h=0; indice_bloc_h < sampling_factor_h;
                          indice_bloc_h++){
                  /*On parcourt bloc par bloc au sein d'une MCU.
                  indice_bloc représente l'indice du bloc au sein de la
                   MCU dans laquelle on se trouve, numérotés en ligne*/
                    uint8_t indice_bloc = sampling_factor_h*indice_bloc_v
                                            + indice_bloc_h;

                    if (verbeux){
                        fprintf(file_blabla, "* bloc %d\n", indice_bloc);
                    }

                    *prev_DC_Y = analyse_bloc(prev_DC_Y, jdesc,
                                      mcu_Y+indice_bloc*64, stream, verbeux, 0, file_blabla);
                    /* La fonction reorganise permet de placer les informations
                    décodées du bloc au bon endroit dans la ligne de MCU*/
                    reorganise(mcu_Y+indice_bloc*64, ligne_Y,
                      j*sampling_factor_h+indice_bloc_h, size_h, indice_bloc_v);
                }
            }

            if (verbeux){
                fprintf(file_blabla, "* component mcu\n[   mcu] ");
                affiche_tableau_8(mcu_Y, 64*size_MCU, file_blabla);
            }

            if (en_couleur){

                if (verbeux){
                    fprintf(file_blabla, "** component Cb\n");
                    fprintf(file_blabla, "* bloc 0\n");
                }

                *prev_DC_Cb = analyse_bloc(prev_DC_Cb, jdesc, mcu_Cb,
                                           stream, verbeux, 1, file_blabla);
                upsampling(mcu_Cb, sampling_factor_h, sampling_factor_v);

                if (verbeux){
                    fprintf(file_blabla, "* component mcu\n[   mcu] ");
                    affiche_tableau_8(mcu_Cb, 64*size_MCU, file_blabla);
                    fprintf(file_blabla, "** component Cr\n");
                    fprintf(file_blabla, "* bloc 0\n");
                }

                *prev_DC_Cr = analyse_bloc(prev_DC_Cr, jdesc, mcu_Cr,
                                           stream, verbeux, 1, file_blabla);
                upsampling(mcu_Cr, sampling_factor_h, sampling_factor_v);

                if (verbeux){
                    fprintf(file_blabla, "* component mcu\n[   mcu] ");
                    affiche_tableau_8(mcu_Cr, 64*size_MCU, file_blabla);
                }
            }
            /*On a "rempli" nos tableaux mcu Cb et Cr : il ne nous reste plus
            qu'à placer les informations au bon endroit dans les tableaux
            des lignes de MCU. Pour ça on refait un parcours bloc à bloc,
            comme on l'a fait pour la composante Y*/
            for (int indice_bloc_v=0; indice_bloc_v < sampling_factor_v; indice_bloc_v++){
                for (int indice_bloc_h=0; indice_bloc_h < sampling_factor_h; indice_bloc_h++){

                    uint8_t indice_bloc = indice_bloc_v*sampling_factor_h+indice_bloc_h;
                    reorganise(mcu_Cb+indice_bloc*64, ligne_Cb, j*sampling_factor_h+indice_bloc_h,
                               size_h, indice_bloc_v);
                    reorganise(mcu_Cr+indice_bloc*64, ligne_Cr, j*sampling_factor_h+indice_bloc_h,
                               size_h, indice_bloc_v);
                }
            }
        }

/* Maintenant que toutes les lignes de MCu sont complètes, il ne reste plus qu'à
écrire les infos dans le fichier de sortie et recommencer le traitement sur
 la ligne de MCU suivante*/
        write_tab(fichier, ligne_Y, ligne_Cb, ligne_Cr, en_couleur, size_h_i,
                  sampling_factor_h, sampling_factor_v);
    }
    free(ligne_Y);
    free(ligne_Cb);
    free(ligne_Cr);

    uint8_t *derniere_ligne_Y = calloc(size_h*8*sampling_factor_v, sizeof(uint8_t));
    uint8_t *derniere_ligne_Cb = calloc(size_h*8*sampling_factor_v, sizeof(uint8_t));
    uint8_t *derniere_ligne_Cr = calloc(size_h*8*sampling_factor_v, sizeof(uint8_t));
/*Il reste la dernière ligne de MCU à traiter. L'analyse est la meme, c'est la
méthode pour écrire dans le fichier qui change, car il faut tronquer les données
en trop sur le bas et sur le droite de l'image.*/
    for (uint32_t j = 0; j < size_h/(8*sampling_factor_h); j++){
      //On parcourt par MCU au sein de la ligne

        if (verbeux){
            fprintf(file_blabla, "**************************************************************\n");
            fprintf(file_blabla, "*** mcu %d\n", ((size_h*((size_v/(8*sampling_factor_v))-1))/(8*sampling_factor_h)) + j);
            fprintf(file_blabla, "** component Y\n");
        }

        for (int indice_bloc_v=0; indice_bloc_v < sampling_factor_v; indice_bloc_v++){
            for (int indice_bloc_h=0; indice_bloc_h < sampling_factor_h; indice_bloc_h++){

                uint8_t indice_bloc = indice_bloc_v*sampling_factor_h+indice_bloc_h;

                if (verbeux){
                    fprintf(file_blabla, "* bloc %d\n", indice_bloc);
                }

                *prev_DC_Y = analyse_bloc(prev_DC_Y, jdesc, mcu_Y+indice_bloc*64,
                                          stream, verbeux, 0, file_blabla);
                reorganise(mcu_Y+indice_bloc*64, derniere_ligne_Y, j*sampling_factor_h+indice_bloc_h,
                           size_h, indice_bloc_v);
            }
        }

        if (verbeux){
            fprintf(file_blabla, "* component mcu\n[   mcu] ");
            affiche_tableau_8(derniere_ligne_Y, 64*size_MCU, file_blabla);
        }

        if (en_couleur){

          if (verbeux){
              fprintf(file_blabla, "** component Cb\n");
              fprintf(file_blabla, "* bloc 0\n");
          }

          *prev_DC_Cb = analyse_bloc(prev_DC_Cb, jdesc, mcu_Cb,
                                  stream, verbeux, 1, file_blabla);
          upsampling(mcu_Cb, sampling_factor_h, sampling_factor_v);

          if (verbeux){
              fprintf(file_blabla, "* component mcu\n[   mcu] ");
              affiche_tableau_8(mcu_Cb, 64*size_MCU, file_blabla);
              fprintf(file_blabla, "** component Cr\n");
              fprintf(file_blabla, "* bloc 0\n");
          }

          *prev_DC_Cr = analyse_bloc(prev_DC_Cr, jdesc, mcu_Cr,
                                  stream, verbeux, 1, file_blabla);
          upsampling(mcu_Cr, sampling_factor_h, sampling_factor_v);

          if (verbeux){
              fprintf(file_blabla, "* component mcu\n[   mcu] ");
              affiche_tableau_8(mcu_Cr, 64*size_MCU, file_blabla);
          }
        }

        for (int indice_bloc_v=0; indice_bloc_v < sampling_factor_v; indice_bloc_v++){
            for (int indice_bloc_h=0; indice_bloc_h < sampling_factor_h; indice_bloc_h++){

                uint8_t indice_bloc = indice_bloc_v*sampling_factor_h+indice_bloc_h;
                reorganise(mcu_Cb+indice_bloc*64, derniere_ligne_Cb, j*sampling_factor_h+indice_bloc_h,
                           size_h, indice_bloc_v);
                reorganise(mcu_Cr+indice_bloc*64, derniere_ligne_Cr, j*sampling_factor_h+indice_bloc_h,
                           size_h, indice_bloc_v);
            }
        }
    }
    write_last_tab(fichier, derniere_ligne_Y, derniere_ligne_Cb, derniere_ligne_Cr,
      en_couleur,size_h_i, size_v_i, sampling_factor_h, sampling_factor_v);
    fermeture(fichier, file_blabla, jdesc, derniere_ligne_Cr, derniere_ligne_Cb,
      derniere_ligne_Y, mcu_Cr, mcu_Cb, mcu_Y, prev_DC_Cr, prev_DC_Cb, prev_DC_Y);
    return EXIT_SUCCESS;
}
