#include <vector>
#include <cmath>
#include <iostream>
#include <algorithm>
#include "tgaimage.h"
#include "model.h"
#include "geometry.h"
#include "our_gl.h"
#include "shaders.h"

const int MAX_DEPTH = 255;
const double GAMMA = 1.0;
const int SCREEN_X = 1000;
const int SCREEN_Y = 1000;

Matrix g_VIEWPORT;
Matrix g_PROJECTION;
Matrix g_MODELVIEW;


double gamma_correction(double intensity) {
	// Corrects for gamma using a power law,
	// and global variable GAMMA.
	// Prefactor is set to 1.0
	const double A = 1.0; 
	return A * intensity * pow(intensity, GAMMA);
}


void draw_line(Vector3i point0, Vector3i point1, TGAImage &image, TGAColor color) {
    // Draws a line using Bresenham's Algorithm.
    // Input:
    // point0 (x0, y0) as integers
    // point1 (x1, y1) as integers
    // An image to modify
    // A color to draw in the form (RGBA)

    bool steep = false;
    int delta_x = std::abs(point0.x - point1.x);
    int delta_y = std::abs(point0.y - point1.y);
    if (delta_x < delta_y) {
        std::swap(point0.x, point0.y);
        std::swap(point1.x, point1.y);
        std::swap(delta_x, delta_y);
        steep = true;
    }
    if (point0.x > point1.x) {
        // Make sure we always go left to right
        std::swap(point0, point1);
    }

    if (point0.y == point1.y) {
        // The line is directly across the screen
        int y = point0.y;
        for (int x = point0.x; x <= point1.x; ++x){
            if (steep) {
                image.set(y, x, color);
            } else {
                image.set(x, y, color);
            }
        }
        return;
    }
    for (int x=point0.x; x <= point1.x; ++x) {
        double t = (x - point0.x) / static_cast<double>(delta_x);
        int y = point0.y * (1.0 - t) + point1.y * t;
        if (steep) {
            image.set(y, x, color);
        } else {
            image.set(x, y, color);
        }
    }
}


Triangle::Triangle(Vector4d point_0, Vector4d point_1, Vector4d point_2, TGAImage &image) {	
	
    // Triangle constructor, which caches 
    // some operations for the barycentric coordinate calculation
    screen_coords[0] = point_0;
	screen_coords[1] = point_1;
	screen_coords[2] = point_2;
		
	vec_1 = proj<3>(screen_coords[1] - screen_coords[0]);
	vec_2 = proj<3>(screen_coords[2] - screen_coords[0]);
	vec_3 = proj<3>(screen_coords[2] - screen_coords[1]);
	determinant = (vec_1.x * vec_2.y) - (vec_1.y * vec_2.x);
		
	std::vector<int> x_coords = {screen_coords[0][0], screen_coords[1][0], screen_coords[2][0]};
	std::vector<int> y_coords  = {screen_coords[0][1], screen_coords[1][1], screen_coords[2][1]};
	std::sort(x_coords.begin(), x_coords.end());
	std::sort(y_coords.begin(), y_coords.end());
	
	bounding_min = Vector2d(static_cast<int>(std::max(x_coords.front(), 0)), static_cast<int>(std::max(y_coords.front(), 0)));
	bounding_max = Vector2d(static_cast<int>(std::min(x_coords.back(), image.get_height()-1)), static_cast<int>(std::min(y_coords.back(), image.get_height()-1)));
}

void Triangle::draw_outline(TGAImage &image, TGAColor color) {
    // Outlines the triangle using Bresenham's line 
    // algorithm. 
	draw_line(proj<3>(screen_coords[0]), proj<3>(screen_coords[1]), image, color);
	draw_line(proj<3>(screen_coords[1]), proj<3>(screen_coords[2]), image, color);
	draw_line(proj<3>(screen_coords[2]), proj<3>(screen_coords[0]), image, color);
}

void Triangle::draw_bounding_box(TGAImage &image, TGAColor color) {
    // Draws a bounding box around a triangle,
    // from (min_x, min_y) to (max_x, max_y)
	draw_line(Vector3i(bounding_min.x, bounding_min.y, 1), Vector3i(bounding_min.x, bounding_max.y, 1), image, color);
	draw_line(Vector3i(bounding_min.x, bounding_min.y, 1), Vector3i(bounding_max.x, bounding_min.y, 1), image, color);
	draw_line(Vector3i(bounding_max.x, bounding_max.y, 1), Vector3i(bounding_min.x, bounding_max.y, 1), image, color);
	draw_line(Vector3i(bounding_max.x, bounding_max.y, 1), Vector3i(bounding_max.x, bounding_min.y, 1), image, color);
}

