#include <cstring>
#include <limits>
#include <algorithm>

#define MAXN 100

template <typename COST_T>
COST_T linearAssignement(const COST_T cost[MAXN][MAXN], int n, int xy[MAXN], int yx[MAXN])
{
    COST_T mincost = 0;
    COST_T reduction[MAXN];
    int matches[MAXN];

    memset(&matches, 0, sizeof(matches));

    for (int y = 0; y < n; y++)
    {
        mincost = cost[0][y];
        int xmin = 0;
        for (int x = 1; x < n; x++)
            if (mincost > cost[x][y])
            {
                mincost = cost[x][y];
                xmin = x;
            }
        reduction[y] = mincost;

        if (++matches[xmin] == 1)
        {
            xy[xmin] = y;
            yx[y] = xmin;
        }
        else
            yx[y] = -1;
    }

    int nbfree = 0;
    int freeRows[MAXN];
    for (int x = 0; x < n; x++)
        if (matches[x] == 0)
            freeRows[nbfree++] = x;
        else if (matches[x] == 1)
        {
            int y1 = xy[x];
            mincost = std::numeric_limits<COST_T>::max();
            for (int y = 0; y < n; y++)
                if (y != y1)
                    mincost = std::min(mincost, cost[x][y] - reduction[y]);
            reduction[y1] -= mincost;
        }

    for (int l = 0; l < 2; ++l)
    {
        int k = 0;
        int oldfree = nbfree;
        nbfree = 0;
        while (k < oldfree)
        {
            int x = freeRows[k];
            k++;

            COST_T umin = cost[x][0] - reduction[0];
            int y1 = 0;
            int y2 = 0;
            COST_T usubmin = std::numeric_limits<COST_T>::max();
            for (int y = 1; y < n; y++)
            {
                COST_T h = cost[x][y] - reduction[y];
                if (h < usubmin)
                {
                    if (h >= umin)
                    {
                        usubmin = h;
                        y2 = y;
                    }
                    else
                    {
                        usubmin = umin;
                        umin = h;
                        y2 = y1;
                        y1 = y;
                    }
                }
            }

            int x0 = yx[y1];
            if (umin < usubmin)
                reduction[y1] = reduction[y1] - (usubmin - umin);
            else if (x0 >= 0)
            {
                y1 = y2;
                x0 = yx[y2];
            }

            xy[x] = y1;
            yx[y1] = x;

            if (x0 >= 0)
            {
                if (umin < usubmin)
                    freeRows[--k] = x0;
                else
                    freeRows[nbfree++] = x0;
            }
        }
    }

    int last = 0;
    COST_T d[MAXN];
    int pred[MAXN];
    int collist[MAXN];
    for (int f = 0; f < nbfree; f++)
    {
        int freeRow = freeRows[f];
        int end;
        for (int y = 0; y < n; y++)
        {
            d[y] = cost[freeRow][y] - reduction[y];
            pred[y] = freeRow;
            collist[y] = y;
        }

        int back = 0;
        int front = 0;
        bool found = false;
        do
        {
            if (front == back)
            {
                last = back - 1;
                mincost = d[collist[front++]];
                for (int k = front; k < n; k++)
                {
                    int y = collist[k];
                    COST_T h = d[y];
                    if (h <= mincost)
                    {
                        if (h < mincost)
                        {
                            front = back;
                            mincost = h;
                        }
                        collist[k] = collist[front];
                        collist[front++] = y;
                    }
                }
                for (int k = back; k < front; k++)
                    if (yx[collist[k]] == -1)
                    {
                        end = collist[k];
                        found = true;
                        break;
                    }
            }

            if (!found)
            {
                int y1 = collist[back++];
                int x = yx[y1];
                COST_T h = cost[x][y1] - reduction[y1] - mincost;

                for (int k = front; k < n; k++)
                {
                    int y = collist[k];
                    COST_T v2 = cost[x][y] - reduction[y] - h;
                    if (v2 < d[y])
                    {
                        pred[y] = x;
                        if (v2 == mincost)
                        {
                            if (yx[y] == -1)
                            {
                                end = y;
                                found = true;
                                break;
                            }
                            else
                            {
                                collist[k] = collist[front];
                                collist[front++] = y;
                            }
                        }
                        d[y] = v2;
                    }
                }
            }
        }
        while (!found);

        for (int k = 0; k <= last; k++)
            reduction[collist[k]] += d[collist[k]] - mincost;

        int x;
        do
        {
            x = pred[end];
            yx[end] = x;
            std::swap(end, xy[x]);
        }
        while (x != freeRow);
    }

    COST_T res = 0;
    for (int x = 0; x < n; x++)
        res += cost[x][xy[x]];

    return res;
}



double cost[MAXN][MAXN] =
{
    { .1, .2, .3 },
    { .4, .5, .3 },
    { .7, .4, .8 },
};

int main()
{
    int xy[MAXN];
    int yx[MAXN];
    double mincost = linearAssignement<double>(cost, 3, xy, yx);
    cout << "mincost = " << mincost << endl;
    for (int x = 0; x < 3; ++x)
    {
        for (int y = 0; y < 3; ++y)
            if (xy[x] == y)
                cout << '[' << cost[x][y] << "]\t";
            else
                cout << cost[x][y] << '\t';
        cout << endl;
    }
}
