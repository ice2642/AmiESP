/* ESP_MODEM fork by Marcio Esper (ice2642)
 * Based on work by Jussi Salin <salinjus@gmail.com>
 *
 * Updated: supports SSID and password with spaces and special characters
 * Adds EEPROM persistence: ATSAVE, ATLOAD, ATCLEARCFG
 */

#include <ESP8266WiFi.h>
#include <EEPROM.h>

#define DEFAULT_BPS 115200
#define LISTEN_PORT 23
#define LED_PIN 2
#define LED_TIME 1
#define MAX_SCAN_NETWORKS 10
#define EEPROM_SIZE 512
#define EEPROM_FLAG_ADDR 0
#define EEPROM_SSID_LEN_ADDR 1
#define EEPROM_SSID_ADDR 2
#define EEPROM_KEY_LEN_ADDR 258
#define EEPROM_KEY_ADDR 259

String scanSSIDs[MAX_SCAN_NETWORKS];
int numScannedNetworks = 0;

String lastUsedSsid = "";
String lastUsedKey = "";

WiFiClient tcpClient;
WiFiServer tcpServer(LISTEN_PORT);

long myBps = DEFAULT_BPS;
bool cmdMode = true;
String cmd = "";

void saveWiFiConfig(const String &ssid, const String &key) {
  EEPROM.begin(EEPROM_SIZE);
  EEPROM.write(EEPROM_FLAG_ADDR, 0xA5);
  int sLen = ssid.length();
  if (sLen > 255) sLen = 255;
  EEPROM.write(EEPROM_SSID_LEN_ADDR, sLen);
  for (int i = 0; i < sLen; i++) EEPROM.write(EEPROM_SSID_ADDR + i, ssid[i]);

  int kLen = key.length();
  if (kLen > 255) kLen = 255;
  EEPROM.write(EEPROM_KEY_LEN_ADDR, kLen);
  for (int i = 0; i < kLen; i++) EEPROM.write(EEPROM_KEY_ADDR + i, key[i]);

  EEPROM.commit();
  EEPROM.end();
}

bool loadWiFiConfig(String &ssid, String &key) {
  EEPROM.begin(EEPROM_SIZE);
  byte flag = EEPROM.read(EEPROM_FLAG_ADDR);
  if (flag != 0xA5) { EEPROM.end(); return false; }
  int sLen = EEPROM.read(EEPROM_SSID_LEN_ADDR);
  ssid = "";
  for (int i = 0; i < sLen; i++) ssid += char(EEPROM.read(EEPROM_SSID_ADDR + i));
  int kLen = EEPROM.read(EEPROM_KEY_LEN_ADDR);
  key = "";
  for (int i = 0; i < kLen; i++) key += char(EEPROM.read(EEPROM_KEY_ADDR + i));
  EEPROM.end();
  return true;
}

void clearWiFiConfig() {
  EEPROM.begin(EEPROM_SIZE);
  EEPROM.write(EEPROM_FLAG_ADDR, 0xFF);
  EEPROM.commit();
  EEPROM.end();
}

void led_on() {
  digitalWrite(LED_PIN, LOW);
}

void setup() {
  Serial.begin(DEFAULT_BPS);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH);

  Serial.println("Virtual modem");
  Serial.println("=============");
  Serial.println();
  Serial.println("Connect to WIFI: ATWIFI\"SSID\",\"PASSWORD\"");
  Serial.println("Connect to WIFI by index: ATWIFINUM<num>,\"PASSWORD\"");
  Serial.println("Scan networks: ATSCAN");
  Serial.println("Save current connection: ATSAVE");
  Serial.println("Load saved connection: ATLOAD");
  Serial.println("Clear saved config: ATCLEARCFG");
  Serial.println("See my IP address: ATIP");
  Serial.println();

  if (LISTEN_PORT > 0) {
    Serial.print("Listening to port ");
    Serial.println(LISTEN_PORT);
    tcpServer.begin();
  } else {
    Serial.println("Incoming connections disabled.");
  }

  // Try load saved WiFi config and connect
  String s, k;
  if (loadWiFiConfig(s, k)) {
    Serial.print("Found saved WiFi config: ");
    Serial.println(s);
    WiFi.begin(s.c_str(), k.c_str());
    for (int i = 0; i < 100; i++) {
      delay(100);
      if (WiFi.status() == WL_CONNECTED) {
        Serial.println("Connected (saved)");
        break;
      }
    }
  }

  Serial.println();
  Serial.println("OK");
}

