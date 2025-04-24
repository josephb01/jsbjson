#pragma once
#include <string_view>
#include <iostream>
#include "bindings.h"

namespace jsbjson
{
    template<class S, std::size_t... Is, class Tup>
    S to_struct( std::index_sequence<Is...>,
                 Tup&& tup )
    {
        using std::get;
        return { get<Is>( std::forward<Tup>( tup ) ) ... };
    }

    template<class S, class Tup>
    S to_struct( Tup&& tup )
    {
        using T = std::remove_reference_t<Tup>;

        return to_struct<S>( std::make_index_sequence<std::tuple_size<T> {}> {}, std::forward<Tup>( tup ) );
    }
}

#define STRING( a ) STR( a )
#define STR( a )    #a

#define CONCAT( a, b )       CONCAT_INNER( a, b )
#define CONCAT_INNER( a, b ) a ## b

#define UNIQUE_NAME( base ) CONCAT( base, __COUNTER__ )

#define CreateMember( aName, aType, aStructName, aMandatory ) \
        template<typename T> \
        struct aStructName \
        { \
            using Type                             = T; \
            static constexpr std::string_view Name = STRING( aName ); \
            T                                 Value; \
            static constexpr bool             IsMandatory = aMandatory; \
            bool                              IsSet       = false; \
            aStructName( const T& aVal ) \
                : Value( aVal ) \
            {} \
            aStructName( const aStructName& aOther ) = default; \
            aStructName()                            = default; \
            const T& operator ()() const \
            { \
                return Value; \
            } \
            auto& operator =( const T& aValue ) \
            { \
                IsSet = true; \
                Value = aValue; \
                return Value; \
            } \
        private: \
            static constexpr bool IsAJsonMember() { return true; } \
        }; \
        aStructName<aType> aName; \
        using aName ## _t = aStructName<aType>

#define JsonObjectBegin( aName ) \
        struct aName { \
            constexpr std::string_view Name() const { return STRING( aName ); } \
        private: \
            static constexpr bool IsAJsonObject() { return true; } \
        public:
#define JsonObjectBeginRoot( aName ) \
        JsonObjectBegin( aName ) \
            std::string ToJson() const { \
                return jsbjson::ToJson<aName> {}( *this, true ); \
            }
#define JsonAddMember( aName, aType ) \
        CreateMember( aName, aType, UNIQUE_NAME( aName ), false )
#define JsonAddMandatoryMember( aName, aType ) \
        CreateMember( aName, aType, UNIQUE_NAME( aName ), true )
#define JsonAddObjectMember( aType )          aType aType;
#define JsonAddMandatoryObjectMember( aType ) aType aType;
#define JsonObjectEnd( aMemberCount ) \
        auto Convert() const { return ToTuple<aMemberCount> {}( *this ); } \
        auto ConvertRef() { return ToRefTuple<aMemberCount> {}( *this ); } \
        };
