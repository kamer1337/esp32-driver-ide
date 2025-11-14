#include "pure_c_renderer.h"
#include <cmath>
#include <algorithm>
#include <cstring>

namespace esp32_ide {
namespace renderer {

// Matrix4x4 structure
struct Matrix4x4 {
    float m[16];
    
    Matrix4x4() {
        Identity();
    }
    
    void Identity() {
        std::memset(m, 0, sizeof(m));
        m[0] = m[5] = m[10] = m[15] = 1.0f;
    }
    
    static Matrix4x4 Multiply(const Matrix4x4& a, const Matrix4x4& b) {
        Matrix4x4 result;
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 4; j++) {
                result.m[i * 4 + j] = 0;
                for (int k = 0; k < 4; k++) {
                    result.m[i * 4 + j] += a.m[i * 4 + k] * b.m[k * 4 + j];
                }
            }
        }
        return result;
    }
    
    Vector3D Transform(const Vector3D& v) const {
        float x = v.x * m[0] + v.y * m[4] + v.z * m[8] + m[12];
        float y = v.x * m[1] + v.y * m[5] + v.z * m[9] + m[13];
        float z = v.x * m[2] + v.y * m[6] + v.z * m[10] + m[14];
        float w = v.x * m[3] + v.y * m[7] + v.z * m[11] + m[15];
        if (w != 0.0f && w != 1.0f) {
            return Vector3D(x / w, y / w, z / w);
        }
        return Vector3D(x, y, z);
    }
};

// Implementation structure
struct PureCRenderer::Impl {
    // Transformation stack
    std::vector<Matrix4x4> matrix_stack;
    Matrix4x4 view_matrix;
    Matrix4x4 projection_matrix;
    float camera_yaw;
    float camera_pitch;
    float camera_distance;
    
    Impl() : camera_yaw(0), camera_pitch(0), camera_distance(5.0f) {}
};

// PureCRenderer implementation
PureCRenderer::PureCRenderer() 
    : pimpl_(std::make_unique<Impl>()), width_(0), height_(0) {
    camera_position_ = Vector3D(0, 0, 5);
    camera_target_ = Vector3D(0, 0, 0);
    camera_up_ = Vector3D(0, 1, 0);
}

PureCRenderer::~PureCRenderer() {
    Shutdown();
}

bool PureCRenderer::Initialize(int width, int height) {
    width_ = width;
    height_ = height;
    
    framebuffer_.resize(width * height, 0);
    depth_buffer_.resize(width * height, 1.0f);
    
    // Initialize projection matrix (perspective)
    pimpl_->projection_matrix.Identity();
    float aspect = static_cast<float>(width) / height;
    float fov = 60.0f * 3.14159f / 180.0f;
    float f = 1.0f / std::tan(fov / 2.0f);
    float near = 0.1f;
    float far = 100.0f;
    
    pimpl_->projection_matrix.m[0] = f / aspect;
    pimpl_->projection_matrix.m[5] = f;
    pimpl_->projection_matrix.m[10] = (far + near) / (near - far);
    pimpl_->projection_matrix.m[11] = -1.0f;
    pimpl_->projection_matrix.m[14] = (2.0f * far * near) / (near - far);
    pimpl_->projection_matrix.m[15] = 0.0f;
    
    // Initialize view matrix
    SetCamera(camera_position_, camera_target_, camera_up_);
    
    // Initialize matrix stack
    pimpl_->matrix_stack.clear();
    Matrix4x4 identity;
    pimpl_->matrix_stack.push_back(identity);
    
    return true;
}

void PureCRenderer::Shutdown() {
    framebuffer_.clear();
    depth_buffer_.clear();
    pimpl_->matrix_stack.clear();
}

void PureCRenderer::BeginFrame() {
    // No-op for software renderer
}

void PureCRenderer::EndFrame() {
    // No-op for software renderer
}

void PureCRenderer::Clear(const Color& color) {
    uint32_t rgba = color.ToRGBA();
    std::fill(framebuffer_.begin(), framebuffer_.end(), rgba);
    std::fill(depth_buffer_.begin(), depth_buffer_.end(), 1.0f);
}

