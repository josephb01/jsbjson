#pragma once
#include <string_view>
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

#define CreateMember( aName, aType, aStructName ) \
        template<typename T> \
        struct aStructName \
        { \
            using Type                             = T; \
            static constexpr std::string_view Name = STRING( aName ); \
            T                                 Value; \
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
                Value = aValue; \
                return Value; \
            } \
            T& operator ->() { return Value; } \
        private: \
            static constexpr bool IsAJsonMember() { return true; } \
        }; \
        aStructName<aType> aName; \
        using aName ## _t = aStructName<aType>

#define CreateArrayMember( aName, aType, aStructName ) \
        template<typename T> \
        struct aStructName \
        { \
            using Type                             = std::vector<T>; \
            using ArrayType                        = T; \
            static constexpr std::string_view Name = STRING( aName ); \
            std::vector<T>                    Value; \
            aStructName( const std::vector<T>& aVal ) \
                : Value( aVal ) \
            {} \
            aStructName( const aStructName& aOther ) = default; \
            aStructName()                            = default; \
            const T& operator ()() const \
            { \
                return Value; \
            } \
            auto& operator =( const Type& aValue ) \
            { \
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
            template<typename...ARGS> \
            static aName Create( const ARGS & ... aArgs ) \
            { \
                using TupleType = std::tuple<ARGS...>; \
                TupleType Tuple = std::make_tuple( aArgs... ); \
                return jsbjson::to_struct<aName, TupleType>( std::move( Tuple ) ); \
            } \
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
        CreateMember( aName, aType, UNIQUE_NAME( aName ) )
#define JsonAddArrayMember( aName, aType ) \
        CreateArrayMember( aName, aType, UNIQUE_NAME( aName ) )
#define JsonAddObjectMember( aType ) aType aType;
#define JsonObjectEnd( aMemberCount ) \
        constexpr size_t MemberCount() const { return aMemberCount; } \
        auto Convert() const { return ToTuple<aMemberCount> {}( *this ); } \
        auto ConvertRef() { return ToRefTuple<aMemberCount> {}( *this ); } \
        };
