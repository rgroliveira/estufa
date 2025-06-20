void func(void);
// Funções para ler e armazenar dados do projeto na memória NVS do ESP32


//===============================================================
void ESTUFA_NVS_Inicializar(void);                             // Initializa a memória NVS
void ESTUFA_NVS_Setpoint_Le( int8_t *SetPoint_Memoria);        // Função para ler o setpoint na NVS
void ESTUFA_NVS_Setpoint_Grava( int8_t SetPoint_Memoria);      // Função para gravar o setpoint na NVS
void ESTUFA_NVS_Controle_Le( int8_t *Controle_Memoria);
void ESTUFA_NVS_Controle_Grava( int8_t Controle_Memoria);
void NVS_Le_SSID( void);                                       // Função para ler o SSID na NVS
void NVS_Le_Senha(void);                                       // Função para ler a senha na NVS


