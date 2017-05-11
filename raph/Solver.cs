using System;
using System.IO;
using System.Linq;
using System.Text;
using System.Collections.Generic;
using MoreLinq;

namespace Hashcode.Qualif
{
	public class Solver
	{
        public static Solution Solve(Input input)
        {
            var solution = new Solution(input);

            //precompute useful stuff
            foreach (var endpoint in input.Endpoints)
            {
                endpoint.ConnectedCaches = solution.Caches.Where(c => endpoint.Connections.Any(con => con.CacheId == c.Id)).ToArray();
            }

            for (int i = 0; i < input.R; i++)
            {
                var req = input.Requests[i];

                foreach (var cach in input.Endpoints[req.SourceEndpointId].ConnectedCaches)
                {
                    cach.AddVideo(req.VideoId);
                    cach.VideosStored[req.VideoId] += ((float)req.NbReq / input.Endpoints[req.SourceEndpointId].ConnectedCaches.Length)/input.VideoSizes[req.VideoId];
                }
            }

            //now clean caches
            var allgood = true;
            do
            {
                allgood = true;
                for (int i = 0; i < solution.Caches.Length; i++)
                {
                    var cache = solution.Caches[i];

                    if (cache.RemainingSpace() < 0)
                    {
                        allgood = false;
                        var worst = cache.VideosStored.MinBy(kvp => kvp.Value);
                        cache.RemoveVideo(worst.Key);
                        Recompute(input, solution, worst.Key);
                    }
                    else
                    {
                        Console.WriteLine("one done");
                    }
                }
            } while (!allgood);

            return solution;
        }

	    private static void Recompute(Input input, Solution sol, int worstKey)
	    {
	        foreach (var cach in sol.Caches)
	        {
	            cach.VideosStored[worstKey] = 0;
	        }

            for (int i = 0; i < input.R; i++)
            {
                var req = input.Requests[i];
                if(req.VideoId != worstKey)
                    continue;

                var enumerable = input.Endpoints[req.SourceEndpointId].ConnectedCaches.Where(ca => ca.VideosStored.ContainsKey(req.VideoId)).ToArray();
                foreach (var cach in enumerable)
                {
                    cach.VideosStored[req.VideoId] += ((float)req.NbReq / enumerable.Length) / input.VideoSizes[req.VideoId];
                }
            }
        }
	}
}
