#pragma once
#include <variant>
#include <string>
#include <map>
#include <vector>
#include <optional>
#include <list>

struct JsonObject
{
    using ObjectType = std::variant<int64_t, bool, std::string, double, JsonObject>;
    using ArrayType  = std::vector<ObjectType>;
    using ValueType  = std::variant<ObjectType, ArrayType>;
    using DictType   = std::map<std::string, ValueType>;

    struct Empty {};

    bool IsArray() const
    {
        return std::holds_alternative<ArrayType>( Value );
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
    std::optional<T> Get( const std::string& aKey )
    {
        using Decayed_t = std::decay_t<T>;

        if constexpr ( std::is_same_v<Decayed_t, ArrayType>) {
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

                if ( !std::holds_alternative<Decayed_t>( lObjectTypeVariant ) ) {
                    return std::nullopt;
                }

                return std::get<Decayed_t>( lObjectTypeVariant );
            }
        }

        return std::nullopt;
    }

    size_t Size() const
    {
        if ( IsArray() ) {
            return std::get<ArrayType>( Value ).size();
        }

        return std::get<DictType>( Value ).size();
    }

    std::variant<Empty, ArrayType, DictType> Value = DictType {};
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
        , InObjectParse
        , InNextElement
        , InType
        , InArray
        , Finished
    };

    ParseState State = ParseState::Init;

private:
    struct ParsedElement
    {
        using ValueType = JsonObject::ObjectType;
        std::string Name;
        ValueType   Value;
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
            State = ParseState::InObject;
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

        if ( std::isdigit( aChar ) ) {
            State               = ParseState::InObjectValueParseNumber;
            ParsedElement.Value = std::string {};
            return true;
        }

        if ( ( std::toupper( aChar ) == 'T' )
             || ( std::toupper( aChar ) == 'F' ) )
        {
            State = ParseState::InObjectValueParseBool;
            return true;
        }

        if ( aChar == '[' ) {
            State = ParseState::InObject;

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
            Parents.push_back( &lObject );
            return true;
        }

        return false;
    }

    bool ParseValueString( const char aChar )
    {
        if ( aChar == '\"' ) {
            State = ParseState::InObjectValueParseFinish;

            if ( Parents.back()->IsArray() ) {
                State = ParseState::InArrayObjectFinish;
                std::get<JsonObject::ArrayType>( Parents.back()->Value ).push_back( ParsedElement.Value );
                return true;
            }

            std::get<JsonObject::DictType>( Parents.back()->Value )[ ParsedElement.Name ] = ParsedElement.Value;
            return true;
        }

        std::get<std::string>( ParsedElement.Value ) += aChar;
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
        Root = {};

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
