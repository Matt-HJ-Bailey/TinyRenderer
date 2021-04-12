#pragma once

#include <vector>

#include "geometry.h"
#include "shaders.h"
#include "tgaimage.h"
#include "model.h"

extern Matrix g_VIEWPORT;
extern Matrix g_PROJECTION;
extern Matrix g_MODELVIEW;
extern Vector3d g_LIGHT_DIRECTION;
extern TGAImage g_SHADOWBUFFER;
extern const int MAX_DEPTH;

mat<3, 3, double> rotation_x(double theta);
mat<3, 3, double> rotation_y(double theta);
mat<3, 3, double> rotation_z(double theta);

struct IShader {
    virtual ~IShader();
    virtual Vector4d vertex(int iface, int nthvert) = 0;
    virtual bool fragment(Vector3d bar, TGAColor &color) = 0;
};

struct GouraudShader : public IShader {
    Model* model;
    Vector3d varying_intensity;
    mat<4, 3, double> varying_tri; // Triangle screen space coordinates, written by VS and read by FS
    mat<4, 4, double> uniform_M;
    mat<4, 4, double> uniform_MIT;

    virtual Vector4d vertex(int iface, int nthvert) {
        Vector4d gl_Vertex = g_VIEWPORT * g_PROJECTION * g_MODELVIEW * embed<4>(model->vert(iface, nthvert));
        varying_tri.set_col(nthvert, gl_Vertex);
        varying_intensity[nthvert] = model->norm(iface, nthvert) * g_LIGHT_DIRECTION;
        return gl_Vertex;
    }
    
    virtual bool fragment(Vector3d barycentric, TGAColor &color) {
        double intensity = varying_intensity * barycentric;
        color = TGAColor(255, 255, 255) * intensity;
        return false;
    }
};

struct TextureShader : public IShader {
    Model* model;
    Vector3d varying_intensity;
    mat<2, 3, double> varying_uv;
    mat<4, 4, double> uniform_M;
    mat<4, 4, double> uniform_MIT;
    
    virtual Vector4d vertex(int iface, int nthvert) {
        Vector4d gl_Vertex = g_VIEWPORT * g_PROJECTION * g_MODELVIEW * embed<4>(model->vert(iface, nthvert));
        Vector2d gl_uv = model->uv(iface, nthvert);
        varying_uv.set_col(nthvert, gl_uv);
        varying_intensity[nthvert] = model->norm(iface, nthvert) * g_LIGHT_DIRECTION;
        return gl_Vertex;
    }
        
    virtual bool fragment(Vector3d barycentric, TGAColor &color) {
        double intensity = varying_intensity * barycentric;
        color = model->diffuse(varying_uv * barycentric) * intensity;
        return false;
    }
};

struct PhongShader : public IShader {
    Model* model;
    Vector3d varying_intensity;
    mat<2, 3, double> varying_uv;
    mat<4, 4, double> uniform_M;
    mat<4, 4, double> uniform_MIT;
    virtual Vector4d vertex(int iface, int nthvert) {
        Vector4d gl_Vertex = g_VIEWPORT * g_PROJECTION * g_MODELVIEW * embed<4>(model->vert(iface, nthvert));
        Vector2d gl_uv = model->uv(iface, nthvert);
        varying_uv.set_col(nthvert, gl_uv);
        varying_intensity[nthvert] = (model->norm(iface, nthvert) * g_LIGHT_DIRECTION) * -1.0;
        return gl_Vertex;
    }
        
    virtual bool fragment(Vector3d barycentric, TGAColor &color) {
        Vector2d uv = varying_uv * barycentric;
        Vector3d norm_vec = proj<3>(uniform_MIT * embed<4>(model->normalmap(uv))).normalize();
        Vector3d light_in = proj<3>(uniform_M   * embed<4>(g_LIGHT_DIRECTION)).normalize();
        Vector3d light_refl = (light_in * (light_in * norm_vec * 2.0) - light_in).normalize();
        
        double diffuse_intensity = std::max(0.0, (light_in * norm_vec) * -1.0);
        double specular_intensity = pow(std::max((light_refl * g_LIGHT_DIRECTION) *-1.0, 0.0), model->specularmap(uv));
        double intensity = 0.30 + 0.60 * diffuse_intensity + 0.10 * specular_intensity;
        color = model->diffuse(varying_uv * barycentric) * intensity;
        return false;
    }
};

struct ShadowShader : public IShader {
    Model* model;
    Vector3d varying_intensity;
    mat<2, 3, double> varying_uv;
    mat<3, 3, double> varying_tri;
    mat<4, 4, double> uniform_M;
    mat<4, 4, double> uniform_MIT;
    mat<4, 4, double> uniform_MShadow;
    TGAImage shadowbuffer;
    virtual Vector4d vertex(int iface, int nthvert) {
        Vector4d gl_Vertex = g_VIEWPORT * g_PROJECTION * g_MODELVIEW * embed<4>(model->vert(iface, nthvert));
        Vector2d gl_uv = model->uv(iface, nthvert);
        varying_uv.set_col(nthvert, gl_uv);
        varying_tri.set_col(nthvert, proj<3>(gl_Vertex/gl_Vertex[3]));
        return gl_Vertex;
    }
        
