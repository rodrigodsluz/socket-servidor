#include<string.h>
#include<stdio.h>

#include<winsock2.h>
#include<sys/types.h>

#include<locale.h>


#define BUFFER 1024
#define PORT 10222


int verificaSoma(char *buffer, int tamanhoMensagem,char *verifica);


int main(){
    setlocale(LC_ALL,"Portuguese");

    char nomeArquivo[50],portaClienteFonte[6];
    WSADATA data;
    SOCKET socketCliente;
    FILE *arquivo;
    struct sockaddr_in enderecoServidor,enderecoClinte;
    char *buffer,ch[3];
    int receTamanhoMensagem, contaPacote, status, portaCliente, tamanEndereco=sizeof(SOCKADDR);

    buffer=(char*)malloc(BUFFER);

    ///Iniciando soket
    if(WSAStartup(MAKEWORD(2, 2), &data)!=0){
        printf("ERRO WSAStartup\n");
        return -1;
    }

    ///Inicia socket local
    socketCliente=socket(AF_INET, SOCK_DGRAM, 0);
    if(socketCliente < 0){
        printf("Erro ao iniciar socket\n");
        return -1;
    }

    ///Dados do socket do cliente
    memset(&enderecoServidor, 0, sizeof(enderecoServidor));
    enderecoServidor.sin_family = AF_INET;
    enderecoServidor.sin_port = htons(PORT);
    enderecoServidor.sin_addr.s_addr = inet_addr("127.0.0.1");

    while(1){

        printf("Digite o nome do arquivo: \n");
        scanf("%s",nomeArquivo);
        strcpy(buffer,nomeArquivo);

        if (sendto(socketCliente, buffer, strlen(buffer) , 0 , (struct sockaddr *) &enderecoServidor, sizeof(enderecoServidor)) == SOCKET_ERROR){
            printf("sendto() falhou, ERRO: %d\n" , WSAGetLastError());
            exit(EXIT_FAILURE);

        }

        printf("\n Esperando resposta do servidor\n");

        status=0;
        while(1){
            memset(buffer,'\0', BUFFER);
            if ((receTamanhoMensagem=recvfrom(socketCliente, buffer, BUFFER, 0, (struct sockaddr *) &enderecoServidor, &tamanEndereco)) == SOCKET_ERROR){
                printf("recvfrom() falhou, ERRO: %d\n" , WSAGetLastError());
                exit(EXIT_FAILURE);

            }
            if(buffer[0]!='\0'){
                if(buffer[0]=='1'){

                    buffer[1]='1';
                    strcpy(portaClienteFonte,&buffer[1]);
                    status=1;
                    printf("Dados recebidos\n");
                    printf("Porta: %s\n",portaClienteFonte);
                    break;
                }
                else if (buffer[0]=='0'){
                    printf("Arquivo não encontrado.\n%s\n",&buffer[1]);
                    break;
                }
            }

        }
        if(status==1){
                printf("Solicitando arquivo no cliente fonte");
                break;
            }
    }
  
    ///definindo enderecoClinte
    memset(&enderecoClinte, 0, sizeof(enderecoServidor));
    enderecoClinte.sin_family = AF_INET;
    portaCliente=atoi(portaClienteFonte);
    enderecoClinte.sin_port = htons(portaCliente);
    enderecoClinte.sin_addr.s_addr = inet_addr("127.0.0.1");

    memset(buffer,'\0', BUFFER);
    strcpy(buffer,nomeArquivo);

    ///nome enviado
    if (sendto(socketCliente, buffer, strlen(buffer) , 0 , (struct sockaddr *) &enderecoClinte, sizeof(enderecoClinte)) == SOCKET_ERROR){
        printf("sendto() falhou, ERRO: %d\n" , WSAGetLastError());
        exit(EXIT_FAILURE);
    }
    while(1){
        memset(buffer,'\0', BUFFER);
        if ((receTamanhoMensagem=recvfrom(socketCliente, buffer, BUFFER, 0, (struct sockaddr *) &enderecoClinte, &tamanEndereco)) == SOCKET_ERROR){
            printf("recvfrom() falhou, ERRO: %d\n" , WSAGetLastError());
            exit(EXIT_FAILURE);
        }
        if(buffer[0]!='\0'){
            if(strcmp(buffer,"5")==0){
                printf("Arquivo localizado\nIniciando transferencia do arquivo %s\n",nomeArquivo);
                arquivo=fopen(nomeArquivo,"wb");
                break;
            }
            else if (strcmp(buffer,"0Erro-0002")==0){
                printf("Aquivo n�o localizado\n%s\n",&buffer[1]);
            }
        }
    }

    contaPacote=0;
    while(1){
        memset(buffer,'\0', BUFFER);
        if ((receTamanhoMensagem=recvfrom(socketCliente, buffer, BUFFER, 0, (struct sockaddr *) &enderecoClinte, &tamanEndereco)) == SOCKET_ERROR){
            printf("recvfrom() falhou, ERRO: %d\n" , WSAGetLastError());
            exit(EXIT_FAILURE);
        }
        if(buffer[0]!='\0'){
            if(buffer[0]=='1'){

                verificaSoma(buffer,receTamanhoMensagem,ch);
                if(ch[0]==buffer[2] && ch[1]==buffer[3]){
                    fwrite(&buffer[4],1,receTamanhoMensagem-4,arquivo);
                    contaPacote++;
                    strcpy(buffer,"ack1");
                    printf("buffer = %s\n",buffer);

                    if (sendto(socketCliente, buffer, strlen(buffer) , 0 , (struct sockaddr *) &enderecoClinte, sizeof(enderecoClinte)) == SOCKET_ERROR){
                        printf("sendto() falhou, ERRO: %d\n" , WSAGetLastError());
                        exit(EXIT_FAILURE);
                    }

                    printf("pacote %d recebido\n",contaPacote);

                }
                else{

                    strcpy(buffer,"ack0");
                    if (sendto(socketCliente, buffer, strlen(buffer) , 0 , (struct sockaddr *) &enderecoClinte, sizeof(enderecoClinte)) == SOCKET_ERROR){
                        printf("sendto() falhou, ERRO: %d\n" , WSAGetLastError());
                        exit(EXIT_FAILURE);
                    }

                }
            }
            else if(buffer[0]=='0'){

                verificaSoma(buffer,receTamanhoMensagem,ch);
                if(ch[0]==buffer[2] && ch[1]==buffer[3]){
                    fwrite(&buffer[4],1,receTamanhoMensagem-4,arquivo);
                    contaPacote++;
                    strcpy(buffer,"ack1");
                    if (sendto(socketCliente, buffer, strlen(buffer) , 0 , (struct sockaddr *) &enderecoClinte, sizeof(enderecoClinte)) == SOCKET_ERROR){
                        printf("sendto() falhou, ERRO: %d\n" , WSAGetLastError());
                        exit(EXIT_FAILURE);
                    }

                    printf("pacote %d recebido\n",contaPacote);
                    printf("Transferencia Efetivada\n");
                    break;
                }
                else{
                    strcpy(buffer,"ack0");
                    if (sendto(socketCliente, buffer, strlen(buffer) , 0 , (struct sockaddr *) &enderecoClinte, sizeof(enderecoClinte)) == SOCKET_ERROR){
                        printf("sendto() falhou, ERRO: %d\n" , WSAGetLastError());
                        exit(EXIT_FAILURE);
                    }
                }
            }
        }
    }
    fclose(arquivo);
system("pause");
}

int verificaSoma(char *buffer, int tamanhoMensagem,char *verifica){

    unsigned int soma=0,i;
    char ch[3];
    for(i=4;i<tamanhoMensagem;i++){
        soma+=buffer[i];
    }
    soma=soma%16;
    if(soma<10){
        soma=soma*10;
    }
    printf("Verifica soma: %d\n",soma);
    itoa(soma,ch,10);
    verifica[0]=ch[0];
    verifica[1]=ch[1];
    return 0;

}