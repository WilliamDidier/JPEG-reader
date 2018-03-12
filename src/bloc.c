#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "jpeg_reader.h"
#include "bitstream.h"
#include "huffman.h"
#include "bloc.h"

/* Place le coefficient DC calculé dans le tableau des coefficient du bloc */
void get_DC(const struct jpeg_desc *jdesc, struct bitstream *stream,
            int32_t tab[], int32_t *prev_DC, uint8_t comp_id){

    struct huff_table *table = get_huffman_table(jdesc, DC, get_scan_component_huffman_index(jdesc, DC, get_scan_component_id(jdesc, comp_id)-1));
    uint8_t magnitude = next_huffman_value(table, stream);
    uint32_t indice_m;
    read_bitstream(stream, magnitude, &indice_m, true);
    tab[0] = valeur_magnitude(magnitude, indice_m) + *prev_DC;
}


/* Place le coefficient AC calculé dans le tableau des coefficient du bloc */
int get_AC(const struct jpeg_desc *jdesc, struct bitstream *stream, int32_t tab[], uint8_t comp_id){
    struct huff_table *table = get_huffman_table (jdesc, AC, get_frame_component_quant_index(jdesc, get_frame_component_id(jdesc, comp_id)-1));
    for (int i=1; i<64; i++){
        uint8_t symbole_RLE = next_huffman_value(table, stream);
        uint8_t poids_faible = symbole_RLE & 0xF;
        uint8_t poids_fort = (symbole_RLE>>4);
        if (poids_faible == 0){
            if (poids_fort == 0){
              return EXIT_SUCCESS;
            } else if (poids_fort == 0xF){
              i+=15;
            } else {
              fprintf(stderr, "Symbole RLE non valide\n");
              return EXIT_FAILURE;
            }
        } else {
            i += poids_fort;
            uint32_t dest;
            read_bitstream(stream, poids_faible, &dest, true);
            tab[i] = valeur_magnitude(poids_faible, dest);
        }
    }
    return EXIT_SUCCESS;
}

/* Retourne l'entier codé par un codage par magnitude, en fonction de sa
classe et de son indice de magnitude */
int32_t valeur_magnitude(uint8_t magnitude, uint32_t indice_m){

    int32_t byte;
    if (magnitude == 0){
        return 0;
    } else if (indice_m < (1<<(magnitude-1))) {
        byte = -(1<<magnitude) + indice_m + 1;
    } else {
        byte = indice_m;
    }
    return byte;
}

/* Fonction qui nous a permis de débuguer, et qui permet d'utiliser le
mode verbeux. Permet d'afficher sur la sortie standard les
éléments d'un tableau de coefficients de bloc */
void affiche_tableau(int32_t tab[], uint32_t taille, FILE* file){

    for (uint32_t i = 0; i < taille; i++){
        fprintf(file, "%hx ", tab[i]);
    }
    fprintf(file, "\n");
}

/* Même fonction que précédemment avec des tableaux contenant
des entiers codés sur 8 bits. */
void affiche_tableau_8(uint8_t tab[], uint32_t taille, FILE* file){

    for (uint32_t i = 0; i < taille; i++){
        fprintf(file, "%hx ", tab[i]);
    }
    fprintf(file, "\n");
}
