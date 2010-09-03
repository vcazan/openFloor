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

void setup()
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
  fill(#000000);
}

void draw()
{
  background(#ffffff);
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

void drawAll() {
  reader.drawAll();
}

void updateStatus()
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

void drawWaiting() {
  fill(#000000);
  text("Waiting for meditators", 30, 40);
  text(reader.countStill()+"/"+numUsers+" ready to start", 30, 70);
  fill(#cccccc);
  ellipse(150, 330, 100, 100);
  ellipse(400, 330, 100, 100);
  ellipse(650, 330, 100, 100);
  //ellipse(250, 420, 60, 60);
}

void drawCountdown() {
  fill(#000000);
  text("Ready to start in", 30, 40);
  int secondsLeft = 3 - ((millis() - readyTime) / 1000);
  text(secondsLeft, 30, 70);
}

void drawRunning() {
  fill(#ff0000);
  text("!!! MEDITATE !!!", 30, 40);
  int secondsLeft = 60 - ((millis() - readyTime) / 1000);
  text(secondsLeft, 30, 70);
}

void drawComplete() {
  fill(#000000);
  text("You are happy now! :)", 30, 40);
  
}
