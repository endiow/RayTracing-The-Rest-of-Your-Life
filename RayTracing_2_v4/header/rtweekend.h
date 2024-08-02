#pragma once
#include <cmath>
#include <iostream>
#include <limits>
#include <memory>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui.hpp>
#include <random>


// C++ Std Usings

using namespace cv;
using std::make_shared;
using std::shared_ptr;

// Constants

const double infinity = std::numeric_limits<double>::infinity();
const double pi = 3.1415926535897932385;

// Utility Functions

// Utility Functions

inline double degrees_to_radians(double degrees) 
{
    return degrees * pi / 180.0;
}

inline double random_double() 
{
    static std::uniform_real_distribution<double> distribution(0.0, 1.0);
    static std::mt19937 generator;
    return distribution(generator);
}

inline double random_double(double min, double max) 
{
    // Returns a random real in [min,max).
    return min + (max - min) * random_double();
}

inline int random_int(int min, int max) 
{
    // Returns a random integer in [min,max].
    return int(random_double(min, max + 1));
}

// Common Headers

#include "color.h"
#include "ray.h"
#include "vec3.h"
#include "interval.h"