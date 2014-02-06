package mcl.config.ui;

import java.io.Console;

import javax.swing.JFrame;
import javax.swing.JDialog;
import javax.swing.JPanel;
import javax.swing.JScrollPane;
import javax.swing.JTable;
import javax.swing.table.AbstractTableModel;
import java.awt.Dimension;
import java.awt.GridLayout;

public class cptEditor extends JPanel {

    private static String pathOf(String args[]) {
	String onto = args[0];
	if (args.length == 1)
	    return "default/"+onto+"_cpt.mcl";
	else if (args.length == 2)
	    return args[1]+"/"+onto+"_cpt.mcl";
	else if (args.length == 2)
	    return args[1]+"/"+args[2]+"/"+onto+"_cpt.mcl";
	else 
	    return args[1]+"/"+args[2]+"/"+args[3]+"/"+onto+"_cpt.mcl";
    }

    public static void main(String args[]) {
	if (args.length < 1) {
	    System.out.println("Usage: cptEditor <ontology> [domain] [agent] [controller]");
	    System.exit(0);
	}
	    
	String path = pathOf(args);
	System.out.println("path: "+path);
	cpt.loadCPTconfig(path);

	cpt.backupAllcptFiles();
	Console console = System.console();
	String comm;
	boolean going=true;
	do {
	    comm = console.readLine("Which CPT: ");
	    cpt thisone = cpt.getCPT(comm);
	    if (thisone != null) {
		makeFrame(thisone);
	    }
	    else if (comm.equals("help")) {
		printHelp();
	    }
	    else if (comm.equals("quit")) {
		going=false;
	    }
	    else if (comm.equals("list")) {
		cpt.dumpCPTlist(true);
	    }
	    else if (comm.equals("putblanks")) {
		cpt.blankInputs();
	    }
	    else if (comm.startsWith("save ")) {
		String[] scv = comm.split(" ");
		try {
		    cpt it = cpt.getCPT(scv[1]);
		    it.saveCPT();
		}
		catch (Exception e) {
		    System.out.println("Save failed: "+e);
		    System.out.println("Are you sure that '"+scv[1]+"' is a valid node?");
		    System.out.println("The argument to the save command is a NODE;");
		    System.out.println("the ontology set to which that node belongs to will be written to the file\nfrom which it was loaded.");
		}
	    }
	    else {
		System.out.println("huh?");
	    }
	} while (going);
    }

    private static void printHelp() {
	System.out.println("Commands:");
 	System.out.println(" list       : lists nodes for which CPTs have been loaded.");
  	System.out.println(" save <cpt> : saves the entire CPT bank for the configuration to which <cpt> belongs.");
  	System.out.println(" putblanks  : sets prior probability to 0 for nodes with no inputs.");

    }

    private static void makeFrame(cpt who) {
        JFrame frame = new JFrame(who.getName());
        frame.setDefaultCloseOperation(JFrame.DISPOSE_ON_CLOSE);

        //Create and set up the content pane.
	cptEditor e = new cptEditor(who);
        e.setOpaque(true); //content panes must be opaque
        frame.setContentPane(e);

        //Display the window.
        frame.pack();
        frame.setVisible(true);

    }

    public cptEditor(cpt c) {
        super(new GridLayout(1,0));
	
        JTable table = new JTable(new cptTableModel(c));
        table.setPreferredScrollableViewportSize(new Dimension(500, 70));
        table.setFillsViewportHeight(true);
	
        //Create the scroll pane and add the table to it.
        JScrollPane scrollPane = new JScrollPane(table);
	
        //Set up stricter input validation for the integer column.
        table.setDefaultEditor(Float.class,
                               new FloatEditor(0, 100));
        
	//If we didn't want this editor to be used for other
	//Integer columns, we'd do this:
	//table.getColumnModel().getColumn(3).setCellEditor(
	//	new IntegerEditor(0, 100));
	
        //Add the scroll pane to this panel.
        add(scrollPane);
    }
    
    class cptTableModel extends AbstractTableModel {

	private cpt localCPT=null;

	public cptTableModel(cpt cpt) {
	    super();
	    localCPT = cpt;
	}

	public int getColumnCount() {
	    return localCPT.nDeps()+1;
        }

        public int getRowCount() {
            return localCPT.nRows();
        }

        public String getColumnName(int col) {
	    if (col < localCPT.nDeps())
		return localCPT.getDep(col);
	    else
		return "P("+localCPT.getName()+")";
        }

        public Object getValueAt(int row, int col) {
	    if (col < localCPT.nDeps()) {
		if (((0x1 << col) & row) == 0)
		    return "false";
		else 
		    return "true";
	    }
	    else {
		return localCPT.P(row);
	    }
        }

        /*
         * JTable uses this method to determine the default renderer/
         * editor for each cell.  If we didn't implement this method,
         * then the last column would contain text ("true"/"false"),
         * rather than a check box.
         */
        public Class getColumnClass(int c) {
            return getValueAt(0, c).getClass();
        }

        public boolean isCellEditable(int row, int col) {
            //Note that the data/cell address is constant,
            //no matter where the cell appears onscreen.
	    return (col >= localCPT.nDeps());
        }

        public void setValueAt(Object value, int row, int col) {
	    // 	    System.out.println("Setting value at " + row + "," + col
	    // 			       + " to " + value
	    // 			       + " (an instance of "
	    // 			       + value.getClass() + ")");
	    if (value instanceof Double) {
		localCPT.setP(row,((Double)value).doubleValue());
	    }

            // data[row][col] = value;
            fireTableCellUpdated(row, col);

	    // System.out.println("New value of data:");
        }
    }

}
