using System;
using System.Diagnostics;
using System.IO;
using System.Threading.Tasks;

namespace Hashcode.Qualif
{
    class MainClass
    {
        public static void Main(string[] args)
        {
            var files = new[]
            {
                "me_at_the_zoo",
//                "kittens",
//                "trending_today",
                "videos_worth_spreading",
            };

            Parallel.ForEach(files, file =>
            {
                string fileName = "../../../" + file + ".in"; //put input file where sources are

                var input = Parser.Parse(fileName);

                var sw = Stopwatch.StartNew();
                var solution = Solver.Solve(input);
                Console.WriteLine("Done, " + sw.ElapsedMilliseconds + "ms elapsed");

                //write output file
                var outputFile = file + "-" + solution.Score + ".out";
                using (var writer = new StreamWriter("../../" + outputFile))
                {
                    writer.Write(solution);
                }
                Console.WriteLine("file dumped : " + outputFile);
            });
        }
    }
}
