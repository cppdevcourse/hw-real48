#include "real48.hpp"

#include <cstring>
#include <stdexcept>
#include <cstdint>

namespace
{

template <class T>
bool test(const std::uint8_t* data)
{
    try
    {
        T input {};
        std::memcpy(&input, data, sizeof(input));
        math::Real48 r48 {input};
        return T{r48} == input;
    }
    catch (const std::overflow_error& err)
    {
    }
    return false;
}

}

extern "C" int
LLVMFuzzerTestOneInput(const std::uint8_t* data, std::size_t size)
{
    if (size < sizeof(float))
    {
    }
    else if (size < sizeof(double))
    {
        return test<float>(data);
    }
    else
    {
        return test<double>(data);
    }
    return 0;
}
