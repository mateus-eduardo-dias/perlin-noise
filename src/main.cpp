#include <SFML/Graphics.hpp>
#include <iostream>
#include <cmath>
#include <random>

std::uniform_int_distribution<int> dist(0,255);
std::vector<int> p(1024);
int SEED = 2025;
std::mt19937 engine(SEED);
int GRID_SIZE = 400;
int OCTAVES = 12;
double CONTRAST = 1.2;

void displayColor(std::vector<std::uint8_t>* pixels, int index, double color) {
    /*
        Color is a number in a range of 0-255
        (*pixels)[index]     = R
        (*pixels)[index + 1] = G
        (*pixels)[index + 2] = B
        (*pixels)[index + 3] = A
    */
    if (color > 191) {
        (*pixels)[index] = 0;
        (*pixels)[index + 1] = 0;
        (*pixels)[index + 2] = 0;
        (*pixels)[index + 3] = 255;
    } else if (color > 127) {
        (*pixels)[index] = 255;
        (*pixels)[index + 1] = 255;
        (*pixels)[index + 2] = 0;
        (*pixels)[index + 3] = 255;
    } else if (color > 64) {
        (*pixels)[index] = 200;
        (*pixels)[index + 1] = 200;
        (*pixels)[index + 2] = 0;
        (*pixels)[index + 3] = 255;
    } else {
        (*pixels)[index] = 0;
        (*pixels)[index + 1] = 0;
        (*pixels)[index + 2] = 255;
        (*pixels)[index + 3] = 127;
    }
}

double lerp(double r, double x, double y) {
    return (1-r)*x + r*y; // (1 - ratio)*n1 + ratio*n2
}

int perm(int x) {
    return p[x & 1023];
}

double gradientVector(int hash, double x, double y) {
    switch (hash & 7) {
        case 0: return x + y; // (1, 1)
        case 1: return -x + y; // (-1, 1)
        case 2: return x + -y; // (1, -1)
        case 3: return -x + -y; // (-1, -1)
        case 4: return x; // (1, 0)
        case 5: return -x; // (-1, 0)
        case 6: return y; // (0, 1)
        case 7: return -y; // (0, -1)
    }
}

double fade(double t) {
    return 6 * (pow(t, 5)) - 15 * (pow(t, 4)) + 10 * (pow(t,3)); // 6t^5 - 15t^4 + 10t^3
}

double perlin(double x, double y) {
    // 10 last significant bits (limited by permutation table, can be changed)
    int xs = static_cast<int>(floor(x)) & 1023;
    int ys = static_cast<int>(floor(y)) & 1023;

    x -= floor(x); // Decimal part of x
    y -= floor(y); // Decimal part of y

    // Cubic curves
    double u = fade(x);
    double v = fade(y);

    // Permutation (hash)
    int A = perm(xs) + ys;
    int AA = perm(A);
    int AB = perm(A + 1);
    int B = perm(xs + 1) + ys;
    int BA = perm(B);
    int BB = perm(B + 1);

    // Dot product
    double res_AA = gradientVector(perm(AA), x, y);
    double res_AB = gradientVector(perm(AB), x, y - 1);
    double res_BA = gradientVector(perm(BA), x - 1, y);
    double res_BB = gradientVector(perm(BB), x - 1, y - 1);

    // Linear Interpolation
    double x1 = lerp(u, res_AA, res_BA);
    double x2 = lerp(u, res_AB, res_BB);

    return lerp(v, x1, x2);
}

int main()
{
    const unsigned int WIDTH = 1920; // Screen Width
    const unsigned int HEIGHT = 1080; // Screen Height

    for (int i = 0; i < 1024; i++) { // Create a Vector with 256 elements random elements with range of 0-255 (permutation table)
        p[i] = dist(engine);
    }

    std::cout << "Application started" << std::endl;
    sf::RenderWindow window(sf::VideoMode({WIDTH, HEIGHT}), "Perlin Noise"); // Windows
    std::vector<std::uint8_t> pixels(WIDTH * HEIGHT * 4); // Perlin Noise representation (image, contains all bytes as RGBA)
    window.setFramerateLimit(60); // 60 FPS, since it's a static image doesn't actually matter
    
    for (int x = 0; x < WIDTH; x++) {
        for (int y = 0; y < HEIGHT; y++) {
            int index = (y * WIDTH + x) * 4; // Index for the image pixel
            
            double val = 0; // Perlin Noise value initialization
            
            double freq = 1; // Frequency iniatialization
            double amp = 1; // Amplitude iniatialization
            for (int i = 0; i < OCTAVES; i++) {
                val += perlin((double) x * freq / GRID_SIZE, (double) y * freq / GRID_SIZE) * amp; // Perlin noise calculation
                freq *= 2; // Increase frequency for new octave
                amp /= 2; // Decrease amplitude for new octave
            }
            val *= CONTRAST; // Add contrast

            // Change value if too high or too low
            if (val > 1.0) val = 1.0;
            else if (val < -1.0) val = -1.0;

            int color = (int) ((val + 1.0) * 0.5 * 255.0); // Perlin Noise in a 0-255 scale
            displayColor(&pixels, index, color); // Display pixel based on 0-255 scale
        }
    }


    sf::Image image({WIDTH, HEIGHT}, pixels.data()); // Load image (Perlin Noise)
    sf::Texture texture(image); // Load texture (image)
    sf::Sprite sprite(texture); // Load sprite (texture)
    
    while (window.isOpen()) // Run program forever
    {
        while (const std::optional event = window.pollEvent())
        {
            if (event->is<sf::Event::Closed>())
            {
                window.close();
            }
        }

        window.clear(); // Clear Perlin Noise
        window.draw(sprite); // Draw Perlin Noise
        window.display(); // Display Perlin Noise
    }
    
    return 0;
}