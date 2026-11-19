#ifndef MEU_BANCO_H
#define MEU_BANCO_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_NOME 100
#define MAX_HIST 5
#define ARQUIVO_DADOS "dados_banco.bin"

typedef struct {
    double valor;
    char op;
} Transacao;

typedef struct {
    int numero_base;
    int tipo;
    char numero_str[15];
    double saldo;
    Transacao historico[MAX_HIST];
    int total_transacoes;
} Conta;

typedef struct {
    char nome[MAX_NOME];
    Conta *contas;
    int qtd_contas;
    int possui_emprestimo;
    int possui_titulo;
} Cliente;

typedef struct {
    Cliente *clientes;
    int qtd_clientes;
    int proximo_numero_conta;
    int banco_aberto;
    char ultimo_sorteado_nome[MAX_NOME];
    char ultimo_sorteado_conta[15];
    double ultimo_premio;
} Banco;

Banco* inicializar_banco();
void salvar_dados(Banco *banco);
void carregar_dados(Banco *banco);
void liberar_banco(Banco *banco);

void cadastrar_cliente(Banco *banco);
void adicionar_conta_cliente(Banco *banco, Cliente *cliente);
void listar_clientes_contas(const Banco *banco);
Conta* buscar_conta(Banco *banco, const char *num_conta, Cliente **cliente_dono);

void menu_cliente(Banco *banco);
void registrar_transacao(Conta *conta, double valor, char op);
void depositar(Banco *banco, Conta *conta);
void sacar(Banco *banco, Conta *conta);
void solicitar_emprestimo(Banco *banco, Cliente *cliente, Conta *conta_destino);
void adquirir_titulo(Banco *banco, Cliente *cliente);
void exibir_extrato(const Conta *conta);

void consultar_historico(const Banco *banco);
void efetuar_sorteio(Banco *banco);

#endif