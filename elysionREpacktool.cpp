#include <cstdint>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <vector>

namespace fs = std::filesystem;

char header[16] = "Pandora.box";
std::string filepath = "GRPH.PBX";  // the PBX file name

struct index {
    char name[12];
    int shift;
    int size;
};

std::vector<index> fileindex;

void modifyFile(const std::string& filename, uint32_t newData) {
    std::fstream file(filename,
                      std::ios::in | std::ios::out | std::ios::binary);
    // the first line is 'Pandora.box' , following the size of the index
    int offset = 0xc;
    file.seekp(offset);
    file.write(reinterpret_cast<const char*>(&newData), sizeof(newData));
    // Adding the offset for each data in the index
    for (offset += 0x10;; offset += 0x10) {
        file.seekg(offset);
        char buffer[4];

        // Read 4 bytes from the file
        file.read(buffer, sizeof(buffer));
        if (file.gcount() < sizeof(buffer)) {
            break;  // Exit if less than 4 bytes are read
        }

        // Convert the buffer to uint32_t
        uint32_t readData = *reinterpret_cast<uint32_t*>(buffer);
        uint32_t newOffset = readData + newData;  // Calculate new value

        // Write the updated value back to the file at the current offset
        file.seekp(offset);
        file.write(reinterpret_cast<const char*>(&newOffset),
                   sizeof(newOffset));

        // Output the read and new values
        // std::cout << std::hex << readData << " " << newOffset << std::endl;

        // Update newData for the next iteration
        newData = newOffset;
    }

    file.close();
}

int BuildingIndex(std::string filepath) {
    std::fstream outfile;
    fs::path currentDir = fs::current_path();  // scannig path

    int i = 0;

    outfile.open(filepath, std::ios::binary | std::ios::out);

    if (!outfile) {  // Check if the file was opened successfully
        std::cerr << "Error opening file!" << std::endl;
        return 1;
    }
    std::cout << "File created" << std::endl;
    outfile.write(reinterpret_cast<char*>(header), sizeof(header));

    for (const auto& entry : fs::directory_iterator(currentDir)) {
        const auto filenameStr = entry.path().filename().string();
        const auto extensionStr = entry.path().extension().string();
        if (entry.is_directory()) {
            continue;
        } else if (entry.is_regular_file()) {
            if (extensionStr == ".bmp") {
                std::uintmax_t fileSize =
                    std::filesystem::file_size(filenameStr);
                fileindex.push_back(index());
                fileSize = static_cast<uint32_t>(fileSize);
                fileindex[i].size = fileSize;
                fileindex[i].shift = 0;
                // writing index to the newfile
                std::strncpy(fileindex[i].name, filenameStr.c_str(),
                             sizeof(fileindex[i].name));
                // fileindex[i].name[sizeof(fileindex[i].name) - 1] = '\0';
                std::cout << reinterpret_cast<char*>(fileindex[i].name)
                          << std::endl;
                outfile.write(reinterpret_cast<char*>(fileindex[i].name),
                              sizeof(fileindex[i].name));
                outfile.write(reinterpret_cast<char*>(&fileindex[i].size),
                              sizeof(fileindex[i].size));
                i++;
            } else
                ;  // not .bmp
        } else
            ;  // not dir nor file
    }
    outfile.close();
    return 0;
}

void writingfiles() {
    std::fstream outfile(filepath,
                         std::ios::binary | std::ios::out | std::ios::app);
    for (int i = 0; i < fileindex.size(); i++) {
        std::string inputFileName = std::string(fileindex[i].name);
        std::ifstream inputFile(inputFileName, std::ios::binary);
        std::vector<char> buffer(fileindex[i].size);
        inputFile.read(buffer.data(), fileindex[i].size);
        outfile.write(buffer.data(), fileindex[i].size);
        std::cout << "Successfully wrote " << inputFileName << " to "
                  << filepath << std::endl;
        inputFile.close();
    }
    outfile.close();
}

int main() {
    BuildingIndex(filepath);
    std::uintmax_t fileSize = std::filesystem::file_size(filepath);
    modifyFile(filepath, static_cast<uint32_t>(fileSize));
    writingfiles();
    return 0;
}
