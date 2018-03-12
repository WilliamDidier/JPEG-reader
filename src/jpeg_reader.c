#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "bitstream.h"
#include "huffman.h"
#include "jpeg_reader.h"


struct jpeg_desc *read_jpeg(const char *filename){

    /* On initialise la structure jpeg_desc */
    struct jpeg_desc *jdesc = malloc(sizeof(struct jpeg_desc));
    jdesc->filename = filename;
    jdesc->stream = create_bitstream(filename);

    /* Initilaisation des variables utilisees par quantization table */
    jdesc->quantization_table_luminance = NULL;
    jdesc->nb_quantization_table = 0;
    uint8_t compteur_tour = 0;

    /* Initilaisation des variables utilisees par huffman table */
    jdesc->nb_huffman_table_DC = 0;
    jdesc->nb_huffman_table_AC = 0;

    jdesc->quantization_index_y = 0;

    uint32_t lg_section;

    /* On itere tant qu'on est pas au debut des donnees brutes */
    uint32_t marqueur_courant = 0;
    while(marqueur_courant != 0xffda){
        read_bitstream(jdesc->stream, 16, &marqueur_courant, true);
        //printf("%hx\n", marqueur_courant);
        switch (marqueur_courant){
            case 0xffd8 :
                //Start of Image (debut des donnees d'image)
                break;
            case 0xffe0 :
                /* Application data */
                /* On verifie que l'en tete contient bien le marqueur JFIF */
                read_bitstream(jdesc->stream, 16, &lg_section, false);
                uint32_t j;
                read_bitstream(jdesc->stream, 8, &j, false);
                uint32_t f;
                read_bitstream(jdesc->stream, 8, &f, false);
                uint32_t i;
                read_bitstream(jdesc->stream, 8, &i, false);
                uint32_t f_bis;
                read_bitstream(jdesc->stream, 8, &f_bis, false);
                uint32_t zero;
                read_bitstream(jdesc->stream, 8, &zero, false);
                uint32_t damso;
                read_bitstream(jdesc->stream, 8, &damso, false);
                /* Cas d'exception */
                if (j != 74 || f != 70 || i != 73 || f_bis != 70 || zero != 0){
                  fprintf(stderr, "ERROR : Le fichier ne contient pas le marqueur JFIF\n");
                  abort();
                }
                break;
            case 0xfffe :
                /* Commentaire */
                /* On ne les traite pas, ils seront consommees par le marqueur_courant */
                break;
            case 0xffdb :
                /* Quantization Tables */
                /* Condition pour definir la table de luminance ssi elle n'a pas deja ete definie */
                if (jdesc->quantization_table_luminance == NULL){
                    jdesc->quantization_table_luminance = calloc(64, sizeof(uint32_t));
                }
                jdesc->quantization_table_chrominance = NULL;
                uint32_t temp_luminance = 0;
                uint32_t temp_chrominance = 0;
                int32_t longueur_section;
                uint32_t longueur_temp;
                read_bitstream(jdesc->stream, 16, &longueur_temp, false);
                longueur_section = (int32_t) longueur_temp;
                /* On a lu 2 octets pour la longueur de la section */
                longueur_section -= 2;
                uint32_t precision;
                /* On boucle selon la longueur de la section si il y a plusieurs tables
                dans une meme section */
                while (longueur_section > 0) {
                    read_bitstream(jdesc->stream, 4, &precision, false);
                    switch (precision){
                        case 0 :
                            precision = 8;
                            break;
                        case 1 :
                            precision = 16;
                            break;
                        default :
                            fprintf(stderr, "ERROR : L'en-tête DQT contient une precision non prise en charge.\n");
                            abort();
                          break;
                    }
                    uint32_t indice;
                    read_bitstream(jdesc->stream, 4, &indice, false);
                    /* 1 octet pour precision + indice */
                    longueur_section -= 1;
                    /* Premiere fois qu'on lit ffdb => table pour la luminance */
                    if (compteur_tour == 0){
                        for (uint8_t i = 0; i < 64; i++){
                            read_bitstream(jdesc->stream, 8, &temp_luminance, false);
                            *(jdesc->quantization_table_luminance+i) = (uint8_t) temp_luminance;
                        }
                        jdesc->nb_quantization_table++;
                    }
                    /* Deuxieme fois qu'on lit ffdb => table pour la chrominance */
                    else if(compteur_tour == 1){
                        jdesc->quantization_table_chrominance = calloc(64, sizeof(uint32_t));
                        for (uint8_t i = 0; i < 64; i++){
                            read_bitstream(jdesc->stream, 8, &temp_chrominance, false);
                            *(jdesc->quantization_table_chrominance+i) = (uint8_t) temp_chrominance;
                        }
                        jdesc->nb_quantization_table++;
                    }
                    /* Il n'y a plus de table à lire */
                    else{
                        break;
                    }
                    /* On a lu 64 octets => 64 valeurs de la table de quantification */
                    longueur_section -= 64;
                    compteur_tour++;
                }
                break;
            case 0xffc0 :
                /* Start of frame */
                longueur_section = 0;
                read_bitstream(jdesc->stream, 16, &longueur_temp, false);
                longueur_section = (int32_t) longueur_temp;
                read_bitstream(jdesc->stream, 8, &precision, false);
                read_bitstream(jdesc->stream, 16, &(jdesc->hauteur), false);
                read_bitstream(jdesc->stream, 16, &(jdesc->largeur), false);
                read_bitstream(jdesc->stream, 8, &(jdesc->nb_composantes), false);
                /* nb_composantes = 1 si noir et blanc, 3 sinon */
                if (jdesc->nb_composantes != 1 && jdesc->nb_composantes != 3){
                    fprintf(stderr, "ERROR : nombre de composantes non pris en compte par le decodeur : != de 1 ou 3\n");
                    abort();
                }
                /* On itere tant qu'il y a des composantes a traiter */
                for (uint8_t cpt = 0; cpt < jdesc->nb_composantes; cpt++){
                    /* Cas Y */
                    if (cpt == 0){
                        read_bitstream(jdesc->stream, 8, &(jdesc->frame_comp_index_y), false);
                        read_bitstream(jdesc->stream, 4, &(jdesc->sampling_factor_h_y), false);
                        read_bitstream(jdesc->stream, 4, &(jdesc->sampling_factor_v_y), false);
                        read_bitstream(jdesc->stream, 8, &(jdesc->quantization_index_y), false);
                    }
                    /* Cas Cb */
                    else if (cpt == 1){
                        read_bitstream(jdesc->stream, 8, &(jdesc->frame_comp_index_cb), false);
                        read_bitstream(jdesc->stream, 4, &(jdesc->sampling_factor_h_cb), false);
                        read_bitstream(jdesc->stream, 4, &(jdesc->sampling_factor_v_cb), false);
                        read_bitstream(jdesc->stream, 8, &(jdesc->quantization_index_cb), false);
                    }
                    /* cas Cr */
                    else{
                        read_bitstream(jdesc->stream, 8, &(jdesc->frame_comp_index_cr), false);
                        read_bitstream(jdesc->stream, 4, &(jdesc->sampling_factor_h_cr), false);
                        read_bitstream(jdesc->stream, 4, &(jdesc->sampling_factor_v_cr), false);
                        read_bitstream(jdesc->stream, 8, &(jdesc->quantization_index_cr), false);
                  }
                }
                break;
            case 0xffc4 :
                /* Huffman Tables */
                longueur_section = 0;
                read_bitstream(jdesc->stream, 16, &longueur_temp, false);
                longueur_section = (int32_t) longueur_temp;
                longueur_section -= 2;
                uint32_t non_utilise;
                uint32_t type;
                uint32_t indice_bis;
                /* On boucle selon la longueur de la section si il y a plusieurs tables
                dans une meme section */
                while (longueur_section > 0) {
                    read_bitstream(jdesc->stream, 3, &non_utilise, false);
                    read_bitstream(jdesc->stream, 1, &type, false);
                    read_bitstream(jdesc->stream, 4, &indice_bis, false);
                    /* 1 octet pour non_utilise + type + indice */
                    longueur_section -= 1;
                    uint16_t nb_byte_read = 0;
                    /* Cas DC */
                    if (type == 0){
                        /* Cas luminance */
                        if(indice_bis == 0){
                          jdesc->huff_table_DC_luminance = load_huffman_table(jdesc->stream, &nb_byte_read);
                        }
                        /* Cas chrominance */
                        else if (indice_bis == 1){
                          jdesc->huff_table_DC_chrominance = load_huffman_table(jdesc->stream, &nb_byte_read);
                        }
                        /* Cas d'exception */
                        else{
                          fprintf(stderr, "ERROR : Indice de la table de Huffman dans la section DHT non pris en charge\n");
                          abort();
                        }
                        jdesc->nb_huffman_table_DC++;
                    }
                    /* Cas AC */
                    else if (type == 1){
                        /* Cas luminance */
                        if(indice_bis == 0){
                          jdesc->huff_table_AC_luminance = load_huffman_table(jdesc->stream, &nb_byte_read);
                        }
                        /* Cas chrominance */
                        else if (indice_bis == 1){
                          jdesc->huff_table_AC_chrominance = load_huffman_table(jdesc->stream, &nb_byte_read);
                        }
                        /* Cas d'exception */
                        else{
                          fprintf(stderr, "ERROR : Indice de la table de Huffman dans la section DHT non pris en charge\n");
                          abort();
                        }
                        jdesc->nb_huffman_table_AC++;
                    }
                    /* Cas d'exception */
                    else{
                        fprintf(stderr, "ERROR : type different de DC ou AC => non pris en charge\n");
                        abort();
                    }
                    /* On a lu nb_byte_read octets dans la table de huffman */
                    longueur_section -= nb_byte_read;
                }
                break;
            case 0xffda :
                /* Start of Scan */
                longueur_section = 0;
                read_bitstream(jdesc->stream, 16, &longueur_temp, false);
                longueur_section = (int32_t) longueur_temp;
                uint32_t nb_composantes_bis;
                read_bitstream(jdesc->stream, 8, &nb_composantes_bis, false);
                jdesc->id_composante_Y = 0;
                jdesc->id_composante_Cb = 0;
                jdesc->id_composante_Cr = 0;
                jdesc->indice_huffman_DC_Y = 0;
                jdesc->indice_huffman_AC_Y = 0;
                jdesc->indice_huffman_DC_Cb = 0;
                jdesc->indice_huffman_AC_Cb = 0;
                jdesc->indice_huffman_DC_Cr = 0;
                jdesc->indice_huffman_AC_Cr = 0;
                /* Cas noir et blanc */
                if (nb_composantes_bis == 1){
                    read_bitstream(jdesc->stream, 8, &(jdesc->id_composante_Y), false);
                    read_bitstream(jdesc->stream, 4, &(jdesc->indice_huffman_DC_Y), false);
                    read_bitstream(jdesc->stream, 4, &(jdesc->indice_huffman_AC_Y), false);
                }
                /* Cas couleur */
                else if (nb_composantes_bis == 3){
                    read_bitstream(jdesc->stream, 8, &(jdesc->id_composante_Y), false);
                    read_bitstream(jdesc->stream, 4, &(jdesc->indice_huffman_DC_Y), false);
                    read_bitstream(jdesc->stream, 4, &(jdesc->indice_huffman_AC_Y), false);
                    read_bitstream(jdesc->stream, 8, &(jdesc->id_composante_Cb), false);
                    read_bitstream(jdesc->stream, 4, &(jdesc->indice_huffman_DC_Cb), false);
                    read_bitstream(jdesc->stream, 4, &(jdesc->indice_huffman_AC_Cb), false);
                    read_bitstream(jdesc->stream, 8, &(jdesc->id_composante_Cr), false);
                    read_bitstream(jdesc->stream, 4, &(jdesc->indice_huffman_DC_Cr), false);
                    read_bitstream(jdesc->stream, 4, &(jdesc->indice_huffman_AC_Cr), false);
                }
                /* Cas d'exception */
                else{
                    fprintf(stderr, "ERROR : nombre de composante du fichier dans la section SOS non pris en charge ( != de 1 ou 3)");
                    abort();
                }
                read_bitstream(jdesc->stream, 24, &non_utilise, false);
                break;
            default :
                break;
        }
    }
    return(jdesc);
}


