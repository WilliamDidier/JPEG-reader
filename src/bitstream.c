#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
/*On a défini la structure suivante :
  point_courant :     Pointeur sur le fichier
  buffer :            Permet de stocker les informations lues en TROP
  size_buffer :       Nombre de bits stockés dans le buffer (entre 0 et 7)
  dernier_octet_lu :  Dernier octet complet lu par la fonction read_bitstream.
                      Permet d'implémenter le byte stuffing.
                      On ne s'en sert que pour savoir si il vaut ff ou non,
                      on aurait donc pu faire le choix d'implémenter un booléen
  */
struct bitstream {
  FILE* point_courant;
  uint8_t buffer;
  uint8_t size_buffer;
  uint8_t dernier_octet_lu;
};

struct bitstream *create_bitstream(const char *filename){
  struct bitstream *stream = malloc(sizeof(struct bitstream));
  stream -> buffer = 0;
  stream -> size_buffer = 0;
  stream -> dernier_octet_lu = 0;
  stream -> point_courant = NULL;
  stream -> point_courant = fopen(filename, "rb");
  if(stream -> point_courant == NULL){
    fprintf(stderr, "ERROR: unable to create bitstream from file %s\n", filename);
    return NULL;
  }

  return stream;
}

void close_bitstream(struct bitstream *stream){
    if (stream != NULL){
        fclose(stream -> point_courant);
        free(stream);
    } else {
        fprintf(stderr, "ERROR : Stream is NULL\n");
    }
}

bool end_of_bitstream(struct bitstream *stream){
  uint32_t dest;
  /*La fonction feof renvoie true ssi le pointeur sur le fichier se situe après EOF.
  On veut que la fonction renvoie true ssi le pointeru est juste avant EOF.
  Donc on lit un caractère, si ensuite la fonction feoef renvoie true on renvoie true
  Si on n'est pas à la fin du fichier, on utilise fseek pour remettre le
  pointeur dans son état initial. */
  fread(&dest, 1, 1, stream -> point_courant);
  if(feof(stream -> point_courant)){
      return true;
  } else {
      fseek(stream -> point_courant, -1, SEEK_CUR);
      return false;
  }
}

uint8_t read_bitstream(struct bitstream *stream, uint8_t nb_bits,
                       uint32_t *dest, bool discard_byte_stuffing){
//Idée : on lit forcément un octet. Si on devait lire moins, on met le reste
// dans un buffer que l'on traite en priorité à chaque appel.
    *dest = 0;
    uint8_t bits_lus = 0;
    int16_t nb_bits_16 = (int16_t) nb_bits;
    //Traitement du rab:
    if (stream -> size_buffer > 0){
        //Si il y a moins de bits qui trainent que de bits à lire on les lit et on continue
        *dest = stream -> buffer;
        if (stream -> size_buffer < nb_bits_16){
            nb_bits_16 -= stream -> size_buffer;
            bits_lus += stream -> size_buffer;
            *dest = stream -> buffer;
            stream -> size_buffer = 0;
            stream -> dernier_octet_lu = 0;

        //Sinon on lit les bits de poids fort du buffer (décalage à droite)
        //Et on les enlèves du buffer (décalages à droite et à gauche pour garder que le poids faible)
        } else {
            *dest = stream -> buffer >> (stream -> size_buffer - nb_bits_16);
            stream -> size_buffer -= nb_bits_16;
            stream -> buffer = stream -> buffer << (8-stream -> size_buffer);
            stream -> buffer = stream -> buffer >> (8-stream -> size_buffer);
            bits_lus += nb_bits_16;
            nb_bits_16 = 0;

        }
    }
    //Tant qu'il y a des bits à lire on boucle
    uint8_t lecture;
    while(nb_bits_16 > 0){
      //On commence par décaler le resultat pour "faire de la place" pour ce qu'on va lire
        *dest = *dest << 8;
        fread(&lecture, 1, 1, stream -> point_courant);
        if(end_of_bitstream(stream)){
            fprintf(stderr, "ERROR : Pas suffisament de bits à lire dans le fichier\n");
            return bits_lus;
            abort();
        }
        stream -> dernier_octet_lu = lecture;
        *dest += stream -> dernier_octet_lu;

        nb_bits_16 -= 8;
        bits_lus += 8;
        if ((stream -> dernier_octet_lu == 0xff) && (discard_byte_stuffing)){
          uint8_t test;
          fread(&test, 1, 1, stream -> point_courant);
            if (test != 0x00){
              fseek(stream -> point_courant, -1, SEEK_CUR);
            }
        }
    }
    if (nb_bits_16 < 0){
        //On a lu -nb_bits_16 bits de trop : on les met dans le buffer
        stream -> size_buffer = -nb_bits_16;
        stream -> buffer = *dest << (8+nb_bits_16);
        stream -> buffer = stream -> buffer >> (8+nb_bits_16);
        //on décrémente bits_lus du nombre de bits en trop qu'on a lus
        bits_lus += nb_bits_16;
        //On veut garder les 8 + nb_bits_16 bits de poids fort de dest
        //C'est ceux qu'on voulait effectivement lire
        *dest = *dest >> -nb_bits_16;
        //on n'a pas lu un octet complet donc on met dernier_octet_lu
        stream -> dernier_octet_lu = 0;

    }
    return bits_lus;
}
