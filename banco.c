#include "meu_banco.h"

Banco* inicializar_banco() {
    Banco *banco = (Banco*) malloc(sizeof(Banco));
    if (!banco) {
        printf("Erro ao alocar memoria para o banco.\n");
        exit(1);
    }
    banco->clientes = NULL;
    banco->qtd_clientes = 0;
    banco->proximo_numero_conta = 1000;
    banco->banco_aberto = 0;
    strcpy(banco->ultimo_sorteado_nome, "Nenhum");
    strcpy(banco->ultimo_sorteado_conta, "0000-00");
    banco->ultimo_premio = 0.0;

    carregar_dados(banco);
    return banco;
}

void salvar_dados(Banco *banco) {
    FILE *f = fopen(ARQUIVO_DADOS, "wb");
    if (!f) {
        printf("Erro ao abrir arquivo para salvar dados.\n");
        return;
    }

    fwrite(&banco->qtd_clientes, sizeof(int), 1, f);
    fwrite(&banco->proximo_numero_conta, sizeof(int), 1, f);
    fwrite(banco->ultimo_sorteado_nome, sizeof(char), MAX_NOME, f);
    fwrite(banco->ultimo_sorteado_conta, sizeof(char), 15, f);
    fwrite(&banco->ultimo_premio, sizeof(double), 1, f);

    for (int i = 0; i < banco->qtd_clientes; i++) {
        Cliente *c = &banco->clientes[i];
        fwrite(c->nome, sizeof(char), MAX_NOME, f);
        fwrite(&c->qtd_contas, sizeof(int), 1, f);
        fwrite(&c->possui_emprestimo, sizeof(int), 1, f);
        fwrite(&c->possui_titulo, sizeof(int), 1, f);

        if (c->qtd_contas > 0) {
            fwrite(c->contas, sizeof(Conta), c->qtd_contas, f);
        }
    }

    fclose(f);
}

void carregar_dados(Banco *banco) {
    FILE *f = fopen(ARQUIVO_DADOS, "rb");
    if (!f) return;

    fread(&banco->qtd_clientes, sizeof(int), 1, f);
    fread(&banco->proximo_numero_conta, sizeof(int), 1, f);
    fread(banco->ultimo_sorteado_nome, sizeof(char), MAX_NOME, f);
    fread(banco->ultimo_sorteado_conta, sizeof(char), 15, f);
    fread(&banco->ultimo_premio, sizeof(double), 1, f);

    if (banco->qtd_clientes > 0) {
        banco->clientes = (Cliente*) malloc(banco->qtd_clientes * sizeof(Cliente));
        for (int i = 0; i < banco->qtd_clientes; i++) {
            Cliente *c = &banco->clientes[i];
            fread(c->nome, sizeof(char), MAX_NOME, f);
            fread(&c->qtd_contas, sizeof(int), 1, f);
            fread(&c->possui_emprestimo, sizeof(int), 1, f);
            fread(&c->possui_titulo, sizeof(int), 1, f);

            if (c->qtd_contas > 0) {
                c->contas = (Conta*) malloc(c->qtd_contas * sizeof(Conta));
                fread(c->contas, sizeof(Conta), c->qtd_contas, f);
            } else {
                c->contas = NULL;
            }
        }
    }

    fclose(f);
}

void liberar_banco(Banco *banco) {
    if (!banco) return;
    for (int i = 0; i < banco->qtd_clientes; i++) {
        free(banco->clientes[i].contas);
    }
    free(banco->clientes);
    free(banco);
}

void adicionar_conta_cliente(Banco *banco, Cliente *cliente) {
    int tipo;
    printf("Tipo de conta (1 - Corrente, 2 - Salario, 3 - Poupanca): ");
    scanf("%d", &tipo);

    if (tipo < 1 || tipo > 3) {
        printf("Tipo de conta invalido.\n");
        return;
    }

    for (int i = 0; i < cliente->qtd_contas; i++) {
        if ((cliente->contas[i].tipo == 1 && tipo == 2) ||
            (cliente->contas[i].tipo == 2 && tipo == 1)) {
            printf("ERRO: O cliente nao pode possuir conta-corrente e conta-salario simultaneamente.\n");
            return;
        }
        if (cliente->contas[i].tipo == tipo) {
            printf("ERRO: O cliente ja possui uma conta deste tipo.\n");
            return;
        }
    }

    cliente->contas = (Conta*) realloc(cliente->contas, (cliente->qtd_contas + 1) * sizeof(Conta));
    Conta *nova = &cliente->contas[cliente->qtd_contas];

    nova->numero_base = banco->proximo_numero_conta++;
    nova->tipo = tipo;
    sprintf(nova->numero_str, "%d-%02d", nova->numero_base, tipo);
    nova->saldo = 0.0;
    nova->total_transacoes = 0;

    cliente->qtd_contas++;
    printf("Conta %s criada com sucesso (Saldo inicial: R$ 0.00).\n", nova->numero_str);
}

