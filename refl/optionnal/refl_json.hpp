

#ifndef FOX_REFLECT_JSON_HPP_
#define FOX_REFLECT_JSON_HPP_

#include <string>
#include <vector>
#include <unordered_map>

#include "json.hpp"
#include "refl.hpp"
// #include "StringCast.h"

using json = fox::json;

namespace refl
{

    /////////////////// SERIALIZATION

    template <typename Class,
        typename = std::enable_if_t <refl::is_registered<Class>()>>
    json serialize(const Class& obj);

    template <typename Class,
        typename = std::enable_if_t <!refl::is_registered<Class>()>,
        typename = void>
    json serialize(const Class& obj);

    template <typename Class>
    json serialize_basic(const Class& obj);

    // specialization for std::vector
    template <typename T>
    json serialize_basic(const std::vector<T>& obj);

    // specialization for std::unodered_map
    template <typename K, typename V>
    json serialize_basic(const std::unordered_map<K, V>& obj);


    /////////////////// DESERIALIZATION
    //
    //template<typename Class>
    //Class deserialize(const json& obj);

    template <typename Class,
        typename = std::enable_if_t<refl::is_registered<Class>()>>
    void deserialize(Class& obj, const json& object);

    template <typename Class,
        typename = std::enable_if_t<!refl::is_registered<Class>()>,
        typename = void>
    void deserialize(Class& obj, const json& object);

}

namespace fox
{
    template <typename T>
    struct Serializer
    {
        static void serialize(json& j, const T& value)
        {
            j = refl::serialize(value);
        }

        static void deserialize(const json& j, T& value)
        {
            refl::deserialize(value, j);
        }
    };
}

#include "refl_json.inl"

#endif