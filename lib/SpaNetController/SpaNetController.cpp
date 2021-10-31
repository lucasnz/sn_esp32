#include "SpaNetController.h"

float SpaNetController::getAmps(){
  return this->amps;
}

int SpaNetController::getVolts(){
  return this->volts;
}

SpaNetController::SpaNetController() {
    Serial2.begin(38400,SERIAL_8N1, 16, 17);
    Serial2.setTimeout(500); 
}

SpaNetController::~SpaNetController() {}


bool SpaNetController::parseStatus(String str) {

  int element=1;
  int commaIndex=str.indexOf(',');
  int elementBoundaries[290];

  elementBoundaries[0]=0;
  
  while (commaIndex>-1){
    elementBoundaries[element] = commaIndex;
    element++;
    commaIndex=str.indexOf(',',commaIndex+1);
  }

  elementBoundaries[element] = str.length();

  if (element != 290) { 
    debugW("Wrong number of parameters read, read %d, expecting 289\r\n", element-1);
    return false;
  }
  else {
    debugI("Successful read of SpaNet status");
    amps = float(str.substring(elementBoundaries[2]+1,elementBoundaries[3]).toInt())/10;
    volts = str.substring(elementBoundaries[3]+1,elementBoundaries[4]).toInt();
    return true;
  }
}

String SpaNetController::sendCommand(String cmd) {
  Serial2.printf("\n");
  delay(100);
  Serial2.print(cmd+"\n");
  delay(100);
  return Serial2.readString();
}

bool SpaNetController::pollStatus(){
  if (parseStatus(sendCommand("RF"))) {
    if (update) {update();}
    return true;
  } else {
    return false;
  }
}

void SpaNetController::tick(){
  if (millis()>_nextUpdate) {
    if (pollStatus()) {
      _nextUpdate = millis() + UPDATEFREQUENCY;
    } else {
      _nextUpdate = millis() + 1000;
    }
  }
}

void SpaNetController::subscribeUpdate(void (*u)()){
  this->update=u;
}