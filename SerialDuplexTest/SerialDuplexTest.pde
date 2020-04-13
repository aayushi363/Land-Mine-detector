// Serial Terminal for Arduino/Bluetooth/etc
// Use: Arrow keys: Up/Down/Left/Right for history

import processing.serial.*;

Serial myPort;      // The serial port

final int displayLinesRx = 28;
final int displayLinesTx =  5;

String inToComputer[]    = new  String[displayLinesRx];
String outFromComputer[] = new  String[displayLinesTx];

String tempStringRx = "";
String tempStringTx = "";
String saveTempStringTx = "";

int cqNextIndexRx = 0;
int cqNextIndexTx = 0;

String txHistoryPointerSymbol = "=>";
int    txHistoryPointerSymbolWidthINITIAL = 20;
int    txHistoryPointerSymbolWidth; 
int    txHistoryPointerIndex= 0;
int    txHistorySelectOptions[] = new int[displayLinesTx];

int windowMarginLeft;

int i, j;
int textFontHeightInPixel = 20;
int lineGap = textFontHeightInPixel;
int printableMessageLength = 80;
int baseTxWindow;
char charRx, charTx;
int countRx=0;
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void settings() {   
   int temp, temp2;
   if(printableMessageLength < 80)
     temp =80;
     else
     temp = printableMessageLength;
   int w = temp * 12 + txHistoryPointerSymbolWidthINITIAL + 60;
   
      if(displayLinesRx < 20)
       temp =20;
     else
       temp = displayLinesRx;
     
      if(displayLinesTx < 5)
         temp2 =5;
       else
         temp2 = displayLinesTx;
   
   int h = (textFontHeightInPixel +2) * (temp + temp2 + 2);
   size(w, h);
   //fullScreen();
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void setup() {
  frameRate(10);
  txHistoryPointerSymbolWidth = round(textWidth(txHistoryPointerSymbol));
  windowMarginLeft = 20 + txHistoryPointerSymbolWidth;
  
  printArray(Serial.list()); // List all available Serial Ports to console
  myPort = new Serial(this, "COM31", 9600);
  // myPort = new Serial(this, "COM25", 38400);// V1 BT 8IR
    //myPort = new Serial(this, "COM30", 115200);
    
  
  PFont mono;
  mono = loadFont("LucidaConsole-48.vlw");
  textFont(mono);
  textSize(textFontHeightInPixel); 

  for(int i=0; i<displayLinesRx; i++)  inToComputer[i]      =  "";
  for(int i=0; i<displayLinesTx; i++)  outFromComputer[i]   =  "";  
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void draw() {
  background(0);
  
  // Display Received data
  j = 0;  
  fill(255, 255, 0);
  for(i=cqNextIndexRx; i<displayLinesRx; i++) {
    text(inToComputer[i], windowMarginLeft, (j * lineGap) + lineGap );
    j++;
  }
  
  for(i=0; i<cqNextIndexRx; i++) {      
    text(inToComputer[i], windowMarginLeft, (j * lineGap) + lineGap );
    j++;
  }
  
  text(tempStringRx, windowMarginLeft, (j * lineGap) + lineGap );
  
  // Display HELP
  fill(100, 200, 100);
  text("Send to Serial\\Arduino\\BT: " , 10, (j * lineGap) + 2*lineGap +5);
  
  fill(128, 255, 128);
  text(tempStringTx, 10 + textWidth("Send to Serial\\Arduino\\BT: "), (j * lineGap) + 2*lineGap +5);
  
  fill(100, 200, 100);
  text("Type something and press enter/return key to send.", 10, (j * lineGap) + 3*lineGap +5);
  //text("DEBUG:  cqNextIndexTx: " + cqNextIndexTx + " txHistoryPointerIndex: " + str(txHistoryPointerIndex) , 10, (displayLinesRx * lineGap) + 2*lineGap +5);
  
  fill(100, 200, 200);  
  text("Tx history (use up/down/left/right key to use history) :", 10,  (j * lineGap) + 4*lineGap +5);
  
  // Display Tx data:
  baseTxWindow =  (j * lineGap) + 3*lineGap  + 5 +textFontHeightInPixel;
  
  j = 0;  
  fill(128, 255, 255);  
  for(i=cqNextIndexTx-1; i>=0; i--) {      
    text(outFromComputer[i], windowMarginLeft, baseTxWindow + (j * lineGap) + lineGap );
    txHistorySelectOptions[j] = i;
    j++;
  }
  for(i=displayLinesTx-1; i>=cqNextIndexTx; i--) {
    text(outFromComputer[i], windowMarginLeft, baseTxWindow + (j * lineGap) + lineGap );
    txHistorySelectOptions[j] = i;
    j++;
  }
  
  // Display: History Pointer Symbol   
    fill(100, 200, 200);  
    text(txHistoryPointerSymbol, 2, baseTxWindow + (txHistoryPointerIndex * lineGap) + lineGap );
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
boolean isAsciiPrintable(char ch) {
      return ch >= 32 && ch < 127;
  }
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  //noFill();
  //stroke(204, 102, 0);
  //rect(4, 4, width - 8, (displayLines * lineGap) - 8, 7);
  //stroke(255, 255, 255);
  //rect(4,(displayLines * lineGap) - 8, width - 8, (displayLines * lineGap) - 8, 7);
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void serialEvent(Serial myPort) {  
  
  charRx =  myPort.readChar();
    
  if(charRx=='\n' || charRx=='\r')
   { 
     inToComputer[cqNextIndexRx] = tempStringRx;
     tempStringRx = "";
          
     countRx = 0;
     cqNextIndexRx++;
     if(cqNextIndexRx >= displayLinesRx) 
       cqNextIndexRx = 0;    
   }
  else   
   {
     if(countRx >= printableMessageLength)
     {
       inToComputer[cqNextIndexRx] = tempStringRx;
       tempStringRx = "";       
     
       countRx = 0;
       cqNextIndexRx++;
       if(cqNextIndexRx >= displayLinesRx) 
          cqNextIndexRx = 0;          
     }
     tempStringRx = tempStringRx +charRx;
     countRx++;
   }
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  void keyPressed() {
    // CHECk if it is a arrow key
   if (key == CODED) {
      if (keyCode == UP ||  keyCode == DOWN) {
            if (keyCode == UP) {
                 txHistoryPointerIndex--;
                 if(txHistoryPointerIndex <0) txHistoryPointerIndex=0;
                 
              } else if (keyCode == DOWN) { /////////////////////////////////////////////////////
                  // DOWN
                    txHistoryPointerIndex++;
                    if(txHistoryPointerIndex >= displayLinesTx) txHistoryPointerIndex=displayLinesTx-1;              
              }
              //saveTempStringTx = tempStringTx;
              tempStringTx = outFromComputer[txHistorySelectOptions[txHistoryPointerIndex]];
              return; 
            }
            
        if (keyCode == RIGHT)
        {          
          tempStringTx = outFromComputer[txHistorySelectOptions[txHistoryPointerIndex]];
          return;
        }
        
        if (keyCode == LEFT)
        {
          tempStringTx  = saveTempStringTx;
          return;
        }        
        return;
    } 
  
    
    // CHECk if it is a Backspace key
    if(key == '\b') { // Backspace delets one char 
    
      if(tempStringTx.length()-1 > 0) // Index must be > 0
          tempStringTx = tempStringTx.substring(0, tempStringTx.length()-1); 
        else
          tempStringTx = "";
        
      saveTempStringTx = tempStringTx;
      return;
    }
    
    // CHECk if it is a enter/return key: If user presses enter/return then send to Serial/Arduino/BlueTooth
    if(key == '\r' || key == '\n') {             
      // If user is just/only pressing enter/return without typing (other intresting ASCII keys)
      if(tempStringTx.length()==0 || tempStringTx==null || tempStringTx=="") 
          //myPort.write(key);
          ; // Do Notthing
      else {
            myPort.write(tempStringTx); // Transmit            
            outFromComputer[cqNextIndexTx]= tempStringTx; // Log
            cqNextIndexTx++;      
            if(cqNextIndexTx == displayLinesTx) cqNextIndexTx = 0;             
           }
      tempStringTx = ""; // Re-initialize
      saveTempStringTx = tempStringTx;
      return;
    }
    
    // If the ky is none of the above then:-
    // Keep on saving everything that user types
    if(isAsciiPrintable(key))
    {
      tempStringTx = tempStringTx + key;
      saveTempStringTx = tempStringTx;
    }
  }
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////