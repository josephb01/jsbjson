#pragma once

#include <unordered_map>
#include <any>
#include <string>
#include <variant>
#include <optional>
#include <vector>
#include <list>
#include "tovalue.h"
#include "typehelpers.h"
#include "tupleutils.h"

namespace jsbjson
{
    using JsonElement = std::unordered_map<std::string, std::any>;

    class FromMap
    {
    private:
        template<typename... ARGS>
        struct sVariantHelper
        {
            using Variant = std::variant<typename VectorDepth<ARGS, 0>::type..., typename ListDepth<ARGS, 0>::type..., typename VectorDepth<ARGS, 1>::type..., typename ListDepth<ARGS, 1>::type..., typename VectorDepth<ARGS, 2>::type..., typename ListDepth<ARGS, 2>::type...>;
        };

        using Variant = sVariantHelper<int32_t, uint32_t, std::string, bool, double, JsonElement, std::any>::Variant;

    private:
        static constexpr const size_t kMaxArrayDepth = 2;                                                                                  // Arrays only can be maximum 3 dimensional (std::vector<std::vector<std::vector<int32_t>>>)

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
                return std::make_tuple( IsValueAnArray<0, int32_t, uint32_t, bool, std::string, double, JsonElement, std::any>( aValue ), DEPTH );
            }
            else {
                if ( IsValueAnArray<DEPTH, int32_t, uint32_t, bool, std::string, double, JsonElement, std::any>( aValue ) ) {
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

            return ToArray<DEPTH, int32_t, uint32_t, std::string, bool, double, JsonElement, std::any>( aValue );
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
            return GetArrayAsVector<DEPTH, int32_t, uint32_t, std::string, bool, double, JsonElement, std::any>( aVariant );
        }

        template<size_t DEPTH>
        auto GetOuterArrayAsList( const Variant& aVariant )
        {
            return GetArrayAsList<DEPTH, int32_t, uint32_t, std::string, bool, double, JsonElement, std::any>( aVariant );
        }

        template<typename ARRAY>
        std::string ProcessArray( const ARRAY& aArray )
        {
            std::string lResult;
            size_t      lIndex = 0;

            for ( const auto& lItem : aArray ) {
                using Decayed_t = std::decay_t<decltype( lItem )>;

                if constexpr ( IsArray<Decayed_t>::value ) {
                    lResult += "[" + ProcessArray( lItem ) + "]";
                }

                if constexpr ( std::is_same_v<Decayed_t, std::any>) {
                    std::tuple<bool, size_t> lIsArray = IsOneValueAnArray<2>( lItem );

                    if ( std::get<bool>( lIsArray ) ) {
                        size_t                     lDepth     = std::get<size_t>( lIsArray );
                        std::optional<std::string> lOptResult = ExtractArray( lItem, lDepth );

                        if ( lOptResult.has_value() ) {
                            lResult += lOptResult.value();
                        }
                    }
                    else {
                        lResult += ToValue( lItem );
                    }
                }

                if constexpr ( std::is_same_v<Decayed_t, JsonElement>) {
                    lResult += "{" + FromMap {}( lItem, false ) + "}";
                }

                if constexpr ( !IsArray<Decayed_t>::value
                               && !std::is_same_v<Decayed_t, std::any>
                               && !std::is_same_v<Decayed_t, JsonElement>)
                {
                    lResult += ToSimpleValue<Decayed_t> {}( lItem );
                }

                ++lIndex;

                if ( lIndex != aArray.size() ) {
                    lResult += ",";
                }
            }

            return lResult;
        }

        template<size_t Depth>
        std::optional<std::string> ExtractArrayByDepth( const std::optional<Variant>& aArrayOpt )
        {
            std::string lResult;

            if ( !aArrayOpt.has_value() ) {
                return std::nullopt;
            }

            const auto lArrayTuple = GetOuterArrayAsVector<Depth>( aArrayOpt.value() );
            bool       lSuccess    = false;
            {
                VisitTuple( lArrayTuple, [ &lSuccess, &lResult, this ] (const auto& aValue)
                            {
                                if ( aValue.has_value() ) {
                                    lSuccess = true;
                                    lResult += "[" + ProcessArray( aValue.value() ) + "]";
                                }
                            } );
            }

            if ( !lSuccess ) {
                const auto lArrayTuple = GetOuterArrayAsList<Depth>( aArrayOpt.value() );

                VisitTuple( lArrayTuple, [ &lSuccess, &lResult, this ] (const auto& aValue)
                            {
                                if ( aValue.has_value() ) {
                                    lSuccess = true;
                                    lResult += "[" + ProcessArray( aValue.value() ) + "]";
                                }
                            } );
            }

            if ( !lSuccess ) {
                return std::nullopt;
            }

            return lResult;
        }

        std::optional<std::string> ExtractArray( const std::any& lItem,
                                                 const size_t    aDepth )
        {
            std::string            lResult;
            std::optional<Variant> lArrayOpt;

            if ( aDepth == 0 ) {
                lArrayOpt = ToArrayOuter<0>( lItem );
            }

            if ( aDepth == 1 ) {
                lArrayOpt = ToArrayOuter<1>( lItem );
            }

            if ( aDepth == 2 ) {
                lArrayOpt = ToArrayOuter<2>( lItem );
            }

            if ( aDepth == 0 ) {
                return ExtractArrayByDepth<0>( lArrayOpt );
            }

            if ( aDepth == 1 ) {
                return ExtractArrayByDepth<1>( lArrayOpt );
            }

            if ( aDepth == 2 ) {
                return ExtractArrayByDepth<1>( lArrayOpt );
            }

            return std::nullopt;
        }

    public:
        std::string operator ()( const JsonElement& aElement,
                                 bool               aIsRoot = true )
        {
            std::string lResult = "";

            if ( aIsRoot ) {
                lResult += "{";
            }

            size_t lIndex = 0;

            for ( const auto& [ lKey, lValue ] : aElement ) {
                lResult                          += "\"" + lKey + "\":";
                std::tuple<bool, size_t> lIsArray = IsOneValueAnArray<2>( lValue );

                if ( std::get<bool>( lIsArray ) ) {
                    size_t                     lDepth     = std::get<size_t>( lIsArray );
                    std::optional<std::string> lOptResult = ExtractArray( lValue, lDepth );

                    if ( lOptResult.has_value() ) {
                        lResult += lOptResult.value();
                    }
                }

                if ( lValue.type() == typeid( JsonElement ) ) {
                    lResult += "{" + FromMap {}( std::any_cast<JsonElement>( lValue ), false ) + "}";
                }
                else {
                    lResult += ToValue( lValue );
                }

                ++lIndex;

                if ( lIndex != aElement.size() ) {
                    lResult += ",";
                }
            }

            if ( aIsRoot ) {
                lResult += "}";
            }

            return lResult;
        }
    };
}
