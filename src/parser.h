#pragma once
#include <memory>
#include <string>
#include <list>
#include <optional>
#include <variant>
#include "iparsernotifier.h"

namespace jsbjson
{
    class JsonParser
    {
    public:
        bool Parse( const std::string&               aJsonDocument,
                    std::shared_ptr<IParserNotifier> aNotifier )
        {
            if ( aNotifier == nullptr ) {
                return false;
            }

            aNotifier->OnParsingStarted();

            for ( const auto& lChar : aJsonDocument ) {
                if ( !CallStateFunction( lChar, aNotifier ) ) {
                    return false;
                }
            }

            if ( mInfo.Parent.empty()
                 && ( mOpeningCurlyCount == 0 )
                 && ( mOpeningSquareCount == 0 ) )
            {
                aNotifier->OnParsingFinished();
                return true;
            }

            return false;
        }

    private:
        enum eParserState
        {
            Init
            , InObjectBegin
            , InItemName
            , InItemValueDelimiter
            , InItemValue
            , ParseStringValue
            , ParseBoolValue
            , ParseNumberValue
            , ParseValueFinish
        };

        struct ParentData final
        {
            enum eParent
            {
                Object
                , Array
                , Unknown
            };

            eParent Parent;
            size_t  ParentID = 0;
        };

        using eValueType = IParserNotifier::eValueType;
        struct ParseInfo final
        {
            std::string           Name;
            std::string           Value;
            std::list<ParentData> Parent;
            std::list<size_t>     IDs;
            eValueType            ValueType = eValueType::Unknown;
        };

    private:
        eParserState mState = eParserState::Init;
        ParseInfo    mInfo;
        size_t       mOpeningCurlyCount  = 0;
        size_t       mOpeningSquareCount = 0;
        size_t       mObjectID           = 0;

    private:
        size_t GetNextID()
        {
            return ++mObjectID;
        }

    private:
        bool CallStateFunction( const char                              aChar,
                                const std::shared_ptr<IParserNotifier>& aNotifier )
        {
            if ( mState == eParserState::Init ) {
                return DoInit( aChar, aNotifier );
            }

            if ( mState == eParserState::InObjectBegin ) {
                return DoInObjectBegin( aChar, aNotifier );
            }

            if ( mState == eParserState::InItemName ) {
                return DoInItemName( aChar, aNotifier );
            }

            if ( mState == eParserState::InItemValueDelimiter ) {
                return DoInItemValueDelimiter( aChar, aNotifier );
            }

            if ( mState == eParserState::InItemValue ) {
                return DoInItemValue( aChar, aNotifier );
            }

            if ( mState == eParserState::ParseStringValue ) {
                return DoParseStringValue( aChar, aNotifier );
            }

            if ( mState == eParserState::ParseBoolValue ) {
                return DoParseBoolValue( aChar, aNotifier );
            }

            if ( mState == eParserState::ParseNumberValue ) {
                return DoParseNumberValue( aChar, aNotifier );
            }

            if ( mState == eParserState::ParseValueFinish ) {
                return DoParseValueFinish( aChar, aNotifier );
            }

            return false;
        }

    private:
        bool DoInit( const char                              aChar,
                     const std::shared_ptr<IParserNotifier>& aNotifier )
        {
            if ( std::isspace( aChar ) ) {
                return true;
            }

            if ( aChar == '{' ) {
                const size_t lID = GetNextID();
                mInfo.IDs.push_back( lID );
                aNotifier->OnObjectBegin( lID, 0, "" );
                mInfo.Parent.push_back( { ParentData::eParent::Object, lID } );

                mOpeningCurlyCount++;
                mState = eParserState::InObjectBegin;
                return true;
            }

            aNotifier->OnError( "Invalid json" );
            return false;
        }

