#include "real48.hpp"

#include <cstring>
#include <stdexcept>


math::Real48::Real48(const float number) : data{}{
    uint64_t number_bits{};
    std::memcpy(&number_bits, &number, sizeof(float));
    const uint64_t sign = number_bits >> 31;
    const uint64_t exponent = number_bits >> 23 & ((1<<8) - 1);
    const uint64_t mantissa = number_bits & ((1<<23) - 1);

    if (exponent == 0)
    {
        return;
    }
    if (exponent == 0xFF)
    {
        throw std::overflow_error("Invalid exponent");
    }
    const uint64_t real48exp = exponent + 2;
    validate_range(real48exp);


    const uint64_t result = (sign << 47 | (mantissa << 16) << 8 | real48exp);
    std::memcpy(data, &result, 6);
}


math::Real48::Real48(const double number) : data{}{
    uint64_t number_bits{};
    std::memcpy(&number_bits, &number, sizeof(double));
    const uint64_t sign = number_bits >> 63;
    const uint64_t exponent = number_bits >> 52 & ((1<<11) - 1);
    const uint64_t mantissa = number_bits & ((1ll<<52) - 1);

    if (exponent == 0)
    {
        return;
    }
    if (exponent == 0x7FF)
    {
        throw std::overflow_error("Invalid exponent");
    }
    const uint64_t real48exp = exponent - 894;
    validate_range(real48exp);

    const uint64_t result =  sign << 47 |  ((mantissa >> 13) << 8) | (exponent - 894);
    std::memcpy(data, &result, 6);
}


math::Real48::operator float() const{
    if (Classify() == Class::ZERO)
    {
        return 0;
    }
    uint64_t number_bits{};
    std::memcpy(&number_bits, data, 6);
    const uint64_t sign = number_bits >> 47;
    const uint64_t exponent = number_bits & ((1 << 8) - 1);
    const uint64_t mantissa = number_bits >> 8 & ((1ULL<<39) - 1) ;

    if (exponent <= 1)
    {
        throw std::runtime_error("Invalid exponent");
    }

    const uint64_t result_bits = sign << 31 | (exponent - 2) << 23 | (mantissa >> 16);
    float result;
    std::memcpy(&result, &result_bits, sizeof(float));
    return result;
}

math::Real48::operator double() const noexcept{
    if (Classify() == Class::ZERO)
    {
        return 0;
    }
    uint64_t number_bits{};
    std::memcpy(&number_bits, data, 6);
    const uint64_t sign = number_bits >> 47;
    const uint64_t exponent = number_bits & ((1 << 8) - 1);
    const uint64_t mantissa = number_bits >> 8 & ((1ULL<<39) - 1) ;

    const uint64_t result_bits = sign << 63 | (exponent + 894) << 52 | mantissa << 13;
    double result;
    std::memcpy(&result, &result_bits, sizeof(double));
    return result;
}

math::Real48& math::Real48::operator+=(const Real48& b){
    const math::Real48 temp{*this + b};
    std::memcpy(data, &temp, 6);
    return *this;
}

math::Real48& math::Real48::operator-=(const Real48& b){
    const math::Real48 temp{*this - b};
    std::memcpy(data, &temp, 6);
    return *this;
}

math::Real48& math::Real48::operator*=(const Real48& b){
    const math::Real48 temp{*this * b};
    std::memcpy(data, &temp, 6);
    return *this;
}

math::Real48& math::Real48::operator/=(const Real48& b){
    const math::Real48 temp{*this / b};
    std::memcpy(data, &temp, 6);
    return *this;
}

math::Real48 math::Real48::operator+() const noexcept{
    return *this;
}

math::Real48 math::Real48::operator-() const noexcept{
    math::Real48 temp{*this};
    temp.data[5] ^= 1<<7;
    return temp;
}

math::Real48 math::Real48::operator+(const Real48& o) const{
    return static_cast<double>(*this) + static_cast<double>(o);

}

math::Real48 math::Real48::operator-(const Real48& o) const{
    return static_cast<double>(*this) - static_cast<double>(o);
}

math::Real48 math::Real48::operator*(const Real48& o) const{
    return static_cast<double>(*this) * static_cast<double>(o);
}

math::Real48 math::Real48::operator/(const Real48& o) const{
    return static_cast<double>(*this) / static_cast<double>(o);
}

bool math::Real48::operator>(const Real48& o) const noexcept{
    return static_cast<double>(*this) > static_cast<double>(o);
}

bool math::Real48::operator<(const Real48& o) const noexcept{
    return static_cast<double>(*this) < static_cast<double>(o);
}

math::Real48::Class math::Real48::Classify() const noexcept {
    for (const char i : data) {
        if (i != 0) {
            return Class::NORMAL;
        }
    }
    return Class::ZERO;
}

void math::Real48::validate_range(const uint64_t real48exp) {
    if (real48exp < 1 || real48exp > 255) {
        throw std::overflow_error("Invalid exponent");
    }
}
