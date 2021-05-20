
#include <iostream>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <memory>
#include "reflect.hpp"

class Meta;

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

#include <cstring>
    
//Attribute information record
struct PropertyInfo
{
    std::string name;
    int offset;
    int size;
    PropertyInfo(const std::string& in_name, int in_offset, int in_size)
    {
        name = in_name;
        offset = in_offset;
        size = in_size;
    }
    PropertyInfo()
        :name("")
        ,offset(0)
        ,size(0)
    {
    }

    template<typename T, typename U>
    void set_value(T& owner, U value)
    {
        std::memcpy((char*)&owner + offset, &value, size);
    }
};
    
// class Meta;
// template<typename T>
// void AddProperty( Meta* owner, T* property, const char* name )
// {
//     PropertyInfo newProperty =  PropertyInfo(name, (char*)(property) - (char*)owner, sizeof(T));
//     owner->m_info.push_back(newProperty);
// }
    
// template<typename T>
// void SetProperty(Meta* owner, const std::string& name, T value)
// {
//     for (std::vector<PropertyInfo>::iterator it = owner->m_info.begin(); it != owner->m_info.end(); ++it)
//     {
//         if (it->name == name)
//         {
//             std::memcpy( (char*)owner + it->offset, &value, sizeof(T));
//         }
//     }
// }

namespace reflect
{
    class IKlass
    {
    public:
        virtual PropertyInfo get_property(const std::string& name) = 0;
    };

    template<typename T>
    class Klass : public IKlass
    {
    private:
    public:
        Klass(const std::string& name) : m_strName(name), m_lSize(sizeof(T))
        {
        }
        
        ~Klass()
        {
        }

        template<typename TVar>
        Klass<T>& prop(const std::string& name, TVar (T::*gettter)() const)
        {
            // auto getter_func = [gettter](VM* vm, int ac, Value* av) -> Value
            // {
            //     T* obj = utils::argp<T>(ac, av, -1);
            //     if (obj) {
            //         TVar v = (obj->*gettter)();
            //         return v;
            //     }
            //     return Fox_Nil;
            // };
            return *this;
        }

        template<typename TVar>
        Klass<T>& prop(const std::string& name, TVar (T::*gettter)() const, void (T::*settter)(TVar))
        {
            // auto getter_func = [gettter]()
            // {
            //     return 0;
            // };

            // auto setter_func = [settter]()
            // {
            //     return 0;
            // };
            return *this;
        }

        template<typename TVar>
        Klass<T>& field(const std::string& name, TVar T::*variable)
        {
            PropertyInfo newProperty =  PropertyInfo(name, my_offsetof(variable), sizeof(TVar));
            m_vProps.push_back(newProperty);
            return *this;
        }

        // Marks the type as default constrctable, and stores a functions that could call constrctor/desctructor. This could be detected automatically I guess.
        Klass<T>& constructable()
        {            
            constructorFn = [](void* dest)
            {
                new((T*)dest) T();
            };

            destructorFn = [](void* dest)
            {
                ((T*)dest)->~T();
            };

            return *this;
        }

        // Marks the class as copyable and stores a function that can call that assign operator. This could be detected automatically I guess.
        Klass<T>& copyable()
        {
            copyFn = [](void* dest, const void* src)
            {
                *(T*)(dest) = *(T*)(src);
            };

            return *this;
        }

        // Marks that the class is comaprable and stores a function that can call operator==. This could be detected automatically I guess.
        Klass<T>& compareable()
        {
            equalsFn = [](const void* a, const void* b) -> bool
            {
                return *(T*)(a) == *(T*)(b);
            };

            return *this;
        }

        virtual PropertyInfo get_property(const std::string& name) override
        {
            for (auto it = m_vProps.begin(); it != m_vProps.end(); ++it)
            {
                if (it->name == name)
                {
                    // std::memcpy( (char*)owner + it->offset, &value, sizeof(T));
                    return *it;
                }
            }
            return PropertyInfo("name", 15, sizeof(T));
        }

        std::string name() const
        {
            return m_strName;
        }

        std::size_t type_size() const
        {
            return m_lSize;
        }

    private:
        std::string m_strName;
        std::vector<PropertyInfo> m_vProps;
        std::size_t m_lSize;

        void (*copyFn)(void* dest, const void* src) = nullptr;
        bool (*equalsFn)(const void* a, const void* b) = nullptr;
        void (*constructorFn)(void* dest) = nullptr;
        void (*destructorFn)(void* dest) = nullptr;
    };

    enum class MetaType
    {
        UNKNOWN,
        CLASS
    };
}

static std::vector<Meta> m_vMetas;

class Meta
{
public:
    // void Describle()
    // {
    //     AddProperty(this, &name, "name");
    //     AddProperty(this, &id, "id");
    // }
    // int GetId(){return id;}
    // std::string GetName() const
    // {
    //     return name;
    // }
    // Meta(const char* in_name,int in_id)
    //     :name(in_name)
    //     ,id(in_id)
    // {
    
    // }

