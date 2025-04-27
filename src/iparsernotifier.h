#pragma once
#include <variant>
#include <string>
#include <cstdint>

namespace jsbjson
{
    class IParserNotifier
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
        virtual void OnObjectBegin()                                                                        = 0;
        virtual void OnObjectFinished()                                                                     = 0;
        virtual void OnItemBegin()                                                                          = 0;
        virtual void OnItemName( const std::string& aName )                                                 = 0;
        virtual void OnItemValueBegin( const eValueType aType )                                             = 0;
        virtual void OnItemValue( const std::variant<uint64_t, int64_t, double, bool, std::string> aValue ) = 0;
        virtual void OnArrayBegin()                                                                         = 0;
        virtual void OnNextItem()                                                                           = 0;
        virtual void OnArrayFinished()                                                                      = 0;
        virtual void OnError( const std::string& aErrorMessage )                                            = 0;
    };
}
