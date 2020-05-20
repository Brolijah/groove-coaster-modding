
#define _CRT_SECURE_NO_WARNINGS //disable the annoying "ooh your fopen is unsafe, use our system instead" errors

#include "..\include\dirent.h"
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <sys/stat.h>
#include <sys/types.h>




/*
    file order is important, name is not

    structure header :
      -char 4: "ALAR"
      -byte  : type
      -byte  : ???
      -int16 : amount of files
      -byte 8: ID's (see "ID structure")
      -struct alar_files

    structure alar_files :
      -int32 : type
      -int32 : start
      -int32 : size
      -byte 4: ??? (constant 0x01000080)

      "file type" structure (i think): (from song "shiva", file "Stage00382.aar")
        XX XX 50 48 = .asn
        XX XX 50 00 = .dat

        E0 XX XX XX starting constant, defined in first byte of ID (E0 is example from shiva)
        XX 17 XX XX constant, defined in second byte of ID (17 is example from shiva)
        
        after each difficulty, one is skipped. as follows:

        E0 sound bgm_b-382_shiva_BGM.asn
        E1 sound bgm_b-382_shiva_SHOT.asn

        E0 easy ac_shiva_easy.dat
        E1 easy ac_shiva_easy_clip.dat
        E2 easy ac_shiva_easy_ext.dat
          E3 (next diff, so skipped)
        E4 normal ac_shiva_normal.dat
        E5 normal ac_shiva_normal_clip.dat
        E6 normal ac_shiva_normal_ext.dat
          E7 (next diff, so skipped)
        E8 hard ac_shiva_hard.dat
        E9 hard ac_shiva_hard_clip.dat
        EA hard ac_shiva_hard_ext.data
    
    structure IDs: this is where the start & end type are stored from the file list. the start can PROBABLY be anything you like, as long as all your files fit inside.
        -int32 : "starting file type", example: 0xE0175000 from shiva
        -int32 : "ending file type:, example: 0xEA175000 from shiva

    file names are stored 0x22 before the "start" of a file, and are 0x20 long
*/



struct HEADERFILEENTRY {
    uint32_t type;
    uint32_t offset;
    uint32_t size;
    uint8_t name[0x20];
};

#define fileOrderLength 13
char fileOrder[fileOrderLength][2][20] = {
    {"easy.dat", 0x00},
    {"BGM.asn", 0x00},
    {"easy_clip.dat", 0x01},
    {"SHOT.asn", 0x01},
    {"easy_ext.dat", 0x02},
    {"__SWITCH__", 0xFF},
    {"normal.dat", 0x04},
    {"normal_clip.dat", 0x05},
    {"normal_ext.dat", 0x06},
    {"__SWITCH__", 0x07},
    {"hard.dat", 0x08},
    {"hard_clip.dat", 0x09},
    {"hard_ext.dat", 0x0A}
};

#define fOrderEasyLength 20
char fOrderEasy[fOrderEasyLength][20] = {
    "easy.dat",
    "easy_clip.dat",
    "easy_ext.dat"
};

#define fOrderNormalLength 20
char fOrderNormal[fOrderNormalLength][20] = {
    "normal.dat",
    "normal_clip.dat",
    "normal_ext.dat"
};

#define fOrderHardLength 20
char fOrderHard[fOrderHardLength][20] = {
    "hard.dat",
    "hard_clip.dat",
    "hard_ext.dat"
};

#define fOrderSoundLength 20
char fOrderSound[fOrderSoundLength][20] = {
    "BGM.asn",
    "SHOT.asn"
};

