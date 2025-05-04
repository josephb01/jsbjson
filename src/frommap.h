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
#include "jsonelement.h"

namespace jsbjson
{
    class FromMap
    {
    private:
        std::string ProcessArray( const std::vector<JsonVariant>& aArray )
        {
            std::string lResult;
            size_t      lIndex = 0;

            for ( const auto& lItem : aArray ) {
                if ( std::holds_alternative<std::vector<JsonVariant>>( lItem.Value ) ) {
                    lResult += "[" + ProcessArray( std::get<std::vector<JsonVariant>>( lItem.Value ) ) + "]";
                }
                else if ( std::holds_alternative<JsonElement>( lItem.Value ) ) {
                    lResult += "{" + FromMap {}( std::get<JsonElement>( lItem.Value ), false ) + "}";
                }
                else {
                    std::visit( [ & ] (const auto& aValue)
                                {
                                    lResult += ToSimpleValue<std::decay_t<decltype( aValue )>> {}( aValue );
                                }, lItem.Value );
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

                if ( std::holds_alternative<JsonElement>( lValue.Value ) ) {
                    lResult += "{" + FromMap {}( std::get<JsonElement>( lValue.Value ), false ) + "}";
                }
                else if ( std::holds_alternative<std::vector<JsonVariant>>( lValue.Value ) ) {
                    lResult += "[" + ProcessArray( std::get<std::vector<JsonVariant>>( lValue.Value ) ) + "]";
                }
                else {
                    std::visit( [ & ] (const auto& aValue)
                                {
                                    lResult += ToSimpleValue<std::decay_t<decltype( aValue )>> {}( aValue );
                                }, lValue.Value );
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
