#include "image.h"

int main(int argc, char* argv[]) {
    if(argc < 2) {
        Image::help();
        return 0;
    }
    std::string flag = argv[1];
    if(flag == "-h") {
        if(argc > 2) {
            std::cerr << "Error: Too many arguments\n";
            return 1;
        }
        Image::help();
        return 0;
    }
    if(argc < 3) {
        std::cerr << "Error: No filepath given!\nSee -h for help!\n";
        return 1;
    }
    std::string filepath = argv[2];
    Image img(filepath);

    if(!img.isValidFormat())
        return 1;

    if(flag == "-i" | flag == "-d")
        if(argc > 3) {
            std::cerr << "Error: Too many arguments!\nSee -h for help!\n";
            return 1;
        }

    if(flag == "-i") {
        img.info();
        return 0;
    }

    if(flag == "-d") {
        img.decode();
        return 0;
    }

    if(flag == "-e" | flag == "-c") {

        if (argc != 4) {
            if (argc < 4)
                std::cerr << "Error: Message was not provided!\nSee -h for help!\n";
            if (argc > 4)
                std::cerr << "Error: Too many arguments provided!\nSee -h for help!\n";
            return 1;
        }
        else {
            const std::string message = argv[3];
            if(img.check(message)) {
                if(flag == "-e") {
                    img.encode(message);
                    std::cout << "message: \"" << message << "\" was encoded successfully!\n";
                    return 0;
                }
                std::cout << "Message: \"" << message << "\" can be encoded!\n";
                return 0;
            }
            return 1;
        }
    }

    std::cerr << "Error: Unknown flag arguments!\n";
    std::cerr << "Try the -h flag for help!\n";
    return 1;
}
