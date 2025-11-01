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

⚙️ Configurações padrão
Parâmetro	Valor
Baud rate	115200
Porta TCP	23
LED de atividade	GPIO2
Máximo de redes listadas	10

🧩 Hardware compatível
NodeMCU 1.0 (ESP-12E)

ESP-01 / ESP-01S (com adaptador serial 3.3V)

Qualquer módulo com ESP8266

📜 Créditos e Licença
Autor original: Jussi Salin
Versão modificada: Marcio Esper (ice2642)
Licença: GNU General Public License v3 (GPLv3)
Ano: 2025

Este projeto é um fork de ESP_MODEM (Jussi Salin) e segue integralmente os termos da GPLv3.
Você pode copiar, modificar e redistribuir, mantendo os créditos e a mesma licença.

🌎 English Section
🛰️ ESP8266 Virtual Modem (ESP_MODEM)
A Wi-Fi virtual modem firmware for the ESP8266, developed by Marcio Esper (ice2642) and based on the original ESP_MODEM by Jussi Salin (salinjus@gmail.com).

This firmware turns the ESP8266 into a Wi-Fi AT-command modem, allowing connection to wireless networks, IP display, and TCP/Telnet dialing — just like a classic serial modem.

Distributed under the terms of the GNU General Public License v3 (GPLv3).

⚙️ Features
Wi-Fi scanning and numbered list (ATSCAN)

Connection by SSID (ATWIFI) or by index (ATWIFINUM)

IP address display (ATIP)

TCP/Telnet connection via ATDT<host>:<port>

Activity LED (GPIO2)

Serial or Telnet control

💻 Basic Usage
Upload
Open Arduino IDE and install the ESP8266 core.

Select your board (NodeMCU, ESP-01, etc.).

Set baud rate to 115200 and upload the sketch esp_modem.ino.

Serial Commands
ATSCAN
1: MyWiFi [SECURED]
2: Guest [OPEN]
OK

ATWIFINUM1,myPassword
Connecting to MyWiFi...
OK

ATIP
IP: 192.168.0.105

Telnet Mode
After connecting to Wi-Fi, open a Telnet client to port 23:

⚙️ Default Configuration
Setting	Value
Baud rate	115200
TCP Port	23
LED Pin	GPIO2
Max scanned networks	10
￼
📜 Credits & License
Original Author: Jussi Salin

Modified by: Marcio Esper (ice2642)

License: GNU General Public License v3 (GPLv3)

This project is a derivative work of ESP_MODEM by Jussi Salin and remains under the same GPLv3 terms.
Version: ESP_MODEM v2.0 – 2025
Language: Portuguese / English
License: GPLv3
Platform: Arduino IDE + ESP8266

---

## 📄 Arquivo de licença recomendado (`LICENSE`)

Se o original de Jussi era GPL, coloque este texto (ou equivalente) no seu repositório:

ESP_MODEM - ESP8266 Virtual Modem
Copyright (C) 2025 Marcio Esper
Based on original work by Jussi Salin salinjus@gmail.com

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, version 3 of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program. If not, see https://www.gnu.org/licenses/.



