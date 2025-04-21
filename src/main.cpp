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
    JsonAddMember( values, std::vector<int64_t> );
    JsonAddMember( moreValues, std::vector<int64_t> );
    JsonAddMember( arrayOfArrays, std::vector<std::vector<uint64_t>> );
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

JsonObjectBeginRoot( boolean );
    JsonAddMember( boolVal1, bool );
    JsonAddMember( boolVal2, bool );
JsonObjectEnd( 2 );

JsonObjectBeginRoot( number )
    JsonAddMember( number1, double );
    JsonAddMember( number2, int64_t );
    JsonAddMember( number3, int64_t );
JsonObjectEnd( 3 );

JsonObjectBeginRoot( array )
    JsonAddMember( values, std::vector<std::string> );
JsonObjectEnd( 1 );

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

    const std::string& lComplexJson = lComplex.ToJson();

    struct X
    {
        void ConvertRef()
        {
            return;
        }
    };

    std::cout << lComplexJson << std::endl;

    std::optional<complex> lParsedComplex = jsbjson::FromJson<complex> {}( lComplexJson );

    std::optional<simple> lParsed = jsbjson::FromJson<simple> {}( "{\"description\":\"Simple test object\",\"person\":{\"name\":\"John\",\"location\":\"USA\"}}" );

    if ( lParsed.has_value() ) {
        std::cout << lParsed.value().ToJson() << std::endl;
    }

    boolean lBool;
    lBool.boolVal1 = true;
    lBool.boolVal2 = false;

    const std::string& lBoolJson = lBool.ToJson();
    std::cout << lBoolJson << std::endl;
    std::optional<boolean> lBoolParsed = jsbjson::FromJson<boolean> {}( lBoolJson );

    number lNumber;
    lNumber.number1 = 55.5;
    lNumber.number2 = -111;
    lNumber.number3 = 555;

    const std::string& lNumberJson = lNumber.ToJson();
    std::cout << lNumberJson << std::endl;

    std::optional<number> lParsedNumber = jsbjson::FromJson<number> {}( lNumberJson );

    array lArray;
    lArray.values                 = { std::string( "apple" ), std::string( "wall" ) };
    const std::string& lArrayJson = lArray.ToJson();
    std::cout << lArrayJson << std::endl;

    std::optional<array> lParsedrray = jsbjson::FromJson<array> {}( lArrayJson );

    jsbjson::JsonDocument lDocument;
    lDocument.Parse( "{\"array\":[\"alma\",\"korte\"]}" );
    lDocument.Parse( "{\"array\":[true,false]}" );
    lDocument.Parse( "{\"array\":[{\"type\":\"fruit\", \"value\":\"3\"},\"korte\"]}" );

    const jsbjson::JsonObject::ArrayType lObject = lDocument.Root.Get<jsbjson::JsonObject::ArrayType>( "array" );
}
