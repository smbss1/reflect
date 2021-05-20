
#include <iostream>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <memory>
#include "reflect.hpp"

struct Some2 {
    int data = 0;
};

struct Some
{
    int data = 0;
    std::string name;
    Some2 data2;

    void func(int d)
    {
        std::cout << "d = " << d << std::endl;
    }
};

namespace reflect
{
    template <>
    auto register_type<Some>()
    {
        return members(
            field("data", &Some::data),
            field("data2", &Some::data2),
            field("name", &Some::name)
        );
    }
}

int main()
{
    Some obj;

    auto type = reflect::get<Some>();
    auto property = type.get_property<int>("data");

    std::cout << property.get(obj) << std::endl;
    property.set(obj, 45);
    std::cout << property.get(obj) << std::endl;
}
