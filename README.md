# Blackjack Cliente-Servidor em C

Jogo de Blackjack implementado em C usando sockets (winsock2.h) compatível com Windows.

## Arquitetura

- **Servidor (server.c)**: Contém toda a lógica do jogo
  - Gerencia o baralho e embaralhamento
  - Controla turnos dos jogadores
  - Calcula pontuações
  - Determina vencedores

- **Cliente (cliente.c)**: Interface simples ("burra")
  - Apenas envia comandos
  - Recebe e exibe informações do servidor
  - Não contém lógica de jogo

## Protocolo de Comunicação

### Comandos do Cliente → Servidor:
- `START` - Inicia uma nova rodada
- `HIT` - Pede uma carta
- `STAND` - Para de pedir cartas
- `QUIT` - Sai do jogo

### Mensagens do Servidor → Cliente:

**ESTADO** (durante o jogo):
\`\`\`
ESTADO|vez|pontos|cartas|carta_dealer|status
\`\`\`
Exemplo: `ESTADO|0|15|5C,TD|AC|SUA_VEZ`

**RESULTADO** (fim do jogo):
\`\`\`
RESULTADO|pontos_jogador|pontos_dealer|cartas_dealer|resultado
\`\`\`
Exemplo: `RESULTADO|20|19|AC,8E,TE|GANHOU`

**Notação das Cartas:**
- Valores: A (Ás), 2-9, T (10), J (Valete), Q (Dama), K (Rei)
- Naipes: C (Copas), E (Espadas), O (Ouros), P (Paus)
- Exemplo: `AC` = Ás de Copas, `TD` = 10 de Ouros

## Compilação

### Com GCC (MinGW no Windows):
\`\`\`bash
gcc server.c -o server.exe -lws2_32
gcc cliente.c -o cliente.exe -lws2_32
\`\`\`

### Com Visual Studio:
- Criar novo projeto Console Application
- Adicionar os arquivos .c
- O pragma comment já inclui ws2_32.lib automaticamente

## Como Jogar

1. **Inicie o servidor:**
   \`\`\`bash
   server.exe
   \`\`\`
   O servidor ficará aguardando na porta 8080.

2. **Conecte os clientes** (em terminais separados):
   \`\`\`bash
   cliente.exe
   \`\`\`
   Suporta até 2 jogadores simultâneos.

3. **Comandos no cliente:**
   - Digite `START` para iniciar o jogo
   - Digite `HIT` para pedir carta
   - Digite `STAND` para parar
   - Digite `QUIT` para sair

## Regras do Blackjack

- Objetivo: Chegar o mais próximo de 21 sem ultrapassar
- Cartas 2-10 valem seu valor
- J, Q, K valem 10
- Ás vale 11 ou 1 (automático)
- Se ultrapassar 21, você "estoura" e perde
- Dealer pega cartas até ter pelo menos 17 pontos
- Quem tiver mais pontos (sem estourar) ganha

## Características Técnicas

- **100% compatível com Windows** (usa winsock2.h)
- **Sem pthreads.h** (usa CreateThread do Windows)
- **Multi-threaded**: Cada cliente em sua própria thread
- **Servidor centralizado**: Toda lógica no servidor
- **Cliente simples**: Apenas UI e comunicação
- **Protocolo baseado em texto**: Fácil de debugar

## Estrutura do Código

### Servidor:
- Gerenciamento de sockets e threads
- Estruturas de dados para cartas, baralho e jogadores
- Lógica completa do Blackjack
- Sistema de turnos e broadcast de estado

### Cliente:
- Conexão ao servidor
- Thread para receber mensagens assincronamente
- Interface de linha de comando
- Formatação visual das cartas e pontuações
