
#include <iostream>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <memory>
#include "refl.hpp"
#include "refl_json.hpp"

struct Some2 {
    int data = 0;
};

class Some
{
public:
    int data = 0;
    std::string name;
    Some2 data2;

    std::unordered_map<std::string, int> get_anmis() const
    {
        return m_vAnims;
    }

    void set_anims(std::unordered_map<std::string, int>& anims)
    {

    }
    std::unordered_map<std::string, int> m_vAnims;

private:

};

namespace refl
{
    template <>
    auto register_type<Some>()
    {
        return members(
            field("data", &Some::data),
            field("data2", &Some::data2),
            field("name", &Some::name),
            field("anims", &Some::m_vAnims)
        );
    }

    template <>
    auto register_type<Some2>()
    {
        return members(
            field("data", &Some2::data)
        );
    }
}

int main()
{
    Some obj;

    auto type = refl::get<Some>();
    auto property = type.get_property<int>("data");

    std::cout << property.get(obj) << std::endl;
    property.set(obj, 45);
    std::cout << property.get(obj) << std::endl;
    
    json j;
    j = obj;
    std::cout << j << std::endl;

    Some obj2;
    obj2 = j.get<Some>();
    std::cout << obj2.data << std::endl;
}
