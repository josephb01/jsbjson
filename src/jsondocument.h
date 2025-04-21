#pragma once
#include <variant>
#include <string>
#include <map>
#include <vector>
#include <optional>
#include <list>
#include <charconv>
#include <any>

namespace jsbjson
{
    struct JsonObject
    {
        using ArrayType = std::vector<std::any>;
        using ValueType = std::variant<int64_t, uint64_t, bool, std::string, double, JsonObject, ArrayType>;
        using DictType  = std::map<std::string, ValueType>;

        struct Empty {};

        bool IsArray() const
        {
            return std::holds_alternative<ArrayType>( Value );
        }

        bool IsEmpty() const
        {
            return std::holds_alternative<Empty>( Value );
        }

        template<typename T>
        std::optional<T> GetOpt( const std::string& aKey ) const
        {
            using Decayed_t = std::decay_t<T>;

            if constexpr ( std::is_same_v<Decayed_t, ArrayType>) {
                DictType lDict = std::get<DictType>( Value );

                if ( lDict.count( aKey ) == 0 ) {
                    return std::nullopt;
                }

                const ValueType& lValue = lDict[ aKey ];

                if ( std::holds_alternative<ArrayType>( lValue ) ) {
                    const ArrayType& lArray = std::get<ArrayType>( lValue );

                    return lArray;
                }
            }
            else {
                DictType lDict = std::get<DictType>( Value );

                if ( lDict.count( aKey ) == 0 ) {
                    return std::nullopt;
                }

                const ValueType& lValue = lDict[ aKey ];

                if constexpr ( std::is_same_v<Decayed_t, uint64_t>) {
                    if ( std::holds_alternative<int64_t>( lValue ) ) {
                        int64_t lItemValue = std::get<int64_t>( lValue );

                        if ( lItemValue >= 0 ) {
                            return lItemValue;
                        }
                    }
                }

                if constexpr ( std::is_same_v<Decayed_t, int64_t>) {
                    if ( std::holds_alternative<uint64_t>( lValue ) ) {
                        uint64_t           lItemValue     = std::get<uint64_t>( lValue );
                        constexpr uint64_t lMaxInt64Value = static_cast<uint64_t>( std::numeric_limits<int64_t>::max() );

                        if ( lMaxInt64Value > lItemValue ) {
                            return lItemValue;
                        }
                    }
                }

                if ( std::holds_alternative<Decayed_t>( lValue ) ) {
                    const T& lObject = std::get<Decayed_t>( lValue );

                    return lObject;
                }
            }

            return std::nullopt;
        }

        template<typename T>
        T Get( const std::string& aKey )
        {
            std::optional<T> lValue = GetOpt<T>( aKey );
            return lValue.value_or( T {} );
        }

        size_t Size() const
        {
            if ( IsArray() ) {
                return std::get<ArrayType>( Value ).size();
            }

            return std::get<DictType>( Value ).size();
        }

        std::variant<Empty, ArrayType, DictType> Value = Empty {};
    };

    class JsonDocument
    {
    private:
        enum class ParseState
        {
            Init
            , InObject
            , InArrayObject
            , InArrayObjectFinish
            , InObjectName
            , InObjectValueDelimiter
            , InObjectValueParseBegin
            , InObjectValueParseString
            , InObjectValueParseNumber
            , InObjectValueParseBool
            , InObjectValueParseArray
            , InObjectValueParseFinish
            , InObjectFinish
        };

        ParseState mState = ParseState::Init;

    private:
        struct ParsedElement final
        {
            std::string Name;
            std::string Value;
        };

        struct Parent final
        {
            using Parent_t = std::variant<JsonObject*, JsonObject::ArrayType*>;
            Parent_t Value;

            bool IsArray() const
            {
                return std::holds_alternative<JsonObject::ArrayType*>( Value );
            }

            JsonObject::ArrayType& GetArray()
            {
                return *std::get<JsonObject::ArrayType*>( Value );
            }

            JsonObject& GetObject()
            {
                return *std::get<JsonObject*>( Value );
            }

            template<typename T>
            constexpr Parent( T* const aParent )
                : Value( aParent )
            {}
        };

        ParsedElement     mParsedElement;
        std::list<Parent> mParents;
        size_t            mOpeningCurlyCount  = 0;
        size_t            mOpeningSquareCount = 0;

    public:
        JsonObject Root;

    private:
        bool ParseInit( const char aChar )
        {
            if ( std::isspace( aChar ) ) {
                return true;
            }

            if ( aChar == '{' ) {
                mOpeningCurlyCount++;
                mState     = ParseState::InObject;
                Root.Value = JsonObject::DictType {};
                mParents.push_back( &Root );
                return true;
            }

            return false;
        }

