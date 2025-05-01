#pragma once

#include <list>
#include <vector>
#include <optional>
#include <variant>
#include <string>
#include <any>
#include <algorithm>

#include "typehelpers.h"
#include "tovalue.h"

namespace jsbjson
{
    std::string ToValue( const std::any& aValue )
    {
        if ( aValue.type() == typeid( std::string ) ) {
            return ToSimpleValue<std::string> {}( std::any_cast<std::string>( aValue ) );
        }

        if ( aValue.type() == typeid( int64_t ) ) {
            return ToSimpleValue<int64_t> {}( std::any_cast<int64_t>( aValue ) );
        }

        if ( aValue.type() == typeid( uint64_t ) ) {
            return ToSimpleValue<uint64_t> {}( std::any_cast<uint64_t>( aValue ) );
        }

        if ( aValue.type() == typeid( int32_t ) ) {
            return ToSimpleValue<int32_t> {}( std::any_cast<int32_t>( aValue ) );
        }

        if ( aValue.type() == typeid( uint32_t ) ) {
            return ToSimpleValue<uint32_t> {}( std::any_cast<uint32_t>( aValue ) );
        }

        if ( aValue.type() == typeid( bool ) ) {
            return ToSimpleValue<bool> {}( std::any_cast<bool>( aValue ) );
        }

        return {};
    }

    template<size_t DEPTH, typename HEAD, typename...TAIL>
    bool IsValueAnArray( const std::any& aValue )
    {
        if constexpr ( sizeof...( TAIL ) == 0 ) {
            return aValue.type() == typeid( typename VectorDepth<HEAD, DEPTH>::type )
                   || aValue.type() == typeid( typename ListDepth<HEAD, DEPTH>::type );
        }
        else {
            return aValue.type() == typeid( typename VectorDepth<HEAD, DEPTH>::type )
                   || aValue.type() == typeid( typename ListDepth<HEAD, DEPTH>::type )
                   || IsValueAnArray<DEPTH, TAIL...>( aValue );
        }

        return false;
    }

    template<size_t DEPTH>
    std::tuple<bool, size_t> IsOneValueAnArray( const std::any& aValue )
    {
        if constexpr ( DEPTH > kMaxArrayDepth ) {
            return std::make_tuple( false, 0 );
        }

        if constexpr ( DEPTH == 0 ) {
            return std::make_tuple( IsValueAnArray<0, uint32_t, int32_t, int64_t, uint64_t, bool, std::string, double, JsonElement, std::any>( aValue ), DEPTH );
        }
        else {
            if ( IsValueAnArray<DEPTH, uint32_t, int32_t, int64_t, uint64_t, bool, std::string, double, JsonElement, std::any>( aValue ) ) {
                return std::make_tuple( true, DEPTH );
            }

            return IsOneValueAnArray<DEPTH - 1>( aValue );
        }
    }

    template<size_t DEPTH, typename HEAD, typename...TAIL>
    std::optional<Variant> ToArray( const std::any& aValue )
    {
        if constexpr ( sizeof...( TAIL ) == 0 ) {
            if ( aValue.type() == typeid( typename VectorDepth<HEAD, DEPTH>::type ) ) {
                return std::any_cast<typename VectorDepth<HEAD, DEPTH>::type>( aValue );
            }

            if ( aValue.type() == typeid( typename ListDepth<HEAD, DEPTH>::type ) ) {
                return std::any_cast<typename ListDepth<HEAD, DEPTH>::type>( aValue );
            }

            return std::nullopt;
        }
        else {
            if ( aValue.type() == typeid( typename VectorDepth<HEAD, DEPTH>::type ) ) {
                return std::any_cast<typename VectorDepth<HEAD, DEPTH>::type>( aValue );
            }

            if ( aValue.type() == typeid( typename ListDepth<HEAD, DEPTH>::type ) ) {
                return std::any_cast<typename ListDepth<HEAD, DEPTH>::type>( aValue );
            }

            return ToArray<DEPTH, TAIL...>( aValue );
        }
    }

    template<size_t DEPTH>
    std::optional<Variant> ToArrayOuter( const std::any& aValue )
    {
        if constexpr ( DEPTH > kMaxArrayDepth ) {
            return std::nullopt;
        }

        return ToArray<DEPTH, uint32_t, int32_t, int64_t, uint64_t, std::string, bool, double, JsonElement, std::any>( aValue );
    }

