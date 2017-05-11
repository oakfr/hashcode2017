using System;
using System.Collections.Generic;
using System.IO;

namespace Hashcode.Qualif
{
    /// <summary>
    /// contains an object representation of the input
    /// </summary>
	public class Input
    {
        public int V, E, R, C, X;

        public int[] VideoSizes;
        public Endpoint[] Endpoints;
        public Request[] Requests;
    }
    
    public class Endpoint
    {
        public int LatencyToDc;
        public int K;

        public Connection[] Connections;
        public Cache[] ConnectedCaches;
    }

    public class Connection
    {
        public int CacheId;
        public int LatencyToCache;
    }

    public class Request
    {
        public bool Dealtwith = false;

        public int VideoId;
        public int SourceEndpointId;
        public int NbReq;
    }
}

