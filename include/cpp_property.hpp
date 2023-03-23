/*===================================================*
|  cpp-property version v0.0.1                       |
|  https://github.com/yosh-matsuda/cpp-property      |
|                                                    |
|  Copyright (c) 2023 Yoshiki Matsuda @yosh-matsuda  |
|                                                    |
|  This software is released under the MIT License.  |
|  https://opensource.org/license/mit/               |
====================================================*/

#pragma once
#include <cassert>
#include <concepts>
#include <functional>
#include <optional>

namespace cpp_property
{
    namespace  // NOLINT
    {
        namespace detail
        {
            template <typename Func>
            concept function_castable = requires(Func f) { std::function{f}; };

            template <function_castable T>
            class function_traits
            {
                template <typename R, typename... As>
                static std::tuple<As...> args(std::function<R(As...)>);
                template <typename R, typename... As>
                static R result(std::function<R(As...)>);

            public:
                using return_type = decltype(result(std::function{std::declval<T>()}));
                using argument_types = decltype(args(std::function{std::declval<T>()}));
            };

            template <typename Func>
            concept getter_function =
                function_castable<Func> &&
                requires {
                    requires(!std::same_as<void, typename function_traits<Func>::return_type>) &&
                                (std::tuple_size_v<typename function_traits<Func>::argument_types> == 0);
                };

            template <typename Func>
            concept setter_function =
                function_castable<Func> &&
                requires {
                    requires std::same_as<void, typename function_traits<Func>::return_type> &&
                                 (std::tuple_size_v<typename function_traits<Func>::argument_types> == 1);
                };

            template <typename OuterReturnType, typename InnerReturnType>
            constexpr auto is_dangling_reference =
                std::is_reference_v<OuterReturnType> && !std::is_reference_v<InnerReturnType>;

            template <setter_function Func>
            using setter_argument_type = std::tuple_element_t<0, typename function_traits<Func>::argument_types>;
            template <getter_function Func>
            using getter_return_type = typename function_traits<Func>::return_type;

            template <typename, typename, typename>
            class property_base;

            template <class Derived>
            concept base_of_property =
                requires(Derived d) { []<typename X, typename Y, typename Z>(const property_base<X, Y, Z>&) {}(d); };

            template <class T>
            concept not_base_of_property = (!base_of_property<T>);

            template <typename DerivedType, typename ReturnType, typename ArgumentType>
            class property_base
            {
                [[nodiscard]] const DerivedType& derived() const& noexcept
                {
                    return static_cast<const DerivedType&>(*this);
                }
                DerivedType& derived() & noexcept { return static_cast<DerivedType&>(*this); }
                DerivedType&& derived() && noexcept { return static_cast<DerivedType&&>(*this); }

                static constexpr auto has_getter = !std::same_as<void, ReturnType>;
                static constexpr auto has_setter = !std::same_as<void, ArgumentType>;

            protected:
                property_base() = default;

            public:
                property_base(const property_base&) = delete;
                property_base(property_base&&) = delete;
                property_base& operator=(const property_base&) = delete;
                property_base& operator=(property_base&&) = delete;

                // explicit cast
                ReturnType operator()() const
                requires has_getter
                {
                    return derived().get();
                }

                // implicit cast
                operator ReturnType() const  // NOLINT
                requires has_getter
                {
                    return derived().get();
                }

                // arrow operator
                ReturnType operator->() const
                requires has_getter
                {
                    return derived().get();
                }

                // indirection operator
                decltype(auto) operator*() const
                requires has_getter && requires(ReturnType v) { *v; }
                {
                    return *derived().get();
                }

                // equal operator (default)
                template <typename U>  // clang-format off
                requires has_setter
                decltype(auto) operator=(U&& value) const  // clang-format on
                {
                    U right = value;
                    derived().set(std::forward<U>(value));
                    return right;
                }
                template <typename U>  // clang-format off
                requires has_setter
                decltype(auto) operator=(U&& value)  // clang-format on
                {
                    U right = value;
                    derived().set(std::forward<U>(value));
                    return right;
                }

#pragma region lvalue operators
                template <typename S>  // clang-format off
                requires has_getter && requires(ReturnType v, S&& i) { v[std::forward<S>(i)]; }
                decltype(auto) operator[](S&& i) const&  // clang-format on
                {
                    return derived()()[std::forward<S>(i)];
                }
                auto operator++(int) const&
                requires has_getter && has_setter && requires(ReturnType v) { v + 1; }
                {
                    const auto prev = derived()();
                    operator=(prev + 1);
                    return prev;
                }
                auto operator--(int) const&
                requires has_getter && has_setter && requires(ReturnType v) { v - 1; }
                {
                    const auto prev = derived()();
                    operator=(prev - 1);
                    return prev;
                }
                decltype(auto) operator++() const&
                requires has_getter && has_setter && requires(ReturnType v) { v + 1; }
                {
                    return operator=(derived()() + 1);
                }
                decltype(auto) operator--() const&
                requires has_getter && has_setter && requires(ReturnType v) { v - 1; }
                {
                    return operator=(derived()() - 1);
                }
                decltype(auto) operator~() const&
                requires has_getter && requires(ReturnType v) { ~v; }
                {
                    return ~derived()();
                }
                decltype(auto) operator!() const&
                requires has_getter && requires(ReturnType v) { !v; }
                {
                    return !derived()();
                }
                decltype(auto) operator-() const&
                requires has_getter && requires(ReturnType v) { -v; }
                {
                    return -derived()();
                }
                decltype(auto) operator+() const&
                requires has_getter && requires(ReturnType v) { +v; }
                {
                    return +derived()();
                }

