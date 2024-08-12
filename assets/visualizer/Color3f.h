#pragma once

#include "GL/glew.h"

#include "UTILITYMath.h"
#include "imgui.h"

namespace UTILITY
{
struct Color3f_t
{
    GLfloat r;
    GLfloat g;
    GLfloat b;

    constexpr Color3f_t()
        : r(0.0F)
        , g(0.0F)
        , b(0.0F){};

    constexpr Color3f_t(GLfloat _r, GLfloat _g, GLfloat _b)
        : r(_r)
        , g(_g)
        , b(_b){};

    Color3f_t& operator+=(const Color3f_t& other)
    {
        this->r += other.r;
        this->g += other.g;
        this->b += other.b;
        return *this;
    }

    Color3f_t& operator*=(GLfloat scale)
    {
        this->r *= scale;
        this->g *= scale;
        this->b *= scale;
        return *this;
    };

    Color3f_t operator+(const Color3f_t& other) const
    {
        return Color3f_t(*this) += other;
    };

    Color3f_t operator*(GLfloat scale) const
    {
        return Color3f_t(*this) *= scale;
    };

    bool operator==(const Color3f_t& other) const
    {
        return r == other.r && g == other.g && b == other.b;
    }

    bool operator!=(const Color3f_t& other) const
    {
        return !(*this == other);
    }

    inline ImVec4 vec4(float32_t alpha = 1.0F) const
    {
        return ImVec4(r, g, b, alpha);
    };
};

// Define a non-member function to allow left-multiplication by a scalar.
inline Color3f_t operator*(GLfloat scale, const Color3f_t& color)
{
    return Color3f_t(color) *= scale;
};
} // namespace UTILITY

// Inject a custom hash function for Color3f_t; enables using Color3f_t as keys in an unordered_map, for instance
namespace std
{
template <>
struct hash<UTILITY::Color3f_t>
{
    std::size_t operator()(UTILITY::Color3f_t const& color) const noexcept
    {
        std::size_t redHash   = std::hash<GLfloat>{}(color.r);
        std::size_t greenHash = std::hash<GLfloat>{}(color.g);
        std::size_t blueHash  = std::hash<GLfloat>{}(color.b);

        return redHash ^ (greenHash << 1) ^ (blueHash << 2);
    }
};
} // namespace std
