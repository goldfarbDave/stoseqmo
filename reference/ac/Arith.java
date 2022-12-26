import java.io.IOException;
public class Arith {
    /** Number of bits available.*/
    final long b = Long.SIZE - 2;

    /** Index of lower quarter.*/
    final long lb = (long) 1 << (b -2);

    /** Index of midpoint.*/
    final long hb = (long) 1 << (b -1);

    /** Index of top point.*/
    final long tb = (( long) 1 << b) - 1;

    /** Mask of b 1- bits.*/
    final long mask = tb;

    /** Current range of coding interval.*/
    long R;

    /** Low index of coding interval.*/
    long L;

    /** Target location in coding interval.*/
    long D; // Decoder only

    /** Number of opposite- valued bits queued.*/
    long bits_waiting ; // Encoder only

    BitWriter output = null;
    BitReader input = null;
    /** Outputs encoder 's processed bits.*/
    void output_bits() throws IOException {
        while(R <= lb) {
            if(L+R <= hb) {
                output_all((byte) 0);
            } else if(L >= hb) {
                output_all((byte) 1);
                L = L - hb;
            } else {
                bits_waiting ++;
                L = L - lb;
            }
            L <<= 1;
            R <<= 1; // zoom in
        }
    }
    /** Writes a bit , followed  by bits_waiting * bits of opposite value.*/
    void output_all(byte bit) throws IOException {
        output.writeBit(bit);
        System.out.println("Outputting " + bit + " waiting " + bits_waiting + " total " + (1+bits_waiting));
        while( bits_waiting > 0) {
            output.writeBit((byte)(1 - bit));
            bits_waiting --;
        }
    }
    /** Sets a region.*/
    // void narrow_region( long l, long h) {
    //     L = L + l; // CAUTION: l, not 1
    //     R = h - l;
    // }
    /** Discards decoder 's processed bits.*/
    void discard_bits() throws IOException {
        while(R <= lb) {
            if(L >= hb) {
                L -= hb; D -= hb;
            } else if(L+R <= hb) { // in lower half: nothing to do
            } else {
                L -= lb; D -= lb;
            }
            L <<= 1; R <<= 1; // zoom in
            D <<= 1; D &= mask; D += input.readBit();
        }
    }
    /** Loads a region.*/
    // public void loadRegion(long l, long h) throws IOException {
    //     narrow_region(l,h);
    //     discard_bits();
    // }
    /** Returns a target pointer.*/
    public long getTarget() { return D-L; }
    /** Returns the coding range.*/
    public long getRange() { return R; }
    /** Encodes a region.*/
    // public void storeRegion(long l, long h) throws IOException {
    //     narrow_region(l,h);
    //     output_bits();
    // }
    /** Starts an encoding process. */
    public void start_encode ( BitWriter output) {
        this.output = output;
        L = 0; // lowest possiblepoint
        R = tb; // full range
        bits_waiting = 0;
    }
    /** Starts a decoding process. */
    public void start_decode ( BitReader input) throws IOException {
        this.input = input;
        D = 0; // fill data pointer with bits
        for (int k=0; k<b; k++) {
            D <<= 1;
            D += input. readBit ();
        }
        L = 0;
        R = tb; // WNC use "tb", MNW use "hb"
    }
    /** Finishes an encoding process. */
    public void finish_encode () throws IOException {
        while (true) {
            if (L + (R >>1) >= hb) {
                output_all ((byte) 1);
                if (L < hb) {
                    R -= hb - L;
                    L = 0;
                } else {
                    L -= hb;
                }
            } else {
                output_all ((byte) 0);
                if (L+R > hb) { R = hb - L; }
            }
            if (R == hb) { break; }
            L <<= 1;
            R <<= 1;
        }
    }
    /** Finishes a decoding process. */
    public void finish_decode () {
        // no action required
    }

    // Scaling methods
    void narrow_region (long l, long h, long t) throws IOException {
        long r = R / t;
        L = L + r*l;
        R = h < t ? r * (h-l) : R - r*l;
    } // Moffat -Neal - Witten (1998)
    void storeRegion(long l, long h, long t) throws IOException {
        narrow_region(l,h,t);
        output_bits();
    }
    void loadRegion(long l, long h, long t) throws IOException {
        narrow_region(l,h,t);
        discard_bits();
    }
    long getTarget(long t) throws IOException {
        long r = R / t;
        long dr =(D-L) / r;
        return(t -1 < dr) ? t -1 : dr;
    } // Moffat -Neal - Witten(1998)
}
