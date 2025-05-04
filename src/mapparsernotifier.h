#pragma once

#include <variant>
#include <string>
#include <cstdint>
#include <iostream>
#include <iomanip>
#include <type_traits>
#include <list>
#include <map>
#include <any>
#include "iparsernotifier.h"
#include "typehelpers.h"

namespace jsbjson
{
    class MapParserNotifier : public IParserNotifier
    {
    public:
        MapParserNotifier( JsonElement& aResult )
            : mRoot( aResult )
        {}

    public:
        void OnParsingStarted()
        {
            mObjects.clear();
            mArrayNames.clear();
            mObjectNames.clear();
            mParents.clear();
        }

        void OnParsingFinished()
        {
            if ( mObjects.size() != 1 ) {
                return;
            }

            mRoot = mObjects.begin()->second;
        }

        void OnObjectBegin( const size_t       aID,
                            const size_t       aParentID,
                            const std::string& aName )
        {
            if ( mObjects.find( aParentID ) != mObjects.cend() ) {
                mObjectNames[ aID ] = aName;
            }

            mParents[ aID ] = aParentID;
            mObjects[ aID ] = JsonElement {};
        }

        void OnObjectFinished( const size_t aID )
        {
            const ID_t  lParent      = mParents[ aID ];
            const auto& lParentArray = mArrays.find( lParent );

            if ( lParentArray != mArrays.cend() ) {
                lParentArray->second.push_back( mObjects[ aID ] );
                mObjects.erase( aID );
                return;
            }

            const std::string& lObjectName   = mObjectNames[ aID ];
            const auto&        lParentObject = mObjects.find( lParent );

            if ( lParentObject != mObjects.cend() ) {
                lParentObject->second[ lObjectName ] = mObjects[ aID ];
                mObjects.erase( aID );
            }
        }

        void OnItemBegin( const size_t aParentID )
        {
        }

        void OnItemName( const std::string& aName,
                         const size_t       aParentID )
        {
        }

        void OnItemValueBegin( const IParserNotifier::eValueType aType )
        {
        }

        void OnItemValue( const std::variant<uint64_t, int64_t, double, bool, std::string> aValue,
                          const size_t                                                     aParentID,
                          const std::string&                                               aName )
        {
            if ( mObjects.find( aParentID ) != mObjects.cend() ) {
                std::visit( [ &, this ] (const auto& aItem)
                            {
                                mObjects[ aParentID ][ aName ] = aItem;
                            }, aValue );
            }

            if ( mArrays.find( aParentID ) != mArrays.end() ) {
                std::visit( [ &, this ] (const auto& aItem)
                            {
                                mArrays[ aParentID ].push_back( aItem );
                            }, aValue );
            }
        }

        virtual void OnArrayBegin( const size_t       aID,
                                   const size_t       aParentID,
                                   const std::string& aName )
        {
            mArrays[ aID ] = std::vector<JsonVariant> {};

            if ( !aName.empty() ) {
                mArrayNames[ aID ] = aName;
            }

            mParents[ aID ] = aParentID;
        }

        virtual void OnNextItem()
        {
        }

        virtual void OnArrayFinished( const size_t aID )
        {
            const ID_t  lParent       = mParents[ aID ];
            const auto& lParentObject = mObjects.find( lParent );

            const auto& lArrayItem = mArrays[ aID ];

            if ( lParentObject != mObjects.cend() ) {
                const std::string& lArrayName = mArrayNames[ aID ];

                lParentObject->second[ lArrayName ] = lArrayItem;

                mArrays.erase( aID );
                return;
            }

            const auto& lParentArray = mArrays.find( lParent );

            if ( lParentArray == mArrays.cend() ) {
                // Should not happen
                return;
            }

            lParentArray->second.push_back( lArrayItem );

            mArrays.erase( aID );
        }

        virtual void OnError( const std::string& aErrorMessage )
        {
            std::cout << "ERRORR!!!" << aErrorMessage << std::endl;
        };

    private:
        JsonElement& mRoot;
        using ID_t = size_t;
        std::map<ID_t, JsonElement>              mObjects;
        std::map<ID_t, std::vector<JsonVariant>> mArrays;
        std::map<ID_t, ID_t>                     mParents;                                                                                 /*!<first: the child id, second: the parent id*/
        std::map<ID_t, std::string>              mArrayNames;
        std::map<ID_t, std::string>              mObjectNames;

    private:
    };
}
