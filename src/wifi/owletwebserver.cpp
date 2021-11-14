#include "owletwebserver.h"
#include "wifi/wifimanager.h"
#include "ota/otamanager.h"
#include "gpiocontroller.h"
#include "debugutils.h"

const char css[] PROGMEM = R"rawliteral(
*,:after,:before{-moz-box-sizing:border-box;-webkit-box-sizing:border-box;box-sizing:border-box}html{height:100%;font-family:Helvetica;display:inline-block;margin:0 auto;text-align:left;background-size:cover;background-repeat:no-repeat;background-attachment:fixed;background-image:linear-gradient(135deg,#9ec7a3,#8bc1b6)}body{font-family:Ubuntu,Roboto,Helvetica,Arial,sans-serif;color:#384047}form{max-width:300px;margin:10px auto;padding:10px 20px;background:#fff;border-radius:8px}h1,h2{margin:0 0 30px 0;text-align:center}.main-form{margin:40px auto;padding-bottom:40px}.container{margin-top:30px;margin-bottom:30px;width:100%}.input{color:#48ab54}.output{color:#ab5348}ul{list-style-type:none;margin:0;padding:0}li:nth-child(even){background-color:rgba(0,0,0,.05)}input[type=date],input[type=datetime],input[type=email],input[type=number],input[type=password],input[type=search],input[type=tel],input[type=text],input[type=time],input[type=url],select,textarea{background:rgba(255,255,255,.1);border:none;font-size:16px;height:auto;margin:0;outline:0;padding:15px;width:100%;background-color:#e8eeef;color:#8a97a0;box-shadow:0 1px 0 rgba(0,0,0,.03) inset;margin-bottom:30px}input[type=checkbox],input[type=radio]{margin:0 4px 8px 0}input[type=radio]{-ms-transform:scale(1.5);-webkit-transform:scale(1.5);transform:scale(1.5)}select{padding:6px;height:32px;border-radius:2px}button{padding:19px 39px 18px 39px;color:#262626;background-color:#fff;font-size:18px;text-align:center;cursor:pointer;font-style:normal;border-radius:5px;width:100%;border:1px solid #262626;border-width:1px 1px 3px;box-shadow:0 -1px 0 rgba(255,255,255,.1) inset;margin-bottom:10px;max-width:350px}.cta-confirm{background-color:#48ab53;box-shadow:0 -1px 0 #245528 inset;border:1px solid #245528;color:#fff}.cta-danger{background-color:#ab5348;box-shadow:0 -1px 0 #753931 inset;border:1px solid #753931;color:#fff}fieldset{margin-bottom:30px;border:none}legend{font-size:1.4em;margin-bottom:10px}label{display:block;margin-bottom:8px}label.light{font-weight:300;display:inline;font-size:1.2em;vertical-align:middle}.width-1-2{box-sizing:border-box;width:49%;display:inline-block}.match-align{display:flex;flex-wrap:wrap;align-items:stretch;flex:1}.auto-margin{margin:10px}.auto-center{display:flex;justify-content:center;align-items:center}.number{background-color:#1a1a1a;color:#fff;height:30px;width:30px;display:inline-block;font-size:.8em;margin-right:4px;line-height:30px;text-align:center;text-shadow:0 1px 0 rgba(255,255,255,.2);border-radius:100%}@media screen and (min-width:480px){form{max-width:480px}}p{display:inline-block}.checkbox{position:relative;display:inline-block}.checkbox:after,.checkbox:before{font-family:FontAwesome;font-feature-settings:normal;-webkit-font-kerning:auto;font-kerning:auto;font-language-override:normal;font-stretch:normal;font-style:normal;font-synthesis:weight style;font-variant:normal;font-weight:400;text-rendering:auto}.checkbox label{width:90px;height:42px;background:#ccc;position:relative;display:inline-block;border-radius:46px;transition:.4s}.checkbox label:after{content:'';position:absolute;width:50px;height:50px;border-radius:100%;left:0;top:-5px;z-index:2;background:#fff;box-shadow:0 0 5px rgba(0,0,0,.2);transition:.4s}.checkbox input{position:absolute;left:0;top:0;width:100%;height:100%;z-index:5;opacity:0;cursor:pointer}.checkbox input:hover+label:after{box-shadow:0 2px 15px 0 rgba(0,0,0,.2),0 3px 8px 0 rgba(0,0,0,.15)}.checkbox input:checked+label:after{left:40px}.gpio-toggle-container{display:flex}.gpio-toggle-container .checkbox:after{content:'';color:#aaa;position:relative;right:30px;bottom:15px}.gpio-toggle-container .checkbox:before{content:'';position:relative;left:35px;bottom:15px;color:#fff;z-index:1}.gpio-toggle-container .checkbox label{width:80px;background:green;box-shadow:0 0 1px 2px rgba(0,0,0,.15)}.gpio-toggle-container .checkbox label:after{top:0;width:42px;height:42px}.gpio-toggle-container .checkbox input:checked+label{background:#ab5348}.gpio-toggle-container .checkbox input:checked+label:after{left:40px}
)rawliteral";

const char indexHtml[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
<link rel="stylesheet" href="style.css"></head>
<body>
  <h1>nymea owlet</h1>
  <h2>Configuration</h>

    %WIFICONFIGSECTION%

    %OTASECTION%

    %GPIOCONTROLSECTION%



</body>
</html>
)rawliteral";

const char wifiSection[] PROGMEM = R"rawliteral(
<h3 class="auto-center"><span class="number">1</span> WiFi</h3>
<form action='/config' method='GET'>
  <div class="container">
    <div class="width-1-2"><input type="radio" id="ap" value="ap" name="wifi_mode" {{checkedAP}}><label for="ap" class="light">Host AP</label></div>

    <div class="width-1-2"><input type="radio" id="client" value="client" name="wifi_mode" {{checkedClient}}><label for="client" class="light">Client</label> </div>
  </div>

  <div><label for='ssid'>SSID</label><input name='ssid' type="text" id='ssid' value='{{SSID}}'></div>
  <div><label for='password'>Password</label><input name='password' id='password' type="password" value=''></div>
  <div class="auto-center"><button>Connect</button></div>
</form>

)rawliteral";

