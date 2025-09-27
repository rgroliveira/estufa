// Componente com os sensores DHT11 e LDR do LAB01 do LAB01 Franzininho WiFi. 
// 
// Autor: Rogerio Oliveira
// Data: 09/2025


#ifndef COMPONENTE_LAB01_SENSORES_H
#define COMPONENTE_LAB01_SENSORES_H

uint8_t LAB01_DHT11_Leitura( int Pino_DHT11,  int16_t *Temperatura, int16_t *Umidade); // Lê o sensor DHT11 da placa LAB01, retorna 1 se ok, 0 se erro
void LAB01_LDR_Inicializa(void); //inicializa o ADC para leitura do LDR da placa LAB01
void LAB01_LDR_Ler( int16_t *Tensao_Lida); //Le o LDR pelo ADC, retorna a tensao lida em mV

#endif