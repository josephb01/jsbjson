#pragma once

#include <tuple>
#include <iostream>
#include <utility>

namespace jsbjson
{
    template<typename TUPLE, typename FUNC, std::size_t... IS>
    void VisitTupleImpl( const TUPLE& lTuple,
                         FUNC&&       lFunc,
                         std::index_sequence<IS...> )
    {
        ( lFunc( std::get<IS>( lTuple ) ), ... );
    }

    template<typename TUPLE, typename FUNC>
    void VisitTuple( const TUPLE& lTuple,
                     FUNC&&       lFunc )
    {
        constexpr std::size_t lTupleSize = std::tuple_size_v<TUPLE>;
        VisitTupleImpl( lTuple, std::forward<FUNC>( lFunc ), std::make_index_sequence<lTupleSize> {} );
    }
}
