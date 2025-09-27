#include <stdio.h>
#include "Componente_LAB01_Sensores.h"
#include "dht.h"      // Biblioteca DHT para DHT11
#include "esp_log.h"

#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"

#define TAG_LAB01_DHT11 "LAB01_DHT11"
#define TAG_LAB01_LDR   "LAB01_LDR"

//Configuração do DHT11
#define LAB01_DHT11_TIPO DHT_TYPE_DHT11
// Configuração do ADC
adc_oneshot_unit_handle_t Manipulador_ADC1;  


//=============================================================================
uint8_t LAB01_DHT11_Leitura( int Pino_DHT11,  int16_t *Temperatura, int16_t *Umidade)
// Lê o sensor DHT11 da placa LAB01, retorna 1 se ok, 0 se erro
{ int16_t Temperatura10 = 0;       //Temperatura * 10
  int16_t Umidade10 = 0;           //Umidade * 10
 
  if(dht_read_data(LAB01_DHT11_TIPO, Pino_DHT11, &Umidade10, &Temperatura10)==ESP_OK)      //read the data from the sensor
    {   
        *Temperatura = Temperatura10/10;    //Armazena o valor da temperatura
        *Umidade = Umidade10/10;            //Armazena o valor da umidade
        return(1);
    }
    else
    {
        ESP_LOGE(TAG_LAB01_DHT11, "Nao pude ler dados do sensor DHT11");
        *Temperatura = 0;
        *Umidade = 0;   
        return(0);
    }
}
//=============================================================================
void LAB01_LDR_Inicializa(void)
{ //inicializa o ADC para leitura do LDR da placa LAB01
    adc_oneshot_unit_init_cfg_t G_ADC1_Config = 
    { //ADC1 Config
        .unit_id = ADC_UNIT_1,                                          //ADC1
    };
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&G_ADC1_Config, &Manipulador_ADC1)); //ADC1 Init

//---------- Configura o ADC ----------------------
    adc_oneshot_chan_cfg_t config = 
    { //ADC1 Channel Config
        .bitwidth = ADC_BITWIDTH_DEFAULT,                                                   //ADC1 Bitwidth (Default)
        .atten = ADC_ATTEN_DB_12,                                                           //ADC1 Attenuation 
    };
    ESP_ERROR_CHECK(adc_oneshot_config_channel(Manipulador_ADC1, ADC_CHANNEL_0, &config));       
}
 
//=============================================================================
void  LAB01_LDR_Ler( int16_t *Tensao_Lida)
{//Le o LDR pelo ADC, retorna a tensao lida em mV
     static int Valor_Bruto;     //ADC Raw Data

    ESP_ERROR_CHECK(adc_oneshot_read(Manipulador_ADC1, ADC_CHANNEL_0, &Valor_Bruto));   // Le o ADC1 Channel 0
    *Tensao_Lida = (Valor_Bruto * 2500)/8192;                                      //Calcula a tensao em mV (2.5V/8192)  (ADC 13 bits)
    ESP_LOGV (TAG_LAB01_LDR, "ADC%d Channel[%d] Dado bruto: %d, Tensao: %d mV", ADC_UNIT_1 + 1, ADC_CHANNEL_0, Valor_Bruto, *Tensao_Lida); //Debug

}

//=============================================================================