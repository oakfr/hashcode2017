#include <iostream>
#include <iomanip>
#include <sstream>
#include <cassert>
#include <algorithm>
#include <fstream>
#include <vector>
#include <cstdlib>
#include <libgen.h>

using namespace std;

int C;
int T;
int S;
int score;
struct Sat
{
    int x0;
    int y0;
    int x;
    int y;
    int dy;
    int w;
    int d;
};
vector<Sat> sats;

struct Rng
{
    int s;
    int e;
};

struct Img
{
    int x;
    int y;
};

struct Col
{
    int pts;
    vector<Img> imgs;
    vector<Rng> rngs;
};
vector<Col> cols;

struct Picture
{
    int t;
    int dx;
    int dy;
    int c;
    int i;
};
vector<vector<Picture> > pics;

void readinput(const string & inputfile)
{
    fstream f(inputfile);
    f >> T;
    f >> S;
    sats.resize(S);
    for (int s = 0; s < S; ++s)
        f >> sats[s].y0 >> sats[s].x0 >> sats[s].dy >> sats[s].w >> sats[s].d;
    f >> C;
    cols.resize(C);
    for (int c = 0; c < C; ++c)
    {
        int I;
        int R;
        f >> cols[c].pts >> I >> R;
        cols[c].imgs.resize(I);
        for (int i = 0; i < I; ++i)
            f >> cols[c].imgs[i].y >> cols[c].imgs[i].x;
        cols[c].rngs.resize(R);
        for (int r = 0; r < R; ++r)
            f >> cols[c].rngs[r].s >> cols[c].rngs[r].e;
    }
}

void updatesat(int s)
{
    sats[s].x -= 15;
    sats[s].y += sats[s].dy;
    if (sats[s].y > 324000)
    {
        sats[s].y = 648000 - sats[s].y;
        sats[s].x = -648000 + sats[s].x;
        sats[s].dy = -sats[s].dy;
    }
    else if (sats[s].y < -324000)
    {
        sats[s].y = -648000 - sats[s].y;
        sats[s].x = -648000 + sats[s].x;
        sats[s].dy = -sats[s].dy;
    }
    while (sats[s].x < -648000) sats[s].x += 2*648000;
    while (sats[s].x >  648000) sats[s].x -= 2*648000;
    // cout << sats[s].y << " " << sats[s].x << endl;
}

bool cansee(int s, int c, int i, int t)
{
    const Img & img = cols[c].imgs[i];
    int dx = img.x - sats[s].x;
    int dy = img.y - sats[s].y;
    if (abs(dx) < sats[s].d && abs(dy) < sats[s].d)
    {
        for (int p = 0; p < (int)pics[s].size(); ++p)
        {
            int t2 = pics[s][p].t;
            int dx2 = pics[s][p].dx;
            int dy2 = pics[s][p].dy;
            int ddx = abs(dx2 - dx);
            int ddy = abs(dy2 - dy);
            int dt = abs(t2 - t);
            if (ddx == 0 && ddy == 0)
                continue;
            if (dt == 0 || ddx / dt > sats[s].w || ddy / dt > sats[s].w)
                return false;
        }
        pics[s].push_back({t, dx, dy, c, i});
        return true;
    }
    return false;
}

string outputfile(const string & inputfile, int score = -1)
{
    if (score == -1)
        score = time(NULL);
    string outputdir = "results/";
    string baseinput = string(basename((char*)inputfile.c_str()));
    stringstream ss;
    ss << baseinput.replace(baseinput.find(".in"), 3, "") << "_" << setfill('0') << setw(8) << score << ".out";
    return outputdir + ss.str();
}

void writeOutput(const string & outputfile)
{
    ofstream f(outputfile);
    int p = 0;
    for (int s = 0; s < S; ++s)
        p += (int)pics[s].size();
    f << p << endl;
    for (int s = 0; s < S; ++s)
        for (int i = 0; i < (int)pics[s].size(); ++i)
        {
            const Img & img = cols[pics[s][i].c].imgs[pics[s][i].i];
            f << img.y << " " << img.x << " " << pics[s][i].t << " " << s << endl;
        }
}

float colscore(int c)
{
    // int sumrngs = 0;
    // for (int r = 0; r < (int)cols[c].rngs.size(); ++r)
        // sumrngs += cols[c].rngs[r].e - cols[c].rngs[r].s + 1;
    return cols[c].pts / (float)cols[c].imgs.size();
}

struct Sorter
{
    bool operator()(int left, int right) const
    {
        return colscore(left) > colscore(right);
    }
};

void simulate(const string & inputfile)
{
    pics.resize(S);

    vector<int> colidx;
    for (int c = 0; c < C; ++c)
        colidx.push_back(c);
    // random_shuffle(colidx.begin(), colidx.end());
    Sorter sorter;
    sort(colidx.begin(), colidx.end(), sorter);

    for (int cidx = 0; cidx < C; ++cidx)
    {
        int c = colidx[cidx];
        const Col & col = cols[c];
        // cout << __func__ << ":" << __LINE__ << "C " << cidx << "/" << C << endl;
        for (int i = 0; i < (int)col.imgs.size(); ++i)
        {
            // const Img & img = col.imgs[i];
            // cout << __func__ << ":" << __LINE__ << "    I " << i << "/" << (int)col.imgs.size() << endl;

            for (int s = 0; s < S; ++s)
            {
                sats[s].x = sats[s].x0;
                sats[s].y = sats[s].y0;
            }

            int t;
            int r = 0;
            for (t = 0; t < T; ++t)
            {
                if (t > cols[c].rngs[r].e)
                {
                    r++;
                    if (r > (int)cols[c].rngs.size())
                        break;
                }
                for (int s = 0; s < S; ++s)
                {
                    if (t >= cols[c].rngs[r].s && t <= cols[c].rngs[r].e)
                        if (cansee(s, c, i, t))
                            goto found;
                    updatesat(s);
                }
            }
            // cout << __func__ << ":" << __LINE__ << " miss t=" << t << endl;
            goto nextcol;
            found:;
            // cout << __func__ << ":" << __LINE__ << " found t=" << t << endl;
        }
        score += col.pts;
        writeOutput(outputfile(inputfile, score));
        cout << __func__ << ":" << __LINE__ << " c=" << cidx << "/" << C <<" score=" << score << endl;
        nextcol:;
    }
}

int main(int argc, char * argv[])
{
    string inputfile = "../constellation.in";
    if (argc > 1)
        inputfile = argv[1];
    readinput(inputfile);
    simulate(inputfile);
}
