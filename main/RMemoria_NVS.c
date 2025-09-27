#include <stdio.h>
#include "RMemoria_NVS.h"


#include <string.h>
#include <inttypes.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "nvs_flash.h" // Biblioteca NVS para armazenamento de dados
#include "nvs.h"
#include "esp_log.h"


const char *TAG_NVS = "Memoria NVS";                                                 //define the tag for the log
    
//int16_t     GSetPoint_Temperatura   = CONFIG_ESTUFA_SETPOINT_DEFAULT; // definido no menuconfig
#define CONFIG_ESTUFA_CONTROLE_DEFAULT  0
#define NVS_NOME_PARTICAO "ParticaoNVS" // define the name of the NVS partition
#define NVS_CHAVE_MODO_CONTROLE "ModoControle" // define the key for the control mode in NVS
#define NVS_CHAVE_SET_POINT "set_point" // define the key for the set point in NVS
#define NVS_CHAVE_REGISTRO "DadosEstufa" // Armazena o registro de temperatura e umidade


char ssid[32];                                                               //variable to store the ssid
char GSenha[32];                                                          //variable to store the password


//===============================================================
void ESTUFA_NVS_Inicializar(void)
{ // Initializa a memória NVS
  esp_err_t Erro;
  
    Erro = nvs_flash_init();                                              // inicializa a partição padrão da NVS
    ESP_LOGI(TAG_NVS, "Inicializando a NVS");                                       // Registra a ação
    if (Erro == ESP_ERR_NVS_NO_FREE_PAGES || Erro == ESP_ERR_NVS_NEW_VERSION_FOUND) // se a partição estiver truncada, apague-a e tente novamente
    {    // A partição NVS foi truncada e precisa ser apagada
        ESP_ERROR_CHECK(nvs_flash_erase());                                         // apaga a partição
        Erro = nvs_flash_init();                                                    // inicializa a partição padrão da NVS
    }
    ESP_ERROR_CHECK(Erro);                                                        // verifica se a inicialização foi bem-sucedida
}

//===============================================================
void ESTUFA_NVS_Setpoint_Le( int8_t *SetPoint_Memoria)
{  // Função para ler o setpoint na NVS
    esp_err_t    Erro;
    nvs_handle_t nvs_handle;                                                                //create a handle to the nvs
  
    //read setpoint from NVS
    Erro = nvs_open(NVS_NOME_PARTICAO, NVS_READWRITE, &nvs_handle);                                  //open the nvs partition
    if (Erro != ESP_OK)                                                                      //check if the partition was opened successfully
    {
        //log the error
        ESP_LOGE(TAG_NVS, "Error (%s) opening NVS handle!\n", esp_err_to_name(Erro));            //log the error
    } 
    else 
    {
        ESP_LOGI(TAG_NVS, "Reading setpoint from NVS ... ");                                    //log the action
        Erro = nvs_get_i8(nvs_handle, NVS_CHAVE_SET_POINT, SetPoint_Memoria);                                //read the setpoint from the nvs
        switch (Erro)                                                                        //check the return of the function
        {
            case ESP_OK:                                                                    //if the function returns ESP_OK
                ESP_LOGI(TAG_NVS, "Done");                                                    //log the action
                ESP_LOGI(TAG_NVS, "Setpoint = %" PRIu8, *SetPoint_Memoria);                              //log the value read
                break;
            case ESP_ERR_NVS_NOT_FOUND:                                                     //if the value was not found
                ESP_LOGI(TAG_NVS, "A variavel 'SetPoint' ainda não foi inicializada!");                        //log the action
                ESP_LOGI(TAG_NVS, "Initializing setpoint to %d", CONFIG_ESTUFA_SETPOINT_DEFAULT);            //log the action
                *SetPoint_Memoria = CONFIG_ESTUFA_SETPOINT_DEFAULT;                                                //initialize the setpoint
                //Write setpoint to NVS
                ESP_LOGI(TAG_NVS, "Gravando setpoint para NVS ... ");                              //log the action
                Erro = nvs_set_i8(nvs_handle, NVS_CHAVE_SET_POINT, *SetPoint_Memoria);                         //write the setpoint to the nvs
                ESP_LOGI(TAG_NVS, "%s",(Erro != ESP_OK) ? "Falhou!" : "Feito");                    //log the action
                Erro = nvs_commit(nvs_handle);                                           //commit the changes to the nvs
                ESP_LOGI(TAG_NVS, "%s",(Erro != ESP_OK) ? "Falhou!" : "Feito");               //log the action
                break;
            default :
                ESP_LOGI(TAG_NVS, "Error (%s) reading!", esp_err_to_name(Erro));               //log the error
        }
        nvs_close(nvs_handle);                                                              //close the nvs handle    
    }

}   

