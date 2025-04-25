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
#include "jsondocument.h"
#include "tovalue.h"
#include "typehelpers.h"

namespace jsbjson
{
    template<typename T>
    class ToJson
    {
        using Decayed_T = std::decay_t<T>;

        static constexpr bool IsComplexType()
        {
            return IsMember<Decayed_T>::value
                   || IsObject<Decayed_T>::value
                   || IsArray<Decayed_T>::value;
        }

        static std::string AppendComma( const size_t aItemCount,
                                        const size_t aCurrent )
        {
            if ( aCurrent < aItemCount ) {
                return ",";
            }

            return "";
        }

    public:
        std::string operator ()( const T&   aObject,
                                 const bool aIsRoot  = true,
                                 const bool aInArray = false ) const
        {
            std::string lResult;

            if ( aIsRoot ) {
                lResult += "{";
            }

            if constexpr ( IsObject<Decayed_T>::value ) {
                if ( !aIsRoot ) {
                    if ( !aInArray ) {
                        lResult += "\"" + std::string { aObject.Name() } + "\":";
                    }

                    lResult += "{";
                }

                std::apply( [ &lResult ] ( auto... aMembers )
                            {
                                size_t lIndex             = 0;
                                const size_t lMemberCount = sizeof...( aMembers );
                                ( ( lResult += ToJson<decltype( aMembers )> {}( aMembers, false ) + AppendComma( lMemberCount, ++lIndex ) ), ... );
                            }, aObject.Convert() );

                lResult += "}";

                return lResult;
            }

            if constexpr ( IsMember<Decayed_T>::value ) {
                lResult += "\"";
                lResult += std::string { aObject.Name } + "\":";
                lResult += ToJson<decltype( aObject.Value )> {}( aObject.Value, false );
                return lResult;
            }

            if constexpr ( IsArray<Decayed_T>::value ) {
                lResult      += "[";
                size_t lIndex = 0;

                for ( const auto& lItem : aObject ) {
                    lResult += ToJson<decltype( lItem )> {}( lItem, false, true );

                    if ( ++lIndex != aObject.size() ) {
                        lResult += ",";
                    }
                }

                lResult += "]";
                return lResult;
            }

            if constexpr ( !IsComplexType() ) {
                return ToSimpleValue<Decayed_T> {}( aObject );
            }
        }
    };

    template<typename OBJECT>
    class FromJson final
    {
    private:
        OBJECT mObject;

    private:
        template<typename MEMBERTYPE>
        MEMBERTYPE ExtractArray( const std::vector<std::any>& aSourceArray )
        {
            if constexpr ( IsArray<MEMBERTYPE>::value ) {
                using ArrayT = MEMBERTYPE::value_type;
                MEMBERTYPE lResult;

                for ( const auto& lItem : aSourceArray ) {
                    if ( lItem.type() == typeid( JsonObject::ArrayType ) ) {
                        using InnerArrayType = MEMBERTYPE::value_type;
                        JsonArray lSource    = std::any_cast<JsonArray>( lItem );

                        if constexpr ( IsArray<InnerArrayType>::value ) {
                            InnerArrayType lInnerResult = ExtractArray<InnerArrayType>( lSource );
                            lResult.push_back( lInnerResult );
                        }
                    }

                    if ( lItem.type() == typeid( ArrayT ) ) {
                        lResult.push_back( std::any_cast<ArrayT>( lItem ) );
                    }

                    if ( lItem.type() != typeid( ArrayT ) ) {
                        if constexpr ( std::is_same_v<ArrayT, uint64_t>) {
                            if ( lItem.type() == typeid( int64_t ) ) {
                                int64_t lItemValue = std::any_cast<int64_t>( lItem );

                                if ( lItemValue >= 0 ) {
                                    lResult.push_back( lItemValue );
                                }
                            }
                        }

                        if constexpr ( std::is_same_v<ArrayT, int64_t>) {
                            if ( lItem.type() == typeid( uint64_t ) ) {
                                uint64_t           lItemValue     = std::any_cast<uint64_t>( lItem );
                                constexpr uint64_t lMaxInt64Value = static_cast<uint64_t>( std::numeric_limits<int64_t>::max() );

                                if ( lMaxInt64Value > lItemValue ) {
                                    lResult.push_back( lItemValue );
                                }
                            }
                        }
                    }

                    if ( lItem.type() == typeid( JsonObject ) ) {
                        if constexpr ( HasConvertRef<ArrayT>::value ) {
                            ArrayT lArrayItem;

                            auto lValuesAsTuple = lArrayItem.ConvertRef();
                            std::apply( [ & ] (auto&... aArgs)
                                        {
                                            ( Process<decltype( aArgs )>( std::forward<decltype( aArgs )>( aArgs ), std::any_cast<JsonObject>( lItem ) ), ... );
                                        }, lValuesAsTuple );
                            lResult.push_back( lArrayItem );
                        }
                    }
                }

                return lResult;
            }
        }

        template<typename OBJECT>
        void ProcessObject( OBJECT&&          aObject,
                            const JsonObject& aJsonObject )
        {
            std::optional<JsonObject> lJsonObject = aJsonObject.GetOpt<JsonObject>( std::string { aObject.Name() } );

            if ( !lJsonObject.has_value() ) {
                return;
            }

            auto lValuesAsTuple = aObject.ConvertRef();
            std::apply( [ & ] (auto&... aArgs)
                        {
                            ( Process<decltype( aArgs )>( std::forward<decltype( aArgs )>( aArgs ), lJsonObject.value() ), ... );
                        }, lValuesAsTuple );
        }

        template<typename MEMBER>
        void Process( MEMBER&&          aMember,
                      const JsonObject& aJsonObject )
        {
            if constexpr ( IsObject<std::decay_t<MEMBER>>::value ) {
                ProcessObject( std::forward<MEMBER>( aMember ), aJsonObject );
            }

            if constexpr ( IsMember<std::decay_t<MEMBER>>::value ) {
                using MemberT = std::decay_t<MEMBER>::Type;

                if constexpr ( IsArray<MemberT>::value ) {
                    std::optional<JsonObject::ArrayType> lArray = aJsonObject.GetOpt<JsonObject::ArrayType>( std::string { aMember.Name } );

                    if ( lArray.has_value() ) {
                        MemberT lResult = ExtractArray<MemberT>( lArray.value() );
                        aMember.Value   = lResult;
                    }
                }
                else {
                    std::optional<MemberT> lValue = aJsonObject.GetOpt<MemberT>( std::string { aMember.Name } );

                    if ( lValue.has_value() ) {
                        aMember.Value = lValue.value();
                    }
                }
            }
        }

    public:
        std::optional<std::decay_t<OBJECT>> operator ()( const std::string& aJsonDocument )
        {
            if constexpr ( !IsObject<OBJECT>::value ) {
                return std::nullopt;
            }

            JsonDocument lDocument;

            if ( !lDocument.Parse( aJsonDocument ) ) {
                return std::nullopt;
            }

            auto lValuesAsTuple = mObject.ConvertRef();
            std::apply( [ & ] (auto&... aArgs)
                        {
                            ( Process<decltype( aArgs )>( std::forward<decltype( aArgs )>( aArgs ), lDocument.Root ), ... );
                        }, lValuesAsTuple );

            return mObject;
        }
    };
}
