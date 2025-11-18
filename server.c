#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <time.h>

#pragma comment(lib, "ws2_32.lib")

#define PORT 8080
#define MAX_CLIENTS 2
#define BUFFER_SIZE 1024
#define MAX_CARDS 52

typedef struct {
    char naipe;  
    char valor; 
} Carta;

typedef struct {
    Carta cartas[MAX_CARDS];
    int topo;
} Baralho;

typedef struct {
    Carta mao[21];
    int num_cartas;
    int pontos;
    int ativo;
    int apostou;
} Jogador;

typedef struct {
    SOCKET socket;
    Jogador jogador;
    int id;
    int conectado;
} ClienteInfo;

void dar_carta_jogador(Jogador *j);
void dar_carta_dealer(Jogador *d);
void iniciar_jogo();
void enviar_mensagem(SOCKET sock, const char *msg);
void broadcast_mensagem(const char *msg, int excluir_id);
void enviar_estado_jogo(int cliente_id);
void enviar_resultado_final();
void processar_turno_dealer();
void proximo_jogador();
void processar_comando(int cliente_id, const char *comando);
DWORD WINAPI handle_cliente(LPVOID param);

Baralho baralho;
ClienteInfo clientes[MAX_CLIENTS];
Jogador dealer;
int jogo_ativo = 0;
int vez_jogador = 0;
int aguardando_novo_jogo = 0;

void inicializar_baralho() {
    char naipes[] = {'C', 'E', 'O', 'P'};
    char valores[] = {'A', '2', '3', '4', '5', '6', '7', '8', '9', 'T', 'J', 'Q', 'K'};
    int index = 0;
    
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 13; j++) {
            baralho.cartas[index].naipe = naipes[i];
            baralho.cartas[index].valor = valores[j];
            index++;
        }
    }
    baralho.topo = 0;
}

void embaralhar() {
    srand((unsigned int)time(NULL));
    for (int i = 0; i < MAX_CARDS; i++) {
        int j = rand() % MAX_CARDS;
        Carta temp = baralho.cartas[i];
        baralho.cartas[i] = baralho.cartas[j];
        baralho.cartas[j] = temp;
    }
}

Carta pegar_carta() {
    if (baralho.topo >= MAX_CARDS) {
        baralho.topo = 0;
        embaralhar();
    }
    return baralho.cartas[baralho.topo++];
}

int valor_carta(char valor) {
    if (valor >= '2' && valor <= '9') return valor - '0';
    if (valor == 'T') return 10;
    if (valor == 'J' || valor == 'Q' || valor == 'K') return 10;
    if (valor == 'A') return 11;
    return 0;
}

int calcular_pontos(Jogador *j) {
    int pontos = 0;
    int ases = 0;
    
    for (int i = 0; i < j->num_cartas; i++) {
        int val = valor_carta(j->mao[i].valor);
        pontos += val;
        if (j->mao[i].valor == 'A') ases++;
    }

    while (pontos > 21 && ases > 0) {
        pontos -= 10;
        ases--;
    }
    
    j->pontos = pontos;
    return pontos;
}

void dar_carta_jogador(Jogador *j) {
    Carta c = pegar_carta();
    j->mao[j->num_cartas++] = c;
    calcular_pontos(j);
}

void iniciar_jogo() {

    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clientes[i].conectado) {
            clientes[i].jogador.num_cartas = 0;
            clientes[i].jogador.pontos = 0;
            clientes[i].jogador.ativo = 1;
            clientes[i].jogador.apostou = 0;
        }
    }

    dealer.num_cartas = 0;
    dealer.pontos = 0;
    dealer.ativo = 1;

    embaralhar();
    
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clientes[i].conectado) {
            dar_carta_jogador(&clientes[i].jogador);
            dar_carta_jogador(&clientes[i].jogador);
        }
    }
  
    dar_carta_dealer(&dealer);
    dar_carta_dealer(&dealer);
    
    jogo_ativo = 1;
    vez_jogador = 0;
}

void dar_carta_dealer(Jogador *d) {
    Carta c = pegar_carta();
    d->mao[d->num_cartas++] = c;
    calcular_pontos(d);
}

void enviar_mensagem(SOCKET sock, const char *msg) {
    send(sock, msg, strlen(msg), 0);
}

void broadcast_mensagem(const char *msg, int excluir_id) {
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clientes[i].conectado && clientes[i].id != excluir_id) {
            enviar_mensagem(clientes[i].socket, msg);
        }
    }
}

