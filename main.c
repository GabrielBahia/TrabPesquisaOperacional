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
    glp_set_prob_name(lp, "exemplo");
    glp_set_obj_dir(lp, GLP_MAX);

    // Lê os dados de horários disponíveis de cada membro do arquivo horarios.txt
    int pesos[NUM_MEMBROS][NUM_HORARIOS];
    FILE *arquivo = fopen("horarios2.txt", "r");
    if (arquivo == NULL) {
        printf("Erro ao abrir o arquivo\n");
        return 1;
    }
    int membro, horario, peso;
    while (fscanf(arquivo, "%d-%d-%d", &membro, &horario, &peso) == 3) {
        pesos[membro][horario] = peso;
    }
    fclose(arquivo);

    // Definição da função objetivo
    int numMembros = 28; // Quantidade de membros
    int numHorarios = 50; // Horas comerciais
    int numVariaveis = numMembros * numHorarios;

    // Definição das variáveis de decisão Xij
    glp_add_cols(lp, numVariaveis);
    int colIndex = 1;
    for (int i = 0; i < numMembros; i++) {
        for (int j = 0; j < numHorarios; j++) {
            char colName[20];
            sprintf(colName, "X%d%d", i, j);
            glp_set_col_name(lp, colIndex, colName);
            glp_set_col_kind(lp, colIndex, GLP_BV);
            glp_set_col_bnds(lp, colIndex, GLP_DB, 0, 1); 
            colIndex++;
        }
    }

    // Definição da função objetivo
    int colIndex2 = 1;
    for (int i = 0; i < numMembros; i++) {
        for (int j = 0; j < numHorarios; j++) {
            int coeficiente = (pesos[i][j] > 0) ? 1 : 0;
            glp_set_obj_coef(lp, colIndex2, coeficiente);
            colIndex2++;
        }
    }

    // Definição das restrições
    // Restrição de duração mínima e máxima de sede para cada membro
    for (int i = 0; i < numMembros; i++) {
        int restricaoIndex = i + 1;
        glp_add_rows(lp, 1); // Uma restrição para cada membro

        // Restrição de duração mínima de sede
        glp_set_row_name(lp, restricaoIndex, "DuracaoMinimaMaxima");
        //glp_set_row_bnds(lp, restricaoIndex, GLP_LO, 6.0, 0.0); // >= 6
        glp_set_row_bnds(lp, restricaoIndex, GLP_DB, 6.0, 8.0); // 6 <= Restrição <= 8

        int numTermos = numHorarios;
        int ia[numTermos + 1]; // Acrescentamos um elemento extra no array ia
        int ja[numTermos + 1]; // Acrescentamos um elemento extra no array ja
        double ar[numTermos + 1]; // Acrescentamos um elemento extra no array ar

        int termoIndex = 1; // Começamos em 1 em vez de 0
        for (int j = 0; j < numHorarios; j++) {
            int variavelIndex = i * numHorarios + j + 1;
            ia[termoIndex] = restricaoIndex;
            ja[termoIndex] = variavelIndex;
            ar[termoIndex] = 1.0;
            termoIndex++;
        }

        glp_set_mat_row(lp, restricaoIndex, numTermos, ja, ar);
    }

    // Restrição de pelo menos 1 membro e no máximo 8 na sede
    for (int j = 0; j < numHorarios; j++) {
        int restricaoIndex = numMembros + j + 1; // Índice da nova restrição
        glp_add_rows(lp, 1); // Adiciona uma nova restrição

        // Define o tipo da nova restrição
        glp_set_row_name(lp, restricaoIndex, "quantidadeMinimaEMaxima");
        //glp_set_row_bnds(lp, restricaoIndex, GLP_LO, 1.0, 0.0); // Restrição >= 1
        glp_set_row_bnds(lp, restricaoIndex, GLP_DB, 1.0, 8.0); // 1 <= Restrição <= 8

        int numTermos = numMembros;
        int ia[numTermos + 1]; // Acrescentamos um elemento extra no array ia
        int ja[numTermos + 1]; // Acrescentamos um elemento extra no array ja
        double ar[numTermos + 1]; // Acrescentamos um elemento extra no array ar

        int termoIndex = 1;
        for (int i = 0; i < numMembros; i++) {
            int variavelIndex = i * numHorarios + j + 1;
            ia[termoIndex] = restricaoIndex;
            ja[termoIndex] = variavelIndex;
            ar[termoIndex] = 1.0;
            
            termoIndex++;
        }

        glp_set_mat_row(lp, restricaoIndex, numTermos, ja, ar);
    }

    // Abre o arquivo para escrita
    FILE *arquivo_saida = fopen("variaveis.txt", "w");
    if (arquivo_saida == NULL) {
    printf("Erro ao abrir o arquivo de saída\n");
    return 1;
    }
    // Impressão dos valores das variáveis de decisão no arquivo
    for (int i = 0; i < numMembros; i++) {
        for (int j = 0; j < numHorarios; j++) {
            int variavelIndex = i * numHorarios + j + 1;
            double valorVariavel = glp_get_col_prim(lp, variavelIndex);
            int valorInteiro = (int) round(valorVariavel); // Converte o valor para inteiro
            int coeficiente = pesos[i][j];
            fprintf(arquivo_saida, "X%d_%d = %d, Coeficiente = %d\n", i+1, j+1, valorInteiro, coeficiente);
        }
    }
    // Fecha o arquivo após a escrita
    fclose(arquivo_saida);
    // Abre o arquivo para escrita
    FILE *arquivo_saida2 = fopen("restricoes.txt", "w");
    if (arquivo_saida2 == NULL) {
        printf("Erro ao abrir o arquivo de saída\n");
        return 1;
    }
    // Impressão das informações das restrições no arquivo
    for (int i = 1; i <= 78; i++) {
        const char* nomeRestricao = glp_get_row_name(lp, i);
        int tipoRestricao = glp_get_row_type(lp, i);
        double limiteInferior = glp_get_row_lb(lp, i);
        double limiteSuperior = glp_get_row_ub(lp, i);
        fprintf(arquivo_saida2, "Restrição %d: %s, Tipo: %d, Limite Inferior: %g, Limite Superior: %g\n", i, nomeRestricao, tipoRestricao, limiteInferior, limiteSuperior);
    }
    // Fecha o arquivo após a escrita
    fclose(arquivo_saida2);

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

        // Impressão dos valores das variáveis de decisão
        for (int i = 0; i < numMembros; i++) {
            int totalHorasMembro = 0;
            for (int j = 0; j < numHorarios; j++) {
                int variavelIndex = i * numHorarios + j + 1;
                double valorVariavel = glp_get_col_prim(lp, variavelIndex);
                if(valorVariavel != 0) {
                    totalHorasMembro++;
                }
                fprintf(arquivo_saida3, "X%d_%d = %g \n", i+1, j+1, valorVariavel);
            }
            fprintf(arquivo_saida3, "Total de horas do membro %d = %d \n", i+1, totalHorasMembro);
        }

        // Impressão das quantidades de horas cumpridas por dia
        int diasSemanaHorariosCumpridos[50];
        for(int x = 0; x < 50; x++) {
            diasSemanaHorariosCumpridos[x] = 0;
        }

        int diasSemanaSomaTotal[5] = {0, 0, 0, 0, 0};
        for (int j = 0; j < numHorarios; j++) {
            for (int i = 0; i < numMembros; i++) {
                int variavelIndex = i * numHorarios + j + 1;
                double valorVariavel = glp_get_col_prim(lp, variavelIndex);
                if(valorVariavel != 0) {
                    diasSemanaHorariosCumpridos[j] = 1;
                    /*if(j+1 >= 1 && j+1 <=10) {
                        diasSemana[0]++;
                    } else if(j+1 >= 11 && j+1 <=20){
                        diasSemana[1]++;
                    }else if(j+1 >= 21 && j+1 <=30){
                        diasSemana[2]++;
                    }else if(j+1 >= 31 && j+1 <=40){
                        diasSemana[3]++;
                    }else if(j+1 >= 41 && j+1 <=50){
                        diasSemana[4]++;
                    }*/
                }
            }
        }

        //fprintf(arquivo_saida3, "Chegou aqui ? \n");
        int contTotalHoras = 0;
        for(int i = 0; i < 50; i++) {
            //fprintf(arquivo_saida3, "diasSemanaHorariosCumpridos[i] = %d \n", diasSemanaHorariosCumpridos[i]);
            if(diasSemanaHorariosCumpridos[i] != 0) {
                if(i >= 0 && i < 10) {
                    contTotalHoras++;
                    if(i == 9) {
                        fprintf(arquivo_saida3, "Total de horários na Segunda = %d \n", contTotalHoras);
                        contTotalHoras = 0;
                    }
                } else if(i >= 10 && i < 20){
                    contTotalHoras++;
                    if(i == 19) {
                        fprintf(arquivo_saida3, "Total de horários na Terça = %d \n", contTotalHoras);
                        contTotalHoras = 0;
                    }
                }else if(i >= 20 && i < 30){
                    contTotalHoras++;
                    if(i == 29) {
                        fprintf(arquivo_saida3, "Total de horários na Quarta = %d \n", contTotalHoras);
                        contTotalHoras = 0;
                    }
                }else if(i >= 30 && i < 40){
                    contTotalHoras++;
                    if(i == 39) {
                        fprintf(arquivo_saida3, "Total de horários na Quinta = %d \n", contTotalHoras);
                        contTotalHoras = 0;
                    } 
                }else if(i >= 40 && i < 50){
                    contTotalHoras++;
                    if(i == 49) {
                        fprintf(arquivo_saida3, "Total de horários na Sexta = %d \n", contTotalHoras);
                        contTotalHoras = 0;
                    }  
                }
            }
        }
        
        fclose(arquivo_saida3);
    } else {
        printf("Não foi possível encontrar uma solução ótima.\n");
    }

    // Liberação de recursos
    glp_delete_prob(lp);
    glp_free_env();

    return 0;
}
