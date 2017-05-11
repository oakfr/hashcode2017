using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Hashcode.Qualif
{
    public class Solution
    {
        private readonly Input _input;
        public long Score {get { return ComputeScore(); } }
        public StringBuilder Builder = new StringBuilder();

        public Cache[] Caches;

        public Solution(Input input)
        {
            _input = input;
            Caches = new Cache[input.C];
            for (var i = 0; i < Caches.Length; i++)
            {
                Caches[i] = new Cache(input, i);
            }
        }

        /// <summary>
        /// returns the solution ready to be written to the output file
        /// </summary>
        public override string ToString()
        {
            Builder.AppendLine(Caches.Length.ToString());
            for (int i = 0; i < Caches.Length; i++)
            {
                Builder.Append(i);
                Builder.Append(' ');
                Builder.AppendLine(String.Join(" ", Caches[i].VideosStored.Keys));
            }
            return Builder.ToString();
        }

        public long ComputeScore()
        {
            long score = 0;
            var totalreq = 0;
            foreach (var request in _input.Requests)
            {
                totalreq += request.NbReq;
                var source = _input.Endpoints[request.SourceEndpointId];
                var sourceCaches = source.ConnectedCaches;
                var gain = 0;
                foreach (var cache in sourceCaches)
                {
                    if (cache.VideosStored.ContainsKey(request.VideoId))
                    {
                        var localgain = source.LatencyToDc - source.Connections.First(con => con.CacheId == cache.Id).LatencyToCache;
                        if (localgain > gain)
                            gain = localgain;
                    }
                }
                score += gain*request.NbReq;
            }
            return (score*1000)/totalreq;
        }
    }

    public class Cache
    {
        public readonly int Id;
        private long capacity;
        public Dictionary<int, float> VideosStored = new Dictionary<int, float>();
        private int[] sizes;

        public int NbConnections;

        public Cache(Input input, int id)
        {
            Id = id;
            sizes = input.VideoSizes;
            capacity = input.X;

            NbConnections = input.Endpoints.Count(e => e.Connections.Any(c => c.CacheId == id));
        }

        public void AddVideo(int id)
        {
            if (!VideosStored.ContainsKey(id))
            {
                capacity -= sizes[id];
                VideosStored.Add(id, 0f);
            }
        }

        public void RemoveVideo(int id)
        {
            VideosStored.Remove(id);
            capacity += sizes[id];
        }

        public long RemainingSpace()
        {
            return capacity;
        }
    }
}