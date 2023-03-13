#include <gtest/gtest.h>
#include "cpp_property.hpp"

// NOLINTBEGIN
import_cpp_property();

// clang-format off
class A
{
    double num_ = 0;

public:
    property<const double&> num
    {
        get_cref
        {
            return num_;
        },
        set_val
        {
            if (value < 0) throw std::invalid_argument("value must be >= 0");
            num_ = std::move(value);
        }
    };
    property<double, get_only> square_num = get_val { return num_ * num_; };
    property<double, set_only> num_set_only = set_val
    {
        if (value < 0) throw std::invalid_argument("value must be >= 0");
        num_ = std::move(value);
    };
};
TEST(CppProperty, Constructor)
{
    auto num1 = 1.0;
    property<const double&> prop_cr1 = {[&num1]() -> const double& { return num1; }, [&num1](double a) { num1 = a; }};
    property<const double&, double> prop_cr2 = {[&num1]() -> const double& { return num1; }, [&num1](double a) { num1 = a; }};
    property<double&, const double&> prop_r_cr = {[&num1]() -> auto& { return num1; }, [&num1](const auto& a) { num1 = std::move(a); }};
    property<const double&, get_only> prop_cr_g = {[&num1]() -> const double& { return num1; }};
    property<const double&, set_only> prop_cr_s = {[&num1](const auto& value) { num1 = value; }};

    auto prop_cr_ded = property{[&num1]() -> const double& { return num1; }, [&num1](double a) { num1 = a; }};
    auto prop_r_cr_ded = property{[&num1]() -> double& { return num1; }, [&num1](const double& a) { num1 = a; }};
    auto prop_cr_g_ded = property{[&num1]() -> const double& { return num1; }};
    auto prop_cr_s_ded = property{[&num1](const double& value) { num1 = value; }};

    static_assert(std::same_as<decltype(prop_cr2), decltype(prop_cr_ded)>);
    static_assert(std::same_as<decltype(prop_r_cr), decltype(prop_r_cr_ded)>);
    static_assert(std::same_as<decltype(prop_cr_g), decltype(prop_cr_g_ded)>);
    static_assert(std::same_as<decltype(prop_cr_s), decltype(prop_cr_s_ded)>);

    property<const double&, double> prop_cr_auto1 = {get_auto {num1}, [&num1](double a) { num1 = a; }};
    property<const double&> prop_cr_auto2 = {get_auto {num1}, set_auto {num1}};
    property<const double&, double> prop_cr_auto3 = {[&num1]() -> const double& { return num1; }, set_auto {num1}};
    property<const double&, get_only> prop_cr_g_auto1 = {get_auto {num1}};
    property<const double&, get_only> prop_cr_g_auto2 = get_auto {num1};
    property<double, set_only> prop_cr_s_auto1 = {set_auto {num1}};
    property<double, set_only> prop_cr_s_auto2 = set_auto {num1};

    auto prop_cr_auto1_ded = property{ get_auto {num1}, [&num1](double a) { num1 = a; } };
    auto prop_cr_auto2_ded = property{ get_auto {num1}, set_auto {num1} };
    auto prop_cr_auto3_ded = property{ [&num1]() -> const double& { return num1; }, set_auto {num1} };
    auto prop_cr_g_auto1_ded = property{get_auto {num1}};
    auto prop_cr_s_auto1_ded = property{set_auto {num1}};

    static_assert(std::same_as<decltype(prop_cr_auto1), decltype(prop_cr_auto1_ded)>);
    static_assert(std::same_as<decltype(prop_cr_auto2), decltype(prop_cr_auto2_ded)>);
    static_assert(std::same_as<decltype(prop_cr_auto3), decltype(prop_cr_auto3_ded)>);
    static_assert(std::same_as<decltype(prop_cr_g_auto1), decltype(prop_cr_g_auto1_ded)>);
    static_assert(std::same_as<decltype(prop_cr_s_auto1), decltype(prop_cr_s_auto1_ded)>);

    static_assert(std::is_constructible_v<auto_property<double>, decltype(get), decltype(set)>);
    auto_property<double> auto_prop_v = {get, set, 10.0};
    static_assert(std::is_constructible_v<auto_property<double, get_only>, decltype(get)>);
    auto_property<const double&, get_only> auto_prop_v_g = {get, num1};
    auto_property<double&, set_only> auto_prop_v_s = {set, num1};

    auto auto_prop_v_ded = auto_property{get, set, 10.0};
    auto auto_prop_v_g_ded = auto_property<const double&, get_only>{get, num1};
    auto auto_prop_v_s_ded = auto_property<double&, set_only>{set, num1};

    static_assert(std::same_as<decltype(auto_prop_v), decltype(auto_prop_v_ded)>);
    static_assert(std::same_as<decltype(auto_prop_v_g), decltype(auto_prop_v_g_ded)>);
    static_assert(std::same_as<decltype(auto_prop_v_s), decltype(auto_prop_v_s_ded)>);
}
// clang-format on

