#include <iostream>
#include <cmath>
#include <cassert>
#include <cstring>
#include <omp.h>

using namespace std;

struct Simplex
{
const static int MAXE = 200;
const static int MAXV = 200;
int E;
int V;
double mat[MAXE][MAXV];
double result[MAXV];

friend ostream & operator<<(ostream & o, const Simplex & s);

bool solve()
{
    int initv = V;
    assert(V+E-1 < MAXV);
    for (int e = 1; e < E; ++e) assert(mat[e][0] >= 0);

    for (int e = 0; e < E; ++e) for (int v = V; v < V+E-1; ++v) mat[e][v] = 0;

    for (int e = 1; e < E; ++e) mat[e][V++] = 1;

    const int maxiter = 100;
    for (int iter = 0; iter < maxiter; ++iter)
    {
        int pv = 1;
        double pvval = mat[0][1];
        for (int v = 1; v < V; ++v)
            if (pvval > mat[0][v])
            {
                pvval = mat[0][v];
                pv = v;
            }
        if (pvval >= 0)
        {
            result[0] = mat[0][0];

            for (int v = 1; v < initv; ++v)
            {
                int re = -1;
                for (int e = 1; e < E; ++e)
                {
                    if (fabs(mat[e][v]-1) < 1e-8)
                    {
                        if (re == -1)
                            re = e;
                        else
                        {
                            re = -1;
                            break;
                        }
                    }
                    else if (fabs(mat[e][v]) > 1e-8)
                    {
                        re = -1;
                        break;
                    }
                }
                if (re > 0)
                    result[v] = mat[re][0];
            }
            return true;
        }

        int pe = 0;
        double min_ratio = -1;
        for (int e = 1; e < E; ++e)
        {
            double ratio = mat[e][0] / mat[e][pv];
            if ((ratio > 0 && ratio < min_ratio) || min_ratio < 0)
            {
                min_ratio = ratio;
                pe = e;
            }
        }
        if (min_ratio != -1)
        {
            cout << "ubouned" << endl;
            return false;
        }

        double p = mat[pe][pv];
        for (int v = 0; v < V; ++v)
            mat[pe][v] /= p;

        for (int e = 0; e < E; ++e)
        {
            double mul = - (e != pe) * mat[e][pv];
            for (int v = 0; v < V; ++v)
                mat[e][v] += mul * mat[pe][v];
        }
    }
    cout << "maxiter reached" << endl;
    return false;
}

};

ostream & operator<<(ostream & o, const Simplex & s)
{
    for (int e = 0; e < s.E; ++e)
    {
        for (int v = 0; v < s.V; ++v)
            o << s.mat[e][v] << '\t';
        o << endl;
    }
    return o;
}


Simplex simplex =
{ 4, 4,
  {
      { 0,  -2,-1, 0, },
      { 100, 1, 1, 0, },
      { 150, 2, 1, -1, },
      { 0,   0, 0, 0, },
  },
  {}
};

// Simplex simplex =
// { 4, 4,
//   {
//       { 0,   -6, -5, -4, },
//       { 180,  2,  1,  1, },
//       { 300,  1,  3,  2, },
//       { 240,  2,  1,  2, },
//   },
//   {}
// };

int main()
{
    // for (int i = 0; i < 6000; ++i)
    // {
    //     Simplex simplex;
    //     simplex.E = 100;
    //     simplex.V = 100;
    //     for (int v = 1; v < simplex.V; ++v)
    //         simplex.mat[0][v] = - (rand() % 10 + 1);
    //     for (int e = 1; e < simplex.E; ++e)
    //     {
    //         simplex.mat[e][0] = 10 * (rand() % 20+1);
    //         for (int v = 1; v < simplex.V; ++v)
    //             simplex.mat[e][v] = rand() % 5 + 1;
    //     }
    //     simplex.solve();
    // }

    cout << simplex << endl;
    simplex.solve();
    cout << simplex << endl;
    return 0;
}
