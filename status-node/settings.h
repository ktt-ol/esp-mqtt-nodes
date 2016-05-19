// change sensorHostname to something unique!
const char *signalHostname = "status-signal-foo";
// const char *otaPassword = "xxx"; // set in credentials.h!

// set ssids and passwords in credentials (better arranged for wifis outside mainframe)
//const char *ssid0 = "xxx";
//const char *ssid0Password = "xxx";
//const char *ssid1 = "xxx";
//const char *ssid1Password = "xxx";

// To get the current fingerprint:
// echo -n | openssl s_client -connect spacegate.mainframe.io:8883 2>/dev/null | openssl x509  -noout -fingerprint | cut -f2 -d'=' | sed 's/:/ /g'
const char *mqttHost = "mainframe.io";
const char *mqttFingerprint = "86 B8 71 77 2D 99 74 42 05 B8 30 78 91 12 F5 32 F1 FA 65 70";

const int mqttPort = 8883;

// Define status topic to display
const char* statusTopic = "/access-control-system/space-state";

// Define pinouts for "lame" non-SPI RGB LED stripes
// Those work for NodeMCU as connected on space-beacon in conference
//#define PIN_RED 14            //< devkit: D5; esp8266: GPIO14
//#define PIN_GREEN 12          //< devkit: D6; esp8266: GPIO12
//#define PIN_BLUE 4            //< devkit: D2; esp8266: GPIO4
// Use those for the witty onboard RGB LED
#define PIN_RED 15
#define PIN_GREEN 12
#define PIN_BLUE 13

// Define parameters for animations
#define FADESPEED 10          //< speed gets slower the higher this value is
#define ANIMATIONSPEED 20      //< separate speed for animations, such as the greeting animation
