#pragma once

#include <cstdint>
#include <type_traits>
#include <tuple>
#include <string>
#include <vector>
#include "bindings.h"
#include "jsonobject.h"

template<typename>
struct IsVector : std::false_type {};

template<typename T, typename...A>
struct IsVector<std::vector<T, A...>>: std::true_type {};

template<typename T>
struct ToSimpleValue {};

namespace jsbjson
{
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


    template<typename T>
    class ToJson
    {
        using Decayed_T = std::decay_t<T>;

        static constexpr bool IsComplexType()
        {
            return std::is_base_of_v<JsonMemberBase, Decayed_T>
                   || std::is_base_of_v<JsonObjectBase, Decayed_T>
                   || IsVector<Decayed_T>::value;
        }

        static std::string AppendComma( const size_t aItemCount,
                                        const size_t aCurrent )
        {
            if ( aCurrent < aItemCount ) {
                return ",";
            }

            return "";
        }

    public:
        std::string operator ()( const T&   aObject,
                                 const bool aIsRoot = true ) const
        {
            std::string lResult;

            if constexpr ( std::is_base_of_v<JsonObjectBase, Decayed_T>) {
                if ( aIsRoot ) {
                    lResult += "{";
                }

                if constexpr ( aObject.HasName() ) {
                    lResult += "\"" + std::string { aObject.Name() } + "\":";
                    lResult += "{";
                }

                std::apply( [ &lResult ] ( auto... aMembers )
                            {
                                size_t lIndex             = 0;
                                const size_t lMemberCount = sizeof...( aMembers );
                                ( ( lResult += ToJson<decltype( aMembers )> {}( aMembers, false ) + AppendComma( lMemberCount, ++lIndex ) ), ... );
                            }, aObject.Convert() );

                if constexpr ( aObject.HasName() ) {
                    lResult += "}";
                }

                if ( aIsRoot ) {
                    lResult += "}";
                }

                return lResult;
            }

            if constexpr ( std::is_base_of_v<JsonMemberBase, Decayed_T>) {
                lResult += "\"";
                lResult += std::string { aObject.Name } + "\":";
                lResult += ToJson<decltype( aObject.Value )> {}( aObject.Value, false );
                return lResult;
            }

            if constexpr ( IsVector<Decayed_T>::value ) {
                lResult      += "[";
                size_t lIndex = 0;

                for ( const auto& lItem : aObject ) {
                    lResult += ToJson<decltype( lItem )> {}( lItem, true );

                    if ( ++lIndex != aObject.size() ) {
                        lResult += ",";
                    }
                }

                lResult += "]";
                return lResult;
            }

            if constexpr ( !IsComplexType() ) {
                return ToSimpleValue<Decayed_T> {}( aObject );
            }
        }
    };
}
