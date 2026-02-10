#include <iostream>

#include <emscripten.h>
#include <emscripten/bind.h>

using namespace emscripten;

int main()
{
    std::cout << "Hello world!\n";

    int x = 5;

    std::cout << "The number is " << x << "\n";

    return 0;
}

float lerp(float a, float b, float t)
{
    return (1 - t) * a + t * b;
}

EMSCRIPTEN_BINDINGS(my_module)
{
    function("lerp", &lerp);
}