                template <typename U>  // clang-format off
                requires has_getter && has_setter && requires(ReturnType v, const U& r) { v * r; }
                decltype(auto) operator*=(const U& right) const&  // clang-format on
                {
                    return operator=(derived()() * right);
                }
                template <typename U>  // clang-format off
                requires has_getter && has_setter && requires(ReturnType v, const U& r) { v / r; }
                decltype(auto) operator/=(const U& right) const&  // clang-format on
                {
                    return operator=(derived()() / right);
                }
                template <typename U>  // clang-format off
                requires has_getter && has_setter && requires(ReturnType v, const U& r) { v % r; }
                decltype(auto) operator%=(const U& right) const&  // clang-format on
                {
                    return operator=(derived()() % right);
                }
                template <typename U>  // clang-format off
                requires has_getter && has_setter && requires(ReturnType v, const U& r) { v + r; }
                decltype(auto) operator+=(const U& right) const&  // clang-format on
                {
                    return operator=(derived()() + right);
                }
                template <typename U>  // clang-format off
                requires has_getter && has_setter && requires(ReturnType v, const U& r) { v - r; }
                decltype(auto) operator-=(const U& right) const&  // clang-format on
                {
                    return operator=(derived()() - right);
                }
                template <typename U>  // clang-format off
                requires has_getter && has_setter && requires(ReturnType v, const U& r) { v << r; }
                decltype(auto) operator<<=(const U& right) const&  // clang-format on
                {
                    return operator=(derived()() << right);
                }
                template <typename U>  // clang-format off
                requires has_getter && has_setter && requires(ReturnType v, const U& r) { v + r; }
                decltype(auto) operator>>=(const U& right) const&  // clang-format on
                {
                    return operator=(derived()() >> right);
                }
                template <typename U>  // clang-format off
                requires has_getter && has_setter && requires(ReturnType v, const U& r) { v & r; }
                decltype(auto) operator&=(const U& right) const&  // clang-format on
                {
                    return operator=(derived()() & right);
                }
                template <typename U>  // clang-format off
                requires has_getter && has_setter && requires(ReturnType v, const U& r) { v | r; }
                decltype(auto) operator|=(const U& right) const&  // clang-format on
                {
                    return operator=(derived()() | right);
                }
                template <typename U>  // clang-format off
                requires has_getter && has_setter && requires(ReturnType v, const U& r) { v ^ r; }
                decltype(auto) operator^=(const U& right) const&  // clang-format on
                {
                    return operator=(derived()() ^ right);
                }
#pragma endregion
            };

#pragma region global operators(property / not property)
            template <not_base_of_property U, base_of_property V>  // clang-format off
            requires requires(const V& v, U&& r) { v() * std::forward<U>(r); }
            decltype(auto) operator*(const V& t1, U&& t2)  // clang-format on
            {
                return t1() * std::forward<U>(t2);
            }

