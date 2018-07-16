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

char absolutPath[1024];
int PORT;

void str_cli(int, int);
void executeCommande(char[]);

int main(int argc, char **argv)
{
	int	clientSocket, fd;
	struct sockaddr_in serverAdresse;

	if (argc != 3)
	{
		printf("Missing params : <ip_address> <port>\n");
		exit(-1);
	}

	clientSocket = socket(AF_INET, SOCK_STREAM, 0);
	
	bzero(&serverAdresse, sizeof(serverAdresse));
	PORT = htons(atoi(argv[2]));
	serverAdresse.sin_family = AF_INET;
	serverAdresse.sin_port = PORT;
	inet_pton(AF_INET, argv[1], &serverAdresse.sin_addr);
	
	if (connect(clientSocket, (struct sockaddr*) &serverAdresse, sizeof(serverAdresse)))
	{
	    printf("ERROR: Failed to connect to the host!\n");
		exit(-1);
	}
	else 
		printf("Connected to server at port %d...ok!\n", PORT);

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
	int maxfdp1, stdineof = 0, n = 0;
	fd_set rset;
	char sendline[MAXCHAR], recvline[MAXCHAR], folderfile[MAXCHAR], concatenation[MAXCHAR];
	getcwd(absolutPath, sizeof(absolutPath));

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
			if (strcmp(recvline, "Disconnect") == 0)
			{
				printf("Vous allez etre deconnecte. Bye.\n");
				sleep(3);
				shutdown(clientSocket, SHUT_WR);
				continue;
			} 
			else if (strcmp(recvline, "ls") == 0 || strcmp(recvline, "pwd") == 0)
			{
				executeCommande(recvline);
				strcpy(recvline, "\n>");
			} 
			else if (strcmp(recvline, "cd") == 0)
			{
				printf("Se déplacer dans quel repertoire ?\n>");
				scanf ("%s", folderfile);
				strcat(absolutPath, "/");
				strcpy(recvline, strcat(absolutPath, folderfile));
				chdir(recvline);
				printf("Vous avez ete deplace dans le repertoire %s.\n", folderfile);
				strcpy(recvline, "\n>");
			} 
			else if (strcmp(recvline, "rm") == 0)
			{
				printf("Supprimer quel fichier ?\n>");
				scanf ("%s", folderfile);
				strcpy(concatenation, "rm ");
				strcpy(recvline, strcat(concatenation, folderfile));
				executeCommande(recvline);
				printf("Le fichier %s a ete supprime.\n", folderfile);
				strcpy(recvline, "\n>");
			}
			write(fd, recvline, strlen(recvline));
		}

		if (FD_ISSET(fd, &rset)) 
		{
			n = read(fd, sendline, MAXCHAR);
			if (n == 0) 
			{
				shutdown(clientSocket, SHUT_WR);
				FD_CLR(fd, &rset);
				continue;
			} 
			else if (n < 0) 
			{
				printf("Erreur de fichier (socket)\n");
				exit(-1);
			}
			sendline[n] = '\0';
			write(clientSocket, sendline, n);
		}
	}
}

void executeCommande(char command[])
{
	char tampon[MAXCHAR];
	FILE *sortie;
	sortie = popen (command, "r");
	if (sortie == NULL) fprintf (stderr, "erreur");
	while (fgets (tampon, sizeof tampon, sortie) != NULL) 
	{
		fputs (tampon, stdout);
	}
	fclose (sortie);
}