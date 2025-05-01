/*
    Guilherme Teodoro de Oliveira RA: 10425362
    Luís Henrique Ribeiro Fernandes RA: 10420079
    Vinícius Brait Lorimier RA: 10420046
*/

// Declaração de Bibliotecas
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_ITEMS 100  // Quantidade máxima de itens por fase
#define MAX_NAME 100   // Tamanho máximo para nomes e tipos

// Declaração das structs
typedef struct {
    char name[MAX_NAME];  
    float weight, value;  
    float originalValue;   
    char type[MAX_NAME]; 
    float valueByWeight;     
    int isComplete;          
    float getWeight;     
    float getValue;     
} Item;

typedef struct {
    char name[MAX_NAME];      
    float capacity;          
    char rule[MAX_NAME];    
    Item items[MAX_ITEMS];    
    int quantItems;        
} Phase;

// Ordenação dos itens de valor por peso
void sortByValueWeight(Item items[], int numItems) {
    for (int i = 0; i < numItems - 1; i++) {
        for (int j = 0; j < numItems - i - 1; j++) {
            if (items[j].valueByWeight < items[j + 1].valueByWeight) {
                Item temp = items[j]; 
                items[j] = items[j + 1]; 
                items[j + 1] = temp;
            }
        }
    }
}

// Aplica a regra da fase e ajusta os valores dos itens
void verifyRules(Phase *phase) {
    for (int i = 0; i < phase->quantItems; i++) {
        Item *item = &phase->items[i];
        item->originalValue = item->value;

        // Regra para itens mágicos
        if (strcmp(phase->rule, "MAGICOS_VALOR_DOBRADO") == 0 && strcmp(item->type, "magico") == 0) {
            item->value *= 2;
        } 
        // Regra para itens de sobrevivência
        else if (strcmp(phase->rule, "SOBREVIVENCIA_DESVALORIZADA") == 0 && strcmp(item->type, "sobrevivencia") == 0) {
            item->value *= 0.8;
        }

        item->valueByWeight = item->value / item->weight;
    }

    if (strcmp(phase->rule, "TRES_MELHORES_VALOR_PESO") == 0) {
        sortByValueWeight(phase->items, phase->quantItems);
        phase->quantItems = 3;
    }
}

// Resolve a fase usando a abordagem do problema da mochila fracionária
void solvePhase(Phase *phase, FILE *out) {

    fprintf(out, "--- FASE: %s ---\n", phase->name);
    fprintf(out, "Capacidade da mochila: %.2f\n", phase->capacity);
    fprintf(out, "Regra aplicada: %s\n\n", phase->rule);

    // Aplica regras e ordena os itens
    verifyRules(phase);
    sortByValueWeight(phase->items, phase->quantItems);

    float remainingCapacity = phase->capacity;
    float profit = 0;

    // Inicializa campos auxiliares dos itens
    for (int i = 0; i < phase->quantItems; i++) {
        phase->items[i].getWeight = 0;
        phase->items[i].getValue = 0;
        phase->items[i].isComplete = 0;
    }

    // Algoritmos para seleção dos itens de maneira correta
    for (int i = 0; i < phase->quantItems && remainingCapacity > 0; i++) {
        Item *item = &phase->items[i];
        float usedWeight = 0, getValue = 0;
        int used = 0;

        // Se a regra exigir que itens tecnológicos sejam inteiros
        if (strcmp(phase->rule, "TECNOLOGICOS_INTEIROS") == 0 && strcmp(item->type, "tecnologico") == 0) {
            if (item->weight <= remainingCapacity) {
                usedWeight = item->weight;
                getValue = item->value;
                used = 1;
            }
        } else {
            if (item->weight <= remainingCapacity) {
                usedWeight = item->weight;
                getValue = item->value;
                used = 1;
            } else {
                usedWeight = remainingCapacity;
                getValue = item->valueByWeight * usedWeight;
            }
        }

        // Atualiza os dados do item se ele for pego
        if (usedWeight > 0) {
            item->getWeight = usedWeight;
            item->getValue = getValue;
            item->isComplete = used;
            remainingCapacity -= usedWeight;
            profit += getValue;
        }
    }

    // Imprime todos os itens selecionados
    for (int i = 0; i < phase->quantItems; i++) {
        Item *item = &phase->items[i];
        if (item->getWeight > 0) {
            fprintf(out, "Pegou (%s) %s, (%.2fkg, R$ %.2f)\n",
                    item->isComplete ? "inteiro" : "fracionado",
                    item->name, item->getWeight, item->getValue);
        }
    }

    fprintf(out, "Lucro da fase: R$: %.2f\n\n\n", profit);
}

// Leitura e processamento de todas as fases
void readInput(FILE *in, FILE *out) {
    Phase phase;
    char line[256];

    while (fgets(line, sizeof(line), in)) 
    {
        if (strncmp(line, "FASE:", 5) == 0) {
            sscanf(line, "FASE: %[^\n]", phase.name);
            phase.quantItems = 0;
        } 
        else if (strncmp(line, "CAPACIDADE:", 11) == 0) {
            sscanf(line, "CAPACIDADE: %f", &phase.capacity);
        } 
        else if (strncmp(line, "REGRA:", 6) == 0) {
            sscanf(line, "REGRA: %[^\n]", phase.rule);
        } 
        else if (strncmp(line, "ITEM:", 5) == 0) {
            Item *item = &phase.items[phase.quantItems++];
            sscanf(line, "ITEM: %[^,], %f, %f, %[^\n]",
                   item->name, &item->weight, &item->value, item->type);
        } 
        else if (line[0] == '\n' || feof(in)) {
            solvePhase(&phase, out);
        }
    }

    // Processa a última fase
    if (phase.quantItems > 0) {
        solvePhase(&phase, out);
    }
}

int main(int argc, char *argv[]) {

    // Verifica se os argumentos foram fornecidos corretamente
    if (argc < 3) {
        printf("Entrada incorreta, siga o formato: ./main <ARQUIVO_ENTRADA> <ARQUIVO_SAIDA>\n");
        return 1;
    }

    // Abre os arquivos de entrada e saída
    FILE *in = fopen(argv[1], "r");
    FILE *out = fopen(argv[2], "w");

    if (!in || !out) {
        printf("Erro ao abrir arquivos, tente novamente!!! \n");
        return 1;
    }

    // Processa as fases com base no conteúdo do arquivo
    readInput(in, out);

    // Fecha os arquivos e exibe mensagem final
    fclose(in);
    fclose(out);

    printf("Dados gravados no arquivo de saída com sucesso.\n");
    return 0;
}
