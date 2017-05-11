import sys
import multiprocessing

def read_data (filename):
    pass

def next_lat_lon_vel (lat, lon, vel):
    l90 = 324000
    l180 = 648000
    out_lat = 0
    out_lon = 0
    out_vel = 0
    lat_next = lat + vel
    if ((-l90 <= lat_next) and (lat_next <= l90)):
        out_lat = lat_next
        out_lon = lon - 15
        out_vel = vel
    elif (l90 < lat_next):
        out_lat = l180 - lat_next
        out_lon = -l180 + (lon - 15)
        out_vel = -vel
    else:
        out_lat = -l180 - lat_next
        out_lon = -l180 + (lon - 15)
        out_vel = -vel
    if out_lon > 647999:
        out_lon -= 648000*2
    if out_lon < -648000:
        out_lon += 648000*2
    return (out_lat, out_lon, out_vel)


def lat_lon_instruction (lat_sat, lon_sat, lat_target, lon_target):
    return (lat_target-lat_sat, lon_target-lon_sat)


def compute_sat (sat, colls, duration, dataset):
    timestamp_step = 0
    filename = 'data/%s/sat-%d.txt' % (dataset, sat.uid)
    gp = open (filename, 'w')
    for k in range(0, duration, 10):
        if (k % 1000) == 0:
            if k > timestamp_step:
                timestamp_step = k
                print('[%d] %d/%d' % (sat.uid, k, duration))
        sat.move()
        for coll in colls:
            for (loc,loc_k) in zip(coll.locs, range(len(coll.locs))):
                for (timebox, timebox_k) in zip(coll.times, range(len(coll.times))):
                    tmin = timebox[0]
                    tmax = timebox[1]
                    if tmin <= k and k <= tmax:
                        if sat.can_see (loc[0], loc[1]):
                            gp.write ('%d %d %d %d %d\n' % (sat.uid, coll.uid, k, loc_k, timebox_k))

    gp.close()

class Game:
    def __init__ (self, sats, colls, duration, dataset):
        self.sats = sats
        self.colls = colls
        self.duration = duration
        self.dataset = dataset

    def par_run (self):
        jobs = []
        for s in self.sats:
            process = multiprocessing.Process (target = compute_sat, args=[s, self.colls, self.duration, self.dataset])
            process.start()
            jobs.append (process)

        for proc in jobs:
            proc.join()

    def read_timeline (self):
        nsats = len(self.sats)
        timeline = [{} for k in range(nsats)]
        for sat_uid in range(nsats):
            filename = 'data/%s/sat-%d.txt' % (self.dataset,sat_uid)
            print ('reading %s' % filename)
            gp = open (filename, 'r')
            for line in gp.readlines():
                d = [int(a) for a in line.strip().split(' ')]
                coll_uid = d[1]
                step = d[2]
                loc_k = d[3]
                key = (coll_uid, loc_k)
                if not key in timeline[sat_uid]:
                    timeline[sat_uid][key] = (step, step)
                else:
                    timeline[sat_uid][key] = (min([step, timeline[sat_uid][key][0]]), max([step, timeline[sat_uid][key][1]]))
            gp.close()
            print ('sat %d: timeline length %d' % (sat_uid, len(timeline[sat_uid])))
        self.timeline = timeline


class Sat:
    def __init__ (self, uid, lat, lon, vel, maxw, maxd):
        print ('new sat : %d %d %d %d %d' % (lat, lon, vel, maxw, maxd))
        self.lat = lat
        self.lon = lon
        self.vel = vel
        self.maxw = maxw
        self.maxd = maxd
        self.uid = uid
    
    def move (self):
        (self.lat, self.lon, self.vel) = next_lat_lon_vel (self.lat, self.lon, self.vel)

    def __str__ (self):
        return "lat = %d, lon = %d" % (self.lat, self.lon)

    def can_see (self, delta_lat, delta_lon):
        return (abs(delta_lat-self.lat)<self.maxd) and (abs(delta_lon-self.lon)<self.maxd)


class Coll:
    def __init__ (self, uid, value, nloc, ntimes):
        self.value = value
        self.nlocs = nloc
        self.ntimes = ntimes
        self.locs = []
        self.times = []
        self.uid = uid

    def add_loc (self, lat, lon):
        self.locs.append((lat,lon))

    def add_time (self, tmin, tmax):
        self.times.append((tmin,tmax))


def read_data (filename):
    with open (filename, 'r') as fp:
        duration = int(fp.readline())
        nsats = int(fp.readline())
        sats = []
        for k in range(nsats):
            d = [int(a) for a in fp.readline().strip().split(' ')]
            sats.append (Sat(k, d[0], d[1], d[2], d[3], d[4]))
        ncoll = int(fp.readline())
        colls = []
        for c in range(ncoll):
            d = [int(a) for a in fp.readline().strip().split(' ')]
            coll = Coll(c, d[0],d[1],d[2])
            for k in range(coll.nlocs):
                d = [int(a) for a in fp.readline().strip().split(' ')]
                coll.add_loc (d[0],d[1])
            for k in range(coll.ntimes):
                d = [int(a) for a in fp.readline().strip().split(' ')]
                coll.add_time (d[0],d[1])
            colls.append(coll)
    return Game (sats, colls, duration, filename.split('.')[0])

def main():
    filename = sys.argv[1]
    print (filename)
    game = read_data (filename)
    
    #game.par_run()
    game.read_timeline()

if __name__ == "__main__":
    main()


