// Estufa
static const char *TAG_VERSAO = "Estufa r1.01";


// Repopsitorio de Componentes: 
//    DHT11:        https://github.com/UncleRus/esp-idf-lib
//    SSD1306:      https://github.com/nopnop2002/esp-idf-ssd1306
//    OLED SSD1306: https://github.com/nopnop2002/esp-idf-ssd1306
//=============================================================================

// Configuração do ESP-IDF
// Atencao: No menuconfig, configure a "Partition Table" para "Custom partition table CSV" 
// e informe o arquivo "partition.csv"
// Arquivo "Partition CSV"
//        # ESP-IDF Partition Table
//        # Name, Type, SubType, Offset, Size, Flags
//        nvs,        data, nvs,      0x9000,  0x6000,
//        phy_init,   data, phy,      0xf000,  0x1000,
//        factory,    app,  factory,  0x10000, 1M,
//        littlefs,data,spiffs,,256K,,

// Hora do sistema
// https://docs-espressif-com.translate.goog/projects/esp-idf/en/stable/esp32/api-reference/system/system_time.html?_x_tr_sl=en&_x_tr_tl=pt&_x_tr_hl=pt&_x_tr_pto=tc


// No Franzininho  com LAB01, para Display OLED SSD1306: SCL 9, SDA 8 e RESET GPIO -1
// Codigo do display
//include the libraries
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include <driver/gpio.h>

// #include "esp_adc/adc_oneshot.h" // Includes para ADC do LDR
// #include "esp_adc/adc_cali.h"
// #include "esp_adc/adc_cali_scheme.h"
// #include "dht.h"      // Biblioteca DHT para DHT11

#include "ssd1306.h"  // Biblioteca SSD1306 para OLED
#include "RMemoria_NVS.h" 
#include "Pinos_Franzininho_LAB01.h"
#include "RMenus.h"

#include "Componente_Rele.h"
#include "Componente_LAB01_Sensores.h"


#define TEMPO_ENTRE_LEITURAS  5000  // Tempo entre leituras dos sensores em ms
#define PINO_RELE    PINO_LED_R     // Define o pino do relé no LED Vermelho (GPIO 14) no LAB01

static QueueHandle_t Handle_Eventos_gpio = NULL;           // Manipulador da fila de interrupcao do GPIO

typedef struct {
    int16_t Temperatura;    //Temperatura °C
    int16_t Umidade;        //Umidade em %
    int16_t Brilho;         //Brilho em mV
    uint8_t Dia;            //Dia
    uint8_t Mes;            //Mes
    uint16_t Ano;           //Ano
    uint8_t Hora;           //Hora
    uint8_t Minuto;         //Minuto
    uint8_t Segundo;        //Segundo

}  Tipo_Registro;
Tipo_Registro GRegistro1;

int8_t      GSetPoint_Temperatura   = CONFIG_ESTUFA_SETPOINT_DEFAULT; // definido no menuconfig
int16_t     GTemperatura_Max        = CONFIG_ESTUFA_MAX_TEMP;         // definido no menuconfig
int16_t     GTemperatura_Min        = CONFIG_ESTUFA_MIN_TEMP;         // definido no menuconfig
uint8_t     GRele_Ativo = 0;            // Rele Ativo ou Inativo
uint8_t     GRele_Estado_Anterior = 0;  // Usado para detectar transicao do relé

char        GAuxs[50];                  //String auxiliar para formatar os dados (aumentado para evitar overflow)
SSD1306_t   GDisplay_OLED;              //Device do OLED SSD1306
long        GInstante_Inicial = 0;      // Instante em que o sistema é iniciado para controle de tempo


// Menus
#define MAX_MENU_TELAS   4
#define MAX_MENUS_LINHAS 3

enum Tipo_Menu_Telas {
        MENU_TELA_PRINCIPAL = 0, // Tela principal do menu
        MENU_TELA_SETPOINT,      // Tela de setpoint
        MENU_TELA_CONTROLE,      // Tela de controle
        MENU_TELA_ARQUIVO        // Tela de arquivo
} GMenu_Tela_Atual;              // Variável global para armazenar a tela atual do menu

