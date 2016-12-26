import time
import sys
import array

def prefix(s1, s2):
    """ Return the length of the common prefix of s1 and s2 """
    sz = len(s2)
    for i in range(sz):
        if s1[i % len(s1)] != s2[i]:
            return i
    return sz

class Bitstream(object):
    def __init__(self):
        self.b = []
    def append(self, sz, v):
        assert 0 <= v
        assert v < (1 << sz)
        for i in range(sz):
            self.b.append(1 & (v >> (sz - 1 - i)))
    def toarray(self):
        bb = [0] * ((len(self.b) + 7) / 8)
        for i,b in enumerate(self.b):
            if b:
                bb[i / 8] |= (1 << (i & 7));
        return array.array('B', bb)

class Codec(object):
    def __init__(self, b_off, b_len):
        self.b_off = b_off
        self.b_len = b_len
        self.history = 2 ** b_off
        refsize = (1 + self.b_off + self.b_len) # bits needed for a backreference
        if refsize < 9:
            self.M = 1
        elif refsize < 18:
            self.M = 2
        else:
            self.M = 3
        # print "M", self.M
        # e.g. M 2, b_len 4, so: 0->2, 15->17
        self.maxlen = self.M + (2**self.b_len) - 1

    def compress(self, blk):
        lempel = {}
        sched = []
        pos = 0
        while pos < len(blk):
            k = blk[pos:pos+self.M]
            older = (pos - self.history - 1)
            candidates = [p for p in lempel.get(k, []) if (older < p)]
            (bestlen, bestpos) = max([(0, 0)] + [(prefix(blk[p:pos], blk[pos:]), p) for p in candidates])
            if k in lempel:
                lempel[k].add(pos)
            else:
                lempel[k] = set([pos])
            bestlen = min(bestlen, self.maxlen)
            if bestlen >= self.M:
                sched.append((bestpos - pos, bestlen))
                pos += bestlen
            else:
                sched.append(blk[pos])
                pos += 1
        return sched

    def toarray(self, blk):
        bs = Bitstream()
        bs.append(4, self.b_off)
        bs.append(4, self.b_len)
        bs.append(2, self.M)
        sched = self.compress(blk)
        bs.append(16, len(sched))
        for c in sched:
            if len(c) != 1:
                (offset, l) = c
                bs.append(1, 1)
                bs.append(self.b_off, -offset - 1)
                bs.append(self.b_len, l - self.M)
            else:
                bs.append(1, 0)
                bs.append(8, ord(c))
        return bs.toarray()


    def to_cfile(self, hh, blk, name):
        print >>hh, "PROGMEM const byte %s[] = {" % name
        bb = self.toarray(blk)
        for i in range(0, len(bb), 16):
            if (i & 0xff) == 0:
                print >>hh
            for c in bb[i:i+16]:
                print >>hh, "0x%02x, " % c,
            print >>hh
        print >>hh, "};"

    def decompress(self, sched):
        s = ""
        for c in sched:
            if len(c) == 1:
                s += c
            else:
                (offset, l) = c
                for i in range(l):
                    s += s[offset]
        return s

def main():
    from optparse import OptionParser
    parser = OptionParser("%prog [ --lookback O ] [ --length L ] --name NAME inputfile outputfile")

    parser.add_option("--lookback", type=int, default=8, dest="O", help="lookback field size in bits")
    parser.add_option("--length", type=int, default=3, dest="L", help="length field size in bits")
    parser.add_option("--name", type=str, default="data", dest="NAME", help="name for generated C array")
    parser.add_option("--binary", action="store_true", default=False, dest="binary", help="write a binary file (default is to write a C++ header file)")
    options, args = parser.parse_args()

    if len(args) != 2:
        parser.error("must specify input and output files");

    print options.O
    print options.L
    print options.NAME
    print args
    (inputfile, outputfile) = args
    cc = Codec(b_off = options.O, b_len = options.L)
    uncompressed = open(inputfile, "rb").read()
    if options.binary:
        compressed = cc.toarray(uncompressed)
        open(outputfile, "wb").write(compressed.tostring())
    else:
        outfile = open(outputfile, "w")
        cc.to_cfile(outfile, uncompressed, options.NAME)

if __name__ == "__main__":
    main()
