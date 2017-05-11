#include <iostream>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <cassert>
#include <algorithm>

using namespace std;

uint32_t seed_ = 123456789; inline double nextDouble() { return (1./~((uint32_t)0)) * (seed_ = 1664525 * seed_ + 1013904223); } inline uint32_t nextInt(int m) { return m * (1./(1L<<32)) * (seed_ = 1664525 * seed_ + 1013904223); }

#define N 1000

int M[N][N];
int A[N];
int bestsolution[N];
extern int score;

struct Perturbation
{
    int a;
    int b;

    void randomize()
    {
        do
        {
            a = rand() % N;
            b = rand() % N;
        } while (a == b);
    }

    int apply()
    {
        int dscore = 0;
        dscore -= (a > 0)   * M[A[a-1]][A[a]];
        dscore -= (a < N-1) * M[A[a+1]][A[a]];
        dscore -= (b > 0)   * M[A[b-1]][A[b]];
        dscore -= (b < N-1) * M[A[b+1]][A[b]];

        swap(A[a], A[b]);

        dscore += (a > 0)   * M[A[a-1]][A[a]];
        dscore += (a < N-1) * M[A[a+1]][A[a]];
        dscore += (b > 0)   * M[A[b-1]][A[b]];
        dscore += (b < N-1) * M[A[b+1]][A[b]];

        return score + dscore;
    }

    void cancel()
    {
        swap(A[a], A[b]);
    }
};

int eval()
{
    int res = 0;
    for (int n = 0; n < N-1; ++n)
        res += M[A[n]][A[n+1]];
    return res;
}

void init()
{
    for (int x = 0; x < N; ++x)
        for (int y = x+1; y < N; ++y)
            M[y][x] = M[x][y] = rand() % 1000;
    for (int n = 0; n < N; ++n)
        A[n] = n;
    random_shuffle(A, A+N);
}

void savebest()
{
    memcpy(bestsolution, A, sizeof(A));
}





int score;
int main()
{
    int bestscore;
    init();
    bestscore = score = eval();
    Perturbation p;
    double maxiter = 3e7;
    for (long long int iter = 0; iter < maxiter; ++iter)
    {
        p.randomize();
        int newscore = p.apply();
        int dscore = newscore - score;
        // if (iter % 1000000 == 0)
            // cout << __func__ << ":" << __LINE__ << " " << score << " " << iter << endl;
        double temperature = 100. * (maxiter-iter) / maxiter;
        if (dscore <= 0 || exp(-dscore / temperature) > nextDouble())
        {
            score = newscore;
            if (score < bestscore)
            {
                savebest();
                bestscore = score;
                cout << score << endl;
            }
        }
        else
        {
            p.cancel();
        }
    }
    cout << bestscore << endl;
}
