#pragma once

#include <cstdint>
#include <type_traits>
#include <tuple>
#include <string>
#include <vector>
#include <list>
#include <optional>
#include <limits>
#include <utility>
#include <array>
#include "bindings.h"
#include "jsonobject.h"
#include "tovalue.h"
#include "typehelpers.h"

namespace jsbjson
{
    class FromObject
    {
        template<typename T>
        static constexpr bool IsComplexType()
        {
            using Decayed_T = std::decay_t<T>;
            return IsMember<Decayed_T>::value
                   || IsObject<Decayed_T>::value
                   || IsArray<Decayed_T>::value;
        }

    public:
        template<typename T>
        std::string operator ()( const T&   aObject,
                                 const bool aIsRoot = true )
        {
            JsonElement lResult;
            Process<T>( aObject, lResult, aIsRoot );
            return jsbjson::FromMap {}( lResult );
        }

    private:
        template<typename ITEM>
        void Process( const ITEM&  aObject,
                      JsonElement& aJsonElement,
                      const bool   aIsRoot = false ) const
        {
            if constexpr ( IsObject<std::decay_t<ITEM>>::value ) {
                std::apply( [ & ] ( auto... aMembers )
                            {
                                if ( !aIsRoot ) {
                                    aJsonElement[ aObject.Name() ] = JsonElement {};
                                    JsonElement& lElement          = aJsonElement[ aObject.Name() ].GetValueRef<JsonElement>();
                                    ( Process( aMembers, lElement, false ), ... );
                                }
                                else {
                                    ( Process( aMembers, aJsonElement, false ), ... );
                                }
                            }, aObject.Convert() );
            }

            if constexpr ( IsMember<std::decay_t<ITEM>>::value ) {
                if constexpr ( IsArray<std::decay_t<typename ITEM::Type>>::value ) {
                    aJsonElement[ aObject.Name() ]     = std::vector<JsonVariant> {};
                    std::vector<JsonVariant>& lElement = aJsonElement[ aObject.Name() ].GetValueRef<std::vector<JsonVariant>>();
                    ProcessArray( aObject.Value, lElement );
                }
                else {
                    aJsonElement[ aObject.Name() ] = aObject.Value;
                }
            }
        }

        template<typename ARRAY>
        void ProcessArray( const ARRAY&              aArray,
                           std::vector<JsonVariant>& aResult ) const
        {
            using ArrayItem_t = typename ARRAY::value_type;

            for ( const auto& lItem : aArray ) {
                if constexpr ( IsObject<std::decay_t<ArrayItem_t>>::value ) {
                    std::apply( [ & ] ( auto... aMembers )
                                {
                                    JsonElement lElement;
                                    ( Process( aMembers, lElement ), ... );
                                    aResult.push_back( lElement );
                                }, lItem.Convert() );
                }

                if constexpr ( IsArray<std::decay_t<ArrayItem_t>>::value ) {
                    std::vector<JsonVariant> lResult;
                    ProcessArray( lItem, lResult );
                    aResult.push_back( lResult );
                }

                if constexpr ( !IsComplexType<ArrayItem_t>() ) {
                    aResult.push_back( lItem );
                }
            }
        }
    };
}
