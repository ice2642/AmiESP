# AmiESP
ESP8266 Virtual Modem — Fork do projeto original ESP_MODEM de Jussi Salin. Um firmware para ESP8266 que transforma o microcontrolador em um modem AT virtual via Wi-Fi, permitindo conexão TCP/Telnet e configuração com comandos AT clássicos.


# 🛰️ ESP8266 Virtual Modem (ESP_MODEM)

Projeto de modem virtual para **ESP8266**, desenvolvido por **Marcio Esper (aka ice2642)** com base no projeto original **ESP_MODEM** criado por **Jussi Salin** (<salinjus@gmail.com>).

Este firmware transforma o ESP8266 em um **modem AT virtual via Wi-Fi**, capaz de conectar-se a redes sem fio, exibir IP e abrir conexões TCP (telnet), simulando o comportamento de modems clássicos.

Distribuído sob os termos da **GNU General Public License v3 (GPLv3)**.  
Modificações e melhorias mantêm compatibilidade com a licença original.

---

## ⚙️ Objetivo

Fornecer uma forma simples de usar o ESP8266 como **modem Wi-Fi controlado por comandos AT**, facilitando integração com microcontroladores, terminais seriais e computadores retrô.

---

## 🚀 Funcionalidades

- Listagem de redes Wi-Fi (`ATSCAN`) com escolha numérica.  
- Conexão por nome (`ATWIFI`) ou índice (`ATWIFINUM`).  
- Exibição do IP atual (`ATIP`).  
- Conexão TCP com `ATDT<host>:<porta>`.  
- LED indicador de atividade (GPIO2).  
- Interface via Serial e Telnet.

---

## 🧠 Comandos AT disponíveis

| Comando | Descrição |
|----------|------------|
| `AT` | Testa comunicação e retorna `OK`. |
| `ATSCAN` | Escaneia redes Wi-Fi e lista com número, RSSI e segurança. |
| `ATWIFI<ssid>,<key>` | Conecta a uma rede Wi-Fi informando SSID e senha. |
| `ATWIFINUM<n>,<key>` | Conecta à rede número `<n>` listada com `ATSCAN`. |
| `ATIP` | Mostra o IP atual do ESP8266. |
| `ATDT<host>:<port>` | Abre conexão TCP/Telnet para o host e porta indicados. |

*(planejado)*  
- `ATWIFIOFF` — Desconecta do Wi-Fi.  
- `ATCLEAR` — Limpa lista de redes escaneadas.

---

## 🖥️ Como usar

### 1. Gravação no Arduino IDE

1. Instale o **core do ESP8266** (Gerenciador de Placas → `esp8266`).  
2. Selecione sua placa (ex: *NodeMCU 1.0*).  
3. Configure porta serial e velocidade **115200 baud**.  
4. Copie o arquivo `esp_modem.ino` e grave no ESP8266.

### 2. Conexão via Serial

Abra o **Monitor Serial** em 115200 baud.  
Digite comandos AT, por exemplo:

```text
ATSCAN
1: MinhaRede [SECURED]
2: Visitantes [OPEN]
OK

ATWIFINUM1,minhasenha
Connecting to MinhaRede...
OK

ATIP
IP: 192.168.0.105
