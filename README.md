Rogerio ifce.edu.br
# Estufa Ditatica 1.0
Projeto submetido ao curso de Sistemas embarcados como atividade final.

## Descrição
O sistema monitora a temperatura, umidade e luminosidade de uma estufa através 
dos sensores DHT11 e LDR, integrantes do módulo de desenvolvimento LAB01. Os 
valores medidos são apresentados em tempo real num display OLED.

Um rele é acionado, pino GPIO 14 ou LED vermelho no LAB01, quando a temperatura 
lida no sensor é menor que o valor do Setpoint. Também esta indicação é 
apresentada no display na tela principal.

São três modos de controle da saída: 
  Automático, 
  Sempre ligado,
  Sempre desligado.

Os parâmetros do Setpoint e Modo de controle sempre tem uma cópia gravada na 
memória NVS, para  manter a integridade numa falha de energia ou reset. 
Os valores são gravados quando são alterados ou na inicialização.

Os valores e parâmentros do sistema podem ser monitorados pela porta serial.

O registro de dados está num arquivo texto na memória NVS interna ao ESP32

## Diagrama de blocos
```
          ┌──────────────────────────┐
          │      Sensores de Entrada │
          │ ───────────────────────  │
          │  • DHT11 (Temperatura)   │
          │  • DHT11 (Umidade)       │
          │  • Sensor LED (Luz)      │
          └──────────┬───────────────┘
                     │
             ┌───────▼────────┐
             │   ESP32-S2     │
             │ ─────────────  │
             │  • Processador │
             │  • NVS interna │◄── Registro de dados em arquivo texto
             │                │
             └───────┬────────┘
                     │
                     │
   ┌──────────┬──────▼─────┬────────────────┐
   │          │            │                │
   │          │            │                │
┌──▼───┐   ┌──▼────┐   ┌───▼────┐  ┌────────▼──────┐
│Botões│   │  OLED │   │ Serial │  │      Saída    │
│  ↑   │   │       │   │        │  │               │
│  ↓   │   │SSD1306│   │USB/UART│  │ Relé → Resist.│
│  ←   │   │       │   │        │  │   da Estufa   │
│  →   │   │       │   │        │  │               │
│ OK   │   │       │   │        │  │               │
│ ESC  │   │       │   │        │  │               │
└──────┘   └───────┘   └────────┘  └───────────────┘
  │            │           │
  └── Comunicação usuário──┘

```

## Estrutura do Menu

 Teclas de Navegação: Cima (BT1), Baixo (BT4), Esquerda (BT2) e Direita (BT3)
 Tecla de comando: OK (BT5) e ESC (BT6)
 Tecla de Controle do Setpoint, dentro do menu setpoint '+' (BT6) e '-' (BT5)

 Telas do Menu:

```
┌─────────────────────┐
├─ 1 Principal        |
├     Estufa <versão> |
├     Rogerio IFCE    |
├     Temperat. =     |
├     Umidade   =     |
├     Brilho    =     |
├                     |
├     Set point =     |
├     Rele: Desligado |
├─────────────────────┤
├─ 2  ===SETPOINT===  |
├     +: Incrementa   |
├     -: Decrementa   |
├     Set Point = xx°C|
├─────────────────────┤
├─ 3  ===CONTROLE==== |
├     A: Automatico   |
├     L: Ligado       |
├     D: Desligado    |
├                     |
├     <OK> Para salvar|
├─────────────────────┤
├─ 4  ====ARQUIVO==== |
├     T: lista Tudo   |
├     Z: Apaga tudo   |
├     G: Ativa Gravac |
├     g: Para Gravaca |
├                     |
├     <Gravando>      |
└─────────────────────┘
```
## Comando pela UART
O usuário pode interagir pelo sistema através da comunicação serial UART a 115200 bps, os comandos são os mesmos acessados pela interfafece teclado e display, através dos caracteres '+','-','a','A','l','L','d','D','g','G','Z' e 'T'.

## Partição de memória
a configuração da memória é:

# ESP-IDF Partition Table
```
Name     , Type , SubType , Offset  , Size   , Flags
nvs      , data , nvs     , 0x9000  , 0x6000 ,
phy_init , data , phy     , 0xf000  , 0x1000 ,
factory  , app  , factory , 0x10000 , 1M     ,
littlefs , data , spiffs  ,         , 256K   ,

```
