using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.IO;

namespace ConvertToBin
{
    class Program
    {
        static void Main(string[] args)
        {
            string fname = args[0];
            StreamReader reader = new StreamReader(fname);
            FileStream writer = new FileStream(args[1], FileMode.Create);
            reader.ReadLine();
            byte count = 0;
            bool skip = false;
            while (!reader.EndOfStream)
            {
                string s = reader.ReadLine();
                if (count % 54 == 0)
                {
                    skip = true;
                }
                if (count % 64 == 0)
                {
                    skip = false;
                    count = 0;
                }
                count++;
                byte b = Convert.ToByte(s, 2);
                if (!skip)
                {
                    writer.WriteByte(b);
                }
            }
            writer.Close();
            reader.Close();
        }
    }
}
