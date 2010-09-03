import processing.core.*; 
import processing.xml.*; 

import java.io.*; 
import java.net.*; 
import java.util.Vector; 

import java.applet.*; 
import java.awt.*; 
import java.awt.image.*; 
import java.awt.event.*; 
import java.io.*; 
import java.net.*; 
import java.text.*; 
import java.util.*; 
import java.util.zip.*; 
import java.util.regex.*; 

public class meditate extends PApplet {

PositionReader reader;
// wait for players at first
String status = "waiting";
int offset = 10;
int startTime;
int readyTime;
int meditateTime;
int meditationTime = 30;
Hashtable users = new Hashtable();
PFont font;
int numUsers = 2;

public void setup()
{
  size(800, 600);
  //size(320, 240);
  frameRate(30);
  background(255);
  smooth();
  strokeWeight(2);
  reader = new PositionReader(this, "tcp");
  reader.setDaemon(true);
  reader.start();
  startTime = millis();
  smooth();
  font = loadFont("LucidaSans-30.vlw"); 
  textFont(font, 30);
  fill(0xff000000);
}

public void draw()
{
  background(0xffffffff);
  if (status == "waiting") {
    drawWaiting();
  } else if (status == "ready") {
    drawCountdown();
  } else if (status == "running") {
    drawRunning();
  } else {
    drawComplete();     
  }
  drawAll();
  updateStatus();
}

public void drawAll() {
  reader.drawAll();
}

public void updateStatus()
{
  if (status == "waiting") {
    if (reader.countStill() >= numUsers) {
      status = "ready";
      readyTime = millis();
    }
  } else if (status == "ready") {
    if (reader.countStill() < numUsers) {
      status = "waiting";
    }
    if (((millis() - readyTime) / 1000) >= 3) {
      reader.setAllMeditating();
      meditateTime = millis();
      status = "running";
      reader.status = "meditating";
    }
  } else if (status == "running") {
    if ((millis() - meditateTime) / 1000 >= meditationTime) {
      reader.setAllEnded();
      status = "complete";
    }
  }
}

public void drawWaiting() {
  fill(0xff000000);
  text("Waiting for meditators", 30, 40);
  text(reader.countStill()+"/"+numUsers+" ready to start", 30, 70);
  fill(0xffcccccc);
  ellipse(150, 330, 100, 100);
  ellipse(400, 330, 100, 100);
  ellipse(650, 330, 100, 100);
  //ellipse(250, 420, 60, 60);
}

public void drawCountdown() {
  fill(0xff000000);
  text("Ready to start in", 30, 40);
  int secondsLeft = 3 - ((millis() - readyTime) / 1000);
  text(secondsLeft, 30, 70);
}

public void drawRunning() {
  fill(0xffff0000);
  text("!!! MEDITATE !!!", 30, 40);
  int secondsLeft = 60 - ((millis() - readyTime) / 1000);
  text(secondsLeft, 30, 70);
}

public void drawComplete() {
  fill(0xff000000);
  text("You are happy now! :)", 30, 40);
  
}



public static class PositionReader extends Thread
{
  static Socket tcpc;
  public static String status = "waiting"; // meditating/waiting
  public static int lastEvent;
  public static int lastUser;
  public static float mood = 0.0f;
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
        //println("? "+txtInput);
        //println(processing.mouseX);
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
    lastEvent = PApplet.parseInt(txtOut[0]);
    //println("correct? "+ txtInput);
    if (lastEvent == 3) { // mood
      this.mood = PApplet.parseFloat(txtOut[2]);
    } else {
      lastUser = PApplet.parseInt(txtOut[1]);
      lastMove[0] = PApplet.parseFloat(txtOut[2]);
      lastMove[1] = PApplet.parseFloat(txtOut[3]);
      lastMove[2] = PApplet.parseFloat(txtOut[4]);
      lastMove[3] = PApplet.parseFloat(txtOut[5]);
      lastMove[4] = this.processing.millis();
      if (lastMove[0] == 0.0f || lastMove[1] == 0.0f) {
        println("ignoring user "+lastUser+ "!");
        return;
      }
      // add to participants object
      if (lastEvent == 0 || lastEvent == 1) {
        add(lastUser, lastMove[0], lastMove[1], lastMove[2], lastMove[3], processing.millis());
      } 
      else if (lastEvent == 2) {
        //users.remove(lastUser);
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
      user = (new meditate()).new User(this.processing, userId);
      users.put(new Integer(userId), user);
    }
    user.update(round(xCor * processing.width), round(yCor * processing.height));
  }

