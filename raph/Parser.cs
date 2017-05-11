using System;
using System.IO;
using System.Linq;

namespace Hashcode.Qualif
{
    public static class Parser
    {
        public static Input Parse(string fileName)
        {
            var input = new Input();
            using(var reader = new StreamReader(fileName))
            {
                var inputParams = reader.ReadLine().Split(' ').Select(Int32.Parse).ToArray();
                input.V = inputParams[0]; //nb videos
                input.E = inputParams[1]; //nb endpoints
                input.R = inputParams[2]; //nb req
                input.C = inputParams[3]; //nb caches
                input.X = inputParams[4]; //caches capacity

                input.VideoSizes = reader.ReadLine().Split(' ').Select(Int32.Parse).ToArray();

                input.Endpoints = new Endpoint[input.E];
                for (int i = 0; i < input.E; i++)
                {
                    var line = reader.ReadLine().Split(' ').Select(Int32.Parse).ToArray();
                    var endpoint = new Endpoint();
                    endpoint.LatencyToDc = line[0];
                    endpoint.K = line[1]; //nb cache servers

                    endpoint.Connections = new Connection[endpoint.K];
                    for (int j = 0; j < endpoint.K; j++)
                    {
                        var connec = reader.ReadLine().Split(' ').Select(Int32.Parse).ToArray();
                        var c = new Connection();
                        c.CacheId = connec[0];
                        c.LatencyToCache = connec[1];

                        endpoint.Connections[j] = c;
                    }
                    input.Endpoints[i] = endpoint;
                }

                input.Requests = new Request[input.R];
                for (int i = 0; i < input.R; i++)
                {
                    var line = reader.ReadLine().Split(' ').Select(Int32.Parse).ToArray();
                    var req = new Request();
                    req.VideoId = line[0];
                    req.SourceEndpointId = line[1];
                    req.NbReq = line[2];
                    input.Requests[i] = req;
                }
            }

            return input;
        }
    }
}

