import sys
import numpy as np
import pickle
import os
import random
from scipy.optimize import linprog
import time

def list_cuts (H):
    res = []
    for i in range(1,H+1):
        for j in range(1,H+1):
            if i*j<=H:
                res.append ((i,j))
    return res

def list_slices (R, C, cuts):
    res = []
    for r in range(R):
        for c in range(C):
            for cut in cuts:
                s = (r, c, r+cut[0]-1, c+cut[1]-1)
                if s[2]<R and s[3]<C:
                    res.append(s)
    return res

def getP (P, s):
    return P[s[0]:s[2]+1,s[1]:s[3]+1]

def setP (P, s, val):
    P[s[0]:s[2]+1,s[1]:s[3]+1]=val

def tofile (P, filename):
    with open (filename,'w') as fp:
        for i in range(P.shape[0]):
            fp.write(''.join(['%d'%b for b in P[i,:]]))
            fp.write('\n')


def is_valid (P, s, L):
    return np.sum(getP(P,s)==1)>=L and np.sum(getP(P,s)==2)>=L

def filter_slices (slices, P, L):
    out = []
    for s in slices:
        if is_valid (P, s, L):
            out.append (s)
    return out

def read_data (filename):
    with open (filename, 'r') as gp:
        d = [int(a) for a in gp.readline().strip().split(' ')]
        R = d[0]
        C = d[1]
        L = d[2]
        H = d[3]
        P = np.zeros((R,C))
        for r in range(R):
            v = gp.readline().strip()
            v = v.replace('M','1').replace('T','2')
            v = [int(a) for a in v]
            P[r] = v
        assert (np.amin(P)>0)
    return (R, C, L, H, P)

def sort_slices (slices):
    return sorted (slices, key=lambda x:surface(x), reverse=True)


def first_pass (P, R, C, slices):
    out = []
    M = np.array (P, copy=True)
    count=0
    for s in slices:
        if np.amin(getP(M,s))>0:
            setP(M,s,0)
            out.append (s)
    #        print(s)
    #        print(M)
    #        tofile (M, 'out/M-%06d.txt'%count)
    #        count+=1
    return out


def check_sol (R, C, slices):
    M = np.zeros((R, C))+1
    for s in slices:
        if np.amin(M[s[0]:s[2]+1,s[1]:s[3]+1])==0:
            return False
        M[s[0]:s[2]+1,s[1]:s[3]+1]=0
    return True


def write_sol (slices, filename):
    with open (filename, 'w') as fp:
        fp.write ('%d\n' % len(slices))
        for s in slices:
            fp.write ('%d %d %d %d\n' % (s[0], s[1], s[2], s[3]))

def score_sol (slices):
    out = 0
    for s in slices:
        out += (s[2]-s[0]+1)*(s[3]-s[1]+1)
    return out


def surface (s):
    return (s[2]-s[0]+1)*(s[3]-s[1]+1)


def is_in (a, p):
    return (a[0] <= p[0]) and (p[0] <= a[2]) and (a[1] <= p[1]) and (p[1] <= a[3])


def overlap (a, b, R, C):
    Ma = np.zeros((R,C))
    Mb = np.zeros((R,C))
    Ma[a[0]:a[2]+1,a[1]:a[3]+1]=1
    Mb[b[0]:b[2]+1,b[1]:b[3]+1]=1
    return np.amax(Ma+Mb)>1.5


def is_inside (a, b):
    return is_in (a, (b[0], b[1])) and is_in (a, (b[0],b[3])) and is_in (a, (b[2],b[1])) and is_in (a, (b[2],b[3]))


def get_map (R, C, slices):
    M = np.zeros((R,C))
    for s in slices:
        setP(M,s,1)
    return M

def is_free (M, s):
    return np.amax(getP(M, s))==0


