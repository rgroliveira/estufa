// Estufa
static const char *TAG_VERSAO = "Estufa 1.02b";

/*
Projeto Final: Rogerio
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
*/

// Repopsitorio de Componentes: 
//    DHT11:        https://github.com/UncleRus/esp-idf-lib
//    SSD1306:      https://github.com/nopnop2002/esp-idf-ssd1306
//    OLED SSD1306: https://github.com/nopnop2002/esp-idf-ssd1306
//=============================================================================


// No Franzininho  com LAB01, para Display OLED SSD1306: SCL 9, SDA 8 e RESET GPIO -1
// Codigo do display
//include the libraries
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

#include "esp_adc/adc_oneshot.h" // Includes para ADC do LDR
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"

#include "dht.h"      // Biblioteca DHT para DHT11
#include "ssd1306.h"  // Biblioteca SSD1306 para OLED
 

//Pinos
#define PINO_LDR                1   //Pino do LDR, GPIO 34 é um pino apenas de entrada
#define PINO_BOTAO_INCREMENTA   2   //LAB01 GPIO2
#define PINO_BOTAO_DECREMENTA   3   //LAB01 GPIO3
#define PINO_BOTAO_BAIXO        4   //LAB01 GPIO4
#define PINO_BOTAO_DIREITA      5   //LAB01 GPIO5
#define PINO_BOTAO_ESQUERDA     6   //LAB01 GPIO6
#define PINO_BOTAO_CIMA         7   //LAB01 GPIO7

#define PINO_I2C_SDA            8   //LAB01 SDA      
#define PINO_I2C_SCL            9   //LAB01 SCL     
#define PINO_OLED_RESET         -1  // Nao implementado no LAB01, GPIO -1    

#define PINO_LED_B              12   //LAB01 LED Azul
#define PINO_LED_G              13   //LAB01 LED Verde
#define PINO_LED_R              14   //LAB01 LED Vermelho
#define PINO_RELE               14   //Pino do relé, GPIO 14 ( no LAB01 é o LED vermelho
#define PINO_DHT11              15
#define PINO_BUZZER             17   //LAB01 Buzzer
#define PINO_LED_F1             21   //LED Franzininho
#define PINO_LED_F2             33   //LED Franzininho

//DHT11 configuration
static const dht_sensor_type_t DHT11_TIPO = DHT_TYPE_DHT11;

//TAG para o log
static const char *TAG_ADC_LDR = "ADC1_0 LDR";

// ADC
adc_oneshot_unit_handle_t G_ADC1_Handle;                              //ADC1 Handle

struct Tipo_Registro{
    int16_t Temperatura;    //Temperatura °C
    int16_t Umidade;        //Umidade em %
    int16_t Brilho;         //Brilho em mV
    uint8_t Dia;            //Dia
    uint8_t Mes;            //Mes
    uint16_t Ano;           //Ano
    uint8_t Hora;           //Hora
    uint8_t Minuto;         //Minuto
    uint8_t Segundo;        //Segundo
   
}  GRegistro1;

int16_t     GSetPoint_Temperatura   = CONFIG_ESTUFA_SETPOINT_DEFAULT; // definido no menuconfig
int16_t     GTemperatura_Max        = CONFIG_ESTUFA_MAX_TEMP;      // definido no menuconfig
int16_t     GTemperatura_Min        = CONFIG_ESTUFA_MIN_TEMP;      // definido no menuconfig
uint8_t     GRele_Ativo = 0;            // Rele Ativo ou Inativo
char        GAuxs[40];                  //String auxiliar para formatar os dados (aumentado para evitar overflow)
SSD1306_t   GDisplay_OLED;              //Device do OLED SSD1306

