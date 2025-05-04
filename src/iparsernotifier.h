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
        virtual void OnParsingStarted() = 0;
        virtual void OnObjectBegin( const size_t       aID,
                                    const size_t       aParentID,
                                    const std::string& aName ) = 0;
        virtual void OnObjectFinished( const size_t aID )      = 0;
        virtual void OnItemBegin( const size_t aParentID )     = 0;
        virtual void OnItemName( const std::string& aName,
                                 const size_t       aParentID ) = 0;
        virtual void OnItemValueBegin( const eValueType aType ) = 0;
        virtual void OnItemValue( const std::variant<uint64_t, int64_t, double, bool, std::string> aValue,
                                  const size_t                                                     aParentID,
                                  const std::string&                                               aName ) = 0;
        virtual void OnArrayBegin( const size_t       aID,
                                   const size_t       aParentID,
                                   const std::string& aName ) = 0;
        virtual void OnNextItem()                             = 0;
        virtual void OnArrayFinished( const size_t aID )      = 0;
        virtual void OnParsingFinished()                      = 0;
        virtual void OnError( const std::string& aError )     = 0;
    };
}
