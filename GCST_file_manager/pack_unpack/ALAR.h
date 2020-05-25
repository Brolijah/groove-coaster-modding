#pragma once

#include <filesystem>

bool ALAR_unpack(std::filesystem::path fileIn, std::filesystem::path dirOut);
bool ALAR_unpack_dir(std::filesystem::path fileIn, std::filesystem::path dirOut);

bool ALAR_pack(std::filesystem::path dirIn, std::filesystem::path fileOut);
bool ALAR_pack_dir(std::filesystem::path dirIn, std::filesystem::path dirOut);