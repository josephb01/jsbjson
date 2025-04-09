#pragma once

#include <type_traits>
#include <tuple>
#include <string>
#include <vector>
#include "bindings.h"
#include "jsonobject.h"

template<typename>
struct is_vector : std::false_type {};

template<typename T, typename...A>
struct is_vector<std::vector<T, A...>>: std::true_type {};

template<typename T>
struct ToSimpleValue {};

template<>
struct ToSimpleValue<int>
{
    std::string operator ()( const int aVal )
    {
        return std::to_string( aVal );
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
struct Test
{
    std::string operator ()( const T& a )
    {
        std::cout << "Test" << std::endl;
        return "x";
    }
};

template<typename T>
struct ToJson
{
    static constexpr bool IsComplexType()
    {
        return std::is_base_of_v<JsonMemberBase, T>
               || std::is_base_of_v<JsonObjectBase, T>
               || is_vector<T>::value;
    }

    static std::string AppendComma( const size_t aItemCount,
                                    const size_t aCurrent )
    {
        if ( aCurrent < aItemCount ) {
            return ",";
        }

        return "";
    }

    std::string operator ()( const T& aObject,
                             bool     aIsRoot = false ) const
    {
        std::string lResult;

        if constexpr ( std::is_base_of_v<JsonObjectBase, T>) {
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
                            ( ( lResult += ToJson<decltype( aMembers )> {}( aMembers ) + AppendComma( lMemberCount, ++lIndex ) ), ... );
                        }, aObject.Convert() );

            if constexpr ( aObject.HasName() ) {
                lResult += "}";
            }

            if ( aIsRoot ) {
                lResult += "}";
            }

            return lResult;
        }

        if constexpr ( std::is_base_of_v<JsonMemberBase, T>) {
            lResult += "\"";
            lResult += std::string { aObject.Name } + "\":";
            lResult += ToJson<decltype( aObject.Value )> {}( aObject.Value );
            return lResult;
        }

        if constexpr ( is_vector<T>::value ) {
            lResult      += "[";
            size_t lIndex = 0;

            for ( const auto& lItem : aObject ) {
                lResult += ToJson<decltype( lItem )>;

                if ( ++lIndex != aObject.size() ) {
                    lResult += ",";
                }
            }

            lResult += "]";
            return lResult;
        }

        if constexpr ( !IsComplexType() ) {
            return ToSimpleValue<std::decay_t<T>> {}( aObject );
        }
    }
};
