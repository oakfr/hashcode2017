import sys
from collections import defaultdict
import heapq
import Queue
import multiprocessing
import random
import datetime
from scipy.optimize import linprog
import numpy as np
import os
import pickle
from pqdict import maxpq
import math


VERBOSE = 0

def log (msg, force=False):
    if VERBOSE or force:
        timestr = datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S")
        print ('%s -- %s' % (timestr, msg))


def read_data (filename):
    fp = open (filename,'r')
    d = [int(a) for a in fp.readline().strip().split(' ')]
    n_videos = d[0]
    n_endpoints = d[1]
    n_requests = d[2]
    n_caches = d[3]
    cache_size = d[4]


    video_sizes = [int(a) for a in fp.readline().strip().split(' ')]

    # endpoints
    endpoints = []
    for i in range(n_endpoints):
        d = [int(a) for a in fp.readline().strip().split(' ')]
        dc_latency = d[0]
        n_endpoint_caches = d[1]
        endpoint = Endpoint (i, dc_latency, n_endpoint_caches)
        for k in range(n_endpoint_caches):
            d = [int(a) for a in fp.readline().strip().split(' ')]
            cache_id = d[0]
            cache_latency = d[1]
            endpoint.cache_latencies[cache_id]=cache_latency
            endpoint.cache_ids.append (cache_id)
        endpoints.append (endpoint)

    # requests
    requests = []
    for i in range(n_requests):
        d = [int(a) for a in fp.readline().strip().split(' ')]
        video_id = d[0]
        endpoint_id = d[1]
        n_local_requests = d[2]
        request = Request (i, video_id, endpoint_id, n_local_requests)
        requests.append (request)
    log (requests[0])

    return Sim (n_videos, n_endpoints, n_requests, n_caches, cache_size, video_sizes, endpoints, requests)


class Endpoint:
    def __init__ (self, uid, dc_latency, n_caches):
        self.dc_latency = dc_latency
        self.n_caches = n_caches
        self.cache_latencies = {}
        self.cache_ids = []
        self.uid = uid
        self.video_ids = []

    def __str__ (self):
        return "endpoint %d : dc_latency = %d" % (self.uid, self.dc_latency)


class Request:
    def __init__ (self, uid, video_id, endpoint_id, n_requests):
        self.video_id = video_id
        self.endpoint_id = endpoint_id
        self.n_requests = n_requests
        self.uid = uid
    def __str__ (self):
        return "video %d from endpoint %d, %d requests" % (self.video_id, self.endpoint_id, self.n_requests)


class Cache_video:
    def __init__ (self, cache_id, video_id, video_size):
        self.cache_id = cache_id
        self.video_id = video_id
        self.video_size = video_size
        self.sources = {}
        self.score = 0
        self.enabled = False

    def add (self, endpoint_id, n_requests, cache_latency, ref_latency):
        if not endpoint_id in self.sources:
            self.sources[endpoint_id] = [n_requests, cache_latency, ref_latency]
        else:
            self.sources[endpoint_id][0] += n_requests

    def compute_score (self):
        alpha = .8 / (1+self.video_size * 1.0 / 500)
        self.score = alpha*sum ([b[0]*abs(b[1]-b[2]) for _,b in self.sources.iteritems()])

    def update (self, endpoint_id, latency):
        if not endpoint_id in self.sources:
            log ('*** WARNING *** endpoint %d not in sources of cache_id = %d, video_id = %d' % (endpoint_id, self.cache_id, self.video_id))
        self.sources[endpoint_id][2] = min ([latency, self.sources[endpoint_id][2]])

    def __str__ (self):
        out = '(%d,%d) ' % (self.cache_id, self.video_id)
        out += ' '.join (['[%d:%d,%d,%d] ' % (a,b[0],b[1],b[2]) for a,b in self.sources.iteritems()])
        out += ' score = %d' % self.score
        return out


