// beam lazy copy n steps
// mcts
// any time beamsearch

#include <iostream>
#include <cstdlib>
#include <vector>
#include <cassert>
#include <limits>
#include <cmath>
#include <cstring>
#include <algorithm>
#include <queue>

using namespace std;

uint32_t seed_ = 123456789; inline double nextDouble() { return (1./~((uint32_t)0)) * (seed_ = 1664525 * seed_ + 1013904223); } inline uint32_t nextInt(int m) { return m * (1./(1L<<32)) * (seed_ = 1664525 * seed_ + 1013904223); }

int benchmark;

template <class STATE, bool MAXIMIZE, bool STOCHASTIC = false>
struct GreedySearch
{
    typedef typename STATE::Eval Eval;
    typedef typename STATE::Move Move;
    typedef typename STATE::State State;
    typedef typename STATE::CancelState CancelState;
    const static bool Maximize = MAXIMIZE;
    const double POW = 3.;

    Eval search(State & root, vector<Move> & output)
    {
        State current = root;
        output.clear();
        vector<Move> moves;
        int nbties;
        double proba, sumproba;
        while (true)
        {
            current.getmoves(moves);
            assert(!moves.empty());
            Move bestmove;
            Eval besteval = MAXIMIZE ? numeric_limits<Eval>::min() : numeric_limits<Eval>::max();
            if (!STOCHASTIC)
                nbties = 1;
            else
                sumproba = 0;
            for (const Move & m : moves)
            {
                CancelState cs;
                current.apply(m, cs);
                Eval eval = current.eval();
                current.cancel(cs);
                if (STOCHASTIC)
                {
                    double p = (MAXIMIZE ? 1 : -1) * eval - 300;
                    proba = pow(POW, p);
                    sumproba += proba;
                    // cout << __func__ << ":" << __LINE__ << " " << sumproba << endl;
                }
                if ((!STOCHASTIC && ((MAXIMIZE && eval > besteval) || (!MAXIMIZE && eval < besteval) || (eval == besteval && nextInt(++nbties) == 0))) ||
                    (STOCHASTIC && (nextDouble() < proba / sumproba)))
                {
                    if (!STOCHASTIC && eval != besteval) nbties = 1;
                    besteval = eval;
                    bestmove = m;
                }
            }
            CancelState cs;
            current.apply(bestmove, cs);
            output.push_back(bestmove);
            if (current.done())
                return besteval;
        }
    }
};

template <class STATE, bool MAXIMIZE, bool STOCHASTIC = false>
struct TwoStepsSearch
{
    typedef typename STATE::Eval Eval;
    typedef typename STATE::Move Move;
    typedef typename STATE::State State;
    typedef typename STATE::CancelState CancelState;
    const static bool Maximize = MAXIMIZE;
    const double POW = 7.;
    Eval search(const State & root, vector<Move> & output)
    {
        vector<Move> moves;
        vector<Move> moves2;
        State current = root;
        output.clear();
        int nbties;
        double proba, sumproba;
        while (true)
        {
            current.getmoves(moves);
            assert(!moves.empty());
            Move bestmove;
            Eval besteval = MAXIMIZE ? numeric_limits<Eval>::min() : numeric_limits<Eval>::max();
            if (!STOCHASTIC)
                nbties = 1;
            else
                sumproba = 0;
            for (const Move & m : moves)
            {
                CancelState cs;
                current.apply(m, cs);
                if (current.done())
                {
                    Eval eval = current.eval();
                    if (STOCHASTIC)
                    {
                        double p = (MAXIMIZE ? 1 : -1) * eval - 300;
                        proba = pow(POW, p);
                        sumproba += proba;
                    }
                    // benchmark++;
                    if ((!STOCHASTIC && ((MAXIMIZE && eval > besteval) || (!MAXIMIZE && eval < besteval) || (eval == besteval && nextInt(++nbties) == 0))) ||
                        (STOCHASTIC && (nextDouble() < proba / sumproba)))
                    {
                        if (!STOCHASTIC && eval != besteval) nbties = 1;
                        besteval = eval;
                        bestmove = m;
                    }
                }
                else
                {
                    current.getmoves(moves2);
                    assert(!moves2.empty());
                    for (const Move & m2 : moves2)
                    {
                        CancelState cs;
                        current.apply(m2, cs);
                        Eval eval = current.eval();
                        if (STOCHASTIC)
                        {
                            double p = (MAXIMIZE ? 1 : -1) * eval - 300;
                            proba = pow(POW, p);
                            sumproba += proba;
                        }
                        // benchmark++;
                        if ((!STOCHASTIC && ((MAXIMIZE && eval > besteval) || (!MAXIMIZE && eval < besteval) || (eval == besteval && nextInt(++nbties) == 0))) ||
                            (STOCHASTIC && (nextDouble() < proba / sumproba)))
                        {
                            if (!STOCHASTIC && eval != besteval) nbties = 1;
                            besteval = eval;
                            bestmove = m;
                        }
                        current.cancel(cs);
                    }
                }
                current.cancel(cs);
            }
            CancelState cs;
            current.apply(bestmove, cs);
            output.push_back(bestmove);
            if (current.done())
                return besteval;
        }
    }
};

