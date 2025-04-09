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

JsonObjectBegin( Complex );
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
    ArrayElement user;
    user.userName     = "John";
    user.userPassword = "asdf";
    user.daysLogin    = { "Monday, Friday" };
    std::vector<ArrayElement> users;
    users.push_back( user );
    user.userName     = "David";
    user.userPassword = "pwd123";
    user.daysLogin    = { "Sunday" };
    users.push_back( user );
    p.users = users;

    std::cout << jsbjson::ToJson<Complex> {}( p, true ) << std::endl;

    JsonDocument lDocument;
    // lDocument.Parse( "{\"string1\":\"value1\",\"string2\":\"value2\",\"object\":{\"string3\":\"value3\"}}" );
    // lDocument.Parse( "{\"object\":{\"string3\":\"value3\", \"xxx\":\"kurva\"}, \"objec2\":{\"string6\":\"value7\"}}" );
    lDocument.Parse( "{\"object\":{\"string3\":\"value3\"}, \"faszom\":\"geci\"}" );
    // lDocument.Parse( "{\"string3\":\"value3\"}" );

    if ( true ) {
        int debug = 3;
    }
}
