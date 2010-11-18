import java.util.Vector;

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
  
  public void setStill()
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
    return 800-this.lastY;
  }
  
  public int getTime()
  {
    return this.lastTime;
  }
  
  public int getId()
  {
    return this.id;
  }
  
  public void drawUser() {
    if (isStill()) {
      processing.fill(#cccccc);
    } else {
      processing.fill(#ffff00);
    }
    processing.ellipse(getX(), getY(), 200, 200);
    processing.text(getId() + " " + getY(), getX() - 40, getY() - 50);
    }
  }