template <class STATE, bool MAXIMIZE, int NBSTEPS, bool STOCHASTIC = false>
struct NStepsSearch
{
    typedef typename STATE::Eval Eval;
    typedef typename STATE::Move Move;
    typedef typename STATE::State State;
    typedef typename STATE::CancelState CancelState;
    const static bool Maximize = MAXIMIZE;
    const double POW = 5.;
    Eval search(const State & root, vector<Move> & output)
    {
        vector<Move> moves;
        State current = root;
        output.clear();
        int nbties;
        double proba, sumproba;
        while (true)
        {
            current.getmoves(moves);
            assert(!moves.empty());
            Move bestmove;
            Eval besteval = MAXIMIZE ? numeric_limits<Eval>::min() : numeric_limits<Eval>::max();
            if (!STOCHASTIC)
                nbties = 1;
            else
                sumproba = 0;
            for (const Move & m : moves)
            {
                CancelState cs;
                current.apply(m, cs);
                if (current.done())
                {
                    Eval eval = current.eval();
                    if (STOCHASTIC)
                    {
                        double p = (MAXIMIZE ? 1 : -1) * eval - 300;
                        proba = pow(POW, p);
                        sumproba += proba;
                    }
                    if ((!STOCHASTIC && ((MAXIMIZE && eval > besteval) || (!MAXIMIZE && eval < besteval) || (eval == besteval && nextInt(++nbties) == 0))) ||
                        (STOCHASTIC && (nextDouble() < proba / sumproba)))
                    {
                        if (!STOCHASTIC && eval != besteval) nbties = 1;
                        besteval = eval;
                        bestmove = m;
                    }
                }
                else
                {
                    search(current, m, bestmove, besteval, nbties, sumproba, NBSTEPS-1);
                }
                current.cancel(cs);
            }
            CancelState cs;
            current.apply(bestmove, cs);
            output.push_back(bestmove);
            if (current.done())
                return besteval;
        }
    }

    void search(State & current, const Move & m, Move & bestmove, Eval & besteval, int & nbties, double & sumproba, int level)
    {
        vector<Move> moves2;
        current.getmoves(moves2);
        assert(!moves2.empty());
        double proba;
        for (const Move & m2 : moves2)
        {
            CancelState cs;
            current.apply(m2, cs);
            if (level != 1)
            {
                search(current, m, bestmove, besteval, nbties, sumproba, level-1);
            }
            else
            {
                Eval eval = current.eval();
                if (STOCHASTIC)
                {
                    double p = (MAXIMIZE ? 1 : -1) * eval - 300;
                    proba = pow(POW, p);
                    sumproba += proba;
                }
                if ((!STOCHASTIC && ((MAXIMIZE && eval > besteval) || (!MAXIMIZE && eval < besteval) || (eval == besteval && nextInt(++nbties) == 0))) ||
                    (STOCHASTIC && (nextDouble() < proba / sumproba)))
                {
                    if (!STOCHASTIC && eval != besteval) nbties = 1;
                    besteval = eval;
                    bestmove = m;
                }
            }
            current.cancel(cs);
        }
    }
};


