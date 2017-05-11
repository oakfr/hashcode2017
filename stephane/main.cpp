#include <cstring>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <cassert>
#include <algorithm>
#include <fstream>
#include <queue>
#include <set>
#include <vector>
#include <cstdlib>
#include <libgen.h>
#include <unistd.h>

using namespace std;

#define MAXV 10000
#define MAXE 1000
#define MAXR 200000
#define MAXC 500

uint32_t seed_ = 123456789; inline double nextDouble() { return (1./~((uint32_t)0)) * (seed_ = 1664525 * seed_ + 1013904223); } inline uint32_t nextInt(int m) { return m * (1./(1L<<32)) * (seed_ = 1664525 * seed_ + 1013904223); }

int V;
int E;
int R;
int C;
int cachesize;
int nbrequest;

struct Video
{
    int size;
    int gain;
    double score;
    vector<int> requests;
    set<int> caches;
    long long int gains[MAXC];
};
Video videos[MAXV];

struct Endpoint
{
    int cachelatencies[MAXC];
    int latency;
    int videolatencies[MAXV];
};
Endpoint endpoints[MAXE];

struct Request
{
    int v;
    int e;
    int number;
};
Request requests[MAXR];

struct Cache
{
    int capacity;
    bool isvideoinside[MAXV];
    bool previousisvideoinside[MAXV];
};
Cache caches[MAXC];

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
    f << C << endl;
    for (int c = 0; c < C; ++c)
    {
        f << c;
        for (int v = 0; v < V; ++v)
            if (caches[c].isvideoinside[v])
                f << " " << v;
        f << endl;
    }
}

void readinput(const string & inputfile)
{
    fstream f(inputfile);
    f >> V >> E >> R >> C >> cachesize;
    for (int v = 0; v < V; ++v)
        f >> videos[v].size;
    for (int e = 0; e < E; ++e)
    {
        for (int c = 0; c < C; ++c)
            endpoints[e].cachelatencies[c] = -1;
        int latency;
        int EC;
        f >> latency >> EC;
        endpoints[e].latency = latency;
        for (int ec = 0; ec < EC; ++ec)
        {
            int latency;
            int cache;
            f >> cache >> latency;
            endpoints[e].cachelatencies[cache] = latency;
        }
    }
    for (int r = 0; r < R; ++r)
    {
        f >> requests[r].v >> requests[r].e >> requests[r].number;
        videos[requests[r].v].requests.push_back(r);
    }
    for (int r = 0; r < R; ++r)
        nbrequest += requests[r].number;
}

void readoutput(const string & outputfile)
{
    fstream f(outputfile);
    string s;
    getline(f, s);
    stringstream ss2(s);
    int C2;
    ss2 >> C2;
    for (int c2 = 0; c2 < C2; ++c2)
    {
        getline(f, s);
        stringstream ss(s);
        int c;
        ss >> c;
        while (ss.good())
        {
            int v;
            ss >> v;
            caches[c].isvideoinside[v] = true;
            videos[v].caches.insert(c);
        }
    }
}

long long int eval()
{
    long long int total = 0;
    for (int r = 0; r < R; ++r)
    {
        int e = requests[r].e;
        int lat = endpoints[e].latency;
        int v = requests[r].v;
        for (int c : videos[v].caches)
            if (endpoints[e].cachelatencies[c] != -1)
                lat = min(lat, endpoints[e].cachelatencies[c]);
        total += requests[r].number * (endpoints[e].latency - lat);
    }
    return 1000. * total / (double) nbrequest;
}

bool cmpvid(int left, int right)
{
    return videos[left].score > videos[right].score;
}

struct VCGain
{
    int v;
    int c;
    long long int gain;
    bool operator<(const VCGain & other) const
    {
        return gain/(double)videos[v].size < other.gain/(double)videos[other.v].size;
    }
};

void updategains(int v, priority_queue<VCGain> & q)
{
    for (int c = 0; c < C; ++c)
        videos[v].gains[c] = 0;

    long long int gain = 0;
    for (int r : videos[v].requests)
    {
        int e = requests[r].e;
        for (int c = 0; c < C; ++c)
            if (endpoints[e].cachelatencies[c] != -1 && !caches[c].isvideoinside[v])
            {
                gain = requests[r].number * (endpoints[e].videolatencies[v] - endpoints[e].cachelatencies[c]);
                if (gain > 0)
                    videos[v].gains[c] += gain;
            }
    }
    for (int c = 0; c < C; ++c)
        if (videos[v].gains[c] > 0 && caches[c].capacity >= videos[v].size)
        {
            videos[v].gains[c] *= 1 + .025 * nextDouble();
            q.push({v, c, videos[v].gains[c]});
        }
}

