#include <stdio.h>
#include <locale.h>
#include <string.h>
#include <sys/types.h>
#include <winsock2.h>
//--------------------------------------------------------------------------
#define bufferTam 1024
#define portaLocal 10333
#define portaServidor 10222
//--------------------------------------------------------------------------
int verificarSoma(char *buffer, int mensagemTam);
//--------------------------------------------------------------------------
int main()
{
    setlocale(LC_ALL, "Portuguese");

    WSADATA data; // contém informações sobre a implementação do Windows Sockets.
    SOCKET sockCliente;

    // struct criado para realizar o endereçamento logo adiante
    struct sockaddr_in enderecoCliente1, enderecoCliente;

    int enderecoTam = sizeof(SOCKADDR), recMensagemTam, status, conteudoPacote, mensagemTam, er;

    char *buffer, nomeDoArquivo[50], chRee, *bufferRecebido;

    buffer = (char *)malloc(bufferTam);
    bufferRecebido = (char *)malloc(bufferTam);

    FILE *arquivo;

    // Inciará o Winsock para fazer outras chamadas de funções do Winsock
    if (WSAStartup(MAKEWORD(2, 2), &data) != 0)
    {
        printf("> FALHOU WSAStartup\n");
        return -1;
    }

    // Criará o socket local do cliente - AF_NET = Protocolo IPV4, SOCK_DGRAM = sem conexão, mensagens com um tamanho máximo (UDP)
    sockCliente = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockCliente < 0)
    {
        printf("> NÃO FOI POSSÍVEL INICIAR O SOCKET\n");
        return -1;
    }

    memset(&enderecoCliente, 0, sizeof(enderecoCliente));

    // vai associar as familias de protocolos, endereço local e porta respectivamente
    enderecoCliente.sin_family = AF_INET;
    enderecoCliente.sin_addr.s_addr = inet_addr("127.0.0.1");
    enderecoCliente.sin_port = htons(portaLocal);

    // Criação do bind do cliente (associa a um endereço na rede)
    //        \/ associar um endereço  \/ protocolos, endereço e porta  \/ tamanho em bytes do struck passado anteriormente
    if (bind(sockCliente, (struct sockaddr *)&enderecoCliente, sizeof(enderecoCliente)) < 0) // se a porta for menor que 0
    {
        printf("> ERRO bind() : %i\n", er = WSAGetLastError());
        return -1;
    }

    // Irá aguardar as conexões, o limite de conexão ao socket é uma
    listen(sockCliente, 1);

    printf("> ESPERANDO O CLIENTE PEDIR PARA QUE EU POSSA ENVIAR...\n");
    chRee = '1';

    // Para que o socket possa enviar algo passará por verificações
    while (1)
    {
        // ressalva que o '\0' é utilizado para desejar byte de valor zero
        // atribuirá aos primeiros bytes do buff o valor de 0 de acordo com o tamanho de bufferTam
        memset(buffer, '\0', bufferTam);

        // recvfrom = vai retornar o tamanho dos dados recebidos (por udp) (endereço dos dados, seu tamanho etc)... e o SOCKET_ERROR irá verificar se há alguma "inconsistência"
        if ((recMensagemTam = recvfrom(sockCliente, buffer, bufferTam, 0, (struct sockaddr *)&enderecoCliente1, &enderecoTam)) == SOCKET_ERROR)
        {
            printf("recvfrom() > FALHOU, CÓDIGO DE ERRO: %d\n", WSAGetLastError());
            exit(EXIT_FAILURE);
        }
        if (buffer[0] != '\0')
        {
            // copia a info do buffer para nomeDoArquivo
            strcpy(nomeDoArquivo, buffer);
            // abre o arquivo binário para leitura.
            arquivo = fopen(nomeDoArquivo, "rb");

            // checagem se arquivo existe
            if (arquivo == NULL)
            {
                // buffer receberá código erro
                strcpy(buffer, "0Erro-0002");
                // SOCKET_ERROR irá verificar se há alguma "inconsistência"
                if (sendto(sockCliente, buffer, strlen(buffer), 0, (struct sockaddr *)&enderecoCliente1, sizeof(enderecoCliente1)) == SOCKET_ERROR)
                {
                    printf("sendto() > FALHOU, CÓDIGO DE ERRO: %d\n", WSAGetLastError());
                    exit(EXIT_FAILURE);
                }
            }
            else
            {
                strcpy(buffer, "5");
                // SOCKET_ERROR irá verificar se há alguma "inconsistência"
                if (sendto(sockCliente, buffer, strlen(buffer), 0, (struct sockaddr *)&enderecoCliente1, sizeof(enderecoCliente1)) == SOCKET_ERROR)
                {
                    printf("sendto() > FALHOU, CÓDIGO DE ERRO: %d\n", WSAGetLastError());
                    exit(EXIT_FAILURE);
                }

                printf("ENVIANDO O ARQUIVO %s\n", nomeDoArquivo);

                conteudoPacote = 0;
                status = 0;
                while (1)
                {
                    memset(buffer, '\0', bufferTam);
                    mensagemTam = fread(&buffer[4], 1, bufferTam - 4, arquivo);
                    if (mensagemTam == bufferTam - 4)
                    {
                        buffer[0] = '1';
                        verificarSoma(buffer, mensagemTam);
                        status = 1;
                    }
                    else
                    {
                        buffer[0] = '0';
                        verificarSoma(buffer, mensagemTam);
                        status = 0;
                    }

                    if (chRee == '1')
                    {
                        chRee = '0';
                    }
                    else
                    {
                        chRee = '1';
                    }

                    buffer[1] = chRee;

                    // SOCKET_ERROR irá verificar se há alguma "inconsistência"
                    if (sendto(sockCliente, buffer, mensagemTam + 4, 0, (struct sockaddr *)&enderecoCliente1, sizeof(enderecoCliente1)) == SOCKET_ERROR)
                    {
                        printf("sendto() > FALHOU, CÓDIGO DE ERRO: %d\n", WSAGetLastError());
                        exit(EXIT_FAILURE);
                    }
                    conteudoPacote++;

                    // Aguardando ack
                    while (1)
                    {
                        memset(bufferRecebido, '\0', bufferTam);
                        printf("> AGUARDANDO ack \n");
                        // SOCKET_ERROR irá verificar se há alguma "inconsistência"
                        if ((recMensagemTam = recvfrom(sockCliente, bufferRecebido, bufferTam, 0, (struct sockaddr *)&enderecoCliente1, &enderecoTam)) == SOCKET_ERROR)
                        {
                            printf("recvfrom() > FALHOU, CÓDIGO DE ERRO: %d\n", WSAGetLastError());
                            exit(EXIT_FAILURE);
                        }
                        if (buffer[0] != '\0')
                        {
                            printf("> BUFFER = %s\n", bufferRecebido);
                            if (strcmp(bufferRecebido, "ack1") == 0)
                            {
                                printf("ack %d ok\n", conteudoPacote);
                                break;
                            }
                            else if (strcmp(bufferRecebido, "ack0") == 0)
                            {
                                printf("ack %d erro\n> FAZER REENVIO DO PACOTE", conteudoPacote);
                                // SOCKET_ERROR irá verificar se há alguma "inconsistência"
                                if (sendto(sockCliente, buffer, mensagemTam + 4, 0, (struct sockaddr *)&enderecoCliente1, sizeof(enderecoCliente1)) == SOCKET_ERROR)
                                {
                                    printf("sendto() > FALHOU, CÓDIGO DE ERRO: %d\n", WSAGetLastError());
                                    exit(EXIT_FAILURE);
                                }
                                system("pause");
                            }
                        }
                    }
                    if (status == 0)
                    {
                        break;
                    }
                }
            }
            printf("> ENVIO REALIZADO!\n");
            fclose(arquivo);
        }
    }
}
//--------------------------------------------------------------------------
int verificarSoma(char *buffer, int mensagemTam)
{
    char ch[3];
    //unsigned: modificador para determinar que um tipo numérico inteiro é sem sinal (só vai obter números positivos)
    unsigned int resultadoSoma = 0;

    for (int i = 4; i < mensagemTam + 4; i++)
    {
        resultadoSoma += buffer[i];
    }

    resultadoSoma = resultadoSoma % 16;

    if (resultadoSoma < 10)
    {
        resultadoSoma = resultadoSoma * 10;
    }

    printf("> VERIFICAR SOMA: %d\n", resultadoSoma);
    itoa(resultadoSoma, ch, 10);
    buffer[2] = ch[0];
    buffer[3] = ch[1];
    return 0;
}
//--------------------------------------------------------------------------