            template <not_base_of_property U, base_of_property V>  // clang-format off
            requires requires(const V& v, U&& r) { v() / std::forward<U>(r); }
            decltype(auto) operator/(const V& t1, U&& t2)  // clang-format on
            {
                return t1() / std::forward<U>(t2);
            }
            template <not_base_of_property U, base_of_property V>  // clang-format off
            requires requires(const V& v, U&& r) { v() % std::forward<U>(r); }
            decltype(auto) operator%(const V& t1, U&& t2)  // clang-format on
            {
                return t1() % std::forward<U>(t2);
            }
            template <not_base_of_property U, base_of_property V>  // clang-format off
            requires requires(const V& v, U&& r) { v() + std::forward<U>(r); }
            decltype(auto) operator+(const V& t1, U&& t2)  // clang-format on
            {
                return t1() + std::forward<U>(t2);
            }
            template <not_base_of_property U, base_of_property V>  // clang-format off
            requires requires(const V& v, U&& r) { v() - std::forward<U>(r); }
            decltype(auto) operator-(const V& t1, U&& t2)  // clang-format on
            {
                return t1() - std::forward<U>(t2);
            }
            template <not_base_of_property U, base_of_property V>  // clang-format off
            requires requires(const V& v, U&& r) { v() << std::forward<U>(r); }
            decltype(auto) operator<<(const V& t1, U&& t2)  // clang-format on
            {
                return t1() << std::forward<U>(t2);
            }
            template <not_base_of_property U, base_of_property V>  // clang-format off
            requires requires(const V& v, U&& r) { v() >> std::forward<U>(r); }
            decltype(auto) operator>>(const V& t1, U&& t2)  // clang-format on
            {
                return t1() >> std::forward<U>(t2);
            }
            template <not_base_of_property U, base_of_property V>  // clang-format off
            requires requires(const V& v, U&& r) { v() < std::forward<U>(r); }
            decltype(auto) operator<(const V& t1, U&& t2)  // clang-format on
            {
                return t1() < std::forward<U>(t2);
            }
            template <not_base_of_property U, base_of_property V>  // clang-format off
            requires requires(const V& v, U&& r) { v() > std::forward<U>(r); }
            decltype(auto) operator>(const V& t1, U&& t2)  // clang-format on
            {
                return t1() > std::forward<U>(t2);
            }
            template <not_base_of_property U, base_of_property V>  // clang-format off
            requires requires(const V& v, U&& r) { v() <= std::forward<U>(r); }
            decltype(auto) operator<=(const V& t1, U&& t2)  // clang-format on
            {
                return t1() <= std::forward<U>(t2);
            }
            template <not_base_of_property U, base_of_property V>  // clang-format off
            requires requires(const V& v, U&& r) { v() >= std::forward<U>(r); }
            decltype(auto) operator>=(const V& t1, U&& t2)  // clang-format on
            {
                return t1() >= std::forward<U>(t2);
            }
            template <not_base_of_property U, base_of_property V>  // clang-format off
            requires requires(const V& v, U&& r) { v() == std::forward<U>(r); }
            decltype(auto) operator==(const V& t1, U&& t2)  // clang-format on
            {
                return t1() == std::forward<U>(t2);
            }
            template <not_base_of_property U, base_of_property V>  // clang-format off
            requires requires(const V& v, U&& r) { v() != std::forward<U>(r); }
            decltype(auto) operator!=(const V& t1, U&& t2)  // clang-format on
            {
                return t1() != std::forward<U>(t2);
            }
            template <not_base_of_property U, base_of_property V>  // clang-format off
            requires requires(const V& v, U&& r) { v() & std::forward<U>(r); }
            decltype(auto) operator&(const V& t1, U&& t2)  // clang-format on
            {
                return t1() & std::forward<U>(t2);
            }
            template <not_base_of_property U, base_of_property V>  // clang-format off
            requires requires(const V& v, U&& r) { v() ^ std::forward<U>(r); }
            decltype(auto) operator^(const V& t1, U&& t2)  // clang-format on
            {
                return t1() ^ std::forward<U>(t2);
            }
            template <not_base_of_property U, base_of_property V>  // clang-format off
            requires requires(const V& v, U&& r) { v() | std::forward<U>(r); }
            decltype(auto) operator|(const V& t1, U&& t2)  // clang-format on
            {
                return t1() | std::forward<U>(t2);
            }
            template <not_base_of_property U, base_of_property V>  // clang-format off
            requires requires(const V& v, U&& r) { v() && std::forward<U>(r); }
            decltype(auto) operator&&(const V& t1, U&& t2)  // clang-format on
            {
                return t1() && std::forward<U>(t2);
            }
            template <not_base_of_property U, base_of_property V>  // clang-format off
            requires requires(const V& v, U&& r) { v() || std::forward<U>(r); }
            decltype(auto) operator||(const V& t1, U&& t2)  // clang-format on
            {
                return t1() || std::forward<U>(t2);
            }
            template <not_base_of_property U, base_of_property V>  // clang-format off
            requires requires(U&& r, const V& v) { std::forward<U>(r) * v(); }
            decltype(auto) operator*(U&& t1, const V& t2)  // clang-format on
            {
                return std::forward<U>(t1) * t2();
            }
            template <not_base_of_property U, base_of_property V>  // clang-format off
            requires requires(U&& r, const V& v) { std::forward<U>(r) / v(); }
            decltype(auto) operator/(U&& t1, const V& t2)  // clang-format on
            {
                return std::forward<U>(t1) / t2();
            }
            template <not_base_of_property U, base_of_property V>  // clang-format off
            requires requires(U&& r, const V& v) { std::forward<U>(r) % v(); }
            decltype(auto) operator%(U&& t1, const V& t2)  // clang-format on
            {
                return std::forward<U>(t1) % t2();
            }
            template <not_base_of_property U, base_of_property V>  // clang-format off
            requires requires(U&& r, const V& v) { std::forward<U>(r) + v(); }
            decltype(auto) operator+(U&& t1, const V& t2)  // clang-format on
            {
                return std::forward<U>(t1) + t2();
            }
            template <not_base_of_property U, base_of_property V>  // clang-format off
            requires requires(U&& r, const V& v) { std::forward<U>(r) - v(); }
            decltype(auto) operator-(U&& t1, const V& t2)  // clang-format on
            {
                return std::forward<U>(t1) - t2();
            }
            template <not_base_of_property U, base_of_property V>  // clang-format off
            requires requires(U&& r, const V& v) { std::forward<U>(r) << v(); }
            decltype(auto) operator<<(U&& t1, const V& t2)  // clang-format on
            {
                return std::forward<U>(t1) << t2();
            }
            template <not_base_of_property U, base_of_property V>  // clang-format off
            requires requires(U&& r, const V& v) { std::forward<U>(r) >> v(); }
            decltype(auto) operator>>(U&& t1, const V& t2)  // clang-format on
            {
                return std::forward<U>(t1) >> t2();
            }
            template <not_base_of_property U, base_of_property V>  // clang-format off
            requires requires(U&& r, const V& v) { std::forward<U>(r) < v(); }
            decltype(auto) operator<(U&& t1, const V& t2)  // clang-format on
            {
                return std::forward<U>(t1) < t2();
            }
            template <not_base_of_property U, base_of_property V>  // clang-format off
            requires requires(U&& r, const V& v) { std::forward<U>(r) > v(); }
            decltype(auto) operator>(U&& t1, const V& t2)  // clang-format on
            {
                return std::forward<U>(t1) > t2();
            }
            template <not_base_of_property U, base_of_property V>  // clang-format off
            requires requires(U&& r, const V& v) { std::forward<U>(r) <= v(); }
            decltype(auto) operator<=(U&& t1, const V& t2)  // clang-format on
            {
                return std::forward<U>(t1) <= t2();
            }
            template <not_base_of_property U, base_of_property V>  // clang-format off
            requires requires(U&& r, const V& v) { std::forward<U>(r) >= v(); }
            decltype(auto) operator>=(U&& t1, const V& t2)  // clang-format on
            {
                return std::forward<U>(t1) >= t2();
            }
            template <not_base_of_property U, base_of_property V>  // clang-format off
            requires requires(U&& r, const V& v) { std::forward<U>(r) == v(); }
            decltype(auto) operator==(U&& t1, const V& t2)  // clang-format on
            {
                return std::forward<U>(t1) == t2();
            }
            template <not_base_of_property U, base_of_property V>  // clang-format off
            requires requires(U&& r, const V& v) { std::forward<U>(r) != v(); }
            decltype(auto) operator!=(U&& t1, const V& t2)  // clang-format on
            {
                return std::forward<U>(t1) != t2();
            }
            template <not_base_of_property U, base_of_property V>  // clang-format off
            requires requires(U&& r, const V& v) { std::forward<U>(r) & v(); }
            decltype(auto) operator&(U&& t1, const V& t2)  // clang-format on
            {
                return std::forward<U>(t1) & t2();
            }
            template <not_base_of_property U, base_of_property V>  // clang-format off
            requires requires(U&& r, const V& v) { std::forward<U>(r) ^ v(); }
            decltype(auto) operator^(U&& t1, const V& t2)  // clang-format on
            {
                return std::forward<U>(t1) ^ t2();
            }
            template <not_base_of_property U, base_of_property V>  // clang-format off
            requires requires(U&& r, const V& v) { std::forward<U>(r) | v(); }
            decltype(auto) operator|(U&& t1, const V& t2)  // clang-format on
            {
                return std::forward<U>(t1) | t2();
            }
            template <not_base_of_property U, base_of_property V>  // clang-format off
            requires requires(U&& r, const V& v) { std::forward<U>(r) && v(); }
            decltype(auto) operator&&(U&& t1, const V& t2)  // clang-format on
            {
                return std::forward<U>(t1) && t2();
            }
            template <not_base_of_property U, base_of_property V>  // clang-format off
            requires requires(U&& r, const V& v) { std::forward<U>(r) || v(); }
            decltype(auto) operator||(U&& t1, const V& t2)  // clang-format on
            {
                return std::forward<U>(t1) || t2();
            }
#pragma endregion operators(property / not property)

#pragma region global operators(property / property)
            template <base_of_property U, base_of_property V>  // clang-format off
            requires requires(const U& u, const V& v) { u() * v(); }
            decltype(auto) operator*(const U& t1, const V& t2)  // clang-format on
            {
                return t1() * t2();
            }
            template <base_of_property U, base_of_property V>  // clang-format off
            requires requires(const U& u, const V& v) { u() / v(); }
            decltype(auto) operator/(const U& t1, const V& t2)  // clang-format on
            {
                return t1() / t2();
            }
            template <base_of_property U, base_of_property V>  // clang-format off
            requires requires(const U& u, const V& v) { u() % v(); }
            decltype(auto) operator%(const U& t1, const V& t2)  // clang-format on
            {
                return t1() % t2();
            }
            template <base_of_property U, base_of_property V>  // clang-format off
            requires requires(const U& u, const V& v) { u() + v(); }
            decltype(auto) operator+(const U& t1, const V& t2)  // clang-format on
            {
                return t1() + t2();
            }
            template <base_of_property U, base_of_property V>  // clang-format off
            requires requires(const U& u, const V& v) { u() - v(); }
            decltype(auto) operator-(const U& t1, const V& t2)  // clang-format on
            {
                return t1() - t2();
            }
            template <base_of_property U, base_of_property V>  // clang-format off
            requires requires(const U& u, const V& v) { u() << v(); }
            decltype(auto) operator<<(const U& t1, const V& t2)  // clang-format on
            {
                return t1() << t2();
            }
            template <base_of_property U, base_of_property V>  // clang-format off
            requires requires(const U& u, const V& v) { u() >> v(); }
            decltype(auto) operator>>(const U& t1, const V& t2)  // clang-format on
            {
                return t1() >> t2();
            }
            template <base_of_property U, base_of_property V>  // clang-format off
            requires requires(const U& u, const V& v) { u() < v(); }
            decltype(auto) operator<(const U& t1, const V& t2)  // clang-format on
            {
                return t1() < t2();
            }
            template <base_of_property U, base_of_property V>  // clang-format off
            requires requires(const U& u, const V& v) { u() > v(); }
            decltype(auto) operator>(const U& t1, const V& t2)  // clang-format on
            {
                return t1() > t2();
            }
            template <base_of_property U, base_of_property V>  // clang-format off
            requires requires(const U& u, const V& v) { u() <= v(); }
            decltype(auto) operator<=(const U& t1, const V& t2)  // clang-format on
            {
                return t1() <= t2();
            }
            template <base_of_property U, base_of_property V>  // clang-format off
            requires requires(const U& u, const V& v) { u() >= v(); }
            decltype(auto) operator>=(const U& t1, const V& t2)  // clang-format on
            {
                return t1() >= t2();
            }
            template <base_of_property U, base_of_property V>  // clang-format off
            requires requires(const U& u, const V& v) { u() == v(); }
            decltype(auto) operator==(const U& t1, const V& t2)  // clang-format on
            {
                return t1() == t2();
            }
            template <base_of_property U, base_of_property V>  // clang-format off
             requires requires(const U& u, const V& v) { u() != v(); }
            decltype(auto) operator!=(const U& t1, const V& t2)  // clang-format on
            {
                return t1() != t2();
            }
            template <base_of_property U, base_of_property V>  // clang-format off
            requires requires(const U& u, const V& v) { u() & v(); }
            decltype(auto) operator&(const U& t1, const V& t2)  // clang-format on
            {
                return t1() & t2();
            }
            template <base_of_property U, base_of_property V>  // clang-format off
            requires requires(const U& u, const V& v) { u() ^ v(); }
            decltype(auto) operator^(const U& t1, const V& t2)  // clang-format on
            {
                return t1() ^ t2();
            }
            template <base_of_property U, base_of_property V>  // clang-format off
            requires requires(const U& u, const V& v) { u() | v(); }
            decltype(auto) operator|(const U& t1, const V& t2)  // clang-format on
            {
                return t1() | t2();
            }
            template <base_of_property U, base_of_property V>  // clang-format off
            requires requires(const U& u, const V& v) { u() && v(); }
            decltype(auto) operator&&(const U& t1, const V& t2)  // clang-format on
            {
                return t1() && t2();
            }
            template <base_of_property U, base_of_property V>  // clang-format off
            requires requires(const U& u, const V& v) { u() || v(); }
            decltype(auto) operator||(const U& t1, const V& t2)  // clang-format on
            {
                return t1() || t2();
            }
#pragma endregion operators(property / property)
        }  // namespace detail
    }      // namespace

