#include <cmath>
#include <complex>
#include <tuple>
#include "geometry.h"
#include "model.h"
#include "tgaimage.h"
// 参考https://haqr.eu/tinyrenderer/bresenham/ 
constexpr int width  = 800;
constexpr int height = 800;

constexpr TGAColor white   = {255, 255, 255, 255}; // attention, BGRA order
constexpr TGAColor green   = {  0, 255,   0, 255};
constexpr TGAColor red     = {  0,   0, 255, 255};
constexpr TGAColor blue    = {255, 128,  64, 255};
constexpr TGAColor yellow  = {  0, 200, 255, 255};
//渲染线段，从最增速最慢的轴开始，这样避免过多的像素的断裂情况
void line(int ax, int ay, int bx, int by, TGAImage &framebuffer, TGAColor color) {
    bool steep = std::abs(ax-bx) < std::abs(ay-by);
    if (steep) { // if the line is steep, we transpose the image
        std::swap(ax, ay);
        std::swap(bx, by);
    }
    if (ax>bx) { // make it left−to−right
        std::swap(ax, bx);
        std::swap(ay, by);
    }
    int y = ay;
    int ierror = 0;
    // 
    for (int x=ax; x<=bx; x++) {
        if (steep) // if transposed, de−transpose
            framebuffer.set(y, x, color);
        else
            framebuffer.set(x, y, color);
        ierror += 2 * std::abs(by-ay);
        if (ierror > bx - ax) {
            y += by > ay ? 1 : -1;
            ierror -= 2 * (bx-ax);
        }
    }
}

std::tuple<int,int> project(vec4 v) { // First of all, (x,y) is an orthogonal projection of the vector (x,y,z).
    return { (v.x + 1.) *  width/2.,   // Second, since the input models are scaled to have fit in the [-1,1]^3 world coordinates,
             (v.y + 1.) * height/2 }; // we want to shift the vector (x,y) and then scale it to span the entire screen.
}
//一个正交投影 把三维的点投影到二维平面上
std::tuple<int,int> project(vec3 v) { // First of all, (x,y) is an orthogonal projection of the vector (x,y,z).
    return { (v.x + 1.) *  width/2.,   // Second, since the input models are scaled to have fit in the [-1,1]^3 world coordinates,
             (v.y + 1.) * height/2 }; // we want to shift the vector (x,y) and then scale it to span the entire screen.
}

//最原始的线框渲染
int main(int argc, char** argv) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " obj/model.obj" << std::endl;
        return 1;
    }

    Model model(argv[1]);
    TGAImage framebuffer(width, height, TGAImage::RGB);

    for (int i=0; i<model.nfaces(); i++) { // iterate through all triangles
        auto [ax, ay] = project(model.vert(i, 0).xyz());
        auto [bx, by] = project(model.vert(i, 1).xyz());
        auto [cx, cy] = project(model.vert(i, 2).xyz());
        line(ax, ay, bx, by, framebuffer, red);
        line(bx, by, cx, cy, framebuffer, red);
        line(cx, cy, ax, ay, framebuffer, red);
    }

    for (int i=0; i<model.nverts(); i++) { // iterate through all vertices
        vec3 v = model.vert(i).xyz();            // get i-th vertex
        auto [x, y] = project(v);          // project it to the screen
        framebuffer.set(x, y, white);
    }

    framebuffer.write_tga_file("framebuffer.tga");
    return 0;
}