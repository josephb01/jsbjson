#pragma once
#include <iostream>
#include "bindings.h"

#define STRING( a ) STR( a )
#define STR( a )    #a

#define CONCAT( a, b )       CONCAT_INNER( a, b )
#define CONCAT_INNER( a, b ) a ## b

#define UNIQUE_NAME( base ) CONCAT( base, __COUNTER__ )

#define CreateMember( aName, aType, aStructName ) \
        template<typename T> \
        struct aStructName \
        { \
            using Type = T; \
            std::string Name() const { return STRING( aName ); } \
            T           Value; \
            bool        IsSet = false; \
            aStructName( const T& aVal ) \
                : Value( aVal ) \
            { IsSet = true; } \
            aStructName( const aStructName& aOther ) \
            { \
                if ( this != &aOther ) { \
                    Value = aOther.Value; \
                    IsSet = true; \
                } \
            } \
            aStructName() = default; \
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
            std::string Name() const { return STRING( aName ); } \
            std::string ToJson() const { \
                return jsbjson::FromObject {}( *this ); \
            } \
        private: \
            static constexpr bool IsAJsonObject() { return true; } \
        public:
#define JsonAddMember( aName, aType ) \
        CreateMember( aName, aType, UNIQUE_NAME( aName ) )
#define JsonAddObjectMember( aType ) aType aType;
#define JsonObjectEnd( aMemberCount ) \
        auto Convert() const { return ToTuple<aMemberCount> {}( *this ); } \
        auto ConvertRef() { return ToRefTuple<aMemberCount> {}( *this ); } \
        };
