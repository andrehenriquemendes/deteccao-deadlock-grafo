# deteccao-deadlock-grafo
Aplicação que simula a detecção de deadlock através da verificação de ciclos no grafo de recursos e processos.</br></br>
Desenvolvido para a disciplina de Sistemas Operacionais na Universidade Federal de São Paulo.
</br>
</br>
## Relatório completo disponível em:
https://drive.google.com/file/d/1igpsvBFOOg22LIfAJI9O8DsX3hSh_hVV/view?usp=sharing
</br>
</br>
## Relatório sucinto:
### Para compilar e rodar a aplicação, digite os comandos abaixo no terminal:
```
gcc -Wall -o deadlock deadlock.c -lpthread
gcc -Wall -shared -o mysemaphore.so mysemaphore.c -ldl -fPIC
LD_PRELOAD=./mysemaphore.so ./deadlock
```
</br>
</br>

### Descrição do funcionamento:

Duas threads (processos leves) simulam acesso a dois recursos (A e B). Eventualmente, caso haja uma condição de espera circular (isto é, cada processo encontra-se
à espera de um recurso que está sendo usado pelo outro), então há o deadlock.

Essa implementação usa grafos direcionados, em que as threads e os recursos representam os vértices, e as solicitações/alocações de um recurso a um processo representam as arestas. A detecção de um deadlock é realizada pela verificação de um ciclo no grafo.

Para ter um melhor entendimento, sugiro que acesse o relatório completo:
https://drive.google.com/file/d/1igpsvBFOOg22LIfAJI9O8DsX3hSh_hVV/view?usp=sharing
