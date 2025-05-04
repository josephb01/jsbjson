#include <string_view>
#include <typeinfo>
#include <iostream>
#include <memory>
#include <optional>
#include <variant>
#include "jsonobject.h"
#include "parser.h"
#include "mapparsernotifier.h"
#include "frommap.h"
#include "toobject.h"
#include "fromobject.h"

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

JsonObjectBegin( complex );
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

JsonObjectBegin( simple );
    JsonAddMember( description, std::string );
    JsonAddObjectMember( person );
JsonObjectEnd( 2 );

JsonObjectBegin( boolean );
    JsonAddMember( boolVal1, bool );
    JsonAddMember( boolVal2, bool );
JsonObjectEnd( 2 );

JsonObjectBegin( number )
    JsonAddMember( number1, double );
    JsonAddMember( number2, int64_t );
    JsonAddMember( number3, int64_t );
JsonObjectEnd( 3 );

JsonObjectBegin( array )
    JsonAddMember( values, std::vector<std::string> );
JsonObjectEnd( 1 );

template<typename OBJECT>
bool IsValid( const OBJECT& aObject )
{
    const std::string lJson = aObject.ToJson();
    std::cout << "From object to json string:" << std::endl;
    std::cout << lJson << std::endl;
    std::cout << "----------------------------" << std::endl;
    std::optional<OBJECT> lObject = jsbjson::ToObject<OBJECT> {}( lJson );

    if ( !lObject.has_value() ) {
        std::cout << "Failed to parse!!" << std::endl;
        return false;
    }

    const std::string lJsonFromParsed = lObject.value().ToJson();

    if ( lJsonFromParsed == lJson ) {
        return true;
    }

    std::cout << "Parsed not the same, parsed:" << std::endl;
    std::cout << lJsonFromParsed << std::endl;
    std::cout << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!" << std::endl;
    return false;
}

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

    if ( !IsValid( lComplex ) ) {
        std::cout << "Complex not OK!" << std::endl;
    }

    boolean lBool;
    lBool.boolVal1 = true;
    lBool.boolVal2 = false;

    if ( !IsValid( lBool ) ) {
        std::cout << "Boolean not OK!" << std::endl;
    }

    number lNumber;
    lNumber.number1 = 55.5;
    lNumber.number2 = -111;
    lNumber.number3 = 555;

    if ( !IsValid( lNumber ) ) {
        std::cout << "number not OK!" << std::endl;
    }

    array lArray;
    lArray.values = { std::string( "apple" ), std::string( "wall" ) };

    if ( !IsValid( lArray ) ) {
        std::cout << "Array not ok!" << std::endl;
    }

    jsbjson::FromMap lFromMap;

    jsbjson::JsonElement lMap;
    lMap[ "alma" ] = jsbjson::JsonElement {
        { std::string( "bicigli" ), 333 }
    };
    lMap[ "price" ]        = 534;
    lMap[ "array" ]        = std::vector<jsbjson::JsonVariant> { std::string( "egy" ), std::string( "ketto" ) };
    lMap[ "variantArray" ] = std::vector<jsbjson::JsonVariant> { std::string( "harom" ), 666 };
    lMap[ "objectArray" ]  = std::vector<jsbjson::JsonVariant> {
        jsbjson::JsonElement {
            { std::string { "obj" }, 23 }, { std::string { "test" }, true }
        }, std::vector<jsbjson::JsonVariant> { 1, 2, 3, 4, 5, 6, 7 }
    };

    lMap[ "arrayOfArray" ] = std::vector<jsbjson::JsonVariant> { std::vector<jsbjson::JsonVariant> { 1, 2, 3 }, std::vector<jsbjson::JsonVariant> { 4, 5, 6 }
    };

    lMap[ "arrayOfVariantArray" ] = std::vector<jsbjson::JsonVariant> {
        std::vector<jsbjson::JsonVariant> { 1, 2, std::string { "harom" }
        }, std::vector<jsbjson::JsonVariant> { 4, std::string { "ot" }, 6, std::vector<jsbjson::JsonVariant> { 66, 77, 88 }
        }
    };

    std::cout << "--------------------" << std::endl;
    std::cout << lFromMap( lMap ) << std::endl;
    std::cout << "-------------------------------" << std::endl;
    jsbjson::JsonParser  lMyParser;
    jsbjson::JsonElement lResult;

    if ( lMyParser.Parse( lFromMap( lMap ), std::make_shared<jsbjson::MapParserNotifier>( lResult ) ) ) {
        if ( auto lValue = lResult[ "price" ].GetValue<uint64_t>(); lValue.has_value() ) {
            std::cout << "WORKS!!!" << std::endl;
        }
    }
}
