#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <WebSocketsServer.h>
#include <Hash.h>

//SSID and Password to your ESP Access Point
const char* ssid = "QuadrupedWifi";
const char* password = "12345678";

const int led = 13;
String lastComm = "";

static const char PROGMEM INDEX_HTML[] = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
<meta name = "viewport" content = "width = device-width, initial-scale = 1.0, maximum-scale = 1.0, user-scalable=0">
<title>ESP8266 Spider Robot</title>
<style>
"body { background-color: #808080; font-family: Arial, Helvetica, Sans-Serif; Color: #000000; }"
#JD {
  text-align: center;
}
#JD {
  text-align: center;
  font-family: "Lucida Sans Unicode", "Lucida Grande", sans-serif;
  font-size: 24px;
}
.foot {
  text-align: center;
  font-family: "Comic Sans MS", cursive;
  font-size: 9px;
  color: #F00;
}
.button {
    border: none;
    color: white;
    padding: 20px;
    text-align: center;
    text-decoration: none;
    display: inline-block;
    font-size: 16px;
    margin: 4px 2px;
    cursor: pointer;
    border-radius: 12px;
  width: 100%;
}
.red {background-color: #F00;}
.green {background-color: #090;}
.yellow {background-color:#F90;}
.blue {background-color:#03C;}
</style>
<script>
var websock;
function start() {
  websock = new WebSocket('ws://' + window.location.hostname + ':81/');
  websock.onopen = function(evt) { console.log('websock open'); };
  websock.onclose = function(evt) { console.log('websock close'); };
  websock.onerror = function(evt) { console.log(evt); };
  websock.onmessage = function(evt) {
    console.log(evt);
    var e = document.getElementById('ledstatus');
    if (evt.data === 'ledon') {
      e.style.color = 'red';
    }
    else if (evt.data === 'ledoff') {
      e.style.color = 'black';
    }
    else {
      console.log('unknown event');
    }
  };
}
function buttonclick(e) {
  websock.send(e.id);
}
</script>
</head>
<body onload="javascript:start();">
&nbsp;
<table width="100%" border="1">
  <tr>
    <td bgcolor="#FFFF33" id="JD">Quadruped Controller</td>
  </tr>
</table>
<table width="100" height="249" border="0" align="center">
  <tr>
    <td>&nbsp;</td>
    <td align="center" valign="middle"><form name="form1" method="post" action="">
      <label>
        <button id="w 1 1"  type="button" onclick="buttonclick(this);" class="button green">Forward</button> 
      </label>
    </form></td>
    <td>&nbsp;</td>
  </tr>
  <tr>
    <td align="center" valign="middle"><form name="form1" method="post" action="">
      <label>
        <button id="w 3 1"  type="button" onclick="buttonclick(this);" class="button green">Turn_Left</button> 
      </label>
    </form></td>
    <td align="center" valign="middle"><form name="form1" method="post" action="">
      <label>
        <button id="w 0 1"  type="button" onclick="buttonclick(this);" class="button red">Stop_all</button> 
      </label>
    </form></td>
    <td align="center" valign="middle"><form name="form1" method="post" action="">
      <label>
        <button id="w 4 1"  type="button" onclick="buttonclick(this);" class="button green">Turn_Right</button> 
      </label>
    </form></td>
  </tr>
  <tr>
    <td>&nbsp;</td>
    <td align="center" valign="middle"><form name="form1" method="post" action="">
      <label>
        <button id="w 2 1"  type="button" onclick="buttonclick(this);" class="button green">Backward</button> 
      </label>
    </form></td>
    <td>&nbsp;</td>
  </tr>
  <tr>
        <td align="center" valign="middle"><form name="form1" method="post" action="">
      <label>
        <button id="w 5 3"  type="button" onclick="buttonclick(this);" class="button yellow">Shake </button> 
      </label>
    </form></td>
        <td align="center" valign="middle"><form name="form1" method="post" action="">
      <label>
        <button id="w 8 5"  type="button" onclick="buttonclick(this);" class="button blue">Head_up</button> 
      </label>
    </form></td>
        <td align="center" valign="middle"><form name="form1" method="post" action="">
      <label>
        <button id="w 6 3"  type="button" onclick="buttonclick(this);" class="button yellow">Wave</button> 
      </label>
    </form></td>
  </tr>
  <tr>
        <td align="center" valign="middle"><form name="form1" method="post" action="">
      <label>
        <button id="w 16"  type="button" onclick="buttonclick(this);" class="button blue">Twist_Left</button> 
      </label>
    </form></td>
        <td align="center" valign="middle"><form name="form1" method="post" action="">
      <label>
        <button id="w 9 5"  type="button" onclick="buttonclick(this);" class="button blue">Head_down</button> 
      </label>
    </form></td>
        <td align="center" valign="middle"><form name="form1" method="post" action="">
      <label>
        <button id="w 17"  type="button" onclick="buttonclick(this);" class="button blue">Twist_Right</button> 
      </label>
    </form></td>
  </tr>
  <tr>
        <td align="center" valign="middle"><form name="form1" method="post" action="">
      <label>
        <button id="w 11 5"  type="button" onclick="buttonclick(this);" class="button blue">Body_left</button> 
      </label>
    </form></td>
        <td align="center" valign="middle"><form name="form1" method="post" action="">
      <label>
        <button id="w 13"  type="button" onclick="buttonclick(this);" class="button blue">Body_higher</button> 
      </label>
    </form></td>
        <td align="center" valign="middle"><form name="form1" method="post" action="">
      <label>
        <button id="w 10 5"  type="button" onclick="buttonclick(this);" class="button blue">Body_right</button>
      </label>
    </form></td>
  </tr>

  <tr>
        <td align="center" valign="middle"><form name="form1" method="post" action="">
      <label>
         <button id="w 12"  type="button" onclick="buttonclick(this);" class="button yellow">Service</button> 
      </label>
    </form></td>
        <td align="center" valign="middle"><form name="form1" method="post" action="">
      <label>
        <button id="w 14"  type="button" onclick="buttonclick(this);" class="button blue">Body_lower</button> 
      </label>
    </form></td>
        <td align="center" valign="middle"><form name="form1" method="post" action="">
      <label>
        <button id="w 15"  type="button" onclick="buttonclick(this);" class="button yellow">Reset_Pose</button> 
      </label>
    </form></td>
  </tr>
  
    <tr>
        <td align="center" valign="middle"><form name="form1" method="post" action="">
      <label>
        <button id="w 0 0"  type="button" onclick="buttonclick(this);" class="button yellow">Sit</button> 
      </label>
    </form></td>
      <td align="center" valign="middle"><form name="form1" method="post" action="">
&nbsp;
    </form></td>
        <td align="center" valign="middle"><form name="form1" method="post" action="">
      <label>
        <button id="w 7 1 "  type="button" onclick="buttonclick(this);" class="button yellow">Dance</button> 
      </label>
    </form></td>
  </tr>
  
</table>
<p class="foot">this application requires Mwilmar Quadruped platform.</p>
</body>
</html>
)rawliteral";

WebSocketsServer webSocket = WebSocketsServer(81);
ESP8266WebServer server(80);


void setup() {
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT); 
  for(uint8_t t = 4; t > 0; t--) {
    Serial.flush();
    delay(1000);
  }
  delay(1000);
  Serial.println (">> Setup");
  WiFi.mode(WIFI_AP);           //Only Access point
  WiFi.softAP(ssid, password);  //Start HOTspot removing password will disable security
 
  IPAddress myIP = WiFi.softAPIP(); //Get IP address
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");   
  Serial.println(myIP);

  server.on("/", [](){
  server.send(200, "text/html", INDEX_HTML);
  });
  
  server.begin();
  webSocket.begin();
  webSocket.onEvent(webSocketEvent);

}

void loop() {
   digitalWrite(LED_BUILTIN, HIGH); 
   if (Serial.available() > 0){
      char c[] = {(char)Serial.read()};
      webSocket.broadcastTXT(c, sizeof(c));
   }
   webSocket.loop();
   server.handleClient();
  
}

void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length)
{
  switch(type) {
    case WStype_DISCONNECTED:
      //Serial.printf("[%u] Disconnected!\r\n", num);
      break;
    case WStype_CONNECTED:
      {
        IPAddress ip = webSocket.remoteIP(num);
      }
      break;
    case WStype_TEXT:
      Serial.printf("%s\r\n", payload);
      digitalWrite(LED_BUILTIN, LOW);
      // send data to all connected clients
      webSocket.broadcastTXT(payload, length);
      break;
    case WStype_BIN:
      hexdump(payload, length);
      // echo data back to browser
      webSocket.sendBIN(num, payload, length);
      break;
      default:
      break;
  }
}
