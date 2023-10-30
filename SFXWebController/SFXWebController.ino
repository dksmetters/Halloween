#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

// SSID and Password to your ESP Access Point
const char* ssid = "add ssid";
const char* password = "addpassword";

const int led = 13;
String lastComm = "";

String contentType = "text/html";
 
String html_1 = R"=====(
<!DOCTYPE html>
<html>
<head>
  <meta name='viewport' content='width=device-width, initial-scale=1.0'/>
  <meta charset='utf-8'>
  <style>
    body     { font-size:120%;} 
    #main    { display: table; width: 300px; margin: auto;  padding: 10px 10px 10px 10px; border: 3px solid blue; border-radius: 10px; text-align:center;} 
    #BTN_LED { width:200px; height:40px; font-size: 110%;  }
    p        { font-size: 75%; }
  </style>
 
  <title>Websockets</title>
</head>
<body>
  <div id='main'>
    <h3>Sound Effects</h3>
    <div id='content'>
      <table width="100%" border="1">
        <tr>
          <td>&nbsp;</td>
          <td align="center" valign="middle">
          <label>
             <button id="s 1"  type="button" onclick="buttonClick(this);" class="button">1</button> 
          </label>
          </td>
          <td align="center" valign="middle">
          <label>
             <button id="s 2"  type="button" onclick="buttonClick(this);" class="button">2</button> 
          </label>
          </td>
          <td align="center" valign="middle">
          <label>
             <button id="s 3"  type="button" onclick="buttonClick(this);" class="button">3</button> 
          </label>
          </td>
          <td>&nbsp;</td>
        </tr>
        <tr>
         <td>&nbsp;</td>
          <td align="center" valign="middle">
          <label>
             <button id="s 4"  type="button" onclick="buttonClick(this);" class="button">4</button> 
          </label>
          </td>
          <td align="center" valign="middle">
          <label>
             <button id="s 5"  type="button" onclick="buttonClick(this);" class="button">5</button> 
          </label>
          </td>
          <td align="center" valign="middle">
          <label>
             <button id="s 6"  type="button" onclick="buttonClick(this);" class="button">6</button> 
          </label>
          </td>
          <td>&nbsp;</td>
        </tr>
       <tr>
         <td>&nbsp;</td>
          <td align="center" valign="middle">
          <label>
             <button id="s 7"  type="button" onclick="buttonClick(this);" class="button">7</button> 
          </label>
          </td>
          <td align="center" valign="middle">
          <label>
             <button id="s 8"  type="button" onclick="buttonClick(this);" class="button">8</button> 
          </label>
          </td>
          <td align="center" valign="middle">
          <label>
             <button id="s 9"  type="button" onclick="buttonClick(this);" class="button">9</button> 
          </label>
          </td>
          <td>&nbsp;</td>
        </tr>
        <tr>
         <td>&nbsp;</td>
         <td align="center" valign="middle">
              <input type="range" id="volume" onmouseup="setVolume(this);" 
                  ontouchend="setVolume(this);" name="volume" min="0" max="11">
              <label for="volume">Volume</label>
          </td>     
          <td>&nbsp;</td>
        </tr>
      </table>
    </div>
    <p>Recieved data = <span id='rd'>---</span> </p>
    <br />
   </div>
</body>
 
<script>
  var websock;
  function init() 
  {
    websock = new WebSocket('ws://' + window.location.hostname + ':81/');
    websock.onopen = function(evt) { console.log('websock open'); };
    websock.onclose = function(evt) { console.log('websock close'); };
    websock.onerror = function(evt) { console.log(evt); };
    websock.onmessage = function(event) { processReceivedCommand(event); };
  }

  function processReceivedCommand(evt) 
  {
    console.log("Recieved: " + evt);
    document.getElementById('rd').innerHTML = evt.data;
    if (evt.data ==='0') 
    {  
        document.getElementById('BTN_LED').innerHTML = 'Turn on the LED';  
        document.getElementById('LED_status').innerHTML = 'LED is off';  
    }
    if (evt.data ==='1') 
    {  
        document.getElementById('BTN_LED').innerHTML = 'Turn off the LED'; 
        document.getElementById('LED_status').innerHTML = 'LED is on';   
    }
  }

  function buttonClick(btn)
  {   
    var btnText = btn.textContent || btn.innerText;
    websock.send(btn.id);
  }
  function setVolume(slider)
  {   
    var volume = slider.valueAsNumber || DEFAULT_VOLUME;
    websock.send(btn.id + ' ' + volume);
  }
  window.onload = function(e)
  { 
    init();
  }