    struct get_only
    {
    };
    struct set_only
    {
    };

    template <typename T>
    class get_auto
    {
        const T* entity_ = nullptr;

    public:
        get_auto() = default;
        explicit get_auto(const T& t) : entity_(&t) {}
        const T& get() const { return *entity_; }
    };
    template <>
    class get_auto<void>
    {
    public:
        get_auto() = default;
    };
    get_auto()->get_auto<void>;

    template <typename T>
    class set_auto
    {
        T* entity_ = nullptr;

    public:
        set_auto() = default;
        explicit set_auto(T& t) : entity_(&t) {}
        template <typename V>
        requires requires(T* t, V&& v) { *t = std::forward<V>(v); }
        void set(V&& value) const
        {
            *entity_ = std::forward<V>(value);
        }
    };
    template <>
    class set_auto<void>
    {
    public:
        set_auto() = default;
    };
    set_auto()->set_auto<void>;

    template <typename T>
    constexpr auto is_const_lvalue_reference_v =
        std::is_const_v<std::remove_reference_t<T>> && std::is_lvalue_reference_v<T>;

    template <typename...>
    class property;

    template <typename ReturnType>
    class property<ReturnType>
        : public detail::property_base<property<ReturnType>, ReturnType, std::remove_cvref_t<ReturnType>>
    {
        using Base = detail::property_base<property<ReturnType>, ReturnType, std::remove_cvref_t<ReturnType>>;
        friend Base;

        template <typename...>
        friend class property;

        using EntityType = std::remove_cvref_t<ReturnType>;
        using ArgumentType = std::remove_cvref_t<ReturnType>;
        const std::function<ReturnType()> getter_;  // NOLINT
        mutable get_auto<EntityType> auto_getter_;
        const std::function<void(ArgumentType)> setter_;  // NOLINT
        mutable set_auto<EntityType> auto_setter_;

    public:
        property() = delete;

        template <typename Getter, typename Setter>
        requires requires(Getter&& g, Setter&& s) {
                     std::function<ReturnType()>{g};
                     std::function<void(ArgumentType)>{s};
                     requires !(detail::is_dangling_reference<ReturnType, decltype(g())>);
                 }
        property(Getter&& get_f, Setter&& set_f)
            : getter_(std::forward<Getter>(get_f)), setter_(std::forward<Setter>(set_f))
        {
        }

        property(get_auto<EntityType> get_f, set_auto<EntityType> set_f)
        requires is_const_lvalue_reference_v<ReturnType>
            : auto_getter_(std::move(get_f)), auto_setter_(std::move(set_f))
        {
        }

        template <typename Setter>
        requires is_const_lvalue_reference_v<ReturnType> &&
                     requires(Setter&& s) { std::function<void(ArgumentType)>{s}; }
        property(get_auto<EntityType> get_f, Setter&& set_f)
            : auto_getter_(std::move(get_f)), setter_(std::forward<Setter>(set_f))
        {
        }

        template <typename Getter>
        requires requires(Getter&& g) {
                     std::function<ReturnType()>{g};
                     requires !(detail::is_dangling_reference<ReturnType, decltype(g())>);
                 }
        property(Getter&& get_f, set_auto<EntityType> set_f)
            : getter_(std::forward<Getter>(get_f)), auto_setter_(std::move(set_f))
        {
        }

        // copy assign operator (but not copy)
        decltype(auto) operator=(const property& right) const { return Base::operator=(right()); }

        // assign operator
        template <detail::base_of_property PropertyType>  // clang-format off
        requires requires (const decltype(setter_)& s, const PropertyType p) { s(p()); } &&
                 requires (const decltype(auto_setter_)& s, const PropertyType p) { s.set(p()); }
        decltype(auto) operator=(const PropertyType& prop) const  // clang-format on
        {
            return Base::operator=(prop());
        };
        template <detail::not_base_of_property U>  // clang-format off
        requires requires(const decltype(setter_)& s, U&& v) { s(std::forward<U>(v)); } &&
                 requires(const decltype(auto_setter_)& s, U&& v) { s.set(std::forward<U>(v)); }
        decltype(auto) operator=(U&& value) const  // clang-format on
        {
            return Base::operator=(std::forward<U>(value));
        };

    private:
        [[nodiscard]] ReturnType get() const
        {
            if constexpr (is_const_lvalue_reference_v<ReturnType>)
            {
                if (getter_) return getter_();
                return auto_getter_.get();
            }
            else
            {
                return getter_();
            }
        }
        template <detail::not_base_of_property U>
        void set(U&& value) const
        {
            if (setter_)
                setter_(std::forward<U>(value));
            else
                auto_setter_.set(std::forward<U>(value));
        }
    };

