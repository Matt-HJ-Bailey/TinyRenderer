#include <vector>
#include <cmath>

#include "geometry.h"
#include "shaders.h"
#include "tgaimage.h"
#include "model.h"

IShader::~IShader() {}

mat<3, 3, double> rotation_x(double theta) {
    mat<3, 3, double> mat;
    mat[0][0] = 1.0;
    mat[0][1] = 0;
    mat[0][2] = 0.0;

    mat[1][0] = 0.0;
    mat[1][1] = cos(theta);
    mat[1][2] = -sin(theta);
    
    mat[2][0] = 0.0;
    mat[2][1] = sin(theta);
    mat[2][2] = cos(theta);
    
    return mat;
}

mat<3, 3, double> rotation_y(double theta) {
    mat<3, 3, double> mat;
    mat[0][0] = cos(theta);
    mat[0][1] = 0;
    mat[0][2] = sin(theta);

    mat[1][0] = 0.0;
    mat[1][1] = 1.0;
    mat[1][2] = 0.0;
    
    mat[2][0] = -sin(theta);
    mat[2][1] = 0.0;
    mat[2][2] = cos(theta);
    
    return mat;
}

mat<3, 3, double> rotation_z(double theta) {
    mat<3, 3, double> mat;
    mat[0][0] = cos(theta);
    mat[0][1] = -sin(theta);
    mat[0][2] = 0.0;

    mat[1][0] = sin(theta);
    mat[1][1] = cos(theta);
    mat[1][2] = 0.0;
    
    mat[2][0] = 0.0;
    mat[2][1] = 0.0;
    mat[2][2] = 1.0;
    
    return mat;
}