#ifndef __MODEL_H__
#define __MODEL_H__

#include <vector>
#include "geometry.h"
#include "tgaimage.h"
class Model {
private:
	std::vector<Vector3d> verts_;
	// The vector is vertex / uv / normal 
	std::vector<std::vector<Vector3i> > faces_;
	std::vector<Vector3d> norms_;
	std::vector<Vector2d> uv_;
	TGAImage diffusemap_;
	TGAImage normalmap_;
	TGAImage specularmap_;
	TGAImage subsurfacemap_;
	void load_texture(std::string filename, const char *suffix, TGAImage &image);
public:
	Model(const char *filename);
	~Model();
	int nverts();
	int nfaces();
	Vector3d norm(int iface, int nvert);
	Vector3d vert(int index);
    Vector3d vert(int iface, int nthvert);

	Vector2d uv(int iface, int nvert);
	TGAColor diffuse(Vector2d uvf);
	Vector3d normalmap(Vector2d uvf);
    double specularmap(Vector2d uvf);
    TGAColor subsurfacemap(Vector2d uvf);
	std::vector<int> face(int index);
};

#endif //__MODEL_H__