    template <typename ReturnType, typename ArgumentType>
    class property<ReturnType, ArgumentType>
        : public detail::property_base<property<ReturnType, ArgumentType>, ReturnType, ArgumentType>
    {
        using Base = detail::property_base<property<ReturnType, ArgumentType>, ReturnType, ArgumentType>;
        friend Base;

        template <typename...>
        friend class property;

        using EntityType = std::remove_cvref_t<ReturnType>;
        const std::function<ReturnType()> getter_;  // NOLINT
        mutable get_auto<EntityType> auto_getter_;
        const std::function<void(ArgumentType)> setter_;  // NOLINT
        mutable set_auto<EntityType> auto_setter_;

    public:
        property() = delete;

        template <typename Getter, typename Setter>
        requires requires(Getter&& g, Setter&& s) {
                     std::function<ReturnType()>{g};
                     std::function<void(ArgumentType)>{s};
                     requires !(detail::is_dangling_reference<ReturnType, decltype(g())>);
                 }
        property(Getter&& get_f, Setter&& set_f)
            : getter_(std::forward<Getter>(get_f)), setter_(std::forward<Setter>(set_f))
        {
        }

        property(get_auto<EntityType> get_f, set_auto<EntityType> set_f)
        requires is_const_lvalue_reference_v<ReturnType>
            : auto_getter_(std::move(get_f)), auto_setter_(std::move(set_f))
        {
        }

        template <typename Setter>
        requires is_const_lvalue_reference_v<ReturnType> &&
                     requires(Setter&& s) { std::function<void(ArgumentType)>{s}; }
        property(get_auto<EntityType> get_f, Setter&& set_f)
            : auto_getter_(std::move(get_f)), setter_(std::forward<Setter>(set_f))
        {
        }

        template <typename Getter>
        requires requires(Getter&& g) {
                     std::function<ReturnType()>{g};
                     requires !(detail::is_dangling_reference<ReturnType, decltype(g())>);
                 }
        property(Getter&& get_f, set_auto<EntityType> set_f)
            : getter_(std::forward<Getter>(get_f)), auto_setter_(std::move(set_f))
        {
        }

        // copy assign operator (but not copy)
        decltype(auto) operator=(const property& right) const { return Base::operator=(right()); }

        // assign operator
        template <detail::base_of_property PropertyType>  // clang-format off
        requires requires (const decltype(setter_)& s, const PropertyType p) { s(p()); } &&
                 requires (const decltype(auto_setter_)& s, const PropertyType p) { s.set(p()); }
        decltype(auto) operator=(const PropertyType& prop) const  // clang-format on
        {
            return Base::operator=(prop());
        };
        template <detail::not_base_of_property U>  // clang-format off
        requires requires(const decltype(setter_)& s, U&& v) { s(std::forward<U>(v)); } &&
                 requires(const decltype(auto_setter_)& s, U&& v) { s.set(std::forward<U>(v)); }
        decltype(auto) operator=(U&& value) const  // clang-format on
        {
            return Base::operator=(std::forward<U>(value));
        };

    private:
        [[nodiscard]] ReturnType get() const
        {
            if constexpr (is_const_lvalue_reference_v<ReturnType>)
            {
                if (getter_) return getter_();
                return auto_getter_.get();
            }
            else
            {
                return getter_();
            }
        }
        template <detail::not_base_of_property U>
        void set(U&& value) const
        {
            if (setter_)
                setter_(std::forward<U>(value));
            else
                auto_setter_.set(std::forward<U>(value));
        }
    };

