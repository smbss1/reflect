#include "reflect_json.hpp"

// namespace fox
// {
//     template <typename T>
//     void serialize(json& j, const T& obj)
//     {
//         j = reflect::serialize(obj);
//     }

//     template <typename T>
//     void deserialize(const json& j, T& obj)
//     {
//         reflect::deserialize(obj, j);
//     }
// }

namespace reflect
{
/////////////////// SERIALIZATION

    template <typename Class,
        typename>
    json serialize(const Class& obj)
    {
        json value;
        reflect::foreach_members<Class>(
            [&obj, &value](auto& member)
            {
                auto& valueName = value[member.get_name()];
                if (member.canGetConstRef()) {
                    valueName = member.get(obj);
                } else if (member.hasGetter()) {
                    valueName = member.getCopy(obj); // passing copy as const ref, it's okay
                }
            }
        );
        return value;
    }

    template <typename Class,
        typename, typename>
    json serialize(const Class& obj)
    {
        return serialize_basic(obj);
    }

    template <typename Class>
    json serialize_basic(const Class& obj)
    {
        return json(obj);
    }

    /////////////////// DESERIALIZATION

    template <typename Class>
    Class deserialize(const json& obj)
    {
        Class c;
        deserialize(c, obj);
        return c;
    }

    template <typename Class,
        typename>
    void deserialize(Class& obj, const json& object)
    {
        if (object.is_object()) {
            reflect::foreach_members<Class>(
                [&obj, &object](auto& member)
                {
                    auto& objName = object[member.get_name()];
                    if (!objName.is_null()) {
                        using MemberT = reflect::get_member_type<decltype(member)>;
                        if (member.hasSetter()) {
                            member.set(obj, objName.template get<MemberT>());
                        } else if (member.canGetRef()) {
                            member.getRef(obj) = objName.template get<MemberT>();
                        } else {
                            throw std::runtime_error("Error: can't deserialize member because it's read only");
                        }
                    }
                }
            );
        } else {
            throw std::runtime_error("Error: can't deserialize from Json::json to Class.");
        }
    }

    template <typename Class,
        typename, typename>
    void deserialize(Class& obj, const json& object)
    {
        obj = object.get<Class>();
    }

}