void unpack(char* fileIn, char* directoryOut) {
    char* headerTypeName[5]; //should be "ALAR"
    uint8_t headerType;

    FILE* fp;
    fp = fopen(fileIn, "rb");
    fread(&headerTypeName, sizeof(uint8_t), 4, fp); //read header type name, should be "ALAR"
    if (strstr(headerTypeName, "ALAR")) {
        printf("ALAR file supplied\n");
    }
    else {
        printf("the file supplied is not an ALAR file!\n");
    }

    fread(&headerType, sizeof(uint8_t), 1, fp); //read the type of header
    fseek(fp, 1, SEEK_CUR); //advance file pointer one byte

    uint8_t* ID[8];
    uint16_t headerFileCount;
    if (headerType == 2) {
        fread(&headerFileCount, sizeof(uint16_t), 1, fp);
        printf("there are %d files in this file\n", headerFileCount);
        fread(&ID, sizeof(uint8_t), 8, fp);
        printf("the ID's in this file are: %x, %x, %x, %x, %x, %x, %x, %x\n", (unsigned char)ID[0], (unsigned char)ID[1], (unsigned char)ID[2], (unsigned char)ID[3], (unsigned char)ID[4], (unsigned char)ID[5], (unsigned char)ID[6], (unsigned char)ID[7]);

        //now read all file data
        struct HEADERFILEENTRY* files = malloc(headerFileCount * sizeof(struct HEADERFILEENTRY));
        int i;
        for (i = 0; i < headerFileCount; i++) {
            printf("working on file %d\n", i + 1);
            fread(&files[i].type, sizeof(uint32_t), 1, fp);
            printf("type: %d\n", (uint32_t)files[i].type);
            fread(&files[i].offset, sizeof(uint32_t), 1, fp);
            printf("offset: %d\n", (uint32_t)files[i].offset);
            fread(&files[i].size, sizeof(uint32_t), 1, fp);
            printf("size: %d\n", (uint32_t)files[i].size);
            fseek(fp, sizeof(uint32_t), SEEK_CUR); //there's a weird fourth value in it

            //get the name
            int tempseek = ftell(fp);
            fseek(fp, files[i].offset - 0x22, SEEK_SET);
            fread(&files[i].name, sizeof(uint8_t), 0x20, fp);
            printf("name: %.32s\n", files[i].name);
            fseek(fp, tempseek, SEEK_SET);
        }

        /*FILE* fpo;
        fpo = fopen("ac_shiva_normal.dat", "wb");
        char poyo[32] = "ac_data_stuff_shiva";
        fwrite(&poyo, sizeof(uint8_t), 32, fpo);
        fclose(fpo);*/
        char outFile[120 + 32];
        FILE* fpout;
        for (i = 0; i < headerFileCount; i++) {
            fseek(fp, files[i].offset, SEEK_SET);
            char* fileData = (char*)malloc(files[i].size * sizeof(uint8_t));
            //read data into fileData
            printf("reading data into mem\n");
            fread(fileData, sizeof(uint8_t), files[i].size, fp);


            snprintf(&outFile, 120 + 32, "%s%.32s", directoryOut, files[i].name);
            printf("filename: %s\n", outFile);
            printf("reading data into file %d with size %d\n", i, files[i].size);
            //printf("filedata: %s", &fileData);

            fpout = fopen(&outFile, "wb");
            printf("%s", fpout);
            fwrite(fileData, sizeof(uint8_t), files[i].size, fpout);
            fclose(fpout);

            free(fileData);
        }

    }

    fclose(fp);
    return;
}

void unpack_dir(char* directoryIn, char* directoryOut) {
    struct DIR* indir;
    indir = opendir(directoryIn);
    struct dirent* entry;
    printf("files to be unpacked:\n");
    uint16_t fileCount = 0;
    while ((entry = readdir(indir)) != NULL) {
        if (entry->d_name[0] != '.') {
            printf("    %s\n", entry->d_name);
            char tmpFileIn[120];
            snprintf(&tmpFileIn, 120, "%s%s", directoryIn, entry->d_name);
            char tmpDirOut[120];
            snprintf(&tmpDirOut, 120, "%s%s", directoryOut, entry->d_name);
            mkdir(&tmpDirOut, 0700);
            char tmpFileOut[120];
            snprintf(&tmpFileOut, 120, "%s/", tmpDirOut);
            printf("DIR OUT: %s\n", tmpFileOut);
            
            unpack(&tmpFileIn, &tmpFileOut);
            fileCount++;
        }
    }
    closedir(indir);
}

