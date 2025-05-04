#pragma once

#include <vector>
#include <list>
#include <unordered_map>
#include <variant>
#include <cstdint>
#include <type_traits>

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
}
