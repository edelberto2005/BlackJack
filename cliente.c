#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>

#pragma comment(lib, "ws2_32.lib")

#define PORT 8080
#define BUFFER_SIZE 1024

void limpar_tela() {
    system("cls");
}

void exibir_cartas(const char *cartas_str) {
    char cartas[BUFFER_SIZE];
    strcpy(cartas, cartas_str);
    
    char *token = strtok(cartas, ",");
    while (token != NULL) {
        if (strlen(token) >= 2) {
            char valor = token[0];
            char naipe = token[1];

            char naipe_nome[20];
            switch (naipe) {
                case 'C': strcpy(naipe_nome, "Copas"); break;
                case 'E': strcpy(naipe_nome, "Espadas"); break;
                case 'O': strcpy(naipe_nome, "Ouros"); break;
                case 'P': strcpy(naipe_nome, "Paus"); break;
                default: strcpy(naipe_nome, "?"); break;
            }

            char valor_nome[20];
            switch (valor) {
                case 'A': strcpy(valor_nome, "As"); break;
                case 'T': strcpy(valor_nome, "10"); break;
                case 'J': strcpy(valor_nome, "Valete"); break;
                case 'Q': strcpy(valor_nome, "Dama"); break;
                case 'K': strcpy(valor_nome, "Rei"); break;
                default: sprintf(valor_nome, "%c", valor); break;
            }
            
            printf("[%s de %s] ", valor_nome, naipe_nome);
        }
        token = strtok(NULL, ",");
    }
    printf("\n");
}

void processar_estado(const char *msg) {
    char buffer[BUFFER_SIZE];
    strcpy(buffer, msg);

    char *token = strtok(buffer, "|");
    if (token == NULL || strcmp(token, "ESTADO") != 0) return;
    
    token = strtok(NULL, "|"); 
    int vez = atoi(token);
    
    token = strtok(NULL, "|");
    int pontos = atoi(token);
    
    token = strtok(NULL, "|");
    char cartas[BUFFER_SIZE] = "";
    if (token != NULL) strcpy(cartas, token);
    
    token = strtok(NULL, "|"); 
    char carta_dealer[BUFFER_SIZE] = "";
    if (token != NULL) strcpy(carta_dealer, token);
    
    token = strtok(NULL, "|\n");
    char status[BUFFER_SIZE] = "";
    if (token != NULL) strcpy(status, token);
 
    limpar_tela();
    printf("========================================\n");
    printf("         BLACKJACK - JOGO ATIVO         \n");
    printf("========================================\n\n");
    
    printf("Dealer mostra: ");
    exibir_cartas(carta_dealer);
    printf("\n");
    
    printf("Suas cartas: ");
    exibir_cartas(cartas);
    printf("Seus pontos: %d\n\n", pontos);
    
    printf("Status: %s\n", status);
    
    if (strcmp(status, "SUA_VEZ") == 0) {
        printf("\n--- SUA VEZ DE JOGAR ---\n");
        printf("Digite 'HIT' para pedir carta ou 'STAND' para parar\n");
    }
    
    printf("========================================\n");
}

