#pragma once
#include <variant>
#include <string>
#include <cstdint>
#include <iostream>
#include <iomanip>
#include <type_traits>
#include "iparsernotifier.h"

namespace jsbjson
{
    class TestParserNotifier : public IParserNotifier
    {
    public:
        enum eValueType
        {
            String
            , Bool
            , Number
            , Array
            , Object
            , Unknown
        };

    public:
        void OnObjectBegin()
        {
            std::cout << "{";
        }

        void OnObjectFinished()
        {
            std::cout << "}";
        }

        void OnItemBegin()
        {
        };

        void OnItemName( const std::string& aName )
        {
            std::cout << std::quoted( aName ) << ":";
        }

        void OnItemValueBegin( const IParserNotifier::eValueType aType )
        {
        }

        void OnItemValue( const std::variant<uint64_t, int64_t, double, bool, std::string> aValue )
        {
            std::visit( [] (const auto& aItem)
                        {
                            if constexpr ( std::is_same_v<std::decay_t<decltype( aItem )>, std::string>) {
                                std::cout << std::quoted( aItem );
                            }
                            else {
                                std::cout << aItem;
                            }
                        }, aValue );
        }

        virtual void OnArrayBegin()
        {
            std::cout << "[";
        }

        virtual void OnNextItem()
        {
            std::cout << ",";
        }

        virtual void OnArrayFinished()
        {
            std::cout << "]";
        }

        virtual void OnError( const std::string& aErrorMessage )
        {
            std::cout << "ERRORR!!!" << aErrorMessage << std::endl;
        };
    };
}
