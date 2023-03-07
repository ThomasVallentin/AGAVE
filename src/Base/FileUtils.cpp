#include "FileUtils.h"

#include <unistd.h>
#include <limits.h>
#include <fstream>
#include <sstream>
#include <iostream>


bool ReadFile(const std::string& filePath, std::string& outContent)
{
    std::ifstream file;
    file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    
    try {
        file.open(filePath);
        std::stringstream stream;
        stream << file.rdbuf();

        outContent = stream.str();

    } catch (std::ifstream::failure &e) {
        fprintf(stderr, "ERROR: %s\nCould not read file %s\n", e.what(), filePath.c_str());
        return false;
    }
    
    return true;
}
