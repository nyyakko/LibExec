# process execution library.

# installation

you may install it with [CPM](https://github.com/cpm-cmake/CPM.cmake) or install directly into your system with the following: 

* ``py install.py``

and then include it with cmake into your project

```cmake
cmake_minimum_required_version(VERSION 3.25)

project(CoolProject LANGUAGES CXX)

find_package(LibExec CONFIG REQUIRED)
add_executable(CoolProject source.cpp)
target_link_libraries(CoolProject PRIVATE LibExec::LibExec)
```

# examples
```c++
#include <libexec/Execute.hpp>

#include <iostream>

int main()
{
    auto result = libexec::execute("echo", { "hello, world!" });

    if (!result.has_value())
    {
        std::cerr << result.error().message();
        return EXIT_FAILURE;
    }

    auto [out, err] = result.value();

    std::cout << "stdout: " << out << '\n';
    std::cout << "stderr: " << err << '\n';

    return EXIT_SUCCESS;
}
```

i recommend you to simply explore the code and see what you can do with it. seriously. do it.
