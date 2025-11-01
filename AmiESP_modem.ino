
/* ESP_MODEM fork by Marcio Esper aka ice2642 based in ESP8266 based
 * virtual modem by Jussi Salin <salinjus@gmail.com>
 *
 * Modificações (31/10/2025):
 * - Lista de SSIDs com seleção numérica
 * - Correção de gerenciamento de memória (remoção de new/delete)
 * - Melhorias gerais de estabilidade
 */

#include <ESP8266WiFi.h>
#include <algorithm>

#define SWITCH_PIN 0
#define DEFAULT_BPS 115200
#define LISTEN_PORT 23
#define RING_INTERVAL 3000
#define LED_PIN 2
#define LED_TIME 1
#define TX_BUF_SIZE 256
#define MAX_CMD_LENGTH 256

// --- NOVAS VARIÁVEIS ---
#define MAX_SCAN_NETWORKS 10
String scanSSIDs[MAX_SCAN_NETWORKS];
int numScannedNetworks = 0;
// ------------------------

WiFiClient tcpClient;
WiFiServer tcpServer(LISTEN_PORT);
unsigned long lastRingMs = 0;
unsigned long ledTime = 0;
long myBps;
bool cmdMode = true;
bool telnet = true;
String cmd = "";

#define DO 0xfd
#define WONT 0xfc
#define WILL 0xfb
#define DONT 0xfe

// ----------------------------------------------------------------------

void setup()
{
  Serial.begin(DEFAULT_BPS);
  myBps = DEFAULT_BPS;

  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH);

  Serial.println("Virtual modem");
  Serial.println("=============");
  Serial.println();
  Serial.println("Connect to WIFI: ATWIFI<ssid>,<key>");
  Serial.println("Connect to WIFI by index: ATWIFINUM<num>,<key>");
  Serial.println("Scan networks: ATSCAN");
  Serial.println("See my IP address: ATIP");
  Serial.println();

  if (LISTEN_PORT > 0)
  {
    Serial.print("Listening to port ");
    Serial.println(LISTEN_PORT);
    tcpServer.begin();
  }
  else
  {
    Serial.println("Incoming connections disabled.");
  }

  Serial.println("");
  Serial.println("OK");
}

// ----------------------------------------------------------------------

void led_on(void)
{
  digitalWrite(LED_PIN, LOW);
  ledTime = millis();
}

// ----------------------------------------------------------------------

void command()
{
  cmd.trim();
  if (cmd == "") return;

  Serial.println();
  String upCmd = cmd;
  upCmd.toUpperCase();

  long newBps = 0;

  /**** Just AT ****/
  if (upCmd == "AT") Serial.println("OK");

  /**** Connect to host ****/
  else if (upCmd.indexOf("ATDT") == 0)
  {
    int portIndex = cmd.indexOf(":");
    String host, port;
    if (portIndex != -1)
    {
      host = cmd.substring(4, portIndex);
      port = cmd.substring(portIndex + 1);
    }
    else
    {
      host = cmd.substring(4);
      port = "23";
    }

    Serial.print("Connecting to ");
    Serial.print(host);
    Serial.print(":");
    Serial.println(port);

    int portInt = port.toInt();
    tcpClient.setNoDelay(true);
    if (tcpClient.connect(host.c_str(), portInt))
    {
      tcpClient.setNoDelay(true);
      Serial.print("CONNECT ");
      Serial.println(myBps);
      cmdMode = false;
      Serial.flush();
      if (LISTEN_PORT > 0) tcpServer.stop();
    }
    else
    {
      Serial.println("NO CARRIER");
    }
  }

  /**** Connect to WiFi (manual SSID) ****/
  else if (upCmd.indexOf("ATWIFI") == 0 && upCmd.indexOf("ATWIFINUM") != 0)
  {
    int keyIndex = cmd.indexOf(",");
    String ssid, key;
    if (keyIndex != -1)
    {
      ssid = cmd.substring(6, keyIndex);
      key = cmd.substring(keyIndex + 1);
    }
    else
    {
      ssid = cmd.substring(6);
      key = "";
    }

    Serial.print("Connecting to ");
    Serial.println(ssid);

    WiFi.begin(ssid.c_str(), key.c_str());

    for (int i = 0; i < 100; i++)
    {
      delay(100);
      if (WiFi.status() == WL_CONNECTED)
      {
        Serial.println("OK");
        return;
      }
    }
    Serial.println("ERROR");
  }

  /**** Scan Wi-Fi Networks ****/
  else if (upCmd == "ATSCAN")
  {
    Serial.println("Scanning available networks. Please wait...");
    int n = WiFi.scanNetworks();
    Serial.println("Scan complete.");

    numScannedNetworks = 0;

    if (n == 0)
    {
      Serial.println("No networks found.");
    }
    else
    {
      if (n > MAX_SCAN_NETWORKS) n = MAX_SCAN_NETWORKS;
      numScannedNetworks = n;

      for (int i = 0; i < n; ++i)
      {
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
  }

  /**** Connect using numeric network index ****/
  else if (upCmd.indexOf("ATWIFINUM") == 0)
  {
    if (numScannedNetworks == 0)
    {
      Serial.println("ERROR: Run ATSCAN first.");
      return;
    }

    int keyIndex = cmd.indexOf(",");
    int indexNum = 0;
    String key = "";

    if (keyIndex != -1)
    {
      String numStr = cmd.substring(9, keyIndex);
      indexNum = numStr.toInt();
      key = cmd.substring(keyIndex + 1);
    }
    else
    {
      String numStr = cmd.substring(9);
      indexNum = numStr.toInt();
    }

    if (indexNum < 1 || indexNum > numScannedNetworks)
    {
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

    for (int i = 0; i < 100; i++)
    {
      delay(100);
      if (WiFi.status() == WL_CONNECTED)
      {
        Serial.println("OK");
        return;
      }
    }
    Serial.println("ERROR");
  }

  /**** Get IP ****/
  else if (upCmd == "ATIP")
  {
    Serial.print("IP: ");
    Serial.println(WiFi.localIP());
  }

  else
  {
    Serial.println("ERROR: Unknown command");
  }
}

// ----------------------------------------------------------------------

void loop()
{
  // Pisca LED brevemente em atividade
  if ((millis() - ledTime) > LED_TIME)
    digitalWrite(LED_PIN, HIGH);

  // Lê comandos da serial
  if (Serial.available())
  {
    char c = Serial.read();
    if (c == '\n' || c == '\r')
    {
      if (cmd.length() > 0)
      {
        command();
        cmd = "";
      }
    }
    else
    {
      cmd += c;
    }
  }
}
