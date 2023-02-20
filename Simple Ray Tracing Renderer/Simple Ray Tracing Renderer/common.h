#pragma once
#include <cmath>
#include <limits>
#include <memory>
#include <cstdlib>
#include <random>
#include <thread>

// Usings

using std::shared_ptr;
using std::make_shared;
using std::sqrt;
using real = double;
using std::thread;

// Constants

const double infinity = std::numeric_limits<double>::infinity();
const double pi = 3.1415926535897932385;

// Utility Functions

inline real degrees_to_radians(real degrees) {
    return degrees * pi / 180.0;
}

inline double random_double() {
    static std::uniform_real_distribution<double> distribution(0.0, 1.0);
    static std::mt19937 generator;
    return distribution(generator);
}

inline real random_double(real min, real max) {
    // Returns a random real in [min,max).
    return min + (max - min) * random_double();
}

inline double clamp(double x, double min, double max) {
    if (x < min) return min;
    if (x > max) return max;
    return x;
}


// Common Headers

#include "ray.h"
#include "vec3.h"

// System Define
#define WIDTH 1280
#define RENDER_THREAD 16

int simple_left = 720;