//===============================================================
void ESTUFA_NVS_Setpoint_Grava( int8_t SetPoint_Memoria)
{ // Função para gravar o setpoint na NVS
    esp_err_t    Erro;
    nvs_handle_t nvs_handle;                                                                //create a handle to the nvs
  
    //write setpoint to NVS
    Erro = nvs_open(NVS_NOME_PARTICAO, NVS_READWRITE, &nvs_handle);                                  //open the nvs partition
    if (Erro != ESP_OK)                                                                      //check if the partition was opened successfully
    {
        ESP_LOGE(TAG_NVS, "Error (%s) opening NVS handle!\n", esp_err_to_name(Erro));            //log the error
    } 
    else 
    {
        ESP_LOGI(TAG_NVS, "Escrevendo setpoint na memoria NVS ... ");
        Erro = nvs_set_i8(nvs_handle, NVS_CHAVE_SET_POINT, SetPoint_Memoria);
        if (Erro != ESP_OK) 
           ESP_LOGE(TAG_NVS, "Error (%s) writing setpoint!", esp_err_to_name(Erro));            //log the error
        //ESP_LOGI(TAG_NVS, "%s",(Erro != ESP_OK) ? "Failed!" : "Done");                            //log the action
        
        Erro = nvs_commit(nvs_handle);                                           //commit the changes to the nvs
        if (Erro != ESP_OK) 
           ESP_LOGE(TAG_NVS, "Error (%s) committing setpoint!", esp_err_to_name(Erro));            //log the error
        //ESP_LOGI(TAG_NVS, "%s",(Erro != ESP_OK) ? "Failed!" : "Done");               //log the action
        nvs_close(nvs_handle);                                                              //close the nvs handle    
    }
}

//===============================================================
void NVS_Le_SSID( void)
{  // Função para ler o SSID na NVS
    esp_err_t    Erro;
    nvs_handle_t nvs_handle;                                                                //create a handle to the nvs
  
    //read ssid from NVS using string
    Erro = nvs_open(NVS_NOME_PARTICAO, NVS_READWRITE, &nvs_handle);                                      //open the nvs partition
    if (Erro != ESP_OK)                                                                          //check if the partition was opened successfully
    {
        //log the error
        ESP_LOGE(TAG_NVS, "Error (%s) opening NVS handle!", esp_err_to_name(Erro));                //log the error
    } 
    else 
    {

        ESP_LOGI(TAG_NVS, "Reading SSID from NVS ... ");                                            //log the action
        size_t required_size = sizeof(ssid);                                                    //get the size of the ssid string
        Erro = nvs_get_str(nvs_handle, "ssid", ssid, &required_size);                            //read the ssid from the nvs
        switch (Erro)                                                                            //check the return of the function
        {
            case ESP_OK:                                                                        //if the function returns ESP_OK
                ESP_LOGI(TAG_NVS, "Done");                                                          //log the action
                ESP_LOGI(TAG_NVS, "SSID = %s", ssid);                                               //log the value read
                break;
            case ESP_ERR_NVS_NOT_FOUND:                                                         //if the value was not found
                ESP_LOGI(TAG_NVS, "The value is not initialized yet!");                             //log the action
                ESP_LOGI(TAG_NVS, "Initializing SSID");                                             //log the action
                strcpy(ssid, "MySSID");
                //Write SSID to NVS
                ESP_LOGI(TAG_NVS, "Writing SSID to NVS ... ");                                      //log the action
                Erro = nvs_set_str(nvs_handle, "ssid", ssid);                                    //write the ssid to the nvs
                ESP_LOGI(TAG_NVS, "%s",(Erro != ESP_OK) ? "Failed!" : "Done");                        //log the action   
                Erro = nvs_commit(nvs_handle);                                           //commit the changes to the nvs
                ESP_LOGI(TAG_NVS, "%s",(Erro != ESP_OK) ? "Failed!" : "Done");               //log the action                         
                break;
            default :
                ESP_LOGI(TAG_NVS, "Error (%s) reading!", esp_err_to_name(Erro));                     //log the error
        }
        nvs_close(nvs_handle);
    }
}

