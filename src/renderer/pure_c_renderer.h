#ifndef ESP32_IDE_PURE_C_RENDERER_H
#define ESP32_IDE_PURE_C_RENDERER_H

#include <cstdint>
#include <vector>
#include <memory>
#include <string>

namespace esp32_ide {
namespace renderer {

// Vector structures (defined first)
struct Vector3D {
    float x, y, z;
    
    Vector3D() : x(0), y(0), z(0) {}
    Vector3D(float x_, float y_, float z_) : x(x_), y(y_), z(z_) {}
    
    Vector3D operator+(const Vector3D& v) const { return Vector3D(x + v.x, y + v.y, z + v.z); }
    Vector3D operator-(const Vector3D& v) const { return Vector3D(x - v.x, y - v.y, z - v.z); }
    Vector3D operator*(float s) const { return Vector3D(x * s, y * s, z * s); }
    
    float Length() const;
    Vector3D Normalized() const;
    static float Dot(const Vector3D& a, const Vector3D& b);
    static Vector3D Cross(const Vector3D& a, const Vector3D& b);
};

// Extended 5D vector for advanced visualization
struct Vector5D {
    float x, y, z, w, v;
    
    Vector5D() : x(0), y(0), z(0), w(0), v(0) {}
    Vector5D(float x_, float y_, float z_, float w_, float v_) 
        : x(x_), y(y_), z(z_), w(w_), v(v_) {}
};

// Color structure (RGBA)
struct Color {
    uint8_t r, g, b, a;
    
    Color() : r(0), g(0), b(0), a(255) {}
    Color(uint8_t r_, uint8_t g_, uint8_t b_, uint8_t a_ = 255)
        : r(r_), g(g_), b(b_), a(a_) {}
    
    uint32_t ToRGBA() const { return (a << 24) | (b << 16) | (g << 8) | r; }
    static Color FromRGBA(uint32_t rgba);
    
    // Common colors
    static Color Black() { return Color(0, 0, 0); }
    static Color White() { return Color(255, 255, 255); }
    static Color Red() { return Color(255, 0, 0); }
    static Color Green() { return Color(0, 255, 0); }
    static Color Blue() { return Color(0, 0, 255); }
    static Color Yellow() { return Color(255, 255, 0); }
    static Color Cyan() { return Color(0, 255, 255); }
    static Color Magenta() { return Color(255, 0, 255); }
};

/**
 * Pure C 3D/5D Renderer
 * 
 * A custom rendering engine with zero external dependencies.
 * Supports 3D visualization and extended 5D rendering for
 * advanced device component visualization.
 * 
 * Features:
 * - Software rasterization (no GPU required)
 * - 3D transformations (translate, rotate, scale)
 * - 5D extended space for multi-dimensional data
 * - Wireframe and solid rendering
 * - Basic lighting and shading
 */
class PureCRenderer {
public:
    PureCRenderer();
    ~PureCRenderer();

    // Initialization
    bool Initialize(int width, int height);
    void Shutdown();

    // Frame management
    void BeginFrame();
    void EndFrame();
    void Clear(const Color& color);

    // 3D rendering primitives
    void DrawLine3D(const Vector3D& start, const Vector3D& end, const Color& color);
    void DrawTriangle3D(const Vector3D& v1, const Vector3D& v2, const Vector3D& v3, const Color& color);
    void DrawCube(const Vector3D& center, float size, const Color& color);
    void DrawSphere(const Vector3D& center, float radius, const Color& color);

    // 5D extended rendering (for advanced visualization)
    void DrawLine5D(const Vector5D& start, const Vector5D& end, const Color& color);
    void DrawHypercube(const Vector5D& center, float size, const Color& color);

    // Camera controls
    void SetCamera(const Vector3D& position, const Vector3D& target, const Vector3D& up);
    void RotateCamera(float yaw, float pitch);
    void ZoomCamera(float delta);

    // Transformations
    void PushMatrix();
    void PopMatrix();
    void Translate(const Vector3D& translation);
    void Rotate(float angle, const Vector3D& axis);
    void Scale(const Vector3D& scale);

    // Framebuffer access
    const uint32_t* GetFramebuffer() const;
    int GetWidth() const { return width_; }
    int GetHeight() const { return height_; }

    // Text rendering (basic)
    void DrawText(int x, int y, const std::string& text, const Color& color);

private:
    struct Impl;  // Forward declaration for pimpl
    std::unique_ptr<Impl> pimpl_;
    
    int width_;
    int height_;
    std::vector<uint32_t> framebuffer_;
    std::vector<float> depth_buffer_;
    
    // Camera state (public interface)
    Vector3D camera_position_;
    Vector3D camera_target_;
    Vector3D camera_up_;
    float camera_yaw_;
    float camera_pitch_;
    float camera_distance_;
    
    // Helper functions
    Vector3D Project3D(const Vector3D& point);
    Vector3D Project5DTo3D(const Vector5D& point);
    void DrawPixel(int x, int y, const Color& color);
    void DrawLine2D(int x1, int y1, int x2, int y2, const Color& color);
    bool DepthTest(int x, int y, float depth);
};

} // namespace renderer
} // namespace esp32_ide

#endif // ESP32_IDE_PURE_C_RENDERER_H
