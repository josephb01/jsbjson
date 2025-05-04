#include <iostream>
#include "jsbjson.h"

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
JsonObjectEnd( 2 );

JsonObjectBegin( fruit );
    JsonAddMember( type, std::string );
JsonObjectEnd( 1 );

JsonObjectBegin( complex );
    JsonAddMember( description, std::string );
    JsonAddObjectMember( fruit );
    JsonAddObjectMember( price );
    JsonAddMember( values, std::vector<int64_t> );
    JsonAddMember( moreValues, std::vector<int64_t> );
    JsonAddMember( arrayOfArrays, std::vector<std::vector<uint64_t>> );
    JsonAddMember( arrayOfObjects, std::vector<arrayItem> );
JsonObjectEnd( 7 );

int main()
{
    complex lComplex;
    lComplex.description    = "This is a fruit";
    lComplex.fruit.type     = "Apple";
    lComplex.price.currency = "$";
    lComplex.price.value    = 12;
    lComplex.values         = { 1, 2, 3 };
    lComplex.moreValues     = { 4, 5, 6 };
    lComplex.arrayOfArrays  = {
        { 11, 22 }, { 33, 44 }
    };
    lComplex.arrayOfObjects = {
        { std::string( "Mike" ), std::string( "USA" ) }, { std::string( "Peter" ), std::string( "Canada" ) }
    };

    std::cout << "Generated json string:" << std::endl;
    std::cout << jsbjson::FromObject {}( lComplex ) << std::endl;

    std::cout << "Generated json from member: (arrayOfObjects)" << std::endl;
    std::cout << jsbjson::FromObject {}( lComplex.arrayOfObjects ) << std::endl;

    std::optional<complex> lParsedComplex = jsbjson::ToObject<complex> {}( jsbjson::FromObject {}( lComplex ) );

    if ( lParsedComplex.has_value() ) {
        if ( lParsedComplex.value().arrayOfObjects.Value[ 0 ].name == lComplex.arrayOfObjects.Value[ 0 ].name ) {
            std::cout << "same" << std::endl;
        }
    }

    jsbjson::JsonElementEx lMyObject;
    lMyObject[ "description" ]   = std::string( "This is a test object" );
    lMyObject[ "arrayOfNumber" ] = std::vector<jsbjson::JsonVariant> { 1, 2, 3, 4, 5, 6 };
    lMyObject[ "arrayOfArray" ]  = std::vector<jsbjson::JsonVariant> {
        std::vector<jsbjson::JsonVariant> { 1, 2, 3 }, std::vector<jsbjson::JsonVariant> { 4, 5, 6 }
    };
    lMyObject[ "data" ] = jsbjson::JsonElement {
        { std::string { "userName" }, std::string { "John" }
        }, { std::string { "age" }, 44 }
    };

    std::cout << "Generated json from JsonElementEx:" << std::endl;
    std::cout << lMyObject.ToJson() << std::endl;

    jsbjson::JsonElementEx lParsedObject;

    if ( lParsedObject.FromJson( lMyObject.ToJson() ) ) {
        std::cout << lParsedObject[ "data" ].GetValueRef<jsbjson::JsonElement>()[ "age" ].GetValueRef<uint64_t>();
    }

    lParsedObject[ "data" ].GetValueRef<jsbjson::JsonElement>()[ "age" ] = 66;
    std::cout << "Modified value:" << std::endl;
    std::cout << lParsedObject.ToJson() << std::endl;
}
