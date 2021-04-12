#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include "model.h"
#include "tgaimage.h"
Model::Model(const char *filename) : verts_(), faces_(), norms_(), uv_(), diffusemap_(), normalmap_(), specularmap_(), subsurfacemap_() {
    std::ifstream in;
    in.open (filename, std::ifstream::in);
    if (in.fail()) return;
    std::string line;
    while (!in.eof()) {
        std::getline(in, line);
        std::istringstream iss(line.c_str());
        char trash;
        if (!line.compare(0, 2, "v ")) {
            iss >> trash;
            Vector3d v;
            for (int i=0; i < 3; ++i){
				iss >> v[i];
			}
            v.x = v[0];
            v.y = v[1];
            v.z = v[2];
            verts_.push_back(v);
		} else if (!line.compare(0, 3, "vt ")) {
			iss >> trash >> trash;
			Vector2d uv;
			for (int i=0; i < 2; ++i){
				iss >> uv[i];
			}
			uv_.push_back(uv);
		} else if (!line.compare(0, 3, "vn ")) {
			iss >> trash >> trash;
			Vector3d n;
			for (int i=0; i < 3; ++i){
				iss >> n[i];
			}
			norms_.push_back(n);
        } else if (!line.compare(0, 2, "f ")) {
            std::vector<Vector3i> f;
            Vector3i temp;
            iss >> trash;
            while (iss >> temp[0] >> trash >> temp[1] >> trash >> temp[2]) {
				for (int i = 0; i < 3; ++i){
					--temp[i]; // in wavefront obj all indices start at 1, not zero
				}
				f.push_back(temp);
			}
        faces_.push_back(f);
        }
    }
	load_texture(filename, "_diffuse.tga", diffusemap_);
	load_texture(filename, "_nm.tga", normalmap_);
	load_texture(filename, "_spec.tga", specularmap_);
	load_texture(filename, "_SSS.tga", subsurfacemap_);
}

Model::~Model() {
}

int Model::nverts() {
    return static_cast<int>(verts_.size());
}

int Model::nfaces() {
    return static_cast<int>(faces_.size());
}

std::vector<int> Model::face(int index) {
	std::vector<int> face;
	for (int i=0; i < static_cast<int>(faces_[index].size()); ++i){
		face.push_back(faces_[index][i][0]);
	}
    return face;
}

Vector3d Model::vert(int index) {
    return verts_[index];
}

Vector3d Model::vert(int iface, int nthvert) {
    return verts_[faces_[iface][nthvert][0]];
}

void Model::load_texture(std::string filename, const char *suffix, TGAImage &image){
	std::string texturefile(filename);
	size_t dot = texturefile.find_last_of(".");
	if (dot != std::string::npos) {
		texturefile = texturefile.substr(0, dot) + std::string(suffix);
		image.read_tga_file(texturefile.c_str());
		image.flip_vertically();
	}
}

TGAColor Model::diffuse(Vector2d uvf){
	return diffusemap_.get(uvf[0] * diffusemap_.get_width() , uvf.y * diffusemap_.get_height());
}

Vector3d Model::normalmap(Vector2d uvf){
    TGAColor color = normalmap_.get(uvf[0] * normalmap_.get_width(), uvf[1] * normalmap_.get_height());
	return Vector3d((128.0-color.bgra[2])/128.0, (128.0-color.bgra[1])/128.0, (color.bgra[0] -128.0)/-128.0);
}

double Model::specularmap(Vector2d uvf){
	return specularmap_.get(uvf[0] * specularmap_.get_width(), uvf[1] * specularmap_.get_height()).bgra[0] / 1.0;
}

TGAColor Model::subsurfacemap(Vector2d uvf){
	return subsurfacemap_.get(uvf[0] * subsurfacemap_.get_width(), uvf[1] * subsurfacemap_.get_height());
}

Vector2d Model::uv(int iface, int nvert){
	int index = faces_[iface][nvert][1];
	return Vector2d(uv_[index].x, uv_[index].y);
}

Vector3d Model::norm(int iface, int nvert) {
	int index = faces_[iface][nvert][2];
	return norms_[index].normalize();
}