    template <typename ReturnType>
    class property<ReturnType, get_only>
        : public detail::property_base<property<ReturnType, get_only>, ReturnType, void>
    {
        using Base = detail::property_base<property<ReturnType, get_only>, ReturnType, void>;
        friend Base;

        template <typename...>
        friend class property;

        using EntityType = std::remove_cvref_t<ReturnType>;
        const std::function<ReturnType()> getter_;  // NOLINT
        mutable get_auto<EntityType> auto_getter_;

    public:
        property() = delete;

        template <typename Getter>
        requires requires(Getter&& g) {
                     std::function<ReturnType()>{g};
                     requires !(detail::is_dangling_reference<ReturnType, decltype(g())>);
                 }
        property(Getter&& get_f) : getter_(std::forward<Getter>(get_f))  // NOLINT
        {
        }

        property(get_auto<EntityType> get_f)  // NOLINT
        requires is_const_lvalue_reference_v<ReturnType>
            : auto_getter_(std::move(get_f))
        {
        }

    private:
        [[nodiscard]] ReturnType get() const
        {
            if constexpr (is_const_lvalue_reference_v<ReturnType>)
            {
                if (getter_) return getter_();
                return auto_getter_.get();
            }
            else
            {
                return getter_();
            }
        }
    };

    template <typename ArgumentType>
    class property<ArgumentType, set_only>
        : public detail::property_base<property<ArgumentType, set_only>, void, ArgumentType>
    {
        using Base = detail::property_base<property<ArgumentType, set_only>, void, ArgumentType>;
        friend Base;

        template <typename...>
        friend class property;

        using EntityType = std::remove_cvref_t<ArgumentType>;
        const std::function<void(ArgumentType)> setter_;  // NOLINT
        mutable set_auto<EntityType> auto_setter_;

    public:
        property() = delete;

        template <typename Setter>
        requires requires(Setter&& s) { std::function<void(ArgumentType)>{s}; }
        property(Setter&& set_f) : setter_(std::forward<Setter>(set_f))  // NOLINT
        {
        }

        property(set_auto<EntityType> set_f) : auto_setter_(std::move(set_f)) {}  // NOLINT

        // copy assign operator (deleted)
        property& operator=(const property&) = delete;

        // assign operator
        template <detail::base_of_property PropertyType>  // clang-format off
        requires requires (const decltype(setter_)& s, const PropertyType p) { s(p()); } &&
                 requires (const decltype(auto_setter_)& s, const PropertyType p) { s.set(p()); }
        decltype(auto) operator=(const PropertyType& prop) const  // clang-format on
        {
            return Base::operator=(prop());
        };
        template <detail::not_base_of_property U>  // clang-format off
        requires requires(const decltype(setter_)& s, U&& v) { s(std::forward<U>(v)); } &&
                 requires(const decltype(auto_setter_)& s, U&& v) { s.set(std::forward<U>(v)); }
        decltype(auto) operator=(U&& value) const  // clang-format on
        {
            return Base::operator=(std::forward<U>(value));
        };

    private:
        template <detail::not_base_of_property U>
        void set(U&& value) const
        {
            if (setter_)
                setter_(std::forward<U>(value));
            else
                auto_setter_.set(std::forward<U>(value));
        }
    };