        bool DoInObjectBegin( const char                              aChar,
                              const std::shared_ptr<IParserNotifier>& aNotifier )
        {
            if ( mInfo.Parent.empty() ) {
                aNotifier->OnError( "No parent!" );
                return false;
            }

            if ( std::isspace( aChar ) ) {
                return true;
            }

            mInfo.Value              = {};
            mInfo.Name               = {};
            const ParentData lParent = mInfo.Parent.back();

            if ( lParent.Parent == ParentData::eParent::Object ) {
                if ( aChar == '\"' ) {
                    aNotifier->OnItemBegin( lParent.ParentID );
                    mState = eParserState::InItemName;
                    return true;
                }

                aNotifier->OnError( "Invalid char..." );
                return false;
            }

            mState = eParserState::InItemValue;
            aNotifier->OnItemBegin( lParent.ParentID );
            return DoInItemValue( aChar, aNotifier );
        }

        bool DoInItemName( const char                              aChar,
                           const std::shared_ptr<IParserNotifier>& aNotifier )
        {
            if ( aChar == '\"' ) {
                aNotifier->OnItemName( mInfo.Name, mInfo.Parent.back().ParentID );
                mState = eParserState::InItemValueDelimiter;
                return true;
            }

            mInfo.Name += aChar;
            return true;
        }

        bool DoInItemValueDelimiter( const char                              aChar,
                                     const std::shared_ptr<IParserNotifier>& aNotifier )
        {
            if ( std::isspace( aChar ) ) {
                return true;
            }

            if ( aChar == ':' ) {
                mState = eParserState::InItemValue;
                return true;
            }

            aNotifier->OnError( std::string( "Expected \":\"" ) );
            return false;
        }

        bool DoInItemValue( const char                              aChar,
                            const std::shared_ptr<IParserNotifier>& aNotifier )
        {
            if ( std::isspace( aChar ) ) {
                return true;
            }

            mInfo.Value = {};

            if ( aChar == '[' ) {
                const size_t lID = GetNextID();
                aNotifier->OnArrayBegin( lID, mInfo.Parent.back().ParentID, mInfo.Name );
                mInfo.Name = {};
                mInfo.Parent.push_back( { ParentData::eParent::Array, lID } );
                mInfo.IDs.push_back( lID );
                mOpeningSquareCount++;
                mState = eParserState::InItemValue;
                return true;
            }

            if ( aChar == '{' ) {
                const size_t lID = GetNextID();
                aNotifier->OnObjectBegin( lID, mInfo.Parent.back().ParentID, mInfo.Name );
                mInfo.Parent.push_back( { ParentData::eParent::Object, lID } );
                mInfo.IDs.push_back( lID );
                mOpeningCurlyCount++;
                mState = eParserState::InObjectBegin;
                return true;
            }

            if ( aChar == '\"' ) {
                mInfo.ValueType = eValueType::String;
                aNotifier->OnItemValueBegin( mInfo.ValueType );
                mState = eParserState::ParseStringValue;
                return true;
            }

            if ( ( std::toupper( aChar ) == 'T' )
                 || ( std::toupper( aChar ) == 'F' ) )
            {
                mInfo.ValueType = eValueType::Bool;
                mInfo.Value     = aChar;
                aNotifier->OnItemValueBegin( mInfo.ValueType );
                mState = eParserState::ParseBoolValue;
                return true;
            }

            if ( ( aChar == '+' )
                 || ( aChar == '-' )
                 || ( aChar == '.' )
                 || std::isdigit( aChar ) )
            {
                mInfo.ValueType = eValueType::Number;
                mInfo.Value     = aChar;
                aNotifier->OnItemValueBegin( mInfo.ValueType );
                mState = eParserState::ParseNumberValue;
                return true;
            }

            aNotifier->OnError( "Failed to parse..." );
            return false;
        }

        void NotifyItemValue( const std::shared_ptr<IParserNotifier>&                           aNotifier,
                              const std::variant<uint64_t, int64_t, double, bool, std::string>& aValue )
        {
            aNotifier->OnItemValue( aValue, mInfo.Parent.back().ParentID, mInfo.Name );
        }

        bool DoParseStringValue( const char                              aChar,
                                 const std::shared_ptr<IParserNotifier>& aNotifier )
        {
            if ( aChar == '\"' ) {
                NotifyItemValue( aNotifier, mInfo.Value );
                mState = eParserState::ParseValueFinish;
                return true;
            }

            mInfo.Value += aChar;
            return true;
        }

