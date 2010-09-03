 PositionReader reader;

int offset = 10;
Hashtable users = new Hashtable();
PFont font;
int numUsers = 2;

void setup()
{
  size(1200, 800);
  //size(320, 240);
  frameRate(30);
  background(255);
  smooth();
  strokeWeight(2);
  reader = new PositionReader(this, "tcp");
  reader.setDaemon(true);
  reader.start();
  smooth();
  font = loadFont("LucidaSans-30.vlw"); 
  textFont(font, 30);
  fill(#000000);
}

void draw()
{
  background(#ffffff);
  drawAll();

}

void drawAll() {
  reader.drawAll();
}


