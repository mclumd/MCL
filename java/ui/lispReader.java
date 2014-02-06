package mcl.config.ui;

import java.util.ArrayList;
import java.io.Console;

public class lispReader {
    
    public static void main(String[] args) {
	Console console = System.console();
	String comm;
	do {
	    comm = console.readLine("Reader: ");
	    lispReader lr = new lispReader(comm);
	    lr.deprotectSource();
	    System.out.println(" > '"+lr.getSource()+"'");
	    lr.dumpTokens(" > -");
	} while (!("quit".equals(comm)));
    }

    public lispReader(String src) {
	source=src;
	original=src;
    }

    public String getSource() { return source; }

    public String nextToken() {
	ArrayList<String> protectStack = new ArrayList<String>();
	boolean white=true;
	int nptr=ptr;
	while ((nptr < source.length()) &&
	       (ptr  < source.length())) {
	    // System.out.println("protectstacksize="+protectStack.size());
	    // System.out.println("P("+ptr+":"+source.charAt(ptr)+") N("+nptr+":"+source.charAt(nptr)+")");
	    // this part is stripping leading whitespace...
	    if (white) {
		if (!isWhitespace(ptr)) {
		    // System.out.println(">W<");
		    white=false;
		    nptr=ptr;
		}
		else
		    ptr++;
	    }
	    // this part is parsing the rest of the shit
	    else {
		// test for termination condition
		if ((nptr >= source.length()) || 
		    (isWhitespace(nptr) && protectStack.isEmpty())) {
		    String rv = new String(source.substring(ptr,nptr));
		    ptr=nptr;
		    return rv;
		}
		// test for close protect
		else if ((deProtectIndex(nptr) != NO_INDEX) &&
			 !protectStack.isEmpty() &&
			 deprotects(protectStack.get(0),deProtectorAt(nptr))) {
		    protectStack.remove(0);
		    nptr++;
		}
		// test for open protect
		else if (protectIndex(nptr) != NO_INDEX) {
		    // System.out.println("protector: "+nptr+"("+protectorAt(nptr)+")");
		    protectStack.add(0,protectorAt(nptr));
		    // this breaks if non-single char protectors/unprotectors
		    nptr++;
		}
		else {
		    // System.out.println("("+nptr+")");
		    nptr++;
		}
	    }
	}
	String rv = new String(source.substring(ptr,nptr));
	ptr=nptr;
	return rv;
    }

    public static String deprotect(String in) {
	lispReader q = new lispReader(in);
	q.deprotectSource();
	return new String(q.getSource());
    }

    public void deprotectSource() {
	int s=0,e=source.length()-1;
	int pie=protectIndex(s);
	int pis=deProtectIndex(e);
	if ((pie != NO_INDEX) &&
	    (pie == pis))
	    source = new String(source.substring(1,e));
    }

    public boolean moreTokens() {
	if (ptr >= source.length())
	    return false;
	else {
	    for (int i=ptr;i<source.length();i++) {
		if (!isWhitespace(i))
		    return true;
	    }
	}
	return false;
    }

    public void reset() {
	ptr=0;
	source=original;
    }

    protected String protectorAt(int index) {
	for (int i=0;i<protector.length;i++) {
	    if (source.startsWith(protector[i],index))
		return protector[i];
	}
	return "";
    }

    protected String deProtectorAt(int index) {
	for (int i=0;i<unprotector.length;i++) {
	    if (source.startsWith(unprotector[i],index))
		return unprotector[i];
	}
	return "";
    }

    protected int protectIndex(int index) {
	for (int i=0;i<protector.length;i++) {
	    if (source.startsWith(protector[i],index)) {
		// System.out.println("yes: "+source.substring(index,index+1)+" = "+
		// protector[i]);

		return i;
	    }
	    // System.out.println("no: "+source.substring(index,index+1)+" = "+
	    // protector[i]);
	}
	return NO_INDEX;
    }

    protected int deProtectIndex(int index) {
	for (int i=0;i<unprotector.length;i++) {
	    if (source.startsWith(unprotector[i],index))
		return i;
	}
	return NO_INDEX;	
    }

    protected boolean deprotects(String pro,String dep) {
	for (int i=0;i<protector.length;i++) {
	    if (pro.equals(protector[i]) &&
		dep.equals(unprotector[i])) {
		// System.out.println("deprotects("+pro+","+dep+")");
		return true;
	    }
	}
	return false;
    }

    protected boolean isWhitespace(int index) {
	// System.out.println("IWS["+source.charAt(index)+","+whitespace.indexOf(source.charAt(index))+"]");
	return (whitespace.indexOf(source.charAt(index)) != -1);
    }

    public void dumpTokens(String prefix) {
	while (moreTokens()) {
	    System.out.println(prefix+"'"+nextToken()+"'");
	}
    }

    protected String original;
    protected String source;
    protected int    ptr=0;
    
    protected String[] protector = {"(","[","\"","{"};
    protected String[] unprotector = {")","]","\"","}"};
    protected String whitespace = " \n\r";

    protected static final int NO_INDEX = -1;

}