    template <typename Getter, typename Setter>
    property(Getter&&, Setter&&) -> property<detail::getter_return_type<Getter>, detail::setter_argument_type<Setter>>;
    template <typename EntityType>
    property(get_auto<EntityType>&&, set_auto<EntityType>&&) -> property<const EntityType&>;
    template <typename Getter, typename EntityType>
    property(Getter&&, set_auto<EntityType>&&) -> property<detail::getter_return_type<Getter>, EntityType>;
    template <typename EntityType, typename Setter>
    property(get_auto<EntityType>&&, Setter&&) -> property<const EntityType&, detail::setter_argument_type<Setter>>;
    template <typename Getter>
    property(Getter&&) -> property<detail::getter_return_type<Getter>, get_only>;
    template <typename EntityType>
    property(get_auto<EntityType>&&) -> property<const EntityType&, get_only>;
    template <typename Setter>
    property(Setter&&) -> property<detail::setter_argument_type<Setter>, set_only>;
    template <typename EntityType>
    property(set_auto<EntityType>&&) -> property<EntityType, set_only>;

    template <typename...>
    class auto_property;

    template <typename EntityType>
    requires(!std::is_rvalue_reference_v<EntityType>)
    class auto_property<EntityType>
        : public detail::property_base<auto_property<EntityType>, const std::remove_cvref_t<EntityType>&,
                                       std::remove_cvref_t<EntityType>>
    {
        using Base = detail::property_base<auto_property<EntityType>, const std::remove_cvref_t<EntityType>&,
                                           std::remove_cvref_t<EntityType>>;
        friend Base;

        template <typename...>
        friend class property;

        using ReturnType = const std::remove_cvref_t<EntityType>&;
        EntityType entity_ = {};

    public:
        auto_property() = default;
        auto_property(const auto_property&) = default;
        auto_property(auto_property&&) noexcept = default;
        template <typename V>  // clang-format off
        requires (!std::is_reference_v<EntityType> || !std::is_rvalue_reference_v<V&&>)  // clang-format on
        explicit auto_property(V&& init) : entity_(std::forward<V>(init))
        {
        }
        auto_property(get_auto<void>, set_auto<void>)
        requires(!std::is_reference_v<EntityType>)
        {
        }
        template <typename V>  // clang-format off
        requires (!std::is_reference_v<EntityType> || !std::is_rvalue_reference_v<V&&>)  // clang-format on
        auto_property(get_auto<void>, set_auto<void>, V&& init) : entity_(std::forward<V>(init))
        {
        }

        // copy assign operator (but not copy)
        auto_property& operator=(const auto_property& right)
        {
            entity_ = right.entity_;
            return *this;
        }
        auto_property& operator=(auto_property&& right) noexcept
        {
            entity_ = right.entity_;
            return *this;
        }

        // assign operator
        template <detail::base_of_property PropertyType>  // clang-format off
        requires requires (EntityType& e, const PropertyType p) { e = p(); }
        decltype(auto) operator=(const PropertyType& prop)  // clang-format on
        {
            return Base::operator=(prop());
        };
        template <detail::not_base_of_property U>  // clang-format off
        requires requires (EntityType& e, U&& v) { e = std::forward<U>(v); } &&
                 (!std::is_reference_v<EntityType> || !std::is_rvalue_reference_v<U&&>)
        decltype(auto) operator=(U&& value)  // clang-format on
        {
            return Base::operator=(std::forward<U>(value));
        };

    private:
        [[nodiscard]] ReturnType get() const { return entity_; }
        template <detail::not_base_of_property U>
        void set(U&& value)
        {
            entity_ = std::forward<U>(value);
        }
    };

