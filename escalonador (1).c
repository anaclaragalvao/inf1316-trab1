//Ana Clara Galvao - 2220505
//Gabriela Soares - 2210347

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#define SIZE 50
int executando; // 1(algum processo esta executando) 0(nenhum esta executando)

// Estrutura para armazenar informações sobre um processo
typedef struct {
  int tipo;        // Tipo de processo: 0 (RT), 1 (PRIO), 2 (RR)
  char nome[SIZE]; // Nome do processo
  int prioridade;  // Prioridade do processo (para PRIO)
  int inicio;      // Instante inicial (para RT)
  int duracao;     // Duração (para RT)
  int execucao; // 0 se estiver livre, 1 se estiver executando, -1 se estiver em
                // espera
  pid_t pid;
  int tempoTotal; // Tempo total de execução do processo
} Processo;

Processo lista_processos[SIZE];
int num_processos = 0;
int num_RT=0;
int executando_RT=0;
int executando_PRIO=0;
int num_PRIO = 0;

// Função para criar um novo processo
void criarProcesso(Processo processo) {
  pid_t pid = fork();

  if (pid == -1) {
    perror("fork");
    exit(EXIT_FAILURE);
  }

  if (pid == 0) { // Processo filho
    processo.pid = getpid();
    exit(EXIT_FAILURE);
  } else { // Processo pai
    // Espera o processo filho terminar
    wait(NULL);
  }
}

// Função para comparar prioridades e ordenar os processos
int comparar_prioridade(const void *a, const void *b) {
  const Processo *pa = (const Processo *)a;
  const Processo *pb = (const Processo *)b;

  // Ordenar por prioridade: REAL-TIME > PRIO 0 > PRIO 7 > ROUND-ROBIN
  if (pa->tipo < pb->tipo)
    return -1;
  else if (pa->tipo > pb->tipo)
    return 1;
  else {
    if (pa->tipo == 1) {
      // Comparar prioridade para PRIO
      return (pa->prioridade < pb->prioridade) ? -1 : 1;
    } else {
      // Ordenar os demais processos (RR)
      return 0;
    }
  }
}

void continuaProcesso(Processo processo) {
  kill(processo.pid, SIGCONT);
  executando = 1;
  printf("Executando processo %s\n", processo.nome);
  processo.execucao=1;
  if(processo.tipo==1){
    executando_PRIO=1;
  }
}

void pausaProcesso(Processo processo) {
  kill(processo.pid, SIGSTOP);
  printf("Parando processo %s\n", processo.nome);
  executando = 0;
  processo.execucao=0;
  if(processo.tipo==1){
    executando_PRIO=0;
  }
}

int buscar_elemento(int inicio) {
  for (int i = inicio; i < num_processos; i++) {
    if (lista_processos[i].execucao == 1) {
      return i; // Retorna a posição se o elemento for encontrado
    }
  }
  return -1; // Retorna -1 se o elemento não for encontrado
}

void colocaFinal(int pos) {
    int j;

    // Verifica se o elemento existe no lista_processos
    if (num_processos<=pos) {// Se o elemento não foi encontrado, retorna -1 indicando erro
      return;
    }

    // Move os elementos para preencher o espaço do elemento removido
    Processo ultimo=lista_processos[pos];
    for (j = pos; j < num_processos-1; j++) {
        lista_processos[j] = lista_processos[j + 1];
    }
    lista_processos[num_processos-1]=ultimo;

    return;
}



int verificarConflitos(int inicio, int duracao)
{
  for (int i = 0; i < num_RT; i++){
      if ((inicio >= lista_processos[i].inicio) && (inicio < (lista_processos[i].inicio + lista_processos[i].duracao))){
        return 0;
      }

      if (((inicio + duracao) >= lista_processos[i].inicio) && ((inicio + duracao) <= (lista_processos[i].inicio + lista_processos[i].duracao))){
        return 0;
      }
    }

    if ((inicio + duracao) > 60){
      return 0;
    }

    return 1;
}



