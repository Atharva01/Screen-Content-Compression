#include <iostream>
#include <fstream>
#include <vector>

// Function to read and display a PPM file
void readAndDisplayPPM(const std::string &filename) {
    std::ifstream file(filename, std::ios::binary);
    if (!file) {
        throw std::runtime_error("Failed to open PPM file.");
    }

    std::string format;
    file >> format;

    if (format != "P6") {
        throw std::runtime_error("Unsupported PPM format.");
    }

    int width, height, max_value;
    file >> width >> height >> max_value;
    file.ignore();  // Skip the newline after the header

    std::cout << "PPM Image Info:" << std::endl;
    std::cout << "Width: " << width << std::endl;
    std::cout << "Height: " << height << std::endl;
    std::cout << "Max color value: " << max_value << std::endl;

    std::vector<unsigned char> imageData(width * height * 3);
    file.read(reinterpret_cast<char *>(imageData.data()), imageData.size());

    std::cout << "Pixel Data (R G B):" << std::endl;
    for (int i = 0; i < height; ++i) {
        for (int j = 0; j < width; ++j) {
            int idx = (i * width + j) * 3;
            std::cout << "(" 
                      << static_cast<int>(imageData[idx]) << " " 
                      << static_cast<int>(imageData[idx + 1]) << " " 
                      << static_cast<int>(imageData[idx + 2]) << ") ";
        }
        std::cout << std::endl;
    }
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <ppm_file_path>" << std::endl;
        return 1;
    }

    std::string filename = argv[1];

    try {
        readAndDisplayPPM(filename);
    } catch (const std::exception &e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }

    return 0;
}