void processar_resultado(const char *msg) {
    char buffer[BUFFER_SIZE];
    strcpy(buffer, msg);
    
    char *token = strtok(buffer, "|");
    if (token == NULL || strcmp(token, "RESULTADO") != 0) return;
    
    token = strtok(NULL, "|");
    int seus_pontos = atoi(token);
    
    token = strtok(NULL, "|");
    int pontos_dealer = atoi(token);
    
    token = strtok(NULL, "|");
    char cartas_dealer[BUFFER_SIZE] = "";
    if (token != NULL) strcpy(cartas_dealer, token);
    
    token = strtok(NULL, "|");
    char resultado[BUFFER_SIZE] = "";
    if (token != NULL) strcpy(resultado, token);
    
    token = strtok(NULL, "|");
    int jogador1_pontos = -1;
    if (token != NULL) jogador1_pontos = atoi(token);
    
    token = strtok(NULL, "|\n");
    int jogador2_pontos = -1;
    if (token != NULL) jogador2_pontos = atoi(token);
    
    limpar_tela();
    printf("\n");
    printf("########################################\n");
    printf("#                                      #\n");
    printf("#         RESULTADO FINAL              #\n");
    printf("#                                      #\n");
    printf("########################################\n\n");
    
    printf(">>> DEALER <<<\n");
    printf("Cartas: ");
    exibir_cartas(cartas_dealer);
    printf("Pontos: %d\n\n", pontos_dealer);
    
    printf(">>> VOCE <<<\n");
    printf("Pontos: %d\n\n", seus_pontos);
    
    printf("----------------------------------------\n");
    printf("   COMPARACAO: Voce (%d) vs Dealer (%d)\n", seus_pontos, pontos_dealer);
    printf("----------------------------------------\n\n");
 
    if (strcmp(resultado, "GANHOU") == 0) {
        printf("########################################\n");
        printf("#                                      #\n");
        printf("#         *** VOCE GANHOU! ***         #\n");
        printf("#                                      #\n");
        printf("########################################\n");
    } else if (strcmp(resultado, "PERDEU") == 0) {
        printf("########################################\n");
        printf("#                                      #\n");
        printf("#         XXX VOCE PERDEU XXX          #\n");
        printf("#                                      #\n");
        printf("########################################\n");
    } else if (strcmp(resultado, "EMPATE") == 0) {
        printf("########################################\n");
        printf("#                                      #\n");
        printf("#            === EMPATE ===            #\n");
        printf("#                                      #\n");
        printf("########################################\n");
    }
    
    printf("\n");
    printf("========================================\n");
    printf("      COMPARACAO MORAL (PvP)            \n");
    printf("========================================\n");
    
    if (jogador1_pontos >= 0 && jogador2_pontos >= 0) {
        printf("   Jogador 1: %d pontos\n", jogador1_pontos);
        printf("   Jogador 2: %d pontos\n", jogador2_pontos);
        printf("----------------------------------------\n");
        
        if (jogador1_pontos > 21 && jogador2_pontos > 21) {
            printf("   Ambos estouraram!\n");
        } else if (jogador1_pontos > 21) {
            printf("   Jogador 2 venceu! (J1 estourou)\n");
        } else if (jogador2_pontos > 21) {
            printf("   Jogador 1 venceu! (J2 estourou)\n");
        } else if (jogador1_pontos > jogador2_pontos) {
            printf("   Jogador 1 venceu!\n");
        } else if (jogador2_pontos > jogador1_pontos) {
            printf("   Jogador 2 venceu!\n");
        } else {
            printf("   Empate entre jogadores!\n");
        }
    }
    
    printf("========================================\n");
    
    printf("\n");
    printf("Digite 'START' para jogar novamente\n");
    printf("Digite 'QUIT' para sair\n");
    printf("########################################\n\n");
}

DWORD WINAPI receber_mensagens(LPVOID param) {
    SOCKET sock = *(SOCKET*)param;
    char buffer[BUFFER_SIZE];
    int recv_size;
    
    while ((recv_size = recv(sock, buffer, BUFFER_SIZE - 1, 0)) > 0) {
        buffer[recv_size] = '\0';
        
        if (strncmp(buffer, "ESTADO|", 7) == 0) {
            processar_estado(buffer);
        }
        else if (strncmp(buffer, "RESULTADO|", 10) == 0) {
            processar_resultado(buffer);
        }
        else if (strncmp(buffer, "BEM_VINDO|", 10) == 0) {
            char *msg = strchr(buffer, '|') + 1;
            printf("\n%s\n", msg);
            printf("Digite 'START' quando estiver pronto para jogar\n");
        }
        else if (strncmp(buffer, "ERRO|", 5) == 0) {
            char *msg = strchr(buffer, '|') + 1;
            printf("\n[ERRO] %s\n", msg);
        }
        else if (strncmp(buffer, "ESTOUROU|", 9) == 0) {
            char *msg = strchr(buffer, '|') + 1;
            printf("\n%s\n", msg);
        }
        else {
            printf("%s", buffer);
        }
    }
    
    printf("\nConexão com servidor perdida!\n");
    return 0;
}

int main() {
    WSADATA wsa;
    SOCKET sock;
    struct sockaddr_in servidor;
    char mensagem[BUFFER_SIZE];
    
    printf("=== Cliente Blackjack ===\n");

    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        printf("Erro ao inicializar Winsock: %d\n", WSAGetLastError());
        return 1;
    }
 
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
        printf("Erro ao criar socket: %d\n", WSAGetLastError());
        return 1;
    }

    servidor.sin_family = AF_INET;
    servidor.sin_addr.s_addr = inet_addr("127.0.0.1");
    servidor.sin_port = htons(PORT);

    if (connect(sock, (struct sockaddr*)&servidor, sizeof(servidor)) < 0) {
        printf("Erro ao conectar ao servidor\n");
        return 1;
    }
    
    printf("Conectado ao servidor!\n\n");
  
    CreateThread(NULL, 0, receber_mensagens, &sock, 0, NULL);
    
    while (1) {
        printf("> ");
        fgets(mensagem, BUFFER_SIZE, stdin);
 
        mensagem[strcspn(mensagem, "\r\n")] = 0;

        for (int i = 0; mensagem[i]; i++) {
            if (mensagem[i] >= 'a' && mensagem[i] <= 'z') {
                mensagem[i] = mensagem[i] - 32;
            }
        }
        
        if (strlen(mensagem) == 0) continue;
        
        if (strcmp(mensagem, "QUIT") == 0) {
            break;
        }

        if (send(sock, mensagem, strlen(mensagem), 0) < 0) {
            printf("Erro ao enviar mensagem\n");
            break;
        }
    }
    
    closesocket(sock);
    WSACleanup();
    
    return 0;
}
