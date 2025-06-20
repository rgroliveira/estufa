# Estufa Ditatica 1.0
Rogerio ifce.edu.br

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


## Brainstorm Menu
```
├─ 1 Principal
├─ 2 Setpoint
├─ 2.1 Setpoint
├─ 3 Modo
├─ 3.1 Automatico
├─ 3.2 Ligado
├─ 3.3 Desligado
├─ 4 Ajusta Hora
├─ 4.1 Hora
├─ 4.2 Minuto
├─ 4.3 Segundo
├─ 5 Ajusta data
├─ 5.1 Dia
├─ 5.2 Mes
└─ 5.3 Ano
```

teste
