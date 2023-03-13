#include <benchmark/benchmark.h>
#include "cpp_property.hpp"

import_cpp_property();

// clang-format off
class A
{
    double num_ = 0;

public:
    property<const double&> p_fn_fn
    {
        get_cref
        {
            return num_;
        },
        set_val
        {
            num_ = value;
        }
    };
    property<const double&> p_auto_fn
    {
        get_auto {num_},
        set_val
        {
            num_ = value;
        }
    };
    property<const double&> p_fn_auto
    {
        get_cref
        {
            return num_;
        },
        set_auto {num_}
    };
    property<const double&> p_auto_auto
    {
        get_auto {num_},
        set_auto {num_}
    };
    property<const double&, get_only> p_fn_get_only = get_cref { return num_; };
    property<const double&, get_only> p_auto_get_only = get_auto { num_ };
    property<double, set_only> p_fn_set_only = set_val
    {
        num_ = value;
    };
    property<double, set_only> p_auto_set_only = set_auto { num_ };

    auto_property<double> ap { get, set };

    [[nodiscard]] const auto& get_num() const { return num_; }
    void set_num(auto value)
    {
        num_ = value;
    }
};
// clang-format on

volatile double tmp;
auto a = A();

void get_fn_fn(benchmark::State& state)
{
    for (auto _ : state)
    {
        tmp = a.p_fn_fn;
    }
    auto x = auto_property<double>();
    auto y = auto_property<double>(x);
    x = y;
}
void get_auto_fn(benchmark::State& state)
{
    for (auto _ : state)
    {
        tmp = a.p_auto_fn;
    }
}
void get_ap(benchmark::State& state)
{
    for (auto _ : state)
    {
        tmp = a.ap;
    }
}
void get_call(benchmark::State& state)
{
    for (auto _ : state)
    {
        tmp = a.get_num();
    }
}
void set_p_fn_fn(benchmark::State& state)
{
    for (auto _ : state)
    {
        a.p_fn_fn = tmp;
    }
}
void set_p_fn_auto(benchmark::State& state)
{
    for (auto _ : state)
    {
        a.p_fn_auto = tmp;
    }
}
void set_ap(benchmark::State& state)
{
    for (auto _ : state)
    {
        a.ap = tmp;
    }
}
void set_num(benchmark::State& state)
{
    for (auto _ : state)
    {
        a.set_num(tmp);
    }
}
void get_fn_get_only(benchmark::State& state)
{
    for (auto _ : state)
    {
        tmp = a.p_fn_get_only;
    }
}
void get_auto_get_only(benchmark::State& state)
{
    for (auto _ : state)
    {
        tmp = a.p_auto_get_only;
    }
}
void set_p_fn_set_only(benchmark::State& state)
{
    for (auto _ : state)
    {
        a.p_fn_set_only = tmp;
    }
}
void set_p_auto_set_only(benchmark::State& state)
{
    for (auto _ : state)
    {
        a.p_auto_set_only = tmp;
    }
}

BENCHMARK(get_fn_fn);
BENCHMARK(get_auto_fn);
BENCHMARK(get_ap);
BENCHMARK(get_call);
BENCHMARK(set_p_fn_fn);
BENCHMARK(set_p_fn_auto);
BENCHMARK(set_ap);
BENCHMARK(set_num);
BENCHMARK(get_fn_get_only);
BENCHMARK(get_auto_get_only);
BENCHMARK(set_p_fn_set_only);
BENCHMARK(set_p_auto_set_only);

BENCHMARK_MAIN();
