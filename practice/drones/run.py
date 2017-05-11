import sys
import copy
import math
import pickle
import os

def distance(r1, c1, r2, c2):
    if (r1==r2) and (c1==c2):
        return 0
    return math.ceil (math.sqrt ((r1-r2)**2+(c1-c2)**2))


def compute_weight (pack, weights):
    return sum ([i*weights[p] for (p,i) in pack])

def pack_to_str (pack):
    return ' '.join(['%d:%d'%(p,i) for (p,i) in pack])


class Warehouse:
    def __init__ (self, id, r, c, items):
        self.id = id
        self.r = r
        self.c = c
        self.items = items

    def __str__ (self):
        return '[warehouse %d] %d,%d %s' % (self.id, self.r, self.c, '  '.join(['%d:%d'%(i,e) for i,e in enumerate(self.items_copy) if e!=0]))


class Order:
    def __init__ (self, id, r, c, items):
        self.id = id
        self.r = r
        self.c = c
        self.items = items
        self.done = False
        self.allocated_drone = -1
        self.best_order = []
        self.best_w_i = -1

    def __str__ (self):
        return '[order %d] %d,%d %s\t(%s)' % (self.id, self.r, self.c, '  '.join(['%d:%d'%(i,e) for i,e in enumerate(self.items) if e != 0]), '  '.join(['%d:%d'%(i,e) for i,e in enumerate(self.items_copy) if e != 0]))

    def affinity_warehouse (self, warehouse, debug=False):
        return 100.0*sum([min([a,b]) for a,b in zip(self.items_copy, warehouse.items_copy)])/distance(self.r,self.c,warehouse.r,warehouse.c)

class Drone:
    def __init__ (self, id, r, c, n_types):
        self.id = id
        self.r = r
        self.c = c
        self.busy = False
        self.time = 0
        self.items = [0]*n_types
        self.best_w_i = -1
        self.best_o_i = -1
        self.busy = False