const char *get_filename(const struct jpeg_desc *jdesc){
    return(jdesc->filename);
}


void close_jpeg(struct jpeg_desc *jdesc){
    if (jdesc != NULL){
        close_bitstream(jdesc->stream);
        free(jdesc);
        jdesc = NULL;
    }
}


struct bitstream *get_bitstream(const struct jpeg_desc *jdesc){
    return(jdesc->stream);
}

uint8_t get_nb_quantization_tables(const struct jpeg_desc *jdesc){
    return(jdesc->nb_quantization_table);
}

uint8_t *get_quantization_table(const struct jpeg_desc *jdesc, uint8_t index){
    if (index == 0){
        return(jdesc->quantization_table_luminance);
    }else if (index == 1){
        return(jdesc->quantization_table_chrominance);
    }else{
        fprintf(stderr, "ERROR : indice invalide\n");
        abort();
    }
}

uint16_t get_image_size(struct jpeg_desc *jdesc,
                        enum direction dir){
    if (dir == DIR_H){
        return((uint16_t) jdesc->largeur);
    }else if (dir == DIR_V){
        return((uint16_t) jdesc->hauteur);
    }else{
        fprintf(stderr, "ERROR : direction invalide\n");
        abort();
    }
}

uint8_t get_nb_components(const struct jpeg_desc *jdesc){
    return ((uint8_t) jdesc->nb_composantes);
}

