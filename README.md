# cpp-property

C#-like property for C++20.

## Features

*   C# like syntax.
*   Minimum overhead.
*   Operator overloadings for transparent accesses.
*   Compile-time checks for dangling reference.

## Example

### Basic Usage

```cpp
#include <iostream>
#include "cpp_property.hpp"

// macro for importing keywords of properties without namespace specifier
import_cpp_property();

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
            // "value" is implicit argument like C#
            if (value < 0) throw std::invalid_argument("value must be >= 0");
            num_ = value;
        }
    };
};

...

auto a = A();

// set access
a.num = 1.0;

// get access
std::cout << a.num << std::endl;

// operator+= with accessor check
a.num += -2.0  // throw
```

### Auto-Implemented Getter and Setter

The trivial getters and setters can be implemented automatically, which is more efficient and has minimum overhead when optimized.

```cpp
#include "cpp_property.hpp"

import_cpp_property();

class A
{
    double num_ = 0;

public:
    // auto-impelemented getter
    property<const double&> num_auto_get
    {
        get_auto { num_ },
        set_val
        {
            if (value < 0) throw std::invalid_argument("value must be >= 0");
            num_ = value;
        }
    };

    // auto-impelemented setter
    property<const double&> num_auto_set
    {
        get_cref
        {
            return num_;
        },
        set_auto { num_ }
    };

    // auto-impelemented getter and setter
    property<const double&> num_auto_both
    {
        get_auto { num_ },
        set_auto { num_ }
    };
};
```

### Get/Set-Only Properties

It may be simpler to implement the function directly, but get/set-only properties are also available.

```cpp
#include "cpp_property.hpp"

import_cpp_property();

class A
{
    double num_ = 0;

public:
    // get-only property is specified in the template argument
    property<double, get_only> numsq_get_only
    {
        get_val
        {
            return num_ * num_;
        }
    }

    // set-only property is specified in the template argument
    property<double, set_only> num_set_only
    {
        set_val
        {
            num_ = value;
        }
    };

    // syntax like "Expression bodies on method-like members in C#"
    property<double, get_only> numsq_get_only2 = get_val { return num_ * num_; };
    property<const double&, get_only> num_get_only_auto = get_auto { num_ };
    property<double, set_only> num_set_only_auto = set_auto { num_ };
};
```

### Auto-Implemented Properties

Auto-implemented properties have backing fields. The difference with C# is that the return type can be specified.

```cpp
#include "cpp_property.hpp"

import_cpp_property();

class A
{
    double num_;

public:
    // auto_property includes backing field
    auto_property<double> num { get, set, 3.14 };

    // get/set-only auto-implemented property
    auto_property<const double&, get_only> num_get_only { get, num_ };
    auto_property<double&, set_only> num_get_only { set, num_ };
};
```

<!---
## Getting Started

## Performance
-->

## Future Issues

*   Property types cannot be deduced in member types and look dirty.
*   Reduce overhead of `std::function`
    *   Is `std::move_only_function` in C++23 more efficient?

<!---
## Reference
-->

## Author

Yoshiki Matsuda ([@yosh-matsuda](https://github.com/yosh-matsuda))