void PureCRenderer::DrawLine3D(const Vector3D& start, const Vector3D& end, const Color& color) {
    // Transform to screen space
    Vector3D p1 = Project3D(start);
    Vector3D p2 = Project3D(end);
    
    // Clip to screen bounds
    if (p1.z < 0 || p1.z > 1 || p2.z < 0 || p2.z > 1) return;
    
    DrawLine2D(static_cast<int>(p1.x), static_cast<int>(p1.y),
               static_cast<int>(p2.x), static_cast<int>(p2.y), color);
}

void PureCRenderer::DrawTriangle3D(const Vector3D& v1, const Vector3D& v2, const Vector3D& v3, const Color& color) {
    // Project vertices
    Vector3D p1 = Project3D(v1);
    Vector3D p2 = Project3D(v2);
    Vector3D p3 = Project3D(v3);
    
    // Draw triangle edges (wireframe for now)
    DrawLine2D(static_cast<int>(p1.x), static_cast<int>(p1.y),
               static_cast<int>(p2.x), static_cast<int>(p2.y), color);
    DrawLine2D(static_cast<int>(p2.x), static_cast<int>(p2.y),
               static_cast<int>(p3.x), static_cast<int>(p3.y), color);
    DrawLine2D(static_cast<int>(p3.x), static_cast<int>(p3.y),
               static_cast<int>(p1.x), static_cast<int>(p1.y), color);
}

void PureCRenderer::DrawCube(const Vector3D& center, float size, const Color& color) {
    float half = size / 2.0f;
    
    // Define cube vertices
    Vector3D vertices[8] = {
        center + Vector3D(-half, -half, -half),
        center + Vector3D( half, -half, -half),
        center + Vector3D( half,  half, -half),
        center + Vector3D(-half,  half, -half),
        center + Vector3D(-half, -half,  half),
        center + Vector3D( half, -half,  half),
        center + Vector3D( half,  half,  half),
        center + Vector3D(-half,  half,  half)
    };
    
    // Draw cube edges
    int edges[12][2] = {
        {0, 1}, {1, 2}, {2, 3}, {3, 0}, // Front face
        {4, 5}, {5, 6}, {6, 7}, {7, 4}, // Back face
        {0, 4}, {1, 5}, {2, 6}, {3, 7}  // Connecting edges
    };
    
    for (int i = 0; i < 12; i++) {
        DrawLine3D(vertices[edges[i][0]], vertices[edges[i][1]], color);
    }
}

void PureCRenderer::DrawSphere(const Vector3D& center, float radius, const Color& color) {
    // Draw sphere using latitude/longitude lines
    int segments = 16;
    
    for (int i = 0; i < segments; i++) {
        float theta1 = (i * 3.14159f * 2.0f) / segments;
        float theta2 = ((i + 1) * 3.14159f * 2.0f) / segments;
        
        for (int j = 0; j < segments / 2; j++) {
            float phi1 = (j * 3.14159f) / (segments / 2);
            float phi2 = ((j + 1) * 3.14159f) / (segments / 2);
            
            Vector3D v1(
                center.x + radius * std::sin(phi1) * std::cos(theta1),
                center.y + radius * std::cos(phi1),
                center.z + radius * std::sin(phi1) * std::sin(theta1)
            );
            
            Vector3D v2(
                center.x + radius * std::sin(phi1) * std::cos(theta2),
                center.y + radius * std::cos(phi1),
                center.z + radius * std::sin(phi1) * std::sin(theta2)
            );
            
            Vector3D v3(
                center.x + radius * std::sin(phi2) * std::cos(theta1),
                center.y + radius * std::cos(phi2),
                center.z + radius * std::sin(phi2) * std::sin(theta1)
            );
            
            DrawLine3D(v1, v2, color);
            DrawLine3D(v1, v3, color);
        }
    }
}

void PureCRenderer::DrawLine5D(const Vector5D& start, const Vector5D& end, const Color& color) {
    // Project 5D to 3D and draw
    Vector3D p1 = Project5DTo3D(start);
    Vector3D p2 = Project5DTo3D(end);
    DrawLine3D(p1, p2, color);
}

void PureCRenderer::DrawHypercube(const Vector5D& center, float size, const Color& color) {
    // Simplified 5D hypercube projection to 3D
    float half = size / 2.0f;
    
    // Project center to 3D
    Vector3D center3d = Project5DTo3D(center);
    
    // Draw as a cube with additional projections
    DrawCube(center3d, size, color);
    
    // Add extra edges for 5D visualization (simplified)
    DrawCube(center3d + Vector3D(size * 0.2f, size * 0.2f, 0), size * 0.8f, 
             Color(color.r / 2, color.g / 2, color.b / 2));
}

