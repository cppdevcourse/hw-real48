#include "real48.hpp"

#include <cmath>
#include <cstring>
#include <limits>
#include <stdexcept>

namespace math
{

template <typename T>
int classify_ieee(T value)
{
    return std::fpclassify(value);
}

Real48::Real48(const float number)
{
    auto cls = classify_ieee(number);

    if (cls == FP_NAN)
    {
        throw std::overflow_error("Cannot represent NaN in Real48");
    }

    if (cls == FP_INFINITE)
    {
        throw std::overflow_error("Cannot represent infinity in Real48");
    }

    if (cls == FP_ZERO || cls == FP_SUBNORMAL)
    {
        data_.exponent = 0;
        data_.mantissa = 0;
        data_.sign = 0;
        return;
    }

    if (cls != FP_NORMAL)
    {
        throw std::overflow_error("Unknown floating point class");
    }

    uint32_t bits;
    std::memcpy(&bits, &number, sizeof(float));

    uint8_t s32 = (bits >> 31) & 1;
    int32_t e32 = (bits >> 23) & 0xFF;
    uint32_t f32 = bits & 0x7FFFFF;

    int32_t e48 = e32 - 127 + 129;

    if (e48 < 1 || e48 > 255)
    {
        throw std::overflow_error("Exponent out of range for Real48");
    }

    uint64_t f48 = static_cast<uint64_t>(f32) << 16;

    data_.exponent = static_cast<uint8_t>(e48);
    data_.mantissa = f48;
    data_.sign = s32;
}

Real48::Real48(const double number)
{
    auto cls = classify_ieee(number);

    if (cls == FP_NAN)
    {
        throw std::overflow_error("Cannot represent NaN in Real48");
    }

    if (cls == FP_INFINITE)
    {
        throw std::overflow_error("Cannot represent infinity in Real48");
    }

    if (cls == FP_ZERO || cls == FP_SUBNORMAL)
    {
        data_.exponent = 0;
        data_.mantissa = 0;
        data_.sign = 0;
        return;
    }

    if (cls != FP_NORMAL)
    {
        throw std::overflow_error("Unknown floating point class");
    }

    uint64_t bits;
    std::memcpy(&bits, &number, sizeof(double));

    uint8_t s64 = (bits >> 63) & 1;
    int32_t e64 = (bits >> 52) & 0x7FF;
    uint64_t f64 = bits & 0xFFFFFFFFFFFFFull;

    int32_t e48 = e64 - 1023 + 129;

    if (e48 < 1 || e48 > 255)
    {
        throw std::overflow_error("Exponent out of range for Real48");
    }

    uint64_t f48 = f64 >> 13;

    data_.exponent = static_cast<uint8_t>(e48);
    data_.mantissa = f48;
    data_.sign = s64;
}

Real48::operator float() const
{
    uint8_t e48 = data_.exponent;

    if (e48 == 0)
    {
        return 0.0f;
    }

    uint64_t f48 = data_.mantissa;
    uint8_t s48 = data_.sign;

    int32_t e32 = e48 - 129 + 127;

    if (e32 < 1 || e32 > 254)
    {
        throw std::overflow_error("Cannot represent in float");
    }

    uint32_t f32 = static_cast<uint32_t>(f48 >> 16);

    uint32_t bits = 0;
    bits |= static_cast<uint32_t>(s48) << 31;
    bits |= static_cast<uint32_t>(e32) << 23;
    bits |= f32 & 0x7FFFFF;

    float result;
    std::memcpy(&result, &bits, sizeof(float));
    return result;
}

Real48::operator double() const noexcept
{
    uint8_t e48 = data_.exponent;

    if (e48 == 0)
    {
        return 0.0;
    }

    uint64_t f48 = data_.mantissa;
    uint8_t s48 = data_.sign;

    int32_t e64 = e48 - 129 + 1023;

    uint64_t f64 = f48 << 13;

    uint64_t bits = 0;
    bits |= static_cast<uint64_t>(s48) << 63;
    bits |= static_cast<uint64_t>(e64) << 52;
    bits |= f64 & 0xFFFFFFFFFFFFFull;

    double result;
    std::memcpy(&result, &bits, sizeof(double));
    return result;
}

Real48& Real48::operator+=(const Real48& b)
{
    auto a_val = static_cast<double>(*this);
    auto b_val = static_cast<double>(b);
    *this = Real48(a_val + b_val);
    return *this;
}

Real48& Real48::operator-=(const Real48& b)
{
    auto a_val = static_cast<double>(*this);
    auto b_val = static_cast<double>(b);
    *this = Real48(a_val - b_val);
    return *this;
}

Real48& Real48::operator*=(const Real48& b)
{
    auto a_val = static_cast<double>(*this);
    auto b_val = static_cast<double>(b);
    *this = Real48(a_val * b_val);
    return *this;
}

Real48& Real48::operator/=(const Real48& b)
{
    auto a_val = static_cast<double>(*this);
    auto b_val = static_cast<double>(b);
    *this = Real48(a_val / b_val);
    return *this;
}

Real48 Real48::operator+() const noexcept
{
    return *this;
}

Real48 Real48::operator-() const noexcept
{
    Real48 result = *this;
    if (result.data_.exponent == 0)
    {
        return result;
    }

    if (result.data_.sign == 1)
    {
        result.data_.sign = 0;
    }
    else
    {
        result.data_.sign = 1;
    }

    return result;
}

Real48 Real48::operator+(const Real48& o) const
{
    auto a_val = static_cast<double>(*this);
    auto o_val = static_cast<double>(o);
    return Real48(a_val + o_val);
}

Real48 Real48::operator-(const Real48& o) const
{
    auto a_val = static_cast<double>(*this);
    auto o_val = static_cast<double>(o);
    return Real48(a_val - o_val);
}

Real48 Real48::operator*(const Real48& o) const
{
    auto a_val = static_cast<double>(*this);
    auto o_val = static_cast<double>(o);
    return Real48(a_val * o_val);
}

Real48 Real48::operator/(const Real48& o) const
{
    auto a_val = static_cast<double>(*this);
    auto o_val = static_cast<double>(o);
    return Real48(a_val / o_val);
}

bool Real48::operator>(const Real48& o) const noexcept
{
    auto a_val = static_cast<double>(*this);
    auto o_val = static_cast<double>(o);
    return a_val > o_val;
}

bool Real48::operator<(const Real48& o) const noexcept
{
    auto a_val = static_cast<double>(*this);
    auto o_val = static_cast<double>(o);
    return a_val < o_val;
}

Real48::Class Real48::Classify() const noexcept
{
    return (data_.exponent == 0) ? Class::ZERO : Class::NORMAL;
}

} // namespace math