void enviar_estado_jogo(int cliente_id) {
    char buffer[BUFFER_SIZE];
    ClienteInfo *c = &clientes[cliente_id];

    sprintf(buffer, "ESTADO|%d|%d|", vez_jogador, c->jogador.pontos);

    for (int i = 0; i < c->jogador.num_cartas; i++) {
        char carta[4];
        sprintf(carta, "%c%c", c->jogador.mao[i].valor, c->jogador.mao[i].naipe);
        strcat(buffer, carta);
        if (i < c->jogador.num_cartas - 1) strcat(buffer, ",");
    }
    strcat(buffer, "|");

    if (dealer.num_cartas > 0) {
        char carta[4];
        sprintf(carta, "%c%c", dealer.mao[0].valor, dealer.mao[0].naipe);
        strcat(buffer, carta);
    }
    strcat(buffer, "|");
    
    if (!jogo_ativo) {
        strcat(buffer, "JOGO_FINALIZADO");
    } else if (!c->jogador.ativo) {
        strcat(buffer, "PARADO");
    } else if (c->jogador.pontos > 21) {
        strcat(buffer, "ESTOUROU");
    } else if (vez_jogador == cliente_id) {
        strcat(buffer, "SUA_VEZ");
    } else {
        strcat(buffer, "AGUARDE");
    }
    
    strcat(buffer, "\n");
    enviar_mensagem(c->socket, buffer);
}

void enviar_resultado_final() {
    char buffer[BUFFER_SIZE];
    
    int jogador1_pontos = -1;
    int jogador2_pontos = -1;
    
    if (clientes[0].conectado) jogador1_pontos = clientes[0].jogador.pontos;
    if (clientes[1].conectado) jogador2_pontos = clientes[1].jogador.pontos;
    
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clientes[i].conectado) {
            sprintf(buffer, "RESULTADO|%d|%d|", clientes[i].jogador.pontos, dealer.pontos);
            
            for (int j = 0; j < dealer.num_cartas; j++) {
                char carta[4];
                sprintf(carta, "%c%c", dealer.mao[j].valor, dealer.mao[j].naipe);
                strcat(buffer, carta);
                if (j < dealer.num_cartas - 1) strcat(buffer, ",");
            }
            strcat(buffer, "|");

            if (clientes[i].jogador.pontos > 21) {
                strcat(buffer, "PERDEU");
            } else if (dealer.pontos > 21) {
                strcat(buffer, "GANHOU");
            } else if (clientes[i].jogador.pontos > dealer.pontos) {
                strcat(buffer, "GANHOU");
            } else if (clientes[i].jogador.pontos < dealer.pontos) {
                strcat(buffer, "PERDEU");
            } else {
                strcat(buffer, "EMPATE");
            }
            
            strcat(buffer, "|");
            char comparacao[128];
            sprintf(comparacao, "%d|%d", jogador1_pontos, jogador2_pontos);
            strcat(buffer, comparacao);
            
            strcat(buffer, "\n");
            enviar_mensagem(clientes[i].socket, buffer);
        }
    }
}

void processar_turno_dealer() {
    while (dealer.pontos < 17) {
        dar_carta_dealer(&dealer);
    }
    
    enviar_resultado_final();
    jogo_ativo = 0;
    aguardando_novo_jogo = 1;
}

int todos_jogadores_inativos() {
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clientes[i].conectado && clientes[i].jogador.ativo) {
            return 0;
        }
    }
    return 1;
}

void proximo_jogador() {
    if (todos_jogadores_inativos()) {
        processar_turno_dealer();
        return;
    }
    
    int proximo = (vez_jogador + 1) % MAX_CLIENTS;
    int tentativas = 0;
    
    // Encontrar próximo jogador ativo
    while (tentativas < MAX_CLIENTS) {
        if (clientes[proximo].conectado && clientes[proximo].jogador.ativo) {
            vez_jogador = proximo;
            return;
        }
        proximo = (proximo + 1) % MAX_CLIENTS;
        tentativas++;
    }
    
    processar_turno_dealer();
}