  public void setAllMeditating() {
    for (Enumeration e = this.users.keys(); e.hasMoreElements();) {
      Integer key = (Integer)e.nextElement();
      User u = (User)users.get(key);
      u.setMeditating();
      //print("now: "+time);
      //println(" key "+key+": "+a[4]);
      //if ((this.processing.millis() - u.getTime()) > 5000) {
      //  println("removing "+ key);
      //  users.remove(key);
      //}
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
    if (winner >= 0) {
      User u = get(winner);
      u.setWinner();
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


public class User
{
  int id;
  String status; // wander, ready, meditating, ended
  int timeJoined;
  int timeStill;
  boolean isWinner = false;
  Vector history;
  Vector movement;
  int lastX;
  int lastY;
  int lastTime = 0;
  int stillTime = 5; // seconds
  int allowedMovement = 10; // 5 pixels
  int score = 0;
  int meditateX;
  int meditateY;
  PApplet processing;
  
  public User(PApplet processing, int id)
  {
    this.processing = processing;
    this.id = id;
    this.setWander();
    this.timeJoined = millis();
    this.movement = new Vector(5);
  }
  
  public void update(int x, int y)
  {
    int time = millis();
    int[] pos = {x, y, time};
    if (status != "meditating") {
      this.lastX = x;
      this.lastY = y;
      //if ((time - lastTime) < 1000) return;
      if (movement.size() == 5) movement.remove(0);
      movement.add(pos);
      if (isStill()) this.setReady();
    } else {
      if ((time - lastTime) < 100) return;
      history.add(pos);
    }
    this.lastTime = time;
  }
  
  public boolean isStill()
  {
    int size = movement.size();
    if (size < 2) return false;
    println("is it still");
    // check history to make sure x and ys within range
    int[] orig = (int[])movement.get(size-2);
    int[] pos = (int[])movement.get(size-1);
    println("X movement: "+abs(orig[0] - pos[0]));
    if (abs(orig[0] - pos[0]) > this.allowedMovement) {
      return false;
    }
    println("Y movement: "+abs(orig[1] - pos[1]));
    if (abs(orig[1] - pos[1]) > this.allowedMovement) {
      return false;
    }
    println(id+": "+abs(millis() - orig[2]) / 1000);
    if ((abs(millis() - orig[2]) / 1000) < stillTime) {
      return false;
    }
    return true;
  }
  
  public void setWander()
  {
    this.status = "wander";
  }
  
  public void setReady()
  {
    this.status = "ready";
  }
  
  public void setMeditating()
  {
    this.meditateX = this.lastX;
    this.meditateY = this.lastY;
    this.history = new Vector();
    this.status = "meditating";
  }
  
  public void setEnded()
  {
    if (this.status != "meditating") return;
    this.status = "ended";
    int score = 0;
    for (int i = 0; i < history.size(); i++) {
      int[] pos = (int[])history.get(i);
      int thisMovement = abs(this.meditateX - pos[0]);
      thisMovement += abs(this.meditateY - pos[1]);
      score += thisMovement;
    }
    this.score = score;
  }
  
  public boolean isReady()
  {
    return (this.status == "ready");
  }
  
  public int getScore() {
    return this.score;
  }
  
  public int getX()
  {
    return this.lastX;
  }
  
  public int getY()
  {
    return this.lastY;
  }
  
  public int getTime()
  {
    return this.lastTime;
  }
  
  public int getId()
  {
    return this.id;
  }
  
  public void setWinner()
  {
    this.isWinner = true;
  }
  
  public void drawUser() {
    if (isStill()) {
      processing.fill(0xffcccccc);
    } else {
      processing.fill(0xffffff00);
    }
    processing.ellipse(getX(), getY(), 80, 80);
    if (isWinner) {
      processing.fill(0xffffff00);
      processing.text("Winner!", getX() - 40, getY() - 50);
    } else {
      processing.text(score, getX() - 40, getY() - 50);
    }
  }
}

  static public void main(String args[]) {
    PApplet.main(new String[] { "--bgcolor=#FFFFFF", "meditate" });
  }
}