void cadastrar_cliente(Banco *banco) {
    banco->clientes = (Cliente*) realloc(banco->clientes, (banco->qtd_clientes + 1) * sizeof(Cliente));
    Cliente *novo = &banco->clientes[banco->qtd_clientes];

    printf("Nome do cliente: ");
    getchar();
    fgets(novo->nome, MAX_NOME, stdin);
    novo->nome[strcspn(novo->nome, "\n")] = '\0';

    novo->contas = NULL;
    novo->qtd_contas = 0;
    novo->possui_emprestimo = 0;
    novo->possui_titulo = 0;

    int mais_contas = 1;
    while (mais_contas) {
        adicionar_conta_cliente(banco, novo);
        printf("Deseja adicionar outra conta para este cliente? (1 - Sim, 0 - Nao): ");
        scanf("%d", &mais_contas);
    }

    if (novo->qtd_contas > 0) {
        banco->qtd_clientes++;
        salvar_dados(banco);
        printf("Cliente cadastrado com sucesso!\n");
    } else {
        printf("Cadastro cancelado: nenhuma conta valida criada.\n");
    }
}

void listar_clientes_contas(const Banco *banco) {
    if (banco->qtd_clientes == 0) {
        printf("Nenhum cliente cadastrado.\n");
        return;
    }
    printf("\n--- Clientes e Contas Cadastradas ---\n");
    for (int i = 0; i < banco->qtd_clientes; i++) {
        printf("Cliente: %s\n", banco->clientes[i].nome);
        for (int j = 0; j < banco->clientes[i].qtd_contas; j++) {
            printf("  -> Conta: %s | Saldo: R$ %.2f\n",
                   banco->clientes[i].contas[j].numero_str,
                   banco->clientes[i].contas[j].saldo);
        }
    }
}

Conta* buscar_conta(Banco *banco, const char *num_conta, Cliente **cliente_dono) {
    for (int i = 0; i < banco->qtd_clientes; i++) {
        for (int j = 0; j < banco->clientes[i].qtd_contas; j++) {
            if (strcmp(banco->clientes[i].contas[j].numero_str, num_conta) == 0) {
                if (cliente_dono) *cliente_dono = &banco->clientes[i];
                return &banco->clientes[i].contas[j];
            }
        }
    }
    return NULL;
}

void registrar_transacao(Conta *conta, double valor, char op) {
    if (conta->total_transacoes < MAX_HIST) {
        conta->historico[conta->total_transacoes].valor = valor;
        conta->historico[conta->total_transacoes].op = op;
        conta->total_transacoes++;
    } else {
        for (int i = 0; i < MAX_HIST - 1; i++) {
            conta->historico[i] = conta->historico[i + 1];
        }
        conta->historico[MAX_HIST - 1].valor = valor;
        conta->historico[MAX_HIST - 1].op = op;
    }
}

void depositar(Banco *banco, Conta *conta) {
    double valor;
    printf("Valor do deposito: R$ ");
    scanf("%lf", &valor);

    if (valor <= 0) {
        printf("Valor invalido.\n");
        return;
    }

    conta->saldo += valor;
    registrar_transacao(conta, valor, 'D');
    salvar_dados(banco);
    printf("Deposito realizado! Novo saldo: R$ %.2f\n", conta->saldo);
}

void sacar(Banco *banco, Conta *conta) {
    double valor;
    printf("Valor do saque: R$ ");
    scanf("%lf", &valor);

    if (valor <= 0) {
        printf("Valor invalido.\n");
        return;
    }

    if (conta->tipo != 1 && conta->saldo < valor) {
        printf("Saldo insuficiente para saque nesta modalidade de conta.\n");
        return;
    }

    conta->saldo -= valor;
    registrar_transacao(conta, valor, 'S');
    salvar_dados(banco);
    printf("Saque realizado! Novo saldo: R$ %.2f\n", conta->saldo);
}

