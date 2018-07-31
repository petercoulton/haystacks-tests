using System;
using System.IO;
using System.Collections.Generic;
using System.Linq;
using System.Threading.Tasks;
using System.Text;
using System.Diagnostics;

namespace Haystack
{
    class Program
    {
        static HashSet<string> needles;
        const long MinBailSize = 4096;

        public static void Main(string[] args)
        {
            var sw = new Stopwatch();
            sw.Start();

            var needlesFile = args[Array.IndexOf(args, "--needles") + 1];
            var haystackFile = args[Array.IndexOf(args, "--haystack") + 1];
            var threadCount = int.Parse(args[Array.IndexOf(args, "--threads") + 1]);

            var lines = File.ReadAllLines(needlesFile);

            needles = new HashSet<string>(lines);

            var haystackInfo = new FileInfo(haystackFile);
            var haystackLength = haystackInfo.Length;
            var bailLength = haystackLength / threadCount;

            if (bailLength < MinBailSize) {
                threadCount = 1;
                bailLength = haystackLength;
            }

            var bails = Enumerable
                .Range(1, threadCount)
                .Select(id => new Bail(id, bailLength, haystackInfo))
                .ToList();

            bails.Last().LastIndex = long.MaxValue;

            Parallel.ForEach(bails, bail => FindNeedles(bail));
        }

        class Bail
        {
            public int ID { get; set; }
            public long LastIndex { get; set; }
            public FileStream FileStream { get; set; }

            public Bail(int id, long length, FileInfo fileInfo)
            {
                var lastIndex = length * id;
                var firstIndex = lastIndex - length;

                var fs = fileInfo.OpenRead();
                fs.Seek(firstIndex, SeekOrigin.Begin);

                this.ID = id;
                this.LastIndex = lastIndex;
                this.FileStream = fs;

            }
        }

        static void FindNeedles(Bail bail)
        {
            var offset = bail.FileStream.Position;
            var reader = new StreamReader(bail.FileStream, Encoding.UTF8);

            if (bail.ID > 1) {
                offset += reader.ReadLine().Length + 1;
            }

            String straw;
            while (offset < bail.LastIndex && (straw = reader.ReadLine()) != null)
            {
                offset += straw.Length + 1;

                if (needles.Contains(straw))
                {
                    Console.WriteLine(straw);
                }

            }
        }
    }
}
