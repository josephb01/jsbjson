#pragma once

#include <string>

namespace jsbjson
{
    template<typename T>
    struct ToSimpleValue {};

    template<>
    struct ToSimpleValue<int8_t>
    {
        std::string operator ()( const int8_t aVal )
        {
            return std::to_string( aVal );
        }
    };

    template<>
    struct ToSimpleValue<int16_t>
    {
        std::string operator ()( const int16_t aVal )
        {
            return std::to_string( aVal );
        }
    };

    template<>
    struct ToSimpleValue<int32_t>
    {
        std::string operator ()( const int32_t aVal )
        {
            return std::to_string( aVal );
        }
    };

    template<>
    struct ToSimpleValue<int64_t>
    {
        std::string operator ()( const int64_t aVal )
        {
            return std::to_string( aVal );
        }
    };

    template<>
    struct ToSimpleValue<uint8_t>
    {
        std::string operator ()( const int8_t aVal )
        {
            return std::to_string( aVal );
        }
    };

    template<>
    struct ToSimpleValue<uint16_t>
    {
        std::string operator ()( const uint16_t aVal )
        {
            return std::to_string( aVal );
        }
    };

    template<>
    struct ToSimpleValue<uint32_t>
    {
        std::string operator ()( const uint32_t aVal )
        {
            return std::to_string( aVal );
        }
    };

    template<>
    struct ToSimpleValue<uint64_t>
    {
        std::string operator ()( const uint64_t aVal )
        {
            return std::to_string( aVal );
        }
    };

    template<>
    struct ToSimpleValue<double>
    {
        std::string operator ()( const double aVal )
        {
            return std::to_string( aVal );
        }
    };

    template<>
    struct ToSimpleValue<bool>
    {
        std::string operator ()( const bool aVal )
        {
            return aVal
                   ? std::string { "true" }
                   : std::string { "false" };
        }
    };

    template<>
    struct ToSimpleValue<std::string>
    {
        std::string operator ()( const std::string aVal )
        {
            return "\"" + aVal + "\"";
        }
    };
}
