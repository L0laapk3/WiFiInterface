This library for esp8266 connects to the last known network, or lets the user connect to an AP to select a new network. 

Usage:
```
WiFiInterface.connect(String APName, function requestUserFn, function doneUserFn);
```

`APName` Is the name of the Access Point that will be created.

`requestUserFn` Will be called if the user needs to connect to the AP to complete the WiFi connection process.
`doneUserFn` Will be called if the user succesfully logged in to a new WiFi network.

The library attempts to set a static ip to speed up the connecting process and save battery.
