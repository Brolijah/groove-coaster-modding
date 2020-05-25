#include <filesystem>

bool ALTX_unpack_pam(std::filesystem::path fileIn, std::filesystem::path fileOut);
bool ALTX_unpack_dir(std::filesystem::path directory);
bool ALTX_repack_tga(std::filesystem::path fileIn, std::filesystem::path fileOut);