//=============================================================================
void Display_OLED_Inicia( int TEspera)
{
    //initialize the OLED
    ESP_LOGI(TAG_VERSAO, "OLED SSD1306 128x64 I2C SCL=%d SDA=%d RESET=%d", PINO_I2C_SCL, PINO_I2C_SDA, PINO_OLED_RESET);                   
 //   ESP_LOGI(TAG_VERSAO, "INTERFACE is i2c");
 //   ESP_LOGI(TAG_VERSAO, "CONFIG_SCL_GPIO=%d",PINO_I2C_SCL);
 //   ESP_LOGI(TAG_VERSAO, "CONFIG_SDA_GPIO=%d",PINO_I2C_SDA);
 //   ESP_LOGI(TAG_VERSAO, "CONFIG_RESET_GPIO=%d",PINO_OLED_RESET);
    i2c_master_init(&GDisplay_OLED,PINO_I2C_SDA,PINO_I2C_SCL,PINO_OLED_RESET);     //initialize the i2c master driver

    //initialize the OLED
    ssd1306_init(&GDisplay_OLED,128,64);                         //initialize the OLED
    ssd1306_clear_screen(&GDisplay_OLED,false);                  //clear the OLED
    ssd1306_contrast(&GDisplay_OLED,0xFF);                       //set the contrast
    ssd1306_display_text_x3(&GDisplay_OLED,0,"IFCE",5,true);   //display the text

//void ssd1306_display_text_box1(SSD1306_t * dev, int page, int seg, const char * text, int box_width, int text_len, bool invert, int delay);
//    ssd1306_display_text_box1(&GDisplay_OLED,1,0,"Estufa 1.0" ,10,10,false,10); 
    ssd1306_display_text_box1(&GDisplay_OLED,5,0,"    Rogerio    "    , 15,19,true,10); 
    
    vTaskDelay(pdMS_TO_TICKS(TEspera * 1000));                    //delay     

    ssd1306_clear_screen(&GDisplay_OLED,false);                  //clear the OLED  
    ssd1306_contrast(&GDisplay_OLED,0xFF);                       //set the contrast
}


//=============================================================================
void LDR_ADC_Inicia(void)
{
    //inicializa o LDR ADC
    adc_oneshot_unit_init_cfg_t G_ADC1_Config = {                        //ADC1 Config
        .unit_id = ADC_UNIT_1,                                          //ADC1
    };
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&G_ADC1_Config, &G_ADC1_Handle)); //ADC1 Init

//---------- Configura o ADC ----------------------
    adc_oneshot_chan_cfg_t config = {                                                       //ADC1 Channel Config
        .bitwidth = ADC_BITWIDTH_DEFAULT,                                                   //ADC1 Bitwidth (Default)
        .atten = ADC_ATTEN_DB_12,                                                           //ADC1 Attenuation 
    };
    ESP_ERROR_CHECK(adc_oneshot_config_channel(G_ADC1_Handle, ADC_CHANNEL_0, &config));       
    
// Leitura do ADC
 
}
 
//=============================================================================
void LDR_ADC_Ler( int16_t *Tensao_Lida)
{//Le o LDR pelo ADC, retorna a tensao lida em mV
     static int Valor_Bruto;     //ADC Raw Data

    ESP_ERROR_CHECK(adc_oneshot_read(G_ADC1_Handle, ADC_CHANNEL_0, &Valor_Bruto));                        //ADC1 Read
    *Tensao_Lida = (Valor_Bruto * 2500)/8192;                                                                //Calculate Voltage
    ESP_LOGV (TAG_ADC_LDR, "ADC%d Channel[%d] Dado bruto: %d, Tensao: %d mV", ADC_UNIT_1 + 1, ADC_CHANNEL_0, Valor_Bruto, *Tensao_Lida);      //Print Voltage

}

//=============================================================================
uint8_t DHT11_Leitura(void)
{ int16_t Temperatura10 = 0;       //Temperatura * 10
  int16_t Umidade10 = 0;           //Umidade * 10
 
  if(dht_read_data(DHT11_TIPO, PINO_DHT11, &Umidade10,&Temperatura10)==ESP_OK)      //read the data from the sensor
    {
        GRegistro1.Temperatura = Temperatura10/10;    //store the temperature data
        GRegistro1.Umidade = Umidade10/10;            //store the humidity data
        return(1);
    }
    else
    {
        ESP_LOGE(TAG_VERSAO, "Nao pude ler dados do sensor DHT11");
        GRegistro1.Temperatura = 0;
        GRegistro1.Umidade = 0;   
        return(0);
    }
}