    virtual bool fragment(Vector3d barycentric, TGAColor &color) {
        Vector4d shadowbuffer_point = uniform_MShadow * embed<4>(varying_tri * barycentric);
        shadowbuffer_point = shadowbuffer_point / shadowbuffer_point[3];
        double shadow;
        int shadow_depth = shadowbuffer.get(static_cast<int>(shadowbuffer_point[0]), static_cast<int>(shadowbuffer_point[1])).bgra[0] - 1;
        if (shadow_depth < shadowbuffer_point[2]) {
            shadow = 1.0;
        } else {
            shadow = 0.1;
        }
        Vector2d uv = varying_uv * barycentric;
        Vector3d norm_vec = proj<3>(uniform_MIT * embed<4>(model->normalmap(uv))).normalize();
        Vector3d light_in = proj<3>(uniform_M   * embed<4>(g_LIGHT_DIRECTION)).normalize();
        Vector3d light_refl = (light_in * (light_in * norm_vec * 2.0) - light_in).normalize();
        
        double diffuse_intensity = std::max(0.0, (light_in * norm_vec) * -1.0);
        double specular_intensity = pow(std::max((light_refl * g_LIGHT_DIRECTION) *-1.0, 0.0), model->specularmap(uv));
        double intensity = 0.30 + shadow * (0.60 * diffuse_intensity + 0.10 * specular_intensity);
        color = model->diffuse(varying_uv * barycentric) * intensity;
        return false;
    }
};

struct SSAOShader : public IShader {
    Model* model;
    mat<2, 3, double> varying_uv;
    mat<4, 4, double> uniform_MIT;
    mat<3, 3, double> varying_tri;
    TGAImage zbuffer;
    std::vector<Vector3d> kernel;
    virtual Vector4d vertex(int iface, int nthvert) {
        Vector4d gl_Vertex = g_VIEWPORT * g_PROJECTION * g_MODELVIEW * embed<4>(model->vert(iface, nthvert));
        Vector2d gl_uv = model->uv(iface, nthvert);
        varying_uv.set_col(nthvert, gl_uv);
        varying_tri.set_col(nthvert, proj<3>(gl_Vertex/gl_Vertex[3]));
        return gl_Vertex;
    }
        
    virtual bool fragment(Vector3d barycentric, TGAColor &color) {
        auto uv = varying_uv * barycentric;
        Vector3d norm_vec = model->normalmap(uv).normalize();
        // Since the normal vector is ... normalised,
        // we know r = 1.0; We can then calculate
        // theta and phi to rotate our sample vector 
        // into effectively being an offset to the normal
        auto point = varying_tri * barycentric;
        double theta = acos(norm_vec.z);
        double phi = atan(norm_vec.y / norm_vec.x);
        
        auto rot_matrix =  rotation_y(theta) * rotation_z(phi);
        
        double occluded = 0;
        for (auto& sample: kernel) {
            auto vec = (rot_matrix * sample) * 0.0;
            //point = proj<3>(g_VIEWPORT * g_PROJECTION * g_MODELVIEW * embed<4>(point + vec));
            //std::cout << "POINT:        " << (point).x << ", " << (point).y << ", " << (point).z << "\n";
            //std::cout << "POINT + NORM: " << (point + norm_vec).x << ", " <<(point + norm_vec).y << ", " <<  (point + norm_vec).z << "\n";
            //std::cout << "POINT + SAMP: " << (point + vec).x << ", " << (point + vec).y << ", " << (point +vec).z << "\n";
            //std::cout << point.z << " vs " << static_cast<int>(zbuffer.get(point.x, point.y).bgra[0]) << "\n";
            if (zbuffer.get(point.x, point.y).bgra[0] > point.z) {
                occluded += 1;  
            }
        }
        occluded /= 32;
        color = TGAColor(255 * occluded, 255 * occluded, 255 * occluded);
        return false;
    }
};

struct DepthShader : public IShader {
    Model* model;
    mat<3, 3, double> varying_tri;
    
    DepthShader() : varying_tri() {}
    
    virtual Vector4d vertex(int iface, int nthvert) {
        Vector4d gl_Vertex = g_VIEWPORT * g_PROJECTION * g_MODELVIEW * embed<4>(model->vert(iface, nthvert));
        varying_tri.set_col(nthvert, proj<3>(gl_Vertex/gl_Vertex[3]));
        return gl_Vertex;
    }
        
    virtual bool fragment(Vector3d barycentric, TGAColor &color) {
        Vector3d point = varying_tri * barycentric;
        color = TGAColor(255, 255, 255) * (point.z / MAX_DEPTH);
        return false;
    }
};

struct EmptyShader : public IShader {
    Model* model;
    mat<3, 3, double> varying_tri;
    
    EmptyShader() : varying_tri() {}
    
    virtual Vector4d vertex(int iface, int nthvert) {
        Vector4d gl_Vertex = g_VIEWPORT * g_PROJECTION * g_MODELVIEW * embed<4>(model->vert(iface, nthvert));
        varying_tri.set_col(nthvert, proj<3>(gl_Vertex/gl_Vertex[3]));
        return gl_Vertex;
    }
        
    virtual bool fragment(Vector3d barycentric, TGAColor &color) {
        color = TGAColor(0, 0, 0);
        return false;
    }
};