def optimize_sol (R, C, sol, slices):
#    print ('current score = %d (%d slices)' % (score_sol (sol), len(sol)))
    old_score = score_sol (sol)

    assert (check_sol (R, C, sol))

    h = min([15,R,C])
    r = random.randint (0, R-h)
    c = random.randint (0, C-h)
    focus = (r, c, r+h-1, c+h-1)
#    print (focus)

    # remove slices and select candidates
    new_sol = []
    candidates = []
    M = get_map (R, C, sol)
    # focus is full, cannot do better
    if np.sum(M)==surface(focus):
        return (sol, False)
    removed = 0
    removed_list = []
    removed_surface = 0
    for s in sol:
        if is_inside (focus, s):
            setP(M, s, 0)
            removed += 1
            removed_list.append (s)
            removed_surface += surface(s)
        else:
            new_sol.append (s)
    for s in slices:
        if is_inside (focus, s):
            if is_free (M, s):
                candidates.append (s)
    for s in removed_list:
        assert (candidates.index(s)!=-1)

 #   print ('picked %d slices' % len(candidates))
 #   for s in removed_list:
 #       print ('removed %d,%d,%d,%d' % (s[0],s[1],s[2],s[3]))

    Z_ub = np.zeros((surface(focus), len (candidates)))

    count=0
    for r in range(focus[0],focus[2]+1):
        for c in range(focus[1],focus[3]+1):
            for (s,k) in zip(candidates, range(len(candidates))):
                if is_in (s, (r,c)):
                    Z_ub[count,k]=1
            count+=1

    b_ub = np.zeros((surface(focus)))+1

    c = np.zeros((len(candidates)))
    for (s,k) in zip (candidates, range(len(candidates))):
        c[k] = surface(s)
    
 #   print ('Optimizing shapes')
 #   print (Z_ub.shape)
 #   print (b_ub.shape)
 #   print (c.shape)

    res = linprog (-c, A_ub=Z_ub, b_ub=b_ub, bounds=(0,1))
 #   print ('Optim result: %s' % res.success)
    res.x = np.round(res.x)
    if not res.success:
        return (sol, False)

    added = 0
    added_surface = 0
    for (s,k) in zip (candidates, range(len(candidates))):
        if res.x[k]==1:
            new_sol.append (candidates[k])
            added += 1
            added_surface += surface (candidates[k])
    
  #  print ('removed %d slices, added %d' % (removed, added))
  #  print ('removed surface %d, added surface %d' % (removed_surface, added_surface))

  #  print ('new score = %d (%d slices)' % (score_sol (new_sol), len(new_sol)))
    new_score = score_sol (new_sol)
    assert (check_sol (R, C, new_sol))
    if new_score > old_score:
        return (new_sol, True)
    else:
        return (sol, False)


def main():

    #random.seed (123456)
    random.seed (time.time())

    filename = sys.argv[1]
    dataset = filename.split('.')[0]
    (R, C, L, H, P) = read_data (filename)
        
    binfilename = 'slices-%s.bin' % dataset
    if os.path.isfile (binfilename):
        slices = pickle.load(open(binfilename,'r'))
    else:
        cuts = list_cuts (H)
        slices = list_slices (R, C, cuts)
        print ('%d slices' % len(slices))
        slices = filter_slices (slices, P, L)
        print ('%d slices' % len(slices))
        slices = sort_slices (slices)
        pickle.dump (slices, open(binfilename, 'wb'))

    # sanity check candidates
    if False:
        for s in slices:
            assert (surface(s)<=H)
            assert (np.sum(getP(P,s)==1)>=L)
            assert (np.sum(getP(P,s)==2)>=L)
        print (len(slices))
        return

    sol = first_pass (P, R, C, slices)

    print ('Initial score : %d' % score_sol (sol))

    for k in range(10000):
        (sol, effective) = optimize_sol (R, C, sol, slices)
        if effective:
            print ('Score = %d' % score_sol (sol))
    return

    out_filename = 'sol-%s.bin' % dataset
    write_sol (sol, out_filename)
    score = score_sol (sol)
    print (score)

if __name__ == "__main__":
    main()

