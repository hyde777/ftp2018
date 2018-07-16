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
int/*bool*/ isBonjour = 0, isUsername = 0, isPassword = 0; 
char username[MAXCHAR], password[MAXCHAR], absolutPath[1024];

void str_echo(int);
char* executeCommande(char[]);
int checkUsername(char[]);
int checkPassword(char[]);
int createFileWithData(char[], char[]);
char* getDataInFile(char[]);

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
    getcwd(absolutPath, sizeof(absolutPath));
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
    return 0;
}

void  str_echo(int sockfd)
{
	ssize_t n;
	char buf[MAXCHAR], folderfile[MAXCHAR], concatenation[MAXCHAR];

	while((n = read(sockfd, buf, MAXCHAR)) > 0)
	{
		totalDonnees += n;

        buf[n - 1] = '\0';
        if(strcmp(buf, "Bonjour") == 0 || strcmp(buf, "BONJ") == 0)
        {
            strcpy(buf, "Qui etes vous ?\n>");
            isBonjour = 1;
            write(sockfd, buf, strlen(buf));
            continue;
        }else if(strcmp(buf, "rls") == 0){
            strcpy(buf, executeCommande("ls"));
        }else if(strcmp(buf, "rpwd") == 0){
            strcpy(buf, executeCommande("pwd"));
		}else if(strcmp(buf, "rcd") == 0){
			printf("Se déplacer dans quel repertoire ?\n>");
			scanf ("%s", folderfile);
			strcat(absolutPath, "/");
			strcpy(buf, strcat(absolutPath, folderfile));
			chdir(buf);
			printf("Vous avez ete deplace dans le repertoire %s.\n", folderfile);
		}else if(strcmp(buf, "upld") == 0){ //get le path du client
            char pathFile[MAXCHAR], data[MAXCHAR*4];
            printf("Upload quel fichier ?\n>");
			scanf ("%s", folderfile);
            strcat(pathFile, absolutPath);
            strcat(pathFile, folderfile);
            strcpy(data, getDataInFile(pathFile));
            createFileWithData(pathFile, data);
        }else if(strcmp(buf, "downl") == 0){ //get le path du client
            char pathFile[MAXCHAR], data[MAXCHAR*4];
            printf("Download quel fichier ?\n>");
			scanf ("%s", folderfile);
            strcat(pathFile, absolutPath);
            strcat(pathFile, folderfile);
            strcpy(data, getDataInFile(pathFile));
            createFileWithData(pathFile, data);
        }

        
        if(isBonjour == 1){
            if(strlen(username) == 0) {
                strcpy(username, buf);
                isUsername = checkUsername(username);
                if(isUsername == 0) 
                    strcpy(buf, "Disconnect");
                else
                    strcpy(buf, "Ton mot de passe ?\n>");

            }
            else {
                strcpy(password, buf);
                isPassword = checkPassword(password);
                if(isPassword == 0) {
                    if(retry >= 2){
                        strcpy(buf, "Disconnect"); 
                    }else{
                        strcpy(buf, "Mot de passe incorrect, reessayer.\n>");
                        retry = retry + 1;
                    }
                }
            }

            if(isUsername == 1 && isPassword == 1){
                isBonjour = 0;
                strcpy(buf, "Welcome !\n>");
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
            if(strcmp(pwd, str) == 0) return 1;
        }
        fclose(fichier);
    }
    return 0;
}

char* executeCommande(char command[])
{
    char* result = malloc(sizeof(char) * 1024);
	char tampon[MAXCHAR];
	FILE *sortie;
	sortie = popen (command, "r");
	if (sortie == NULL) fprintf (stderr, "erreur");
	while (fgets (tampon, sizeof tampon, sortie) != NULL)
    {
        strcat(result, tampon);
	}
	fclose (sortie);
    printf("=> %s", result);
    return result;
}

int createFileWithData(char strFile[], char strData[])
{
	char tampon[MAXCHAR];
	FILE *file;
	file = popen (strFile, "r+");
	if (file == NULL) return 1;
    fputs (strData, file);
	fclose (file);
    return 0;
}
char* getDataInFile(char data[]){

}