void processar_comando(int cliente_id, const char *comando) {
    ClienteInfo *c = &clientes[cliente_id];
    char buffer[BUFFER_SIZE];
    
    if (strcmp(comando, "HIT") == 0) {
        if (aguardando_novo_jogo) {
            enviar_mensagem(c->socket, "ERRO|Jogo finalizado! Digite START para começar novo jogo.\n");
            return;
        }
        
        if (vez_jogador != cliente_id) {
            enviar_mensagem(c->socket, "ERRO|Não é sua vez!\n");
            return;
        }
        
        dar_carta_jogador(&c->jogador);
        
        if (c->jogador.pontos > 21) {
            c->jogador.ativo = 0;
            sprintf(buffer, "ESTOUROU|Você estourou com %d pontos!\n", c->jogador.pontos);
            enviar_mensagem(c->socket, buffer);
            
            if (todos_jogadores_inativos()) {
                processar_turno_dealer();
                return; // Não enviar estado, o resultado já foi enviado
            } else {
                proximo_jogador();
            }
        }
        
        // Atualizar estado para todos
        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (clientes[i].conectado) {
                enviar_estado_jogo(i);
            }
        }
    }
    else if (strcmp(comando, "STAND") == 0) {
        if (aguardando_novo_jogo) {
            enviar_mensagem(c->socket, "ERRO|Jogo finalizado! Digite START para começar novo jogo.\n");
            return;
        }
        
        if (vez_jogador != cliente_id) {
            enviar_mensagem(c->socket, "ERRO|Não é sua vez!\n");
            return;
        }
        
        c->jogador.ativo = 0;
        
        if (todos_jogadores_inativos()) {
            processar_turno_dealer();
            return; // Não enviar estado, o resultado já foi enviado
        } else {
            proximo_jogador();
        }
        
        // Atualizar estado para todos
        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (clientes[i].conectado) {
                enviar_estado_jogo(i);
            }
        }
    }
    else if (strcmp(comando, "START") == 0) {
        if (!jogo_ativo) {
            aguardando_novo_jogo = 0;
            iniciar_jogo();
            
            for (int i = 0; i < MAX_CLIENTS; i++) {
                if (clientes[i].conectado) {
                    enviar_estado_jogo(i);
                }
            }
        } else {
            enviar_mensagem(c->socket, "ERRO|Jogo já está ativo!\n");
        }
    }
    else {
        enviar_mensagem(c->socket, "ERRO|Comando inválido!\n");
    }
}

DWORD WINAPI handle_cliente(LPVOID param) {
    int cliente_id = *(int*)param;
    free(param);
    
    ClienteInfo *c = &clientes[cliente_id];
    char buffer[BUFFER_SIZE];
    int recv_size;
    
    sprintf(buffer, "BEM_VINDO|Você é o Jogador %d\n", cliente_id + 1);
    enviar_mensagem(c->socket, buffer);
    
    while ((recv_size = recv(c->socket, buffer, BUFFER_SIZE - 1, 0)) > 0) {
        buffer[recv_size] = '\0';

        buffer[strcspn(buffer, "\r\n")] = 0;
        
        printf("Cliente %d: %s\n", cliente_id, buffer);
        
        processar_comando(cliente_id, buffer);
    }
    
    printf("Cliente %d desconectado\n", cliente_id);
    c->conectado = 0;
    closesocket(c->socket);
    
    return 0;
}

int main() {
    WSADATA wsa;
    SOCKET servidor_socket, cliente_socket;
    struct sockaddr_in servidor, cliente;
    int c_size;
    
    printf("=== Servidor Blackjack ===\n");

    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        printf("Erro ao inicializar Winsock: %d\n", WSAGetLastError());
        return 1;
    }

    if ((servidor_socket = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
        printf("Erro ao criar socket: %d\n", WSAGetLastError());
        return 1;
    }

    servidor.sin_family = AF_INET;
    servidor.sin_addr.s_addr = INADDR_ANY;
    servidor.sin_port = htons(PORT);
    
    if (bind(servidor_socket, (struct sockaddr*)&servidor, sizeof(servidor)) == SOCKET_ERROR) {
        printf("Erro no bind: %d\n", WSAGetLastError());
        return 1;
    }

    listen(servidor_socket, MAX_CLIENTS);
    printf("Servidor iniciado na porta %d\n", PORT);
    printf("Aguardando jogadores...\n");

    inicializar_baralho();
    embaralhar();

    for (int i = 0; i < MAX_CLIENTS; i++) {
        clientes[i].conectado = 0;
        clientes[i].id = i;
    }

    while (1) {
        c_size = sizeof(struct sockaddr_in);
        cliente_socket = accept(servidor_socket, (struct sockaddr*)&cliente, &c_size);
        
        if (cliente_socket == INVALID_SOCKET) {
            printf("Erro ao aceitar conexão\n");
            continue;
        }

        int cliente_id = -1;
        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (!clientes[i].conectado) {
                cliente_id = i;
                break;
            }
        }
        
        if (cliente_id == -1) {
            printf("Servidor cheio!\n");
            send(cliente_socket, "ERRO|Servidor cheio!\n", 21, 0);
            closesocket(cliente_socket);
            continue;
        }
        
        clientes[cliente_id].socket = cliente_socket;
        clientes[cliente_id].conectado = 1;
        clientes[cliente_id].jogador.num_cartas = 0;
        clientes[cliente_id].jogador.pontos = 0;
        clientes[cliente_id].jogador.ativo = 0;
        
        printf("Jogador %d conectado\n", cliente_id + 1);

        int *id = malloc(sizeof(int));
        *id = cliente_id;
        CreateThread(NULL, 0, handle_cliente, id, 0, NULL);
    }
    
    closesocket(servidor_socket);
    WSACleanup();
    
    return 0;
}
