#include <cassert>
#include <tuple>
#include <cstring>

#include "Member.hpp"
#include "detail/template_helpers.hpp"
#include "detail/MetaHolder.hpp"

namespace reflect
{
    template <typename... Args>
    auto members(Args&&... args)
    {
        // just this... but may become more complex later, who knows!
        //  Still, better no to expose too much to end-user.
        return std::make_tuple(std::forward<Args>(args)...);
    }

    template <typename Class>
    inline auto register_type()
    {
        return std::make_tuple();
    }

    template <typename Class>
    constexpr auto register_name()
    {
        return "";
    }

    template <typename Class>
    constexpr auto get_name()
    {
        return detail::MetaHolder<Class, decltype(register_type<Class>())>::name();
    }

    template <typename Class>
    constexpr std::size_t get_member_count()
    {
        return std::tuple_size<decltype(register_type<Class>())>::value;
    }

    template <typename Class>
    auto& get_members()
    {
        return detail::MetaHolder<Class, decltype(register_type<Class>())>::members;
    }

    template <typename Class>
    constexpr bool is_registered()
    {
        return !std::is_same<std::tuple<>, decltype(register_type<Class>())>::value;
    }

    // Check if Class has non-default ctor registered
    template <typename Class>
    constexpr bool ctor_registered()
    {
        return !std::is_same<type_list<>, constructor_arguments<Class>>::value;
    }

    template <typename Class>
    bool has_member(const char* name)
    {
        bool found = false;
        foreach_members<Class>(
            [&found, &name](const auto& member)
            {
                if (!strcmp(name, member.get_name())) {
                    found = true;
                }
            }
        );
        return found;
    }

    template <typename Class, typename F, typename>
    void foreach_members(F&& f)
    {
        detail::for_tuple(std::forward<F>(f), get_members<Class>());
    }

    // version for non-registered classes (to generate less template stuff)
    template <typename Class, typename F,
        typename, typename>
        void foreach_members(F&& /*f*/)
    {
        // do nothing! Nothing gets generated
    }

    template <typename Class, typename T, typename F>
    void for_member(const char* name, F&& f)
    {
        foreach_members<Class>(
            [&](const auto& member)
            {
                if (!strcmp(name, member.get_name())) {
                    using MemberT = reflect::get_member_type<decltype(member)>;
                    assert((std::is_same<MemberT, T>::value) && "Member doesn't have type T");
                    detail::call_if<std::is_same<MemberT, T>::value>(std::forward<F>(f), member);
                }
            }
        );
    }

    template <typename T, typename Class>
    T get_member_value(Class& obj, const char* name)
    {
        T value;
        for_member<Class, T>(name,
            [&value, &obj](const auto& member)
            {
                value = member.getCopy(obj);
            }
        );
        return value;
    }

    template <typename Class>
    auto get()
    {
        return detail::MetaHolder<Class, decltype(register_type<Class>())>::type();
    }

    template <typename T, typename Class, typename V,
        typename>
    void set_member_value(Class& obj, const char* name, V&& value)
    {
        for_member<Class, T>(name,
            [&obj, value = std::forward<V>(value)](const auto& member)
            {
                member.set(obj, value);
            }
        );
    }

} // end of namespace reflect
