#include <Process.h>


#define SERVER "http://YOUR-SERVER-IP-HERE:5000"


#define LOGIN_ENDPOINT  "/login"
#define STATUS_ENDPOINT  "/login"

const int numChars = 64;
char receivedChars[numChars];

boolean authenticate(unsigned long uuid) {

  /*
  switch(uuid) {
    case 1148012432 : // Card 1: 0x44 0x6D 0x47 0x90
    case 2715109411 : // Card 2: 0xA1 0xD5 0x48 0x23
    {
      return true;  
    }
    case 2704814351 : // Card 3: 0xA1 0x38 0x31 0x0F
    default : {
      return false;  
    }
  }
  */  
  // manually execute curl
  // see: http://forum.arduino.cc/index.php?topic=263742.0
  Process p;              
  p.begin("curl");      
  p.addParameter("-s");   // supress errors -> only return data
  p.addParameter("-m");   // timeout after 1 second and return nothing
  p.addParameter("1"); 
  p.addParameter("-d");
  p.addParameter("uuid="+String(uuid)); 
  p.addParameter(String(SERVER) + String(LOGIN_ENDPOINT)); 
  p.run();

  // Read upto numChars of the Response Body
  int ndx = 0;
  const int numChars = 16;
  char responseBody[numChars];
  while(p.available() > 0) {
    responseBody[ndx] = p.read();
    ndx++;
    if (ndx >= numChars) {
      ndx = numChars - 1;  
    }
  }
  responseBody[ndx] = '\0';
//TODO: must mimify JSON object. bug waiting to happen
  if(String(responseBody).startsWith("{\"access\":\"1\"}")) {
  //if(String(responseBody).startsWith("{\n  \"access\": 1\n}")) {
    return true;  
  } else {
    return false;  
  }  

}

boolean serverStatus() {
  
  Process p;
  String cmd = "curl -s -m 1 " + String(SERVER) + String(STATUS_ENDPOINT);
  //Serial.println(cmd);
  p.runShellCommand(cmd);

  
  // Read upto numChars of the Response Body
  int ndx = 0;
  const int numChars = 16;
  char responseBody[numChars];
  while(p.available() > 0) {
    responseBody[ndx] = p.read();
    ndx++;
    if (ndx >= numChars) {
      ndx = numChars - 1;  
    }
  }
  responseBody[ndx] = '\0';
  //Serial.println(responseBody);

//TODO: must mimify JSON object. bug waiting to happen
  if(String(responseBody).startsWith("{\"status\":\"1\"}")) {
  //if(String(responseBody).startsWith("{\n  \"status\": 1\n}")) {
    return true;  
  } else {
    return false;  
  }    
}