    Meta(reflect::MetaType eType, const std::string& strName, std::size_t lSize)
        : m_eType(eType), m_strName(strName), m_lSize(lSize), klass_data(nullptr)
    {
    }

    Meta(reflect::MetaType eType, const std::string& strName, std::size_t lSize, std::shared_ptr<reflect::IKlass> data)
        : m_eType(eType), m_strName(strName), m_lSize(lSize), klass_data(std::move(data))
    {
    }

    ~Meta()
    {
    }

    template<typename T>
    static Meta& get()
    {
        for (auto it = m_vMetas.begin(); it != m_vMetas.end(); it++)
        {
            if (it->m_lSize == sizeof(T))
                return *it;
        }

        throw std::runtime_error("Cannot acces a non-registered reflect type");
    }

    template<typename T>
    static void define(const reflect::Klass<T>& klass)
    {
        // reflect::Klass<T>& rem_cast = const_cast<reflect::Klass<T>&>(klass);
        reflect::IKlass* rem_cast = new reflect::Klass<T>(klass);
        // if (std::is_class<T>::value)
        //     return Meta(reflect::MetaType::CLASS, name, sizeof(T));
        // return Meta(reflect::MetaType::UNKNOWN, name, sizeof(T));
        m_vMetas.push_back(Meta(reflect::MetaType::CLASS, klass.name(), klass.type_size(), std::shared_ptr<reflect::IKlass>(rem_cast)));
    }

    PropertyInfo get_property(const std::string& name)
    {
        if (m_eType == reflect::MetaType::CLASS && klass_data)
        {
            return klass_data->get_property(name);
            // return PropertyInfo(name, 15, sizeof(Some));
        }
        // throw std::runtime_error("sdvdsb");
    }

private:
    reflect::MetaType m_eType;
    std::shared_ptr<reflect::IKlass> klass_data;
    std::string m_strName;
    int offset;
    std::size_t m_lSize;
};

int main()
{
    // constexpr auto wrapped = wrap_lambda([](int i) { return i * 2; });

    // // Initializer lists works too, we are not using templates deduction.
    // static_assert(wrapped({4}) == 8);

    // // We make a deffered lambda
    // auto func = make_deferred(
    //     [](int a, std::vector<int>& b, double c) {
    //         std::cout << a << '\n';

    //         for (auto&& i : b) {
    //             std::cout << ' ' << i;
    //         }

    //         std::cout << '\n' << c << '\n';
    //     }
    // );
    
    // auto vec = std::vector{1, 2, 3, 4, 5, 42};
    
    // // Bind the parameters to our function
    // // func.bind(12, vec, 5.4);
    
    // // Call the function with bound parameters:
    // func(12, vec, 5.4);

    // auto func2 = make_deferred(
    //     []() -> int {
    //         std::cout << "Hello!" << '\n';
    //         return 251454;
    //     }
    // );

    // // Bind the parameters to our function
    // // func2.bind();
    
    // // Call the function with bound parameters:
    // std::cout << "f" << func2() << std::endl;

    // magic_call(
    // [](Some, Some2, int i, double d, auto&& s1, auto&& s2  ) {
    //     std::cout << i << std::endl;
    //     std::cout << d << std::endl;
    //     std::cout << s1 << std::endl;
    //     std::cout << s2 << std::endl;
    // },
    //     /*magic*/  /*magic*/  4,   5.4,    "str1", "str2"
    // );

    // ReflectObj ref("my", 4);
    // ref.Describle();

    // std::cout << "Ref.name: " << ref.GetName() << std::endl;
    // std::cout << "Ref.id: " << ref.GetId() << std::endl;

    // SetProperty(&ref, "name", "mine");
    // SetProperty(&ref, "id", 7);

    // std::cout << " --- NEW --- " << std::endl;
    // std::cout << "Ref.name: " << ref.GetName() << std::endl;
    // std::cout << "Ref.id: " << ref.GetId() << std::endl;

    // Meta::define(reflect::Klass<Some>("Some")
    //     //  .constructor<>()
    //     .field("data", &Some::data)
    //     .field("data2", &Some::data2));
    // Meta& type = Meta::get<Some>();
    Some obj;
    // std::cout << "Property Name: " << type.get_property("data").name << std::endl;
    // std::cout << "Property Value: " << obj.data << std::endl;
    // type.get_property("data").set_value(obj, 10);
    // std::cout << "Property Value: " << obj.data << std::endl;
    // type.get_property("data").set_value(obj, 455454946);
    // std::cout << "Property Value: " << obj.data << std::endl;

    // std::cout << "Property Value2: " << obj.data2.data << std::endl;
    // type.get_property("data2").set_value(obj, Some2{10});
    // std::cout << "Property Value2: " << obj.data2.data << std::endl;
    // type.get_property("data2").set_value(obj, Some2{455454946});
    // std::cout << "Property Value2: " << obj.data2.data << std::endl;

    auto type = reflect::get<Some>();
    auto property = type.get_property<int>("data");

    std::cout << property.get(obj) << std::endl;
    property.set(obj, 45);
    std::cout << property.get(obj) << std::endl;
}
