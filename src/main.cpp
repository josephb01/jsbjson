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
    AddJsonMember( subMember, Member3 );
JsonObjectEndWithName( 2, complexMember3 );

JsonObjectBegin( Complex );
    JsonAddMember( description, std::string );
    AddJsonMember( complexMember1, Member1 );
    AddJsonMember( complexMember2, Member2 );
JsonObjectEndWithName( 3, complex );

int main()
{
    struct S : public JsonMemberBase {};
    Complex p;
    p.description                     = "This is a description";
    p.complexMember1.korte            = 23;
    p.complexMember2.desc2            = "Description2";
    p.complexMember2.subMember.valami = 12;

    std::cout << ToJson<Complex> {}( p, true ) << std::endl;

    JsonDocument lDocument;
    // lDocument.Parse( "{\"string1\":\"value1\",\"string2\":\"value2\",\"object\":{\"string3\":\"value3\"}}" );
    // lDocument.Parse( "{\"object\":{\"string3\":\"value3\", \"xxx\":\"kurva\"}, \"objec2\":{\"string6\":\"value7\"}}" );
    lDocument.Parse( "{\"object\":{\"string3\":\"value3\"}, \"faszom\":\"geci\"}" );
    // lDocument.Parse( "{\"string3\":\"value3\"}" );

    if ( true ) {
        int debug = 3;
    }
}
