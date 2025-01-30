#include "image.h"

Image::Image(const std::string& filepath) {
    filename = filepath;
    magicNum = {0};
    width = {0};
    height = {0};
    filesize = {0};
    ppmMaxVal = {0};
    data = {0};
    header = {0};
    initialize();
}


void Image::decode() {
    std::bitset<32> lengthBits;
    int bitIndex = 0;
    // Extract message length from LSBs of first 32 pixels
    for (int i = 0; i < 32; ++i) {
        for (int j = 0; j < 3; ++j) {
            // Extract LSB from each color channel
            lengthBits[bitIndex] = data[i * 3 + j] & 0x01;
            ++bitIndex;
        }
    }

    // Reconstruct message length
    int messageLength = static_cast<int>(lengthBits.to_ulong());

    // Extract message bits from image data
    std::string decodedMessage;
    std::bitset<8> charBits;
    for (int i = 0; i < messageLength; ++i) {
        charBits.reset();
        for (int j = 0; j < 8; ++j) {
            for (int k = 0; k < 3; ++k) {
                // Extract LSB from each color channel
                charBits[j] = (charBits[j] << 1) | (data[(i * 8 + j + 32) * 3 + k] & 0x01);
            }
        }
        // Convert bitset to char and append to decoded message
        decodedMessage += static_cast<char>(charBits.to_ulong());
    }
    std::cout << decodedMessage;
}

void Image::initialize() {
    if(Image::isPPM())
        this->readPPM();
    else if(Image::isBMP())
        this->readBMP();
}

void Image::help() {
    std::cout << "Image steganography program by Falo\n";
    std::cout << "Valid file formats: .ppm .bmp\n";
    std::cout << "-h\t\t\t\t\tDisplay this help message\n";
    std::cout << "-i\t<filepath>\t\t\tDisplay information about file\n";
    std::cout << "-d\t<filepath>\t\t\tDecode message from file\n";
    std::cout << "-c\t<filepath>\t<message>\tCheck if message can be encoded\n";
    std::cout << "-e\t<filepath>\t<message>\tEncode message into file\n";
    std::cout << "Remember to put the message in \"quotation marks\" for proper handling\n";
}

bool Image::isValidFormat() {
    if(!Image::isBMP() && !Image::isPPM())
        std::cerr << "Invalid file format: " << filename << "!\n";
    return Image::isBMP() | Image::isPPM();
}

bool Image::isPPM() {
    std::ifstream file(filename, std::ios::binary);
    // here we read 2 first bytes to compare the magic num to see if its ppm format P6 or 0x5036 hex
    std::vector<unsigned char> headertmp(2);
    file.read(reinterpret_cast<char*>(headertmp.data()), headertmp.size());
    file.close();
    return headertmp[0] == 0x50 && headertmp[1] == 0x36;
}

bool Image::isBMP() {
    std::ifstream file(filename, std::ios::binary);
    // here we read 2 first bytes to compare the magic num to see if its bmp format BM or 0x424d hex
    std::vector<unsigned char> headertmp(2);
    file.read(reinterpret_cast<char*>(headertmp.data()), headertmp.size());
    file.close();
    return headertmp[0] == 0x42 && headertmp[1] == 0x4d;
}

std::vector<unsigned char> Image::readBMP() {
    std::ifstream file(filename, std::ios::binary);
    // here we read the whole bmp header file to extract information from

    std::vector<unsigned char> tmpHead(54);
    file.read(reinterpret_cast<char*>(tmpHead.data()), tmpHead.size());
    magicNum = {*(char*)&tmpHead[0], *(char*)&tmpHead[1]};
    width = *(int*)&tmpHead[18];
    height = *(int*)&tmpHead[22];
    int imageSize = width * height * 3;
    std::vector<unsigned char> imageData(imageSize);
    file.read(reinterpret_cast<char*>(imageData.data()), imageSize);
    this->data = imageData;
    this->header = tmpHead;
    filesize = std::filesystem::file_size(filename);
    file.close();
    return imageData;
}

std::vector<unsigned char> Image::readPPM() {
    std::ifstream file(filename, std::ios::binary);
    file >> magicNum;
    file >> width;
    file >> height;
    file >> ppmMaxVal;
    // without +1, eof gets cut off, making the file unable to open
    int imageSize = (width * height * 3)+1;
    std::vector<unsigned char> imageData(imageSize);
    file.read(reinterpret_cast<char*>(imageData.data()), imageSize);
    this->data = imageData;
    this->header = Image::createPPMHeader();
    filesize = std::filesystem::file_size(filename);
    file.close();
    return imageData;
}

void Image::info() {
    std::cout << "filename:\t" << filename << '\n';
    std::cout << "filesize:\t" << filesize << " bytes\n";
    std::cout << "width:\t\t" << width << " pixels\n";
    std::cout << "height:\t\t" << height << " pixels\n";
    std::cout << "magic num:\t" << magicNum << '\n';
}

std::vector<unsigned char> Image::createPPMHeader() {
    std::vector<unsigned char> tmp;
    for(auto c: magicNum) {
        tmp.push_back((unsigned char)c);
    }
    tmp.push_back(0x0a);
    for(auto c : std::to_string(width)) {
        tmp.push_back((unsigned char)c);
    }
    tmp.push_back(0x0a);
    for(auto c : std::to_string(height)) {
        tmp.push_back((unsigned char)c);
    }
    tmp.push_back(0x0a);
    for(auto c : std::to_string(ppmMaxVal)) {
        tmp.push_back((unsigned char)c);
    }
    return tmp;
}

// Checks if there is sufficient data to encode the message
bool Image::check(const std::string& message) const {
    // Set to 4, as the message length gets encoded in the first 4 bytes;
    int bytesNeeded = 4;
    // message length gets multiplied by 8, as each bit of a character(8), is encoded into the LSB of a given byte;
    bytesNeeded += (message.length()*8);
    // max length is the pixel amount times the color channel amount;
    int maxLength = (width * height * 3);
    if(maxLength < bytesNeeded)
        std::cerr << "Error: Message to long: " << bytesNeeded << " is bigger than limit: " << maxLength <<'\n';
    return maxLength > bytesNeeded;
}

// Function that encodes the message and its size into the least significant bits of each pixels color channel values
void Image::encode(const std::string& message) {
    int messageLength = message.size();
    std::bitset<32> lengthBits(messageLength); // 32 bits for message length
    int bitIndex = 0;
    // Encode message length into LSB of first 32 pixels
    for (int i = 0; i < 32; ++i) {
        for (int j = 0; j < 3; ++j) {
            // Modify the least significant bit of each color channel
            data[i * 3 + j] &= 0xFE; // Set LSB to 0
            data[i * 3 + j] |= lengthBits[bitIndex]; // Set LSB to message bit
            ++bitIndex;
        }
    }
    // Encode message into LSB of image data
    for (int i = 0; i < messageLength; ++i) {
        std::bitset<8> charBits(message[i]);
        for (int j = 0; j < 8; ++j) {
            for (int k = 0; k < 3; ++k) {
                // Modify the least significant bit of each color channel
                data[(i * 8 + j + 32) * 3 + k] &= 0xFE; // Set LSB to 0
                data[(i * 8 + j + 32) * 3 + k] |= charBits[j]; // Set LSB to message bit
            }
        }
    }
    this->write();
}

// Function to write encoded data into file
void Image::write() {
    std::ofstream output(filename, std::ios::binary | std::ios::out);
    output.write(reinterpret_cast<char*>(this->header.data()), this->header.size());
    output.write(reinterpret_cast<char*>(this->data.data()), this->data.size());
    output.close();
}


