#include <iostream>
#include <fstream>
#include <vector>
#include <cmath>

// Function to read a PPM file and store pixel values for each color channel
void readPPM(const std::string &filename, std::vector<std::vector<int>> &red, std::vector<std::vector<int>> &green, std::vector<std::vector<int>> &blue, int &width, int &height) {
    std::ifstream file(filename, std::ios::binary);
    std::string format;
    file >> format;
    
    if (format != "P6") {
        throw std::runtime_error("Unsupported PPM format.");
    }

    file >> width >> height;
    int max_value;
    file >> max_value;
    file.ignore(); // Skip the newline after max_value
    
    red.resize(height, std::vector<int>(width));
    green.resize(height, std::vector<int>(width));
    blue.resize(height, std::vector<int>(width));
    
    for (int i = 0; i < height; ++i) {
        for (int j = 0; j < width; ++j) {
            unsigned char pixel[3];
            file.read(reinterpret_cast<char *>(pixel), 3);
            red[i][j] = static_cast<int>(pixel[0]);
            green[i][j] = static_cast<int>(pixel[1]);
            blue[i][j] = static_cast<int>(pixel[2]);
        }
    }

    std::cout << "PPM file read successfully. Width: " << width << ", Height: " << height << std::endl;
}

// Function to apply Sobel operator to a single channel (Red, Green, or Blue)
void applySobel(const std::vector<std::vector<int>> &channel, std::vector<std::vector<int>> &gradient, int width, int height) {
    // Sobel kernels for x and y directions
    int Gx[3][3] = {
        {-1, 0, 1},
        {-2, 0, 2},
        {-1, 0, 1}
    };

    int Gy[3][3] = {
        {-1, -2, -1},
         {0,  0,  0},
         {1,  2,  1}
    };

    // Initialize gradient to zeros
    gradient.resize(height, std::vector<int>(width, 0));

    // Apply Sobel operator (ignoring image edges)
    for (int i = 1; i < height - 1; ++i) {
        for (int j = 1; j < width - 1; ++j) {
            int sumX = 0, sumY = 0;

            // Convolution with Sobel kernel
            for (int m = -1; m <= 1; ++m) {
                for (int n = -1; n <= 1; ++n) {
                    sumX += channel[i + m][j + n] * Gx[m + 1][n + 1];
                    sumY += channel[i + m][j + n] * Gy[m + 1][n + 1];
                }
            }

            // Calculate the magnitude of the gradient
            gradient[i][j] = std::sqrt(sumX * sumX + sumY * sumY);
        }
    }
}

// Function to print a sample of the gradients
void printGradientSample(const std::vector<std::vector<int>> &gradient, int width, int height) {
    std::cout << "Gradient sample (first 5x5 block): " << std::endl;
    for (int i = 0; i < 5 && i < height; ++i) {
        for (int j = 0; j < 5 && j < width; ++j) {
            std::cout << gradient[i][j] << "\t";
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
    std::vector<std::vector<int>> red, green, blue;
    int width, height;

    // Step 1: Read PPM file
    readPPM(filename, red, green, blue, width, height);

    // Step 2: Apply Sobel operator to each channel
    std::vector<std::vector<int>> gradient_red, gradient_green, gradient_blue;
    applySobel(red, gradient_red, width, height);
    applySobel(green, gradient_green, width, height);
    applySobel(blue, gradient_blue, width, height);

    // Step 3: Print gradient samples for verification
    std::cout << "Sobel Gradient for Red Channel:" << std::endl;
    printGradientSample(gradient_red, width, height);
    std::cout << "\nSobel Gradient for Green Channel:" << std::endl;
    printGradientSample(gradient_green, width, height);
    std::cout << "\nSobel Gradient for Blue Channel:" << std::endl;
    printGradientSample(gradient_blue, width, height);

    return 0;
}
