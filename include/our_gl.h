#pragma once

#include "tgaimage.h"
#include "model.h"
#include "geometry.h"
#include "shaders.h"
extern const int MAX_DEPTH;
extern const double GAMMA;
extern const double AMBIENT;
extern const int SCREEN_X;
extern const int SCREEN_Y;

extern Matrix ModelView;
extern Matrix Viewport;
extern Matrix Projection;

double gamma_correction(double intensity);

void draw_line(Vector3i, Vector3i, TGAImage&, TGAColor);


class Triangle{
    private:
		Vector3d vec_1;
		Vector3d vec_2;
		Vector3d vec_3;
		Vector2i bounding_min;
		Vector2i bounding_max;
		double determinant;
		template <class T>
		Vector3d barycentric(T);
		Vector3d perspective_correct(Vector3d);
		Vector4d screen_coords[3];
    public:
		Triangle(Vector4d, Vector4d, Vector4d, TGAImage &image);
		void draw_texture(TGAImage&, TGAImage&, IShader&);
		void draw_outline(TGAImage&, TGAColor);
		void draw_bounding_box(TGAImage&, TGAColor);
};

void projection(double coeff);
void viewport(int x, int y, int width, int height);
void lookat(Vector3d cam_pos, Vector3d origin, Vector3d upward_vector);
