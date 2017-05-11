#include <iostream>
#include <cstdlib>
#include <cstring>
#include <cassert>

using namespace std;

#define N (1<<24)

uint32_t seed_ = 123456789; double nextDouble() { return (1./~((uint32_t)0)) * (seed_ = 1664525 * seed_ + 1013904223); } uint32_t nextInt(int m) { return m * (1./~((uint32_t)0)) * (seed_ = 1664525 * seed_ + 1013904223); }

int * A;

void init()
{
    A = new int[N];
    for (int n = 0; n < N; ++n)
        A[n] = n;
}

int find(int n)
{
    return (A[n] == n) ? n : A[n] = find(A[n]);
}

int height(int n)
{
    return (A[n] == n) ? 0 : height(A[n])+1;
}

void merge(int n1, int n2)
{
    if (height(n1) > height(n2))
        A[find(n2)] = find(n1);
    else
        A[find(n1)] = find(n2);
}

int main()
{
    init();

    for (int n = 0; n < N; ++n)
        merge(nextInt(N), nextInt(N));

    int sum = 0;
    for (int n = 0; n < N; ++n)
        sum += height(n);
    cout << sum / (double)N << endl;
}
