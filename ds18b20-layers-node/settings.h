// change sensorHostname to something unique!
const char *sensorHostname = "sensornode-layers";
// const char *otaPassword = "xxx"; // set in credentials.h!

const char *ssid0 = "mainframe.iot";
// const char *ssid0Password = "xxx"; // set in credentials.h!

const char *ssid1 = "mainframe-legacy";
// const char *ssid1Password = "xxx"; // set in credentials.h!

// To get the current fingerprint:
// echo -n | openssl s_client -connect spacegate.mainframe.io:8883 2>/dev/null | openssl x509  -noout -fingerprint | cut -f2 -d'=' | sed 's/:/ /g'
const char *mqttHost = "spacegate.mainframe.lan";
const char *mqttFingerprint = "D5 CB 44 A6 4C FB 43 B7 BD B3 BE FA 1B A7 E4 6E BB 64 10 17";
//const char *mqttHost = "mainframe.io";
//const char *mqttFingerprint = "86 B8 71 77 2D 99 74 42 05 B8 30 78 91 12 F5 32 F1 FA 65 70";

const int mqttPort = 8883;
const char *mqttUser = "test";
// const char *mqttPassword = "xxx"; // set in credentials.h!

# define BASE_TOPIC "/test/shop"
const char *mqttSensorTopic = BASE_TOPIC"/fritz-fridge/temperature";
//const char *mqttSensorTopic2 = "/test/shop/eis-freezer/temperature";
const char *mqttSensorTopic3 = BASE_TOPIC"/temperature";
const char *mqttSensorTopic4 = BASE_TOPIC"/humidity";
// Update intervals:
// updateInterval defines how often the sendsor should be read and
// published via MQTT (in ms)
const long updateInterval = 10000;
// retryInterval defines after how many ms measureAndPublish should
// be retried after it failed (i.e. due to sensor read error)
const long retyInterval = 1000;
