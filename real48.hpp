#include <cstdint>
#include <cstring>
#include <cmath>
#include <stdexcept>

namespace math
{

class Real48
{
public:
    // constructors
    constexpr Real48();
    Real48(const float number);
    Real48(const double number);
    constexpr Real48(const Real48& o) = default;

    // conversion operators
    operator float() const;
    operator double() const noexcept;

    // assignment operators
    Real48& operator=(const Real48& b) noexcept = default;
    Real48& operator+=(const Real48& b);
    Real48& operator-=(const Real48& b);
    Real48& operator*=(const Real48& b);
    Real48& operator/=(const Real48& b);

    // arithmetic operators
    Real48 operator+() const noexcept;
    Real48 operator-() const noexcept;
    Real48 operator+(const Real48& o) const;
    Real48 operator-(const Real48& o) const;
    Real48 operator*(const Real48& o) const;
    Real48 operator/(const Real48& o) const;

    // comparison operators
    bool operator>(const Real48& o) const noexcept;
    bool operator<(const Real48& o) const noexcept;

    // classify
    enum class Class
    {
        NORMAL,
        ZERO
    };
    Class Classify() const noexcept;

    // limits
    static constexpr Real48 min();
    static constexpr Real48 max();
    static constexpr Real48 epsilon();

private:
    uint8_t _b[6];

    static constexpr uint64_t MASK39 = (uint64_t(1) << 39) - 1;

    constexpr explicit Real48(const uint8_t bytes[6]);
    static constexpr Real48 fromParts(bool sign, uint32_t exp, uint64_t mantissa);

    void unpack_parts(bool &sign, uint32_t &exp, uint64_t &mantissa) const noexcept;
};

constexpr Real48::Real48() : _b{0,0,0,0,0,0} {}

constexpr Real48::Real48(const uint8_t bytes[6]) : _b{bytes[0],bytes[1],bytes[2],bytes[3],bytes[4],bytes[5]} {}

constexpr Real48 Real48::fromParts(bool sign, uint32_t exp, uint64_t mantissa) {
    uint64_t word = 0;
    word |= (uint64_t)(exp & 0xFF);
    word |= (mantissa & MASK39) << 8;
    word |= (uint64_t)(sign ? 1ull : 0ull) << 47;
    uint8_t bytes[6] = {
        static_cast<uint8_t>(word & 0xFF),
        static_cast<uint8_t>((word >> 8) & 0xFF),
        static_cast<uint8_t>((word >> 16) & 0xFF),
        static_cast<uint8_t>((word >> 24) & 0xFF),
        static_cast<uint8_t>((word >> 32) & 0xFF),
        static_cast<uint8_t>((word >> 40) & 0xFF)
    };
    return Real48(bytes);
}

constexpr Real48 Real48::min() {
    return fromParts(false, 1u, 0u);
}

constexpr Real48 Real48::max() {
    return fromParts(false, 255u, MASK39);
}

constexpr Real48 Real48::epsilon() {
    return fromParts(false, 90u, 0u);
}

inline void Real48::unpack_parts(bool &sign, uint32_t &exp, uint64_t &mantissa) const noexcept {
    uint64_t word = 0;
    word |= uint64_t(_b[0]);
    word |= uint64_t(_b[1]) << 8;
    word |= uint64_t(_b[2]) << 16;
    word |= uint64_t(_b[3]) << 24;
    word |= uint64_t(_b[4]) << 32;
    word |= uint64_t(_b[5]) << 40;
    exp = static_cast<uint32_t>(word & 0xFF);
    mantissa = (word >> 8) & MASK39;
    sign = ((word >> 47) & 0x1) != 0;
}

inline Real48::Class Real48::Classify() const noexcept {
    bool s; uint32_t e; uint64_t m;
    unpack_parts(s,e,m);
    return (e == 0) ? Class::ZERO : Class::NORMAL;
}

} // namespace math