template <class STATE, bool MAXIMIZE, int NBMINRANDOM, int NBMAXRANDOM>
struct RandomGreedySearch
{
    typedef typename STATE::Eval Eval;
    typedef typename STATE::Move Move;
    typedef typename STATE::State State;
    typedef typename STATE::CancelState CancelState;
    const static bool Maximize = MAXIMIZE;
    Eval search(const State & root, vector<Move> & output)
    {
        vector<Move> moves;
        State current = root;
        output.clear();

        int nbrandom = nextInt(NBMAXRANDOM-NBMINRANDOM+1)+NBMINRANDOM;
        while (nbrandom--)
        {
            current.getmoves(moves);
            assert(!moves.empty());
            Move move(moves[nextInt((int)moves.size())]);
            CancelState cs;
            current.apply(move, cs);
            output.push_back(move);
            if (current.done())
                return current.eval();
        }

        while (true)
        {
            current.getmoves(moves);
            assert(!moves.empty());
            Move bestmove;
            Eval besteval = MAXIMIZE ? numeric_limits<Eval>::min() : numeric_limits<Eval>::max();
            int nbties = 1;
            for (const Move & m : moves)
            {
                CancelState cs;
                current.apply(m, cs);
                Eval eval = current.eval();
                current.cancel(cs);
                if ((MAXIMIZE && eval > besteval) || (!MAXIMIZE && eval < besteval) || (eval == besteval && nextInt(++nbties) == 0))
                {
                    if (eval != besteval) nbties = 1;
                    besteval = eval;
                    bestmove = m;
                }
            }
            CancelState cs;
            current.apply(bestmove, cs);
            output.push_back(bestmove);
            if (current.done())
                return besteval;
        }
    }
};

template <class SEARCH, int ITER>
struct RepeatSearch
{
    typedef typename SEARCH::Eval Eval;
    typedef typename SEARCH::Move Move;
    typedef typename SEARCH::State State;
    Eval search(const State & root, vector<Move> & output)
    {
        vector<Move> bestoutput;
        SEARCH search;
        Eval besteval = SEARCH::Maximize ? numeric_limits<Eval>::min() : numeric_limits<Eval>::max();
        vector<Move> currentoutput;
        for (int i = 0; i < ITER; ++i)
        {
            currentoutput.clear();
            State current = root;
            Eval eval = search.search(current, currentoutput);
            if ((SEARCH::Maximize && eval > besteval) || (!SEARCH::Maximize && eval < besteval))
            {
                besteval = eval;
                bestoutput = currentoutput;
            }
        }
        output = bestoutput;
        return besteval;
    }
};


template <int SIZE>
struct IdPool
{
    IdPool()
    {
        clear();
    }
    void clear()
    {
        for (int i = 0; i < SIZE; ++i)
            ids[i] = SIZE-i-1;
        nbids = SIZE;
    }
    int get()
    {
        assert(nbids > 0);
        return ids[--nbids];
    }
    void put(int i)
    {
        assert(nbids < SIZE);
        ids[nbids++] = i;
    }
    int ids[SIZE];
    int nbids;
};


template <class STATE>
struct BeamSearchState : public STATE
{
    typedef typename STATE::Eval Eval;
    typedef typename STATE::Move Move;
    typedef typename STATE::State State;
    int id;
    int parent;
    short ref;
    Move move;
    Eval val;
    BeamSearchState() {}
    BeamSearchState(const State & root, int id) :
        State(root),
        id(id),
        parent(-1),
        ref(0),
        val(0)
    {}
};