//=============================================================================
void Termostato_Processa( uint8_t Linha1_Display, uint8_t Linha2_Display )
// Processa o rele de acordo com a temperatura lida, e escreve no display OLED
{ 
    if( Linha1_Display > 0)
    {
        sprintf(GAuxs, "Set Point = %2d'C", GSetPoint_Temperatura);          //formata linha do setpoint
        ssd1306_display_text(&GDisplay_OLED, Linha1_Display, GAuxs, strlen(GAuxs),false);     //mostra o setpoint
    }
    if (GRegistro1.Temperatura > GSetPoint_Temperatura)  // Se a temperatura for maior que o set point
    { 
        gpio_set_level(PINO_RELE, 0);  // Desliga o relé
        GRele_Ativo = 0;               // Rele Inativo
        if ( Linha2_Display > 0)
           ssd1306_display_text(&GDisplay_OLED, Linha2_Display, "Rele: Desligado", 16, false); // Exibe no OLED
    }
    else
    {
        gpio_set_level(PINO_RELE, 1);  // Liga o relé
        GRele_Ativo = 1;               // Rele Ativo
        if ( Linha2_Display > 0)
           ssd1306_display_text(&GDisplay_OLED, Linha2_Display, "Rele: Ligado"   , 16, true); // Exibe no OLED
    }

}

//=============================================================================
//manipulador da fila de interrupcao do GPIO
static QueueHandle_t gpio_evt_queue = NULL;                 //queue to handle gpio events

//ISR handler
static void IRAM_ATTR gpio_isr_handler(void* arg)   
{
    uint32_t Numero_Pino = (uint32_t) arg;                    //get the GPIO number
    xQueueSendFromISR(gpio_evt_queue , &Numero_Pino, NULL); //send the GPIO number to the queue
}

//button task
void buttonTask(void *pvpameters)
{
  uint32_t Numero_Pino;                     // Numero da GPIO
  TickType_t Tempo_Botao_Pressionado = 0;   // Armazena o instante em que o botao e pressionado   

  while (true) 
  {
    xQueueReceive(gpio_evt_queue, &Numero_Pino, portMAX_DELAY); // Espera por eventos de interrupção de GPIO
    ESP_LOGI(TAG_VERSAO, "Botao GPIO[%li]", Numero_Pino);        // Log the GPIO number that triggered the interrupt
    TickType_t Tempo_Atual = xTaskGetTickCount();               // Pega o tempo atual da tarefa

    if (Tempo_Atual - Tempo_Botao_Pressionado >= pdMS_TO_TICKS(250)) // Verifica se o tempo desde o último pressionamento é maior que 250ms
    {
        Tempo_Botao_Pressionado = Tempo_Atual;                  // Atualiza o tempo do último pressionamento do botão
        if (Numero_Pino == PINO_BOTAO_INCREMENTA)               // Checa se o botão 1 foi pressionado
            { GSetPoint_Temperatura++;                          // Aumenta o set point de temperatura
                Termostato_Processa(6,7);
            };
        if (Numero_Pino == PINO_BOTAO_DECREMENTA)               // Checa se o botão 2 foi pressionado
            { GSetPoint_Temperatura--;                          // Decrementa o set point de temperatura
                Termostato_Processa(6,7);
            };
    }
  }
}


//=============================================================================
void Inicia_os_GPIO(void)
{ // Configura os pinos dos botões como entrada e interrupcao

    // LEDs
    gpio_set_direction(PINO_LED_F1, GPIO_MODE_OUTPUT);              // Configura o pino do LED1 como saída
    gpio_set_level(PINO_LED_F1, 0);                                 // Desliga o LED1 inicialmente
    // Rele
    gpio_set_direction(PINO_RELE, GPIO_MODE_OUTPUT);                // Configura o pino do relé como saída
    gpio_set_level(PINO_RELE, GRele_Ativo);   
    // buzzer
    gpio_set_direction(PINO_BUZZER, GPIO_MODE_OUTPUT);              // Configura o pino do buzzer como saída
    gpio_set_level(PINO_BUZZER, 0);                                 // Desliga o buzzer inicialmente

    //initializa botão incrementa
    gpio_reset_pin(PINO_BOTAO_INCREMENTA);                          //select BUTTON_PIN as GPIO
    gpio_set_direction(PINO_BOTAO_INCREMENTA, GPIO_MODE_INPUT);     //set as input
    gpio_pullup_en(PINO_BOTAO_INCREMENTA);                          //enable pull-up
    gpio_pulldown_dis(PINO_BOTAO_INCREMENTA);                       //disable pull-down
    gpio_set_intr_type(PINO_BOTAO_INCREMENTA, GPIO_INTR_NEGEDGE);   //interrupt on negative edge

    //Incicializa botão decrementa
    gpio_reset_pin(PINO_BOTAO_DECREMENTA);                          //select BUTTON_PIN2 as GPIO
    gpio_set_direction(PINO_BOTAO_DECREMENTA, GPIO_MODE_INPUT);     //set as input
    gpio_pullup_en(PINO_BOTAO_DECREMENTA);                          //enable pull-up
    gpio_pulldown_dis(PINO_BOTAO_DECREMENTA);                       //disable pull-down
    gpio_set_intr_type(PINO_BOTAO_DECREMENTA, GPIO_INTR_NEGEDGE);   //interrupt on negative edge

    gpio_evt_queue  = xQueueCreate(1, sizeof(uint32_t));            //create queue to handle gpio event from ISR
    xTaskCreate(buttonTask, "buttonTask", 2048, NULL, 2, NULL);     //create button task

    gpio_install_isr_service(ESP_INTR_FLAG_LEVEL1);                  //install interrupt service routine
    gpio_isr_handler_add(PINO_BOTAO_INCREMENTA, gpio_isr_handler, (void*)PINO_BOTAO_INCREMENTA); //add ISR handler for button1
    gpio_isr_handler_add(PINO_BOTAO_DECREMENTA, gpio_isr_handler, (void*)PINO_BOTAO_DECREMENTA); //add ISR handler for button2

}

