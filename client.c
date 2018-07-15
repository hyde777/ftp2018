#include	<sys/socket.h>
#include	<netinet/in.h>
#include	<arpa/inet.h>
#include	<sys/types.h>
#include	<sys/wait.h>
#include	<signal.h>
#include 	<stdio.h>
#include 	<stdlib.h>
#include 	<errno.h>
#include 	<string.h>
#include	<unistd.h>
#include	<fcntl.h>

#define	max(a,b)	((a) > (b) ? (a) : (b))
#define MAXCHAR	256

void str_cli(int, int);

int main(int argc, char **argv)
{
	int	clientSocket;
	struct sockaddr_in serverAdresse;
	int fd;

	if (argc != 3)
	{
		printf("usage: client <IPaddress> <Port>\n");
		exit(-1);
	}

	clientSocket = socket(AF_INET, SOCK_STREAM, 0);
	
	bzero(&serverAdresse, sizeof(serverAdresse));

	serverAdresse.sin_family = AF_INET;
	serverAdresse.sin_port = htons(atoi(argv[2]));
	inet_pton(AF_INET, argv[1], &serverAdresse.sin_addr);
	
	connect(clientSocket, (struct sockaddr*) &serverAdresse, sizeof(serverAdresse));

	/**if(send(clientSocket, saisieUtilisateur, strlen(saisieUtilisateur), 0) < 0)
	{
		perror("send()");
		exit(-1);
	}**/

	fd = open("monTerminal.txt", O_RDONLY);
	if (fd < 0)
	{
		printf("Erreur d'ouverture du fichier %s\n", argv[3]);
		exit(-1);
	}
	else fd = STDIN_FILENO;
	str_cli(fd, clientSocket);

	exit(0);
}

void str_cli(int fd, int clientSocket)
{
	int maxfdp1, stdineof = 0;
	fd_set rset;
	char sendline[MAXCHAR], recvline[MAXCHAR];
	int n;

	FD_ZERO(&rset);
	for ( ; ; )
	{
		if (stdineof == 0)
		{
			FD_SET(fd, &rset); 
			maxfdp1 = max(fd, clientSocket) + 1;
		}
		else maxfdp1 = clientSocket + 1;

		FD_SET(clientSocket, &rset);
		select(maxfdp1, &rset, NULL, NULL, NULL);

		if (FD_ISSET(clientSocket, &rset))
		{	
			n = read(clientSocket, recvline, MAXCHAR);
			if (n == 0) 
			{
				if (stdineof == 1)
					return;
				else
				{
					printf("str_cli: serveur termine prematurement\n");
					exit(-1);
				}
			}
			else if (n < 0)
			{
				printf("Erreur de fichier (file)\n");
				exit(-1);
			}
			
			recvline[n] = '\0';
			
			if(strcmp(recvline, "Disconnect") == 0){
				printf("Vous allez etre deconnecte. Aurevoir.");
				shutdown(clientSocket, SHUT_WR);
				continue;
			}
			if(strcmp(recvline, "Retry") == 0){
				printf("Veuillez reessayer svp.");
				
			}
			write(fd, recvline, n);
		}

		if (FD_ISSET(fd, &rset))
		{
			n = read(fd, sendline, MAXCHAR);
			if (n == 0)
			{
				shutdown(clientSocket, SHUT_WR);
				FD_CLR(fd, &rset);
				continue;
			}else if (n < 0)
			{
				printf("Erreur de fichier (socket)\n");
				exit(-1);
			}

			sendline[n] = '\0';
			//printf("je passe ici\n");
			//printf("=[%s]\n", sendline);
			write(clientSocket, sendline, n);
			//sleep(3);
		}
	}
}