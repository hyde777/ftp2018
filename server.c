#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>
#include <sys/wait.h>

#define MAXCHAR	256

int totalDonnees = 0, n = 0, retry = 0;
int/*bool*/ bonjour = 0, bUsername = 0, bPassword = 0; 
char username[MAXCHAR], password[MAXCHAR];

void str_echo(int);

int main(int argc, char **argv)
{
    int serverSocket, codefd;
    pid_t childPID;
    socklen_t lenClientAdresse;
    struct sockaddr_in clientAdresse, serverAdresse;
    struct sigaction act;

    if (argc != 2)
    {
        printf("<serveur Port>\n");
        exit(-1);
    }

    act.sa_handler = SIG_DFL;
    act.sa_flags = SA_NOCLDWAIT;
    sigaction(SIGCHLD, &act, NULL);

    serverSocket = socket(AF_INET, SOCK_STREAM, 0);

    bzero(&serverAdresse, sizeof(serverAdresse));

    serverAdresse.sin_family = AF_INET;
    serverAdresse.sin_addr.s_addr = htonl(INADDR_ANY);
    serverAdresse.sin_port = htons(atoi(argv[1]));

    bind(serverSocket, (struct sockaddr*) &serverAdresse, sizeof(serverAdresse));	
    perror("bind");

    listen(serverSocket, 10);

    for( ; ; )
    {
        
        lenClientAdresse = sizeof(clientAdresse);

        codefd = accept(serverSocket, (struct sockaddr*) &clientAdresse, &lenClientAdresse);
        if(codefd < 0)
        {
            printf("erreur accept\n");
            exit(-1);
        }

        childPID = fork();
        if(childPID == 0)
        {
            close(serverSocket);
            str_echo(codefd);
            exit(0);
        }
        close(codefd);
    }
    
    /**if((n = recv(serverSocket, saisieUtilisateur, sizeof saisieUtilisateur - 1, 0)) < 0)
    {
        perror("recv()");
        exit(-1);
    }

    saisieUtilisateur[n] = '\0';
    printf("%s\n", saisieUtilisateur);**/
    return 0;
}

void  str_echo(int sockfd)
{
	ssize_t n;
	char buf[MAXCHAR];

	while((n = read(sockfd, buf, MAXCHAR)) > 0)
	{
		totalDonnees += n;

        buf[n - 1] = '\0';


        if(strcmp(buf, "Bonjour") == 0)
        {
            //printf("Who ? %s\n ", buf);
            strcpy(buf, "Qui etes vous ?\n");
            //buf[] = "WHO";
            //buf[n - 1] = '\0';
            
            //printf("Who 2 ? %s\n ", buf);
            bonjour = 1;
            write(sockfd, buf, strlen(buf));
            continue;
        }
        
        if(bonjour == 1){
            if(strlen(username) == 0) {
                strcpy(username, buf);
                bUsername = checkUsername(username);
                if(bUsername == 0) 
                    strcpy(buf, "Disconnect");
                else
                    strcpy(buf, "Ton mot de passe ?");

            }
            else {
                strcpy(password, buf);
                printf("password : %s\n", buf);
                bPassword = checkPassword(password);
                if(bPassword == 0) {
                    if(retry >= 2){
                        strcpy(buf, "Disconnect"); 
                    }else{
                        strcpy(buf, "Retry\n");
                        retry = retry + 1;
                    }
                }
            }

            if(bUsername == 1 && bPassword == 2){
                bonjour = 0;
                strcpy(buf, "Welcome !\n");
            }
        }
        write(sockfd, buf, strlen(buf));
		
	}
	printf("Total données reçues: %d\n", totalDonnees);
}

int checkUsername(char str[])
{
    FILE* fichier = NULL;
    char name[MAXCHAR], pwd[MAXCHAR], buff[MAXCHAR];
 
    fichier = fopen("informations.csv", "r");
 
    if (fichier != NULL)
    {
        while (fscanf(fichier, "%s %s\n", name, pwd) > 0)
        {
            printf("username : %s %s\n", name, pwd);

            if(strcmp(name, str) == 0) return 1;
        }
        fclose(fichier);
    }
    return 0;
}

int checkPassword(char str[])
{
    FILE* fichier = NULL;
    char name[MAXCHAR], pwd[MAXCHAR];
 
    fichier = fopen("informations.csv", "r");
 
    if (fichier != NULL)
    {
        while (fscanf(fichier, "%s %s\n", name, pwd) > 0)
        {
            printf("username : %s %s\n", name, pwd);
            if(strcmp(pwd, str) == 0) return 2;
        }
        fclose(fichier);
    }
    return 0;
}