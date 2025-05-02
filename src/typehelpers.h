#pragma once

#include <vector>
#include <list>
#include <unordered_map>
#include <variant>
#include <cstdint>

namespace jsbjson
{
    template<typename>
    struct IsArray : std::false_type {};

    template<typename T, typename...A>
    struct IsArray<std::vector<T, A...>>: std::true_type {};

    template<typename T, typename...A>
    struct IsArray<std::list<T, A...>>: std::true_type {};

    template<class T, class = void>
    struct IsMember : std::false_type {};

    template<class T>
    struct IsMember<T, std::void_t<decltype( T::IsAJsonMember )>>: std::true_type {};

    template<class T, class = void>
    struct IsObject : std::false_type {};

    template<class T>
    struct IsObject<T, std::void_t<decltype( T::IsAJsonObject )>>: std::true_type {};

    template<class T, class = void>
    struct HasConvertRef : std::false_type {};

    template<class T>
    struct HasConvertRef<T, std::void_t<decltype( std::declval<T&>().ConvertRef() )>>: std::true_type {};

    struct JsonVariant;
    using JsonElement = std::unordered_map<std::string, JsonVariant>;
    struct sEmptyValue {};

    using JsonVariantRecursive = std::variant<sEmptyValue, int32_t, uint32_t, int64_t, uint64_t, std::string, bool, double, JsonElement, std::vector<JsonVariant>>;
    struct JsonVariant
    {
        JsonVariantRecursive Value;

        JsonVariant( const int32_t aValue )
            : Value( aValue )
        {}

        JsonVariant( const uint32_t aValue )
            : Value( aValue )
        {}

        JsonVariant( const int64_t aValue )
            : Value( aValue )
        {}

        JsonVariant( const uint64_t aValue )
            : Value( aValue )
        {}

        JsonVariant( const std::string& aValue )
            : Value( aValue )
        {}
        JsonVariant( const bool aValue )
            : Value( aValue )
        {}

        JsonVariant( const double aValue )
            : Value( aValue )
        {}

        JsonVariant( const JsonElement& aValue )
            : Value( aValue )
        {}

        JsonVariant( const std::vector<JsonVariant>& aValue )
            : Value( aValue )
        {}

        JsonVariant()
            : Value( sEmptyValue {} )
        {};

        bool IsArray()
        {
            return std::holds_alternative<std::vector<JsonVariant>>( Value );
        }

        template<typename T>
        std::optional<T> GetValue()
        {
            if ( !std::holds_alternative<std::decay_t<T>>( Value ) ) {
                return std::nullopt;
            }

            return std::get<std::decay_t<T>>( Value );
        }
    };
}