String trimQuotes(const String &s) {
  String t = s;
  t.trim();
  if (t.length() >= 2 && t[0] == '"' && t[t.length()-1] == '"') {
    return t.substring(1, t.length()-1);
  }
  return t;
}

void handleATWIFI(String args) {
  args.trim();
  String ssid = "";
  String key = "";

  if (args.length() == 0) {
    Serial.println("ERROR: SSID not provided.");
    return;
  }

  if (args[0] == '"') {
    // quoted SSID
    int endQuote = -1;
    for (int i = 1; i < args.length(); i++) if (args[i] == '"') { endQuote = i; break; }
    if (endQuote == -1) { Serial.println("ERROR: Unterminated SSID quotes."); return; }
    ssid = args.substring(1, endQuote);
    int pos = endQuote + 1;
    while (pos < args.length() && isSpace(args[pos])) pos++;
    if (pos < args.length() && args[pos] == ',') {
      pos++;
      while (pos < args.length() && isSpace(args[pos])) pos++;
      if (pos < args.length() && args[pos] == '"') {
        int kEnd = -1;
        for (int j = pos+1; j < args.length(); j++) if (args[j] == '"') { kEnd = j; break; }
        if (kEnd == -1) { Serial.println("ERROR: Unterminated key quotes."); return; }
        key = args.substring(pos+1, kEnd);
      } else {
        key = args.substring(pos);
        key.trim();
      }
    } else {
      key = "";
    }
  } else {
    // unquoted SSID, split on first comma
    int comma = args.indexOf(',');
    if (comma == -1) {
      ssid = args;
      ssid.trim();
      key = "";
    } else {
      ssid = args.substring(0, comma);
      ssid.trim();
      key = args.substring(comma + 1);
      key.trim();
      if (key.length() >= 2 && key[0] == '"' && key[key.length()-1] == '"') key = key.substring(1, key.length()-1);
    }
  }

  if (ssid.length() == 0) {
    Serial.println("ERROR: SSID empty.");
    return;
  }

  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid.c_str(), key.c_str());
  for (int i = 0; i < 100; i++) {
    delay(100);
    if (WiFi.status() == WL_CONNECTED) {
      Serial.println("OK");
      lastUsedSsid = ssid;
      lastUsedKey = key;
      return;
    }
  }
  Serial.println("ERROR");
}

void handleATWIFINUM(String args) {
  args.trim();
  if (numScannedNetworks == 0) {
    Serial.println("ERROR: Run ATSCAN first.");
    return;
  }
  int comma = args.indexOf(',');
  int indexNum = 0;
  String key = "";
  if (comma == -1) {
    indexNum = args.toInt();
  } else {
    String nStr = args.substring(0, comma);
    nStr.trim();
    indexNum = nStr.toInt();
    key = args.substring(comma+1);
    key.trim();
    if (key.length() >= 2 && key[0] == '"' && key[key.length()-1] == '"') key = key.substring(1, key.length()-1);
  }
  if (indexNum < 1 || indexNum > numScannedNetworks) {
    Serial.print("ERROR: Invalid network number. Max is ");
    Serial.println(numScannedNetworks);
    return;
  }
  String ssid = scanSSIDs[indexNum - 1];
  Serial.print("Connecting to network #");
  Serial.print(indexNum);
  Serial.print(" (");
  Serial.print(ssid);
  Serial.println(")...");
  WiFi.begin(ssid.c_str(), key.c_str());
  for (int i = 0; i < 100; i++) {
    delay(100);
    if (WiFi.status() == WL_CONNECTED) {
      Serial.println("OK");
      lastUsedSsid = ssid;
      lastUsedKey = key;
      return;
    }
  }
  Serial.println("ERROR");
}