template <class STATE, bool MAXIMIZE, int MAX_ITER, int MAX_STATE, int BEAM_SIZE, bool USE_HASH = false>
struct BeamSearch
{
    typedef typename STATE::Eval Eval;
    typedef typename STATE::Move Move;
    typedef typename STATE::State State;
    typedef typename STATE::CancelState CancelState;
    const static bool Maximize = MAXIMIZE;
    typedef BeamSearchState<State> BSS;
    int freeStates[MAX_STATE];
    int freeStateIdx;
#define HASH_SIZE (1<<20)
    bool hashset[HASH_SIZE];
    Eval lowerBound;

    static BeamSearchState<State> states[MAX_STATE];

    inline int getfreestate()
    {
        assert(freeStateIdx >= 0);
        return freeStates[freeStateIdx--];
    }

    struct StateCmp
    {
        inline bool operator()(int l, int r) const
        {
            if (MAXIMIZE)
                return states[l].val > states[r].val;
            return states[l].val < states[r].val;
        }
    };

    template <int RANGE, int SIZE>
    struct IPQ
    {
        IPQ()
        {
            memset(&first, -1, sizeof(first));
            memset(&next, -1, sizeof(next));
            minval = RANGE-1;
            maxval = 0;
        }
        void push(int s)
        {
            int val = states[s].val;
            assert(val >= 0 && val < RANGE);
            int newid = idPool.get();
            next[newid] = first[val];
            first[val] = newid;
            stateid[newid] = s;
            minval = min(minval, val);
            maxval = max(maxval, val);
        }
        int top()
        {
            if (MAXIMIZE)
            {
                for (; minval <= maxval; ++minval)
                    if (first[minval] != -1)
                        return stateid[first[minval]];
            }
            else
            {
                for (; maxval >= minval; --maxval)
                    if (first[maxval] != -1)
                        return stateid[first[maxval]];
            }
            assert(false);
        }
        void pop()
        {
            if (MAXIMIZE)
            {
                for (; minval <= maxval; ++minval)
                    if (first[minval] != -1)
                    {
                        idPool.put(first[minval]);
                        first[minval] = next[first[minval]];
                        return;
                    }
            }
            else
            {
                for (; maxval >= minval; --maxval)
                    if (first[maxval] != -1)
                    {
                        idPool.put(first[maxval]);
                        first[maxval] = next[first[maxval]];
                        return;
                    }
            }
            assert(false);
        }
        void clear()
        {
            for (; minval <= maxval; ++minval)
                while (first[minval] != -1)
                {
                    idPool.put(first[minval]);
                    first[minval] = next[first[minval]];
                }
            memset(&next, -1, sizeof(next));
            minval = RANGE-1;
            maxval = 0;
        };
        int size()
        {
            return SIZE - idPool.nbids;
        }
        bool empty()
        {
            return size() == 0;
        }
        void copy(vector<int> & v)
        {
            v.clear();
            for (int m = maxval; m >= minval; --m)
                if (first[m] != -1)
                {
                    int s = first[m];
                    do
                    {
                        v.push_back(stateid[s]);
                        s = next[s];
                    }
                    while (s != -1);
                }
        }
        IdPool<SIZE> idPool;
        int first[RANGE];
        int next[SIZE];
        int stateid[SIZE];
        int minval;
        int maxval;
        void dump()
        {
            for (int i = 0; i < RANGE; ++i) if (first[i] != -1) cout << "first[" << i << "]=" << first[i] << endl; cout << endl;
            for (int i = 0; i < MAX_STATE; ++i)  if (next[i] != -1)  cout << "next[" << i << "]=" << next[i] << endl;   cout << endl;
        }
    };
    template <int RANGE, int SIZE> void copy(IPQ<RANGE, SIZE> & q, vector<int> & v) { q.copy(v); }
    template <int RANGE, int SIZE> void clear(IPQ<RANGE, SIZE> & q) { q.clear(); }

    inline void garbage(BSS & state)
    {
        freeStates[++freeStateIdx] = state.id;
        BSS & parent = states[state.parent];
        parent.ref--;
        if (parent.ref == 0)
            garbage(parent);
    }

