#pragma once

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <Eigen/Dense>
#include <stdexcept>

typedef float  float32_t;
typedef double float64_t;

namespace UTILITY
{
using Vector2f_t = Eigen::Vector2f;
using Vector3f_t = Eigen::Vector3f;

typedef Eigen::Rotation2D<float32_t> RotationMatrix2D_t;

using Matrix2f_t = Eigen::Matrix<float32_t, 2, 2>;
using Matrix3f_t = Eigen::Matrix<float32_t, 3, 3>;

typedef std::vector<UTILITY::Vector3f_t> LiDARPtVector_t;

template <size_t _Rows, size_t _Cols>
using MatrixXf_t = Eigen::Matrix<float32_t, _Rows, _Cols>;

// Degree to radian conversions
constexpr float32_t degreesToRadians(float32_t deg)
{
    // (π / 180.0) = 0.0174532925199433F
    return deg * 0.0174532925199433F;
}
constexpr float64_t degreesToRadians(float64_t deg)
{
    // (π / 180.0) = 0.0174532925199433F
    return deg * 0.0174532925199433;
}

} // namespace UTILITY