        bool ParseInObject( const char aChar )
        {
            if ( std::isspace( aChar ) ) {
                return true;
            }

            if ( aChar == '{' ) {
                if ( mParents.empty()
                     || !mParents.back().IsArray() )
                {
                    return false;
                }

                mOpeningCurlyCount++;
                JsonObject::ArrayType& lArray = mParents.back().GetArray();
                JsonObject             lNewObject;
                lNewObject.Value = JsonObject::DictType {};
                lArray.push_back( lNewObject );
                std::any& lObjectAny = lArray.back();
                auto&     lObject    = std::any_cast<JsonObject&>( lObjectAny );
                mParents.push_back( &lObject );
                mState = ParseState::InArrayObject;
                return true;
            }

            if ( mParents.back().IsArray() ) {
                return ParseValueBegin( aChar );
            }

            if ( aChar == '\"' ) {
                mState              = ParseState::InObjectName;
                mParsedElement.Name = "";
                return true;
            }

            return false;
        }

        bool ParseInObjectName( const char aChar )
        {
            if ( aChar == '\"' ) {
                mState = ParseState::InObjectValueDelimiter;
                return true;
            }

            mParsedElement.Name += aChar;
            return true;
        }

        bool ParseValueDelimiter( const char aChar )
        {
            if ( std::isspace( aChar ) ) {
                return true;
            }

            if ( aChar == ':' ) {
                mState = ParseState::InObjectValueParseBegin;
                return true;
            }

            return false;
        }

        bool ParseValueBegin( const char aChar )
        {
            if ( std::isspace( aChar ) ) {
                return true;
            }

            if ( aChar == '\"' ) {
                mState               = ParseState::InObjectValueParseString;
                mParsedElement.Value = std::string {};
                return true;
            }

            if ( ( std::toupper( aChar ) == 'T' )
                 || ( std::toupper( aChar ) == 'F' ) )
            {
                mState               = ParseState::InObjectValueParseBool;
                mParsedElement.Value = std::string {};

                return ParseValueBoolean( aChar );
            }

            if ( aChar == '[' ) {
                mState = ParseState::InObject;
                mOpeningSquareCount++;

                if ( !mParents.back().IsArray() ) {
                    JsonObject& lParent                                                                        = mParents.back().GetObject();
                    std::get<JsonObject::DictType>( mParents.back().GetObject().Value )[ mParsedElement.Name ] = JsonObject::ArrayType {};
                    auto& lValue                                                                               = std::get<JsonObject::DictType>( mParents.back().GetObject().Value )[ mParsedElement.Name ];
                    auto& lArrayRef                                                                            = std::get<JsonObject::ArrayType>( lValue );
                    mParents.push_back( &lArrayRef );
                    return true;
                }

                JsonObject::ArrayType& lArray = mParents.back().GetArray();
                lArray.push_back( JsonObject::ArrayType {} );
                JsonObject::ArrayType& lNewParent = std::any_cast<JsonObject::ArrayType&>( lArray.back() );

                mParents.push_back( &lNewParent );

                return true;
            }

            if ( aChar == '{' ) {
                if ( !mParents.back().IsArray() ) {
                    if ( mParsedElement.Name.empty() ) {
                        return false;
                    }
                }

                mOpeningCurlyCount++;
                mState = ParseState::InObject;

                if ( mParents.back().IsArray() ) {
                    return true;
                }

                JsonObject& lParent = mParents.back().GetObject();
                JsonObject  lNewObject {};
                lNewObject.Value                                                                           = JsonObject::DictType {};
                std::get<JsonObject::DictType>( mParents.back().GetObject().Value )[ mParsedElement.Name ] = lNewObject;
                auto& lValue                                                                               = std::get<JsonObject::DictType>( lParent.Value )[ mParsedElement.Name ];
                auto& lObjectRef                                                                           = std::get<JsonObject>( lValue );
                mParents.push_back( &lObjectRef );
                return true;
            }

            mState = ParseState::InObjectValueParseNumber;
            mParsedElement.Value.clear();

            return ParseValueNumber( aChar );
        }

        template<typename T>
        void AddValueToParent( const T& aValue )
        {
            mState = ParseState::InObjectValueParseFinish;

            if ( mParents.back().IsArray() ) {
                mState = ParseState::InArrayObjectFinish;
                mParents.back().GetArray().push_back( aValue );
                return;
            }

            std::get<JsonObject::DictType>( mParents.back().GetObject().Value )[ mParsedElement.Name ] = aValue;
        }