void PureCRenderer::SetCamera(const Vector3D& position, const Vector3D& target, const Vector3D& up) {
    camera_position_ = position;
    camera_target_ = target;
    camera_up_ = up;
    
    // Calculate view matrix (look-at)
    Vector3D z = (position - target).Normalized();
    Vector3D x = Vector3D::Cross(up, z).Normalized();
    Vector3D y = Vector3D::Cross(z, x);
    
    pimpl_->view_matrix.m[0] = x.x; pimpl_->view_matrix.m[4] = x.y; pimpl_->view_matrix.m[8] = x.z;
    pimpl_->view_matrix.m[1] = y.x; pimpl_->view_matrix.m[5] = y.y; pimpl_->view_matrix.m[9] = y.z;
    pimpl_->view_matrix.m[2] = z.x; pimpl_->view_matrix.m[6] = z.y; pimpl_->view_matrix.m[10] = z.z;
    
    pimpl_->view_matrix.m[12] = -Vector3D::Dot(x, position);
    pimpl_->view_matrix.m[13] = -Vector3D::Dot(y, position);
    pimpl_->view_matrix.m[14] = -Vector3D::Dot(z, position);
    pimpl_->view_matrix.m[15] = 1.0f;
}

void PureCRenderer::RotateCamera(float yaw, float pitch) {
    pimpl_->camera_yaw += yaw;
    pimpl_->camera_pitch += pitch;
    
    // Clamp pitch
    pimpl_->camera_pitch = std::max(-89.0f, std::min(89.0f, pimpl_->camera_pitch));
    
    // Update camera position based on spherical coordinates
    float yaw_rad = pimpl_->camera_yaw * 3.14159f / 180.0f;
    float pitch_rad = pimpl_->camera_pitch * 3.14159f / 180.0f;
    
    camera_position_.x = pimpl_->camera_distance * std::cos(pitch_rad) * std::sin(yaw_rad);
    camera_position_.y = pimpl_->camera_distance * std::sin(pitch_rad);
    camera_position_.z = pimpl_->camera_distance * std::cos(pitch_rad) * std::cos(yaw_rad);
    
    SetCamera(camera_position_, camera_target_, camera_up_);
}

void PureCRenderer::ZoomCamera(float delta) {
    pimpl_->camera_distance += delta;
    pimpl_->camera_distance = std::max(1.0f, std::min(50.0f, pimpl_->camera_distance));
    
    // Update camera position
    Vector3D direction = (camera_position_ - camera_target_).Normalized();
    camera_position_ = camera_target_ + direction * pimpl_->camera_distance;
    SetCamera(camera_position_, camera_target_, camera_up_);
}

void PureCRenderer::PushMatrix() {
    pimpl_->matrix_stack.push_back(pimpl_->matrix_stack.back());
}

void PureCRenderer::PopMatrix() {
    if (pimpl_->matrix_stack.size() > 1) {
        pimpl_->matrix_stack.pop_back();
    }
}

void PureCRenderer::Translate(const Vector3D& translation) {
    Matrix4x4 mat;
    mat.Identity();
    mat.m[12] = translation.x;
    mat.m[13] = translation.y;
    mat.m[14] = translation.z;
    pimpl_->matrix_stack.back() = Matrix4x4::Multiply(pimpl_->matrix_stack.back(), mat);
}

void PureCRenderer::Rotate(float angle, const Vector3D& axis) {
    float rad = angle * 3.14159f / 180.0f;
    float c = std::cos(rad);
    float s = std::sin(rad);
    Vector3D a = axis.Normalized();
    
    Matrix4x4 mat;
    mat.m[0] = c + a.x * a.x * (1 - c);
    mat.m[1] = a.x * a.y * (1 - c) - a.z * s;
    mat.m[2] = a.x * a.z * (1 - c) + a.y * s;
    mat.m[4] = a.y * a.x * (1 - c) + a.z * s;
    mat.m[5] = c + a.y * a.y * (1 - c);
    mat.m[6] = a.y * a.z * (1 - c) - a.x * s;
    mat.m[8] = a.z * a.x * (1 - c) - a.y * s;
    mat.m[9] = a.z * a.y * (1 - c) + a.x * s;
    mat.m[10] = c + a.z * a.z * (1 - c);
    
    pimpl_->matrix_stack.back() = Matrix4x4::Multiply(pimpl_->matrix_stack.back(), mat);
}

