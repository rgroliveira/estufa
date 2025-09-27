// components/Componente_Rele/Componente_Rele.c
// Componente para controle de relé
// Autor: Rogerio da Silva Oliveira 09/2025
// Baseado no exempo de Fábio Souza - embarcados.com.br

#include <stdio.h>
#include "Componente_Rele.h"
#include "driver/gpio.h"

// =============================================================================
void Rele_Inicia(Tipo_Rele *Rele, int Pino) 
{// Função de inicialização do componente relé, define o pino e o estado inicial
    Rele->Pino = Pino;
    Rele->Estado = 0;
    gpio_set_direction(Rele->Pino, GPIO_MODE_OUTPUT);
    gpio_set_level(Rele->Pino, Rele->Estado);
}

// =============================================================================
void Rele_Liga(Tipo_Rele *Rele)             
{ // Liga o relé, definindo o estado como 1
    Rele->Estado = 1;
    gpio_set_level(Rele->Pino, Rele->Estado);
}

// =============================================================================
void Rele_Desliga(Tipo_Rele *Rele) 
{  // Desliga o relé, definindo o estado como 0
    Rele->Estado = 0;
    gpio_set_level(Rele->Pino, Rele->Estado);
}

// =============================================================================
int Rele_Estado(Tipo_Rele *Rele) 
{ // Retorna o estado atual do relé, 1 para ligado, 0 para desligado
    return Rele->Estado;
}

// =============================================================================
void Rele_Define_Estado(Tipo_Rele *Rele, int Estado) 
{ // Define o estado do relé (1 para ligado, 0 para desligado)
    Rele->Estado = Estado ? 1 : 0; // Garante que o estado seja 0 ou 1
    gpio_set_level(Rele->Pino, Rele->Estado);
}