import numpy as np
import random
import time
import itertools

def init_map (n_rows, n_cols, dead_slots):

    M = np.zeros((n_rows,n_cols)).astype(int)
    for dead_slot in dead_slots:
        M[dead_slot[0],dead_slot[1]]=1
    return M

def print_map (M):
    for i in range(M.shape[0]):
        str = '[%03d] ' % i
        for j in range(M.shape[1]):
            str += '%d' % M[i,j]
            if (j+1)%10==0:
                str += ' '
        print(str)

def update_full_map (M, servers):
    for s in servers:
        if s[2] != -1 and s[3] != -1:
            if not (M[s[2],s[3]:s[3]+s[0]]==0).all():
                print ('*** ERROR *** server at %d,%d overlaps' % (s[2],s[3]))
            assert(M[s[2],s[3]:s[3]+s[0]]==0).all()
            M[s[2],s[3]:s[3]+s[0]]=1
    return M

def avail_slot (M,row,size):
    for k in range(M.shape[1]-size+1):
        if (M[row,k:k+size]==0).all():
            return k
    return -1

def update_map (M,row,col,size):
    if not (M[row,col:col+size]==0).all():
        print('*** ERROR *** Space already taken at %d,%d' % (row,col))
    assert((M[row,col:col+size]==0).all())
    M[row,col:col+size]=1
    return M

def read_data(in_file):
    with open(in_file) as fp:
        lines = fp.readlines()
    d = lines[0].split(' ')
    n_rows = int(d[0])
    n_cols = int(d[1])
    n_unav = int(d[2])
    n_pools = int(d[3])
    n_servers = int(d[4])

    # unavailable slots
    dead_slots = []
    for i in range(1,n_unav):
        d = lines[i].split(' ')
        dead_slots.append((int(d[0]),int(d[1])))

    # servers (size, capacity, row, col, pool)
    servers = []
    for i in range(1+n_unav,len(lines)):
        d = lines[i].split(' ')
        servers.append([int(d[0]),int(d[1]),-1,-1,-1])

    return (n_rows,n_cols,n_pools,dead_slots,servers)

def read_solution (in_file, servers):
    with open(in_file,'r') as fp:
        lines = fp.readlines()

    for (i,line) in zip(range(len(lines)),lines):
        if line.find('x') is not -1:
            continue
        d = line.split(' ')
        servers[i][2] = int(d[0])
        servers[i][3] = int(d[1])
        servers[i][4] = int(d[2])

    return servers

def score_pool (servers, n_rows, i_pool):
    pool_cap_per_row = np.zeros(n_rows)
    for s in servers:
        capacity = s[1]
        row = s[2]
        pool = s[4]
        if pool == i_pool:
            pool_cap_per_row[row] += capacity
    score = int(np.sum(pool_cap_per_row) - np.amax(pool_cap_per_row))
    return score

def score_config (servers, n_pools, n_rows):
    return np.amin([score_pool(servers,n_rows,pool) for pool in range(n_pools)])

def alloc_servers (M, servers,n_pools):

    n_rows = M.shape[0]

    # sort servers by decreasing size
    servers = sorted(servers,key=lambda x:-(100.0*x[1]/x[0]-x[0]))
    print(servers)
    n_servers = len(servers)

    target_row = 0
    # alloc
    pool = 0
    for i in range(n_servers):
        server_size=servers[i][0]
        for k in range(n_rows):
            #target_row = (target_row+1)%n_rows#random.randrange(0,n_rows)
            target_row = (target_row+1)%n_rows
            col = avail_slot(M,target_row,server_size)
            #print('trying row %d, got %d' % (target_row,col))
            if col != -1:
                break
        if col != -1:
            #print('putting server %d at %d,%d' % (i,target_row,col))
            servers[i][2]=target_row
            servers[i][3]=col
            servers[i][4]=pool
            pool = (pool+1)%n_pools
            M=update_map(M,target_row,col,server_size)
    return servers

def swap_groups (servers,score):
    score_ref = score
    n_servers = len(servers)

    print(score_ref)
    for k in range(100000000):
        i =  random.randint(0,len(servers)-1)
        j =  random.randint(0,len(servers)-1)
#    for (i,j) in itertools.product(range(n_servers),range(n_servers)):
        if i == j:
            continue
        si = servers[i]
        sj = servers[j]
        if (si[4] == sj[4]) or (si[2]==-1) or (sj[2]==-1):
            continue
        gi = si[4]
        servers[i][4] = servers[j][4]
        servers[j][4] = gi

        score = score_config(servers, n_pools, n_rows)
        #print ('\t %3d <--> %3d  ==> %d' % (i,j,score))
        if score > score_ref:
            print (score)
            score_ref = score

def swap_places (servers,score):

    score_ref = score
    n_servers = len(servers)

    print(score_ref)
#    for k in range(10000000):
#        i =  random.randint(0,len(servers)-1)
#        j =  random.randint(0,len(servers)-1)
    for (i,j) in itertools.product(range(n_servers),range(n_servers)):
        if i == j:
            continue
        si = servers[i]
        sj = servers[j]
        if (si[0] != sj[0]) or (si[1] == sj[1]) or (si[2]==-1) or (sj[2]==-1):
            continue
        ri = si[2]
        ci = si[3]
        servers[i][2] = sj[2]
        servers[i][3] = sj[3]
        servers[j][2] = ri
        servers[j][3] = ci

        score = score_config(servers, n_pools, n_rows)
        #print ('\t %3d <--> %3d  ==> %d' % (i,j,score))
        if score > score_ref:
            print (score)
            score_ref = score

if __name__ == "__main__":

    # read data
    (n_rows,n_cols,n_pools,dead_slots,servers) = read_data('dc.in')

    # init map
    M = init_map (n_rows,n_cols,dead_slots)

    # allocation
    #servers = read_solution('res_0000000418', servers)
    servers = alloc_servers(M,servers,n_pools)

    # score
    score = score_config(servers, n_pools, n_rows)

    # swap places
    servers = swap_groups (servers,score)
    # update map
    #M = update_full_map (M, servers)

    #print_map (M)

