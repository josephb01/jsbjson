#pragma once
#include <optional>
#include <vector>
#include "parser.h"
#include "typehelpers.h"
#include "mapparsernotifier.h"

namespace jsbjson
{
    template<typename OBJECT>
    class ToObject final
    {
    private:
        OBJECT mObject;

    private:
        template<typename MEMBERTYPE>
        MEMBERTYPE ExtractArray( const std::vector<JsonVariant>& aSourceArray )
        {
            if constexpr ( IsArray<MEMBERTYPE>::value ) {
                using ArrayItemType = MEMBERTYPE::value_type;
                MEMBERTYPE lResult;

                for ( const auto& lItem : aSourceArray ) {
                    if ( lItem.IsArray() ) {
                        using InnerArrayType = MEMBERTYPE::value_type;

                        if constexpr ( IsArray<InnerArrayType>::value ) {
                            InnerArrayType lInnerResult = ExtractArray<InnerArrayType>( lItem.GetValue<std::vector<JsonVariant>>().value() );
                            lResult.push_back( lInnerResult );
                        }

                        continue;
                    }

                    if ( lItem.IsA<JsonElement>() ) {
                        if constexpr ( HasConvertRef<ArrayItemType>::value ) {
                            ArrayItemType lArrayItem;

                            auto lValuesAsTuple = lArrayItem.ConvertRef();
                            std::apply( [ & ] (auto&... aArgs)
                                        {
                                            ( Process<decltype( aArgs )>( std::forward<decltype( aArgs )>( aArgs ), lItem.GetValue<JsonElement>().value() ), ... );
                                        }, lValuesAsTuple );
                            lResult.push_back( lArrayItem );
                        }

                        continue;
                    }

                    const std::optional<ArrayItemType> lValueOpt = lItem.GetValue<ArrayItemType>();

                    if ( lValueOpt.has_value() ) {
                        lResult.push_back( lValueOpt.value() );
                    }
                }

                return lResult;
            }
        }

        template<typename OBJECT>
        void ProcessObject( OBJECT&&           aObject,
                            const JsonElement& aJsonObject )
        {
            const auto& lItemIt = aJsonObject.find( aObject.Name() );

            if ( lItemIt == aJsonObject.cend() ) {
                return;
            }

            const JsonVariant& lItem = lItemIt->second;

            if ( !lItem.IsA<JsonElement>() ) {
                return;
            }

            std::optional<JsonElement> lElement = lItem.GetValue<JsonElement>();

            if ( !lElement.has_value() ) {
                return;
            }

            auto lValuesAsTuple = aObject.ConvertRef();
            std::apply( [ & ] (auto&... aArgs)
                        {
                            ( Process<decltype( aArgs )>( std::forward<decltype( aArgs )>( aArgs ), lElement.value() ), ... );
                        }, lValuesAsTuple );
        }

        template<typename MEMBER>
        void Process( MEMBER&&           aMember,
                      const JsonElement& aJsonObject )
        {
            if constexpr ( IsObject<std::decay_t<MEMBER>>::value ) {
                ProcessObject( std::forward<MEMBER>( aMember ), aJsonObject );
            }

            if constexpr ( IsMember<std::decay_t<MEMBER>>::value ) {
                using MemberT = std::decay_t<MEMBER>::Type;

                if constexpr ( IsArray<MemberT>::value ) {
                    const auto& lItemIt = aJsonObject.find( aMember.Name() );

                    if ( lItemIt == aJsonObject.cend() ) {
                        return;
                    }

                    if ( !lItemIt->second.IsArray() ) {
                        return;
                    }

                    std::optional<std::vector<JsonVariant>> lArray = lItemIt->second.GetValue<std::vector<JsonVariant>>();

                    if ( lArray.has_value() ) {
                        MemberT lResult = ExtractArray<MemberT>( lArray.value() );
                        aMember.Value   = lResult;
                    }
                }
                else {
                    const auto& lItemIt = aJsonObject.find( aMember.Name() );

                    if ( lItemIt == aJsonObject.cend() ) {
                        return;
                    }

                    const std::optional<MemberT> lValue = lItemIt->second.GetValue<MemberT>();

                    if ( lValue.has_value() ) {
                        aMember.Value = lValue.value();
                    }
                    else {
                        int x = 3;
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

            JsonParser  lParser;
            JsonElement lResult;

            if ( !lParser.Parse( aJsonDocument, std::make_shared<jsbjson::MapParserNotifier>( lResult ) ) ) {
                return std::nullopt;
            }

            auto lValuesAsTuple = mObject.ConvertRef();
            std::apply( [ & ] (auto&... aArgs)
                        {
                            ( Process<decltype( aArgs )>( std::forward<decltype( aArgs )>( aArgs ), lResult ), ... );
                        }, lValuesAsTuple );

            return mObject;
        }
    };
}
