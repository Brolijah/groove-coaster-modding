#pragma once
#include <stdint.h>
int main_LZSS(char* pathIn, char* pathOut);
size_t ALLZ_Decode(uint8_t** ptr_dst, uint8_t* src, size_t srcSize);
int32_t ALLZ_Setup_EncFlags(int32_t* encFlags, uint8_t alFlag, uint8_t** ptr_encoded_data);
int32_t ALLZ_AnalyzeBlock(int32_t* encFlags, uint8_t alFlag, uint8_t** ptr_encoded_data);