enum Tipo_Modo_Controle {
        MODO_AUTOMATICO = 0, // Controle automático
        MODO_MANUAL_LIGADO,
        MODO_MANUAL_DESLIGADO
};

int8_t GModo_Controle;         // Variável global para armazenar o modo de controle
int8_t GMenu_Coluna_Atual = 0; // Coluna atual do menu
int8_t GMenu_Linha_Atual = 0;  // Sub-menu atual, usado para telas com sub-menus
int8_t GModo_Gravacao = 0;     // Modo de gravação de arquivo, habilitado ou não
Tipo_Rele GRele_EStufa;        // Componente relé para controle da estufa


// Protótipo das funcoes deste arquivo
void Display_OLED_Inicia(int TEspera); // Inicializa o display OLED SSD1306
void Termostato_Processa(void); // Processa o controle do relé conforme o modo de operação e temperatura
static void IRAM_ATTR gpio_isr_handler(void* arg); // Tratador de interrupção dos GPIOs dos botões
void Tarefa_Botao(void *pvpameters); // Tarefa que processa eventos dos botões
void Inicia_os_GPIO(void); // Inicializa GPIOs, relé, buzzer, LEDs e botões
void RTC_DS3231_Leitura(void); // Lê dados do RTC DS3231 (não implementado)
void RTC_DS3231_Mostra_Hora(uint8_t Linha_Display); // Mostra hora no display OLED (não implementado)
void Tela_OLED_Escreve(void); // Escreve informações na tela OLED conforme menu ativo
void Processa_Botoes_Teclado(uint8_t Botao_Pressionado); // Processa navegação e ações dos botões no menu
void app_main(void); // Função principal do sistema

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
void Termostato_Processa( void )
// Processa o rele de acordo com a temperatura lida, e escreve no display OLED
{ 
    switch (GModo_Controle) 
    { // Verifica o modo de controle
        case MODO_AUTOMATICO: // Controle automático
                                    if (GRegistro1.Temperatura <= GSetPoint_Temperatura)  // Se a temperatura for menor que o set point
                                          GRele_Ativo = 1;
                                    else  GRele_Ativo = 0; 
                                    break;

        case MODO_MANUAL_LIGADO:    GRele_Ativo = 1;
                                    break;
        case MODO_MANUAL_DESLIGADO: GRele_Ativo = 0; // Desativa o relé manualmente
                                    break;      
        default: // Modo desconhecido
                                    GRele_Ativo = 0; // Desativa o relé por segurança
                                    break;      
    }   
    
  Rele_Define_Estado(&GRele_EStufa, GRele_Ativo); // Atualiza o estado do relé
}

//============== ISR handler ====================================================
static void IRAM_ATTR gpio_isr_handler(void* arg)   
{
    uint32_t Numero_Pino = (uint32_t) arg;                      //get the GPIO number
    xQueueSendFromISR(Handle_Eventos_gpio , &Numero_Pino, NULL);     //send the GPIO number to the queue
}

//=============================================================================
void Tarefa_Botao(void *pvpameters)
{
  uint32_t Numero_Pino;                                         // Numero da GPIO
  TickType_t Tempo_Botao_Pressionado = 0;                       // Armazena o instante em que o botao e pressionado   

  while (true) 
  {
    xQueueReceive(Handle_Eventos_gpio, &Numero_Pino, portMAX_DELAY); // Espera por eventos de interrupção de GPIO
    ESP_LOGV(TAG_VERSAO, "Botao GPIO[%li]", Numero_Pino);       // log do botao pressionado
    TickType_t GInstante_Inicial = xTaskGetTickCount();         // Pega o tempo atual da tarefa

    

    if (GInstante_Inicial - Tempo_Botao_Pressionado >= pdMS_TO_TICKS(250)) // Verifica se o tempo desde o último pressionamento é maior que 250ms
    {
        Tempo_Botao_Pressionado = GInstante_Inicial;                  // Atualiza o tempo do último pressionamento do botão
        Processa_Botoes_Teclado( Numero_Pino );
        Tela_OLED_Escreve();
    }
  }
}

