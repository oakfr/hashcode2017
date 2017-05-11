import numpy as np
import os
import sys
import random
from scipy.optimize import linprog
import multiprocessing

def show (M):
    for r in range(M.shape[0]):
        str = ''
        for c in range(M.shape[1]):
            if M[r,c]:
                str += '#'
            else:
                str += '.'
        print(str)

def read_data (in_file):
    with open(in_file,'r') as fp:
        d = [int(x) for x in fp.readline().split(' ')]
        R = d[0]
        C = d[1]
        M = np.zeros((R,C))
        for (r,l) in zip(np.arange(R),fp.readlines()):
            M[r,:] = [int(x) for x in l.strip().replace('.','0').replace('#','1')]
    return (M,R,C)

def rle (ia):
    n=ia.size
    y = np.array(ia[1:] != ia[:-1])
    i = np.append(np.where(y),[n-1])
    z = np.diff(np.append([-1], i))
    p = np.cumsum(np.append([0],z))[:-1]
    return(z,p,ia[i])

def greedy_solution(M,R,C):

    S = np.copy(M)
    commands = []
    changed = True
    while np.sum(S)>0 and changed:
        changed = False

        # search for columns
        for c in range(C):
            (z,p,ia) = rle(S[:,c])
            z = z[ia==1]
            p = p[ia==1]
            if z.size==0 or np.amax(z)<3:
                continue
            r1 = p[np.argmax(z)]
            r2 = r1+np.amax(z)-1
            c1 = c
            c2 = c
            cmd = (r1,c1,r2,c2,1)
            S[r1:r2+1,c1:c2+1]=0
            commands.append(cmd)
            changed = True
            break
        # search for rows
        for r in range(R):
            (z,p,ia) = rle(S[r,:])
            z = z[ia==1]
            p = p[ia==1]
            if z.size==0 or np.amax(z)<3:
                continue
            c1 = p[np.argmax(z)]
            c2 = c1+np.amax(z)-1
            r1 = r
            r2 = r
            cmd = (r1,c1,r2,c2,1)
            S[r1:r2+1,c1:c2+1]=0
            commands.append(cmd)
            changed = True
            break
        # finish with single cells
        ind = np.array(list(np.where(S>0))).transpose()
        for i in range(ind.shape[0]):
            cmd = (ind[i][0],ind[i][1],ind[i][0],ind[i][1],1)
            commands.append(cmd)
            S[ind[i][0],ind[i][1]]=0
    return commands

def paint_surface(R,C,commands):
    S = np.zeros((R,C))
    for cmd in commands:
        if cmd[4]>0:
            S[cmd[0]:cmd[2]+1,cmd[1]:cmd[3]+1]=1
        else:
            S[cmd[0]:cmd[2]+1,cmd[1]:cmd[3]+1]=0
    return S

def get_score(M,R,C,commands):
    assert(check_sol(M,R,C,commands))
    return R*C - len(commands)

def check_sol(M,R,C,commands):
    for cmd in commands:
        assert(is_in(cmd,[0,0,R-1,C-1]))
    S = paint_surface(R,C,commands)
    if not (S==M).all():
        print('*** WARNING *** %d,%d' % (np.sum(S-M),np.sum(M-S)))
    return (S==M).all()

def print_solution(commands,out_file):
    with open(out_file,'w') as fp:
        fp.write('%d\n' % len(commands))
        for cmd in commands:
            if cmd[4]==-1:
                assert((cmd[0]==cmd[2]) and (cmd[1]==cmd[3]))
                fp.write('ERASE_CELL %d %d' % (cmd[0],cmd[1]))
            else:
                if (cmd[0]==cmd[2]) or (cmd[1]==cmd[3]):
                    fp.write('PAINT_LINE %d %d %d %d\n' % (cmd[0],cmd[1],cmd[2],cmd[3]))
                else:
                    assert(cmd[2]-cmd[0]==cmd[3]-cmd[1])
                    s = (cmd[2]-cmd[0])/2
                    r0 = cmd[0]+s
                    c0 = cmd[1]+s
                    fp.write('PAINT_SQUARE %d %d %d\n' % (r0,c0,s))


def list_commands(r1,c1,r2,c2):#R,C):
    commands = []
    maxs = max([c2-c1+1,r2-r1+1])

    # paint_square
    for r in range(r1,r2+1):
        for c in range(c1,c2+1):
            for s in range(1,maxs+1):
                cmd = [r-s,c-s,r+s,c+s,1]
                if is_in (cmd, [r1,c1,r2,c2]):
                    commands.append(cmd)
    # paint line
    for r in range(r1,r2+1):
        for c1 in range(c1,c2+1):
            for c2 in range(c1,c2+1):
                commands.append((r,c1,r,c2,1))
    for c in range(c1,c2+1):
        for r1 in range(r1,r2+1):
            for r2 in range(r1,r2+1):
                commands.append((r1,c,r2,c,1))

    # erase commands
    for r in range(r1,r2+1):
        for c in range(c1,c2+1):
            commands.append((r,c,r,c,-1))

    return commands

def is_in (a,b):
    assert(a[0]<=a[2])
    assert(a[1]<=a[3])
    assert(b[0]<=b[2])
    assert(b[1]<=b[3])
    return (a[0]>=b[0]) and (a[2]<=b[2]) and (a[1]>=b[1]) and (a[3]<=b[3])