        bool ParseValueString( const char aChar )
        {
            if ( aChar == '\"' ) {
                AddValueToParent( mParsedElement.Value );
                return true;
            }

            mParsedElement.Value += aChar;

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

        bool ParseValueNumber( const char aChar )
        {
            if ( std::isspace( aChar )
                 || ( aChar == ']' )
                 || ( aChar == ',' )
                 || ( aChar == '}' ) )
            {
                std::optional<std::variant<int64_t, uint64_t, double>> lResult = ToNumber<uint64_t, int64_t, double>( mParsedElement.Value );

                if ( lResult.has_value() ) {
                    std::visit( [ & ] (auto aValue)
                                {
                                    AddValueToParent( aValue );
                                }, lResult.value() );
                }
                else {
                    return false;
                }

                if ( mState == ParseState::InArrayObjectFinish ) {
                    return ParseInArrayObjectFinish( aChar );
                }

                if ( mState == ParseState::InObjectValueParseFinish ) {
                    return ParseValueFinish( aChar );
                }

                return false;
            }

            mParsedElement.Value += aChar;

            return true;
        }

        bool ParseValueBoolean( const char aChar )
        {
            if ( std::isspace( aChar )
                 || ( aChar == ']' )
                 || ( aChar == ',' )
                 || ( aChar == '}' ) )
            {
                bool lParsedValue = false;

                if ( mParsedElement.Value == "true" ) {
                    lParsedValue = true;
                }
                else if ( mParsedElement.Value == "false" ) {
                    lParsedValue = false;
                }
                else {
                    return false;
                }

                AddValueToParent( lParsedValue );

                if ( mState == ParseState::InArrayObjectFinish ) {
                    return ParseInArrayObjectFinish( aChar );
                }

                if ( mState == ParseState::InObjectValueParseFinish ) {
                    return ParseValueFinish( aChar );
                }

                return false;
            }

            mParsedElement.Value += aChar;
            return true;
        }

        bool ParseValueFinish( const char aChar )
        {
            if ( std::isspace( aChar ) ) {
                return true;
            }

            if ( aChar == ',' ) {
                mState = ParseState::InObject;
                return true;
            }

            if ( aChar == '}' ) {
                if ( mOpeningCurlyCount == 0 ) {
                    return false;
                }

                mOpeningCurlyCount--;
                mParents.pop_back();
                mState = ParseState::InObjectFinish;

                return true;
            }

            return false;
        }

        bool PareseObjectFinish( const char aChar )
        {
            if ( std::isspace( aChar ) ) {
                return true;
            }

            if ( aChar == '}' ) {
                if ( mOpeningCurlyCount == 0 ) {
                    return false;
                }

                mParents.pop_back();
                mOpeningCurlyCount--;
                return true;
            }

            if ( aChar == ',' ) {
                mState = ParseState::InObject;
                return true;
            }

            if ( aChar == ']' ) {
                if ( mOpeningSquareCount == 0 ) {
                    return false;
                }

                mOpeningSquareCount--;
                mParents.pop_back();
                return true;
            }

            return false;
        }

        bool ParseInArrayObject( const char aChar )
        {
            if ( std::isspace( aChar ) ) {
                return true;
            }

            if ( aChar == '}' ) {
                mState = ParseState::InArrayObjectFinish;
                return true;
            }

            if ( aChar == '\"' ) {
                mState              = ParseState::InObjectName;
                mParsedElement.Name = "";
                return true;
            }

            return false;
        }

        bool ParseInArrayObjectFinish( const char aChar )
        {
            if ( std::isspace( aChar ) ) {
                return true;
            }

            if ( aChar == ',' ) {
                mState = ParseState::InObject;
                return true;
            }

            if ( aChar == ']' ) {
                if ( mOpeningSquareCount == 0 ) {
                    return false;
                }

                mOpeningSquareCount--;
                mParents.pop_back();
                mState = ParseState::InObjectFinish;
                return true;
            }

            return false;
        }

    public:
        bool Parse( const std::string& aDocument )
        {
            mParents.clear();
            Root   = {};
            mState = ParseState::Init;

            for ( const auto lChar : aDocument ) {
                bool lParseResult = [ this ] ( const char bChar )->bool
                                    {
                                        if ( mState == ParseState::Init ) {
                                            return ParseInit( bChar );
                                        }

                                        if ( mState == ParseState::InObject ) {
                                            return ParseInObject( bChar );
                                        }

                                        if ( mState == ParseState::InObjectName ) {
                                            return ParseInObjectName( bChar );
                                        }

                                        if ( mState == ParseState::InObjectValueDelimiter ) {
                                            return ParseValueDelimiter( bChar );
                                        }

                                        if ( mState == ParseState::InObjectValueParseBegin ) {
                                            return ParseValueBegin( bChar );
                                        }

                                        if ( mState == ParseState::InObjectValueParseString ) {
                                            return ParseValueString( bChar );
                                        }

                                        if ( mState == ParseState::InObjectValueParseBool ) {
                                            return ParseValueBoolean( bChar );
                                        }

                                        if ( mState == ParseState::InObjectValueParseNumber ) {
                                            return ParseValueNumber( bChar );
                                        }

                                        if ( mState == ParseState::InObjectValueParseFinish ) {
                                            return ParseValueFinish( bChar );
                                        }

                                        if ( mState == ParseState::InObjectFinish ) {
                                            return PareseObjectFinish( bChar );
                                        }

                                        if ( mState == ParseState::InArrayObject ) {
                                            return ParseInArrayObject( bChar );
                                        }

                                        if ( mState == ParseState::InArrayObjectFinish ) {
                                            return ParseInArrayObjectFinish( bChar );
                                        }

                                        return false;
                                    } ( lChar );

                if ( !lParseResult ) {
                    return false;
                }
            }

            if ( ( mState == ParseState::InObjectFinish )
                 && ( mOpeningCurlyCount == 0 )
                 && ( mOpeningSquareCount == 0 )
                 && mParents.empty() )
            {
                return true;
            }

            return false;
        }
    };
}
