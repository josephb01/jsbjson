#pragma once

#include "typehelpers.h"
#include "jsonelement.h"
#include "frommap.h"
#include "parser.h"
#include "mapparsernotifier.h"

namespace jsbjson
{
    using JsonElement_t = JsonElement;
    class JsonElementEx : public JsonElement_t
    {
    public:
        using JsonElement_t::JsonElement_t;

        std::string ToJson() const
        {
            return FromMap {}( *this );
        }

        bool FromJson( const std::string& aJsonString )
        {
            clear();
            jsbjson::JsonParser lParser;

            return lParser.Parse( aJsonString, std::make_shared<jsbjson::MapParserNotifier>( *this ) );
        }
    };
}
