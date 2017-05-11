#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) < (b) ? (b) : (a))

#define GETX(c) ((c)/Y)
#define GETY(c) ((c)%Y)
#define GETC(x,y) ((x)*Y+(y))

template <typename T>
T softmax(T a, T b, float softness)
{
    return (a*pow(a, softness) + b*pow(b, softness)) / (pow(a, softness) + pow(b, softness));
}

template <typename T>
T softmin(T a, T b, T softness)
{
    return (a*pow(-a, softness) + b*pow(-b, softness)) / (pow(-a, softness) + pow(-b, softness));
}

vector<string> split(const string &s, char d) { vector<string> res; stringstream ss(s); string t; while (getline(ss, t, d)) res.push_back(t); return res; }

template <int BASE>
float halton(int index)
{
    float result = 0;
    float f = 1. / BASE;
    int i = index;
    while (i > 0)
    {
        result += f * (i % BASE);
        i /= BASE;
        f /= BASE;
    }
    return result;
}

#pragma GCC optimize "O3,omit-frame-pointer,inline"

__attribute__((optimize("-O3")))
int f() {}

#define INLINE __attribute__((always_inline))

#define MARGIN 4
#define MAXXY (100+2*MARGIN)
#define OFFSET (MARGIN*(MAXXY+1))
char OK[MAXXY*MAXXY];
for (int x = 0; x < MAXXY; ++x)
    for (int y = 0; y < MAXXY; ++y)
        OK[MAXXY*y+x] = (x >= MARGIN && x < X+MARGIN && y >= MARGIN && y < Y+MARGIN);


#define MAXN 100000
int N;
int A[2*MAXN];
int R[4*MAXN];
void initRangeMax()
{
    int N2 = 1; while (N2 < N) N2 *= 2;
    for (int i = 0; i < N2; ++i)
        R[N2+i] = A[i];
    for (int i = N2-1; i >= 0; --i)
        R[i] = max(R[2*i], R[2*i+1]);
}
int rangeMax(unsigned int a, unsigned int b)
{
    int N2 = 1; while (N2 < N) N2 *= 2;
    int res = 0;
    while (a < b)
    {
        unsigned int p = 0;
        if (a == 0)
            p = 31;
        else
            while (! (a & (1<<p))) p++;
        while (a + (1<<p) > b) p--;
        a += (1<<p);
        res = max(res, R[((N2+a-1)>>p)]);
    }
    return res;
}