void solicitar_emprestimo(Banco *banco, Cliente *cliente, Conta *conta_destino) {
    if (cliente->possui_emprestimo) {
        printf("ERRO: Este cliente ja possui um emprestimo ativo (limite de 1 por cliente).\n");
        return;
    }

    int tem_corrente = 0;
    double saldo_poupanca = 0.0;
    double saldo_total_cliente = 0.0;

    for (int i = 0; i < cliente->qtd_contas; i++) {
        saldo_total_cliente += cliente->contas[i].saldo;
        if (cliente->contas[i].tipo == 1) tem_corrente = 1;
        if (cliente->contas[i].tipo == 3) saldo_poupanca = cliente->contas[i].saldo;
    }

    if (!tem_corrente || saldo_poupanca <= 0.0) {
        printf("ERRO: Necessario possuir conta corrente e saldo positivo na poupanca.\n");
        return;
    }

    double limite = 0.25 * (10.0 * saldo_total_cliente);
    printf("Limite disponivel para emprestimo: R$ %.2f\n", limite);
    printf("Valor desejado: R$ ");
    double valor;
    scanf("%lf", &valor);

    if (valor <= 0 || valor > limite) {
        printf("Valor fora do limite permitido.\n");
        return;
    }

    conta_destino->saldo += valor;
    cliente->possui_emprestimo = 1;
    registrar_transacao(conta_destino, valor, 'E');
    salvar_dados(banco);
    printf("Emprestimo de R$ %.2f creditado na conta %s!\n", valor, conta_destino->numero_str);
}

void adquirir_titulo(Banco *banco, Cliente *cliente) {
    if (cliente->possui_titulo) {
        printf("Cliente ja possui titulo de capitalizacao.\n");
        return;
    }

    Conta *conta_salario = NULL;
    for (int i = 0; i < cliente->qtd_contas; i++) {
        if (cliente->contas[i].tipo == 2) {
            conta_salario = &cliente->contas[i];
            break;
        }
    }

    if (!conta_salario) {
        printf("ERRO: Apenas clientes com conta-salario podem adquirir titulos.\n");
        return;
    }

    if (conta_salario->saldo >= 1000.0 && conta_salario->saldo <= 1500.0) {
        cliente->possui_titulo = 1;
        salvar_dados(banco);
        printf("Titulo de capitalizacao adquirido com sucesso!\n");
    } else {
        printf("ERRO: O saldo da conta-salario deve estar entre R$ 1.000,00 e R$ 1.500,00 (Atual: R$ %.2f).\n", conta_salario->saldo);
    }
}

void exibir_extrato(const Conta *conta) {
    printf("\n--- Extrato (Ultimas %d transacoes) ---\n", conta->total_transacoes);
    if (conta->total_transacoes == 0) {
        printf("Nenhuma movimentacao registrada.\n");
        return;
    }
    for (int i = 0; i < conta->total_transacoes; i++) {
        printf("%.2f :%c\n", conta->historico[i].valor, conta->historico[i].op);
    }
}

void menu_cliente(Banco *banco) {
    char num_conta[15];
    printf("Digite o numero da conta (ex: 1000-01): ");
    scanf("%14s", num_conta);

    Cliente *cliente = NULL;
    Conta *conta = buscar_conta(banco, num_conta, &cliente);

    if (!conta) {
        printf("Conta nao encontrada.\n");
        return;
    }

    for (int i = 0; i < cliente->qtd_contas; i++) {
        if (cliente->contas[i].tipo == 1 && cliente->contas[i].saldo < 0) {
            printf("\n[ALERTA]: Sua conta corrente (%s) esta com saldo NEGATIVO (R$ %.2f)!\n",
                   cliente->contas[i].numero_str, cliente->contas[i].saldo);
        }
    }

    int opcao;
    do {
        printf("\n=== Cliente: %s | Conta: %s | Saldo: R$ %.2f ===\n",
               cliente->nome, conta->numero_str, conta->saldo);
        printf("1. Depositar\n");
        printf("2. Sacar\n");
        printf("3. Solicitar emprestimo\n");
        printf("4. Adquirir titulos de capitalizacao\n");
        printf("5. Solicitar extrato\n");
        printf("0. Voltar ao menu principal\n");
        printf("Escolha: ");
        scanf("%d", &opcao);

        switch (opcao) {
            case 1: depositar(banco, conta); break;
            case 2: sacar(banco, conta); break;
            case 3: solicitar_emprestimo(banco, cliente, conta); break;
            case 4: adquirir_titulo(banco, cliente); break;
            case 5: exibir_extrato(conta); break;
            case 0: break;
            default: printf("Opcao invalida.\n");
        }
    } while (opcao != 0);
}

