Para acessar o jogo inicialize com os seguintes comandos

gcc server.c -o server.exe -lws2_32
gcc cliente.c -o cliente.exe -lws2_32

- Digite HIT para pedir carta
- Digite STAND para parar
- Digite QUIT para sair

- Objetivo: Chegar o mais próximo de 21 sem ultrapassar
- Cartas 2-10 valem seu valor
- J, Q, K valem 10
- Ás vale 11 ou 1 (automático)
- Se ultrapassar 21, você "estoura" e perde
- Dealer pega cartas até ter pelo menos 17 pontos
- Quem tiver mais pontos (sem estourar) ganha