void initgreedy()
{
    for (int c = 0; c < C; ++c)
        caches[c].capacity = cachesize;

    for (int e = 0; e < E; ++e)
        for (int v = 0; v < V; ++v)
            endpoints[e].videolatencies[v] = endpoints[e].latency;
    for (int r = 0; r < R; ++r)
    {
        int e = requests[r].e;
        int v = requests[r].v;
        for (int c = 0; c < C; ++c)
            if (endpoints[e].cachelatencies[c] != -1 && caches[c].isvideoinside[v])
                endpoints[e].videolatencies[v] = min(endpoints[e].videolatencies[v], endpoints[e].cachelatencies[c]);
    }

    priority_queue<VCGain> q;
    for (int v = 0; v < V; ++v)
        updategains(v, q);

    while (!q.empty())
    {
        int v = q.top().v;
        int c = q.top().c;
        int gain = q.top().gain;
        q.pop();
        if (videos[v].gains[c] != gain)
            continue;
        if (caches[c].capacity < videos[v].size)
            continue;

        caches[c].capacity -= videos[v].size;
        caches[c].isvideoinside[v] = true;
        for (int e = 0; e < E; ++e)
            if (endpoints[e].cachelatencies[c] != -1)
                endpoints[e].videolatencies[v] = min(endpoints[e].videolatencies[v], endpoints[e].cachelatencies[c]);
        cout << eval() << " " << q.size() << " " << v << " " << c << endl;
        updategains(v, q);
    }
}

int score;
void improvegreedy(const string & inputfile)
{
    for (int c = 0; c < C; ++c)
        memcpy(caches[c].previousisvideoinside, caches[c].isvideoinside, sizeof(caches[c].isvideoinside));
    set<int> flushedcaches;
    for (int c = 0; c < (int)nextInt(10)+2; ++c)
        flushedcaches.insert(nextInt(C));
    for (int c : flushedcaches)
    {
        caches[c].capacity = cachesize;
        memset(caches[c].isvideoinside, 0, sizeof(caches[c].isvideoinside));
        for (int v = 0; v < V; ++v)
            videos[v].caches.erase(c);
    }

    for (int e = 0; e < E; ++e)
        for (int v = 0; v < V; ++v)
            endpoints[e].videolatencies[v] = endpoints[e].latency;
    for (int r = 0; r < R; ++r)
    {
        int e = requests[r].e;
        int v = requests[r].v;
        for (int c : videos[v].caches)
            if (endpoints[e].cachelatencies[c] != -1)
                endpoints[e].videolatencies[v] = min(endpoints[e].videolatencies[v], endpoints[e].cachelatencies[c]);
    }

    priority_queue<VCGain> q;
    for (int v = 0; v < V; ++v)
        updategains(v, q);

    while (!q.empty())
    {
        int v = q.top().v;
        int c = q.top().c;
        int gain = q.top().gain;
        q.pop();
        if (videos[v].gains[c] != gain)
            continue;
        if (caches[c].capacity < videos[v].size)
            continue;

        caches[c].capacity -= videos[v].size;
        caches[c].isvideoinside[v] = true;
        videos[v].caches.insert(c);
        for (int e = 0; e < E; ++e)
            if (endpoints[e].cachelatencies[c] != -1)
                endpoints[e].videolatencies[v] = min(endpoints[e].videolatencies[v], endpoints[e].cachelatencies[c]);
        updategains(v, q);
    }

    int newscore = eval();
    if (newscore >= score)
    {
        if (newscore > score)
        {
            cout << newscore << endl;
            if (newscore > 516550 && newscore < 1060000)
            {
                writeOutput(outputfile(inputfile, newscore));
                system("pkill exe");
            }
        }
        score = newscore;
    }
    else
    {
        for (int v = 0; v < V; ++v)
            videos[v].caches.clear();
        for (int c = 0; c < C; ++c)
        {
            for (int v = 0; v < V; ++v)
                if (caches[c].previousisvideoinside[v])
                {
                    caches[c].isvideoinside[v] = true;
                    videos[v].caches.insert(c);
                }
                else
                {
                    caches[c].isvideoinside[v] = false;
                }
        }

        for (int c = 0; c < C; ++c)
        {
            caches[c].capacity = cachesize;
            for (int v = 0; v < V; ++v)
                if (caches[c].isvideoinside[v])
                    caches[c].capacity -= videos[v].size;
        }
    }
}

void solve(const string & inputfile)
{
    int maxiter = 10000000;
    score = eval();
    for (int iter = 0; iter < maxiter; ++iter)
        improvegreedy(inputfile);
}


int main(int argc, char * argv[])
{
    seed_ = getpid() + time(NULL);
    srand(seed_);
    string inputfile = "../me_at_the_zoo.in";
    if (argc > 1)
        inputfile = argv[1];
    readinput(inputfile);
    if (argc > 2)
        readoutput(string(argv[2]));
    else
        // initrandom();
        initgreedy();
    solve(inputfile);
}