class Game:
    def __init__ (self, maxr, maxc, duration, maxload, ntypes, weights, drones, orders, warehouses, filename):
        self.maxr = maxr
        self.maxc = maxc
        self.duration = duration
        self.maxload = maxload
        self.drones = drones
        self.orders = orders
        self.warehouses = warehouses
        self.commands = []
        self.n_types = ntypes
        self.weights = weights
        self.filename = filename
        self.points = 0

    def __str__ (self):
        s = 'grid %dx%d\n' % (self.maxr,self.maxc)
        s+= 'drones:\n'
        for d in self.drones:
            s+='\t[drone %d] %d,%d\n' %(d.id,d.r,d.c)
        s+='orders\n'
        for o in self.orders:
            s+='\t[order %d] %d %d %s\n' % (o.id, o.r, o.c, ' , '.join(['%d'%i for i,e in enumerate(o.items) if e != 0]))
        s+='warehouses\n'
        for w in self.warehouses:
            s+='\t[warehouse %d] %d %d %s\n' % (w.id, w.r, w.c, ' , '.join(['%d:%d'%(i,e) for i,e in enumerate(w.items) if e!=0]))
        return s


    def commands_to_file (self, filename):
        cmd_dict = ['L','U','D','W']
        with open (filename,'w') as fp:
            fp.write ('%d\n' % len(self.commands))
            for cmd in self.commands:
                if cmd[1]==0 or cmd[1]==1 or cmd[1]==2:
                    fp.write('%d %c %d %d %d\n' % (cmd[0], cmd_dict[cmd[1]], cmd[2], cmd[3], cmd[4]))
                elif cmd[1]==3:
                    fp.write('%d W %d\n' % (cmd[0],cmd[2]))


    def cap_load (self, pack):
        res = []
        exp_pack = []
        for p in pack:
            exp_pack += [p[0] for k in range(p[1])]
        cum_weight = 0
        for p in exp_pack:
            cum_weight += self.weights[p]
            if cum_weight <= self.maxload:
                res.append((p,1))
            else:
                break
        #print ('%s reduced to %s' % (pack_to_str (pack), pack_to_str (res)))
        return res
            

    def match_order_warehouse (self, o, w):
        d = [a*b for a,b in zip(o.items_copy,w.items_copy)]
        indx = [i for (i,e) in enumerate(d) if e>0]
        pack = [(i,min(o.items_copy[i],w.items_copy[i])) for i in indx]
        res = self.cap_load (pack)
        return res


    def allocate_orders_to_warehouses (self):
        for w in self.warehouses:
            w.items_copy = copy.copy (w.items)
        for o in self.orders:
            o.items_copy = copy.copy (o.items)
        # sort orders by decreasing value
        self.orders = sorted (self.orders, key=lambda o:sum(o.items))
        for i,o in enumerate(self.orders):
            o.id=i
        for o in self.orders:
            o.packs = []
            n_trials=0
            while sum(o.items_copy)>0:
                n_trials+=1
                affs = [o.affinity_warehouse(w) for w in self.warehouses]
                best_w_i = affs.index(max(affs))
                w = self.warehouses[best_w_i]
                pack = self.match_order_warehouse(o, w)
                assert (compute_weight (pack, self.weights) <= self.maxload)
                for (p,i) in pack:
                    w.items_copy[p]-=i
                    assert(w.items_copy[p]>=0)
                    o.items_copy[p]-=i
                    assert (o.items_copy[p]>=0)
                o.packs.append ((w.id,pack))
                assert (w.id == best_w_i)
            print ('order %d allocated to warehouse %d in %d parts' % (o.id, w.id, len(o.packs)))
        for w in self.warehouses:
            print ('warehouse %d has %d items left' % (w.id, sum(w.items_copy)))


    def allocate_drones_to_warehouses (self):
        # look for nearest warehouse
        for d in self.drones:
            if d.busy:
                d.best_w_i = -1
                continue
            dist = [distance(d.r, d.c, w.r, w.c) for w in self.warehouses]
            indx = dist.index(min(dist))
            d.best_w_i = indx
            assert (self.warehouses[d.best_w_i].id==d.best_w_i)
            print ('drone %d --> warehouse %d' % (d.id, d.best_w_i))

    def count_remaining_orders (self):
        return len (filter(lambda o:len(o.packs)>0,self.orders))

    def allocate_packs_to_drones (self):
        for o in self.orders:
            o.allocated_drone=-1
        for d in self.drones:
            if d.busy:
                d.allocated_order = -1
                continue
            done=False
            attempts=0
            while not done:
                # allocate first available pack to drone
                for o in self.orders:
                    if o.allocated_drone != -1:
                        continue
                    if len(o.packs)==0:
                        continue
                    pack = o.packs[0]
                    if pack[0]==d.best_w_i:
                        o.allocated_drone=d.id
                        d.allocated_order=o.id
                        done=True
                        print ('drone %d --> warehouse %d, order %d' % (d.id, pack[0], o.id))
                        break
                    if done:
                        break        
                if done:
                    break
                # if not found, allocate drone to next warehouse
                remaining=self.count_remaining_orders()
                if not done:
                    print ('failed to allocate drone %d to pack.  %d orders remaining. switching to next warehouse.' % (d.id, remaining))
                    d.best_w_i = (d.best_w_i+1)%len(self.warehouses)
                    attempts+=1
                    if attempts>len(self.warehouses):
                        print ('Warning : cycled through all warehouses.  nothing to do...')
                        d.busy=True
                        break


    def compute_commands (self):
        commands = []
        for d in self.drones:
            if d.busy:
                continue
            o = self.orders[d.allocated_order]
            w = self.warehouses[d.best_w_i]
            pack = o.packs[0]
            print ('commanding drone %d, order %d, warehouse %d' % (d.id, o.id, w.id))
            assert (pack[0]==w.id)
            for q in pack[1]:
                # load command
                commands.append([d.id, 0, w.id, q[0], q[1]])
            for q in pack[1]:
                # deliver command
                commands.append([d.id, 2, o.id, q[0], q[1]])
            # remove pack
            o.packs = o.packs[1:]
        return commands


    def solve (self):
        # static allocation of orders to warehouses
        filename = self.filename + '.bin'
        if os.path.isfile (filename):
            self = pickle.load (open (filename,'rb'))
        else:
            self.allocate_orders_to_warehouses ()
            pickle.dump (self, open(filename,'wb'))

        self.time = 0
            
        max_turns = 10000
        for turn in range(max_turns):
            print ('#################### TURN %d   TIME=%d #################' % (turn, self.time))
            # allocate drones to warehouses
            self.allocate_drones_to_warehouses ()
            # allocate packs to drones
            self.allocate_packs_to_drones ()
            # generate commands
            commands = self.compute_commands ()
            # execute commands
            for cmd in commands:
                self.points += self.execute_command (cmd)
            # add commands to stack
            self.commands += commands
            # update simulation time
            self.time = min([d.time for d in self.drones])
            for d in self.drones:
                if d.time > self.time:
                    d.busy=True
                else:
                    d.busy=False
            # quit
            remaining = self.count_remaining_orders ()
            if remaining==0:
                print ('All orders fullfilled. Stopping.  Time=%d, max duration=%d' % (self.time, self.duration))
                break
            if self.time > self.duration:
                print ('End of game reached (duration=%d)' % self.duration)
                break
 
        # save solution to file
        dirname = self.filename.split('.')[0]
        if not os.path.isdir (dirname):
            os.makedirs (dirname)
        filename = '%s/res_%d' % (dirname,self.points)
        self.commands_to_file (filename)
        print ('%d points.  saved %d commands to file %s' % (self.points, len(self.commands), filename))


    def load_solution (self, filename):
        self.commands = []
        with open (filename,'r') as fp:
            ncommands = int(fp.readline())
            for i,a in enumerate(fp.readlines()):
                s = a.strip().replace('L','0').replace('U','1').replace('D','2').replace('W','3').split(' ')
                s = [int(c) for c in s]
                self.commands.append(s)
            assert (ncommands == len(self.commands))


    def execute_command (self, command):
        points=0
        d = self.drones[command[0]]
        if command[1]==0 or command[1]==1:
            w = self.warehouses[command[2]]
            d.time += 1+distance(d.r, d.c, w.r, w.c)
            #print ('drone %d moved %d,%d --> %d,%d.  cost=%d.  time=%d' % (d.id, d.r, d.c, w.r, w.c, distance(d.r, d.c, w.r, w.c), d.time))
            d.r = w.r
            d.c = w.c
            if command[1]==0:
                w.items[command[3]]-=command[4]
                d.items[command[3]]+=command[4]
            else:
                w.items[command[3]]+=command[4]
                d.items[command[3]]-=command[4]
            assert(w.items[command[3]]>=0)
        elif command[1]==2:
            o = self.orders[command[2]]
            d.time += 1+distance(d.r, d.c, o.r, o.c)
            #print ('drone %d moved %d,%d --> %d,%d.  cost=%d.  time=%d' % (d.id, d.r, d.c, o.r, o.c, distance(d.r, d.c, o.r, o.c), d.time))
            d.r = o.r
            d.c = o.c
            assert(d.items[command[3]]>=command[4])
            o.items[command[3]]-=command[4]
            d.items[command[3]]-=command[4]
            # check item is fullfilled
            if sum(o.items)==0:
                #print ('order %d is fullfilled at time %d by drone %d!' % (o.id, d.time, d.id))
                points += math.ceil (100.0*(1.0*self.duration-1.0*d.time)/(1.0*self.duration))
        elif command[1]==3:
            d.time += command[2]
        assert (d.time<=self.duration)
        return points

    
    def save_state (self):
        self._drones = copy.deepcopy(self.drones)
        self._orders = copy.deepcopy (self.orders)
        self._warehouses = copy.deepcopy (self.warehouses)

    def restore_state (self):
        self.drones = self._drones
        self.orders = self._orders
        self.warehouses = self._warehouses

    def run (self):
        points=0
        self.save_state ()
        for command in self.commands:
            points += self.execute_command(command)
        self.restore_state()
        return points



