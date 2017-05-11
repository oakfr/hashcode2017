import scala.io.Source
import scala.collection.mutable.Map
import scala.math.abs
import scala.math.round
import collection.mutable.PriorityQueue
import scala.collection.mutable.TreeSet

class Endpoint (var uid: Int, var dc_latency: Int, var n_caches: Int) {
    var cache_latencies : Map[Int,Int] = Map()
    override def toString: String=
        "endpoint (uid=" + uid + ")" 

    def add_cache (cache_id: Int, cache_latency: Int) {
        cache_latencies(cache_id) = cache_latency
    }
}

class Request (var video_id : Int, var endpoint_id : Int, var n_requests: Int) {
    override def toString : String =
        n_requests + " of video " + video_id + " requested by endpoint " + endpoint_id
}

class CacheVideo (var cache_id : Int, var video_id : Int, var video_size : Int) {
    var sources : Map[Int,(Int,Int,Int)] = Map()
    var score:Long = 0
    var used = false
    var tested = false

    def add (endpoint_id: Int, n_requests: Int, cache_latency: Int, ref_latency: Int) {
        if (sources.contains(endpoint_id)) {
            sources(endpoint_id) = (n_requests + sources(endpoint_id)._1, cache_latency, ref_latency)
        } else {
            sources(endpoint_id) = (n_requests, cache_latency, ref_latency)
        }
    }
    
    def compute_score () {
        score = 0
        for ((_,v) <- sources) {
            score += v._1 * abs (v._2 - v._3)
        }
        score = round(score * 1.0 / (1.0 + video_size * 1.0 / 200))
//        score = sources.map(v => v._2._1 * abs(v._2._2 - v._2._3)).reduce(_+_)
    }


    def update (endpoint_id: Int, cache_latency: Int) {
        //println ("check " + sources(endpoint_id)._3 + "(cache_latency = " + cache_latency + ")")
        val new_val = (sources(endpoint_id)._1, sources(endpoint_id)._2, cache_latency.min(sources(endpoint_id)._3))
        sources(endpoint_id) = new_val
        //println ("check again " + sources(endpoint_id)._3)
        compute_score()
    }

    override def toString: String=
        "(" + cache_id + "," + video_id + ") " + sources.map(v=>" ["+v._1+":"+v._2._1+","+v._2._2+","+v._2._3+"] ").reduce(_+_) + " score = " + score
}

object CacheVideoOrdering extends Ordering[CacheVideo] {
      def compare(a:CacheVideo, b:CacheVideo) = a.score compare b.score
}

class Sim (var n_videos: Int, n_endpoints: Int, n_requests: Int, n_caches: Int, cache_size: Int) {
    var video_sizes : Map[Int, Int] = Map()
    var endpoints: Map[Int, Endpoint] = Map()
    var requests: Map[Int, Request] = Map()
    var cache_videos : Map[(Int,Int), CacheVideo] = Map()
    //var pq: PriorityQueue[CacheVideo] = PriorityQueue()(CacheVideoOrdering.reverse)
    //var pq: TreeSet[CacheVideo] = TreeSet()(CacheVideoOrdering.reverse)
    var pq: TreeSet[(Int,Int)] = TreeSet()(CVOrdering.reverse)
    var cache_loads: Map[Int,Int] = Map()
    var score:Long = 0

    override def toString: String=
        "sim : " + n_videos + " videos, " + n_endpoints + " endpoints, " + n_requests + " requests desc, " + n_caches + " caches, cache size = " + cache_size


    object CVOrdering extends Ordering[(Int,Int)] {
          def compare(a:(Int,Int), b:(Int,Int)) = cache_videos(a).score compare cache_videos(b).score
    }

    def compute_score () {
        score = 0
        var total_requests = 0
        for  ((_,r) <- requests) {
            val endpoint = endpoints(r.endpoint_id)
            val video_id = r.video_id
            val n_requests = r.n_requests
            var best_latency = endpoint.dc_latency
            
            for ((cache_id, cache_latency) <- endpoint.cache_latencies) {
                if (cache_videos((cache_id,video_id)).used) {
                    best_latency = best_latency.min (cache_latency)
                }
            }
            total_requests += n_requests
            score += n_requests * (endpoint.dc_latency - best_latency)
        }
        score = score * 1000 / total_requests
    }


