#pragma once

#include <vector>
#include <list>

namespace jsbjson
{
    template<typename>
    struct IsArray : std::false_type {};

    template<typename T, typename...A>
    struct IsArray<std::vector<T, A...>>: std::true_type {};

    template<typename T, typename...A>
    struct IsArray<std::list<T, A...>>: std::true_type {};

    template<class T, class = void>
    struct IsMember : std::false_type {};

    template<class T>
    struct IsMember<T, std::void_t<decltype( T::IsAJsonMember )>>: std::true_type {};

    template<class T, class = void>
    struct IsObject : std::false_type {};

    template<class T>
    struct IsObject<T, std::void_t<decltype( T::IsAJsonObject )>>: std::true_type {};

    template<class T, class = void>
    struct HasConvertRef : std::false_type {};

    template<class T>
    struct HasConvertRef<T, std::void_t<decltype( std::declval<T&>().ConvertRef() )>>: std::true_type {};

    template<typename T, std::size_t Depth>
    struct VectorDepth
    {
        using type = typename VectorDepth<std::vector<std::vector<T>>, Depth - 1>::type;
    };


    template<typename T>
    struct VectorDepth<T, 1>
    {
        using type = std::vector<std::vector<T>>;
    };


    template<typename T>
    struct VectorDepth<T, 0>
    {
        using type = std::vector<T>;
    };

    template<typename T, std::size_t Depth>
    struct ListDepth
    {
        using type = typename ListDepth<std::list<std::list<T>>, Depth - 1>::type;
    };


    template<typename T>
    struct ListDepth<T, 1>
    {
        using type = std::list<std::list<T>>;
    };


    template<typename T>
    struct ListDepth<T, 0>
    {
        using type = std::list<T>;
    };
}