//===============================================================
void    NVS_Le_Senha()
{ // Função para ler a senha na NVS
    esp_err_t    Erro;
    nvs_handle_t nvs_handle;                                                                //create a handle to the nvs
    //Le uma"senha da NVS usando string
    Erro = nvs_open(NVS_NOME_PARTICAO, NVS_READWRITE, &nvs_handle);                                      //open the nvs partition
    if (Erro != ESP_OK)                                                                          //check if the partition was opened successfully
    {
        //log the error
        ESP_LOGE(TAG_NVS, "Error (%s) opening NVS handle!", esp_err_to_name(Erro));                //log the error
    } 
    else 
    {
        ESP_LOGI(TAG_NVS, "Lendo GSenha da NVS ... ");                                            //Registra a ação
        size_t required_size = sizeof(GSenha);                                                //Pega o tamanho da string GSenha
        Erro = nvs_get_str(nvs_handle, "GSenha", GSenha, &required_size);                      //Lê a senha da NVS
        switch (Erro)                                                                          //Checa o retorno da função
        {
            case ESP_OK:                                                                        //Se a função retorna ESP_OK
                ESP_LOGI(TAG_NVS, "Done");                                                          //log para ação
                ESP_LOGI(TAG_NVS, "GSenha = %s", GSenha);                                           // Log para o valor lido
                break;
            case ESP_ERR_NVS_NOT_FOUND:                                                         //Se o valor não foi encontrado
                ESP_LOGI(TAG_NVS, "Este valor ainda não está inicializado!");                       //Log para ação
                ESP_LOGI(TAG_NVS, "Inicializando GSenha");                                          //Log para ação
                strcpy(GSenha, "0123456789");
                //Escreve GSenha na NVS
                ESP_LOGI(TAG_NVS, "Escrevendo GSenha na NVS ... ");                                 //Log para ação
                Erro = nvs_set_str(nvs_handle, "GSenha", GSenha);                                    //escreve a senha na NVS
                ESP_LOGI(TAG_NVS, "%s",(Erro != ESP_OK) ? "Falhou!" : "Feito");                      //Log para ação
                Erro = nvs_commit(nvs_handle);                                           //commit para as mudanças na NVS
                ESP_LOGI(TAG_NVS, "%s",(Erro != ESP_OK) ? "Falhou!" : "Feito");                      //Log para ação
                break;
            default :
                ESP_LOGI(TAG_NVS, "Erro (%s) ao ler!", esp_err_to_name(Erro));                       //Log para ação
        }
        nvs_close(nvs_handle);
    }

};

