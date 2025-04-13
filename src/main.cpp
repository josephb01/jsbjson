#include <string_view>
#include <typeinfo>
#include <iostream>
#include "jsonobject.h"
#include "jsongenerator.h"
#include "jsondocument.h"

JsonObjectBegin( Member1 );
    JsonAddMember( korte, int );
JsonObjectEndWithName( 1, complexMember1 );

JsonObjectBegin( Member3 );
    JsonAddMember( valami, int );
JsonObjectEndWithName( 1, subMember );

JsonObjectBegin( Member2 );
    JsonAddMember( desc2, std::string );
    JsonAddObjectMember( subMember, Member3 );
JsonObjectEndWithName( 2, complexMember3 );

JsonObjectBegin( ArrayElement );
    JsonAddMember( userName, std::string );
    JsonAddMember( userPassword, std::string );
    JsonAddMember( daysLogin, std::vector<std::string> );
JsonObjectEndWithName( 3, userInfo );

JsonObjectBeginRoot( Complex );
JsonAddMember( description, std::string );
JsonAddObjectMember( complexMember1, Member1 );
JsonAddObjectMember( complexMember2, Member2 );
JsonAddMember( values, std::vector<int> );
JsonAddMember( users, std::vector<ArrayElement> );
JsonObjectEndWithName( 5, complex );

int main()
{
    Complex p;
    p.description                     = "This is a description";
    p.complexMember1.korte            = 23;
    p.complexMember2.desc2            = "Description2";
    p.complexMember2.subMember.valami = 12;
    p.values                          = { 1, 2, 3, 4 };
    p.users                           = { ArrayElement::Create( ArrayElement::userName_t( "John" ), ArrayElement::userPassword_t( "pwd1" ), ArrayElement::daysLogin_t( { std::string { "Monday" }, std::string { "Tuesday" }
                                                                                                                                                                       } ) ) };
    p.users.Value.push_back( ArrayElement::Create( ArrayElement::userName_t( "Peter" ), ArrayElement::userPassword_t( "pwd123" ), ArrayElement::daysLogin_t( { std::string { "Tuesday" }, std::string { "Friday" }
                                                                                                                                                             } ) ) );
    // std::cout << jsbjson::ToJson<Complex> {}( p, true ) << std::endl;
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
