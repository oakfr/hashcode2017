import numpy as np
import pickle
import os

R = 70
C = 300
A = 8
L = 1050
V = 7
B = 53
T = 400
start = (62,269)

def column_dist (c1,c2):
    return min([abs(c2-c1),C-abs(c2-c1)])

def is_covered(a,b):
    return ((b[0]-a[0])**2 + column_dist(a[1],b[1])**2) <= V

def read_data (in_file):

    with open(in_file,'r') as fp:
        fp.readline()
        fp.readline()
        fp.readline()
        targets = []
        for k in range(L):
            targets.append([int(x) for x in fp.readline().split(' ')])
        winds = np.zeros((R,C,A,2))
        for a in range(A):
            for r in range(R):
                d = [int(x) for x in fp.readline().split(' ')]
                for c in range(C):
                    winds[r,c,a,:] = d[2*c:2*c+1]
    return(winds,targets)

if __name__ == "__main__":
    (winds,targets) = read_data('loon_r70_c300_a8_radius7_saturation_250.in')

    print('computing moves')
    next_cell = np.zeros((R,C,A,2))
    for r in range(R):
        for c in range(C):
            for a in range(A):
                next_cell[r,c,a,:] = [r,c]+winds[r,c,a,:]
                next_cell[r,c,a,1] = next_cell[r,c,a,1] % C

    print('computing coverage')
    if os.path.isfile('coverage.bin'):
        covered_cities = pickle.load(open('coverage.bin','rb'))
    else:
        covered_cities = np.zeros((R,C,2,L))
        for r in range(R):
            for c in range(C):
                for l in range(L):
                    covered_cities[r,c,1,l] = is_covered([r,c],targets[l])
        pickle.dump(covered_cities,open('coverage.bin','wb'))

    print('compute score map')
    scoremap = np.zeros((R,C,A+1,T))-1
    covered = np.zeros((L,T))
    covered[:,0] = covered_cities[start[0],start[1],0,:]
    scoremap[start[0],start[1],0,0] = 0
    for t in range(1,T):
        ind = np.where(scoremap[:,:,:,t-1]>=0)
        pos = np.array(list(ind)).transpose()
        print('t=%d'%t)
        #print(pos)
        for i in range(pos.shape[0]):
            r = pos[i,0]
            c = pos[i,1]
            a = pos[i,2]
            for k in range(-1,2):
                if (a+k>=1) and (a+k)<= A:
                    n = next_cell[r,c,a+k-1]
                    if (n[0]>=R) or (n[0])<0:
                        continue
                    scoremap[n[0],n[1],a+k,t] = max([scoremap[n[0],n[1],a+k,t],\
                                                    scoremap[r,c,a,t-1] + np.sum(covered_cities[n[0],n[1],1,:])])
        #if t > 3:
        #    break
    print(np.amax(scoremap.flatten()))