void consultar_historico(const Banco *banco) {
    double saldo_total_banco = 0.0;
    for (int i = 0; i < banco->qtd_clientes; i++) {
        for (int j = 0; j < banco->clientes[i].qtd_contas; j++) {
            saldo_total_banco += banco->clientes[i].contas[j].saldo;
        }
    }

    printf("\nSaldo Total do banco\n");
    printf("%d | R$ %.2f\n", banco->qtd_clientes, saldo_total_banco);

    printf("Clientes Aptos a Emprestimos ==\n");
    for (int i = 0; i < banco->qtd_clientes; i++) {
        Cliente *c = &banco->clientes[i];
        int tem_cc = 0;
        double saldo_poup = 0.0, total_c = 0.0;
        char num_cc[15] = "";

        for (int j = 0; j < c->qtd_contas; j++) {
            total_c += c->contas[j].saldo;
            if (c->contas[j].tipo == 1) {
                tem_cc = 1;
                strcpy(num_cc, c->contas[j].numero_str);
            }
            if (c->contas[j].tipo == 3) saldo_poup = c->contas[j].saldo;
        }

        if (!c->possui_emprestimo && tem_cc && saldo_poup > 0) {
            double limite = 0.25 * (10.0 * total_c);
            printf("%s|%s|R$ %.2f\n", c->nome, num_cc, limite);
        }
    }

    printf("Clientes com Saldo Negativo na Conta Salario\n");
    for (int i = 0; i < banco->qtd_clientes; i++) {
        Cliente *c = &banco->clientes[i];
        for (int j = 0; j < c->qtd_contas; j++) {
            if (c->contas[j].tipo == 2 && c->contas[j].saldo < 0) {
                printf("%s|%s|R$ %.2f\n", c->nome, c->contas[j].numero_str, c->contas[j].saldo);
            }
        }
    }

    printf("Clientes Aptos a Compra de Titulos\n");
    for (int i = 0; i < banco->qtd_clientes; i++) {
        Cliente *c = &banco->clientes[i];
        for (int j = 0; j < c->qtd_contas; j++) {
            if (c->contas[j].tipo == 2 && c->contas[j].saldo >= 1000.0 && c->contas[j].saldo <= 1500.0) {
                printf("%s|%s|R$ %.2f\n", c->nome, c->contas[j].numero_str, c->contas[j].saldo);
            }
        }
    }

    printf("Clientes Contemplado no Sorteio =\n");
    printf("%s|%s|R$ %.2f\n", banco->ultimo_sorteado_nome, banco->ultimo_sorteado_conta, banco->ultimo_premio);
}

static unsigned long long fibonacci(int n) {
    if (n <= 1) return n;
    unsigned long long a = 0, b = 1, c;
    for (int i = 2; i <= n; i++) {
        c = a + b;
        a = b;
        b = c;
    }
    return b;
}

void efetuar_sorteio(Banco *banco) {
    if (banco->qtd_clientes < 5) {
        printf("ERRO: Sao necessarios pelo menos 5 clientes cadastrados para realizar o sorteio.\n");
        return;
    }

    typedef struct {
        Cliente *cliente;
        Conta *maior_conta;
        double saldo_total;
    } Candidato;

    Candidato *lista = (Candidato*) malloc(banco->qtd_clientes * sizeof(Candidato));
    for (int i = 0; i < banco->qtd_clientes; i++) {
        lista[i].cliente = &banco->clientes[i];
        lista[i].saldo_total = 0.0;
        lista[i].maior_conta = &banco->clientes[i].contas[0];

        for (int j = 0; j < banco->clientes[i].qtd_contas; j++) {
            double s = banco->clientes[i].contas[j].saldo;
            lista[i].saldo_total += s;
            if (s > lista[i].maior_conta->saldo) {
                lista[i].maior_conta = &banco->clientes[i].contas[j];
            }
        }
    }

    for (int i = 0; i < banco->qtd_clientes - 1; i++) {
        for (int j = 0; j < banco->qtd_clientes - i - 1; j++) {
            if (lista[j].saldo_total < lista[j + 1].saldo_total) {
                Candidato temp = lista[j];
                lista[j] = lista[j + 1];
                lista[j + 1] = temp;
            }
        }
    }

    for (int i = 0; i < 5; i++) {
        if (!lista[i].cliente->possui_titulo) {
            printf("ERRO: Nem todos os 5 clientes com maiores saldos possuem titulos de capitalizacao.\n");
            free(lista);
            return;
        }
    }

    unsigned long long fib = fibonacci(banco->qtd_clientes + 5);
    int indice_vencedor = fib % 5;

    Candidato vencedor = lista[indice_vencedor];
    double premio = vencedor.maior_conta->saldo * 10.0;
    vencedor.maior_conta->saldo += premio;

    strcpy(banco->ultimo_sorteado_nome, vencedor.cliente->nome);
    strcpy(banco->ultimo_sorteado_conta, vencedor.maior_conta->numero_str);
    banco->ultimo_premio = premio;

    salvar_dados(banco);
    printf("\nSORTEIO REALIZADO!\n");
    printf("Vencedor: %s | Conta: %s | Premio: R$ %.2f\n",
           vencedor.cliente->nome, vencedor.maior_conta->numero_str, premio);

    free(lista);
}