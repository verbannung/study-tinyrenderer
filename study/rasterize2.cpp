#include "model.h"
#include "tgaimage.h"
#include <cmath>

constexpr int width  = 125;
constexpr int height = 125;

constexpr TGAColor white   = {255, 255, 255, 255}; // attention, BGRA order
constexpr TGAColor green   = {  0, 255,   0, 255};
constexpr TGAColor red     = {  0,   0, 255, 255};
constexpr TGAColor blue    = {255, 128,  64, 255};
constexpr TGAColor yellow  = {  0, 200, 255, 255};

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
// 向量叉积计算三角形面积 
double signed_triangle_area(int ax,int ay,int bx,int by,int cx,int cy){
    return .5*((by-ay)*(bx+ax) + (cy-by)*(cx+bx) + (ay-cy)*(ax+cx));
}

// 重心坐标插值

void triangle(int ax, int ay, int bx, int by, int cx, int cy, TGAImage &framebuffer, TGAColor color) {
   int minx= std::min(ax, std::min(bx, cx));
   int maxx= std::max(ax, std::max(bx, cx));
   int miny= std::min(ay, std::min(by, cy));
   int maxy= std::max(ay, std::max(by, cy));
   double total_area = signed_triangle_area(ax, ay, bx, by, cx, cy);
   for (int y=miny; y<=maxy; y++) {
    for (int x=minx;x<=maxx;x++){
        double alpha = signed_triangle_area(x,y,ax,ay,bx,by)/total_area;
        double beta = signed_triangle_area(x,y,bx,by,cx,cy)/total_area;
        double gamma = signed_triangle_area(x,y,cx,cy,ax,ay)/total_area;
        if(alpha>0 && beta>0 && gamma>0){
            framebuffer.set(x,y,color);
        }
    }
   }
}

   

int main(int argc, char** argv) {
    TGAImage framebuffer(width, height, TGAImage::RGB);
    triangle(  7, 45, 35, 100, 45,  60, framebuffer, red);
    triangle(120, 35, 90,   5, 45, 110, framebuffer, white);
    triangle(115, 83, 80,  90, 85, 120, framebuffer, green);
    framebuffer.write_tga_file("framebuffer.tga");
    return 0;
}