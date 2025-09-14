#include <stdio.h>
#include "Componente_LAB01_Sensores.h"
#include "dht.h"      // Biblioteca DHT para DHT11

//Configuração do DHT11
// static const dht_sensor_type_t LAB01_DHT11_TIPO = DHT_TYPE_DHT11;
#define LAB01_DHT11_TIPO DHT_TYPE_DHT11
#define TAG_VERSAO "LAB01_Sensores"


//=============================================================================
uint8_t LAB01_DHT11_Leitura( int Pino_DHT11,  int16_t *Temperatura, int16_t *Umidade)
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
        // ESP_LOGE(TAG_VERSAO, "Nao pude ler dados do sensor DHT11");
        *Temperatura = 0;
        *Umidade = 0;   
        return(0);
    }
}
//=============================================================================
