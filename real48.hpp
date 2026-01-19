#include <cstdint>

namespace math
{

class Real48
{
public:
    constexpr Real48() : data_ {0, 0, 0} {};
    Real48(const float number);
    Real48(const double number);
    constexpr Real48(const Real48& o) = default;

    operator float() const;
    operator double() const noexcept;

    Real48& operator=(const Real48& b) noexcept = default;
    Real48& operator+=(const Real48& b);
    Real48& operator-=(const Real48& b);
    Real48& operator*=(const Real48& b);
    Real48& operator/=(const Real48& b);

    Real48 operator+() const noexcept;
    Real48 operator-() const noexcept;
    Real48 operator+(const Real48& o) const;
    Real48 operator-(const Real48& o) const;
    Real48 operator*(const Real48& o) const;
    Real48 operator/(const Real48& o) const;

    bool operator>(const Real48& o) const noexcept;
    bool operator<(const Real48& o) const noexcept;

    enum class Class
    {
        NORMAL,
        ZERO
    };
    Class Classify() const noexcept;

    consteval static Real48 min()
    {
        auto result = Real48();
        result.data_.exponent = 1;
        result.data_.mantissa = 0;
        result.data_.sign = 0;
        return result;
    }

    consteval static Real48 max()
    {
        auto result = Real48();
        result.data_.exponent = 255;
        // NOTE: 39 '1' bits
        result.data_.mantissa = 0x7FFFFFFFFFULL;
        result.data_.sign = 0;
        return result;
    }

    consteval static Real48 epsilon()
    {
        auto result = Real48();
        result.data_.exponent = 129 - 39;
        result.data_.mantissa = 0;
        result.data_.sign = 0;
        return result;
    }

private:
#pragma pack(push, 1)
    struct DataPack
    {
        uint8_t exponent  : 8;
        uint64_t mantissa : 39;
        uint8_t sign      : 1;
    };
#pragma pack(pop)

private:
    DataPack data_;
};

} // namespace math
