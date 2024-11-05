#include <iostream>
#include <fstream>
#include <vector>
#include <cmath>
#include <numeric>
#include <algorithm>

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
void applySobel(const std::vector<std::vector<int>> &channel, std::vector<int> &gradient, int width, int height) {
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
    gradient.resize(height * width, 0);

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
            gradient[i * width + j] = std::sqrt(sumX * sumX + sumY * sumY);
        }
    }
}

// Function to build a histogram from gradient values
std::vector<int> buildHistogram(const std::vector<int> &gradients, int bin_size = 50, int max_val = 255) {
    std::vector<int> histogram(bin_size, 0);
    int bin_width = max_val / bin_size;

    for (int val : gradients) {
        if (val > 0) {
            int bin_index = std::min(val / bin_width, bin_size - 1);
            histogram[bin_index]++;
        }
    }

    // Verbose output for histogram
    std::cout << "Histogram: ";
    for (auto h : histogram) {
        std::cout << h << " ";
    }
    std::cout << std::endl;

    return histogram;
}

// Function to fit a regression line to the histogram data
double fitRegressionLine(const std::vector<int> &histogram, int &gmax) {
    int bin_size = histogram.size();
    std::vector<int> bin_centers(bin_size);
    std::iota(bin_centers.begin(), bin_centers.end(), 0);

    double sum_x = 0, sum_y = 0, sum_xy = 0, sum_x2 = 0;
    int total = std::accumulate(histogram.begin(), histogram.end(), 0);
    
    for (int i = 0; i < bin_size; ++i) {
        sum_x += bin_centers[i];
        sum_y += histogram[i];
        sum_xy += bin_centers[i] * histogram[i];
        sum_x2 += bin_centers[i] * bin_centers[i];
    }
    
    double slope = (total * sum_xy - sum_x * sum_y) / (total * sum_x2 - sum_x * sum_x);
    double intercept = (sum_y - slope * sum_x) / total;

    // Verbose output for regression values
    std::cout << "Slope: " << slope << ", Intercept: " << intercept << std::endl;

    // Find the gmax (maximum gradient)
    gmax = bin_centers[bin_size - 1];

    // Return the g0 (intersection with the x-axis)
    return -intercept / slope;
}

// Function to calculate syntheticness score based on g0 and gmax
double calculateSyntheticness(double g0, int gmax) {
    double sy_prime = ((g0 - gmax) / 2) * (200 / (gmax / 2));
    if (sy_prime >= 200) return 200;
    if (sy_prime <= 0) return 0;
    return sy_prime;
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
    std::vector<int> gradient_red, gradient_green, gradient_blue;
    applySobel(red, gradient_red, width, height);
    applySobel(green, gradient_green, width, height);
    applySobel(blue, gradient_blue, width, height);

    // Print a sample of gradient values
    std::cout << "Sample Gradient for Red Channel: " << gradient_red[0] << std::endl;
    std::cout << "Sample Gradient for Green Channel: " << gradient_green[0] << std::endl;
    std::cout << "Sample Gradient for Blue Channel: " << gradient_blue[0] << std::endl;

    // Step 3: Build histograms from the gradients
    auto hist_red = buildHistogram(gradient_red);
    auto hist_green = buildHistogram(gradient_green);
    auto hist_blue = buildHistogram(gradient_blue);

    // Step 4: Fit regression line and calculate syntheticness for each channel
    int gmax_red, gmax_green, gmax_blue;
    double g0_red = fitRegressionLine(hist_red, gmax_red);
    double g0_green = fitRegressionLine(hist_green, gmax_green);
    double g0_blue = fitRegressionLine(hist_blue, gmax_blue);

    double syntheticness_red = calculateSyntheticness(g0_red, gmax_red);
    double syntheticness_green = calculateSyntheticness(g0_green, gmax_green);
    double syntheticness_blue = calculateSyntheticness(g0_blue, gmax_blue);

    // Output the results
    std::cout << "Syntheticness for Red channel: " << syntheticness_red << std::endl;
    std::cout << "Syntheticness for Green channel: " << syntheticness_green << std::endl;
    std::cout << "Syntheticness for Blue channel: " << syntheticness_blue << std::endl;

    return 0;
}