void PureCRenderer::Scale(const Vector3D& scale) {
    Matrix4x4 mat;
    mat.Identity();
    mat.m[0] = scale.x;
    mat.m[5] = scale.y;
    mat.m[10] = scale.z;
    pimpl_->matrix_stack.back() = Matrix4x4::Multiply(pimpl_->matrix_stack.back(), mat);
}

const uint32_t* PureCRenderer::GetFramebuffer() const {
    return framebuffer_.data();
}

void PureCRenderer::DrawText(int x, int y, const std::string& text, const Color& color) {
    // Basic text rendering (8x8 bitmap font)
    // Simplified - just draw rectangles for now
    for (size_t i = 0; i < text.length(); i++) {
        int px = x + i * 8;
        if (px >= 0 && px < width_ - 8 && y >= 0 && y < height_ - 8) {
            // Draw character placeholder
            for (int dy = 0; dy < 8; dy++) {
                for (int dx = 0; dx < 6; dx++) {
                    DrawPixel(px + dx, y + dy, color);
                }
            }
        }
    }
}

Vector3D PureCRenderer::Project3D(const Vector3D& point) {
    // Apply model-view-projection transformation
    Vector3D transformed = pimpl_->matrix_stack.back().Transform(point);
    transformed = pimpl_->view_matrix.Transform(transformed);
    transformed = pimpl_->projection_matrix.Transform(transformed);
    
    // Convert to screen coordinates
    float x = (transformed.x + 1.0f) * width_ / 2.0f;
    float y = (1.0f - transformed.y) * height_ / 2.0f;
    
    return Vector3D(x, y, transformed.z);
}

Vector3D PureCRenderer::Project5DTo3D(const Vector5D& point) {
    // Simple 5D to 3D projection (orthographic)
    // Project w and v dimensions into color/alpha
    return Vector3D(point.x, point.y, point.z);
}

void PureCRenderer::DrawPixel(int x, int y, const Color& color) {
    if (x >= 0 && x < width_ && y >= 0 && y < height_) {
        int index = y * width_ + x;
        framebuffer_[index] = color.ToRGBA();
    }
}

void PureCRenderer::DrawLine2D(int x1, int y1, int x2, int y2, const Color& color) {
    // Bresenham's line algorithm
    int dx = std::abs(x2 - x1);
    int dy = std::abs(y2 - y1);
    int sx = (x1 < x2) ? 1 : -1;
    int sy = (y1 < y2) ? 1 : -1;
    int err = dx - dy;
    
    while (true) {
        DrawPixel(x1, y1, color);
        
        if (x1 == x2 && y1 == y2) break;
        
        int e2 = 2 * err;
        if (e2 > -dy) {
            err -= dy;
            x1 += sx;
        }
        if (e2 < dx) {
            err += dx;
            y1 += sy;
        }
    }
}

bool PureCRenderer::DepthTest(int x, int y, float depth) {
    if (x < 0 || x >= width_ || y < 0 || y >= height_) return false;
    
    int index = y * width_ + x;
    if (depth < depth_buffer_[index]) {
        depth_buffer_[index] = depth;
        return true;
    }
    return false;
}

// Vector3D methods
float Vector3D::Length() const {
    return std::sqrt(x * x + y * y + z * z);
}

Vector3D Vector3D::Normalized() const {
    float len = Length();
    if (len > 0) {
        return Vector3D(x / len, y / len, z / len);
    }
    return Vector3D(0, 0, 0);
}

float Vector3D::Dot(const Vector3D& a, const Vector3D& b) {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

Vector3D Vector3D::Cross(const Vector3D& a, const Vector3D& b) {
    return Vector3D(
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x
    );
}

// Color methods
Color Color::FromRGBA(uint32_t rgba) {
    return Color(
        static_cast<uint8_t>(rgba & 0xFF),
        static_cast<uint8_t>((rgba >> 8) & 0xFF),
        static_cast<uint8_t>((rgba >> 16) & 0xFF),
        static_cast<uint8_t>((rgba >> 24) & 0xFF)
    );
}

} // namespace renderer
} // namespace esp32_ide
