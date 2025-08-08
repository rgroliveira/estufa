void func(void);
// Funções para ler e armazenar dados do projeto na memória NVS do ESP32

#if !defined  VARIAVEL_REGISTRO
#define VARIAVEL_REGISTRO 1 
  

  
#endif

//===============================================================
void ESTUFA_NVS_Inicializar(void);                             // Initializa a memória NVS
void ESTUFA_NVS_Setpoint_Le( int8_t *SetPoint_Memoria);        // Função para ler o setpoint na NVS
void ESTUFA_NVS_Setpoint_Grava( int8_t SetPoint_Memoria);      // Função para gravar o setpoint na NVS
void ESTUFA_NVS_Controle_Le( int8_t *Modo);
void ESTUFA_NVS_Controle_Grava( int8_t Modo);
void NVS_Le_SSID( void);                                       // Função para ler o SSID na NVS
void NVS_Le_Senha(void);                                       // Função para ler a senha na NVS
void NVS_Lista_Entradas( void);
void NVS_Apaga_Tudo( void);

//void ESTUFA_NVS_Registro_Grava( Tipo_Registro *Registro);
//void ESTUFA_NVS_Registro_Le( int32_t Indice, Tipo_Registro *Registro);

void NVS_Datalogger_Grava_Linha ( const char* Linha);
void NVS_Datalogger_Lista_Tudo( void);


