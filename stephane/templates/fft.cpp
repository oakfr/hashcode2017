#include <iostream>
#include <cstdlib>
#include <cmath>
#include <cstring>

using namespace std;

#define MAXS 513

double oldmap[MAXS][MAXS];
double poffset[MAXS][MAXS];
double presult[MAXS][MAXS];

int S;

double are[2*MAXS][2*MAXS];
double aim[2*MAXS][2*MAXS];
double bre[2*MAXS][2*MAXS];
double bim[2*MAXS][2*MAXS];
double cre[2*MAXS][2*MAXS];
double cim[2*MAXS][2*MAXS];

void fft(double * re, double * im, int N, bool invert = false)
{
    for (int i = 1, j = 0; i < N; i++)
    {
        int bit = N >> 1;
        for (; j >= bit; bit >>= 1)
            j -= bit;
        j += bit;
        if (i < j)
        {
            double temp = re[i];
            re[i] = re[j];
            re[j] = temp;
            temp = im[i];
            im[i] = im[j];
            im[j] = temp;
        }
    }

    for (int len = 2; len <= N; len <<= 1)
    {
        int halfLen = len >> 1;
        double angle = 2 * M_PI / len;
        if (invert)
            angle = -angle;
        double wLenRe = cos(angle);
        double wLenIm = sin(angle);
        for (int i = 0; i < N; i += len)
        {
            double wRe = 1;
            double wIm = 0;
            for (int j = 0; j < halfLen; j++)
            {
                double uRe = re[i + j];
                double uIm = im[i + j];
                double vRe = re[i + j + halfLen] * wRe - im[i + j + halfLen] * wIm;
                double vIm = re[i + j + halfLen] * wIm + im[i + j + halfLen] * wRe;
                re[i + j] = uRe + vRe;
                im[i + j] = uIm + vIm;
                re[i + j + halfLen] = uRe - vRe;
                im[i + j + halfLen] = uIm - vIm;
                double nextWRe = wRe * wLenRe - wIm * wLenIm;
                wIm = wRe * wLenIm + wIm * wLenRe;
                wRe = nextWRe;
            }
        }
    }

    if (invert)
        for (int i = 0; i < N; i++)
        {
            re[i] /= N;
            im[i] /= N;
        }
}

int powerof2(int v)
{
    v--;
    v |= v >> 1;
    v |= v >> 2;
    v |= v >> 4;
    v |= v >> 8;
    v |= v >> 16;
    v++;
    return v;
}

void fft2d(double (*re)[2*MAXS], double (*im)[2*MAXS], int NX, int NY, bool invert = false)
{
    double tre[max(NX,NY)];
    double tim[max(NX,NY)];
    for (int y = 0; y < NY; ++y)
    {
        for (int x = 0; x < NX; ++x)
        {
            tre[x] = re[x][y];
            tim[x] = im[x][y];
        }
        fft(tre, tim, NX, invert);
        for (int x = 0; x < NX; ++x)
        {
            re[x][y] = tre[x];
            im[x][y] = tim[x];
        }
    }
    for (int x = 0; x < NX; ++x)
        fft(re[x], im[x], NY, invert);
}

void fftmult(int N)
{
    for (int x = 0; x < N; ++x)
        for (int y = 0; y < N; ++y)
        {
            cre[x][y] = are[x][y] * bre[x][y] - aim[x][y] * bim[x][y];
            cim[x][y] = aim[x][y] * bre[x][y] + are[x][y] * bim[x][y];
        }
}

void fftconvolve()
{
    int S2 = powerof2(2*S);

    for (int x = 0; x < S; ++x)
        for (int y = 0; y < S; ++y)
            are[x][y] = oldmap[x][y];
    fft2d(are, aim, S2, S2);

    for (int x = 0; x < S; ++x)
        for (int y = 0; y < S; ++y)
            bre[x][y] = poffset[(S-x)%S][(S-y)%S];
    fft2d(bre, bim, S2, S2);

    fftmult(S2);

    fft2d(cre, cim, S2, S2, true);
    for (int x = 0; x < S2; ++x)
        for (int y = 0; y < S2; ++y)
            presult[x%S][y%S] += cre[x][y];
}


#define PRINT(array) \
    cout << #array << endl; \
    for (int x = 0; x < S; ++x) \
    { \
        for (int y = 0; y < S; ++y) \
            cout << (array[x][y] > 1e-10 ? array[x][y] : 0.) << '\t'; \
        cout << endl; \
    } \
    cout << endl

uint32_t seed_ = 123456789; inline double nextDouble() { return (1./~((uint32_t)0)) * (seed_ = 1664525 * seed_ + 1013904223); } inline uint32_t nextInt(int m) { return m * (1./(1L<<32)) * (seed_ = 1664525 * seed_ + 1013904223); }

int main()
{
    S = 10;

    int k = 4;
    for (int i = 0; i < k; ++i)
        poffset[nextInt(S)][nextInt(S)] += 1./k;
    for (int i = 0; i < 2; ++i)
        oldmap[nextInt(S)][nextInt(S)] = 1;

    PRINT(oldmap);
    PRINT(poffset);

    fftconvolve();

    PRINT(presult);
}
