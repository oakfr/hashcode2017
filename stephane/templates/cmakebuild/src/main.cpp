#include <iostream>
#include <cstdlib>
#include <typeindex>

using namespace std;

int a;

void g()
{
    for (int i = 0; i < 1e4; ++i)
        ++a;
}

void f()
{
    for (int i = 0; i < 1e5; ++i)
        g();
}

int main()
{
    f();
    cout << a << endl;
}
