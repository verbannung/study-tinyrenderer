#include "tgaimage.h"
#include "geometry.h"

// 以下初始化函数修改软件版固定功能状态：相机、投影、视口与深度缓冲。
void lookat(const vec3 eye, const vec3 center, const vec3 up);
void init_perspective(const double f);
void init_viewport(const int x, const int y, const int w, const int h);
void init_zbuffer(const int width, const int height);

struct IShader {
    // 最近邻采样：UV 乘纹理尺寸后截断为整数，不执行双线性过滤。
    static TGAColor sample2D(const TGAImage &img, const vec2 &uvf) {
        return img.get(uvf[0] * img.width(), uvf[1] * img.height());
    }
    // bar 是透视校正后的重心坐标；first=true 表示片元丢弃（fragment discard）。
    virtual std::pair<bool,TGAColor> fragment(const vec3 bar) const = 0;
};

// 三个顶点仍处于裁剪空间（clip space），尚未除以各自的 w。
typedef vec4 Triangle[3];
void rasterize(const Triangle &clip, const IShader &shader, TGAImage &framebuffer);
