/**
 * @file utils/detail/variant.hpp
 * @brief Implementation for variant helpers.
 * @copyright Copyright 2015 Mediafire
 *
 * Parts taken from Stack Overflow
 */
#pragma once

#include <algorithm>
#include <type_traits>

#include "boost/variant/apply_visitor.hpp"
#include "boost/variant/static_visitor.hpp"
#include "boost/variant/variant.hpp"

namespace mf
{
namespace utils
{
namespace detail
{

template <typename T>
struct is_variant : public std::false_type
{
};

template <BOOST_VARIANT_ENUM_PARAMS(typename T)>
struct is_variant<boost::variant<BOOST_VARIANT_ENUM_PARAMS(T)>>
        : public std::true_type
{
};

template <typename ReturnT, typename... Lambdas>
struct lambda_visitor;

template <typename ReturnT, typename L1, typename... Lambdas>
struct lambda_visitor<ReturnT, L1, Lambdas...>
        : public L1, public lambda_visitor<ReturnT, Lambdas...>

{
    using L1::operator();

    using lambda_visitor<ReturnT, Lambdas...>::operator();

    lambda_visitor(L1 l1, Lambdas... lambdas)

            : L1(l1), lambda_visitor<ReturnT, Lambdas...>(lambdas...)

    {
    }
};

template <typename ReturnT, typename L1>
struct lambda_visitor<ReturnT, L1> : public L1,
                                     public boost::static_visitor<ReturnT>

{
    using L1::operator();

    lambda_visitor(L1 l1)

            : L1(l1), boost::static_visitor<ReturnT>()

    {
    }
};

template <typename ReturnT>
struct lambda_visitor<ReturnT> : public boost::static_visitor<ReturnT>
{
    lambda_visitor() : boost::static_visitor<ReturnT>() {}
};

template <typename T>
struct return_trait : public return_trait<decltype(&T::operator())>
{
};
// For generic types, directly use the result of the signature of its
// 'operator()'

template <typename ClassType, typename ReturnType, typename... Args>
struct return_trait<ReturnType (ClassType::*)(Args...) const>
// we specialize for pointers to member function
{
    typedef ReturnType result_type;
};

/**
 * @brief Make visitor for boost::variant by series of lambdas.
 *
 * Keep logic near apply_visitor by placing the lambdas to process a variant
 * with the function that iterates its types.
 *
 * @template ReturnT Return type
 * @param[in] lambdas Lambdas that handle all the types of the variant they
 *                    expect the process.
 *
 * @return New visitor for passing to apply_visitor.
 */
template <typename ReturnT, typename... Lambdas>
detail::lambda_visitor<ReturnT, Lambdas...> make_lambda_visitor(
        Lambdas... lambdas)
{
    return {lambdas...};
}

template <typename... Lambdas>
struct partial_lambda_visitor : public lambda_visitor<void, Lambdas...>
{
    using lambda_visitor<void, Lambdas...>::operator();

    partial_lambda_visitor(Lambdas... lambdas)
            : lambda_visitor<void, Lambdas...>(lambdas...)
    {
    }

    // Non-matching events
    template <typename T>
    void operator()(const T &) const
    {
    }
};

template <typename... Lambdas>
partial_lambda_visitor<Lambdas...> make_partial_lambda_visitor(
        Lambdas... lambdas)
{
    return {lambdas...};
}

template <typename... Lambdas>
struct partial_recursive_lambda_visitor
        : public lambda_visitor<void, Lambdas...>
{
    using lambda_visitor<void, Lambdas...>::operator();

    partial_recursive_lambda_visitor(Lambdas... lambdas)
            : lambda_visitor<void, Lambdas...>(lambdas...)
    {
    }

    // Non-matching events
    template <typename T,
              typename std::enable_if<!is_variant<T>::value>::type * = nullptr>
    void operator()(const T &) const
    {
    }

    template <typename T,
              typename std::enable_if<is_variant<T>::value>::type * = nullptr>
    void operator()(const T & variant) const
    {
        return boost::apply_visitor(*this, variant);
    }
};

template <typename... Lambdas>
partial_recursive_lambda_visitor<Lambdas...>
        make_partial_recursive_lambda_visitor(Lambdas... lambdas)
{
    return {lambdas...};
}

template <typename ResultT, typename... Lambdas>
struct partial_lambda_visitor_with_default
        : public lambda_visitor<ResultT, Lambdas...>
{
    using lambda_visitor<ResultT, Lambdas...>::operator();

    partial_lambda_visitor_with_default(ResultT default_result,
                                        Lambdas... lambdas)
            : lambda_visitor<ResultT, Lambdas...>(lambdas...),
              default_result_(std::move(default_result))
    {
    }

    // Non-matching events
    template <typename T>
    ResultT operator()(const T &) const
    {
        return default_result_;
    }

    ResultT default_result_;
};

template <typename ResultT, typename... Lambdas>
partial_lambda_visitor_with_default<ResultT, Lambdas...>
make_partial_lambda_visitor_with_default(ResultT default_result,
                                         Lambdas... lambdas)
{
    return {default_result, lambdas...};
}

template <typename ResultT, typename... Lambdas>
struct partial_recursive_lambda_visitor_with_default
        : public lambda_visitor<ResultT, Lambdas...>
{
    using lambda_visitor<ResultT, Lambdas...>::operator();

    partial_recursive_lambda_visitor_with_default(ResultT default_result,
                                                  Lambdas... lambdas)
            : lambda_visitor<ResultT, Lambdas...>(lambdas...),
              default_result_(std::move(default_result))
    {
    }

    template <typename T,
              typename std::enable_if<!is_variant<T>::value>::type * = nullptr>
    ResultT operator()(const T &) const
    {
        return default_result_;
    }

    template <typename T,
              typename std::enable_if<is_variant<T>::value>::type * = nullptr>
    ResultT operator()(const T & variant) const
    {
        return boost::apply_visitor(*this, variant);
    }

    ResultT default_result_;
};

template <typename ResultT, typename... Lambdas>
partial_recursive_lambda_visitor_with_default<ResultT, Lambdas...>
make_partial_recursive_lambda_visitor_with_default(ResultT default_result,
                                                   Lambdas... lambdas)
{
    return {default_result, lambdas...};
}

}  // namespace detail
}  // namespace utils
}  // namespace mf