void command() {
  cmd.trim();
  if (cmd.length() == 0) return;
  Serial.println();
  String upCmd = cmd;
  upCmd.toUpperCase();

  if (upCmd == "AT") { Serial.println("OK"); cmd = ""; return; }

  if (upCmd.indexOf("ATDT") == 0) {
    // Connect to host: ATDThost:port
    int portIndex = cmd.indexOf(':');
    String host, port;
    if (portIndex != -1) {
      host = cmd.substring(4, portIndex);
      port = cmd.substring(portIndex + 1);
    } else {
      host = cmd.substring(4);
      port = "23";
    }
    host.trim(); port.trim();
    int portInt = port.toInt();
    Serial.print("Connecting to ");
    Serial.print(host);
    Serial.print(":");
    Serial.println(portInt);
    tcpClient.setNoDelay(true);
    if (tcpClient.connect(host.c_str(), portInt)) {
      Serial.print("CONNECT ");
      Serial.println(myBps);
      cmdMode = false;
      if (LISTEN_PORT > 0) tcpServer.stop();
    } else {
      Serial.println("NO CARRIER");
    }
    cmd = "";
    return;
  }

  if (upCmd.indexOf("ATWIFINUM") == 0) {
    String args = cmd.substring(9);
    handleATWIFINUM(args);
    cmd = "";
    return;
  }

  if (upCmd.indexOf("ATWIFI") == 0) {
    String args = cmd.substring(6);
    handleATWIFI(args);
    cmd = "";
    return;
  }

  if (upCmd == "ATSCAN") {
    Serial.println("Scanning available networks. Please wait...");
    int n = WiFi.scanNetworks();
    Serial.println("Scan complete.");
    numScannedNetworks = 0;
    if (n == 0) {
      Serial.println("No networks found.");
    } else {
      if (n > MAX_SCAN_NETWORKS) n = MAX_SCAN_NETWORKS;
      numScannedNetworks = n;
      for (int i = 0; i < n; ++i) {
        scanSSIDs[i] = WiFi.SSID(i);
        Serial.print(i + 1);
        Serial.print(": ");
        Serial.print(scanSSIDs[i]);
        Serial.print(" (RSSI: ");
        Serial.print(WiFi.RSSI(i));
        Serial.print(")");
        Serial.println((WiFi.encryptionType(i) == ENC_TYPE_NONE) ? " [OPEN]" : " [SECURED]");
        delay(10);
      }
    }
    Serial.println("OK");
    cmd = "";
    return;
  }

  if (upCmd == "ATIP") {
    Serial.print("IP: ");
    Serial.println(WiFi.localIP());
    cmd = "";
    return;
  }

  if (upCmd == "ATSAVE") {
    // Save last used credentials. Require that lastUsedSsid is set (after ATWIFI/ATWIFINUM).
    if (lastUsedSsid.length() == 0) {
      Serial.println("ERROR: No recent connection to save. Use ATWIFI or ATWIFINUM first.");
    } else {
      saveWiFiConfig(lastUsedSsid, lastUsedKey);
      Serial.println("OK");
    }
    cmd = "";
    return;
  }

  if (upCmd == "ATLOAD") {
    String s, k;
    if (loadWiFiConfig(s, k)) {
      Serial.print("Loading saved network: ");
      Serial.println(s);
      WiFi.begin(s.c_str(), k.c_str());
      for (int i = 0; i < 100; i++) {
        delay(100);
        if (WiFi.status() == WL_CONNECTED) {
          Serial.println("OK");
          lastUsedSsid = s;
          lastUsedKey = k;
          break;
        }
      }
    } else {
      Serial.println("ERROR: No saved configuration.");
    }
    cmd = "";
    return;
  }

  if (upCmd == "ATCLEARCFG") {
    clearWiFiConfig();
    Serial.println("OK");
    cmd = "";
    return;
  }

  Serial.println("ERROR: Unknown command");
  cmd = "";
}

void loop() {
  // Read serial input
  while (Serial.available()) {
    char c = Serial.read();
    if (c == '\r' || c == '\n') {
      if (cmd.length() > 0) {
        command();
        cmd = "";
      }
    } else {
      cmd += c;
    }
  }

  // If TCP client connected, bridge serial to TCP and vice versa
  if (tcpClient && tcpClient.connected()) {
    // serial -> tcp
    while (Serial.available()) {
      char c = Serial.read();
      tcpClient.write(c);
      led_on();
    }
    // tcp -> serial
    while (tcpClient.available()) {
      char c = tcpClient.read();
      Serial.write(c);
      led_on();
    }
  } else {
    // accept incoming telnet
    if (LISTEN_PORT > 0) {
      WiFiClient cl = tcpServer.available();
      if (cl) {
        tcpClient = cl;
        tcpClient.setNoDelay(true);
        Serial.println("CONNECT 115200");
        cmdMode = false;
        tcpServer.stop();
      }
    }
  }

  // turn off LED quick blink
  static unsigned long ledTime = 0;
  if (millis() - ledTime > LED_TIME) digitalWrite(LED_PIN, HIGH);
}
