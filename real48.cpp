#include "real48.hpp"
#include <limits>
#include <cstring>
#include <stdexcept>

namespace math {

static inline void pack_bytes(uint8_t out[6], uint64_t word) {
    out[0] = static_cast<uint8_t>(word & 0xFF);
    out[1] = static_cast<uint8_t>((word >> 8) & 0xFF);
    out[2] = static_cast<uint8_t>((word >> 16) & 0xFF);
    out[3] = static_cast<uint8_t>((word >> 24) & 0xFF);
    out[4] = static_cast<uint8_t>((word >> 32) & 0xFF);
    out[5] = static_cast<uint8_t>((word >> 40) & 0xFF);
}

Real48::Real48(const float number) {
    if (std::isnan(number) || std::isinf(number)) {
        throw std::overflow_error("Real48: cannot represent NaN or infinity");
    }

    uint32_t bits = 0;
    static_assert(sizeof(bits) == sizeof(number));
    std::memcpy(&bits, &number, sizeof(bits));
    bool sign = (bits >> 31) != 0;
    uint32_t exp_f = (bits >> 23) & 0xFF;
    uint32_t mant_f = bits & ((1u << 23) - 1);

    if (exp_f == 0) {
        std::memset(_b, 0, 6);
        return;
    }

    int32_t e48 = static_cast<int32_t>(exp_f) + 2;

    if (e48 <= 0) {
        throw std::overflow_error("Real48: too small (underflow) converting from float");
    }
    if (e48 > 255) {
        throw std::overflow_error("Real48: exponent overflow converting from float");
    }

    uint64_t f48 = uint64_t(mant_f) << (39 - 23);

    uint64_t word = 0;
    word |= (uint64_t)(e48 & 0xFF);
    word |= (f48 & MASK39) << 8;
    word |= (uint64_t)(sign ? 1ull : 0ull) << 47;
    pack_bytes(_b, word);
}

Real48::Real48(const double number) {
    if (std::isnan(number) || std::isinf(number)) {
        throw std::overflow_error("Real48: cannot represent NaN or infinity");
    }

    uint64_t bits = 0;
    static_assert(sizeof(bits) == sizeof(number));
    std::memcpy(&bits, &number, sizeof(bits));
    bool sign = (bits >> 63) != 0;
    uint32_t exp_d = static_cast<uint32_t>((bits >> 52) & 0x7FF);
    uint64_t mant_d = bits & ((uint64_t(1) << 52) - 1);

    if (exp_d == 0) {
        std::memset(_b, 0, 6);
        return;
    }

    int32_t e48 = static_cast<int32_t>(exp_d) - 894;

    if (e48 <= 0) {
        throw std::overflow_error("Real48: too small (underflow) converting from double");
    }
    if (e48 > 255) {
        throw std::overflow_error("Real48: exponent overflow converting from double");
    }

    const int shift = 52 - 39;
    uint64_t add = uint64_t(1) << (shift - 1);
    uint64_t f48 = (mant_d + add) >> shift;

    if (f48 == (uint64_t(1) << 39)) {
        f48 = 0;
        e48 += 1;
        if (e48 > 255) {
            throw std::overflow_error("Real48: exponent overflow after rounding");
        }
    }

    uint64_t word = 0;
    word |= (uint64_t)(e48 & 0xFF);
    word |= (f48 & MASK39) << 8;
    word |= (uint64_t)(sign ? 1ull : 0ull) << 47;
    pack_bytes(_b, word);
}

Real48::operator double() const noexcept {
    bool sign; uint32_t e48; uint64_t f48;
    unpack_parts(sign, e48, f48);
    if (e48 == 0) {
        return 0.0;
    }

    uint64_t exp_d = static_cast<uint64_t>(e48) - 129 + 1023;
    uint64_t mant_d = (f48 & MASK39) << (52 - 39);


    uint64_t bits = 0;
    bits |= (uint64_t)(sign ? 1ull : 0ull) << 63;
    bits |= (exp_d & 0x7FFull) << 52;
    bits |= mant_d & ((uint64_t(1) << 52) - 1);

    double out;
    std::memcpy(&out, &bits, sizeof(out));
    return out;
}

Real48::operator float() const {
    bool sign; uint32_t e48; uint64_t f48;
    unpack_parts(sign, e48, f48);
    if (e48 == 0) {
        return 0.0f;
    }

    int32_t exp_f = static_cast<int32_t>(e48) - 2;
    if (exp_f <= 0) {
        throw std::overflow_error("Real48: value too small for float (would be denormal/zero)");
    }
    if (exp_f >= 255) {
        throw std::overflow_error("Real48: exponent overflow converting to float");
    }

    const int shift = 39 - 23;
    uint64_t add = uint64_t(1) << (shift - 1);
    uint32_t mant_f = static_cast<uint32_t>((f48 + add) >> shift);

    if (mant_f == (1u << 23)) {
        mant_f = 0;
        ++exp_f;
        if (exp_f >= 255) {
            throw std::overflow_error("Real48: exponent overflow converting to float after rounding");
        }
    }

    uint32_t bits = 0;
    bits |= (sign ? 1u : 0u) << 31;
    bits |= (static_cast<uint32_t>(exp_f) & 0xFFu) << 23;
    bits |= mant_f & ((1u << 23) - 1);

    float out;
    std::memcpy(&out, &bits, sizeof(out));
    return out;
}

Real48& Real48::operator+=(const Real48& b) {
    double a_d = static_cast<double>(*this);
    double b_d = static_cast<double>(b);
    double res = a_d + b_d;
    Real48 r(res);
    *this = r;
    return *this;
}
Real48& Real48::operator-=(const Real48& b) {
    double a_d = static_cast<double>(*this);
    double b_d = static_cast<double>(b);
    double res = a_d - b_d;
    Real48 r(res);
    *this = r;
    return *this;
}
Real48& Real48::operator*=(const Real48& b) {
    double a_d = static_cast<double>(*this);
    double b_d = static_cast<double>(b);
    double res = a_d * b_d;
    Real48 r(res);
    *this = r;
    return *this;
}
Real48& Real48::operator/=(const Real48& b) {
    double a_d = static_cast<double>(*this);
    double b_d = static_cast<double>(b);
    double res = a_d / b_d;
    Real48 r(res);
    *this = r;
    return *this;
}

Real48 Real48::operator+() const noexcept {
    return *this;
}

Real48 Real48::operator-() const noexcept {
    Real48 r = *this;
    r._b[5] ^= 0x80;
    return r;
}

Real48 Real48::operator+(const Real48& o) const {
    Real48 r = *this;
    r += o;
    return r;
}
Real48 Real48::operator-(const Real48& o) const {
    Real48 r = *this;
    r -= o;
    return r;
}
Real48 Real48::operator*(const Real48& o) const {
    Real48 r = *this;
    r *= o;
    return r;
}
Real48 Real48::operator/(const Real48& o) const {
    Real48 r = *this;
    r /= o;
    return r;
}

bool Real48::operator>(const Real48& o) const noexcept {
    return static_cast<double>(*this) > static_cast<double>(o);
}
bool Real48::operator<(const Real48& o) const noexcept {
    return static_cast<double>(*this) < static_cast<double>(o);
}

} // namespace math
