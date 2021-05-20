/* -----------------------------------------------------------------------------------------------

MetaHolder holds all Member objects constructed via reflect::register_type<T> call.
If the class is not registered, members is std::tuple<>

-------------------------------------------------------------------------------------------------*/

#ifndef FOX_METAHOLDER_HPP_
#define FOX_METAHOLDER_HPP_

#include <tuple>

namespace reflect
{
    namespace detail
    {

        template <typename T, typename TupleType>
        struct MetaHolder
        {
            static TupleType members;
            static const char* name() 
            {
                return register_name<T>();
            }

            struct Type
            {
                Type() {}
                ~Type() {}

                template <typename TVar>
                Member<T, TVar> get_property(const char* name)
                {
                    Member<T, TVar> property(name, nullptr);
                    for_member<T, TVar>(name,
                        [&property](const auto& member)
                        {
                            property = member;
                        }
                    );
                    return property;
                }
            };

            static Type type()
            {
                return Type();
            }
        };

        template <typename T, typename TupleType>
        TupleType MetaHolder<T, TupleType>::members = register_type<T>();

    } // end of namespace detail
} // end of namespace reflect

#endif