    template <class T, class S, class C>
    void copy(priority_queue<T, S, C>& q, S & s)
    {
        struct HackedQueue : private priority_queue<T, S, C> { static void copy(priority_queue<T, S, C>& q, S & s) { s = q.*&HackedQueue::c; } };
        HackedQueue::copy(q, s);
    }

    template <class T, class S, class C>
    void clear(priority_queue<T, S, C>& q)
    {
        q = priority_queue<T, S, C>();
    }

    Eval search(State & root, vector<Move> & output)
    {
        Eval besteval = MAXIMIZE ? numeric_limits<Eval>::min() : numeric_limits<Eval>::max();

        if (USE_HASH)
            State::inithash();

        // priority_queue<int, vector<int>, StateCmp> pq;
        IPQ<10*MAX_ITER, MAX_STATE> pq;

        memset(freeStates, 0, sizeof(freeStates));
        freeStateIdx = 0;
        for (int i = 0; i < MAX_STATE; ++i)
            freeStates[i] += MAX_STATE-i-1;
        freeStateIdx = MAX_STATE-1;

        BSS bsroot(root, getfreestate());
        pq.push(bsroot.id);
        states[bsroot.id] = bsroot;

        memset(hashset, 0, sizeof(hashset));
        vector<int> previous;
        vector<Move> moves;
        lowerBound = MAXIMIZE ? numeric_limits<Eval>::min() : numeric_limits<Eval>::max();
        const int HASHCLEAR = 1;
        for (int iter = 0; iter < MAX_ITER; ++iter)
        {
            if (USE_HASH && iter % HASHCLEAR == 0)
                memset(hashset, 0, sizeof(hashset));

            copy(pq, previous);
            clear(pq);

            int idx;
            for (idx = 0; idx < (int)previous.size() && idx < BEAM_SIZE; ++idx)
            {
                int stateId = previous[idx];
                BSS & state = states[stateId];

                if (iter != 0)
                {
                    BSS & parent = states[state.parent];
                    int id = state.id;
                    Move move = state.move;
                    state = parent;
                    state.id = id;
                    state.ref = 0;
                    state.move = move;
                    state.parent = parent.id;
                    CancelState cs;
                    state.apply(state.move, cs);
                }

                state.getmoves(moves);
                state.ref++;
                for (const Move & m : moves)
                {
                    CancelState cs;
                    state.apply(m, cs);
                    bool pruned = false;
                    // benchmark++;
                    if (USE_HASH && hashset[state.hash]) pruned = true;
                    if (!pruned && ((int)pq.size() < BEAM_SIZE ||
                                    (MAXIMIZE && state.eval() > lowerBound) ||
                                    (!MAXIMIZE && state.eval() < lowerBound)))
                    {
                        state.ref++;
                        int newStateId = getfreestate();
                        BSS & newState = states[newStateId];

                        newState.id = newStateId;
                        newState.parent = stateId;
                        newState.ref = 0;
                        newState.move = m;
                        newState.val = state.eval();
                        if (state.done())
                        {
                            if ((MAXIMIZE && newState.val > besteval) ||
                                (!MAXIMIZE && newState.val < besteval))
                            {
                                besteval = newState.val;
                                output.clear();
                                int idx = newState.id;

                                while (true)
                                {
                                    BSS & state = states[idx];
                                    idx = state.parent;
                                    if (idx == -1)
                                        break;
                                    else
                                        output.push_back(state.move);
                                }
                                reverse(output.begin(), output.end());
                            }
                        }
                        else
                        {
                            pq.push(newStateId);
                            if ((int)pq.size() > BEAM_SIZE)
                            {
                                garbage(states[pq.top()]);
                                pq.pop();
                            }
                            lowerBound = states[pq.top()].val;
                            if (USE_HASH) hashset[state.hash] = true;
                        }
                    }
                    state.cancel(cs);
                }
                if (--state.ref == 0)
                    garbage(state);
            }
        }

        return besteval;
    }
};

