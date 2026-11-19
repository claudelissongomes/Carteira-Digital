#include "meu_banco.h"

int main() {
    Banco *banco = inicializar_banco();
    int opcao;

    do {
        printf("\n==============================\n");
        printf("      CARTEIRA DIGITAL        \n");
        printf("Status do Banco: %s\n", banco->banco_aberto ? "ABERTO" : "FECHADO");
        printf("==============================\n");
        printf("1. Abrir o banco (Cadastrar / Acessar / Listar)\n");
        printf("2. Consultar historico\n");
        printf("3. Efetuar sorteio\n");
        printf("4. Sair\n");
        printf("Escolha uma opcao: ");
        scanf("%d", &opcao);

        switch (opcao) {
            case 1: {
                banco->banco_aberto = 1;
                int sub_opcao;
                printf("\n--- Banco Aberto ---\n");
                printf("1. Cadastrar novo cliente\n");
                printf("2. Acessar conta de cliente\n");
                printf("3. Listar clientes e contas\n");
                printf("0. Voltar\n");
                printf("Escolha: ");
                scanf("%d", &sub_opcao);

                if (sub_opcao == 1) {
                    cadastrar_cliente(banco);
                } else if (sub_opcao == 2) {
                    menu_cliente(banco);
                } else if (sub_opcao == 3) {
                    listar_clientes_contas(banco);
                }
                break;
            }
            case 2:
                if (!banco->banco_aberto) {
                    printf("ERRO: O banco precisa estar aberto para consultar o historico.\n");
                } else {
                    consultar_historico(banco);
                }
                break;

            case 3:
                if (!banco->banco_aberto) {
                    printf("ERRO: O banco precisa estar aberto para efetuar o sorteio.\n");
                } else {
                    efetuar_sorteio(banco);
                }
                break;

            case 4:
                salvar_dados(banco);
                printf("Encerrando o sistema e salvando dados...\n");
                break;

            default:
                printf("Opcao invalida.\n");
        }
    } while (opcao != 4);

    liberar_banco(banco);
    return 0;
}