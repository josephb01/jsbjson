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
            using Variant = std::variant<std::vector<ARGS>..., std::list<ARGS>...>;
        };

        using Variant = sVariantHelper<int32_t, uint32_t, std::string, bool, double, JsonElement, std::any>::Variant;

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

        template<typename HEAD, typename...TAIL>
        bool IsValueAnArray( const std::any& aValue )
        {
            if constexpr ( sizeof...( TAIL ) == 0 ) {
                return aValue.type() == typeid( std::vector<HEAD> )
                       || aValue.type() == typeid( std::list<HEAD> );
            }
            else {
                return aValue.type() == typeid( std::vector<HEAD> )
                       || aValue.type() == typeid( std::list<HEAD> )
                       || IsValueAnArray<TAIL...>( aValue );
            }

            return false;
        }

        bool IsOneValueAnArray( const std::any& aValue )
        {
            return IsValueAnArray<int32_t, uint32_t, bool, std::string, double, JsonElement, std::any>( aValue );
        }

        template<typename HEAD, typename...TAIL>
        auto ToArray( const std::any& aValue )->std::optional<Variant>
        {
            if constexpr ( sizeof...( TAIL ) == 0 ) {
                if ( aValue.type() == typeid( std::vector<HEAD> ) ) {
                    return std::any_cast<std::vector<HEAD>>( aValue );
                }

                if ( aValue.type() == typeid( std::list<HEAD> ) ) {
                    return std::any_cast<std::list<HEAD>>( aValue );
                }

                return std::nullopt;
            }
            else {
                if ( aValue.type() == typeid( std::vector<HEAD> ) ) {
                    return std::any_cast<std::vector<HEAD>>( aValue );
                }

                if ( aValue.type() == typeid( std::list<HEAD> ) ) {
                    return std::any_cast<std::list<HEAD>>( aValue );
                }

                return ToArray<TAIL...>( aValue );
            }
        }

        auto ToArrayOuter( const std::any& aValue )->std::optional<Variant>
        {
            return ToArray<int32_t, uint32_t, std::string, bool, double, JsonElement, std::any>( aValue );
        }

        template<typename T>
        using OptionalList = std::optional<std::list<T>>;

        template<typename TYPE>
        OptionalList<TYPE> GetOneAsList( const Variant& aVariant )
        {
            if ( std::holds_alternative<std::list<TYPE>>( aVariant ) ) {
                return std::get<std::list<TYPE>>( aVariant );
            }

            return std::nullopt;
        }

        template<typename HEAD, typename... TAIL>
        auto GetArrayAsList( const Variant& aVariant )->std::tuple<OptionalList<HEAD>, OptionalList<TAIL>...>
        {
            return std::make_tuple( GetOneAsList<HEAD>( aVariant ), GetOneAsList<TAIL>( aVariant ) ... );
        }

        template<typename T>
        using OptionalVector = std::optional<std::vector<T>>;

        template<typename TYPE>
        OptionalVector<TYPE> GetOneAsVector( const Variant& aVariant )
        {
            if ( std::holds_alternative<std::vector<TYPE>>( aVariant ) ) {
                return std::get<std::vector<TYPE>>( aVariant );
            }

            return std::nullopt;
        }

        template<typename HEAD, typename... TAIL>
        auto GetArrayAsVector( const Variant& aVariant )->std::tuple<OptionalVector<HEAD>, OptionalVector<TAIL>...>
        {
            return std::make_tuple( GetOneAsVector<HEAD>( aVariant ), GetOneAsVector<TAIL>( aVariant ) ... );
        }

        auto GetOuterArrayAsVector( const Variant& aVariant )
        {
            return GetArrayAsVector<int32_t, uint32_t, std::string, bool, double, JsonElement, std::any>( aVariant );
        }

        auto GetOuterArrayAsList( const Variant& aVariant )
        {
            return GetArrayAsList<int32_t, uint32_t, std::string, bool, double, JsonElement, std::any>( aVariant );
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
                    if ( IsOneValueAnArray( lItem ) ) {
                        std::optional<Variant> lArrayOpt = ToArrayOuter( lItem );

                        if ( lArrayOpt.has_value() ) {
                            const auto lArrayTuple = GetOuterArrayAsVector( lArrayOpt.value() );
                            bool       lSuccess    = false;
                            {
                                VisitTuple( lArrayTuple, [ &lSuccess, &lResult, this ] (const auto& aValue)
                                            {
                                                if ( aValue.has_value() ) {
                                                    lSuccess = true;
                                                    lResult += "[" + ProcessArray( aValue.value() ) + "]";
                                                }
                                            } );

                                if ( lSuccess ) {
                                    continue;
                                }
                            }
                            {
                                const auto lArrayTuple = GetOuterArrayAsList( lArrayOpt.value() );

                                VisitTuple( lArrayTuple, [ &lSuccess, &lResult, this ] (const auto& aValue)
                                            {
                                                if ( aValue.has_value() ) {
                                                    lSuccess = true;
                                                    lResult += "[" + ProcessArray( aValue.value() ) + "]";
                                                }
                                            } );
                            }
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
                lResult += "\"" + lKey + "\":";

                if ( IsOneValueAnArray( lValue ) ) {
                    std::optional<Variant> lArrayOpt = ToArrayOuter( lValue );

                    if ( lArrayOpt.has_value() ) {
                        const auto lArrayTuple = GetOuterArrayAsVector( lArrayOpt.value() );
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
                            const auto lArrayTuple = GetOuterArrayAsList( lArrayOpt.value() );

                            VisitTuple( lArrayTuple, [ &lSuccess, &lResult, this ] (const auto& aValue)
                                        {
                                            if ( aValue.has_value() ) {
                                                lSuccess = true;
                                                lResult += "[" + ProcessArray( aValue.value() ) + "]";
                                            }
                                        } );
                        }
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
    }

    ;
}
