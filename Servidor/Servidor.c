#include <stdio.h>
#include <locale.h>
#include <winsock2.h>
#include <string.h>
#include <windows.h>
#include <sys/types.h>

#define BUFFER 1024
#define PORT 10222

typedef struct
{
    char clientePorta[3];

    char arquivoNome[100];

    char clienteIP[16];
} Cliente;

int adicionarArquivo();

int main(void)
{
    setlocale(LC_ALL, "Portuguese");

    int i, msgTamanhoRec, erro, arquivoQtd, status;

    printf("Digite 2 para iniciar o servidor: \n");

    scanf("%d", &i);

    if (i == 1)
        adicionarArquivo();

    SOCKET serverSocket;

    struct sockaddr_in serverAddr, clienteAddr;

    WSADATA dado;

    int tamAddr = sizeof(SOCKADDR);

    char *buffer = (char *)malloc(BUFFER), arquivoNome[50];

    FILE *dadosRepositorio = fopen("dadosRepositorio.bs", "rb");

    fread(&arquivoQtd, sizeof(arquivoQtd), 1, dadosRepositorio);

    Cliente dados_repositorio[arquivoQtd], dados_arq;

    fread(dados_repositorio, sizeof(dados_repositorio), arquivoQtd, dadosRepositorio);

    fclose(dadosRepositorio);

    if (WSAStartup(MAKEWORD(2, 2), &dado) != 0)
    {
        printf("Erro\n");
        return -1;
    }

    serverSocket = socket(AF_INET, SOCK_DGRAM, 0);
    if (serverSocket < 0)
    {
        printf("Erro\n");
        return -1;
    }

    memset(&serverAddr, 0, sizeof(serverAddr));

    serverAddr.sin_family = AF_INET;

    serverAddr.sin_port = htons(PORT);

    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(serverSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0)
    {
        printf("Erro bind() : %i\n", erro = WSAGetLastError());
        return -1;
    }

    listen(serverSocket, 1);

    printf("Carregando\n");

    while (1)
    {
        memset(buffer, '\0', BUFFER);
        if ((msgTamanhoRec = recvfrom(serverSocket, buffer, BUFFER, 0, (struct sockaddr *)&clienteAddr, &tamAddr)) == SOCKET_ERROR)
        {
            printf("recvfrom() erro : %d\n", WSAGetLastError());
            exit(EXIT_FAILURE);
        }
        if (buffer[0] != '\0')
        {
            strcpy(arquivoNome, buffer);

            status = 0;

            for (i = 0; i < arquivoQtd; i++)
            {
                if (strcmp(dados_repositorio[i].arquivoNome, arquivoNome) == 0)
                {
                    dados_arq = dados_repositorio[i];
                    printf("arquivo %s localizado\n", arquivoNome);
                    status = 1;
                    break;
                }
            }

            memset(buffer, '\0', BUFFER);

            if (status)
            {
                buffer[0] = '1';

                strcat(buffer, dados_arq.clientePorta);

                if (sendto(serverSocket, buffer, strlen(buffer), 0, (struct sockaddr *)&clienteAddr, sizeof(clienteAddr)) == SOCKET_ERROR)
                {
                    printf("sendto() erro : %d\n", WSAGetLastError());
                    exit(EXIT_FAILURE);
                }

                printf("O Arquivo foi enviado\n No aguardo...\n");
            }
            else if (status == 0)
            {
                strcpy(buffer, "0Erro-0001");

                if (sendto(serverSocket, buffer, strlen(buffer), 0, (struct sockaddr *)&clienteAddr, sizeof(clienteAddr)) == SOCKET_ERROR)
                {
                    printf("sendto() erro : %d\n", WSAGetLastError());
                    exit(EXIT_FAILURE);
                }
                printf("O arquivo não foi encontrado\n No aguardo...\n");
            }
        }
    }

    return 0;
}

int adicionarArquivo()
{
    int arquivoQntd = 0;

    FILE *dadosRepositorio;

    dadosRepositorio = fopen("dadosRepositorio.bs", "rb");

    fread(&arquivoQntd, sizeof(int), 1, dadosRepositorio);

    Cliente add, atual[arquivoQntd];

    fread(&atual, sizeof(Cliente), arquivoQntd, dadosRepositorio);
    
    fclose(dadosRepositorio);

    printf("Entre com o nome do arquivo\n");

    scanf("%s", add.arquivoNome);

    fflush(stdin);

    printf("Entre com a porta do cliente\n");

    scanf("%s", add.clientePorta);

    fflush(stdin);

    strcpy(add.clienteIP, "127.0.0.1");

    arquivoQntd = arquivoQntd + 1;

    dadosRepositorio = fopen("dadosRepositorio.bs", "wb");

    fwrite(&arquivoQntd, sizeof(int), 1, dadosRepositorio);

    fwrite(&atual, sizeof(Cliente), arquivoQntd - 1, dadosRepositorio);

    fwrite(&add, sizeof(Cliente), 1, dadosRepositorio);

    fclose(dadosRepositorio);

    printf("Os dados foram armazenados no repositório de dados!!!\n");

    return 0;
}
{"mode":"full","isActive":false}