</script>
 
 
</html>
)=====";


WebSocketsServer webSocket = WebSocketsServer(81);
ESP8266WebServer server(80);


void setup() {
  Serial.begin(9600);
  pinMode(LED_BUILTIN, OUTPUT);  
  delay(500);
  Serial.println(F("Serial started at 9600"));
  Serial.println();
 
  // Connect to a WiFi network
  Serial.print(F("Connecting to "));  Serial.println(ssid);
  WiFi.begin(ssid,pass);
 
  // connection with timeout
  int count = 0; 
  while ( (WiFi.status() != WL_CONNECTED) && count < 17) 
  {
      Serial.print(".");  delay(500);  count++;
  }
 
  if (WiFi.status() != WL_CONNECTED)
  { 
     Serial.println("");  Serial.print("Failed to connect to ");  Serial.println(ssid);
     while(1);
  }
  Serial.println("Connected to the WiFi network");
  Serial.println("");
  Serial.println(F("[CONNECTED]"));
  Serial.print("[IP ");              
  Serial.print(WiFi.localIP()); 
  Serial.println("]");
 
  // start a server
  server.on("/", handleRoot);
  server.on("/play", HTTP_POST, handlePlay); 
  server.on("/volume", HTTP_POST, handlePost); 
  server.onNotFound(handleNotFound);
  
  server.begin();
  Serial.println("Server started");
  webSocket.begin();
  webSocket.onEvent(webSocketEvent);
  Serial.println("WebSocket started listening");
}

void loop() {
    digitalWrite(LED_BUILTIN, HIGH); 
    webSocket.loop();
    server.handleClient(); 
 
    delay(5);
}

void handleRoot() {
  server.send(200, contentType, html_1);   // Send HTTP status 200 (Ok) and send some text to the browser/client
}

void handleNotFound(){
  server.send(404, "text/plain", "404: Not found"); // Send HTTP status 404 (Not Found) when there's no handler for the URI in the request
}


void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length)
{
  // Use if/else instead of switch
  // If type is WStype_TEXT, this is a normal command from the web page or
  // other UI. Parse it and do what it says.
  if (type == WStype_TEXT)
  {
     Serial.printf("[%u] Handling payload %s!\r\n", num);
     handlePayload(payload, length);
  }
  else if (type == WStype_DISCONNECTED) 
  {
    Serial.printf("[%u] Disconnected!\r\n", num);
  }
  else if (type == WStype_CONNECTED) 
  {
    Serial.printf("[%u] Recconnected as %s!\r\n", webSocket.remoteIP(num));
  }
  else 
  {
    Serial.print("Unsupported WStype = ");
    Serial.println(type);  
    Serial.print("WS payload = ");
    for(int i = 0; i < length; i++) { Serial.print((char) payload[i]); }
    Serial.println();
  }
}

void handlePayload(uint8_t * payload, size_t length) {
  if (payload[0] == 's') {
    int track = atoi(&payload[2],length-2);
    playTrack(track);
  } else if (payload[0] == 'v') {
    int volume = atoi(&payload[2],length-2);
    setVolume(volume);
  }
}

void handlePlay(int track) {
  JsonObject& json = parseJson();
  Serial.print("Play payload = ");
  Serial.println(json);
}

void handleVolume(int volume) {
  JsonObject& json = parseJson();
  Serial.print("Volume payload = ");
  Serial.println(json);
}

JsonObject& parseJson() {
  StaticJsonBuffer<200> newBuffer;
  JsonObject& newjson = newBuffer.parseObject(server.arg("plain"));
  return newjson;
}

void playTrack(int track) {
}

void setVolume(int volume) {
}
}
