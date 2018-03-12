#include <stdlib.h>
#include <stdio.h>

#include "jpeg_reader.h"
#include "bitstream.h"


int main(int argc, char **argv)
{
    if (argc != 2) {
	/* Si y'a pas au moins un argument en ligne de commandes, on
	 * boude. */
	fprintf(stderr, "Usage: %s fichier.jpeg\n", argv[0]);
	return EXIT_FAILURE;
    }
    /* On recupere le nom du fichier JPEG sur la ligne de commande. */
    const char *filename = argv[1];

    /* On cree un jpeg_desc qui permettra de lire ce fichier. */
    struct jpeg_desc *jdesc = read_jpeg(filename);

    /* On recupere le flux des donnees brutes a partir du descripteur. */
    struct bitstream *stream = get_bitstream(jdesc);

    /* On lit un octet depuis le flux, et on l'affiche. */
    uint32_t byte = 0;
    uint8_t nb_read = read_bitstream(stream, 8, &byte, false);
    if (nb_read == 8) {
	printf("Le premier octet des donnees brutes du fichier JPEG %s vaut 0x%x.\n",
	       filename,
	       byte);
    } else {
	fprintf(stderr, "Erreur de lecture! ");
	fprintf(stderr, "(on voulait en lire 8 bits, on n'en a lu que %u)\n", nb_read);
    }

    /* Nettoyage de printemps : close_jpeg ferme aussi le bitstream
     * (voir Annexe C du sujet). */
    close_jpeg(jdesc);

    /* On se congratule. */
    return EXIT_SUCCESS;
}