//=============================================================================
void Inicia_os_GPIO(void)
{ // Configura os pinos dos botões como entrada e interrupcao

    // Rele
    Rele_Inicia(&GRele_EStufa, PINO_RELE);  // Inicia o componente relé
    GRele_Ativo = 0;                        // Rele inativo
    Rele_Define_Estado(&GRele_EStufa, GRele_Ativo); // Define o estado inicial do relé
    
    // buzzer
    gpio_reset_pin(PINO_BUZZER);                                       // Reseta o pino do buzzer
    gpio_set_direction(PINO_BUZZER, GPIO_MODE_OUTPUT);              // Configura o pino do buzzer como saída
    gpio_set_level(PINO_BUZZER, 0);                                 // Desliga o buzzer inicialmente

    // LEDs como saída
    int LEDs[] = {PINO_LED_F1, PINO_LED_F2, PINO_LED_R, PINO_LED_G, PINO_LED_B};
    for (int i = 0; i < sizeof(LEDs) / sizeof(LEDs[0]); i++)
    {   gpio_reset_pin(LEDs[i]);
        gpio_set_direction(LEDs[i], GPIO_MODE_OUTPUT);
        gpio_set_level(LEDs[i], 0); // Inicializa os LEDs desligados
    }

    // inicializa os botoes
    int Botoes[] = {PINO_BOTAO_ESC, PINO_BOTAO_OK, PINO_BOTAO_BAIXO, 
                    PINO_BOTAO_DIREITA, PINO_BOTAO_ESQUERDA, PINO_BOTAO_CIMA};
    for (int i = 0; i < sizeof(Botoes)/sizeof(Botoes[0]); i++) 
    {
        gpio_reset_pin    (Botoes[i]);                      //select BUTTON_PIN as GPIO
        gpio_set_direction(Botoes[i], GPIO_MODE_INPUT);     //set as input
        gpio_pullup_en    (Botoes[i]);                      //enable pull-up
        gpio_pulldown_dis (Botoes[i]);                      //disable pull-down
        gpio_set_intr_type(Botoes[i], GPIO_INTR_NEGEDGE);   //interrupt on negative edge
    };

    Handle_Eventos_gpio  = xQueueCreate(1, sizeof(uint32_t));            //create queue to handle gpio event from ISR
    xTaskCreate(Tarefa_Botao, "Tarefa_Botao", 2048, NULL, 2, NULL);     //create button task

    gpio_install_isr_service(ESP_INTR_FLAG_LEVEL1);                  //install interrupt service routine
    gpio_isr_handler_add(PINO_BOTAO_ESC,        gpio_isr_handler, (void*)PINO_BOTAO_ESC); //add ISR handler for button1
    gpio_isr_handler_add(PINO_BOTAO_OK,         gpio_isr_handler, (void*)PINO_BOTAO_OK); //add ISR handler for button2
    gpio_isr_handler_add(PINO_BOTAO_BAIXO,      gpio_isr_handler, (void*)PINO_BOTAO_BAIXO); 
    gpio_isr_handler_add(PINO_BOTAO_DIREITA,    gpio_isr_handler, (void*)PINO_BOTAO_DIREITA); 
    gpio_isr_handler_add(PINO_BOTAO_ESQUERDA,   gpio_isr_handler, (void*)PINO_BOTAO_ESQUERDA); 
    gpio_isr_handler_add(PINO_BOTAO_CIMA,       gpio_isr_handler, (void*)PINO_BOTAO_CIMA); 

}

//=============================================================================
void RTC_DS3231_Leitura(void)
{ // Leitura do RTC DS3231
  // Esta função deve ser implementada se o RTC for utilizado
  // No momento, não há implementação para o RTC
// Houve conflito com a biblioteca do display OLED
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


}