const char otaSection[] PROGMEM = R"rawliteral(
<form action='/ota' method='GET'>
  <div class="container"><label for='ota-url'>OTA Update URL</label><input name='ota-url' id='ota-url' type="url" value=''>
    <div class="auto-center"><button>Upgrade</button></div>

  </div>
</form>
)rawliteral";

const char gpioSection[] PROGMEM = R"rawliteral(
    <form action='/gpio' method='GET'>
      <div class="container">
        <ul>
          {{buttons}}
        </ul>
      </div>
      <div class="auto-center"><button class="cta-confirm">Apply</button></div>
      <div class="auto-center"><button class="cta-danger">Cancel</button></div>
    </form>

    <script>function toggleGPIOCheckBox(element) {
      var xhr = new XMLHttpRequest();
      if(element.checked){ xhr.open("GET", "/update?output="+element.id+"&state=1", true); }
      else { xhr.open("GET", "/update?output="+element.id+"&state=0", true); }
            xhr.send();
    }
    </script>

)rawliteral";

const char gpioTemplate[] PROGMEM = R"rawliteral(
          <li>
            <section class="gpio-toggle-container">
              <div class="width-1-2 match-align auto-center">
                <h3 class="">GPIO {{number}}</h3>
              </div>
              <div class="width-1-2 match-align">
                <p class="auto-center auto-margin input">Input</p>
                <div class="checkbox">
                  <input type="checkbox" id="{{number}}" onchange="toggleGPIOCheckBox(this)" />
                  <label></label>
                </div>
                <p class="auto-center auto-margin output">Output</p>
              </div>
            </section>
          </li>
)rawliteral";

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <title>ESP Web Server</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <link rel="icon" href="data:,">
  <style>
    html {font-family: Arial; display: inline-block; text-align: center;}
    h2 {font-size: 3.0rem;}
    p {font-size: 3.0rem;}
    body {max-width: 600px; margin:0px auto; padding-bottom: 25px;}
    .switch {position: relative; display: inline-block; width: 120px; height: 68px}
    .switch input {display: none}
    .slider {position: absolute; top: 0; left: 0; right: 0; bottom: 0; background-color: #ccc; border-radius: 6px}
    .slider:before {position: absolute; content: ""; height: 52px; width: 52px; left: 8px; bottom: 8px; background-color: #fff; -webkit-transition: .4s; transition: .4s; border-radius: 3px}
    input:checked+.slider {background-color: #b30000}
    input:checked+.slider:before {-webkit-transform: translateX(52px); -ms-transform: translateX(52px); transform: translateX(52px)}
  </style>
</head>
<body>
  <h2>nymea owlet</h2>
  <h2>Configuration</h>
  <h3>WiFi</h3>
    <form action='/config' method='GET'>
      <div><input type='radio' id='ap' name='wifi_mode' value='ap'><label for='ap'>Host AP</label></div>
      <div><input type='radio' id='client' name='wifi_mode' value='client'><label for='client'>Client</label></div>
      <div><label for='ssid'>SSID</label><input name='ssid' id='ssid' value=''></div>
      <div><label for='password'>Password</label><input name='password' id='password' value=''></div>
      <div><button>Apply</button></div>
    </form>
  %BUTTONPLACEHOLDER%
  <form action="/ota" method="GET">
    <div><input name="otaurl" id="otaurl" value='http://10.10.10.236:8081/nymea-owlet.ino.bin'></div>
    <div><button>Upgrade</button></div>
  </form>
<script>function toggleCheckbox(element) {
  var xhr = new XMLHttpRequest();
  if(element.checked){ xhr.open("GET", "/update?output="+element.id+"&state=1", true); }
  else { xhr.open("GET", "/update?output="+element.id+"&state=0", true); }
        xhr.send();
}
</script>
</body>
</html>
)rawliteral";


OwletWebServer::OwletWebServer(int port):
    m_webServer(AsyncWebServer(port))
{

    m_webServer.on("/", HTTP_GET, [this](AsyncWebServerRequest *request) {
        DebugPrintln("Webserver request. Serving welcome page.");
        request->send_P(200, "text/html", indexHtml, [this](const String &var){
            return processor(var);
        });
    });
    m_webServer.on("/style.css", HTTP_GET, [this](AsyncWebServerRequest *request) {
        DebugPrintln("style request");
        request->send(200, "text/css", css);
    });

    m_webServer.on("/config", HTTP_GET, [this](AsyncWebServerRequest *request) {
        DebugPrintln("Config handler!!!!!!");
        if (request->hasParam("wifi_mode") && request->hasParam("ssid") && request->hasParam("password")) {
            bool hostAP = request->getParam("wifi_mode")->value() == "ap";
            String ssid = request->getParam("ssid")->value();
            String pass = request->getParam("password")->value();
            m_wifiManager->configure(hostAP, ssid, pass, true);

            request->send_P(200, "text/html", indexHtml, [this](const String &var){
                return processor(var);
            });
        } else {
            request->send(501);
        }
    });
    m_webServer.on("/update", HTTP_GET, [this] (AsyncWebServerRequest *request) {
        String inputMessage1;
        String inputMessage2;
        // GET input1 value on <ESP_IP>/update?output=<inputMessage1>&state=<inputMessage2>
        if (request->hasParam("output") && request->hasParam("state")) {
            inputMessage1 = request->getParam("output")->value();
            inputMessage2 = request->getParam("state")->value();
            digitalWrite(inputMessage1.toInt(), inputMessage2.toInt());
        }
        else {
            inputMessage1 = "No message sent";
            inputMessage2 = "No message sent";
        }
        DebugPrint("GPIO: ");
        DebugPrint(inputMessage1);
        DebugPrint(" - Set to: ");
        DebugPrintln(inputMessage2);
        request->send(200, "text/plain", "OK");

        m_gpioController->setGPIOPower(inputMessage1.toInt(), inputMessage2.toInt());
    });
    m_webServer.on("/ota", HTTP_GET, [this](AsyncWebServerRequest *request) {
        if (!request->hasParam("ota-url")) {
            DebugPrintln("Invalid OTA request. Update url missing.");
            request->send(400, "text/plain", "ota-url parameter missing.");
            return;
        }
        String otaUrl = request->getParam("ota-url")->value();
        DebugPrint("OTA update request: " + otaUrl);
        request->send(200, "text/plain", "OK");
        m_otaManager->update(otaUrl);
    });
}

OwletWebServer::~OwletWebServer() {
}

void OwletWebServer::begin()
{
    m_webServer.begin();
    DebugPrintln("Webserver started");
}

void OwletWebServer::enableGPIOController(GPIOController *gpioController)
{
    m_gpioController = gpioController;
}

void OwletWebServer::enableWiFiManager(WiFiManager *wifiManager)
{
    m_wifiManager = wifiManager;
}

void OwletWebServer::enableOTA(OTAManager *otaManager)
{
    m_otaManager = otaManager;
}

String OwletWebServer::processor(const String& var){
    //DebugPrintln(var);
    if(var == "BUTTONPLACEHOLDER"){
        String buttons = "";
        //    buttons += "<h4>Output - GPIO 2</h4><label class=\"switch\"><input type=\"checkbox\" onchange=\"toggleCheckbox(this)\" id=\"4\" " + outputState(4) + "><span class=\"slider\"></span></label>";
        //    buttons += "<h4>Output - GPIO 4</h4><label class=\"switch\"><input type=\"checkbox\" onchange=\"toggleCheckbox(this)\" id=\"5\" " + outputState(5) + "><span class=\"slider\"></span></label>";
        return buttons;
    }


    if (var == "WIFICONFIGSECTION") {
        if (m_wifiManager) {
            String ret(wifiSection);
            WiFiManager::WiFiConfig config = m_wifiManager->wifiConfig();
            ret.replace("{{checkedAP}}", config.ap ? "checked" : "");
            ret.replace("{{checkedClient}}", config.ap ? "" : "checked");
            ret.replace("{{SSID}}", config.ssid);
            return ret;
        }
    }

    if (var == "OTASECTION") {
        if (m_otaManager) {
            String ret(otaSection);
            return ret;
        }
    }

    if (var == "GPIOCONTROLSECTION") {
        if (m_gpioController) {
            String ret(gpioSection);
            String buttons;
            for (int i = 0; i < m_gpioController->gpioCount(); i++) {
                String button = gpioTemplate;
                button.replace("{{number}}", String(i));
                buttons += button;
            }
            ret.replace("{{buttons}}", buttons);
            return ret;
        }
    }
    return String();
}


