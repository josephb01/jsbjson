#pragma once
#include <variant>
#include <string>
#include <map>
#include <vector>
#include <list>

struct JsonObject
{
    using ObjectType = std::variant<int64_t, bool, std::string, double, JsonObject, std::vector<JsonObject>>;
    using DictType   = std::map<std::string, ObjectType>;

    DictType Value;
};

struct JsonDocument
{
private:
    enum class ParseState
    {
        Init
        , InObject
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
        using ObjectType = JsonObject::ObjectType;
        std::string Name;
        ObjectType  Value;
    };

    ParsedElement          ParsedElement;
    std::list<JsonObject*> Parents;
    size_t                 OpeningCount = 0;
    JsonObject             Root;

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
            State = ParseState::InObjectValueParseArray;
            return true;
        }

        if ( aChar == '{' ) {
            if ( ParsedElement.Name.empty() ) {
                return false;
            }

            OpeningCount++;
            State = ParseState::InObject;

            Parents.back()->Value[ ParsedElement.Name ] = JsonObject {};
            JsonObject* lNewItem                        = &std::get<JsonObject>( Parents.back()->Value[ ParsedElement.Name ] );
            Parents.push_back( lNewItem );
            return true;
        }

        return false;
    }

    bool ParseValueString( const char aChar )
    {
        if ( aChar == '\"' ) {
            Parents.back()->Value[ ParsedElement.Name ] = ParsedElement.Value;
            State                                       = ParseState::InObjectValueParseFinish;
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

public:
    bool Parse( const std::string& aDocument )
    {
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
