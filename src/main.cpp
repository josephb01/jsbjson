#include <string_view>
#include <typeinfo>
#include <iostream>
#include "jsonobject.h"
#include "jsongenerator.h"
#include "jsondocument.h"

JsonObjectBegin( justForFun )
    JsonAddMember( funny, std::string );
JsonObjectEnd( 1 )

JsonObjectBegin( arrayItem )
    JsonAddMember( name, std::string );
    JsonAddMember( location, std::string );
JsonObjectEnd( 2 )

JsonObjectBegin( price );
    JsonAddMember( currency, std::string );
    JsonAddMember( value, uint64_t );
    JsonAddObjectMember( justForFun );
JsonObjectEnd( 3 );

JsonObjectBegin( fruit );
    JsonAddMember( type, std::string );
JsonObjectEnd( 1 );

JsonObjectBeginRoot( complex );
    JsonAddMember( description, std::string );
    JsonAddObjectMember( fruit );
    JsonAddObjectMember( price );
    JsonAddMember( values, std::vector<int> );
    JsonAddMember( moreValues, std::list<int> );
    JsonAddMember( arrayOfArrays, std::vector<std::vector<int>> );
    JsonAddMember( arrayOfObjects, std::vector<arrayItem> );
JsonObjectEnd( 7 );

int main()
{
    complex p;
    p.description            = "This is a fruit";
    p.fruit.type             = "Apple";
    p.price.currency         = "$";
    p.price.value            = 12;
    p.price.justForFun.funny = "Hehe";
    p.values                 = { 1, 2, 3 };
    p.moreValues             = { 4, 5, 6 };
    p.arrayOfArrays          = {
        { 11, 22 }, { 33, 44 }
    };
    p.arrayOfObjects = {
        { std::string( "Peterke" ), std::string( "USA" ) }
    };
    std::cout << p.ToJson() << std::endl;

    JsonDocument lDocument;

// lDocument.Parse( "{\"string1\":\"value1\",\"string2\":\"value2\",\"object\":{\"string3\":\"value3\"}}" );
// lDocument.Parse( "{\"object\":{\"string3\":\"value3\", \"xxx\":\"kurva\"}, \"objec2\":{\"string6\":\"value7\"}}" );
// lDocument.Parse( "{\"object\":{\"string3\":\"value3\"}, \"faszom\":\"geci\"}" );

    // lDocument.Parse( "{\"array\":[\"alma\",\"korte\"]}" );
    lDocument.Parse( "{\"array\":[{\"type\":\"fruit\", \"value\":\"3\"},\"korte\"]}" );
    // const std::optional<JsonObject::ArrayType>& lArray  = lDocument.Root.GetAsArray<JsonObject::ArrayType>( "array" );
    const std::optional<JsonObject> lOjbect = lDocument.Root.Get<JsonObject>( "array" );

    // const std::optional<JsonObject> lString = lDocument.Root.Get<JsonObject>( "object" );

// const auto& l       = std::get<JsonObject::DictType>( lDocument.Root.Value )[ "faszom" ];
// const auto& lObject = std::get<JsonObject::ObjectType>( l );
// const auto& lValue  = std::get<std::string>( lObject );

    // const JsonObject::ValueType& lValue      = lDict[ "faszom" ];

// lDocument.Parse( "{\"string3\":\"value3\"}" );

    if ( true ) {
        int debug = 3;
    }
}
