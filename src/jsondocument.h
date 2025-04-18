#pragma once
#include <variant>
#include <string>
#include <map>
#include <vector>
#include <optional>
#include <list>
#include <charconv>

namespace jsbjson
{
    struct JsonObject
    {
        using ObjectType = std::variant<int64_t, uint64_t, bool, std::string, double, JsonObject>;
        using ArrayType  = std::vector<ObjectType>;
        using ValueType  = std::variant<ObjectType, ArrayType>;
        using DictType   = std::map<std::string, ValueType>;

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
        const std::optional<T> GetAsArray( const std::string& aKey )
        {
            using Decayed_t = std::decay_t<T>;

            if constexpr ( !std::is_same_v<Decayed_t, ArrayType>) {
                return std::nullopt;
            }
            else {
                DictType& lDict = std::get<DictType>( Value );

                if ( lDict.count( aKey ) == 0 ) {
                    return std::nullopt;
                }

                const ValueType& lValue = lDict[ aKey ];

                if ( std::holds_alternative<ObjectType>( lValue ) ) {
                    const ObjectType& lObjectTypeVariant = std::get<ObjectType>( lValue );

                    if ( !std::holds_alternative<JsonObject>( lObjectTypeVariant ) ) {
                        return std::nullopt;
                    }

                    const JsonObject& lJsonObject = std::get<JsonObject>( lObjectTypeVariant );

                    if ( !lJsonObject.IsArray() ) {
                        return std::nullopt;
                    }

                    return std::get<ArrayType>( lJsonObject.Value );
                }
            }

            return std::nullopt;
        }

