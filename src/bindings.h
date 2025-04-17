#pragma once

#include <tuple>

template<typename std::size_t>
struct ToTuple {};

template<>
struct ToTuple<1>
{
    template<typename STRUCT>
    auto operator ()( STRUCT&& aStruct )
    {
        auto [ M0 ] = std::forward<STRUCT>( aStruct );
        return std::make_tuple( M0 );
    }
};

template<>
struct ToTuple<2>
{
    template<typename STRUCT>
    auto operator ()( STRUCT&& aStruct )
    {
        auto [ M0, M1 ] = std::forward<STRUCT>( aStruct );
        return std::make_tuple( M0, M1 );
    }
};

template<>
struct ToTuple<3>
{
    template<typename STRUCT>
    auto operator ()( STRUCT&& aStruct )
    {
        auto [ M0, M1, M2 ] = std::forward<STRUCT>( aStruct );
        return std::make_tuple( M0, M1, M2 );
    }
};

template<>
struct ToTuple<4>
{
    template<typename STRUCT>
    auto operator ()( STRUCT&& aStruct )
    {
        auto [ M0, M1, M2, M3 ] = std::forward<STRUCT>( aStruct );
        return std::make_tuple( M0, M1, M2, M3 );
    }
};

template<>
struct ToTuple<5>
{
    template<typename STRUCT>
    auto operator ()( STRUCT&& aStruct )
    {
        auto [ M0, M1, M2, M3, M4 ] = std::forward<STRUCT>( aStruct );
        return std::make_tuple( M0, M1, M2, M3, M4 );
    }
};

template<>
struct ToTuple<6>
{
    template<typename STRUCT>
    auto operator ()( STRUCT&& aStruct )
    {
        auto [ M0, M1, M2, M3, M4, M5 ] = std::forward<STRUCT>( aStruct );
        return std::make_tuple( M0, M1, M2, M3, M4, M5 );
    }
};

template<>
struct ToTuple<7>
{
    template<typename STRUCT>
    auto operator ()( STRUCT&& aStruct )
    {
        auto [ M0, M1, M2, M3, M4, M5, M6 ] = std::forward<STRUCT>( aStruct );
        return std::make_tuple( M0, M1, M2, M3, M4, M5, M6 );
    }
};

template<>
struct ToTuple<8>
{
    template<typename STRUCT>
    auto operator ()( STRUCT&& aStruct )
    {
        auto [ M0, M1, M2, M3, M4, M5, M6, M7 ] = std::forward<STRUCT>( aStruct );
        return std::make_tuple( M0, M1, M2, M3, M4, M5, M6, M7 );
    }
};

template<>
struct ToTuple<9>
{
    template<typename STRUCT>
    auto operator ()( STRUCT&& aStruct )
    {
        auto [ M0, M1, M2, M3, M4, M5, M6, M7, M8 ] = std::forward<STRUCT>( aStruct );
        return std::make_tuple( M0, M1, M2, M3, M4, M5, M6, M8 );
    }
};

template<>
struct ToTuple<10>
{
    template<typename STRUCT>
    auto operator ()( STRUCT&& aStruct )
    {
        auto [ M0, M1, M2, M3, M4, M5, M6, M7, M8, M9 ] = std::forward<STRUCTS>( aStruct );
        return std::make_tuple( M0, M1, M2, M3, M4, M5, M6, M7, M8, M9 );
    }
};

template<typename std::size_t>
struct ToRefTuple {};

template<>
struct ToRefTuple<1>
{
    template<typename STRUCT>
    auto operator ()( STRUCT&& aStruct )
    {
        auto& [ M0 ] = std::forward<STRUCT>( aStruct );
        return std::forward_as_tuple( M0 );
    }
};

template<>
struct ToRefTuple<2>
{
    template<typename STRUCT>
    auto operator ()( STRUCT&& aStruct )
    {
        auto& [ M0, M1 ] = std::forward<STRUCT>( aStruct );
        return std::forward_as_tuple( M0, M1 );
    }
};

template<>
struct ToRefTuple<3>
{
    template<typename STRUCT>
    auto operator ()( STRUCT&& aStruct )
    {
        auto& [ M0, M1, M2 ] = std::forward<STRUCT>( aStruct );
        return std::forward_as_tuple( M0, M1, M2 );
    }
};

template<>
struct ToRefTuple<4>
{
    template<typename STRUCT>
    auto operator ()( STRUCT&& aStruct )
    {
        auto& [ M0, M1, M2, M3 ] = std::forward<STRUCT>( aStruct );
        return std::forward_as_tuple( M0, M1, M2, M3 );
    }
};

template<>
struct ToRefTuple<5>
{
    template<typename STRUCT>
    auto operator ()( STRUCT&& aStruct )
    {
        auto& [ M0, M1, M2, M3, M4 ] = std::forward<STRUCT>( aStruct );
        return std::forward_as_tuple( M0, M1, M2, M3, M4 );
    }
};

template<>
struct ToRefTuple<6>
{
    template<typename STRUCT>
    auto operator ()( STRUCT&& aStruct )
    {
        auto& [ M0, M1, M2, M3, M4, M5 ] = std::forward<STRUCT>( aStruct );
        return std::forward_as_tuple( M0, M1, M2, M3, M4, M5 );
    }
};

template<>
struct ToRefTuple<7>
{
    template<typename STRUCT>
    auto operator ()( STRUCT&& aStruct )
    {
        auto& [ M0, M1, M2, M3, M4, M5, M6 ] = std::forward<STRUCT>( aStruct );
        return std::forward_as_tuple( M0, M1, M2, M3, M4, M5, M6 );
    }
};

template<>
struct ToRefTuple<8>
{
    template<typename STRUCT>
    auto operator ()( STRUCT&& aStruct )
    {
        auto& [ M0, M1, M2, M3, M4, M5, M6, M7 ] = std::forward<STRUCT>( aStruct );
        return std::forward_as_tuple( M0, M1, M2, M3, M4, M5, M6, M7 );
    }
};

template<>
struct ToRefTuple<9>
{
    template<typename STRUCT>
    auto operator ()( STRUCT&& aStruct )
    {
        auto& [ M0, M1, M2, M3, M4, M5, M6, M7, M8 ] = std::forward<STRUCT>( aStruct );
        return std::forward_as_tuple( M0, M1, M2, M3, M4, M5, M6, M8 );
    }
};

template<>
struct ToRefTuple<10>
{
    template<typename STRUCT>
    auto operator ()( STRUCT&& aStruct )
    {
        auto& [ M0, M1, M2, M3, M4, M5, M6, M7, M8, M9 ] = std::forward<STRUCTS>( aStruct );
        return std::forward_as_tuple( M0, M1, M2, M3, M4, M5, M6, M7, M8, M9 );
    }
};
