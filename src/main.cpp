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

JsonObjectBegin( person );
    JsonAddMember( name, std::string );
    JsonAddMember( location, std::string );
JsonObjectEnd( 2 );

JsonObjectBeginRoot( simple );
    JsonAddMember( description, std::string );
    JsonAddObjectMember( person );
JsonObjectEnd( 2 );

int main()
{
    complex lComplex;
    lComplex.description            = "This is a fruit";
    lComplex.fruit.type             = "Apple";
    lComplex.price.currency         = "$";
    lComplex.price.value            = 12;
    lComplex.price.justForFun.funny = "Hehe";
    lComplex.values                 = { 1, 2, 3 };
    lComplex.moreValues             = { 4, 5, 6 };
    lComplex.arrayOfArrays          = {
        { 11, 22 }, { 33, 44 }
    };
    lComplex.arrayOfObjects = {
        { std::string( "Peterke" ), std::string( "USA" ) }
    };
    std::cout << lComplex.ToJson() << std::endl;

    std::optional<simple> lParsed = jsbjson::FromJson<simple> {}( "{\"description\":\"Simple test object\",\"person\":{\"name\":\"John\",\"location\":\"USA\"}}" );

    if ( lParsed.has_value() ) {
        std::cout << lParsed.value().ToJson() << std::endl;
    }

    JsonDocument lDocument;
    lDocument.Parse( "{\"array\":[\"alma\",\"korte\"]}" );
    lDocument.Parse( "{\"array\":[{\"type\":\"fruit\", \"value\":\"3\"},\"korte\"]}" );

    const std::optional<JsonObject> lOjbect = lDocument.Root.Get<JsonObject>( "array" );
}
