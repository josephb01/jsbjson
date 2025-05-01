#pragma once

#include <vector>
#include <list>
#include <unordered_map>
#include <cstdint>

namespace jsbjson
{
    using JsonElement = std::unordered_map<std::string, std::any>;

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
        using type = typename VectorDepth<std::vector<T>, Depth - 1>::type;
    };

    template<typename T>
    struct VectorDepth<T, 0>
    {
        using type = std::vector<T>;
    };

    template<typename T, std::size_t Depth>
    struct ListDepth
    {
        using type = typename ListDepth<std::list<T>, Depth - 1>::type;
    };

    template<typename T>
    struct ListDepth<T, 0>
    {
        using type = std::list<T>;
    };

    template<typename... ARGS>
    struct sVariantHelper
    {
        using Variant = std::variant<typename VectorDepth<ARGS, 0>::type..., typename ListDepth<ARGS, 0>::type..., typename VectorDepth<ARGS, 1>::type..., typename ListDepth<ARGS, 1>::type..., typename VectorDepth<ARGS, 2>::type..., typename ListDepth<ARGS, 2>::type...>;
    };

    using Variant                                = sVariantHelper<uint32_t, int32_t, int64_t, uint64_t, std::string, bool, double, JsonElement, std::any>::Variant;
    static constexpr const size_t kMaxArrayDepth = 2;                                                                                      // Arrays only can be maximum 3 dimensional (std::vector<std::vector<std::vector<int32_t>>>)

    static constexpr const size_t kMaxAnyArrayDepth = 2;
    template<typename T, std::size_t Depth>
    struct AnyVectorDepth
    {
        using type = typename AnyVectorDepth<std::vector<T>, Depth - 1>::type;
    };

    template<typename T>
    struct AnyVectorDepth<T, 1>
    {
        using type = std::vector<T>;
    };

    template<typename T>
    struct AnyVectorDepth<T, 0>
    {
        using type = T;
    };

    template<typename...ARGS>
    struct VectorVariantHelper
    {
        using Variant = std::variant<std::vector<typename AnyVectorDepth<ARGS, 0>::type>..., std::vector<typename AnyVectorDepth<ARGS, 1>::type>..., std::vector<typename AnyVectorDepth<ARGS, 2>::type>...>;
    };

    using VectorVariant = typename VectorVariantHelper<int32_t, uint32_t, int64_t, uint64_t, std::string, bool, double, JsonElement>::Variant;
}
