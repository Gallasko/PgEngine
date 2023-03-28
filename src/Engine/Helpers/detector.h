#pragma once

// Implements N4502 for c++ that enable compile time check of the presence of a function member in a type
// found in https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2015/n4502.pdf

// Todo need to test this
namespace pg
{
    // Use case :
    // template <typename T>
    // using toString_t = decltype( std::declval<T&>().toString() );
    // 
    // template <typename T>
    // constexpr bool has_toString = std::is_detected_v<toString_t, T>;
    // 
    // template <class T>
    // std::string optionalToString(T* obj)
    // {
    //     if constexpr (has_toString<T>)
    //         return obj->toString();
    //     else
    //         return "toString not defined";
    // }


    template <class...>
    using void_t = void;

    // Primary template handles all types not supporting the archetypal Op:
    template <class Default, class, template <class...> class Op, class... Args>
    struct detector
    {
        using value_t = false_type;
        using type = Default;
    };

    // The specialization recognizes and handles only types supporting Op:
    template <class Default, template <class...> class Op, class... Args>
    struct detector<Default, void_t<Op<Args...>>, Op, Args...>
    {
        using value_t = true_type;
        using type = Op<Args...>;
    };

    // Non sensical struct to differenciate between void and error
    struct nonesuch
    {
        nonesuch( ) = delete;
        ~nonesuch( ) = delete;
        nonesuch( nonesuch const& ) = delete;
        void operator = ( nonesuch const& ) = delete;
    };

    // All the detection templates
    template <template <class...> class Op, class... Args>
    using is_detected = typename detector<nonesuch, void, Op, Args...>::value_t;
    
    template <template <class...> class Op, class... Args>
    constexpr bool is_detected_v = is_detected<Op, Args...>::value;
    
    template <template <class...> class Op, class... Args>
    using detected_t = typename detector<nonesuch, void, Op, Args...>::type;

    template <class Default, template <class...> class Op, class... Args>
    using detected_or = detector<Default, void, Op, Args...>;

    template <class Default, template <class...> class Op, class... Args>
    using detected_or_t = typename detected_or<Default, Op, Args...>::type;

    template <class Expected, template <class...> class Op, class... Args>
    using is_detected_exact = is_same< Expected, detected_t<Op, Args...> >;

    template <class Expected, template <class...> class Op, class... Args>
    constexpr bool is_detected_exact_v = is_detected_exact<Expected, Op, Args...>::value;

    template <class To, template <class...> class Op, class... Args>
    using is_detected_convertible = is_convertible<detected_t<Op, Args...>, To >;

    template <class To, template <class...> class Op, class... Args>
    constexpr bool is_detected_convertible_v = is_detected_convertible<To, Op, Args...>::value;

}