void executarProcesso(int segundoAtual){
  for(int j = 0; j < num_RT; j++){
    if(executando_RT==1){
      lista_processos[j].tempoTotal+=1;
      if(lista_processos[j].tempoTotal==301){//ve se acabou o tempo daquele processo, RT min 5 min
        kill(lista_processos[j].pid, SIGKILL);
        printf("Matamos o processo %s\n", lista_processos[j].nome);
        colocaFinal(j);
        num_processos-=1;
        num_RT-=1;
        executando_RT=0;
        executando=0;
      }
    }
    else{
      if(segundoAtual == lista_processos[j].inicio){
        
        int pos = buscar_elemento(num_processos);//buscar o elemento
        if(pos!=-1){//se tiver alguem executando paramos o processo
          pausaProcesso(lista_processos[pos]);
          colocaFinal(pos);
        }
        //}
        continuaProcesso(lista_processos[j]);
        executando_RT=1;
      }
    }
    if(segundoAtual==lista_processos[j].inicio+lista_processos[j].duracao && executando_RT==1){
      pausaProcesso(lista_processos[j]);
      executando_RT=0;
    }
  }
  if(!executando_RT  && lista_processos[num_RT].tipo==1){
      continuaProcesso(lista_processos[num_RT]);
      lista_processos[num_RT].tempoTotal+=1;
      //pausaProcesso(lista_processos[num_RT]);
      if(lista_processos[num_RT].tempoTotal==20){
        kill(lista_processos[num_RT].pid, SIGKILL);
        printf("Matamos o processo %s\n", lista_processos[num_RT].nome);
        colocaFinal(num_RT);
        num_processos-=1;
        num_PRIO-=1;
        executando_PRIO=0;
        executando=0;
      }
  }  
  else if (!executando_PRIO && !executando_RT && lista_processos[num_PRIO+num_RT].tipo==2){
      continuaProcesso(lista_processos[num_PRIO+num_RT]);
      lista_processos[num_PRIO+num_RT].tempoTotal+=1;
      sleep(1/2);
      if(lista_processos[num_PRIO+num_RT].tempoTotal==301){//ve se acabou o tempo daquele processo, RR min 5 min
        kill(lista_processos[num_PRIO+num_RT].pid, SIGKILL);
        printf("Matamos o processo %s\n", lista_processos[num_PRIO+num_RT].nome);
        colocaFinal(num_PRIO+num_RT);
        num_processos-=1;
        executando=0;
      }
      pausaProcesso(lista_processos[num_PRIO+num_RT]);
      colocaFinal(num_PRIO+num_RT);
  }
}

int buscar_elemento1(Processo lista_processo[], int num_processos) {
  for (int i = 0; i < num_processos; i++) {
    if (lista_processos[i].execucao == 1) {
      return i; // Retorna a posição se o elemento for encontrado
    }
  }
  return -1; // Retorna -1 se o elemento não for encontrado
}


int main() {
  int pipefd[2];
  pid_t pid;
  char linha[SIZE];
  int segundoAtual = 0;


  if (pipe(pipefd) == -1) {
    perror("pipe");
    exit(EXIT_FAILURE);
  }

  pid = fork();

  if (pid == -1) {
    perror("fork");
    exit(EXIT_FAILURE);
  }

  else if (pid == 0) { // Processo filho
    close(pipefd[1]);  // Fecha a escrita no pipe no processo filho

    // Ler todos os processos do pipe
    while (read(pipefd[0], linha, sizeof(linha)) > 0) {
      Processo processo;
      sscanf(linha, "%s", processo.nome);
      if (strstr(linha, "PR=")) {
        processo.tipo = 1;
        sscanf(linha, "%*s PR=%d", &processo.prioridade);
      } else if (strstr(linha, "RR")) {
        processo.tipo = 2;
      } else if (strstr(linha, "I=") && strstr(linha, "D=")) {
        processo.tipo = 0;
        sscanf(linha, "%*s I=%d D=%d", &processo.inicio, &processo.duracao);
      } else {
        printf("Formato inválido para o processo: %s\n", linha);
        continue;
      }
      int temConflito=1;//inicializa como verdadeira
      if(processo.tipo==0){
        if (processo.inicio + processo.duracao > 60){
          fprintf(stderr, "Erro: O tempo de execução excede 60 segundos.\n");
        }
        temConflito= verificarConflitos(processo.inicio, processo.duracao);
      }
      else if(processo.tipo==1){
        num_PRIO+=1;
      }
      processo.tempoTotal=0;
      
    
      if(temConflito==0){
        printf("Nao pode adicionar o processo %s pois tem conflito\n", processo.nome);
      }
      else{
        lista_processos[num_processos++] = processo;
        criarProcesso(processo);
        qsort(lista_processos, num_processos, sizeof(Processo), comparar_prioridade);
        if(processo.tipo==0){
          num_RT+=1;
        }
        executarProcesso(segundoAtual);
        segundoAtual=(segundoAtual+1)%60;//passando segundo p segundo ate 1 minuto
        sleep(1);
      }
    }

    while(1){
      sleep(1);
      executarProcesso(segundoAtual);
      segundoAtual=(segundoAtual+1)%60;//passando segundo p segundo ate 1 minuto
    }

    // Executar os processos na ordem ordenada
    printf("%d\n",num_RT);
    printf("%d\n",num_processos);
    for (int i = 0; i < num_processos; i++) {
      printf("%s\n", lista_processos[i].nome);
    }

    //Processo lista_nova[num_processos-1];

    close(pipefd[0]); // Fechar a leitura do pipe no processo filho
    exit(EXIT_SUCCESS);
  } else {            // Processo pai
    close(pipefd[0]); // Fecha a leitura no pipe no processo pai

    FILE *listaProcessos;
    listaProcessos = fopen("processos.txt", "r");
    if (listaProcessos == NULL) {
      printf("Erro ao abrir o arquivo\n");
      exit(EXIT_FAILURE);
    }

    // Enviar processos ao processo filho pelo pipe
    while (fgets(linha, sizeof(linha), listaProcessos) != NULL) {
      write(pipefd[1], linha, strlen(linha) + 1);
      sleep(1); // Simular intervalo entre envios de processos
    }

    fclose(listaProcessos);
    close(pipefd[1]); // Fechar a escrita no pipe no processo pai
    wait(NULL);       // Esperar pelo processo filho terminar
    exit(EXIT_SUCCESS);
  }
}