void pack(uint32_t songID, char* directoryIn, char* fileOut) {
    //headertypename = ALAR
    //headertype = 2
    FILE* fp;
    fp = fopen(fileOut, "wb");
    fwrite("ALAR", sizeof(char), 4, fp); //ALAR
    fwrite("\x2", sizeof(uint8_t), 1, fp); //type of file
    fseek(fp, 1, SEEK_CUR); //advance pointer once

    //should be user-defined later on
    //uint16_t headerIDstartCount = 0x17E0; //inverted cuz endian
    uint16_t headerIDstartCount = 0x0000; //inverted cuz endian
    printf("song ID: %d\n", songID);
    headerIDstartCount = songID << 4;

    uint16_t headerIDstartType = 0x0050; //inverted cuz endian

    uint8_t fileTypeSoundConstant = 0x48;

    uint32_t headerIDstart = headerIDstartCount ^ (headerIDstartType << 16); //inverted cuz endian
    uint32_t headerIDend = 0xDEADBEEF; //will be adjusted later

    uint8_t fileTypeCounter = headerIDstartCount & 0x00FF; //for loading the files into the file
    uint8_t fileTypeSoundCounter = fileTypeCounter;
    


    struct DIR* indir;
    indir = opendir(directoryIn);
    struct dirent* entry;
    printf("files to be packed:\n");
    uint16_t fileCount = 0;
    while ((entry = readdir(indir)) != NULL) {
        if (entry->d_name[0] != '.') {
            printf("    %s\n", entry->d_name);
            fileCount++;
        }
    }
    closedir(indir);
    struct HEADERFILEENTRY* files = malloc(fileCount * sizeof(struct HEADERFILEENTRY));
    printf("total file count: %d\n", fileCount);
    fwrite(&fileCount, sizeof(uint16_t), 1, fp);

    fwrite(&headerIDstart, sizeof(uint8_t), 4, fp);
    fwrite(&headerIDend, sizeof(uint8_t), 4, fp); //just zeroes for now, should be updated once done writing all files

    indir = opendir(directoryIn);
    readdir(indir); readdir(indir); //clean out the "." and ".."
    int i;
    for (i = 0; i < fileCount; i++) {
        entry = readdir(indir);
        snprintf(&files[i].name, 0x20, "%s", entry->d_name);
        if (strstr(entry->d_name, ".asn")) {
            files[i].type = headerIDstartType ^ 0x4800;
        }
        else if (strstr(entry->d_name, ".dat")) {
            files[i].type = headerIDstartType;
        }
        FILE* fpi;
        char fileIn[120];
        snprintf(&fileIn, 120, "%s%s", directoryIn, files[i].name);
        fpi = fopen(&fileIn, "rb");
        fseek(fpi, 0, SEEK_END);
        files[i].size = (uint32_t)ftell(fpi);
        fclose(fpi);
        printf("file name: %s, file size: %d, file type (not final): %x\n", files[i].name, files[i].size, files[i].type);
    }

    
    //add empty index table for file offset
    for (i = 0; i < fileCount; i++) {
        fwrite("EmptyIndexEntry!", sizeof(uint8_t), 16, fp);
    }
    fwrite("\x1\x1", sizeof(uint8_t), 2, fp); //add two empty bytes, since the OG files also have it.

    

    int progress = 0;
    int indexprogress = 16;
    //use fileTypeCounter
    //use fileTypeSoundCounter

    //sort all files first
    //struct HEADERFILEENTRY* filesSorted = malloc(fileCount * sizeof(struct HEADERFILEENTRY));
    int b;
    for (b = 0; b < 1; b++) {
        for (i = 0; i < fileOrderLength; i++) {
            int a;
            if (strstr(&fileOrder[i][0], "__SWITCH__")) {
                printf("switching difficulty\n");
                continue;
            }
            printf("looking for %s (priority %x) in:\n", fileOrder[i][0], fileOrder[i][1][0]);
            for (a = 0; a < fileCount; a++) {
                printf("    %s\n", files[a].name);
                if (strstr(&files[a].name, &fileOrder[i][0])) {
                    printf("MATCH FOUND!\n");

                    char* fileData = (char*)malloc(files[a].size * sizeof(uint8_t));
                    FILE* fpi;
                    char fileIn[120];
                    snprintf(&fileIn, 120, "%s%s", directoryIn, files[a].name);
                    fpi = fopen(&fileIn, "rb");
                    //read data into fileData
                    printf("saving data from file %s into the archive\n", files[a].name);
                    fread(fileData, sizeof(uint8_t), files[a].size, fpi);
                    fclose(fpi);


                    fwrite(&files[a].name, sizeof(uint8_t), 0x20, fp); //write name
                    fseek(fp, 2, SEEK_CUR); //advance pointer twice
                    int fileOffsetPreFile = ftell(fp);
                    fwrite(fileData, sizeof(uint8_t), files[a].size, fp); //write file data
                    free(fileData);
                    //update index
                    int fileOffset = ftell(fp);
                    fseek(fp, indexprogress, SEEK_SET);
                    indexprogress += 16;

                    //update "type" in index
                    //uint16_t type = ((headerIDstartCount & 0xFF00) ^ fileOrder[a][1][0] ^ (headerIDstart));
                    uint16_t type = headerIDstartCount ^ fileOrder[i][1][0];
                    uint32_t typeComplete = (files[a].type & 0x0000FFFF) << 16;
                    typeComplete ^= type;
                    headerIDend = typeComplete;
                    fwrite(&typeComplete, sizeof(uint32_t), 1, fp);

                    uint32_t fileStart = (uint32_t)fileOffsetPreFile;
                    printf("fileOffset: %x\n", fileStart); //broken?
                    fwrite(&fileStart, sizeof(uint32_t), 1, fp);

                    fwrite(&files[a].size, sizeof(uint32_t), 1, fp);

                    uint32_t magicalValue = 0x80000001;
                    fwrite(&magicalValue, sizeof(uint32_t), 1, fp);

                    fseek(fp, fileOffset, SEEK_SET);


                    progress++;

                    break;
                }
            }
        }
    }

    //correcting the "end" part of IDs
    fseek(fp, 0xc, SEEK_SET);
    fwrite(&headerIDend, sizeof(uint8_t), 4, fp); //just zeroes for now, should be updated once done writing all files

    printf("packed %d files into %s with ID %d", fileCount, fileOut, songID);

    fclose(fp);
}

int main(void) {
    //unpack("./packed_in/Stage00951.aar", "./unpacked/Stage00951.aar/");
    unpack_dir("./packed_in/", "./unpacked/");
    printf("\n\n\n\n\n\n\n\n\n\n\n\n\n");
    pack(951, "./unpacked/Stage00951.aar/", "./packed_out/Stage00951.aar");
    //unpack();
    

    return 0;
}
