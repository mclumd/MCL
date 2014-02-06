package mcl.config.ui;

import java.io.*;
import java.util.ArrayList;
import java.util.HashMap;

public class cpt {

    private static HashMap<String,cpt> cpts = new HashMap<String,cpt>();

    public  static cpt getCPT(String name) {
	if (cpts.containsKey(name))
	    return cpts.get(name);
	else {
	    int c=0;
	    String dKey="";
	    for (String key : cpts.keySet()) {
		// System.out.println("Searching: "+key);
		String[] skd = key.split("::");
		if ((skd.length > 1) && name.equals(skd[1])) {
		    c++;
		    if (c > 1)
			dKey += " "+key;
		    else
			dKey = key;
		}
	    }
	    if (c > 1) {
		System.out.println("Table exists in multiple files: "+dKey);
		return null;
	    }
	    else
		return cpts.get(dKey);
	}
    }

    public static void backupAllcptFiles() {
	HashMap<String,String> already = new HashMap<String,String>();
	for (String key : cpts.keySet()) {
	    if (!already.containsKey(getCPT(key).getFilename())) {
		// make a backup
		getCPT(key).saveCPT(".bak");
		already.put(getCPT(key).getFilename(),"yay");
	    }
	}
    }

    public static void loadCPTconfig(String tablePath) {
        BufferedReader pro_file = null;
        try {
	    String fn = System.getenv("MCL_CONFIG_PATH")+"config/"+tablePath;
            pro_file = new BufferedReader(new FileReader(fn));
	    String sline = pro_file.readLine();
	    while (sline != null) {
		cpt k = new cpt(sline,fn);
		sline=pro_file.readLine();
	    }
	}
	catch (Exception e) {
	    System.out.println("Error: "+e);
	}
    }

    public static void blankInputs() {
	for (String key : cpts.keySet()) {
	    cpt c = getCPT(key);
	    if (c.nDeps() == 0)
		c.blank();
	}
    }

    public static void dumpCPTlist(boolean stripFN) {
	int q=0;
	for (String key : cpts.keySet()) {
	    cpt c = cpts.get(key);
	    if (stripFN) {
		String[] kss = key.split("::");
		if ((q % 2) == 0)
		    System.out.format(" >> [%d]%-28s",c.nDeps(),kss[1]);
		else 
		    System.out.format(" >> [%d]%-28s\n",c.nDeps(),kss[1]);
	    }
	    else {
		System.out.println(" >> "+"["+c.nDeps()+"]"+key);
	    }
	    q++;
	}
	System.out.format("(%d total)\n",q);
    }

    public void saveCPT() {
	saveCPT("");
    }

    public void saveCPT(String ext) {
        BufferedWriter pro_file = null;
        try {
	    String fn = filename+ext;
	    System.out.println("writing "+fn+"..."); 
            pro_file = new BufferedWriter(new FileWriter(fn));
	    for (String cptkey : cpts.keySet()) {
		cpt theCPT = cpts.get(cptkey);
		// System.out.println("writing "+theCPT.getName()+"..."); 
		if (theCPT.getFilename().equals(filename)) 
		    pro_file.write(theCPT.descriptor()+"\n");
	    }
	    pro_file.close();
	}
	catch (Exception e) {
	    System.out.println("Error: "+e);
	}	
    }

    public static void main(String[] arg) {
        BufferedReader pro_file = null;
        try {
	    String fn = System.getenv("MCL_CONFIG_PATH")+"config/default/basic_cpt.mcl";
	    System.out.println("source = "+fn);
            pro_file = new BufferedReader(new FileReader(fn));
	    String sline = pro_file.readLine();
	    // System.out.println(sline);
	    while (sline != null) {
		cpt k = new cpt(sline,fn);
		sline=pro_file.readLine();
	    }
	}
	catch (Exception e) {
	    System.out.println("Error: "+e);
	    e.printStackTrace();
	}
	cpt poo = getCPT("missed-target");
	boolean[] k = { false,true,true };
	if (poo == null) {
	    System.err.println("Can't find 'missed-target'");
	}
	else {
	    System.out.println("p="+poo.P(k));
	}
    }

    public cpt(String descriptor,String fname) {
	recoverFromDescriptor(descriptor);
	filename=fname;
	cpts.put(fname+"::"+name,this);
	// System.out.println("put("+name+") : "+cpts.size()+"entries ");
    }

    protected void recoverFromDescriptor(String d) {
	// System.out.println("recover: "+d);
	lispReader lr = new lispReader(d);
	lr.deprotectSource();
	name = lispReader.deprotect(lr.nextToken());
	String depstr="";
	String cpv="";
	while (lr.moreTokens()) {
	    String key = lr.nextToken();
	    String val = lr.nextToken();
	    if (key.equals(":dep")) {
		// System.out.println("dependencies = "+val);
		depstr=val;
	    }
	    if (key.equals(":cpv")) {
		// System.out.println("probabilities : "+val.length());
		cpv=val;
	    }
	}
	if (depstr.length() > 0) {
	    lispReader dr = new lispReader(depstr);
	    dr.deprotectSource();
	    ArrayList<String> dal = new ArrayList<String>();
	    while (dr.moreTokens()) {
		dal.add(lispReader.deprotect(dr.nextToken()));
	    }
	    dependencies = new String[dal.size()];
	    for (int i=0;i<dal.size();i++) {
		dependencies[i]=dal.get(i);
		// System.out.println("d["+i+"]="+dependencies[i]);
	    }
	}
	if (cpv.length() > 0) {
	    lispReader cr = new lispReader(cpv);
	    cr.deprotectSource();
	    probs = new double[(int)Math.floor(Math.pow(2,dependencies.length))];
	    for (int i=0;i<probs.length;i++) {
		probs[i]=Double.parseDouble(cr.nextToken());
		// System.out.println("p["+i+"]="+probs[i]);
	    }
	}
	//	lr.dumpTokens(" ::>");
    }

    public String descriptor() {
	String rv = "(";
	rv+="\""+name+"\"";
	if (nDeps() > 0) {
	    rv+=" :dep (";
	    for (int i=0;i<nDeps();i++) {
		rv+= "\""+getDep(i)+"\" ";
	    }
	    rv+=")";
	}
	if (nRows() > 0) {
	    rv+=" :cpv (";
	    for (int i=0;i<nRows();i++) {
		rv+= ""+P(i)+" ";		
	    }
	    rv+=")";
	}
	rv+=")";
	return rv;
    }

    public void setP(int k,double d) {
	probs[k]=d;
    }

    public double P(int k) {
	return probs[k];
    }

    public double P(boolean[] dvals) {
	int index=0;
	for (int i=0;i<dvals.length;i++) {
	    if (dvals[i])
		index+=(int)Math.pow(2,i);
	}
	return probs[index];
    }

    public void blank() {
	for (int i=0;i<nRows();i++) {
	    probs[i]=0;
	    System.out.println(name+" blanked...");
	}
    }
    
    public int nRows() { return probs.length; }
    public int nDeps() { return dependencies.length; }
    public String getName() { return name; }
    public String getFilename() { return filename; }
    public String getDep(int i) { return dependencies[i]; }
    

    protected double[] probs = null;
    protected String[] dependencies = new String[0];
    protected String   name = "";
    protected String   filename = "unknown_cpt.mcl";

}