    def list_cache_videos () {
        var count=0

        for ((_,r) <- requests) {
            val video_id = r.video_id
            val endpoint = endpoints(r.endpoint_id)
            val video_size = video_sizes(video_id)
            for ((cache_id,cache_latency) <- endpoint.cache_latencies) {
                val key = (cache_id, video_id)
                if (!(cache_videos.contains(key))) {
                    cache_videos(key) = new CacheVideo (cache_id, video_id, video_size)
                    count+=1
                }
                cache_videos(key).add (endpoint.uid, r.n_requests, cache_latency, endpoint.dc_latency)
            }
        }
        println(count + " cvs created")
        // compute initial scores
        for ((_,cv) <- cache_videos) {
            cv.compute_score ()
          //  println(cv)
        }
    }

    def init_queue () {
        for ((key,_) <- cache_videos) {
            pq += key
        }
    }
    
    def update_cache_video (cache_id: Int, video_id: Int) {
        for ((endpoint_id, nl) <- cache_videos((cache_id, video_id)).sources) {
            val endpoint = endpoints (endpoint_id)
            val cache_latency = nl._2
            for ((cache_id_n, _) <- endpoint.cache_latencies) {
                if (! (cache_videos((cache_id_n, video_id)).tested)) {
                    val key = (cache_id_n, video_id)
                    var cv = cache_videos(key)
                    pq -= key
                    //println("updating cache video for endpoint " + endpoint_id + " with cache latency " + cache_latency)
                    //println(cv)
                    cache_videos(key).update (endpoint_id, cache_latency)
                    //cv.update (endpoint_id, cache_latency)
                    //println(cv)
                    pq += key
                }
            }
        }
    }

    def solve () {
        for (cache_id <- 0 to n_caches-1) cache_loads(cache_id) = 0

        var n_used=0
        var n_unused = 0
        while (!(pq.isEmpty)) {
            //val cv = pq.dequeue()
            val key = pq.head
            pq -= key
            val cv = cache_videos(key)
            if (!(cv.tested)) {
                if (cache_loads(cv.cache_id) + video_sizes(cv.video_id) <= cache_size) {
                    cv.used = true
                    n_used += 1
                    cache_loads(cv.cache_id) += video_sizes(cv.video_id)
                    //println ("score = " + cv.score + " --> video " + cv.video_id + " goes to cache " + cv.cache_id)
                    update_cache_video (cv.cache_id, cv.video_id)
                } else {
                    n_unused += 1
                }
            }
            cv.tested = true
        }
        println (n_used + " cvs used, " + n_unused + " unused")
    }

}

object svideos {
    def read_data (filename: String): Sim = {
        print ("reading " + filename + "\n")
        val txtSource = io.Source.fromFile(filename)
        val lines = txtSource.getLines.toList
        val linesdd = lines.map(_.split(" ").map(_.toInt)).toArray

        var lineno = 0
        var data = linesdd(lineno)
        lineno+=1
        val n_videos = data(0)
        val n_endpoints = data(1)
        val n_requests = data(2)
        val n_caches = data(3)
        val cache_size = data(4)
        val sim = new Sim (n_videos, n_endpoints, n_requests, n_caches, cache_size)
        data = linesdd(lineno)
        lineno+=1
        for ((a,b) <- data.zipWithIndex) {
            sim.video_sizes(b) = a
        }

        println("reading " + n_endpoints + " endpoints...")
        for (a <- 0 to n_endpoints-1) {
            data = linesdd(lineno)
            lineno+=1
            var endpoint = new Endpoint (a, data(0), data(1))
            for (b <- 0 to endpoint.n_caches-1) {
                data = linesdd(lineno)
                lineno+=1
                endpoint.add_cache (data(0), data(1))
                }
            sim.endpoints(a) = endpoint
        }
        println ("reading " + n_requests + " requests...")
        for (a <- 0 to n_requests - 1) {
            data = linesdd(lineno)
            lineno+=1
            val video_id = data(0)
            val endpoint_id = data(1)
            val n_req = data(2)
            var request = new Request (video_id, endpoint_id, n_req)
            sim.requests(a) = request
        }
        txtSource.close()
        return sim
    }

    def main(args: Array[String]): Unit = {
        val endpoint = new Endpoint (0,0,0)
        print (endpoint)
        var sim = read_data (args(0))
        println (sim)
        println ("listing cvs...")
        sim.list_cache_videos ()
        println ("creating queue...")
        sim.init_queue()
        println ("solving...")
        sim.solve()
        println ("computing score...")
        sim.compute_score()
        println ("Score = " + sim.score)
    }
}


