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
        void OnParsingStarted()
        {
        }

        void OnParsingFinished()
        {
        }

        void OnObjectBegin( const size_t       aID,
                            const size_t       aParentID,
                            const std::string& aName )
        {
            std::cout << "#ObjectBegin, ID:" << aID << "; ParentID:" << aParentID << "#\r\n" << "{";
        }

        void OnObjectFinished( const size_t aID )
        {
            std::cout << "#ObjectFinished, ID" << aID << "#";
            std::cout << "}";
        }

        void OnItemBegin( const size_t aParentID )
        {
            std::cout << "#OnItemBegin, ParentID" << aParentID << "#";
        };

        void OnItemName( const std::string& aName,
                         const size_t       aParentID )
        {
            std::cout << std::quoted( aName ) << ":";
        }

        void OnItemValueBegin( const IParserNotifier::eValueType aType )
        {
        }

        void OnItemValue( const std::variant<uint64_t, int64_t, double, bool, std::string> aValue,
                          const size_t                                                     aParentID,
                          const std::string&                                               aName )
        {
            std::cout << "#OnItemValue, ParentID" << aParentID << "ItemName:" << aName << "#";
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

        virtual void OnArrayBegin( const size_t       aID,
                                   const size_t       aParentID,
                                   const std::string& aName )
        {
            std::cout << "#OnArrayBegin, ID:" << aID << "; ParentID:" << aParentID << " ArrayName:" << aName << "#\r\n" << "[";
        }

        virtual void OnNextItem()
        {
            std::cout << ",";
        }

        virtual void OnArrayFinished( const size_t aID )
        {
            std::cout << "#OnArrayFinished, ID" << aID << "#";
            std::cout << "]";
        }

        virtual void OnError( const std::string& aErrorMessage )
        {
            std::cout << "ERRORR!!!" << aErrorMessage << std::endl;
        };
    };
}