uint8_t get_frame_component_id(const struct jpeg_desc *jdesc,
                               uint8_t frame_comp_index){
    if(frame_comp_index == 0){
        return((uint8_t) jdesc->frame_comp_index_y);
    }else if(frame_comp_index == 1){
        return((uint8_t) jdesc->frame_comp_index_cb);
    }else if (frame_comp_index == 2){
        return((uint8_t) jdesc->frame_comp_index_cr);
    }else{
        fprintf(stderr, "ERROR : frame_comp_index invalide\n");
        abort();
    }
}

uint8_t get_frame_component_sampling_factor(const struct jpeg_desc *jdesc,
                                            enum direction dir,
                                            uint8_t frame_comp_index){
    if (frame_comp_index == 0){
        if (dir == DIR_H){
            return((uint8_t) jdesc->sampling_factor_h_y);
        }else if (dir == DIR_V){
            return((uint8_t) jdesc->sampling_factor_v_y);
        }else{
            fprintf(stderr, "ERROR : direction invalide\n");
            abort();
        }
    }
    else if (frame_comp_index == 1){
        if (dir == DIR_H){
            return((uint8_t) jdesc->sampling_factor_h_cb);
        }else if (dir == DIR_V){
            return((uint8_t) jdesc->sampling_factor_v_cb);
        }else{
            fprintf(stderr, "ERROR : direction invalide\n");
            abort();
        }
    }
    else if (frame_comp_index == 2){
        if(dir == DIR_H){
            return((uint8_t) jdesc->sampling_factor_h_cr);
        }else if (dir == DIR_V){
            return((uint8_t) jdesc->sampling_factor_v_cr);
        }else{
            fprintf(stderr, "ERROR : direction invalide\n");
            abort();
        }
    }else{
        fprintf(stderr, "ERROR : frame_comp_index invalide\n");
        abort();
    }
}

