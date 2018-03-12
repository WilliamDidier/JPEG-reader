# Repertoires du projet

BIN_DIR = bin
SRC_DIR = src
INC_DIR = include
OBJ_DIR = obj
OBJPROF_DIR = obj-prof

# Options de compilation/édition des liens

CC = clang
LD = clang
INC = -I$(INC_DIR)

CFLAGS += $(INC) -Wall -std=c99 -O0 -g  -Wextra
LDFLAGS = -lm

# Liste des fichiers objet

# les notres...
OBJPROF_FILES =  $(OBJPROF_DIR)/huffman.o

# et les votres!
OBJ_FILES = $(OBJ_DIR)/main.o $(OBJ_DIR)/idct.o $(OBJ_DIR)/write_ppm.o $(OBJ_DIR)/bloc.o $(OBJ_DIR)/inverse_zigzag.o $(OBJ_DIR)/quantification.o $(OBJ_DIR)/traitement_bloc.o $(OBJ_DIR)/upsampling.o $(OBJ_DIR)/bitstream.o $(OBJ_DIR)/jpeg_reader.o

# cible par défaut

TARGET = $(BIN_DIR)/jpeg2ppm

all: $(TARGET)

$(TARGET): $(OBJPROF_FILES) $(OBJ_FILES)
	$(LD) $(LDFLAGS) $(OBJPROF_FILES) $(OBJ_FILES) -o $(TARGET)

$(OBJ_DIR)/main.o: $(SRC_DIR)/main.c $(INC_DIR)/jpeg_reader.h $(INC_DIR)/bitstream.h $(INC_DIR)/huffman.h $(INC_DIR)/bloc.h $(INC_DIR)/inverse_zigzag.h $(INC_DIR)/quantification.h $(INC_DIR)/idct.h $(INC_DIR)/upsampling.h
	$(CC) $(CFLAGS) -c $(SRC_DIR)/main.c -o $(OBJ_DIR)/main.o
$(OBJ_DIR)/bloc.o: $(SRC_DIR)/bloc.c $(INC_DIR)/jpeg_reader.h $(INC_DIR)/bitstream.h $(INC_DIR)/huffman.h $(INC_DIR)/bloc.h
	$(CC) $(CFLAGS) -c $(SRC_DIR)/bloc.c -o $(OBJ_DIR)/bloc.o
$(OBJ_DIR)/write_ppm.o: $(SRC_DIR)/write_ppm.c $(INC_DIR)/jpeg_reader.h $(INC_DIR)/bitstream.h $(INC_DIR)/huffman.h $(INC_DIR)/write_ppm.h
	$(CC) $(CFLAGS) -c $(SRC_DIR)/write_ppm.c -o $(OBJ_DIR)/write_ppm.o
$(OBJ_DIR)/inverse_zigzag.o: $(SRC_DIR)/inverse_zigzag.c $(INC_DIR)/jpeg_reader.h $(INC_DIR)/bitstream.h $(INC_DIR)/huffman.h $(INC_DIR)/inverse_zigzag.h
	$(CC) $(CFLAGS) -c $(SRC_DIR)/inverse_zigzag.c -o $(OBJ_DIR)/inverse_zigzag.o
$(OBJ_DIR)/quantification.o: $(SRC_DIR)/quantification.c $(INC_DIR)/jpeg_reader.h $(INC_DIR)/bitstream.h $(INC_DIR)/huffman.h $(INC_DIR)/quantification.h
	$(CC) $(CFLAGS) -c $(SRC_DIR)/quantification.c -o $(OBJ_DIR)/quantification.o
$(OBJ_DIR)/idct.o: $(SRC_DIR)/idct.c $(INC_DIR)/jpeg_reader.h $(INC_DIR)/bitstream.h $(INC_DIR)/huffman.h $(INC_DIR)/idct.h
	$(CC) $(CFLAGS) -c $(SRC_DIR)/idct.c -o $(OBJ_DIR)/idct.o
$(OBJ_DIR)/traitement_bloc.o: $(SRC_DIR)/traitement_bloc.c $(INC_DIR)/jpeg_reader.h $(INC_DIR)/bitstream.h $(INC_DIR)/huffman.h $(INC_DIR)/idct.h $(INC_DIR)/bitstream.h $(INC_DIR)/bloc.h $(INC_DIR)/quantification.h $(INC_DIR)/inverse_zigzag.h $(INC_DIR)/write_ppm.h $(INC_DIR)/traitement_bloc.h
	$(CC) $(CFLAGS) -c $(SRC_DIR)/traitement_bloc.c -o $(OBJ_DIR)/traitement_bloc.o
$(OBJ_DIR)/upsampling.o: $(SRC_DIR)/upsampling.c $(INC_DIR)/jpeg_reader.h $(INC_DIR)/bitstream.h $(INC_DIR)/huffman.h $(INC_DIR)/upsampling.h
	$(CC) $(CFLAGS) -c $(SRC_DIR)/upsampling.c -o $(OBJ_DIR)/upsampling.o
$(OBJ_DIR)/jpeg_reader.o: $(SRC_DIR)/jpeg_reader.c $(INC_DIR)/jpeg_reader.h $(INC_DIR)/bitstream.h $(INC_DIR)/huffman.h
	$(CC) $(CFLAGS) -c $(SRC_DIR)/jpeg_reader.c -o $(OBJ_DIR)/jpeg_reader.o
$(OBJ_DIR)/bitstream.o: $(SRC_DIR)/bitstream.c
	$(CC) $(CFLAGS) -c $(SRC_DIR)/bitstream.c -o $(OBJ_DIR)/bitstream.o
.PHONY: clean

clean:
	rm -f $(TARGET) $(OBJ_FILES)
