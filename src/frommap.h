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
#include "typeutils.h"

namespace jsbjson
{
    class FromMap
    {
    private:
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