    template<typename TYPE>
    std::optional<TYPE> GetOne( const Variant& aVariant )
    {
        if ( std::holds_alternative<TYPE>( aVariant ) ) {
            return std::get<TYPE>( aVariant );
        }

        return std::nullopt;
    }

    template<size_t DEPTH, typename HEAD, typename... TAIL>
    auto GetArrayAsVector( const Variant& aVariant )
    {
        return std::make_tuple( GetOne<typename VectorDepth<HEAD, DEPTH>::type>( aVariant ), GetOne<VectorDepth<TAIL, DEPTH>::type>( aVariant ) ... );
    }

    template<size_t DEPTH, typename HEAD, typename... TAIL>
    auto GetArrayAsList( const Variant& aVariant )
    {
        return std::make_tuple( GetOne<typename ListDepth<HEAD, DEPTH>::type>( aVariant ), GetOne<typename ListDepth<TAIL, DEPTH>::type>( aVariant ) ... );
    }

    template<size_t DEPTH>
    auto GetOuterArrayAsVector( const Variant& aVariant )
    {
        return GetArrayAsVector<DEPTH, uint32_t, int32_t, int64_t, uint64_t, std::string, bool, double, JsonElement, std::any>( aVariant );
    }

    template<size_t DEPTH>
    auto GetOuterArrayAsList( const Variant& aVariant )
    {
        return GetArrayAsList<DEPTH, uint32_t, int32_t, int64_t, uint64_t, std::string, bool, double, JsonElement, std::any>( aVariant );
    }

    template<typename ARRAY>
    bool IsHomogen( const ARRAY& aArray )
    {
        if ( aArray.size() <= 1 ) {
            return true;
        }

        for ( size_t lIndex = 1; lIndex < aArray.size(); ++lIndex ) {
            if ( aArray[ lIndex - 1 ].type() != aArray[ lIndex ].type() ) {
                return false;
            }
        }

        return true;
    }

    template<size_t DEPTH, typename HEAD, typename...TAIL>
    bool AnyArrayIsA( const std::vector<std::any>& aValue )
    {
        if constexpr ( sizeof...( TAIL ) == 0 ) {
            return aValue[ 0 ].type() == typeid( typename AnyVectorDepth<HEAD, DEPTH>::type );
        }
        else {
            return aValue[ 0 ].type() == typeid( typename AnyVectorDepth<HEAD, DEPTH>::type )
                   || AnyArrayIsA<DEPTH, TAIL...>( aValue );
        }

        return false;
    }

    template<size_t DEPTH>
    std::tuple<bool, size_t> AnyArrayIsVector( const std::vector<std::any>& aValue )
    {
        if constexpr ( DEPTH > kMaxAnyArrayDepth ) {
            return std::make_tuple( false, 0 );
        }

        if ( aValue.empty() ) {
            return std::make_tuple( false, 0 );
        }

        if constexpr ( DEPTH == 0 ) {
            return std::make_tuple( AnyArrayIsA<0, uint32_t, int32_t, int64_t, uint64_t, bool, std::string, double, JsonElement>( aValue ), DEPTH );
        }
        else {
            if ( AnyArrayIsA<DEPTH, uint32_t, int32_t, int64_t, uint64_t, bool, std::string, double, JsonElement>( aValue ) ) {
                return std::make_tuple( true, DEPTH );
            }

            return AnyArrayIsVector<DEPTH - 1>( aValue );
        }
    }

    template<typename T>
    std::optional<std::vector<T>> ConvertToVector( const std::vector<std::any>& aFrom )
    {
        if ( !IsHomogen<std::vector<std::any>>( aFrom ) ) {
            return std::nullopt;
        }

        if ( aFrom.empty() ) {
            return std::nullopt;
        }

        if ( aFrom[ 0 ].type() != typeid( std::decay_t<T> ) ) {
            return std::nullopt;
        }

        std::vector<T> lResult;
        std::transform( aFrom.cbegin(), aFrom.cend(), std::back_inserter( lResult ), [] (const std::any& aValue)
                        {
                            return std::any_cast<T>( aValue );
                        } );

        return lResult;
    }
}
