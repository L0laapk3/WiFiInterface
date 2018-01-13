#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <EEPROM.h>
#include "WiFiInterface.h"
#include "user_interface.h"





WiFiInterfaceClass::WiFiInterfaceClass(void) { }


String WiFiInterfaceClass::getNetworkString() {
  uint16_t n;
  do {
    n = WiFi.scanNetworks();
  } while (n == 0);

  String networkString = "";
  
  bool* done = new bool[n]();                               //sort by rssi signal strength
  for (uint16_t j = 0; j < n; j++) {
    uint16_t nr = 0;
    int32_t strongest = INT32_MIN;
    for (uint16_t i = 0; i < n; i++) {
      if (!done[i] && WiFi.RSSI(i) > strongest) {
        nr = i;
        strongest = WiFi.RSSI(i);
      }
    }
    done[nr] = true;
    /*uint8_t* mac = WiFi.BSSID(nr);
    char macstr[12] = { 0 };
    sprintf(macstr, "%02X%02X%02X%02X%02X%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);*/
    networkString += "<a href='#' onclick='run(this)' data-encr='" + (String)WiFi.encryptionType(nr) + /*"' data-ch='" + (String)WiFi.channel(nr) + "' data-bssid='" + (String)macstr +*/ "'>" + WiFi.SSID(nr) + "</a>";
  }
  return networkString;
}








