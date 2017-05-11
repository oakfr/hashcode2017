import sys
from collections import defaultdict
import heapq
import Queue
import multiprocessing
import random
import datetime


def log (msg):
    timestr = datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S")
    print ('%s -- %s' % (timestr, msg))


class PQEntry:
    def __init__(self, priority, value):
        self.priority = priority
        self.value = value

    def __cmp__(self, other):
        return cmp(self.priority, other.priority)


class Graph:
    def __init__ (self):
        self.nodes = set()
        self.edges = defaultdict(list)
        self.distances = {}
        self.costs = {}

    def add_node (self, value):
        self.nodes.add (value)

    def add_edge (self, from_node, to_node, bidirect, cost, distance):
        self.edges[from_node].append (to_node)
        if bidirect:
            self.edges[to_node].append (from_node)
        self.distances[(from_node, to_node)] = distance
        self.costs[(from_node, to_node)] = cost
        self.costs[(to_node, from_node)] = cost


class Map:

    def __init__ (self):
        pass

    def read_from_file (self, filename):
        with open (filename,'r') as fp:
            d = [int(a) for a in fp.readline().split(' ')]
            self.n_junctions = d[0]
            self.n_streets = d[1]
            self.n_seconds = d[2]
            self.n_cars = d[3]
            self.start_node = d[4]
            self.graph = Graph()
            for k in range(self.n_junctions):
                d = [float(a) for a in fp.readline().split(' ')]
                self.graph.add_node (k)
            for k in range(self.n_streets):
                d = [int(a) for a in fp.readline().split(' ')]
                self.graph.add_edge (d[0], d[1], d[2]==2, d[3], d[4])
            

    def __str__ (self):
        res = ""
        res += " %s nodes" % str(self.n_junctions)
        res += " %s edges" % str(self.n_streets)
        res += " %s seconds" % str(self.n_seconds)
        res += " %s cars" % str(self.n_cars)
        res += " %s start node" % str(self.start_node)
        res += " %d nodes\n" % (len(self.graph.nodes))
        for k in range(len(self.graph.nodes)):
            res += "[%d] %s\n" % (k,",".join(["%d"%n for n in self.graph.edges[k]]))
        return res


    def dijkstra (self, source):
        dist = [sys.maxint for k in self.graph.nodes]
        prev = [None for k in self.graph.nodes]
        dist[source] = 0
        Q = Queue.PriorityQueue()

        for k in self.graph.nodes:
            Q.put (PQEntry (dist[k],k))

        while not Q.empty():
            u = Q.get()
            u = u.value
            
            for v in self.graph.edges[u]:
                alt = dist[u] + self.graph.costs[(u,v)]
                if alt < dist[v]:
                    dist[v] = alt
                    prev[v] = u
                    Q.put(PQEntry(alt,v))

        log ('done running dijstra on %s' % source)

        with open ('data/%d.txt'%source,'w') as gp:
            for dest in self.graph.nodes:
                if dest != source:
                    path = [dest]
                    while path[0] != source:
                        path = [prev[path[0]]] + path
                    gp.write (','.join(['%d'%a for a in path]))
                    gp.write('\n')

        return dist, prev
            
    def random_path_forward (self, src, length):
        path = [src]
        current = src
        for k in range(length):
            targets = self.graph.edges[current]
            current = targets[random.randint(0,len(targets)-1)]
            path.append (current)
        return path

        
def main(filename):
    mmap = Map()
    mmap.read_from_file (filename)

    jobs = []
    for src in mmap.graph.nodes:
        p = multiprocessing.Process (target=mmap.dijkstra, args=[src])
        p.start()
        jobs.append (p)

    for job in jobs:
        job.join()

    return
    

if __name__ == "__main__":
    filename = sys.argv[1]
    main(filename)


