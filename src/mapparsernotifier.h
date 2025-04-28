#pragma once

#include <variant>
#include <string>
#include <cstdint>
#include <iostream>
#include <iomanip>
#include <type_traits>
#include <list>
#include "iparsernotifier.h"
#include "typehelpers.h"

namespace jsbjson
{
    class MapParserNotifier : public IParserNotifier
    {
    public:
        void OnParsingStarted()
        {
        }

        void OnParsingFinished()
        {
        }

        void OnObjectBegin()
        {
        }

        void OnObjectFinished()
        {
        }

        void OnItemBegin()
        {
        };

        void OnItemName( const std::string& aName )
        {
            mItemName = aName;
        }

        void OnItemValueBegin( const IParserNotifier::eValueType aType )
        {
        }

        void OnItemValue( const std::variant<uint64_t, int64_t, double, bool, std::string> aValue )
        {
            std::visit( [ & ] (const auto& aItem)
                        {
                            if ( mInObject ) {}

                            if constexpr ( std::is_same_v<std::decay_t<decltype( aItem )>, std::string>) {}
                            else {}
                        }, aValue );
        }

        void OnArrayBegin()
        {
        }

        void OnNextItem()
        {
        }

        void OnArrayFinished()
        {
        }

        void OnError( const std::string& aErrorMessage )
        {
            std::cout << aErrorMessage << std::endl;
        };

    private:
        bool         mInObject = true;
        bool         mInArray  = false;
        std::string  mItemName;
        JsonElement  mRoot;
        JsonElement* mActualElement = &mRoot;

    private:
    };
}
