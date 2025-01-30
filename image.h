#ifndef IMAGE_H
#define IMAGE_H

#include <iostream>
#include <string>
#include <fstream>
#include <bitset>
#include <vector>


class Image{
public:
    explicit Image(const std::string& filepath);
    bool isValidFormat();
    void static help();
    void info();
    void decode();
    void encode(const std::string& message);
    bool check(const std::string& message) const;
    void write();
private:
    std::string filename;
    std::string magicNum;
    int width;
    int height;
    int filesize;
    int ppmMaxVal;
    std::vector<unsigned char> header;
    std::vector<unsigned char> data;
    bool isPPM();
    bool isBMP();
    void initialize();
    std::vector<unsigned char> readPPM();
    std::vector<unsigned char> readBMP();
    std::vector<unsigned char> createPPMHeader();
};

#endif //IMAGE_H
