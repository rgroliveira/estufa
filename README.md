Rogerio ifce.edu.br
**# Estufa Ditatica 1.0**
Projeto submetido ao curso de Sistemas embarcados como atividade final.

**## Descrição**
O sistema monitora a temperatura, umidade e luminosidade de uma estufa através 
dos sensores DHT11 e LDR, integrantes do módulo de desenvolvimento LAB01. Os 
valores medidos são apresentados em tempo real num display OLED.

Um rele é acionado, pino GPIO 14 ou LED vermelho no LAB01, quando a temperatura 
lida no sensor é menor que o valor do Setpoint. Também esta indicação é 
apresentada no disply na tela principal.

São três modos de controle da saída: 
  Automático, 
  Sempre ligado,
  Sempre desligado.

Os parâmetros do Setpoint e Modo de controle sempre tem uma cópia gravada na 
memória NVS, para  manter a integridade numa falha de energia ou reset. 
Os valores são gravados quando são alterados ou na inicialização.

Os valores e parâmentros do sistema podem ser monitorados pela porta serial.

O registro de dados está num arquivo texto na memória NVS interna ao ESP32


**## Estrutura do Menu**

 Teclas de Navegação: Cima (BT1), Baixo (BT4), Esquerda (BT2) e Direira (BT3)
 Tecla de comando: OK (BT5) e ESC (BT6)
 Tecla de Controle do Setpoint, dentro do menu setpoint '+' (BT6) e '-' (BT5)

 Telas do Menu:

├─ 1 Principal
├     Estufa <versão>
├     Rogerio IFCE
├     Temperat. = 
├     Umidade   = 
├     Brilho    = 
├     00/00/0000 00:00
├     Set point = 
├     Rele: Desligado
|
├─ 2  ===SETPOINT==="
├     +: Incrementa  "
├     -: Decrementa  "
├     Set Point = xx °C
|
├─ 3  ===CONTROLE====
├     A: Automatico  
├     L: Ligado      
├     D: Desligado   
├   
├     <OK> Para salvar
|
├─ 4  ====ARQUIVO===="
├     L: Lista tudo  "
├     Z: Apaga tudo  "


**# INSTRUÇÕES**
##Projeto Final: Rogerio
1	Descrição do Projeto
   O projeto consiste em criar um sistema embarcado para monitorar e controlar os parâmetros 
   de uma estufa. O sistema utiliza sensores para medir temperatura, umidade e luminosidade, 
   exibindo os valores em um display OLED. O usuário poderá ajustar o set point de temperatura 
   por meio de um menu interativo no display. Além disso, será possível registrar os dados 
   monitorados em um arquivo na memória do ESP32 e acessá-los posteriormente via comunicação 
   serial.

2 Requisitos do Projeto
2.1 Monitoramento:
    DHT11: Monitorar temperatura e umidade.
    LDR: Monitorar a luminosidade da estufa.

2.2 Controle e Exibição:
    Display OLED: Exibir os valores de temperatura, umidade e luminosidade em tempo real.
    Implementar um menu interativo para o ajuste do set point de temperatura e outras configurações.

2.3 Configuração e Operação:
   O usuário poderá ligar/desligar o sistema de controle manualmente.
   Configurar o menuconfig para permitir ajustes de pré-compilação, como pinos dos sensores e 
   parâmetros padrões, etc

2.4 Registro de Dados:
   Implementar o SPIFFS ou LittleFS para salvar os valores monitorados em um arquivo log.txt.
   O usuário poderá habilitar/desabilitar o registro de dados via menu interativo.
   Implementar um comando serial para leitura do arquivo de log, exibindo os dados salvos no terminal.

3 Estrutura do Projeto:
  O projeto deve ser organizado em bibliotecas (componentes) para modularidade e reutilização de código.

# Arquivos no LittleFS
// Atencao: No menuconfig, configure:
// "Partition Table" -> "Custom partition table CSV" 
// File -> "partition.csv"
// Arquivo "partition.csv"
//        # ESP-IDF Partition Table
//        # Name, Type, SubType, Offset, Size, Flags
//        nvs,        data, nvs,      0x9000,  0x6000,
//        phy_init,   data, phy,      0xf000,  0x1000,
//        factory,    app,  factory,  0x10000, 1M,
//        littlefs,data,spiffs,,256K,,

