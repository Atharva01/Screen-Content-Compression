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

// Function to calculate horizontal and vertical gradients for a color channel
void calculateGradients(const std::vector<std::vector<int>> &channel, std::vector<int> &ghor, std::vector<int> &gver, const std::string &channel_name) {
    int height = channel.size();
    int width = channel[0].size();
    
    // Calculate horizontal gradients
    for (int i = 0; i < height; ++i) {
        for (int j = 1; j < width; ++j) {
            ghor.push_back(std::abs(channel[i][j] - channel[i][j - 1]));
        }
    }

    // Calculate vertical gradients
    for (int i = 1; i < height; ++i) {
        for (int j = 0; j < width; ++j) {
            gver.push_back(std::abs(channel[i][j] - channel[i - 1][j]));
        }
    }
    
    std::cout << "Gradients calculated for " << channel_name << " channel. Horizontal gradient sample: " << ghor[0] << ", Vertical gradient sample: " << gver[0] << std::endl;
}

// Function to build a histogram from gradient values
std::vector<int> buildHistogram(const std::vector<int> &gradients, const std::string &channel_name, int bin_size = 1, int max_val = 255) {
    std::vector<int> histogram(bin_size, 0);
    int bin_width = max_val / bin_size;

    for (int val : gradients) {
        if (val > 0) {
            int bin_index = std::min(val / bin_width, bin_size - 1);
            histogram[bin_index]++;
        }
    }

    std::cout << "Histogram built for " << channel_name << " channel. Sample bin value: " << histogram[0] << std::endl;
    return histogram;
}

// Function to fit a regression line to the histogram data
double fitRegressionLine(const std::vector<int> &histogram, int &gmax, const std::string &channel_name) {
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

    // Find the gmax (maximum gradient)
    gmax = bin_centers[bin_size - 1];

    std::cout << "Regression line fitted for " << channel_name << " channel. g0: " << -intercept / slope << ", gmax: " << gmax << std::endl;

    // Return the g0 (intersection with the x-axis)
    return -intercept / slope;
}

// Function to calculate syntheticness score
double calculateSyntheticness(double g0, int gmax, const std::string &channel_name) {
    double sy_prime = ((g0 - gmax) / 2) * (200 / (gmax / 2));
    if (sy_prime >= 200) sy_prime = 200;
    if (sy_prime <= 0) sy_prime = 0;

    std::cout << "Syntheticness calculated for " << channel_name << " channel: " << sy_prime << std::endl;
    return sy_prime;
}

// Main function to calculate syntheticness for an RGB image
void calculateImageSyntheticness(const std::string &filename) {
    std::vector<std::vector<int>> red, green, blue;
    int width, height;

    // Step 1: Read PPM file
    readPPM(filename, red, green, blue, width, height);
    
    // Step 2: Calculate gradients and build histograms
    std::vector<int> ghor_red, gver_red, ghor_green, gver_green, ghor_blue, gver_blue;
    calculateGradients(red, ghor_red, gver_red, "Red");
    calculateGradients(green, ghor_green, gver_green, "Green");
    calculateGradients(blue, ghor_blue, gver_blue, "Blue");

    // Step 3: Build histograms for each channel
    auto hist_red = buildHistogram(ghor_red, "Red");
    auto hist_green = buildHistogram(ghor_green, "Green");
    auto hist_blue = buildHistogram(ghor_blue, "Blue");

    // Step 4: Fit regression line and calculate syntheticness for each channel
    int gmax_red, gmax_green, gmax_blue;
    double g0_red = fitRegressionLine(hist_red, gmax_red, "Red");
    double g0_green = fitRegressionLine(hist_green, gmax_green, "Green");
    double g0_blue = fitRegressionLine(hist_blue, gmax_blue, "Blue");

    double syntheticness_red = calculateSyntheticness(g0_red, gmax_red, "Red");
    double syntheticness_green = calculateSyntheticness(g0_green, gmax_green, "Green");
    double syntheticness_blue = calculateSyntheticness(g0_blue, gmax_blue, "Blue");

    // Output the results
    std::cout << "Syntheticness for Red channel: " << syntheticness_red << std::endl;
    std::cout << "Syntheticness for Green channel: " << syntheticness_green << std::endl;
    std::cout << "Syntheticness for Blue channel: " << syntheticness_blue << std::endl;
}

int main(int argc, char* argv[]) {
    // Check if the input PPM file path is provided as a command-line argument
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <ppm_file_path>" << std::endl;
        return 1;
    }

    // Retrieve the PPM file path from the command-line argument
    std::string filename = argv[1];

    // Call the function to calculate the syntheticness
    calculateImageSyntheticness(filename);

    return 0;
}
