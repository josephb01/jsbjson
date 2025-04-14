#pragma once

#include <cstdint>
#include <type_traits>
#include <tuple>
#include <string>
#include <vector>
#include <list>
#include "bindings.h"
#include "jsonobject.h"

namespace jsbjson
{
    template<typename>
    struct IsArray : std::false_type {};

    template<typename T, typename...A>
    struct IsArray<std::vector<T, A...>>: std::true_type {};

    template<typename T, typename...A>
    struct IsArray<std::list<T, A...>>: std::true_type {};

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

    template<class T, class = void>
    struct IsMember : std::false_type {};

    template<class T>
    struct IsMember<T, std::void_t<decltype( T::IsAJsonMember )>>: std::true_type {};

    template<class T, class = void>
    struct IsObject : std::false_type {};

    template<class T>
    struct IsObject<T, std::void_t<decltype( T::IsAJsonObject )>>: std::true_type {};

    template<typename T>
    class ToJson
    {
        using Decayed_T = std::decay_t<T>;

        static constexpr bool IsComplexType()
        {
            return IsMember<Decayed_T>::value
                   || IsObject<Decayed_T>::value
                   || IsArray<Decayed_T>::value;
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
                                 const bool aIsRoot  = true,
                                 const bool aInArray = false ) const
        {
            std::string lResult;

            if ( aIsRoot ) {
                lResult += "{";
            }

            if constexpr ( IsObject<Decayed_T>::value ) {
                if ( !aIsRoot ) {
                    if ( !aInArray ) {
                        lResult += "\"" + std::string { aObject.Name() } + "\":";
                    }

                    lResult += "{";
                }

                std::apply( [ &lResult ] ( auto... aMembers )
                            {
                                size_t lIndex             = 0;
                                const size_t lMemberCount = sizeof...( aMembers );
                                ( ( lResult += ToJson<decltype( aMembers )> {}( aMembers, false ) + AppendComma( lMemberCount, ++lIndex ) ), ... );
                            }, aObject.Convert() );

                lResult += "}";

                return lResult;
            }

            if constexpr ( IsMember<Decayed_T>::value ) {
                lResult += "\"";
                lResult += std::string { aObject.Name } + "\":";
                lResult += ToJson<decltype( aObject.Value )> {}( aObject.Value, false );
                return lResult;
            }

            if constexpr ( IsArray<Decayed_T>::value ) {
                lResult      += "[";
                size_t lIndex = 0;

                for ( const auto& lItem : aObject ) {
                    lResult += ToJson<decltype( lItem )> {}( lItem, false, true );

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
