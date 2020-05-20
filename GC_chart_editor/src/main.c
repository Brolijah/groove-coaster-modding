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
	int32 : offset for where the gameplay table begins (with human-endian, no weird reversal)

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
	char* fileData = (char*)malloc(finSize * sizeof(uint8_t));
	fread(&fileData, sizeof(uint8_t), finSize, fpi);

	FILE* fpo;
	fpo = fopen(fileOut, "wb");

	return;
}


int main(void) {
	humanize("C:/dev/groove-coaster-modding/ALAR_decode_encode/unpacked/Stage00951.aar/ac_plotmgc_hard.dat", "./humanized/ac_plotmgc_hard.txt");


	return 0;
}