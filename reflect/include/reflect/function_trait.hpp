
#ifndef FOX_FUNCTION_TRAIT_HPP_
#define FOX_FUNCTION_TRAIT_HPP_

#include <tuple>
#include <optional>

// Pattern for non function type
template<typename Lambda>
struct function_traits : function_traits<decltype(&Lambda::operator())> {};

// template<typename NotAFunction>
// struct function_traits {};

// Pattern for function type with a return type
template<typename R>
struct function_traits<R(*)()>
{
    using result = R;
};

// Pattern for function type with a return type & parameters
template<typename R, typename... Args>
struct function_traits<R(*)(Args...)> {
    using result = R;
    using parameters = std::tuple<Args...>;
};

// Pattern for function type lambda
template<typename R, typename C, typename... Args>
struct function_traits<R(C::*)(Args...) const> {
    using result = R;
    using parameters = std::tuple<Args...>;
};

// Alias for easy to use
template<typename F>
using function_result_t = typename function_traits<F>::result;

template<typename F>
using function_arguments_t = typename function_traits<F>::parameters;

template<std::size_t N, typename F>
using nth_argument_t = std::tuple_element_t<N, function_arguments_t<F>>;

template<typename F>
constexpr auto arguments_count = std::tuple_size<function_arguments_t<F>>::value;






// --------------
template<typename L, std::size_t... S>
constexpr auto wrap_lambda(std::index_sequence<S...>, L lambda) {

    // A wrapper, local struct that recreate the lambda passed as parameter.
    struct Wrapper {
        constexpr Wrapper(L l) noexcept : lambda{std::move(l)} {}
        
        // Reify the call operator with the exact same parameter type and return type.
        constexpr auto operator() (nth_argument_t<S, L>... args) const -> function_result_t<L> {
            return lambda(std::forward<nth_argument_t<S, L>>(args)...);
        }
        
    private:
        L lambda;
    };
    
    return Wrapper{std::move(lambda)};
}

// Provide an overload without the sequence:
template<typename L>
constexpr auto wrap_lambda(L lambda) {
    return wrap_lambda(std::make_index_sequence<arguments_count<L>>(), std::move(lambda));
}

// ---------------
template<typename L, std::size_t... S>
constexpr auto make_deferred(std::index_sequence<S...>, L lambda) {
    // We create a tuple type that can store the list of arguments of the lambda 
    // We are going to store that in our callable type to cache parameters before call
    using parameter_pack = std::tuple<nth_argument_t<S, L>...>;
        
    // We define our wrapper struct
    // We inherit privately to leverage empty base class optimization, but we could easily
    // have choose to contain a private member instead.
    struct Wrapper : private L {
        constexpr Wrapper(L l) : L{std::move(l)} {}
        
        // We make a bind function that take the same arguments as our lambda
        // we are going to store each argument in ...args for a later call
        // void bind(nth_argument_t<S, L>... args) {
        //     bound.reset();
        //     bound = parameter_pack{std::forward<nth_argument_t<S, L>>(args)...};
        // }
        
        explicit operator bool () const {
            return bound.has_value();
        }
        
        // We make a call operator that has the same return type as our lambda
        auto operator() (nth_argument_t<S, L>... args) -> function_result_t<L> {
            bound.reset();
            bound = parameter_pack{std::forward<nth_argument_t<S, L>>(args)...};
            // Here we are using the stored parameters set in the `bind` function
            return L::operator()(std::forward<nth_argument_t<S, L>>(std::get<S>(*bound))...);
        }
        
    private:
        std::optional<parameter_pack> bound;
    };
    
    return Wrapper{std::move(lambda)};
}

// Make an overload without the index sequence
template<typename L>
constexpr auto make_deferred(L lambda) {
    return make_deferred(std::make_index_sequence<arguments_count<L>>(), lambda);
}



// Default case, the lambda cannot be instantiated, yields false
template<typename, typename, typename = void>
constexpr bool is_call_operator_instantiable = false;

// Specialization if the expression `&L::template operator()<Args...>` is valid, yields true
template<typename L, typename... Args>
constexpr bool is_call_operator_instantiable<
    L, std::tuple<Args...>,
    std::void_t<decltype(&L::template operator()<Args...>)> > = true;




// Declaration of our function. Must be declared to provide specializations.
template<typename, typename, typename = void>
struct deduced_function_traits_helper;

// This specialization matches the first path of the pseudocode,
// more presicely the condition `if TFunc instantiable with ArgTypes... then`
template<typename TFunc, typename... ArgTypes>
struct deduced_function_traits_helper<TFunc, std::tuple<ArgTypes...>, // arguments TFunc and ArgTypes
    // if TFunc is instantiable with ArgTypes
    std::enable_if_t<is_call_operator_instantiable<TFunc, std::tuple<ArgTypes...>>>
> // return function_traits with function pointer
  // Returning is modelized as inheritance. We inherit (returning) the function trait with the deduced pointer to member.
     : function_traits<decltype(&TFunc::template operator()<ArgTypes...>)> {};

// This specialisation matches the second path of the pseudocode,
// the `else if size of ArgTypes larger than 0`
template<typename TFunc, typename First, typename... ArgTypes>
struct deduced_function_traits_helper<TFunc, std::tuple<First, ArgTypes...>, // arguments TFunc and First, ArgTypes...
    // if not instantiable
    std::enable_if_t<!is_call_operator_instantiable<TFunc, std::tuple<First, ArgTypes...>>>
> // return deduced_function_traits(TFunc, drop first ArgTypes...)
  // again, returning is modelized as inheritance. We are returning the next step of the algorithm (recursion)
     :  deduced_function_traits_helper<TFunc, std::tuple<ArgTypes...>> {};

// Third path of the algorithm.
// Else return nothing, end of algorithm
template<typename, typename, typename>
struct deduced_function_traits_helper {};


// Alias
template<typename F, typename... Args>
using deduced_function_traits = deduced_function_traits_helper<F, std::tuple<Args...>>;

template<typename F, typename... Args>
using deduced_function_result_t = typename deduced_function_traits<F, Args...>::result;

template<typename F, typename... Args>
using deduced_function_arguments_t = typename deduced_function_traits<F, Args...>::parameters;

template<std::size_t N, typename F, typename... Args>
using deduced_nth_argument_t = std::tuple_element_t<N, deduced_function_arguments_t<F, Args...>>;

template<typename F, typename... Args>
constexpr auto deduced_arguments_count = std::tuple_size<deduced_function_arguments_t<F, Args...>>::value;


template<typename T>
constexpr T magic_val() {
    return T{}; // Simple implementation that default construct the type
}

//   We could have used decltype(auto) instead of reflection here ------v
template<typename L, typename... Args, std::size_t... S> //             v
constexpr auto magic_call(std::index_sequence<S...>, L lambda, Args&&... args) -> deduced_function_result_t<L, Args...> {
    // Call the lambda with both magic_val and provided parameter in ...args 
    return lambda(magic_val<deduced_nth_argument_t<S, L, Args...>>()..., std::forward<Args>(args)...);
}

template<typename L, typename... Args>
constexpr auto magic_call(L lambda, Args&&... args) -> deduced_function_result_t<L, Args...> {
    // We generate a sequence from 0 up to the number of parameter we need to get through `magic_val`
    auto sequence = std::make_index_sequence<deduced_arguments_count<L, Args...> - sizeof...(Args)>();
    return magic_call(sequence, std::move(lambda), std::forward<Args>(args)...);
}

#endif