        template<typename T>
        std::optional<T> GetOpt( const std::string& aKey )
        {
            using Decayed_t = std::decay_t<T>;

            if constexpr ( std::is_same_v<Decayed_t, ArrayType>) {
                DictType& lDict = std::get<DictType>( Value );

                if ( lDict.count( aKey ) == 0 ) {
                    return std::nullopt;
                }

                const ValueType& lValue = lDict[ aKey ];

                if ( std::holds_alternative<ObjectType>( lValue ) ) {
                    const ObjectType& lObjectTypeVariant = std::get<ObjectType>( lValue );

                    if ( !std::holds_alternative<JsonObject>( lObjectTypeVariant ) ) {
                        return std::nullopt;
                    }

                    const JsonObject& lJsonObject = std::get<JsonObject>( lObjectTypeVariant );

                    if ( !lJsonObject.IsArray() ) {
                        return std::nullopt;
                    }

                    return std::get<ArrayType>( lJsonObject.Value );
                }
            }
            else {
                DictType& lDict = std::get<DictType>( Value );

                if ( lDict.count( aKey ) == 0 ) {
                    return std::nullopt;
                }

                const ValueType& lValue = lDict[ aKey ];

                if ( std::holds_alternative<ObjectType>( lValue ) ) {
                    const ObjectType& lObjectTypeVariant = std::get<ObjectType>( lValue );

                    if ( !std::holds_alternative<Decayed_t>( lObjectTypeVariant ) ) {
                        return std::nullopt;
                    }

                    return std::get<Decayed_t>( lObjectTypeVariant );
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

    struct JsonDocument
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

        ParseState State = ParseState::Init;

    private:
        struct ParsedElement
        {
            std::string Name;
            std::string Value;
        };

        ParsedElement          ParsedElement;
        std::list<JsonObject*> Parents;
        size_t                 OpeningCount        = 0;
        size_t                 OpeningBracketCount = 0;

    public:
        JsonObject Root;

    private:
        bool ParseInit( const char aChar )
        {
            if ( std::isspace( aChar ) ) {
                return true;
            }

            if ( aChar == '{' ) {
                OpeningCount++;
                State      = ParseState::InObject;
                Root.Value = JsonObject::DictType {};
                Parents.push_back( &Root );
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
                if ( !Parents.back()->IsArray() ) {
                    return false;
                }

                OpeningCount++;
                JsonObject::ArrayType& lArray = std::get<JsonObject::ArrayType>( Parents.back()->Value );
                lArray.push_back( JsonObject {} );
                JsonObject& lObject = std::get<JsonObject>( lArray.back() );
                lObject.Value       = JsonObject::DictType {};
                Parents.push_back( &lObject );
                State = ParseState::InArrayObject;
                return true;
            }

            if ( Parents.back()->IsArray() ) {
                return ParseValueBegin( aChar );
            }

            if ( aChar == '\"' ) {
                State              = ParseState::InObjectName;
                ParsedElement.Name = "";
                return true;
            }

            return false;
        }

        bool ParseInObjectName( const char aChar )
        {
            if ( aChar == '\"' ) {
                State = ParseState::InObjectValueDelimiter;
                return true;
            }

            ParsedElement.Name += aChar;
            return true;
        }

        bool ParseValueDelimiter( const char aChar )
        {
            if ( std::isspace( aChar ) ) {
                return true;
            }

            if ( aChar == ':' ) {
                State = ParseState::InObjectValueParseBegin;
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
                State               = ParseState::InObjectValueParseString;
                ParsedElement.Value = std::string {};
                return true;
            }

            if ( ( std::toupper( aChar ) == 'T' )
                 || ( std::toupper( aChar ) == 'F' ) )
            {
                State               = ParseState::InObjectValueParseBool;
                ParsedElement.Value = std::string {};

                return ParseValueBoolean( aChar );
            }

            if ( aChar == '[' ) {
                State = ParseState::InObject;
                OpeningBracketCount++;

                if ( !Parents.back()->IsArray() ) {
                    std::get<JsonObject::DictType>( Parents.back()->Value )[ ParsedElement.Name ] = JsonObject {};
                    auto&       lValue                                                            = std::get<JsonObject::DictType>( Parents.back()->Value )[ ParsedElement.Name ];
                    auto&       lVariant                                                          = std::get<JsonObject::ObjectType>( lValue );
                    JsonObject& lObject                                                           = std::get<JsonObject>( lVariant );
                    lObject.Value                                                                 = JsonObject::ArrayType {};
                    Parents.push_back( &lObject );
                    return true;
                }

                JsonObject::ArrayType& lArray = std::get<JsonObject::ArrayType>( Parents.back()->Value );
                lArray.push_back( JsonObject {} );
                JsonObject& lObject = std::get<JsonObject>( lArray.back() );
                Parents.push_back( &lObject );

                return true;
            }

            if ( aChar == '{' ) {
                if ( !Parents.back()->IsArray() ) {
                    if ( ParsedElement.Name.empty() ) {
                        return false;
                    }
                }

                OpeningCount++;
                State = ParseState::InObject;

                if ( Parents.back()->IsArray() ) {
                    return true;
                }

                std::get<JsonObject::DictType>( Parents.back()->Value )[ ParsedElement.Name ] = JsonObject {};
                auto&       lValue                                                            = std::get<JsonObject::DictType>( Parents.back()->Value )[ ParsedElement.Name ];
                auto&       lVariant                                                          = std::get<JsonObject::ObjectType>( lValue );
                JsonObject& lObject                                                           = std::get<JsonObject>( lVariant );
                lObject.Value                                                                 = JsonObject::DictType {};
                Parents.push_back( &lObject );
                return true;
            }

            State = ParseState::InObjectValueParseNumber;
            ParsedElement.Value.clear();

            return ParseValueNumber( aChar );
        }

        template<typename T>
        void AddValueToParent( const T& aValue )
        {
            State = ParseState::InObjectValueParseFinish;

            if ( Parents.back()->IsArray() ) {
                State = ParseState::InArrayObjectFinish;
                std::get<JsonObject::ArrayType>( Parents.back()->Value ).push_back( aValue );
                return;
            }

            std::get<JsonObject::DictType>( Parents.back()->Value )[ ParsedElement.Name ] = aValue;
        }

        bool ParseValueString( const char aChar )
        {
            if ( aChar == '\"' ) {
                AddValueToParent( ParsedElement.Value );
                return true;
            }

            ParsedElement.Value += aChar;

            return true;
        }

        template<typename HEAD, typename... TAIL>
        std::optional<std::variant<int64_t, uint64_t, double>> ToNumber( const std::string& aString )
        {
            if constexpr ( sizeof...( TAIL ) == 0 ) {
                HEAD lNumber {};
                auto [ lPtr, lErrorCode ] = std::from_chars( aString.data(), aString.data() + aString.size(), lNumber );

                if ( ( lPtr == aString.data() + aString.size() )
                     && ( lErrorCode == std::errc() ) )
                {
                    std::variant<int64_t, uint64_t, double> lVariant;
                    lVariant.emplace<HEAD>( lNumber );
                    return lVariant;
                }
            }
            else {
                HEAD lNumber {};
                auto [ lPtr, lErrorCode ] = std::from_chars( aString.data(), aString.data() + aString.size(), lNumber );

                if ( ( lPtr == aString.data() + aString.size() )
                     && ( lErrorCode == std::errc() ) )
                {
                    std::variant<int64_t, uint64_t, double> lVariant;
                    lVariant.emplace<HEAD>( lNumber );
                    return lVariant;
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
                std::optional<std::variant<int64_t, uint64_t, double>> lResult = ToNumber<int64_t, uint64_t, double>( ParsedElement.Value );

                if ( lResult.has_value() ) {
                    std::visit( [ & ] (auto aValue)
                                {
                                    AddValueToParent( aValue );
                                }, lResult.value() );
                }
                else {
                    return false;
                }

                if ( State == ParseState::InArrayObjectFinish ) {
                    return ParseInArrayObjectFinish( aChar );
                }

                if ( State == ParseState::InObjectValueParseFinish ) {
                    return ParseValueFinish( aChar );
                }

                return false;
            }

            ParsedElement.Value += aChar;

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

                if ( ParsedElement.Value == "true" ) {
                    lParsedValue = true;
                }
                else if ( ParsedElement.Value == "false" ) {
                    lParsedValue = false;
                }
                else {
                    return false;
                }

                AddValueToParent( lParsedValue );

                if ( State == ParseState::InArrayObjectFinish ) {
                    return ParseInArrayObjectFinish( aChar );
                }

                if ( State == ParseState::InObjectValueParseFinish ) {
                    return ParseValueFinish( aChar );
                }

                return false;
            }

            ParsedElement.Value += aChar;
            return true;
        }

        bool ParseValueFinish( const char aChar )
        {
            if ( std::isspace( aChar ) ) {
                return true;
            }

            if ( aChar == ',' ) {
                State = ParseState::InObject;
                return true;
            }

            if ( aChar == '}' ) {
                if ( OpeningCount == 0 ) {
                    return false;
                }

                OpeningCount--;
                Parents.pop_back();
                State = ParseState::InObjectFinish;

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
                if ( OpeningCount == 0 ) {
                    return false;
                }

                Parents.pop_back();
                OpeningCount--;
                return true;
            }

            if ( aChar == ',' ) {
                State = ParseState::InObject;
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
                State = ParseState::InArrayObjectFinish;
                return true;
            }

            if ( aChar == '\"' ) {
                State              = ParseState::InObjectName;
                ParsedElement.Name = "";
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
                State = ParseState::InObject;
                return true;
            }

            if ( aChar == ']' ) {
                if ( OpeningBracketCount == 0 ) {
                    return false;
                }

                OpeningBracketCount--;
                Parents.pop_back();
                State = ParseState::InObjectFinish;
                return true;
            }

            return false;
        }

    public:
        bool Parse( const std::string& aDocument )
        {
            Parents.clear();
            Root  = {};
            State = ParseState::Init;

            for ( const auto lChar : aDocument ) {
                bool lParseResult = [ this ] ( const char bChar )->bool
                                    {
                                        if ( State == ParseState::Init ) {
                                            return ParseInit( bChar );
                                        }

                                        if ( State == ParseState::InObject ) {
                                            return ParseInObject( bChar );
                                        }

                                        if ( State == ParseState::InObjectName ) {
                                            return ParseInObjectName( bChar );
                                        }

                                        if ( State == ParseState::InObjectValueDelimiter ) {
                                            return ParseValueDelimiter( bChar );
                                        }

                                        if ( State == ParseState::InObjectValueParseBegin ) {
                                            return ParseValueBegin( bChar );
                                        }

                                        if ( State == ParseState::InObjectValueParseString ) {
                                            return ParseValueString( bChar );
                                        }

                                        if ( State == ParseState::InObjectValueParseBool ) {
                                            return ParseValueBoolean( bChar );
                                        }

                                        if ( State == ParseState::InObjectValueParseNumber ) {
                                            return ParseValueNumber( bChar );
                                        }

                                        if ( State == ParseState::InObjectValueParseFinish ) {
                                            return ParseValueFinish( bChar );
                                        }

                                        if ( State == ParseState::InObjectFinish ) {
                                            return PareseObjectFinish( bChar );
                                        }

                                        if ( State == ParseState::InArrayObject ) {
                                            return ParseInArrayObject( bChar );
                                        }

                                        if ( State == ParseState::InArrayObjectFinish ) {
                                            return ParseInArrayObjectFinish( bChar );
                                        }

                                        return false;
                                    } ( lChar );

                if ( !lParseResult ) {
                    return false;
                }
            }

            if ( ( State == ParseState::InObjectFinish )
                 && ( OpeningCount == 0 )
                 && Parents.empty() )
            {
                return true;
            }

            return false;
        }
    };
}
