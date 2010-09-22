import java.io.*;
import java.net.*;

public static class PositionReader extends Thread
{
  static Socket tcpc;
  public static String status = "waiting"; // meditating/waiting
  public static int lastEvent;
  public static int lastUser;
  public static float mood = 0.0;
  public static float[] lastMove = new float[5];
  public static Hashtable users = new Hashtable();
  public static String mode = "tcp"; // tcp, pipe, mouse, mouse2
  BufferedReader br;
  public static PApplet processing;
  //String filename;
  //InputStreamReader isr;
  //String buffer = "";

  public PositionReader(PApplet processing, String mode) {
    //filename = "txtFile.txt";
    //txtInput = loadStrings("txtFile.txt")[0];
    //br = Executor.runCommand("java -jar ../../output_position/applet/output_position.jar");
    this.processing = processing;
    this.mode = mode;
    if (mode == "pipe") {
      br = runCommand("/example");
    } else if (mode == "tcp") {
      try {
        tcpc = new Socket("127.0.0.1", 9090);
        InputStreamReader in = new InputStreamReader(tcpc.getInputStream());
        br = new BufferedReader(in);
      } catch (Exception e) {
          System.err.println("Error when attempting connection: \"" + e.getMessage()+ "\"");
      }
    }
    //users = new Hashtable();
    //isr = Executor.runCommandIsr("/example");
  }

  public void run() {
    while (true) { 
      this.parsetxt(); 
      try { Thread.sleep(50); } catch(Exception e) { println("Exception"); } 
    }
  }

  public static BufferedReader runCommand(String command) {
    BufferedReader br = null;
    try {
      Runtime rtime = Runtime.getRuntime();
      Process child = rtime.exec(command);
      InputStream stdout = child.getInputStream();
      InputStreamReader isr = new InputStreamReader(stdout);
      br = new BufferedReader(isr);
    } 
    catch(Exception e) {
      System.out.println("Error");
      e.printStackTrace();
    }
    return br;
  }

  public void parsetxt(){
    //char cRead = ' ';
    String txtInput = null;
    //String[] lines = loadStrings(filename);
    //if (lines.length < 1) return;
    //txtInput = lines[0];
    try {
      if (!mode.startsWith("mouse")) {
        txtInput = br.readLine(); 
      } else {
        int user = 0;
        int mx = processing.mouseX;
        int my = processing.mouseY;
        if (mode.endsWith("2")) user = processing.round(processing.random(0, 1));
        if (user == 1) {
            mx = mx + (processing.width / 2);
            my = my + (processing.height / 2);
        }
        txtInput = "1|"+user+"|"+(mx/(float)processing.width)+"|"+(my/(float)processing.height)+"|0|0";
        println("? "+txtInput);
        println(processing.mouseX);
      }
    } 
    catch (Exception e) {
      e.printStackTrace();
    }
    //try { cRead = (char)isr.read(); } catch (Exception e) {e.printStackTrace();}
    //buffer = buffer + cRead;
    //if (cRead == '\n') {
    //txtInput = buffer;
    //buffer = "";
    if (txtInput == null) return;
    //println(txtInput);
    if (txtInput.startsWith("[/TCP]")) {
      txtInput = txtInput.substring(7);
    }
    String[] txtOut =  split(txtInput, "|");
    if (txtOut.length != 6) return;
    lastEvent = int(txtOut[0]);
    //println("correct? "+ txtInput);
    if (lastEvent == 3) { // mood
      this.mood = float(txtOut[2]);
    } else {
      lastUser = int(txtOut[1]);
      lastMove[0] = float(txtOut[2]);
      lastMove[1] = float(txtOut[3]);
      lastMove[2] = float(txtOut[4]);
      lastMove[3] = float(txtOut[5]);
      lastMove[4] = this.processing.millis();
      if (lastMove[0] == 0.0 || lastMove[1] == 0.0) {
        println("ignoring user "+lastUser+ "!");
        return;
      }
      // add to participants object
      if (lastEvent == 0 || lastEvent == 1) {
        add(lastUser, lastMove[0], lastMove[1], lastMove[2], lastMove[3], processing.millis());
      } 
      else if (lastEvent == 2) {
        users.remove(lastUser);
      }
    }
    println(toString());
    //}
  }

  public void add(int userId, float xCor, float yCor, float xOrt, float yOrt, float time) {
    float[] details = { xCor, yCor, xOrt, yOrt, time };
    
    Integer key = new Integer(userId);
    User user;
    if (users.containsKey(key)) {
      user = (User)users.get(key);
    } else {
      if (status == "meditating") return;
      user = (new processingExample()).new User(this.processing, userId);
      users.put(new Integer(userId), user);
    }
    user.update(round(xCor * processing.width), round(yCor * processing.height));
  }

  public void setAllMeditating() {
    for (Enumeration e = this.users.keys(); e.hasMoreElements();) {
      Integer key = (Integer)e.nextElement();
      User u = (User)users.get(key);
      u.setStill();
    }
  }

  public void setAllEnded() {
    int winner = -1;
    int score = 0;
    for (Enumeration e = this.users.keys(); e.hasMoreElements();) {
      Integer key = (Integer)e.nextElement();
      User u = (User)users.get(key);
      u.setEnded();
      if (winner == -1) {
        score = u.getScore();
        winner = u.getId();
      } else if (u.getScore() < score) {
        score = u.getScore();
        winner = u.getId();
      }
    }

  }

  public void drawAll() {
    for (Enumeration e = this.users.keys(); e.hasMoreElements();) {
      Integer key = (Integer)e.nextElement();
      User u = (User)users.get(key);
      u.drawUser();
    }
  }

  public void remove(int userId) {
    users.remove(new Integer(userId));
  }

  public int count() {
    return users.size();
  }
  
  public int countStill() {
    int count = 0;
    for (Enumeration e = this.users.keys(); e.hasMoreElements();) {
      User u = (User)users.get((Integer)e.nextElement());
      if (u.isStill()) {
        println("user "+u.getId()+" is still");
        count++;
      } else {
        println("user "+u.getId()+" is not still");
      }
    }
    return count;
  }

  public User get(int userId) {
    return (User)users.get(new Integer(userId));
  }

  public static float getX() {
    return lastMove[0] * processing.width;
  }

  public static float getY() {
    return lastMove[1] * processing.height;
  }

  public float getY(int userId) {
    float y = 0;
    try { y = get(userId).getY(); } 
      catch (Exception e) { }
    return y;
  }
  
  public float getX(int userId) {
    float x = 0;
    try { x = get(userId).getX(); } 
      catch (Exception e) { }
    return x;
  }
  
  public float getMood() {
    return this.mood;
  }

  public String toString() {
    return ""+count() + " users. Last position: "+getX()+","+getY();
  }
}
