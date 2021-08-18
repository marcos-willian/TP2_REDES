#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#define SERVER_PORT 54321
#define SERVER_IP INADDR_ANY
const int MaxByteMsg = 32*1024; //* Tamanho maximo suportado pelas duas aplicações 

int main(){
    printf("Inicializando servidor...\n");
    /**======================
     **      Variáveis
     *========================**/
    struct sockaddr_in server, client_addr; //* Endereços do cliente e do server
    char buf[MaxByteMsg]; //* buffer de troca de mensagesn 
    int len, numBytes, sock; //* Tamanho da variavel cliente, numero de bytes recebidos, descritor do socket

    /**======================
     **      Inicializa servidor
     *========================**/
    memset(&server, 0, sizeof(server)); //* Configura os parâmetros do endereço do servidor 
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = SERVER_IP; 
    server.sin_port = htons(SERVER_PORT);

    if((sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0){ //* Criação do socket 
        perror("Erro na criação do socket:");
        exit(1);
    }
    if((bind(sock, (struct sockaddr *) &server, sizeof(server))) < 0){ //* Realiza o bind com o socket 
        perror("Erro no bind:");
        close(sock);
        exit(1);  
    }
    printf("Servidor: %s:%d  inicializado com sucesso\n",  inet_ntoa(server.sin_addr), SERVER_PORT);
    /**=====================================
     **      Interação com o client
     *======================================**/
    len = sizeof(client_addr);
    while(1){
        numBytes = recvfrom(sock, (char *)buf, sizeof(buf), 0, (struct sockaddr *) &client_addr, &len); //* Recebe mensagens de outra aplicação 
        if(numBytes < 0){
            perror("Erro ao receber mensagem:");
            close(sock);
            exit(1); 
        }
        buf[numBytes] = '\0';
        if(strncmp(buf, "CONF",4) == 0){ //* Trata as mensagens de controle 
            if(buf[5] == 'M'){
                strtok(buf, "-");
                printf("%s\n", strtok(NULL,"-"));
            }else if(buf[5] == 'F'){ //* Caso o comando seja de finalização 
                printf("Teste finalizado!!\n");
                exit(1);
            } 
        }else{
            numBytes = strlen(buf) + 1; //* Se não for mensagem de controle reflete
            if(sendto(sock, (char *)buf, numBytes, 0, (const struct sockaddr *) &client_addr, sizeof(client_addr)) < 0){ 
                perror("Erro ao enviar mensagem:");
                close(sock);
                exit(1);
            }
        }
               
    }
    close(sock);

}