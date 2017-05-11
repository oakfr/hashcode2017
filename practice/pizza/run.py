import os
import numpy as np
import itertools
import pickle
import random
import math
import scipy.optimize

R = 180
C = 60
H = 3
A = 12

def get_hams (M,a):
    return np.sum(M[a[0][0]:a[0][1],a[1][0]:a[1][1]])

def get_area (a):
    assert(a[0][1]>=a[0][0])
    assert(a[1][1]>=a[1][0])
    return (a[0][1]-a[0][0]+1)*(a[1][1]-a[1][0]+1)

def cut_is_valid (P,a):
    if np.sum(P[a[0][0]:a[0][1]+1,a[1][0]:a[1][1]+1]) > 0:
        return False
    else:
        return True

def update_map (P,a,v=1):
    P[a[0][0]:a[0][1]+1,a[1][0]:a[1][1]+1]=v
    return P

def comp_score (M,cuts):
    score = 0
    for c in cuts:
        v = get_area(c)
        h = get_hams(M,c)
        assert(v<=A)
        assert(v>=H)
        score += v
    return score

def get_fill_rate (cuts):
    P = np.zeros((R,C))
    for a in cuts:
        P = update_map(P,a)

    return np.sum(P) / (R*C)

def draw_cuts (cuts):
    P = np.zeros((R,C))
    for a in cuts:
        P = update_map(P,a)

    for r in range(R):
        str = ''
        for c in range(C):
            str += '%d' % P[r,c]
        print(str)

def grid_cuts(M):
    cuts = []
    rl = np.arange(0,R,3)
    rr = rl+2
    rz = list(zip(rl,rr))
    cl = np.arange(0,C,4)
    cr = cl+3
    cz = list(zip(cl,cr))
    xz = itertools.product(rz,cz)
    for c in xz:
        if get_hams(M,c) >= H:
            cuts.append(c)
    return cuts

def list_fmts (M):
    fmts = []
    for c in range(1,12):
        for r in range(1,12):
            if c*r >= 3 and c*r <= A:
                fmts.append((r,c))
    fmts = list(set(fmts))
    for fmt in fmts:
        assert(fmt[0]*fmt[1]<=A)
    return fmts

def is_in (a,b):
    return (a[0][0]>=b[0][0]) and (a[0][0]<=b[0][1]) and \
        (a[0][1]>=b[0][0]) and (a[0][1]<=b[0][1]) and \
        (a[1][0]>=b[1][0]) and (a[1][0]<=b[1][1]) and \
        (a[1][1]>=b[1][0]) and (a[1][1]<=b[1][1])

def list_good_cuts (M,fmts):
    if os.path.isfile('gcuts.bin'):
        return pickle.load(open('gcuts.bin','rb'))

    cuts = []
    for fmt in fmts:
        for c in range(C):
            for r in range(R):
                #print('%d %d %d x %d' % (c,r,fmt[0],fmt[1]))
                a = ((r,r+fmt[0]-1),(c,c+fmt[1]-1))
                assert(get_area(a)<=A)
                if is_in (a,((0,R-1),(0,C-1))) and get_hams(M,a)>=3:
                    cuts.append(a)
    pickle.dump(cuts,open('gcuts.bin','wb'))
    return(cuts)

def sigmoid(x):
    return 1 / (1+math.exp(-x))

def check_cuts (cuts):
    for a in cuts:
        area_a = get_area(a)
        if area_a>A:
            print(a)
            print(area_a)
        assert(area_a<=A)

def init_solution (M, g_cuts):

    best_score = 0
    best_cuts = None
    best_idx = None
    for r in range(10):

        ng = len(g_cuts)
        is_kept = np.zeros(ng)
        P = np.zeros(M.shape)
        score = 0

        for k in range(100000):
            i = random.randint(0,ng-1)
            a = g_cuts[i]
            area_a = get_area(a)
            assert(area_a)<=A

            if is_kept[i] == 0:
                if cut_is_valid (P,a):
                    P = update_map(P,a)
                    score += area_a
                    assert(score == np.sum(P))
                    is_kept[i] = 1
                    #print(score)

            elif is_kept[i] == 1:
                # remove at random
                thresh = .9#15*(1-sigmoid(math.log(math.log(k)))*(12/area_a))
                #print(thresh)
                if random.random() > thresh:
                    is_kept[i] = 0
                    score -= area_a
                    P = update_map(P,a,0)
                    assert(score == np.sum(P))
            #print(score)

        if score>best_score:
            best_score=score
            best_idx = np.nonzero(is_kept==1)[0].astype(int)
            best_cuts = []
            for j in range(ng):
                if is_kept[j]==1:
                    best_cuts.append(g_cuts[j])
        print(best_score)
    print('Initial score : %d' % best_score)
    print('Check : %d/%d' % (comp_score(M,best_cuts),np.sum(P)))
    return (best_cuts,best_idx)