//=============================================================================
void Tela_OLED_Escreve( )
{ // Escreve a tela atual no OLED
    // Esta função deve ser implementada para escrever a tela atual no OLED

    switch (GMenu_Tela_Atual) 
    {
        case MENU_TELA_PRINCIPAL: // Menu principal
                ssd1306_display_text(&GDisplay_OLED, 0, TAG_VERSAO     , strlen ( TAG_VERSAO ), false); 
                ssd1306_display_text(&GDisplay_OLED, 1,"Rogerio IFCE" , 12, false); 

                sprintf(GAuxs, "Temperat. = %2d'C", GRegistro1.Temperatura);                       //format the temperature data   
                ssd1306_display_text(&GDisplay_OLED, 2,GAuxs,strlen(GAuxs),false);      //display the temperature data
                sprintf(GAuxs, "Umidade   = %2d %%", GRegistro1.Umidade);                        //format the humidity data
                ssd1306_display_text(&GDisplay_OLED, 3,GAuxs,strlen(GAuxs),false);      //display the humidity data
                sprintf(GAuxs, "Brilho    = %3d ", GRegistro1.Brilho);
                ssd1306_display_text(&GDisplay_OLED, 4,GAuxs,strlen(GAuxs),false);      //display the humidity data

                sprintf(GAuxs, "%02d/%02d/%04d %02d:%02d:%02d", 
                        GRegistro1.Dia, GRegistro1.Mes, GRegistro1.Ano, 
                        GRegistro1.Hora, GRegistro1.Minuto, GRegistro1.Segundo); 
//                ssd1306_display_text(&GDisplay_OLED, 5,GAuxs,strlen(GAuxs),false);      //display the date and time   

                sprintf(GAuxs, "Set Point = %2d'C", GSetPoint_Temperatura);          //formata linha do setpoint
                ssd1306_display_text(&GDisplay_OLED, 6, GAuxs, strlen(GAuxs),false);     //mostra o setpoint
                if ( GRele_Ativo)
                     ssd1306_display_text(&GDisplay_OLED, 7, "Rele: Ligado"   , 16, true); // Exibe no OLED
                else ssd1306_display_text(&GDisplay_OLED, 7, "Rele: Desligado", 16, true); // Exibe no OLED
                break;

        case MENU_TELA_SETPOINT:
                ssd1306_display_text(&GDisplay_OLED, 0, "====SETPOINT===", 16, false);
                ssd1306_display_text(&GDisplay_OLED, 2, "+: Incrementa  ", 16, false);
                ssd1306_display_text(&GDisplay_OLED, 3, "-: Decrementa  ", 16, false);
                sprintf(GAuxs, "Set Point = %2d'C", GSetPoint_Temperatura);          //formata linha do setpoint
                ssd1306_display_text(&GDisplay_OLED, 4, GAuxs, strlen(GAuxs),false);     //mostra o setpoint
                break;

        case MENU_TELA_CONTROLE:
                ssd1306_display_text(&GDisplay_OLED, 0, "===CONTROLE====", 16, false);
                ssd1306_display_text(&GDisplay_OLED, 2, "A: Automatico  ", 16, GMenu_Linha_Atual == MODO_AUTOMATICO);
                ssd1306_display_text(&GDisplay_OLED, 3, "L: Ligado      ", 16, GMenu_Linha_Atual == MODO_MANUAL_LIGADO);
                ssd1306_display_text(&GDisplay_OLED, 4, "D: Desligado   ", 16, GMenu_Linha_Atual == MODO_MANUAL_DESLIGADO);

                ssd1306_display_text(&GDisplay_OLED, 6, "<OK> Para salvar", 16, false);
                switch (GModo_Controle) { // Exibe o modo de controle atual
                    case MODO_AUTOMATICO:
                        ssd1306_display_text(&GDisplay_OLED, 7, "Modo: Automatico", 16, true);
                        break;
                    case MODO_MANUAL_LIGADO:
                        ssd1306_display_text(&GDisplay_OLED, 7, "Modo: Manual Ligado", 16, true);
                        break;
                    case MODO_MANUAL_DESLIGADO:
                        ssd1306_display_text(&GDisplay_OLED, 7, "Modo: Manual Desligado", 16, true);
                        break;
                    default:
                        ssd1306_display_text(&GDisplay_OLED, 7, "Modo: Desconhecido", 16, true);
                        break;
                }
                break;

        case MENU_TELA_ARQUIVO:
                ssd1306_display_text(&GDisplay_OLED, 0, "====ARQUIVO====", 16, true);
                ssd1306_display_text(&GDisplay_OLED, 2, "L: Lista tudo  ", 16, GMenu_Linha_Atual == 0);
                ssd1306_display_text(&GDisplay_OLED, 3, "Z: Apaga tudo  ", 16, GMenu_Linha_Atual == 1);
                ssd1306_display_text(&GDisplay_OLED, 4, "G: Ativa Gravac", 16, GMenu_Linha_Atual == 2);
                ssd1306_display_text(&GDisplay_OLED, 5, "g: Para Gravaca", 16, GMenu_Linha_Atual == 3);

                if ( GModo_Gravacao == 1) // Se o modo de gravação estiver ativo
                {   ssd1306_display_text(&GDisplay_OLED, 7, "   <Gravando>   ", 16, true);
                }
                else
                {   ssd1306_display_text(&GDisplay_OLED, 7, "Nao Gravando    ", 16, false);
                }
                break;

        default:
                ssd1306_display_text(&GDisplay_OLED, 0, "Tela Invalida  ", 16, true);
                break;
    }

}

