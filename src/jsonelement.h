#pragma once

#include <optional>
#include <variant>
#include <unordered_map>
#include "typehelpers.h"

namespace jsbjson
{
    struct JsonVariant;
    using JsonElement = std::unordered_map<std::string, JsonVariant>;

    struct sEmptyValue {};

    using JsonVariantRecursive = std::variant<sEmptyValue, int32_t, uint32_t, int64_t, uint64_t, std::string, bool, double, JsonElement, std::vector<JsonVariant>>;
    struct JsonVariant
    {
    public:
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

        bool IsArray() const
        {
            return std::holds_alternative<std::vector<JsonVariant>>( Value );
        }

        template<typename T>
        bool IsA() const
        {
            if constexpr ( IsValidType<T>() ) {
                return std::holds_alternative<T>( Value );
            }
            else {
                return false;
            }
        }

        template<typename T>
        static constexpr bool IsValidType()
        {
            return std::is_same_v<T, sEmptyValue>
                   || std::is_same_v<T, int32_t>
                   || std::is_same_v<T, std::vector<JsonVariant>>
                   || std::is_same_v<T, uint32_t>
                   || std::is_same_v<T, int64_t>
                   || std::is_same_v<T, uint64_t>
                   || std::is_same_v<T, std::string>
                   || std::is_same_v<T, bool>
                   || std::is_same_v<T, double>
                   || std::is_same_v<T, JsonElement>;
        }

        template<typename T>
        std::optional<T> GetValue() const
        {
            if constexpr ( std::is_same_v<std::decay_t<T>, int64_t>) {
                if ( IsA<uint64_t>() ) {
                    const uint64_t lValue = GetValueRef<uint64_t>();

                    constexpr uint64_t lMaxInt64Value = static_cast<uint64_t>( std::numeric_limits<int64_t>::max() );

                    if ( lMaxInt64Value >= lValue ) {
                        return static_cast<int64_t>( lValue );
                    }
                }
            }

            if constexpr ( std::is_same_v<std::decay_t<T>, uint64_t>) {
                if ( IsA<int64_t>() ) {
                    const int64_t lValue = GetValueRef<int64_t>();

                    if ( lValue >= 0 ) {
                        return static_cast<uint64_t>( lValue );
                    }
                }
            }

            if ( !IsA<std::decay_t<T>>() ) {
                return std::nullopt;
            }

            if constexpr ( IsValidType<std::decay_t<T>>() ) {
                return std::get<std::decay_t<T>>( Value );
            }
            else {
                return std::nullopt;
            }
        }

        template<typename T>
        const T& GetValueRef() const
        {
            return const_cast<JsonVariant&>( *this ).GetValueRef<T>();
        }

        template<typename T>
        T& GetValueRef()
        {
            static T kEmpty {};

            if ( !IsA<std::decay_t<T>>() ) {
                return kEmpty;
            }

            if constexpr ( IsValidType<std::decay_t<T>>() ) {
                return std::get<std::decay_t<T>>( Value );
            }
            else {
                return kEmpty;
            }
        }

        JsonVariant& operator []( const std::string& aKey )
        {
            if ( std::holds_alternative<JsonElement>( Value ) ) {
                JsonVariant& lVariant = std::get<JsonElement>( Value )[ aKey ];
                return lVariant;
            }

            Value                 = JsonElement {};
            JsonVariant& lVariant = std::get<JsonElement>( Value )[ aKey ];
            return lVariant;
        }
    };
}
