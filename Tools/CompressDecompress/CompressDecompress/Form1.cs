using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using System.IO;
using System.Globalization;

namespace CompressDecompress
{
    public class Flashbits
    {
        public void begin(List<byte> s)
        {
            src = s;
            mask = 0x01;
            count = 0;
        }
        public byte get1()
        {

            byte r = 0;
            if ((src[count] & mask) != 0) r = 1;
            mask <<= 1;
            if (mask == 0)
            {
                mask = 1;
                count++;
            }
            return r;
        }
        public byte getn(byte n)
        {
            byte r = 0;
            while (n-- > 0)
            {
                r <<= 1;
                r |= get1();
            }
            return r;
        }

        private List<byte> src;
        private byte mask;
        private int count;
    }

    public partial class Form1 : Form
    {
        public Form1()
        {
            InitializeComponent();
        }

        private void button1_Click(object sender, EventArgs e)
        {
            Stream myStream = null;
            OpenFileDialog openFileDialog1 = new OpenFileDialog();

            openFileDialog1.Filter = "mcm files (*.mcm)|*.mcm|All files (*.*)|*.*";
            openFileDialog1.FilterIndex = 1;
            openFileDialog1.RestoreDirectory = true;
            if (openFileDialog1.ShowDialog() == DialogResult.OK)
            {
                try
                {
                    if ((myStream = openFileDialog1.OpenFile()) != null)
                    {
                        StreamReader reader = new StreamReader(myStream);
                        FileStream writer = new FileStream(Path.ChangeExtension(openFileDialog1.FileName, ".bin"), FileMode.Create);
                        reader.ReadLine();
                        byte count = 0;
                        bool skip = false;
                        List<byte> byteBuf2 = new List<byte>();
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
                                byteBuf2.Add(b);
                                writer.WriteByte(b);
                            }
                        }
                        writer.Close();
                        reader.Close();
                    }
                }
                catch (Exception ex)
                {
                    MessageBox.Show("Error: Could not read file from disk. Original error: " + ex.Message);
                }
            }
        }

        byte O;
        byte L;
        byte M;
        byte length;
        int ringHead;
        int m_length;
        int m_offset;
        const byte RING_BUF_SIZE = 255;
        byte[] ringBuf;
        Flashbits BS;

        private byte decompress()
        {
            byte next = 0;
            if (m_length == 0 && BS.get1() == 0)
            {
                next = BS.getn(8);
            }
            else
            {
                if (m_length == 0)
                {
                    m_offset = -BS.getn(O) - 1;
                    m_length = BS.getn(L) + M;
                }
                if (m_length-- > 0)
                {
                    next = ringBuf[(ringHead + m_offset) % RING_BUF_SIZE];
                }
            }
            ringBuf[ringHead % RING_BUF_SIZE] = next;
            ringHead++;
            return next;
        }

        private void button2_Click(object sender, EventArgs e)
        {
            Stream myStream = null;
            OpenFileDialog openFileDialog1 = new OpenFileDialog();

            openFileDialog1.Filter = "h files (*.h)|*.h|All files (*.*)|*.*";
            openFileDialog1.FilterIndex = 1;
            openFileDialog1.RestoreDirectory = true;
            if (openFileDialog1.ShowDialog() == DialogResult.OK)
            {
                try
                {
                    if ((myStream = openFileDialog1.OpenFile()) != null)
                    {
                        StreamReader reader = new StreamReader(myStream);
                        StreamWriter writer = new StreamWriter(Path.ChangeExtension(openFileDialog1.FileName, ".mcm"));
                        FileStream writerBin = new FileStream(Path.ChangeExtension(openFileDialog1.FileName, ".bin"), FileMode.Create);
                        reader.ReadLine();
                        List<byte> byteBuf = new List<byte>();
                        while (!reader.EndOfStream)
                        {
                            string s = reader.ReadLine();
                            if (s.Length == 0) s = reader.ReadLine();
                            string[] items = s.Split(',');
                            foreach (string item in items)
                            {
                                if (item.Trim().Length == 4)
                                {
                                    byte result;
                                    Byte.TryParse(item.Trim().Split('x')[1], NumberStyles.HexNumber, null as IFormatProvider, out result);
                                    byteBuf.Add(result);
                                }
                            }
                        }
                        reader.Close();
                        writer.WriteLine("MAX7456");
                        BS = new Flashbits();
                        BS.begin(byteBuf);
                        O = BS.getn(4);
                        L = BS.getn(4);
                        M = BS.getn(2);
                        length = BS.getn(16);
                        ringHead = 0;
                        m_length = 0;
                        m_offset = 0;
                        ringBuf = new byte[RING_BUF_SIZE];
                        for (int x = 0; x < 255; x++)
                        {
                            for (int i = 0; i < 54; i++)
                            {
                                byte b = decompress();
                                writerBin.WriteByte(b);
                                writer.WriteLine(Convert.ToString(b, 2).PadLeft(8, '0'));
                            }
                        }
                        writer.Close();
                        writerBin.Close();
                    }
                }
                catch (Exception ex)
                {
                    MessageBox.Show(ex.Message);
                }

            }
        }
    }
}