void WiFiInterfaceClass::connect(char* APName, std::function<void()> requestUserFn, std::function<void()> doneUserFn) {
  bool changed = false;

  ESP8266WebServer webServer(80);
  DNSServer dnsServer;
  String networkString;
  String errorString;
  bool keepWaiting = false;
      
  EEPROM.begin(32 + 64 + 4 + 4 + 4 /*+ 6 + 1*/);

  char ssid[33];
  char pwd[65];
  uint8_t ip[4];
  uint8_t gateway[4];
  uint8_t subnet[4];
  /*uint8_t bssid[6];
  uint8_t ch;*/
  for (int i = 0; i < 32; i++) ssid[i] = EEPROM.read(i);
  for (int i = 0; i < 64; i++) pwd[i] = EEPROM.read(32 + i);
  for (int i = 0; i < 4; i++) ip[i] = EEPROM.read(96 + i);
  for (int i = 0; i < 4; i++) gateway[i] = EEPROM.read(100 + i);
  for (int i = 0; i < 4; i++) subnet[i] = EEPROM.read(104 + i);
  /*
  for (int i = 0; i < 6; i++) bssid[i] = EEPROM.read(96 + i);
  ch = EEPROM.read(102);*/

  char status = 0;
  do {
    Serial.print("connecting to: ");
    Serial.print(ssid);
    Serial.print(" ");
    Serial.print(pwd);
    Serial.printf(" %u.%u.%u.%u", ip[0], ip[1], ip[2], ip[3]);
    Serial.printf(" %u.%u.%u.%u", gateway[0], gateway[1], gateway[2], gateway[3]);
    Serial.printf(" %u.%u.%u.%u\n", subnet[0], subnet[1], subnet[2], subnet[3]);

    long start = millis();
    //WiFi.begin(ssid, pwd, ch, bssid);
    //WiFi.config(IPAddress(192, 168, 10, 15), IPAddress(192, 168, 10, 1), IPAddress(255, 255, 255, 0));
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, pwd);
    if (!changed)
      WiFi.config((IPAddress)ip, (IPAddress)gateway, (IPAddress)subnet);
      
    do {
      delay(50);
      status = WiFi.status();
    } while (status == WL_DISCONNECTED);
    long tot = millis() - start;
    Serial.println("connecting took " + (String)tot + " ms");

    if (status != WL_CONNECTED) {
      Serial.println("connection failed");

      if (changed) {
        
        switch(wifi_station_get_connect_status()) {
          case STATION_NO_AP_FOUND:
            errorString = "<a style='background-color:darkred;font-weight:bold'>Could not find " + String(ssid) + "</a>";
          case STATION_WRONG_PASSWORD:
            errorString = "<a style='background-color:darkred;font-weight:bold'>Wrong Password</a>";
          case STATION_CONNECT_FAIL:
          default:
            errorString = "<a style='background-color:darkred;font-weight:bold'>Could not connect to " + String(ssid) + "</a>";
        }
        
      } else {

        if (requestUserFn) requestUserFn();

        Serial.println("starting ap+server");
        WiFi.mode(WIFI_AP_STA);
        IPAddress apIP(192, 168, 1, 1);
        WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
        WiFi.softAP(APName);

        dnsServer.start(53, "*", apIP);

        webServer.on("/rescan", [&]() {
          networkString = getNetworkString();
          webServer.sendHeader("Location", String("/"), true);
          webServer.send(302, "text/plain", "");
        });
        webServer.on("/go", [&]() {
          webServer.sendHeader("Location", String("/"), true);
          webServer.send(302, "text/plain", "");
          keepWaiting = true;
          webServer.arg("ssid").toCharArray(ssid, 33);
          webServer.arg("pwd").toCharArray(pwd, 65);
          /*String macstr = webServer.arg("bssid");
          Serial.println(macstr);
          char macchr[13] = { 0 };
          macstr.toCharArray(macchr, 13);
          uint64_t lmac = (uint64_t)strtoll(macchr, NULL, 16);
          memcpy(bssid, &lmac, 6);
          Serial.println(bssid[5], HEX);
          ch = (uint8_t)webServer.arg("ch").toInt();*/
        });
        webServer.onNotFound([&]() {
          webServer.send(200, "text/html", ""
                         "<!DOCTYPE html><html><head>"
                         "<title>" + String(APName) + "</title>"
                         "<meta name='apple-mobile-web-app-capable' content='yes' />"
                         "<meta name='viewport' content='minimal-ui, width=device-width, height=device-height, initial-scale = 1.0, minimum-scale = 1.0, maximum-scale = 1.0, user-scalable = no' />"
                         "<body onbeforeunload='document.getElementById(\"loader\").style.display=\"block\"'><style>*{margin:0}a{background:#f5f5f5;margin:10px 20px;display:block;padding:10px 30px;border-radius:10px;color:#000;text-decoration:none}a:first-of-type{font-weight:bold;width:auto;display:inline-block;float:right}h2{position:relative;left:40px;top:17px;margin-bottom:31px;width:0;white-space:nowrap}.overlay{position:fixed;width:100%;height:100%;background-color:rgba(0,0,0,0.25);top:0;left:0}.overlay:before{width:104px;height:104px;background-color:rgba(0,0,0,0.5);content:"";position:absolute;border-radius:30px;left:calc(50% - 52px);top:calc(50% - 52px)}#spinner{position:absolute;left:calc(50% - 26px);top:calc(50% - 26px)}</style><script type='text/javascript'>"
                         "function run(a){pwd='';while(a.dataset.encr!=7&&pwd.length<8)pwd=prompt('Please enter password for '+a.text,pwd);window.location.href='/go?ssid='+encodeURIComponent(a.text)+'&pwd='+encodeURIComponent(pwd)}"
                         "</script><div style='height:100vh;overflow-y:scroll'><a href='/rescan'>rescan</a><h2>Select a Network</h2>" + networkString + 
                         "</div><div id='loader' class='overlay' style='display: none'><svg id='spinner' width='52px' height='52px' xmlns='http://www.w3.org/2000/svg' viewBox='0 0 100 100' preserveAspectRatio='xMidYMid' class='uil-ring-alt'><rect x='0' y='0' width='100' height='100' fill='none' class='bk'></rect><circle cx='50' cy='50' r='40' stroke='#757575' fill='none' stroke-width='10' stroke-linecap='round'></circle><circle cx='50' cy='50' r='40' stroke='#D3D3D3' fill='none' stroke-width='6' stroke-linecap='round'><animate attributeName='stroke-dashoffset' dur='2s' repeatCount='indefinite' from='0' to='502'></animate><animate attributeName='stroke-dasharray' dur='2s' repeatCount='indefinite' values='150.6 100.4;1 250;150.6 100.4'></animate></circle></svg></div>"
                         "</body>");
        });

        webServer.begin();
      

        changed = true;
      }

      for (uint8_t i = 0; i < 32; i++) ssid[i] = 0;
      for (uint8_t i = 0; i < 64; i++) pwd[i] = 0;
      
      networkString = errorString + getNetworkString();

      Serial.println("waiting for connection");
      while (!ssid[0]) {
        dnsServer.processNextRequest();
        webServer.handleClient();
        delay(50);
      }
      Serial.println("done with handling clients");
      

    }

  } while (status != WL_CONNECTED);

  

  if (changed) {
    WiFi.softAPdisconnect();
    dnsServer.stop();
    webServer.stop();
    WiFi.mode(WIFI_STA);

    if (doneUserFn) doneUserFn();
    
    Serial.println("gotta save this!!");

    uint32_t newip = WiFi.localIP();
    uint32_t newgateway = WiFi.gatewayIP();
    uint32_t newsubnet = WiFi.subnetMask();
    memcpy(ip, &newip, sizeof(newip));
    memcpy(gateway, &newgateway, sizeof(newgateway));
    memcpy(subnet, &newsubnet, sizeof(newsubnet));
    
    for (int i = 0; i < 32; i++) EEPROM.write(i, ssid[i]);
    for (int i = 0; i < 64; i++) EEPROM.write(32 + i, pwd[i]);
    for (int i = 0; i < 4; i++) EEPROM.write(96 + i, ip[i]);
    for (int i = 0; i < 4; i++) EEPROM.write(100 + i, gateway[i]);
    for (int i = 0; i < 4; i++) EEPROM.write(104 + i, subnet[i]);
  }

  EEPROM.commit();
  EEPROM.end();
}


#if !defined(NO_GLOBAL_INSTANCES) && !defined(NO_GLOBAL_WiFiInterface)
WiFiInterfaceClass WiFiInterface;
#endif