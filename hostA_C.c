#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <time.h>

#define SERVER_PORT 54321
#define N_CYCLES 10


int main(int argc, char * argv[]){
    /**=====================================
     **  Variáveis e parametros de inicio
     *======================================**/
    struct sockaddr_in server; //* Endereço Server 
    char *host; //* Usado para transferir o endereço do server passado por parâmetro
    int sock, numBytes, len; //* descritor do sockete, numero de bytes da mensagen, e temporaria para guardar tamanho da variavel server 
    int msgTam; //*Tamanho da mensagen, usado no for para indicar todas as mensagens
    long double sumLatencia; //* Usado para guardar a soma de cada teste
    int MaxByteMsg = 1000; //* Tamanho maximo da mensagem 
    long double latencia; //* Variavel temporária para calculos
    long double sumVazao; //* Usado para guardar a soma de cada teste
    char *questao; //* Usado para eqcolher se medirálatência ou vazão
    FILE * saida; //* Arquivo de saida
    if( argc == 3) { //*Trata parâmetros de entrada
        host = argv[1];
        questao = argv[2];
    }else{
        fprintf(stderr, "Usage: A_hostA_C IP_server Questao (A/B)\n");
        exit(1);
    }
    if((strcmp(questao,"A") != 0) && (strcmp(questao,"B") != 0)){ //* Check se as opções são válidas 
        fprintf(stderr, "Questao deve ser A ou B!!\n");
        exit(1);
    }
    if(questao[0] == 'A'){ //* Opção A - medição de latência 
        saida = fopen("latencia.csv", "w"); //* Inicializa as variaveis para calculo de latencia
        if(saida == NULL){
            perror("Erro na abertura do arquivo");
            exit(1);
        }
        MaxByteMsg = 1000;
        msgTam = 1;
    }else if(questao[0] == 'B'){ //* Opção B - medição de vazão
        saida = fopen("vazao.csv", "w"); //* Inicializa as variaveis para calculo de vazão
        if(saida == NULL){
            perror("Erro na abertura do arquivo");
            exit(1);
        } 
        MaxByteMsg = 32*1024; //* 1024 para converter medidas para Kbyte
        msgTam = 1024; 
    }
        
    char bufEnv[MaxByteMsg]; //* Usado para mandar mensagens
    char bufRec[MaxByteMsg]; //* Usado para receber mensagens

    
     /**======================
     **      Inicializa server
     *========================**/
    printf("Inicializando client...\n");
    memset(&server, 0, sizeof(server));  //* Configura os parâmetros do endereço do servidor 
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = inet_addr(host);
    server.sin_port = htons(SERVER_PORT);
    
    /**======================
     **      Realiza a conexão
     *========================**/
    if((sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0){  //* Criação do socket 
        perror("Erro na criação do socket:");
        exit(1);
    }


    /**======================
     **      Inicia teste
     *========================**/
    printf("Inicia teste com host: %s:%d\n", host, SERVER_PORT);
    clock_t inicio, fim;
    len =  sizeof(server);
    if(questao[0] == 'A'){//* Configuração do arquivo para medição de latência 
        fprintf(saida,"Tamanho mensagem (bytes);Latencia 1 (s); Latencia 2 (s); Latencia 3 (s); Media (s)\n"); 
    }else if(questao[0] == 'B'){//* Configuração do arquivo para medição de vazão
        fprintf(saida,"Tamanho mensagem (bytes);Vazao 1 (M bits/s); Vazao 2 (M bits/s); Vazao 3 (M bits/s); Media (M bits/s)\n"); 
    }
    
    for(int j = 0; msgTam <= MaxByteMsg; j++){
        memset(bufEnv, 'A', sizeof(char) * msgTam); //* Preenche mensagem de envio 
        bufEnv[msgTam-1] = '\0';
        sumLatencia = 0;
        sumVazao = 0;
        fprintf(saida,"%d;",msgTam); //* Coloca o tamanho da mensagem
        for(int numTeste = 0; numTeste < 3; numTeste++){ //* Repete cada teste 3 vezes
            inicio = clock(); //*Inicia a contagem de tempo para 100000 repetições
            for (int i = 0; i < N_CYCLES; i++ ){ //* Envia e recebe  mensagens 
                numBytes = strlen(bufEnv) + 1;
                if(sendto(sock, (char *)bufEnv, numBytes, 0, (const struct sockaddr *) &server, sizeof(server)) < 0){
                    perror("Erro ao enviar mensagem:");
                    close(sock);
                    exit(1);
                }
                numBytes = recvfrom(sock, (char *)bufRec, sizeof(bufRec), 0, (struct sockaddr *) &server, &len);
                if(numBytes < 0){
                    perror("Erro ao receber mensagem:");
                    close(sock);
                    exit(1); 
                }
                if(numBytes != strlen(bufEnv) + 1){ 
                    fprintf(stderr, "Mensage recebida incompleta!\n");
                }
            }
            fim = clock();
            latencia = ((fim-inicio)/(double)CLOCKS_PER_SEC)/((double)N_CYCLES); //* Calculo para latência 
            sumLatencia += latencia;
            sumVazao += ((msgTam * 8)/(latencia * 1000000)); //* Calculo para vazão em Mbits/s
            if(questao[0] == 'A'){
                fprintf(saida,"%Lf;",latencia);
            }else if(questao[0] == 'B'){
                fprintf(saida,"%Lf;",(msgTam * 8)/(latencia *1000000));
            }
        }
        snprintf(bufEnv, sizeof(char) * MaxByteMsg, "CONF M-Teste com %d bytes concluído", msgTam); //* Mensagens de controle 
        numBytes = strlen(bufEnv) + 1;
        if(sendto(sock, (char *)bufEnv, numBytes, 0, (const struct sockaddr *) &server, sizeof(server)) < 0){
            perror("Erro ao enviar mensagem:");
            close(sock);
            exit(1);
        }
        if(questao[0] == 'A'){
            fprintf(saida,"%Lf\n", (sumLatencia/3));
            msgTam = 100*j + 10*(j==0); //* Ajusta tamanho da mensagem 
        }else if(questao[0] == 'B'){
            fprintf(saida,"%Lf\n", (sumVazao/3));
            msgTam = (j + 2)*1024; //* Ajusta tamanho da mensagem 
        }
    }
    snprintf(bufEnv, sizeof(char) * MaxByteMsg, "CONF F"); //*Mensagem de encerramento de teste
    numBytes = strlen(bufEnv) + 1;
    if(sendto(sock, (char *)bufEnv, numBytes, 0, (const struct sockaddr *) &server, sizeof(server)) < 0){
        perror("Erro ao enviar mensagem:");
        close(sock);
        exit(1);
    }
    close(sock);
}