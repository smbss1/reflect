/* -----------------------------------------------------------------------------------------------

refl::register_type<T> is used for class registration and it has the following form when specialized:

namespace refl
{
    template <>
    auto register_type<YourClass>()
    {
        return members(
            field(...),
            ...
        );
    }
}

-------------------------------------------------------------------------------------------------*/

#ifndef FOX_META_HPP_
#define FOX_META_HPP_

#include <type_traits>
#include <tuple>
#include <utility>

// type_list is array of types
template <typename... Args>
struct type_list
{
    template <std::size_t N>
    using type = std::tuple_element_t<N, std::tuple<Args...>>;
    using indices = std::index_sequence_for<Args...>;
    static const size_t size = sizeof...(Args);
};

namespace refl
{
    template <typename Class>
    auto get();

    template <typename... Args>
    auto members(Args&&... args);

    // function used for registration of classes by user
    template <typename Class>
    inline auto register_type();

    // function used for registration of class name by user
    template <typename Class>
    constexpr auto register_name();

    // returns set name for class
    template <typename Class>
    constexpr auto get_name();

    // returns the number of registered members of the class
    template <typename Class>
    constexpr std::size_t get_member_count();

    // returns std::tuple of Members
    template <typename Class>
    auto& get_members();

    // Check if class has register_type<T> specialization (has been registered)
    template <typename Class>
    constexpr bool is_registered();

    // Check if Class has non-default ctor registered
    template <typename Class>
    constexpr bool ctor_registered();

    template <typename T>
    struct constructor_args {
        using types = type_list<>;
    };

    template <typename T>
    using constructor_arguments = typename constructor_args<T>::types;

    // Check if user registered non default constructor
    template <typename Class>
    constexpr bool ctor_registered();

    // Check if class T has member
    template <typename Class>
    bool has_member(const char* name);


    template <typename Class, typename F,
        typename = std::enable_if_t<is_registered<Class>()>>
    void foreach_members(F&& f);

    // version for non-registered classes (to generate less template stuff)
    template <typename Class, typename F,
        typename = std::enable_if_t<!is_registered<Class>()>,
        typename = void>
    void foreach_members(F&& f);

    // Do F for member named 'name' with type T. It's important to pass correct type of the member
    template <typename Class, typename T, typename F>
    void for_member(const char* name, F&& f);

    // Get value of the member named 'name'
    template <typename T, typename Class>
    T get_member_value(Class& obj, const char* name);

    // Set value of the member named 'name'
    template <typename T, typename Class, typename V,
        typename = std::enable_if_t<std::is_constructible<T, V>::value>>
    void set_member_value(Class& obj, const char* name, V&& value);

}

#include "Meta.inl"

#endif