//=============================================================================
void RTC_DS3231_Leitura(void)
{ // Leitura do RTC DS3231
  // Esta função deve ser implementada se o RTC for utilizado
  // No momento, não há implementação para o RTC
//  ESP_LOGW(TAG_VERSAO, "RTC DS3231 não implementado");
  
    GRegistro1.Dia = 0;          // Dia
    GRegistro1.Mes = 0;          // Mês
    GRegistro1.Ano = 0;          // Ano
    GRegistro1.Hora = 0;         // Hora
    GRegistro1.Minuto = 0;       // Minuto
    GRegistro1.Segundo = 0;      // Segundo
    return;

}
//=============================================================================
void RTC_DS3231_Mostra_Hora(  uint8_t Linha_Display)
{ // Mostra a hora no display OLED

    sprintf(GAuxs, "%02d/%02d/%04d %02d:%02d:%02d", 
            GRegistro1.Dia, GRegistro1.Mes, GRegistro1.Ano, 
            GRegistro1.Hora, GRegistro1.Minuto, GRegistro1.Segundo); 
    ssd1306_display_text(&GDisplay_OLED,Linha_Display,GAuxs,strlen(GAuxs),false);      //display the date and time   

}

//=============================================================================
//=============================================================================
//=============================================================================
void app_main(void)
{
    uint8_t Piscada = 0;             // Pisca no LCD para indicar que o sistema esta funcionando
   
    Display_OLED_Inicia( 1 );
    LDR_ADC_Inicia();
    Inicia_os_GPIO();  // Inicia as entradas e saídas do GPIO

    ssd1306_display_text(&GDisplay_OLED,0, TAG_VERSAO     , strlen ( TAG_VERSAO ), false); 
    ssd1306_display_text(&GDisplay_OLED,1,"Rogerio IFCE" , 12, false); 

    while(1)
    {   LDR_ADC_Ler( &GRegistro1.Brilho );          //read the LDR data
        DHT11_Leitura();
        RTC_DS3231_Leitura();  

        sprintf(GAuxs, "Temperat. = %2d'C", GRegistro1.Temperatura);                       //format the temperature data   
        ssd1306_display_text(&GDisplay_OLED,2,GAuxs,strlen(GAuxs),false);      //display the temperature data
        sprintf(GAuxs, "Umidade   = %2d %%", GRegistro1.Umidade);                        //format the humidity data
        ssd1306_display_text(&GDisplay_OLED,3,GAuxs,strlen(GAuxs),false);      //display the humidity data
        sprintf(GAuxs, "Brilho    = %3d ", GRegistro1.Brilho);
        ssd1306_display_text(&GDisplay_OLED,4,GAuxs,strlen(GAuxs),false);      //display the humidity data

        ESP_LOGI(TAG_VERSAO,"Temperatura: %d'C, Umidade: %d%%, Brilho: %d, SP = %d'C", GRegistro1.Temperatura, GRegistro1.Umidade, GRegistro1.Brilho, GSetPoint_Temperatura); 
        
        //RTC_DS3231_Mostra_Hora( 5 ); // Mostra a hora no display OLED
        
        gpio_set_level(PINO_LED_F1, Piscada = !Piscada);

        Termostato_Processa( 6, 7);                               // Processa o rele de acordo com a temperatura lida
        
        vTaskDelay(pdMS_TO_TICKS(5000));                                        //delay             
    }
}
