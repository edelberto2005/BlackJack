# Protocolo de Comunicação - Blackjack

## Formato das Mensagens

Todas as mensagens seguem o padrão:
\`\`\`
COMANDO|parametro1|parametro2|...|parametroN\n
\`\`\`

## Comandos Cliente → Servidor

### START
Inicia uma nova rodada do jogo.
\`\`\`
START
\`\`\`

**Resposta do servidor:**
- Envia `ESTADO` para todos os jogadores com as cartas iniciais

### HIT
Jogador pede uma carta adicional (na sua vez).
\`\`\`
HIT
\`\`\`

**Resposta do servidor:**
- `ESTADO` atualizado com a nova carta
- `ESTOUROU` se passar de 21 pontos
- `ERRO` se não for a vez do jogador

### STAND
Jogador decide parar de pedir cartas (na sua vez).
\`\`\`
STAND
\`\`\`

**Resposta do servidor:**
- Próximo jogador recebe sua vez
- Se todos pararam, dealer joga e envia `RESULTADO`

## Mensagens Servidor → Cliente

### BEM_VINDO
Mensagem de boas-vindas ao conectar.
\`\`\`
BEM_VINDO|Você é o Jogador N
\`\`\`

### ESTADO
Estado atual do jogo durante a partida.
\`\`\`
ESTADO|vez|pontos|cartas|carta_dealer|status
\`\`\`

**Parâmetros:**
- `vez`: ID do jogador da vez (0 ou 1)
- `pontos`: Pontuação atual do jogador
- `cartas`: Lista de cartas separadas por vírgula (ex: `AC,5E,TD`)
- `carta_dealer`: Primeira carta visível do dealer (ex: `KP`)
- `status`: Estado do jogador
  - `SUA_VEZ` - Pode jogar
  - `AGUARDE` - Aguardando sua vez
  - `PARADO` - Já decidiu parar
  - `ESTOUROU` - Passou de 21

**Exemplo:**
\`\`\`
ESTADO|0|17|AC,6C|KP|SUA_VEZ
\`\`\`
Significa: É a vez do jogador 0, tem 17 pontos com Ás de Copas e 6 de Copas, dealer mostra Rei de Paus.

### RESULTADO
Resultado final da rodada.
\`\`\`
RESULTADO|pontos_jogador|pontos_dealer|cartas_dealer|resultado
\`\`\`

**Parâmetros:**
- `pontos_jogador`: Pontuação final do jogador
- `pontos_dealer`: Pontuação final do dealer
- `cartas_dealer`: Todas as cartas do dealer (ex: `KP,9E,2C`)
- `resultado`: 
  - `GANHOU` - Jogador venceu
  - `PERDEU` - Jogador perdeu
  - `EMPATE` - Empate

**Exemplo:**
\`\`\`
RESULTADO|20|19|KP,9E|GANHOU
\`\`\`

### ESTOUROU
Jogador ultrapassou 21 pontos.
\`\`\`
ESTOUROU|Você estourou com N pontos!
\`\`\`

### ERRO
Mensagem de erro.
\`\`\`
ERRO|Descrição do erro
\`\`\`

**Exemplos:**
\`\`\`
ERRO|Não é sua vez!
ERRO|Jogo já está ativo!
ERRO|Comando inválido!
\`\`\`

## Notação de Cartas

Cada carta é representada por 2 caracteres: **ValorNaipe**

### Valores:
- `A` - Ás (vale 11 ou 1)
- `2` a `9` - Valores numéricos
- `T` - Dez (Ten)
- `J` - Valete (Jack)
- `Q` - Dama (Queen)
- `K` - Rei (King)

### Naipes:
- `C` - Copas (♥)
- `E` - Espadas (♠)
- `O` - Ouros (♦)
- `P` - Paus (♣)

### Exemplos:
- `AC` - Ás de Copas
- `TD` - 10 de Ouros
- `KE` - Rei de Espadas
- `5P` - 5 de Paus

## Fluxo do Jogo

1. **Conexão**
   \`\`\`
   Servidor → Cliente: BEM_VINDO|Você é o Jogador 1
   \`\`\`

2. **Início do Jogo**
   \`\`\`
   Cliente → Servidor: START
   Servidor → Todos: ESTADO|0|15|AC,4E|KP|SUA_VEZ (para jogador 0)
   Servidor → Todos: ESTADO|0|18|TC,8C|KP|AGUARDE (para jogador 1)
   \`\`\`

3. **Turno do Jogador**
   \`\`\`
   Cliente → Servidor: HIT
   Servidor → Todos: ESTADO|0|21|AC,4E,6C|KP|SUA_VEZ
   
   Cliente → Servidor: STAND
   Servidor → Todos: ESTADO|1|18|TC,8C|KP|SUA_VEZ (próximo jogador)
   \`\`\`

4. **Fim do Jogo**
   \`\`\`
   Servidor → Cliente 0: RESULTADO|21|20|KP,TD|GANHOU
   Servidor → Cliente 1: RESULTADO|18|20|KP,TD|PERDEU
   \`\`\`

## Tratamento de Erros

### Cliente tenta jogar fora da vez:
\`\`\`
Cliente → Servidor: HIT
Servidor → Cliente: ERRO|Não é sua vez!
\`\`\`

### Comando inválido:
\`\`\`
Cliente → Servidor: ATTACK
Servidor → Cliente: ERRO|Comando inválido!
\`\`\`

### Servidor cheio:
\`\`\`
Cliente → Servidor: (tentativa de conexão)
Servidor → Cliente: ERRO|Servidor cheio!
(conexão fechada)
\`\`\`

## Considerações de Implementação

1. **Quebras de linha**: Todas as mensagens terminam com `\n`
2. **Case-insensitive**: Cliente converte comandos para maiúsculas
3. **Buffer size**: 1024 bytes
4. **Separador**: Pipe `|` entre campos
5. **Codificação**: ASCII puro
6. **Timeout**: Sem timeout (conexão persistente)