def read_data (filename):
    with open (filename,'r') as fp:
        # main params
        C, R, DRONES, DURATION, MAXLOAD = [int(a) for a in fp.readline().strip().split(' ')]
        # weights
        NTYPES = int(fp.readline())
        WEIGHTS = [int(a) for a in fp.readline().strip().split(' ')]
        assert(len(WEIGHTS)==NTYPES)
        # warehouses
        warehouses = []
        NWARE = int(fp.readline())
        for k in range(NWARE):
            r,c = [int(a) for a in fp.readline().strip().split(' ')]
            items = [int(a) for a in fp.readline().strip().split(' ')]
            assert(len(items)==NTYPES)
            warehouses.append (Warehouse (k, r, c, items))
        # orders
        orders = []
        NORDERS = int(fp.readline())
        for k in range(NORDERS):
            r,c = [int(a) for a in fp.readline().strip().split(' ')]
            nitems = int(fp.readline())
            ptypes = [int(a) for a in fp.readline().strip().split(' ')]
            assert(len(ptypes)==nitems)
            items = [0]*NTYPES
            for t in ptypes:
                items[t]+=1
            orders.append (Order(k,r,c,items))
        # drones
        drones = []
        for k in range(DRONES):
            drones.append(Drone(k,warehouses[0].r,warehouses[0].c, NTYPES))
    game = Game(C, R, DURATION, MAXLOAD, NTYPES, WEIGHTS, drones, orders, warehouses, filename)
    return game


def main(filename, solution):
    game = read_data (filename)

 #   print(game)
    #game.allocate_orders_to_warehouses()
    if solution is not None:
        game.load_solution (solution)
        points = game.run()
        print ('%d points' % points)
    else:
        game.solve()


if __name__ == "__main__":
    filename = sys.argv[1]
    solution=None
    if len(sys.argv)>2:
        solution = sys.argv[2]
    main(filename, solution)