//=============================================================================
void Processa_Botoes_Teclado( uint8_t Botao_Pressionado )
{ 

    // Navegação horizontal no menu
    if ((Botao_Pressionado == PINO_BOTAO_DIREITA) && (GMenu_Tela_Atual < MAX_MENU_TELAS-1) )
    {  GMenu_Tela_Atual++; // Incrementa a tela do menu
       GMenu_Linha_Atual = 0;
       ssd1306_clear_screen(&GDisplay_OLED,false);                  //Apaga a tela OLED
    }  
    if ((Botao_Pressionado == PINO_BOTAO_ESQUERDA) && (GMenu_Tela_Atual > 0) )
    {  GMenu_Tela_Atual--; // Decrementa a tela do menu
       GMenu_Linha_Atual = 0;
       ssd1306_clear_screen(&GDisplay_OLED,false);                  //Apaga a tela OLED
    }  
 

    // Dentro do Menu Setpoint        
    if (GMenu_Tela_Atual == MENU_TELA_SETPOINT)
    { // Se estiver na tela de setpoint, processa os botões de incremento e decremento
        if (Botao_Pressionado == PINO_BOTAO_ESC )               
        {   GSetPoint_Temperatura++;                          // Aumenta o set point de temperatura
            Termostato_Processa();
            ESTUFA_NVS_Setpoint_Grava( GSetPoint_Temperatura ); // Grava o set point na NVS
        };
        if (Botao_Pressionado == PINO_BOTAO_OK)               
        {   GSetPoint_Temperatura--;                          // Decrementa o set point de temperatura
            Termostato_Processa();
            ESTUFA_NVS_Setpoint_Grava( GSetPoint_Temperatura ); // Grava o set point na NVS
        };
    }

    // Dentro do menu Modo de controle
    if (GMenu_Tela_Atual == MENU_TELA_CONTROLE)
    { // Se estiver na tela de controle, processa os botões de controle manual
        if ((Botao_Pressionado == PINO_BOTAO_CIMA) && (GMenu_Linha_Atual > MODO_AUTOMATICO))
            GMenu_Linha_Atual--;
        if ((Botao_Pressionado == PINO_BOTAO_BAIXO) && (GMenu_Linha_Atual < MODO_MANUAL_DESLIGADO))
            GMenu_Linha_Atual++;
        if (Botao_Pressionado == PINO_BOTAO_OK)
        {   GModo_Controle = GMenu_Linha_Atual; // Atualiza o modo de controle com a linha atual do menu
            ESP_LOGI(TAG_VERSAO, "Modo controle = %d", GModo_Controle);
            ESTUFA_NVS_Controle_Grava( GModo_Controle );        // Grava o modo de controle na NVS
        };
        if (Botao_Pressionado == PINO_BOTAO_ESC)
        {   GMenu_Linha_Atual = 0; 
            GMenu_Tela_Atual  = 0;
        };
    }

   // Dentro do menu Arquivo
    if (GMenu_Tela_Atual == MENU_TELA_ARQUIVO)
    { 
        if ((Botao_Pressionado == PINO_BOTAO_CIMA) && (GMenu_Linha_Atual > 0))
            GMenu_Linha_Atual--;
        if ((Botao_Pressionado == PINO_BOTAO_BAIXO) && (GMenu_Linha_Atual < 3))
            GMenu_Linha_Atual++;
        if (Botao_Pressionado == PINO_BOTAO_OK)
        {   switch (GMenu_Linha_Atual) 
            { // Verifica a linha atual do menu
                case 0: // Lista tudo
                    GModo_Gravacao = 0; // Desativa o modo de gravação
                    NVS_Lista_Entradas(); // Lista os valores armazenados na NVS
                    ESP_LOGI(TAG_VERSAO, "Listando os valores armazenados");
                    NVS_Datalogger_Lista_Tudo( );
                    break;
                case 1: // Apaga tudo
                    ESP_LOGI(TAG_VERSAO, "Apagando todos valores armazenados");
                    GModo_Gravacao = 0; // Desativa o modo de gravação
                    NVS_Apaga_Tudo(); // Apaga tudo da NVS
                    break;
                case 2: // Ativa gravação
                    ESP_LOGI(TAG_VERSAO, "Ativando modo de gravação");
                    GModo_Gravacao = 1; // Ativa o modo de gravação
                    break;
                case 3: // Para gravação
                    ESP_LOGI(TAG_VERSAO, "Desativando modo de gravação");
                    GModo_Gravacao = 0; // Desativa o modo de gravação
                    break;
                default:
                    ESP_LOGW(TAG_VERSAO, "Opção inválida no menu Arquivo");
                    break;
            }
        };


        if (Botao_Pressionado == PINO_BOTAO_ESC)
        {   GMenu_Linha_Atual = 0; 
            GMenu_Tela_Atual  = 0;
        };
    }
}