template <class STATE, bool MAXIMIZE, int MAX_ITER, int MAX_STATE, int BEAM_SIZE, bool USE_HASH>
BeamSearchState<typename BeamSearch<STATE, MAXIMIZE, MAX_ITER, MAX_STATE, BEAM_SIZE, USE_HASH>::State> BeamSearch<STATE, MAXIMIZE, MAX_ITER, MAX_STATE, BEAM_SIZE, USE_HASH>::states[MAX_STATE];


int DX[] = { -1, 1, 0, 0 };
int DY[] = { 0, 0, -1, 1 };
int N;
#define MAXN 30

int * MOD;
typedef int Move;
typedef int Eval;
struct CancelState
{
    int x;
    int y;
    int a;
    int hash;
};
int xhashes[MAXN];
int yhashes[MAXN];
int xyhashes[MAXN][MAXN];
struct State
{
    typedef int Move;
    typedef int Eval;
    typedef ::CancelState CancelState;
    unsigned char A[MAXN][MAXN];
    int x;
    int y;
    Eval score;
    int turn;
    int hash;
    void getmoves(vector<Move> & moves)
    {
        moves.clear();
        moves.push_back(0);
        moves.push_back(1);
        moves.push_back(2);
        moves.push_back(3);
    }
    void apply(const Move & move, CancelState & cs)
    {
        cs.hash = hash;
        cs.x = x;
        cs.y = y;
        hash ^= xhashes[x];
        hash ^= yhashes[y];
        x = MOD[x + DX[move]];
        y = MOD[y + DY[move]];
        hash ^= xhashes[x];
        hash ^= yhashes[y];
        hash ^= xyhashes[x][y];
        cs.a = A[x][y];
        score += A[x][y];
        A[x][y] = 0;
        turn++;
    }
    void cancel(const CancelState & cs)
    {
        turn--;
        A[x][y] = cs.a;
        score -= A[x][y];
        y = cs.y;
        x = cs.x;
        hash = cs.hash;
    }
    Eval eval()
    {
        return score;
    }
    bool done()
    {
        return turn == 90;
    }

    static void inithash()
    {
        for (int x = 0; x < N; ++x)
        {
            xhashes[x] = rand() & (HASH_SIZE-1);
            yhashes[x] = rand() & (HASH_SIZE-1);
            for (int c = 0; c < N; ++c)
                xyhashes[x][c] = rand() & (HASH_SIZE-1);
        }
    }
};

void print(int (*A)[MAXN])
{
    for (int x = 0; x < N; ++x)
    {
        for (int y = 0; y < N; ++y)
            cout << (A[x][y] != 0 ? (char)(A[x][y]+'0') : '.');
        cout << endl;
    }
    cout << endl;
}

int main()
{
    N = 30;

    MOD = new int[MAXN+2]+1;
    for (int m = -1; m < MAXN+1; ++m)
        MOD[m] = (m+N)%N;

    State root;
    root.x = N/2;
    root.y = N/2;
    root.score = 0;
    root.turn = 0;
    for (int x = 0; x < N; ++x)
        for (int y = 0; y < N; ++y)
            root.A[x][y] = nextInt(10);
    root.A[root.x][root.y] = 0;
    // typedef GreedySearch<State, true, true> S;
    // typedef RandomGreedySearch<State, true, 2, 2> S;
    // typedef TwoStepsSearch<State, true, true> S;
    // typedef NStepsSearch<State, true, 4, true> S;
    typedef BeamSearch<State, true, 100, 300000, 22000, true> S;

    S search;
    // RepeatSearch<S, 10500> search;
    vector<Move> output;
    Eval eval = search.search(root, output);
    // cout << benchmark << endl;
    State::CancelState cs;
    for (auto m : output)
    {
        // print(root.A);
        assert(!root.done());
        root.apply(m, cs);
    }
    // print(root.A);
    cout << eval << " " << root.score << endl;
    assert(eval == root.score);
    assert(root.done());
}