//===============================================================
void ESTUFA_NVS_Controle_Le( int8_t *Modo)
{  // Função para ler o Modo de controle na NVS
    esp_err_t    Erro;
    nvs_handle_t nvs_handle;     
   
    Erro = nvs_open(NVS_NOME_PARTICAO, NVS_READWRITE, &nvs_handle);                                  //open the nvs partition
    if (Erro != ESP_OK)                                                                      //check if the partition was opened successfully
    {
        //log the error
        ESP_LOGE(TAG_NVS, "Error (%s) opening NVS handle!\n", esp_err_to_name(Erro));            //log the error
    } 
    else 
    {
        ESP_LOGI(TAG_NVS, "Lendo o modo de controle na NVS ... ");                                    //log the action
        Erro = nvs_get_i8(nvs_handle, NVS_CHAVE_MODO_CONTROLE , Modo);                                
        switch (Erro)                                                                   
        {
            case ESP_OK:                                                                
                ESP_LOGI(TAG_NVS, "Feito");                                             
                ESP_LOGI(TAG_NVS, "Modo controle = %" PRIu8, *Modo);                    
                break;
            case ESP_ERR_NVS_NOT_FOUND:                                                 
                ESP_LOGI(TAG_NVS, "A variavel 'ModoControle' ainda não foi inicializada!" );         
                ESP_LOGI(TAG_NVS, "Inicializando o Mode de Controle para %d", CONFIG_ESTUFA_CONTROLE_DEFAULT); 
                *Modo = CONFIG_ESTUFA_CONTROLE_DEFAULT;                                                

                ESP_LOGI(TAG_NVS, "Escrevendo o modo de controle na NVS ... ");
                Erro = nvs_set_i8(nvs_handle, NVS_CHAVE_MODO_CONTROLE, *Modo);
                ESP_LOGI(TAG_NVS, "%s",(Erro != ESP_OK) ? "Failed!" : "Done");
                Erro = nvs_commit(nvs_handle);                                
                ESP_LOGI(TAG_NVS, "%s",(Erro != ESP_OK) ? "Failed!" : "Done");
                break;
            default :
                ESP_LOGI(TAG_NVS, "Error (%s) reading!", esp_err_to_name(Erro));
        }
        nvs_close(nvs_handle);                                                              //close the nvs handle    
    }

}   

//===============================================================
void ESTUFA_NVS_Controle_Grava( int8_t Modo)
{ // Função para gravar o modo de controle na NVS
    esp_err_t    Erro;
    nvs_handle_t nvs_handle;                                                                //create a handle to the nvs
  
    //Escreve modo de controle na NVS
    Erro = nvs_open(NVS_NOME_PARTICAO, NVS_READWRITE, &nvs_handle);                                  //open the nvs partition
    if (Erro != ESP_OK)                                                                      //check if the partition was opened successfully
    {
        //log the error
        ESP_LOGE(TAG_NVS, "Error (%s) opening NVS handle!\n", esp_err_to_name(Erro));            //log the error
    } 
    else 
    {
        ESP_LOGI(TAG_NVS, "Escrevendo o modo de controle para NVS: %d", Modo);                                    //log the action
        Erro = nvs_set_i8(nvs_handle, NVS_CHAVE_MODO_CONTROLE, Modo);                                //write the setpoint to the nvs
        ESP_LOGI(TAG_NVS, "%s",(Erro != ESP_OK) ? "Failed!" : "Done");                            //log the action
        Erro = nvs_commit(nvs_handle);                                           //commit the changes to the nvs
        ESP_LOGI(TAG_NVS, "%s",(Erro != ESP_OK) ? "Failed!" : "Done");               //log the action
        nvs_close(nvs_handle);                                                              //close the nvs handle    
    }
}


//===============================================================
void NVS_Lista_Entradas( void)
{
    // Atualizado para nova assinatura da função nvs_entry_find
    nvs_iterator_t it = NULL;
    esp_err_t err = nvs_entry_find("nvs", NULL, NVS_TYPE_ANY, &it);
    while (err == ESP_OK && it != NULL) 
    {
        nvs_entry_info_t info;
        nvs_entry_info(it, &info);
        ESP_LOGI(TAG_NVS, "Entry info: key=%s, type=%d", info.key, info.type);
        nvs_entry_next(&it);
    }    
}

