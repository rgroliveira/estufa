void func(void);
// Funções para ler e armazenar dados do projeto na memória NVS do ESP32

#if !defined  VARIAVEL_REGISTRO
#define VARIAVEL_REGISTRO 1 
  
#endif

#ifndef RMEMORIA_NVS_H
#define RMEMORIA_NVS_H

#include <stdint.h>

// Protótipos das funções
void NVS_Le_SSID(void);                                  // Função para ler o SSID na NVS 
void NVS_Le_Senha(void);                                 // Função para ler a senha na NVS
void NVS_Lista_Entradas(void);                           // Lista todas as entradas da NVS
void NVS_Apaga_Tudo(void);                               // Função para apagar tudo da NVS
void NVS_Datalogger_Grava_Linha(const char* Linha);      // Grava uma linha de dados na NVS
void NVS_Datalogger_Lista_Tudo(void);                    // Lista tudo da NVS_CHAVE_REGISTRO que está armazenado na NVS

void ESTUFA_NVS_Inicializar(void);                       // Initializa a memória NVS
void ESTUFA_NVS_Setpoint_Le(int8_t *SetPoint_Memoria);   // Função para ler o setpoint na NVS
void ESTUFA_NVS_Setpoint_Grava(int8_t SetPoint_Memoria); // Função para gravar o setpoint na NVS
void ESTUFA_NVS_Controle_Le(int8_t *Modo);               // Função para ler o Modo de controle na NVS
void ESTUFA_NVS_Controle_Grava(int8_t Modo);             // Função para gravar o modo de controle na NVS

#endif // RMEMORIA_NVS_H


//===============================================================



