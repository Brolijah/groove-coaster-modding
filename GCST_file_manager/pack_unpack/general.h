#pragma once
#include <filesystem>


bool pack(std::filesystem::path directoryIn, std::filesystem::path fileOut);
bool unpack(std::filesystem::path fileIn, std::filesystem::path directoryOut);

bool to_game_files(std::filesystem::path directoryIn);
bool to_human_files(std::filesystem::path directoryIn);

bool unpack_game(std::filesystem::path gameDir, std::filesystem::path unpackedDir);