uint8_t get_frame_component_quant_index(const struct jpeg_desc *jdesc,
                                        uint8_t frame_comp_index){
    if (frame_comp_index == 0){
        return((uint8_t) jdesc->quantization_index_y);
    }else if (frame_comp_index == 1){
        return((uint8_t) jdesc->quantization_index_cb);
    }else if (frame_comp_index == 2){
        return((uint8_t) jdesc->quantization_index_cr);
    }else{
        fprintf(stderr, "ERROR : frame_comp_index invalide\n");
        abort();
    }
}

uint8_t get_nb_huffman_tables(const struct jpeg_desc *jdesc,
                              enum acdc acdc){
    if (acdc == 0){
        return((uint8_t) jdesc->nb_huffman_table_DC);
    }else if (acdc == 1){
        return((uint8_t) jdesc->nb_huffman_table_AC);
    }else{
        fprintf(stderr, "ERROR : enum acdc invalide\n");
        abort();
    }
}

struct huff_table *get_huffman_table(const struct jpeg_desc *jdesc,
                                     enum acdc acdc,
                                     uint8_t index){
    if (acdc == 0){
        if (index == 0){
            return(jdesc->huff_table_DC_luminance);
        }else if (index == 1){
            return(jdesc->huff_table_DC_chrominance);
        }else{
            fprintf(stderr, "ERROR : indice invalide\n");
            abort();
        }
    }
    else if (acdc == 1){
        if (index == 0){
            return(jdesc->huff_table_AC_luminance);
        }else if (index == 1){
            return(jdesc->huff_table_AC_chrominance);
        }else{
            fprintf(stderr, "ERROR : indice invalide\n");
            abort();
        }
    }else{
        fprintf(stderr, "ERROR : enum acdc invalide\n");
        abort();
    }
}

