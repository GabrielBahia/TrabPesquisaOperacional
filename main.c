#include <stdio.h>
#include <stdlib.h>
#include <glpk.h>
#include <math.h>
#define NUM_MEMBROS 28
#define NUM_HORARIOS 50

int main(void)
{   
    // Criação do objeto de problema
    glp_prob *lp;
    lp = glp_create_prob();

    // Definição do tipo do problema como um problema de programação linear
    glp_set_prob_name(lp, "AlocacaoHorarioMembros");
    glp_set_obj_dir(lp, GLP_MAX); // Maximização

    // Lê os dados de horários disponíveis de cada membro do arquivo horarios.txt
    int pesos[NUM_MEMBROS][NUM_HORARIOS];
    FILE *arquivo = fopen("horariosReal.txt", "r");
    if (arquivo == NULL) {
        printf("Erro ao abrir o arquivo\n");
        return 1;
    }
    int membro, horario, peso;
    while (fscanf(arquivo, "%d-%d-%d", &membro, &horario, &peso) == 3) {
        pesos[membro][horario] = peso;
    }
    fclose(arquivo);

    // Definição das variáveis de decisão Xij (Variáveis binarias)
    int numVariaveis = NUM_MEMBROS * NUM_HORARIOS;
    glp_add_cols(lp, numVariaveis);
    int colIndex = 1;
    for (int i = 0; i < NUM_MEMBROS; i++) {
        for (int j = 0; j < NUM_HORARIOS; j++) {
            char colName[20];
            sprintf(colName, "X%d_%d", i, j);
            glp_set_col_name(lp, colIndex, colName);
            glp_set_col_kind(lp, colIndex, GLP_BV);
            glp_set_col_bnds(lp, colIndex, GLP_DB, 0, 1); 
            colIndex++;
        }
    }

    // Definição da função objetivo
    // Incluindo coeficientes em cada variável
    int colIndex2 = 1;
    for (int i = 0; i < NUM_MEMBROS; i++) {
        for (int j = 0; j < NUM_HORARIOS; j++) {
            int coeficiente = (pesos[i][j] > 0) ? 1 : 0;
            glp_set_obj_coef(lp, colIndex2, coeficiente);
            colIndex2++;
        }
    }

    // Definição das restrições
    // Restrição de duração mínima e máxima de sede para cada membro
    int restricaoIndex = 0;
    for (int i = 0; i < NUM_MEMBROS; i++) {
        restricaoIndex = restricaoIndex + 1; 
        glp_add_rows(lp, 1); // Uma restrição para cada membro
        // Restrição de duração mínima de sede
        glp_set_row_name(lp, restricaoIndex, "DuracaoMinimaMaxima");
        glp_set_row_bnds(lp, restricaoIndex, GLP_DB, 6.0, 8.0); // 6 <= Restrição <= 8

        int numTermos = NUM_HORARIOS;
        int ja[numTermos + 1]; 
        double ar[numTermos + 1]; 

        int termoIndex = 1; // Começamos em 1 em vez de 0
        for (int j = 0; j < NUM_HORARIOS; j++) {
            int variavelIndex = i * NUM_HORARIOS + j + 1;
    
            // Matriz esparsa
            ja[termoIndex] = variavelIndex; // guarda o indice da coluna da variavel
            ar[termoIndex] = 1.0;  // guarda o valor 

            termoIndex++;
        }
        glp_set_mat_row(lp, restricaoIndex, numTermos, ja, ar);
    }

    // Restrição de quantidade mínima e máxima de membro em cada horário
    for (int k = 0; k < NUM_HORARIOS; k++) {  // Variando cada horario
        restricaoIndex = restricaoIndex + 1; 
        glp_add_rows(lp, 1); 

        // Define o tipo da nova restrição
        glp_set_row_name(lp, restricaoIndex, "quantidadeMinimaEMaxima");
        glp_set_row_bnds(lp, restricaoIndex, GLP_DB, 1.0, 8.0); // 1 <= Restrição <= 8

        int numTermos = NUM_MEMBROS;
        int ja[numTermos + 1]; 
        double ar[numTermos + 1]; 
        int termoIndex = 1;

        int j = 0; // Variavel para dar saltos de horario dentro de um mesmo membro (50 a 50)
        for (int i = 0; i < NUM_MEMBROS; i++) { // Variando os membros dentro do mesmo horario
            int variavelIndex = (j * NUM_HORARIOS) + k + 1;

            // Matriz esparsa
            ja[termoIndex] = variavelIndex; // guarda o indice da coluna da variavel
            ar[termoIndex] = 1.0; // guarda o valor 

            termoIndex++;
            j++;
        }
        glp_set_mat_row(lp, restricaoIndex, numTermos, ja, ar);
    }

    // Carregamento e resolução do problema
    glp_simplex(lp, NULL);
    // Verificação do status da solução
    int status = glp_get_status(lp);
    if (status == GLP_OPT) {
        // Solução ótima encontrada
        // Impressão do valor ótimo
        double valorOtimo = glp_get_obj_val(lp);

        printf("Valor ótimo: %g\n", valorOtimo);

        // Abre o arquivo para escrita
        FILE *arquivo_saida3 = fopen("resultado.txt", "w");
        if (arquivo_saida3 == NULL) {
            printf("Erro ao abrir o arquivo de saída\n");
            return 1;
        }

        // Impressão do valor ótimo
        fprintf(arquivo_saida3, "Valor ótimo: %g\n", valorOtimo);
          
        fclose(arquivo_saida3);
    } else {
        printf("Não foi possível encontrar uma solução ótima.\n");
    }
    glp_write_lp(lp, NULL, "model.lp");
    // Liberação de recursos
    glp_delete_prob(lp);
    glp_free_env();

    return 0;
}
