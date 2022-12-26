import java.io.BufferedInputStream;
import java.io.BufferedOutputStream;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.InputStream;
import java.io.OutputStream;
import java.io.IOException;
public class ABCTest {
    static BitReader getBitReader(String fnm) throws IOException {
        InputStream bis = new BufferedInputStream(new FileInputStream(fnm));
        return new InputStreamBitReader(bis);
    }
    static BitWriter getBitWriter(String fnm) throws IOException {
        OutputStream os = new BufferedOutputStream(new FileOutputStream(fnm));
        return new OutputStreamBitWriter(os);
    }
    public static void main(String [] args) throws Exception {
        /* Compressing a sequence of ternary values */
        String data = new String("BBBBABBBC");
        ABC abc = new ABC();
        Arith ac = new Arith();
        BitWriter bw = getBitWriter("output.bin");
        ac.start_encode(bw);
        for(char x : data.toCharArray()) {
            abc.encode(x,ac);
            abc.learn(x);
        }
        ac.finish_encode(); // writes "0111 1001 0100 1"
        bw.close(); // appends "000" to fill the last byte
        /* Decompressing the sequence */
        abc = new ABC();
        ac = new Arith();
        BitReader br = getBitReader("output.bin");
        ac.start_decode(br);
        String s=""; // we append decompressed symbols here
        char x;
        do {
            x = abc.decode(ac);
            abc.learn(x);
            s += x; // append symbol x
        } while(x != 'C');
        ac.finish_decode();
        br.close();
        System.out.println(" Decoded:  " + s);
        System.out.println(" Original: " + data);
        System.out.println(" Matches:  " + s.equals(data));
        System.out.println("" + Long.SIZE);
    }
} // end of class ABCTest
