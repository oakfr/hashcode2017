#include <iostream>
#include <cstdlib>

using namespace std;

#define MAXN 100
int N;
int A[MAXN][MAXN];

void pseudoHilbert(bool vertical, int xmin, int xmax, int ymin, int ymax, int & index)
{
    int dx = xmax - xmin;
    int dy = ymax - ymin;

    if (dx == 0 && dy == 0)
    {
        A[xmin][ymin] = index++;
        return;
    }

    if (dx == 0)
    {
        A[xmin][ymin] = index++;
        A[xmin][ymax] = index++;
        return;
    }

    if (dy == 0)
    {
        A[xmin][ymin] = index++;
        A[xmax][ymin] = index++;
        return;
    }

    int xsplit1;
    int xsplit2;
    if ((xmin+xmax) & 1 == 0)
    {
        if (abs(xmax-xmin) & 3 == 2)
        {
            xsplit1 = (xmax+xmin)/2;
            xsplit2 = (xmin < xmax) ? xsplit1+1 : xsplit1-1;
        }
        else
        {
            xsplit2 = (xmax+xmin)/2;
            xsplit1 = (xmin < xmax) ? xsplit2-1 : xsplit2+1;
        }
    }
    else
    {
        xsplit1 = (xmax+xmin)/2;
        xsplit2 = xsplit1+1;
        if (xmin > xmax) swap(xsplit1, xsplit2);
    }

    int ysplit1;
    int csplit2;
    if ((ymin+ymax) & 1 == 0)
    {
        if (abs(ymax-ymin) & 3 == 2)
        {
            ysplit1 = (ymax+ymin)/2;
            csplit2 = (ymin < ymax) ? ysplit1+1 : ysplit1-1;
        }
        else
        {
            csplit2 = (ymax+ymin)/2;
            ysplit1 = (ymin < ymax) ? csplit2-1 : csplit2+1;
        }
    }
    else
    {
        ysplit1 = (ymax+ymin)/2;
        csplit2 = ysplit1+1;
        if (ymin > ymax) swap(ysplit1, csplit2);
    }

    if (vertical)
    {
        pseudoHilbert(false, xmin,    xsplit1, ymin,    ysplit1, index);
        pseudoHilbert(true,  xmin,    xsplit1, csplit2, ymax,    index);
        pseudoHilbert(true,  xsplit2, xmax,    csplit2, ymax,    index);
        pseudoHilbert(false, xmax,    xsplit2, ysplit1, ymin,    index);
    }
    else
    {
        pseudoHilbert(true,  xmin,    xsplit1, ymin,    ysplit1, index);
        pseudoHilbert(false, xsplit2, xmax,    ymin,    ysplit1, index);
        pseudoHilbert(false, xsplit2, xmax,    csplit2, ymax,    index);
        pseudoHilbert(true,  xsplit1, xmin,    ymax,    csplit2, index);
    }
}

int main()
{
    N = 10;
    int index = 0;
    pseudoHilbert(false,
                  0, N-1,
                  0, N-1,
                  index);
    for (int x = 0; x < N; ++x)
    {
        for (int y = 0; y < N; ++y)
            cout << '\t' << A[x][y];
        cout << endl;
    }
}
