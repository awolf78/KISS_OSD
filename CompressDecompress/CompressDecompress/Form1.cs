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
    public partial class Form1 : Form
    {
        public Form1()
        {
            InitializeComponent();
        }

        List<byte> byteBuf2;
        List<byte> compressed;

        private int checkOffset(int nextByteIndex, int maxO, int maxL, int offset)
        {
            int lastByteIndex = nextByteIndex;
            while (nextByteIndex < byteBuf2.Count
                && byteBuf2[nextByteIndex - offset] == byteBuf2[nextByteIndex]
                && (nextByteIndex - lastByteIndex) < maxL) nextByteIndex++;
            int length = nextByteIndex - lastByteIndex;
            return length;
        }

        int maxLength2;

        private int findMaxLength(int nextByteIndex, int maxO, int maxL)
        {
            maxLength2 = 0;
            int newOffset = 1;
            int maxLengthOffset = newOffset;
            while (newOffset < maxO && (nextByteIndex - newOffset) >= 0)
            {
                int newLength = checkOffset(nextByteIndex, maxO, maxL, newOffset);
                if (newLength > maxLength2)
                {
                    maxLength2 = newLength;
                    maxLengthOffset = newOffset;
                }
                newOffset++;
            }
            return maxLengthOffset;
        }

        int bitIndex;
        byte next;

        private void AddBits(int toAdd, byte count)
        {
            while (count > 0)
            {
                next |= (byte)(((toAdd >> (count - 1)) & 1) << (bitIndex % 8));
                count--;
                bitIndex++;
                if (bitIndex % 8 == 0)
                {
                    compressed.Add(next);
                    next = 0;
                }
            }
        }

        private void compress(byte O, byte L)
        {
            bitIndex = 0;
            next = 0;
            const byte M = 2;
            compressed = new List<byte>();
            AddBits(O, 4);
            AddBits(L, 4);
            AddBits(M, 2);
            AddBits((byte)((byteBuf2.Count & 0xFF00) >> 8), 8);
            AddBits((byte)(byteBuf2.Count & 0x00FF), 8);
            int maxO = 1;
            int i;
            for (i = 0; i < O - 1; i++)
            {
                maxO = (maxO << 1) + 1;
            }
            int maxL = 1;
            for (i = 0; i < L - 1; i++)
            {
                maxL = (maxL << 1) + 1;
            }
            maxL += M;
            AddBits(0, 1);
            AddBits(byteBuf2[0], 8);
            int nextByteIndex = 1;
            while (nextByteIndex < byteBuf2.Count)
            {
                int newOffset = findMaxLength(nextByteIndex, maxO, maxL);
                if (maxLength2 >= 2)
                {
                    nextByteIndex += maxLength2 - 1;
                    maxLength2 -= M;
                    AddBits(1, 1);
                    AddBits(newOffset - 1, O);
                    AddBits(maxLength2, L);
                }
                else
                {
                    AddBits(0, 1);
                    AddBits(byteBuf2[nextByteIndex], 8);
                }
                nextByteIndex++;
            }
            if (next > 0 || (bitIndex % 8) != 0) compressed.Add(next);
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
                        if (radioButton1.Checked)
                        {
                            StreamReader reader = new StreamReader(myStream);
#if DEBUG
                            FileStream writer = new FileStream(Path.ChangeExtension(openFileDialog1.FileName, ".bin"), FileMode.Create);
#endif
                            reader.ReadLine();
                            byte count = 0;
                            bool skip = false;
                            byteBuf2 = new List<byte>();
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
#if DEBUG
                                    writer.WriteByte(b);
#endif
                                }
                            }
                        }
                        else
                        {
                            FileStream reader = new FileStream(openFileDialog1.FileName, FileMode.Open, FileAccess.Read);
                            int newByte = reader.ReadByte();
                            byteBuf2 = new List<byte>();
                            while (newByte > 0)
                            {
                                byteBuf2.Add((byte)newByte);
                                newByte = reader.ReadByte();
                            }
                        }
                        compress((byte)OffsetUpDown.Value, (byte)LengthUpDown.Value);
                        if (radioButton1.Checked)
                        {
                            string fname = Path.GetDirectoryName(openFileDialog1.FileName) + "\\fontCompressed.h";
                            StreamWriter writer2 = new StreamWriter(fname);
                            writer2.WriteLine("PROGMEM const byte fontCompressed[" + compressed.Count.ToString() + "] = {");
                            byte count = 0;
                            foreach (byte item in compressed)
                            {
                                string temp = "0x" + item.ToString("X2") + ", ";
                                writer2.Write(temp);
                                count++;
                                if (count % 16 == 0) writer2.WriteLine();
                            }
                            writer2.WriteLine("};");
                            writer2.Close();
                        }
                        else
                        {
                            FileStream writer = new FileStream(Path.ChangeExtension(openFileDialog1.FileName, ".bin"), FileMode.Create);
                            foreach (byte item in compressed)
                            {
                                writer.WriteByte(item);                          
                            }
                            writer.Close();
                        }
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
        int ringHead;
        int m_length;
        int m_offset;
        int maxLength;
        const int RING_BUF_SIZE = 512; // 9 bit offset max
        byte[] ringBuf;
        Flashbits BS;

        private byte decompress()
        {
            byte next = 0;
            if (m_length == 0 && BS.get1() == 0)
            {
                next = (byte)BS.getn(8);
            }
            else
            {
                if (m_length == 0)
                {
                    m_offset = -((int)BS.getn(O)) - 1;
                    m_length = (int)BS.getn(L) + M;
                }
                if (m_length-- > 0)
                {                    
                    next = ringBuf[(ringHead + m_offset) % RING_BUF_SIZE];
                }
                if (m_offset < maxLength) maxLength = m_offset;
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
                        O = (byte)BS.getn(4);
                        L = (byte)BS.getn(4);
                        M = (byte)BS.getn(2);
                        int byteLength = 0;
                        byteLength |= ((int)BS.getn(8)) << 8;
                        byteLength |= (int)BS.getn(8);
                        ringHead = 0;
                        m_length = 0;
                        m_offset = 0;
                        ringBuf = new byte[RING_BUF_SIZE];
                        maxLength = 0;
                        List<byte> decompressed = new List<byte>();
                        /*for (int x = 0; x < 255; x++) //should be x < 256 otherwise last char is omitted - whatever
                        {
                            for (int i = 0; i < 54; i++)
                            {
                                byte b = decompress();
                                decompressed.Add(b);
                                writerBin.WriteByte(b);
                                writer.WriteLine(Convert.ToString(b, 2).PadLeft(8, '0'));
                            }
                        }*/
                        for(int i=0; i < byteLength; i++)
                        {
                            byte b = decompress();
                            decompressed.Add(b);
                            writerBin.WriteByte(b);
                        }
                        Console.WriteLine(maxLength);
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
        public uint getn(byte n)
        {
            uint r = 0;
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
}
