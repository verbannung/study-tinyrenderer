#include <algorithm>
#include "our_gl.h"

mat<4,4> ModelView, Viewport, Perspective; // "OpenGL" state matrices
std::vector<double> zbuffer;               // depth buffer

void lookat(const vec3 eye, const vec3 center, const vec3 up) {
    // n 指向 eye，l/m/n 构成相机正交基。本项目采用旋转 * 平移(-center)；
    // 这与常见 OpenGL 教材使用平移(-eye) 的 look-at 不同，是课程的简化相机约定。
    vec3 n = normalized(eye-center);
    vec3 l = normalized(cross(up,n));
    vec3 m = normalized(cross(n, l));
    ModelView = mat<4,4>{{{l.x,l.y,l.z,0}, {m.x,m.y,m.z,0}, {n.x,n.y,n.z,0}, {0,0,0,1}}} *
                mat<4,4>{{{1,0,0,-center.x}, {0,1,0,-center.y}, {0,0,1,-center.z}, {0,0,0,1}}};
}

void init_perspective(const double f) {
    // 仅令 clip.w = 1-view.z/f；这不是带 near/far 裁剪面的标准 OpenGL 投影矩阵。
    Perspective = {{{1,0,0,0}, {0,1,0,0}, {0,0,1,0}, {0,0, -1/f,1}}};
}

void init_viewport(const int x, const int y, const int w, const int h) {
    // 把 NDC 的 [-1,1] 映射至给定屏幕矩形；本项目的 z 保持不变。
    Viewport = {{{w/2., 0, 0, x+w/2.}, {0, h/2., 0, y+h/2.}, {0,0,1,0}, {0,0,0,1}}};
}

void init_zbuffer(const int width, const int height) {
    zbuffer = std::vector(width*height, -1000.);
}

void rasterize(const Triangle &clip, const IShader &shader, TGAImage &framebuffer) {
    // 先透视除法得到 NDC，再由 Viewport 映射至屏幕像素坐标。
    vec4 ndc[3]    = { clip[0]/clip[0].w, clip[1]/clip[1].w, clip[2]/clip[2].w };                // normalized device coordinates
    vec2 screen[3] = { (Viewport*ndc[0]).xy(), (Viewport*ndc[1]).xy(), (Viewport*ndc[2]).xy() }; // screen coordinates

    mat<3,3> ABC = {{ {screen[0].x, screen[0].y, 1.}, {screen[1].x, screen[1].y, 1.}, {screen[2].x, screen[2].y, 1.} }};
    // 行列式为有符号二倍面积；<1 同时实现当前绕序的背面剔除及亚像素三角形剔除。
    if (ABC.det()<1) return;

    auto [bbminx,bbmaxx] = std::minmax({screen[0].x, screen[1].x, screen[2].x}); // bounding box for the triangle
    auto [bbminy,bbmaxy] = std::minmax({screen[0].y, screen[1].y, screen[2].y}); // defined by its top left and bottom right corners
    // OpenMP 仅切分 x 列，同一三角形内线程写不同像素；若并行外层三角形则共享缓冲会有竞争。
#pragma omp parallel for
    for (int x=std::max<int>(bbminx, 0); x<=std::min<int>(bbmaxx, framebuffer.width()-1); x++) {         // clip the bounding box by the screen
        for (int y=std::max<int>(bbminy, 0); y<=std::min<int>(bbmaxy, framebuffer.height()-1); y++) {
            // ABC 的行是三个顶点；解 ABC^T*bc=pixel，故乘 inverse-transpose 得屏幕重心坐标。
            vec3 bc_screen = ABC.invert_transpose() * vec3{static_cast<double>(x), static_cast<double>(y), 1.};
            // 各分量先除 clip.w 再归一化，供 varying 作透视校正插值（perspective-correct interpolation）。
            vec3 bc_clip   = { bc_screen.x/clip[0].w, bc_screen.y/clip[1].w, bc_screen.z/clip[2].w };
            bc_clip = bc_clip / (bc_clip.x + bc_clip.y + bc_clip.z);
            if (bc_screen.x<0 || bc_screen.y<0 || bc_screen.z<0) continue; // negative barycentric coordinate => the pixel is outside the triangle
            // NDC z 在屏幕平面线性插值，特意使用 bc_screen；着色属性则使用 bc_clip。
            double z = bc_screen * vec3{ ndc[0].z, ndc[1].z, ndc[2].z };
            if (z <= zbuffer[x+y*framebuffer.width()]) continue;   // discard fragments that are too deep w.r.t the z-buffer
            auto [discard, color] = shader.fragment(bc_clip);
            if (discard) continue;                                 // fragment shader can discard current fragment
            zbuffer[x+y*framebuffer.width()] = z;                  // update the z-buffer
            framebuffer.set(x, y, color);                          // update the framebuffer
        }
    }
}