class Sim:
    def __init__ (self, n_videos, n_endpoints, n_requests, n_caches, cache_size, video_sizes, endpoints, requests):
        self.n_videos = n_videos
        self.n_endpoints = n_endpoints
        self.n_requests = n_requests
        self.n_caches = n_caches
        self.cache_size = cache_size
        self.video_sizes = np.array(video_sizes)
        self.endpoints = endpoints
        self.requests = requests
       
        self.cache_videos = [[None for a in range(self.n_videos)] for b in range(self.n_caches)]

        self.allocs = np.zeros((self.n_caches, self.n_videos))

    def __str__ (self):
        return "%d videos, %d endpoints, %d requests, %d caches, %d cache size" % (self.n_videos, self.n_endpoints, self.n_requests, self.n_caches, self.cache_size)


    def stats (self):
        max_requests = max ([x.n_requests for x in self.requests])
        log ('%d max requests' % max_requests)
        #for endpoint in self.endpoints:
        #    log (endpoint)

    def write_sol (self, filename):
        fp = open (filename, 'w')
        fp.write ('%d\n' % self.n_caches)
        for k in range(self.n_caches):
            fp.write ('%d %s\n' % (k, ' '.join (['%d'%n for n in np.nonzero(self.allocs[k])[0]])))
        fp.close()

    def check (self):
        for endpoint in self.endpoints:
            s = ' , '.join(['%d' % k for k,_ in endpoint.cache_latencies.iteritems()])
            log ('endpoint %d is connected to caches %s' % (endpoint.uid, s))


    def list_cache_videos (self):
        #for r in self.requests:
        #    video_id = r.video_id
        #    endpoint = self.endpoints[r.endpoint_id]
        #    endpoint.video_ids.append (video_id)

        binfilename = 'data/cv-%s.bin' % self.dataset
        if os.path.isfile (binfilename):
            print ('\t reading from file %s' % binfilename)
            self.cache_videos = pickle.load (open (binfilename,'rb'))
            return
        count=0
        rcount=0
        nrequests = len(self.requests)
        for r in self.requests:
            video_id = r.video_id
            endpoint = self.endpoints[r.endpoint_id]
            for (cache_id,cache_latency) in endpoint.cache_latencies.iteritems():
                if self.cache_videos[cache_id][video_id] is None:
                    video_size = self.video_sizes[video_id]
                    self.cache_videos[cache_id][video_id] = Cache_video (cache_id, video_id, video_size)
                    count+=1
                self.cache_videos[cache_id][video_id].add (endpoint.uid, r.n_requests, cache_latency, endpoint.dc_latency)
                log ('added endpoint %d for cache_id = %d, video_id = %d' % (endpoint.uid, cache_id, video_id))
        log ('%d cvs created' % count, force=True)
        for i in range(self.n_caches):
            for j in range(self.n_videos):
                if self.cache_videos[i][j] is not None:
                    self.cache_videos[i][j].compute_score()
                    print(self.cache_videos[i][j])
        pickle.dump (self.cache_videos, open (binfilename,'wb'))

    def compute_score (self):
        score = 0
        total_requests = 0
        for r in self.requests:
            endpoint = self.endpoints[r.endpoint_id]
            best_latency = endpoint.dc_latency
            for cache_id in range(self.n_caches):
                if self.allocs[cache_id][r.video_id] and cache_id in endpoint.cache_latencies:
                    best_latency = min([best_latency, endpoint.cache_latencies[cache_id]])
            score += r.n_requests * (endpoint.dc_latency - best_latency)
            total_requests += r.n_requests
        return score * 1000 / total_requests


    def create_pq (self):
        self.pq = maxpq()
        for i in range(self.n_caches):
            for j in range(self.n_videos):
                if self.cache_videos[i][j] is not None:
                    pq_key = (i,j)
                    pq_val = self.cache_videos[i][j].score
                    self.pq[pq_key] = pq_val

    
    def log_pq (self):
        top_item = self.pq.top()
        top_value = self.pq[top_item]
        log ('*** HEAP *** Top item : (%d,%d) score=%d' % (top_item[0], top_item[1], top_value))


    def update_cache_video_2 (self, cache_id, video_id):
        for endpoint_id_n,nl in self.cache_videos[cache_id][video_id].sources.iteritems():
            endpoint = self.endpoints[endpoint_id_n]
            cache_latency = endpoint.cache_latencies[cache_id]
            for cache_id_n,_ in endpoint.cache_latencies.iteritems():
                if self.cache_videos[cache_id_n][video_id].enabled:
                    continue
                log ('updating cv (%d,%d) endpoint=%d, cache latency = %d' % (cache_id_n, video_id, endpoint_id_n, cache_latency))
                log (self.cache_videos[cache_id_n][video_id])
                self.cache_videos[cache_id_n][video_id].update (endpoint_id_n, cache_latency)
                self.cache_videos[cache_id_n][video_id].compute_score()
                log (self.cache_videos[cache_id_n][video_id])
                pq_key = (cache_id_n, video_id)
                pq_val = self.cache_videos[cache_id_n][video_id].score
                self.pq[pq_key] = pq_val
                self.log_pq()

#                log ('updating cv (%d,%d) with latency %d at endpoint %d' % (cache_id_n, video_id, cache_latency, endpoint_id_n))


    def init_sol (self):
        unassigned=0
        assigned=0
        self.cache_loads = [0 for k in range(self.n_caches)]
        count=0
        ncount = len(self.pq)
        while True:
            if count % 1000 == 0:
                print ('============= %d / %d ===============================================' % (count, ncount))
            count+=1
            try:
                pq = self.pq.pop()
            except KeyError:
                break
            cache_id = pq[0]
            video_id = pq[1]
            video_size = self.video_sizes[video_id]
            if self.cache_loads[cache_id] + video_size <= self.cache_size:
                log ('score = %d  -> video %d goes to cache %d' % (self.cache_videos[cache_id][video_id].score, video_id, cache_id), force=True)
                self.allocs[cache_id, video_id]=1
                self.cache_loads[cache_id] += video_size
                self.cache_videos[cache_id][video_id].enabled = True
                # update neighbor cvs
                self.update_cache_video_2 (cache_id, video_id)
                assigned+=1
            else:
                self.cache_videos[cache_id][video_id].enabled = True
                unassigned+=1
        log ('%d assigned, %d unassigned' % (assigned, unassigned), force=True)


def main(filename):
    log ('=============== %s ===============' % filename)
    sim = read_data (filename)

    dataset = filename.split('.')[0]
    sim.dataset = dataset
    log (sim)
    #sim.check()
    print ('listing cache videos...')
    sim.list_cache_videos ()
    print ('done.')

    sim.create_pq()

    sim.init_sol ()

    if dataset == 'zoo' or dataset == 'worth':
        print ('Score = %d' % sim.compute_score())

    sol_filename = 'sol-%s.txt' % dataset
    sim.write_sol (sol_filename)


if __name__ == "__main__":
    filename = sys.argv[1]
    main(filename)