//===============================================================
void NVS_Apaga_Tudo( void)
{ // Função para apagar tudo da NVS
    esp_err_t Erro;
    nvs_handle_t nvs_handle;                                                                //create a handle to the nvs
  
    Erro = nvs_open(NVS_NOME_PARTICAO, NVS_READWRITE, &nvs_handle);                                  //open the nvs partition
    if (Erro != ESP_OK)                                                                      //check if the partition was opened successfully
    {
        //log the error
        ESP_LOGE(TAG_NVS, "Error (%s) opening NVS handle!\n", esp_err_to_name(Erro));            //log the error
    } 
    else 
    {
        ESP_LOGI(TAG_NVS, "Apagando tudo da NVS ... ");                                    //log the action
        Erro = nvs_erase_all(nvs_handle);                                           //commit the changes to the nvs
        ESP_LOGI(TAG_NVS, "%s",(Erro != ESP_OK) ? "Failed!" : "Done");               //log the action
        nvs_close(nvs_handle);                                                              //close the nvs handle    
    }
}


//===============================================================
// NVS_CHAVE_REGISTRO
void NVS_Datalogger_Grava_Linha ( const char* Linha)
{ // Grava uma linha de dados na NVS
    esp_err_t    Erro;
    nvs_handle_t nvs_handle;                                                                //create a handle to the nvs
  
    //Escreve a linha na NVS
    Erro = nvs_open(NVS_NOME_PARTICAO, NVS_READWRITE, &nvs_handle);                                  //open the nvs partition
    if (Erro != ESP_OK)                                                                      //check if the partition was opened successfully
    {
        //log the error
        ESP_LOGE(TAG_NVS, "Error (%s) opening NVS handle!\n", esp_err_to_name(Erro));            //log the error
    } 
    else 
    {
        ESP_LOGI(TAG_NVS, "Gravando linha na NVS: %s", Linha);                           //log the action
        Erro = nvs_set_str(nvs_handle, NVS_CHAVE_REGISTRO, Linha);                 //write the setpoint to the nvs
        ESP_LOGI(TAG_NVS, "%s",(Erro != ESP_OK) ? "Failed!" : "Done");                       //log the action
        Erro = nvs_commit(nvs_handle);                                           //commit the changes to the nvs
        ESP_LOGI(TAG_NVS, "%s",(Erro != ESP_OK) ? "Failed!" : "Done");               //log the action
        nvs_close(nvs_handle);                                                              //close the nvs handle    
    }
} 

//===============================================================
void NVS_Datalogger_Lista_Tudo( void)
{ // Lista tudo da NVS_CHAVE_REGISTRO que está armazenado na NVS
    esp_err_t    Erro;
    nvs_handle_t nvs_handle;                                                                //create a handle to the nvs
    char Linha[128];                                                                        // Cria uma chave para o registro
    size_t required_size = sizeof(Linha);                                                   // Pega o tamanho do registro
    //Lê o registro da NVS
    Erro = nvs_open(NVS_NOME_PARTICAO, NVS_READWRITE, &nvs_handle);                                  //open the nvs partition
    if (Erro != ESP_OK)                                                                      //check if the partition was opened successfully
    {
        //log the error
        ESP_LOGE(TAG_NVS, "Error (%s) opening NVS handle!\n", esp_err_to_name(Erro));            //log the error
        return;
    } 
    ESP_LOGI(TAG_NVS, "Lendo tudo da NVS_CHAVE_REGISTRO ... ");                                    //log the action         
    Erro = nvs_get_str(nvs_handle, NVS_CHAVE_REGISTRO, Linha, &required_size);                    //read the setpoint from the nvs
    switch (Erro)                                                                   
    {
        case ESP_OK:                                                                
            ESP_LOGI(TAG_NVS, "Feito");                                            
            ESP_LOGI(TAG_NVS, "Linha = %s", Linha);                                
            break;
        case ESP_ERR_NVS_NOT_FOUND:                                                 
            ESP_LOGI(TAG_NVS, "A variavel 'NVS_CHAVE_REGISTRO' ainda não foi inicializada!" );         
            break;
        default :
            ESP_LOGI(TAG_NVS, "Error (%s) reading!", esp_err_to_name(Erro));
    }
    nvs_close(nvs_handle);                                                              //close the nvs handle  
                

}