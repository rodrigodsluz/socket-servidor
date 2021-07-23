#include<stdio.h>
#include<winsock2.h>

#include <sys/types.h>
#include <windows.h>

#include<string.h>
#include<locale.h>

#define PORTA 10222
#define BUFFER 1024

//--------------------------------------------------------------------------
int adicionarAquivo();

typedef struct{
    char nome_arq[50];
    char porta_cliente[6];
    char ip_cliente[16];
}InformaCliente;


//--------------------------------------------------------------------------
int main(){
    int qnt_arquivos,i,rec_msg_tam,status,er;
    setlocale(LC_ALL,"Portuguese");

    printf("Iniciar Servidor?\n 2 - Iniciar\n");
    scanf("%d",&i);
    if(i==1){

        adicionarAquivo();

    }

    WSADATA data;
    SOCKET sock_servidor;

    struct sockaddr_in addr_servidor,addr_cliente;

    int addr_tam=sizeof(SOCKADDR);
    char *buffer;
    buffer = (char*)malloc(BUFFER);
    FILE *repositorio;
    repositorio=fopen("repositorio.bs","rb");

    fread(&qnt_arquivos,sizeof(qnt_arquivos),1,repositorio);
    char nome_arquivo[50];
    InformaCliente dados_repositorio[qnt_arquivos],dados_arq;

    fread(dados_repositorio,sizeof(dados_repositorio),qnt_arquivos,repositorio);
    fclose(repositorio);


    if(WSAStartup(MAKEWORD(2, 2), &data)!=0){
        printf("Falha WSAStartup\n");
        return -1;
    }


    sock_servidor=socket(AF_INET,SOCK_DGRAM,0);
    if(sock_servidor<0){
        printf("IMPOSSIVEL ISSO MEU, socket não funcionou\n");
        return -1;
    }

    memset(&addr_servidor, 0, sizeof(addr_servidor));

    addr_servidor.sin_family = AF_INET;
    addr_servidor.sin_port = htons(PORTA);
    addr_servidor.sin_addr.s_addr = htonl(INADDR_ANY);


    if(bind(sock_servidor,(struct sockaddr *)&addr_servidor , sizeof(addr_servidor))<0){

        printf("Erro bind() : %i\n",er=WSAGetLastError());
        return -1;

    }
  
    listen(sock_servidor,1);

    printf("Esperando recebimento de dados . . .\n");

    while(1){

        memset(buffer,'\0', BUFFER);
        if ((rec_msg_tam=recvfrom(sock_servidor, buffer, BUFFER, 0, (struct sockaddr *) &addr_cliente, &addr_tam)) == SOCKET_ERROR){
            printf("recvfrom() falhou, ERRO: %d\n" , WSAGetLastError());
            exit(EXIT_FAILURE);
        }
        if(buffer[0]!='\0'){
            strcpy(nome_arquivo,buffer);
            status=0;
            for(i=0;i<qnt_arquivos;i++){
                if(strcmp(dados_repositorio[i].nome_arq,nome_arquivo)==0){
                    dados_arq=dados_repositorio[i];
                    printf("O arquivo: %s foi encontrado \n",nome_arquivo);
                    status=1;
                    break;
                }
            }
            memset(buffer,'\0', BUFFER);
            if(status==1){
                buffer[0]='1';
                strcat(buffer,dados_arq.porta_cliente);
                if (sendto(sock_servidor, buffer, strlen(buffer) , 0 , (struct sockaddr *) &addr_cliente, sizeof(addr_cliente)) == SOCKET_ERROR){
                    printf("sendto() falhou, ERRO: %d\n" , WSAGetLastError());
                    exit(EXIT_FAILURE);
                }
                printf("A espera de um milagre ...\n ");
                printf("Aguardando dados '-' \n");
            }
            else if (status==0){

                strcpy(buffer,"0Erro-0001");
                if (sendto(sock_servidor, buffer, strlen(buffer) , 0 , (struct sockaddr *) &addr_cliente, sizeof(addr_cliente)) == SOCKET_ERROR){
                    printf("sendto() falhou, ERRO: %d\n" , WSAGetLastError());
                    exit(EXIT_FAILURE);
                }
                printf("Arquivo nao localizado . . . Aguardando dados '-' \n");
            }
        }

    }


}

int adicionarAquivo(){
    FILE *repositorio;
    int qnt_arquivo=0;
    repositorio=fopen("repositorio.bs","rb");

    fread(&qnt_arquivo,sizeof(int),1,repositorio);
    InformaCliente inserir,existente[qnt_arquivo];

    fread(&existente,sizeof(InformaCliente),qnt_arquivo,repositorio);

    fclose(repositorio);
    printf("Inserir nome do arquivo\n");
    scanf("%s",inserir.nome_arq);
    fflush(stdin);
    printf("Informe a porta do cliente\n");
    scanf("%s",inserir.porta_cliente);
    fflush(stdin);

    strcpy(inserir.ip_cliente,"127.0.0.1");
    qnt_arquivo++;

    repositorio=fopen("repositorio.bs","wb");
    fwrite(&qnt_arquivo,sizeof(int),1,repositorio);

    fwrite(&existente,sizeof(InformaCliente),qnt_arquivo-1,repositorio);

    fwrite(&inserir,sizeof(InformaCliente),1,repositorio);

    fclose(repositorio);

    printf("dados do arquivo inseridos no repositorio\n");
    return 0;
}