def read_data (in_file):
    with open(in_file,'r') as fp:
        d = fp.readline().split(' ')
        vs = []
        for x in fp.readlines():
            v = x.strip().replace('H','1').replace('T','0')
            v = list(map(int,v))
            vs.append(v)
        M = np.vstack(tuple(vs))
        print(M)
    return M

def optimize_cuts (M, good_cuts, init_cuts, init_cuts_idx):

    # init map
    P = np.zeros(M.shape)
    for i in init_cuts_idx:
        a = good_cuts[i]
        assert(cut_is_valid(P,a))
        P = update_map(P,a)

    assert(len(init_cuts)==len(init_cuts_idx))
    score = np.sum(P)
    print('Initial score : %d (%d cuts)' % (score,len(init_cuts_idx)))
    print('Check : %d' % comp_score(M,init_cuts))
    cuts_idx = init_cuts_idx.copy()

    # fix rectangles
    for k in range(10):

        # select a rectangle at random
        r1 = random.randint(0,R-30)
        c1 = random.randint(0,C-30)
        z = ((r1,r1+30),(c1,c1+30))

        # remove all cuts inside Z
        tmp_cuts_idx = []
        for i in cuts_idx:
            a = good_cuts[i]
            if not is_in(a,z):
                tmp_cuts_idx.append(i)
            else:
                P = update_map(P,a,0)
        cuts_idx = tmp_cuts_idx
        #print('%d cuts left after removal. New score : %d' % (len(cuts_idx),np.sum(P)))

        # list candidate cuts
        candidate_cuts = []
        candidate_idx = []
        for (ca_c,a) in zip(range(len(good_cuts)),good_cuts):
            if is_in(a,z) and cut_is_valid(P,a):
                candidate_cuts.append(a)
                candidate_idx.append(ca_c)
        n_candidates = len(candidate_cuts)

        W = np.zeros((get_area(z),n_candidates))
        W_row=0
        for r in range(z[0][0],z[0][1]+1):
            for c in range(z[1][0],z[1][1]+1):
                for (ca_c,ca) in zip(range(n_candidates),candidate_cuts):
                    if is_in(((r,r),(c,c)),ca):
                        W[W_row,ca_c] = 1
                W_row += 1
        Q = np.zeros(W.shape[0])+1
        Z = np.array([get_area(a) for a in candidate_cuts])

        # solve min(-Z'x) with W.x <= Q
        #print('Solving for %d candidates...' % n_candidates)
        res = scipy.optimize.linprog(-Z,W,Q,None,None,(0,1),'simplex',None,None)
        if not res.success:
            continue
        resx = np.round(res.x)
        assert((W.dot(resx)<=Q).all())
        for (x_c,s) in zip(range(resx.size),resx):
            if s == 1:
                a = good_cuts[candidate_idx[x_c]]
                assert(cut_is_valid(P,a))
                P = update_map(P,a)
                cuts_idx.append(candidate_idx[x_c])
        score = np.sum(P)
        cuts = []
        for i in cuts_idx:
            cuts.append(good_cuts[i])
        print('%d(%d),%s' % (int(score),comp_score(M,cuts),get_fill_rate(cuts)))
    return cuts

def main():

    random.seed(123456)

    M = read_data ('test_round.in')

    cuts = grid_cuts(M)
    grid_score = comp_score(M,cuts)
    print(grid_score)

    # list good cuts
    good_cuts = list_good_cuts(M,list_fmts(M))
    print('%d possible cuts' % len(good_cuts))

    # check
    check_cuts(good_cuts)

    # find init solution
    (init_cuts,init_cuts_idx) = init_solution (M, good_cuts)

    # optimize cuts
    opt_cuts = optimize_cuts (M,good_cuts,init_cuts,init_cuts_idx)

    # draw cuts
    draw_cuts (opt_cuts)

if __name__ == "__main__":
    main()

