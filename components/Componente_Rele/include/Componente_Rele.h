// Componente para controle de relé da estugfa. 
// Tem funções para iniciar, ligar, desligar e definir o estado do relé.
// Autor: Rogerio da Oliveira
// Data: 09/2025


#ifndef COMPONENTE_RELE_H
#define COMPONENTE_RELE_H

#define RELE_DESLIGADO 0  // Adapta a definição do estado do relé ligado/desligado
#define RELE_LIGADO    1

// Estrutura para armazenar as características do relé
typedef struct {
    int Pino;        // Pino do relé
    int Estado;      // Estado do relé (ligado ou desligado)
} Tipo_Rele;

void Rele_Inicia(Tipo_Rele *Rele, int Pino);
void Rele_Liga(Tipo_Rele *Rele);
void Rele_Desliga(Tipo_Rele *Rele);
int Rele_Estado(Tipo_Rele *Rele);
void Rele_Define_Estado(Tipo_Rele *Rele, int Estado);

#endif