def cmd_surface(cmd):
    return (cmd[2]-cmd[0]+1)*(cmd[3]-cmd[1]+1)

def fix_cmds (M,R,C,cmds):
    S = paint_surface(R,C,cmds)
    new_cmds = cmds
    if not (S==M).all():

        # paint more
        Z = M>S
        for r in range(R):
            for c in range(C):
                if Z[r,c]:
                    new_cmds.append((r,c,r,c,1))
        
        S = paint_surface(R,C,new_cmds)

        #erase more
        Z = S>M
        for r in range(R):
            for c in range(C):
                if Z[r,c]:
                    new_cmds.append((r,c,r,c,-1))

    S = paint_surface(R,C,new_cmds)
    assert(S==M).all()
    return new_cmds

def iterate_sol (M,R,C,greedy_sol):

    depth = 8
    stepr = min([depth,int(R-1)])
    stepc = min([depth,int(C-1)])

    # pick a random rectangle
    r1 = random.randint(0,R-stepr-1)
    r2 = r1+stepr
    c1 = random.randint(0,C-stepc-1)
    c2 = c1+stepc
    print((r1,r2,c1,c2))
    #r1 = 2
    #r2 = 10
    #c1 = 2
    #c2 = 10
    nr = r2-r1+1
    nc = c2-c1+1

    # check surface
    W0 = paint_surface(R,C,greedy_sol)
    assert(W0==M).all()

    # remove all commands inside the target
    new_greedy_cmds = []
    removed_pixels=0
    for cmd in greedy_cmds:
        if not is_in(cmd,[r1,c1,r2,c2]):
            new_greedy_cmds.append(cmd)
        else:
            #print('removed command : %d,%d,%d,%d' % (cmd[0],cmd[1],cmd[2],cmd[3]))
            removed_pixels+=cmd_surface(cmd)

    #print('Command set from %d to %d (%d pixels removed)' % (len(greedy_cmds),len(new_greedy_cmds),removed_pixels))

    # list possible commands inside the target
    possible_cmds = list_commands (r1,c1,r2,c2)

    # build matching matrix
    A = np.zeros((nr*nc,len(possible_cmds)))
    B = np.zeros(nr*nc)

    print('building %d x %d matrix' % (A.shape[0],A.shape[1]))

    id = 0
    for r in range(r1,r2+1):
        for c in range(c1,c2+1):
            for j in range(A.shape[1]):
                A[id,j] = is_in ([r,c,r,c],possible_cmds[j]) * possible_cmds[j][4]
            B[id] = M[r,c]
            id += 1

    CX = np.zeros(A.shape[1])+1

    res = linprog(CX, None, None, A, B, bounds=(0,1))

    if not res.success:
        return greedy_sol

    if res.success:
        resx = np.round(res.x)
        G = A.dot(resx)-B
        check_G = np.amax(G)
        if check_G>.0000001:
            return greedy_sol
        added_pixels=0
        for (xv,idx) in zip(resx,range(len(resx))):
            if xv == 1:
                cmd = possible_cmds[idx]
                new_greedy_cmds.append(cmd)
                added_pixels += cmd_surface(cmd)
                #print('added command %d,%d,%d,%d' %(cmd[0],cmd[1],cmd[2],cmd[3]))
        #print('added pixels:%d'%added_pixels)
    
    W1 = paint_surface(R,C,new_greedy_cmds)

    # fix minor issues
    if not check_sol(M,R,C,new_greedy_cmds):
        new_greedy_cmds = fix_cmds (M,R,C,new_greedy_cmds)
        W1 = paint_surface(R,C,new_greedy_cmds)

    assert(check_sol(M,R,C,new_greedy_cmds))
    assert(W1==W0).all()
    assert(W1==M).all()

    return new_greedy_cmds

def full_iteration (M,R,C,cmds,best_score,in_file):
    # improve
    num_iter = 100
    for k in range(num_iter):

        # read best solution so far
        tmp_greedy_cmds = iterate_sol(M,R,C,cmds)

        assert(check_sol(M,R,C,tmp_greedy_cmds))

        score = get_score(M,R,C,tmp_greedy_cmds)

        print('tmp score = %d' % score)

        if score > best_score:
            best_score = score
            #greedy_cmds = tmp_greedy_cmds
            cmds = tmp_greedy_cmds

    print('\t\t\t\t\tnew best score = %d' % best_score)
    out_file = 'output/out_%s_%06d.txt' % (in_file,best_score)
    print_solution(tmp_greedy_cmds,out_file)

if __name__ == "__main__":

    in_files = ['logo.in','learn_and_teach.in','right_angle.in']
    #in_files = ['logo.in']
    for in_file in in_files:

        (M,R,C) = read_data(in_file)

        print('building greedy solution')
        greedy_cmds = greedy_solution(M,R,C)

        assert(check_sol(M,R,C,greedy_cmds))

        score = get_score(M,R,C,greedy_cmds)
        print(score)

        best_score = score

        jobs = []

        num_threads = 32
        for p in range(num_threads):

            process = multiprocessing.Process(target=full_iteration, args=[M,R,C,greedy_cmds,best_score,in_file])
            process.start()
            jobs.append(process)

        # wait for everyone
        for proc in jobs:
            proc.join()


        #out_file = 'output/out_%04d_%s' % (best_score,in_file)
        #print_solution(greedy_cmds,out_file)


    #commands = list_commands (R,C)