        bool DoParseBoolValue( const char                              aChar,
                               const std::shared_ptr<IParserNotifier>& aNotifier )
        {
            if ( ( mInfo.Value == "true" )
                 || ( mInfo.Value == "false" ) )
            {
                const bool lValue = mInfo.Value == "true"
                                    ? true
                                    : false;
                NotifyItemValue( aNotifier, lValue );
                mState = eParserState::ParseValueFinish;
                return DoParseValueFinish( aChar, aNotifier );
            }

            mInfo.Value += aChar;
            return true;
        }

        template<typename NUMBER>
        std::optional<std::variant<int64_t, uint64_t, double>> GetNumber( const std::string& aString )
        {
            NUMBER lNumber {};
            auto [ lPtr, lErrorCode ] = std::from_chars( aString.data(), aString.data() + aString.size(), lNumber );

            if ( ( lPtr == aString.data() + aString.size() )
                 && ( lErrorCode == std::errc() ) )
            {
                std::variant<int64_t, uint64_t, double> lVariant;
                lVariant.emplace<NUMBER>( lNumber );
                return lVariant;
            }

            return std::nullopt;
        }

        template<typename HEAD, typename... TAIL>
        std::optional<std::variant<int64_t, uint64_t, double>> ToNumber( const std::string& aString )
        {
            if constexpr ( sizeof...( TAIL ) == 0 ) {
                return GetNumber<HEAD>( aString );
            }
            else {
                auto lNumber = GetNumber<HEAD>( aString );

                if ( lNumber.has_value() ) {
                    return lNumber;
                }

                return ToNumber<TAIL...>( aString );
            }

            return std::nullopt;
        }

        bool DoParseNumberValue( const char                              aChar,
                                 const std::shared_ptr<IParserNotifier>& aNotifier )
        {
            if ( ( aChar == ',' )
                 || ( aChar == ']' )
                 || ( aChar == '}' )
                 || std::isspace( aChar ) )
            {
                std::optional<std::variant<int64_t, uint64_t, double>> lResult = ToNumber<uint64_t, int64_t, double>( mInfo.Value );

                if ( lResult.has_value() ) {
                    std::variant<uint64_t, int64_t, double, bool, std::string> lValue;

                    if ( lResult.has_value() ) {
                        std::visit( [ & ] (auto aValue)
                                    {
                                        lValue = aValue;
                                    }, lResult.value() );
                        NotifyItemValue( aNotifier, lValue );
                        mState = eParserState::ParseValueFinish;
                        return DoParseValueFinish( aChar, aNotifier );
                    }
                }

                aNotifier->OnError( "Value is not a number: " + mInfo.Value );
                return false;
            }

            mInfo.Value += aChar;
            return true;
        }

        bool DoParseValueFinish( const char                              aChar,
                                 const std::shared_ptr<IParserNotifier>& aNotifier )
        {
            if ( mInfo.Parent.empty() ) {
                return false;
            }

            const ParentData lParent = mInfo.Parent.back();

            if ( lParent.Parent == ParentData::eParent::Object ) {
                if ( aChar == ',' ) {
                    aNotifier->OnNextItem();
                    mState = eParserState::InObjectBegin;
                    return true;
                }

                if ( aChar == '}' ) {
                    if ( mOpeningCurlyCount == 0 ) {
                        return false;
                    }

                    mOpeningCurlyCount--;
                    mInfo.Parent.pop_back();
                    aNotifier->OnObjectFinished( mInfo.IDs.back() );
                    mInfo.IDs.pop_back();
                    return true;
                }

                return false;
            }

            if ( aChar == ',' ) {
                aNotifier->OnNextItem();
                mState = eParserState::InItemValue;
                return true;
            }

            if ( aChar == ']' ) {
                if ( mOpeningSquareCount == 0 ) {
                    return false;
                }

                aNotifier->OnArrayFinished( mInfo.IDs.back() );
                mOpeningSquareCount--;
                mInfo.Parent.pop_back();
                mInfo.IDs.pop_back();
                return true;
            }

            if ( aChar == '}' ) {
                if ( mOpeningCurlyCount == 0 ) {
                    return false;
                }

                mOpeningCurlyCount--;
                aNotifier->OnObjectFinished( mInfo.IDs.back() );
                mInfo.IDs.pop_back();
                return true;
            }

            return false;
        }
    }

    ;
}