//=============================================================================
//=============================================================================
//=============================================================================
void app_main(void)
{   uint8_t Piscada = 0;             // Pisca no LCD para indicar que o sistema esta funcionando
    long Instante_Transicao_Rele;       // Instante da transição do relé       
    uint8_t AuxC;


    ESTUFA_NVS_Inicializar();
    Display_OLED_Inicia( 1 );
//NVS_Apaga_Tudo( );
    NVS_Lista_Entradas( );
    LAB01_LDR_Inicializa();
    Inicia_os_GPIO();                                   // Inicia as entradas e saídas do GPIO
    ESTUFA_NVS_Setpoint_Le( &GSetPoint_Temperatura );   // Le o setpoint da NVS
    ESTUFA_NVS_Controle_Le( &GModo_Controle );          // Le o modo de controle da NVS

    GMenu_Tela_Atual = 0;
    Tela_OLED_Escreve();
 
    GInstante_Inicial = xTaskGetTickCount();
    ESP_LOGI(TAG_VERSAO, "Iniciando o sistema Estufa r1.01");   
    ESP_LOGI(TAG_VERSAO, "Instante = %ld", GInstante_Inicial);

    while(1)
    {   LAB01_DHT11_Leitura( PINO_DHT11,  &GRegistro1.Temperatura, &GRegistro1.Umidade); // Le o DHT11
        LAB01_LDR_Ler( &GRegistro1.Brilho );                                             // Le o LDR
        RTC_DS3231_Leitura();  
        Tela_OLED_Escreve();
        Termostato_Processa( );                               // Processa o rele de acordo com a temperatura lida

        if ( GRele_Estado_Anterior != GRele_Ativo)  // Rele mudou de estado
        {   GRele_Estado_Anterior = GRele_Ativo; // Atualiza o estado anterior do relé
            Instante_Transicao_Rele = xTaskGetTickCount(); // Armazena o instante da transição do relé
            ESP_LOGI(TAG_VERSAO, "Rele mudou de estado em: %ld ms", Instante_Transicao_Rele - GInstante_Inicial); // Loga a transição do relé
        }

        if (GModo_Controle == MODO_AUTOMATICO)
        {    ESP_LOGI(TAG_VERSAO,"Temperatura: %d'C, Umidade: %d%%, Brilho: %d, SP = %d'C, Rele = %d", GRegistro1.Temperatura, GRegistro1.Umidade, GRegistro1.Brilho, GSetPoint_Temperatura, GRele_Ativo); 
        };
        gpio_set_level(PINO_LED_F1, Piscada = !Piscada);
      
        if ( GModo_Gravacao == 1) // Se o modo de gravação estiver ativo
        {  sprintf(GAuxs, "[%ld ms] T= %d'C, U= %d%%, B= %d", xTaskGetTickCount(),
                         GRegistro1.Temperatura, GRegistro1.Umidade, GRegistro1.Brilho);
            ESP_LOGI(TAG_VERSAO, "%s", GAuxs); // Loga os dados lidos
            NVS_Datalogger_Grava_Linha( GAuxs);
        }

        // Processa leitura pela porta serial de comando
        AuxC = getchar();
        if (AuxC > ' ')
        { switch (AuxC)
        {  case '+': // Incrementa o setpoint
                        GSetPoint_Temperatura++;
                        Termostato_Processa();
                        ESTUFA_NVS_Setpoint_Grava( GSetPoint_Temperatura ); // Grava o setpoint na NVS
                        ESP_LOGI(TAG_VERSAO, "Setpoint incrementado para %d'C", GSetPoint_Temperatura);
                        break;
            case '-': // Decrementa o setpoint
                        GSetPoint_Temperatura--;
                        Termostato_Processa();
                        ESTUFA_NVS_Setpoint_Grava( GSetPoint_Temperatura ); // Grava o setpoint na NVS
                        ESP_LOGI(TAG_VERSAO, "Setpoint decrementado para %d'C", GSetPoint_Temperatura);
                        break;
            case 'a':
            case 'A': // Modo automático
                        GModo_Controle = MODO_AUTOMATICO;
                        Termostato_Processa();
                        ESTUFA_NVS_Controle_Grava( GModo_Controle );        // Grava o modo de controle na NVS
                        ESP_LOGI(TAG_VERSAO, "Modo de controle alterado para automático");
                        break;
            case 'l':
            case 'L': // Modo manual ligado
                        GModo_Controle = MODO_MANUAL_LIGADO;
                        Termostato_Processa();
                        ESTUFA_NVS_Controle_Grava( GModo_Controle );        // Grava o modo de controle na NVS
                        ESP_LOGI(TAG_VERSAO, "Modo de controle alterado para manual ligado");
                        break;
            case 'd':
            case 'D': // Modo manual desligado
                        GModo_Controle = MODO_MANUAL_DESLIGADO;
                        Termostato_Processa();
                        ESTUFA_NVS_Controle_Grava( GModo_Controle );        // Grava o modo de controle na NVS
                        ESP_LOGI(TAG_VERSAO, "Modo de controle alterado para manual desligado");
                        break;
            case 'G': // Ativa gravação
                        GModo_Gravacao = 1; // Ativa o modo de gravação
                        ESP_LOGI(TAG_VERSAO, "Modo de gravação ativado");
                        break;
            case 'g': // Desativa gravação
                        GModo_Gravacao = 0; // Desativa o modo de gravação
                        ESP_LOGI(TAG_VERSAO, "Modo de gravação desativado");
                        break;
            case 'Z': // Apaga tudo da NVS
                        ESP_LOGI(TAG_VERSAO, "Apagando todos valores armazenados na NVS");
                        GModo_Gravacao = 0; // Desativa o modo de gravação
                        NVS_Apaga_Tudo(); // Apaga tudo da NVS
                        break;
            case 'T': // Lista tudo da NVS
                        GModo_Gravacao = 0; // Desativa o modo de gravação
                        ESP_LOGI(TAG_VERSAO, "Listando todos valores armazenados na NVS");
                        NVS_Lista_Entradas(); // Lista os valores armazenados na NVS
                        ESP_LOGI(TAG_VERSAO, "Listando os valores armazenados");
                        NVS_Datalogger_Lista_Tudo( );
                        break;

        } // Fim do switch
        } // Fim do if AuxC > ' '
        vTaskDelay(pdMS_TO_TICKS(TEMPO_ENTRE_LEITURAS));                                        //delay             
    }
}
