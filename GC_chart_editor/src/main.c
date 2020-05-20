#define _CRT_SECURE_NO_WARNINGS //disable the annoying "ooh your fopen is unsafe, use our system instead" errors

//#include "..\include\dirent.h"
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <sys/stat.h>
#include <sys/types.h>

/*
header:
	int32 : ???
	int32 : ???
	int32 : pointer to start of visual table - 0x20 (in human-endian)
	 \> take last 16 bits, not quite sure if it's 32-bit yet
	int32 : offset for where the gameplay table begins (with human-endian, no weird reversal)
	 \> take last 16 bits, not quite sure if it's 32-bit yet

visual table: every "entry" is 16 bytes long and consists of 4 32-bit values
	int32 : some type of upwards-going value. starting at 00 00 00 01 probably
		FFFFFFFF makes the line go straight forward (you can't see it)
	int32 : random thing, stays consistent at the end
	int32 : 

gameplay table:
	

*/

//0x1502 - 0x1509

void humanize(char* fileIn, char* fileOut) {
	FILE* fpi;
	fpi = fopen(fileIn, "rb");
	fseek(fpi, 0, SEEK_END);
	int finSize = ftell(fpi);
	fseek(fpi, 0, SEEK_SET);
	//char* fileData = (char*)malloc(finSize * sizeof(uint8_t));
	//fread(&fileData, sizeof(uint8_t), finSize, fpi);

	FILE* fpo;
	fpo = fopen(fileOut, "w");

	char tempstr[0xff];

	//write header stuff
	fwrite("HEADER:\n", 1, 8, fpo);
	fseek(fpi, 14, SEEK_SET);
	uint32_t tempvar;
	uint32_t gameplayTableOffset;
	fread(&gameplayTableOffset, sizeof(int32_t), 1, fpi);
	//offset currently only works with 16-bit values!
	//gameplayTableOffset = ((gameplayTableOffset & 0xFF000000) >> 24) ^ ((gameplayTableOffset & 0x00FF0000) >> 8) ^ ((gameplayTableOffset & 0x0000FF00) << 8) ^ ((gameplayTableOffset & 0x000000FF) << 24);
	gameplayTableOffset = ((gameplayTableOffset & 0xFF00) >> 8) ^ ((gameplayTableOffset & 0x00FF) << 8);
	snprintf(&tempstr, 0xff, "  gameplay table offset: %d\n", (uint32_t)gameplayTableOffset);
	fwrite(&tempstr, sizeof(int8_t), strlen(&tempstr), fpo);

	fseek(fpi, (uint32_t)gameplayTableOffset + 4, SEEK_SET);
	uint32_t gameplayTableEntries;
	fread(&gameplayTableEntries, sizeof(int32_t), 1, fpi);
	gameplayTableEntries = ((gameplayTableEntries & 0xFF000000) >> 24) ^ ((gameplayTableEntries & 0x00FF0000) >> 8) ^ ((gameplayTableEntries & 0x0000FF00) << 8) ^ ((gameplayTableEntries & 0x000000FF) << 24);
	printf("gameplaytable entries: %x\n", (uint32_t)gameplayTableEntries);
	snprintf(&tempstr, 0xff, "  gameplay table entry count: %d\n", (uint32_t)gameplayTableEntries);
	fwrite(&tempstr, 1, strlen(&tempstr), fpo);

	//write visual table
	fwrite("\nVISUAL TABLE:\n", 1, 15, fpo);

	//write gameplay table
	fwrite("\nGAMEPLAY TABLE:\n", 1, 17, fpo);
	fseek(fpi, (uint32_t)gameplayTableOffset + 0x63, SEEK_SET);
	for (int i = 0; i < gameplayTableEntries; i++) {
		uint32_t magicCounter1;
		fread(&magicCounter1, sizeof(uint32_t), 1, fpi);

		snprintf("thing: %d, magicCounter1: %d, ??type1: %d, ??type2: %d, ")
	}


	fclose(fpi);
	fclose(fpo);
	return;
}

chartisize(char* fileIn, char* fileOut) {
	//use strtol
}

int main(void) {
	humanize("C:/dev/groove-coaster-modding/ALAR_decode_encode/unpacked/Stage00951.aar/ac_plotmgc_hard.dat", "./humanized/ac_plotmgc_hard.txt");


	return 0;
}