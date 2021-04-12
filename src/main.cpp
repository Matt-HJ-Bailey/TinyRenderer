#include <vector>
#include <cmath>
#include <chrono>
#include <iostream>
#include <algorithm>
#include <stdexcept>
#include <random>
#include "tgaimage.h"
#include "model.h"
#include "geometry.h"
#include "our_gl.h"
#include "shaders.h"

std::vector<Model *> models;

Vector3d g_ORIGIN(0.0, 0.0, 0.0);
Vector3d g_CAMERA_POS(0.0, 0.0, 100.0);
Vector3d g_LIGHT_DIRECTION(0.33, 0.33, 0.33);
Vector3d g_UPWARDS(0, 1.0, 0.0);
const double PI = std::atan(1.0)*4;
//extern const int SCREEN_X;
//extern const int SCREEN_Y;
//extern Matrix g_VIEWPORT;
//extern Matrix g_PROJECTION;
//extern Matrix g_MODELVIEW;
TGAImage g_SHADOWBUFFER;
//double CAMERA_SPEED = 0.5;


void draw_frame(std::vector<Model*> models/*, SDL_Renderer*& renderer*/) {

    auto model_end_time = std::chrono::high_resolution_clock::now();
    TGAImage image(SCREEN_X, SCREEN_Y, TGAImage::RGB);
    TGAImage ssao_buffer(SCREEN_X, SCREEN_Y, TGAImage::RGB);
	TGAImage zbuffer(SCREEN_X, SCREEN_Y, TGAImage::GRAYSCALE);
    TGAImage g_SHADOWBUFFER(SCREEN_X, SCREEN_Y, TGAImage::GRAYSCALE);

    // Shadowbuffer pass
    for (auto& model: models) {
        lookat(g_LIGHT_DIRECTION, g_ORIGIN, g_UPWARDS);
        viewport(0, 0, SCREEN_X, SCREEN_Y);
        projection(0);
        DepthShader shader;
        shader.model = model;
        for (int i=0; i < model->nfaces(); ++i) {
            Vector4d screen_coords[3];
            for (int j = 0; j < 3; ++j){
                // The vertex shader replaces the world to screen calculation
                screen_coords[j] = shader.vertex(i, j);
            }
            Triangle triangle(screen_coords[0], screen_coords[1], screen_coords[2], g_SHADOWBUFFER);
            triangle.draw_texture(zbuffer, g_SHADOWBUFFER, shader);
        }
    }
    
    Matrix MShadow = g_VIEWPORT * g_PROJECTION * g_MODELVIEW;
 	zbuffer.clear();
 
    // Final rendering
    for (auto& model: models) {
        lookat(g_CAMERA_POS, g_ORIGIN, g_UPWARDS);
        viewport(0, 0, SCREEN_X, SCREEN_Y);
        projection(-1.0/(g_CAMERA_POS - g_ORIGIN).norm());
        ShadowShader shader;
        shader.model = model;
        shader.uniform_M = g_PROJECTION * g_MODELVIEW;
        shader.uniform_MIT = (g_PROJECTION * g_MODELVIEW).invert_transpose();
        shader.uniform_MShadow = MShadow * (g_VIEWPORT * g_PROJECTION * g_MODELVIEW).invert();
        shader.shadowbuffer = g_SHADOWBUFFER;
        for (int i=0; i < model->nfaces(); ++i) {
            Vector4d screen_coords[3];
            for (int j = 0; j < 3; ++j){
                // The vertex shader replaces the world to screen calculation
                screen_coords[j] = shader.vertex(i, j);
            }
            Triangle triangle(screen_coords[0], screen_coords[1], screen_coords[2], image);
            triangle.draw_texture(zbuffer, image, shader);
        }
    }

    auto render_end_time = std::chrono::high_resolution_clock::now();
    auto render_duration = std::chrono::duration_cast<std::chrono::duration<double>>(render_end_time - model_end_time);
    std::cout << "Render done  in " << render_duration.count()*1000 << "ms\n";
    image.flip_vertically(); // i want to have the origin at the left bottom corner of the image
    image.write_tga_file("output.tga");
	zbuffer.flip_vertically();
	zbuffer.write_tga_file("zbuffer.tga");
	ssao_buffer.flip_vertically();
	ssao_buffer.write_tga_file("zbuffer.tga");
    g_SHADOWBUFFER.flip_vertically(); // i want to have the origin at the left bottom corner of the image
    g_SHADOWBUFFER.write_tga_file("shadow.tga");
    auto output_end_time = std::chrono::high_resolution_clock::now();
    auto output_duration = std::chrono::duration_cast<std::chrono::duration<double>>(output_end_time - render_end_time);
    std::cout << "Written out  in " << output_duration.count()*1000 << "ms\n";
    
    //SDL_RenderPresent(renderer);    
}
int main() {
    
    /* SDL_Event event;
    SDL_Renderer *renderer;
    SDL_Window *window;
    SDL_Init(SDL_INIT_VIDEO);
    SDL_CreateWindowAndRenderer(SCREEN_X, SCREEN_Y, 0, &window, &renderer);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
    SDL_RenderClear(renderer);  */  
    

    auto start_time = std::chrono::high_resolution_clock::now();
 

    auto model = new Model("obj/african_head.obj");
    models.push_back(model);
    //MODEL = new Model("obj/african_head_eye_outer.obj");
    //models.push_back(MODEL);
    model = new Model("obj/african_head_eye_inner.obj");
    models.push_back(model);
    model = new Model("obj/floor.obj");
    models.push_back(model);    
    auto model_end_time = std::chrono::high_resolution_clock::now();
    auto model_duration = std::chrono::duration_cast<std::chrono::duration<double>>(model_end_time - start_time);
    std::cout << "Model loaded in " << model_duration.count() * 1000 << "ms\n";
    // Vector3d camera_vel(0.0, 0.0, 0.0);
    // bool BREAK_FLAG = false;
    draw_frame(models /*, renderer*/);
    /* while (true) {
        SDL_PollEvent(&event);
        switch (event.type){
            case SDL_KEYDOWN:
                switch(event.key.keysym.sym) {
                    case SDLK_a:
                        camera_vel.x = CAMERA_SPEED;
                        break;
                    case SDLK_d:
                        camera_vel.x  = -CAMERA_SPEED;
                        break;
                    case SDLK_w:
                        camera_vel.y = CAMERA_SPEED;
                        break;
                    case SDLK_s:
                        camera_vel.y = -CAMERA_SPEED;
                        break;
                    case SDLK_SPACE:
                        camera_vel.z = CAMERA_SPEED;
                        break; 
                    case SDLK_c:
                        camera_vel.z = -CAMERA_SPEED;
                        break;
                    default:
                        break;
                }
                break;
            case SDL_KEYUP:
                switch(event.key.keysym.sym) {
                    case SDLK_a:
                        if (camera_vel.x < 0) {
                            camera_vel.x = 0.0;
                        }
                        break;
                    case SDLK_d:
                        if (camera_vel.x > 0) {
                            camera_vel.x = 0.0;
                        }                   
                        break;
                    case SDLK_w:
                        if (camera_vel.y > 0) {
                            camera_vel.y = 0.0;
                        }
                        break;
                    case SDLK_s:
                        if (camera_vel.y < 0) {
                            camera_vel.y = 0.0;
                        }
                        break;
                    case SDLK_SPACE:
                        if (camera_vel.y > 0) {
                            camera_vel.z = 0.0;
                        }
                        break; 
                    case SDLK_c:
                        if (camera_vel.y < 0) {
                            camera_vel.z = 0.0;
                        }
                        break;
                    default:
                        break;
                }
                break;
            case SDL_QUIT:
                BREAK_FLAG = true;
                break;
        }
        if (!BREAK_FLAG) {
            if (camera_vel.x != 0.0 && camera_vel.y != 0.0 && camera_vel.z != 0.0) {
                g_CAMERA_POS = camera_vel + g_CAMERA_POS;
                draw_frame(model, renderer);
                std::cout << g_CAMERA_POS << "\n";
            }
        } else {
            break;
        }
    }
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit(); */

    // delete model;
    return 0;
}

