
public class ABC {
    /** Local state */
    int na = 1;
    int nb = 1;
    int nc = 1;
    int all = 3;
    /** Records a symbol observation . */
    public void learn(char c) throws Exception {
        switch(c) {
        case 'A': na++; break;
        case 'B': nb++; break;
        case 'C': nc++; break;
        default : throw new IllegalArgumentException();
        }
        all ++;
    }
    /** Encodes a symbol. */
    public void encode(char c, Arith ec) throws Exception {
        System.out.println("Encoding " + c);
        switch(c) {
        case 'A': ec.storeRegion(0, na , all ); break;
        case 'B': ec.storeRegion(na , na+nb , all ); break;
        case 'C': ec.storeRegion(na+nb , all , all ); break;
        default : throw new IllegalArgumentException();
        }
    }
    /** Decodes a symbol. */
    public char decode(Arith dc) throws Exception {
        long t = dc.getTarget(all );
        if(t >= 0) {
            if(t < na) {
                dc.loadRegion(0, na , all );
                return 'A';
            } else if(t < na+nb) {
                dc.loadRegion(na , na+nb , all );
                return 'B';
            } else if(t < all) {
                dc.loadRegion(na+nb , all , all );
                return 'C';
            }
        }
        throw new IllegalStateException();
    }
} // end of class ABC