    template <typename EntityType>
    requires(!std::is_rvalue_reference_v<EntityType>)
    class auto_property<EntityType, get_only>
        : public detail::property_base<auto_property<EntityType, get_only>, const std::remove_cvref_t<EntityType>&,
                                       std::remove_cvref_t<EntityType>>
    {
        using Base = detail::property_base<auto_property<EntityType, get_only>, const std::remove_cvref_t<EntityType>&,
                                           std::remove_cvref_t<EntityType>>;
        friend Base;

        template <typename...>
        friend class property;

        using ReturnType = const std::remove_cvref_t<EntityType>&;
        EntityType entity_ = {};

    public:
        auto_property() = default;
        auto_property(const auto_property&) = default;
        auto_property(auto_property&&) noexcept = default;
        template <typename V>  // clang-format off
        requires (!std::is_reference_v<EntityType> || !std::is_rvalue_reference_v<V&&>)  // clang-format on
        explicit auto_property(V&& init) : entity_(std::forward<V>(init))
        {
        }
        auto_property(get_auto<void>)  // NOLINT
        requires(!std::is_reference_v<EntityType>)
        {
        }
        template <typename V>  // clang-format off
        requires (!std::is_reference_v<EntityType> || !std::is_rvalue_reference_v<V&&>)  // clang-format on
        auto_property(get_auto<void>, V&& init) : entity_(std::forward<V>(init))
        {
        }

        // copy assign operator (but not copy)
        auto_property& operator=(const auto_property& right)
        {
            entity_ = right.entity_;
            return *this;
        }
        auto_property& operator=(auto_property&& right) noexcept
        {
            entity_ = right.entity_;
            return *this;
        }

    private:
        [[nodiscard]] ReturnType get() const { return entity_; }
    };

    template <typename EntityType>
    requires std::is_lvalue_reference_v<EntityType>
    class auto_property<EntityType, set_only>
        : public detail::property_base<auto_property<EntityType, set_only>, const std::remove_cvref_t<EntityType>&,
                                       std::remove_cvref_t<EntityType>>
    {
        using Base = detail::property_base<auto_property<EntityType, set_only>, const std::remove_cvref_t<EntityType>&,
                                           std::remove_cvref_t<EntityType>>;
        friend Base;

        template <typename...>
        friend class property;

        using ReturnType = const std::remove_cvref_t<EntityType>&;
        EntityType entity_;

    public:
        auto_property() = delete;
        auto_property(const auto_property&) = default;
        auto_property(auto_property&&) noexcept = default;
        template <typename V>
        explicit auto_property(V& init) : entity_(init)
        {
        }
        template <typename V>
        auto_property(set_auto<void>, V& init) : entity_(init)
        {
        }

        // copy assign operator (but not copy)
        auto_property& operator=(const auto_property& right)
        {
            entity_ = right.entity_;
            return *this;
        }
        auto_property& operator=(auto_property&& right) noexcept
        {
            entity_ = right.entity_;
            return *this;
        }

        // assign operator
        template <detail::base_of_property PropertyType>  // clang-format off
        requires requires (EntityType& e, const PropertyType p) { e = p(); }
        decltype(auto) operator=(const PropertyType& prop)  // clang-format on
        {
            return Base::operator=(prop());
        };
        template <detail::not_base_of_property U>  // clang-format off
        requires requires (EntityType& e, U&& v) { e = std::forward<U>(v); }
        decltype(auto) operator=(U&& value)  // clang-format on
        {
            return Base::operator=(std::forward<U>(value));
        };

    private:
        template <detail::not_base_of_property U>
        void set(U&& value)
        {
            entity_ = std::forward<U>(value);
        }
    };

    template <typename ValueType>
    requires(!std::same_as<get_auto<void>, std::remove_cvref_t<ValueType>>) &&
            (!std::same_as<set_auto<void>, std::remove_cvref_t<ValueType>>)
            auto_property(ValueType&&) -> auto_property<std::remove_cvref_t<ValueType>>;
    template <typename ValueType>
    auto_property(get_auto<void>, set_auto<void>, ValueType&&) -> auto_property<std::remove_cvref_t<ValueType>>;
    template <typename ValueType>
    requires(!std::same_as<set_auto<void>, std::remove_cvref_t<ValueType>>)
    auto_property(get_auto<void>, ValueType&&) -> auto_property<std::remove_cvref_t<ValueType>, get_only>;
    template <typename ValueType>
    auto_property(set_auto<void>, ValueType&) -> auto_property<ValueType&, set_only>;

    // close private namespace
    namespace detail
    {
    }
}  // namespace cpp_property

#ifndef DISABLE_CPP_PROPERTY_FRIENDLY_MACRO
namespace cpp_property
{
    inline constexpr auto get = get_auto();
    inline constexpr auto set = set_auto();
}  // namespace cpp_property
#define import_cpp_property()                                                                                  \
    using cpp_property::property, cpp_property::auto_property, cpp_property::get_only, cpp_property::set_only, \
        cpp_property::get_auto, cpp_property::set_auto, cpp_property::get, cpp_property::set
#define get_val [this]() -> auto
#define get_cref [this]() -> const auto&
#define get_ref [this]() -> auto&
#define set_val [this](auto value) -> void
#define set_cref [this](const auto& value) -> void
#define set_ref [this](auto& value) -> void
#else
#define import_cpp_property()                                                                                  \
    using cpp_property::property, cpp_property::auto_property, cpp_property::get_only, cpp_property::set_only, \
        cpp_property::get_auto, cpp_property::set_auto
#endif