uint8_t get_scan_component_id(const struct jpeg_desc *jdesc,
                              uint8_t scan_comp_index){
    if (scan_comp_index == 0){
        return((uint8_t) jdesc->id_composante_Y);
    }else if (scan_comp_index == 1){
        return((uint8_t) jdesc->id_composante_Cb);
    }else if (scan_comp_index == 2){
        return((uint8_t) jdesc->id_composante_Cr);
    }else{
        fprintf(stderr, "ERROR : scan_comp_index invalide\n");
        abort();
    }
}

uint8_t get_scan_component_huffman_index(const struct jpeg_desc *jdesc,
                                         enum acdc acdc,
                                         uint8_t scan_comp_index){
    if (acdc == 0){
        if (scan_comp_index == 0){
            return((uint8_t) jdesc->indice_huffman_DC_Y);
        }else if (scan_comp_index == 1){
            return((uint8_t) jdesc->indice_huffman_DC_Cb);
        }else if (scan_comp_index == 2){
            return((uint8_t) jdesc->indice_huffman_DC_Cr);
        }else{
            fprintf(stderr, "ERROR : scan_comp_index invalide\n");
            abort();
        }
    }
    else if (acdc == 1){
        if (scan_comp_index == 0){
            return((uint8_t) jdesc->indice_huffman_AC_Y);
        }else if (scan_comp_index == 1){
            return((uint8_t) jdesc->indice_huffman_AC_Cb);
        }else if (scan_comp_index == 2){
            return((uint32_t) jdesc->indice_huffman_AC_Cr);
        }else{
            fprintf(stderr, "ERROR : scan_comp_index invalide\n");
            abort();
        }
    }else{
        fprintf(stderr, "ERROR : enum acdc invalide\n");
        abort();
    }
}