TEST(CppProperty, AssignOperator)
{
    static_assert(std::is_assignable_v<property<const double&>, property<const double&>>);
    static_assert(std::is_assignable_v<property<const double&>, property<const int&>>);
    static_assert(std::is_assignable_v<property<const double&>, property<const double&, get_only>>);
    static_assert(!std::is_assignable_v<property<const double&>, property<const double&, set_only>>);
    static_assert(std::is_assignable_v<property<const double&, double>, property<const double&>>);
    static_assert(std::is_assignable_v<property<const double&, double>, property<const int&>>);
    static_assert(std::is_assignable_v<property<const double&, double>, property<const double&, get_only>>);
    static_assert(!std::is_assignable_v<property<const double&, double>, property<const double&, set_only>>);
    static_assert(!std::is_assignable_v<property<const double&, get_only>, property<const double&>>);
    static_assert(!std::is_assignable_v<property<const double&, get_only>, property<const int&>>);
    static_assert(!std::is_assignable_v<property<const double&, get_only>, property<const double&, get_only>>);
    static_assert(!std::is_assignable_v<property<const double&, get_only>, property<const double&, set_only>>);
    static_assert(std::is_assignable_v<property<const double&, set_only>, property<const double&>>);
    static_assert(std::is_assignable_v<property<const double&, set_only>, property<const int&>>);
    static_assert(std::is_assignable_v<property<const double&, set_only>, property<const double&, get_only>>);
    static_assert(!std::is_assignable_v<property<const double&, set_only>, property<const double&, set_only>>);

    static_assert(std::is_assignable_v<property<const double&>, double>);
    static_assert(std::is_assignable_v<property<const double&, double>, double>);
    static_assert(!std::is_assignable_v<property<const double&, get_only>, double>);
    static_assert(std::is_assignable_v<property<const double&, set_only>, double>);
}

TEST(CppProperty, Cast)
{
    constexpr auto BASE_VALUE = 1.0;
    auto num1 = BASE_VALUE;
    property<const double&> prop_cr = {[&num1]() -> const double& { return num1; }, [&num1](double a) { num1 = a; }};
    property<double&, double> prop_r = {[&num1]() -> double& { return num1; }, [&num1](double a) { num1 = a; }};
    property<const double&, get_only> prop_cr_g = {[&num1]() -> const double& { return num1; }};
    property<const double&, set_only> prop_cr_s = {[&num1](const auto& value) { num1 = value; }};

    // explicit cast
    static_assert(std::same_as<const double&, decltype(prop_cr())>);
    EXPECT_EQ(num1, prop_cr());
    static_assert(std::same_as<double&, decltype(prop_r())>);
    prop_r() *= 2;
    EXPECT_EQ(2 * BASE_VALUE, num1);
    EXPECT_EQ(num1, prop_r());
    static_assert(std::same_as<const double&, decltype(prop_cr_g())>);
    EXPECT_EQ(num1, prop_cr_g());

    // implicit cast
    constexpr auto func = [](double d) { return d; };
    static_assert(std::same_as<double, decltype(func(prop_cr))>);
    static_assert(std::same_as<double, decltype(func(prop_r))>);
    static_assert(std::same_as<double, decltype(func(prop_cr_g))>);
}

TEST(CppProperty, Operators)
{
    constexpr auto VALUE = 3.0;
    auto real = 1.0;
    struct
    {
        int num = 2;
    } obj;
    auto opt = std::optional(obj);

    auto p_real = property{[&real]() -> const decltype(real)& { return real; },
                           [&real](const decltype(real)& value) { real = value; }};
    auto p_obj =
        property{[&obj]() -> const decltype(obj)& { return obj; }, [&obj](const decltype(obj)& value) { obj = value; }};
    auto p_objp = property{[&obj]() -> const decltype(obj)* { return &obj; }};
    auto p_opt =
        property{[&opt]() -> const decltype(opt)& { return opt; }, [&opt](const decltype(opt)& value) { opt = value; }};

    // arrow operator
    EXPECT_EQ(obj.num, p_objp->num);
    EXPECT_EQ(obj.num, p_opt->num);

    // TODO: operators leftover
    EXPECT_EQ(real + VALUE, p_real + VALUE);
    EXPECT_EQ(VALUE + real, VALUE + p_real);
    EXPECT_EQ(real - VALUE, p_real - VALUE);
    EXPECT_EQ(VALUE - real, VALUE - p_real);
    EXPECT_EQ(real * VALUE, p_real * VALUE);
    EXPECT_EQ(VALUE * real, VALUE * p_real);
    EXPECT_EQ(real / VALUE, p_real / VALUE);
    EXPECT_EQ(VALUE / real, VALUE / p_real);

    auto result = real + VALUE;
    p_real += VALUE;
    EXPECT_EQ(result, real);
    EXPECT_EQ(result, p_real);
}
// NOLINTEND
