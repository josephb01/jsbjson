#pragma once
#include <string_view>
#include "bindings.h"

struct JsonObjectBase {};

struct JsonMemberBase {};

#define STRING( a ) STR( a )
#define STR( a )    #a

#define CONCAT( a, b )       CONCAT_INNER( a, b )
#define CONCAT_INNER( a, b ) a ## b

#define UNIQUE_NAME( base ) CONCAT( base, __COUNTER__ )

#define CreateMember( aName, aType, aStructName ) \
        template<typename T> \
        struct aStructName : public JsonMemberBase { \
            static constexpr std::string_view Name = STRING( aName ); \
            T                                 Value; \
            const T& operator ()() const { return Value; } \
            auto& operator =( const T& aValue ) { Value = aValue; return Value; } \
        }; \
        aStructName<aType> aName;

#define JsonObjectBegin( aName ) struct aName:public JsonObjectBase {
#define JsonAddMember( aName, aType ) \
        CreateMember( aName, aType, UNIQUE_NAME( aName ) )
#define AddJsonMember( aName, aType ) aType aName;
#define JsonObjectEnd( aMemberCount ) \
        constexpr size_t MemberCount() const { return aMemberCount; } \
        static constexpr bool HasName() { return false; } \
        auto Convert() const { return ToTuple<aMemberCount> {}( *this ); } \
        }; \

#define JsonObjectEndWithName( aMemberCount, aName ) \
        constexpr size_t MemberCount() const { return aMemberCount; } \
        static constexpr bool HasName() { return true; } \
        constexpr std::string_view Name() const { return STRING( aName ); } \
        auto Convert() const { return ToTuple<aMemberCount> {}( *this ); } \
        }; \