template <class T>
Vector3d Triangle::barycentric(T point_in) {
	// Unsubtly computes the barycentric coordinates of a point
	// within a given triangle using the equation on
	// wikipedia. 
	// Takes in a 3D point (x, y, z) in Cartesian space,
	// and a triangle made up of three points (x0, y0, z0) ... (x2, y2, z2)
	// Returns a 3D vector (lambda_1, lambda_2, lambda_3)

	if (determinant == 0) {
		// If the matrix is non-invertible, then bail out.
		// Return a barycentric coordinate with negative parts
		// to indicate that the point can be discarded.
		return Vector3d(-1, -1, -1);		
	}
    // The point we receive is in 4D homogenous coordinates
    // so project down to 3D.
	auto point = proj<3>(point_in);
	Vector3d delta(point.x - screen_coords[0][0],point.y - screen_coords[0][1],point.z - screen_coords[0][2]);
	double lambda_1 = (delta.x * vec_2.y - delta.y * vec_2.x) / determinant;
	double lambda_2 = (-delta.x * vec_1.y + delta.y * vec_1.x) / determinant;
	return Vector3d(1 - lambda_1 - lambda_2, lambda_1, lambda_2);
}


Vector3d Triangle::perspective_correct(Vector3d point) {
    // Perspective corrects a set of barycentric
    // coordinates
    auto working = Vector3d(point[0] / screen_coords[0][3],
                            point[1] / screen_coords[1][3],
                            point[2] / screen_coords[2][3]);
    return working / (working.x + working.y + working.z);
}


void Triangle::draw_texture(TGAImage &zbuffer, TGAImage &image, IShader& shader) {
	// Draws a triangle described by the three points t0, t1 and t2
	// Then fills it.
	// Proceeds by sorting t0, t1 and t2 into descenting y-order
    for (int pix_x = bounding_min.x; pix_x <= bounding_max.x; ++pix_x){
        for (int pix_y = bounding_min.y; pix_y <= bounding_max.y; ++pix_y){
            auto point = Vector3i(pix_x, pix_y, 0);
            //auto bc_screen = perspective_correct(barycentric(point));
            auto bc_screen = barycentric(point);
			if (bc_screen.x < 0 || bc_screen.y < 0 || bc_screen.z < 0) {
				continue;
			}
            
	        double depth = screen_coords[0][2] * bc_screen.x 
			             + screen_coords[1][2] * bc_screen.y 
			             + screen_coords[2][2] * bc_screen.z;            
            if (zbuffer.get(point.x, point.y).bgra[0] > depth) {
                continue;  
            }
            TGAColor color;
            bool discard = shader.fragment(bc_screen, color);
			if (!discard) {		
                zbuffer.set(point.x, point.y, TGAColor(depth));
				image.set(pix_x, pix_y, color);
			}
        }
    }
}


void projection(double coeff) {
    // Sets the global projection matrix
    g_PROJECTION = Matrix::identity();
    g_PROJECTION[3][2] = coeff;
}


void viewport(int x, int y, int width, int height) {
    // Sets the global viewport matrix
	g_VIEWPORT = Matrix::identity();
	g_VIEWPORT[0][3] = x + width / 2.0;
	g_VIEWPORT[1][3] = y + height / 2.0;
	g_VIEWPORT[2][3] = MAX_DEPTH / 2.0;
	
	g_VIEWPORT[0][0] = width / 2.0;
	g_VIEWPORT[1][1] = height / 2.0;
	g_VIEWPORT[2][2] = MAX_DEPTH / 2.0;
}

void lookat(Vector3d cam_pos, Vector3d origin, Vector3d upward_vector) {
    // Sets the global model view matrix
    Vector3d z = (cam_pos - origin).normalize();
    Vector3d x = cross(upward_vector,z).normalize();
    Vector3d y = cross(z, x).normalize();
    
    g_MODELVIEW = Matrix::identity();
    for (int i=0; i<3; i++) {
        g_MODELVIEW [0][i] = x[i];
        g_MODELVIEW [1][i] = y[i];
        g_MODELVIEW [2][i] = z[i];
        g_MODELVIEW [i][3] = -origin[i];
    }   
}
