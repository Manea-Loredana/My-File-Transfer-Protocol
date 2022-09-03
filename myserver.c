#include	<stdio.h>
#include	<stdlib.h>
#include	<ctype.h>
#include	<errno.h>
#include	<fcntl.h>
#include	<time.h>
#include	<netinet/tcp.h>
#include	<sys/stat.h>
#include	<sys/types.h>
#include 	<dirent.h>
#include	<netinet/in.h>
#include	<stdbool.h>
#include	<netdb.h>
#include	<time.h>
#include	<sys/types.h>
#include	<sys/socket.h>
#include	<strings.h>
#include	<string.h>
#include	<arpa/inet.h>
#include	<unistd.h>

#define 	MAXIM 	4096
#define		LISQ		1024
#define		TRUE		1
#define		FALSE		0

void taie(char *str)
{int i;
 int b=0;
 int e=strlen(str)-1;
while(isspace((unsigned char) str[b]))
        b++;
while((e>=b)&&isspace((unsigned char) str[e]))
        e--;
    for (i=b;i<=e;i++)
        str[i-b]=str[i];
    str[i-b] = '\0'; 
}

int c_ip_port(char *str,char *c_ip,int *c_port){
	char *a1,*a2,*a3,*a4,*a5,*a6;
	int b5, b6;
        strtok(str," ");
	a1=strtok(NULL,",");
	a2=strtok(NULL,",");
	a3=strtok(NULL,",");
	a4=strtok(NULL,",");
	a5=strtok(NULL,",");
	a6=strtok(NULL,",");
      sprintf(c_ip,"%s.%s.%s.%s",a1,a2,a3,a4);
        b5 = atoi(a5);
	b6 = atoi(a6);
	*c_port = (256*b5)+b6;
    printf("c_ip:%s c_port: %d\n",c_ip,*c_port);
	return 1;
}

int setup_connection(int *fd,char *c_ip,int c_port,int s_port){
      struct sockaddr_in cliaddr,serveradrss;
       if ((*fd=socket(AF_INET,SOCK_STREAM,0))<0){
    	perror("Eroare");
    	return -1;}
	bzero(&serveradrss, sizeof(serveradrss));
    serveradrss.sin_family=AF_INET;
   serveradrss.sin_addr.s_addr=htonl(INADDR_ANY);
    serveradrss.sin_port=htons(s_port-1);
while((bind(*fd,(struct sockaddr*)&serveradrss,sizeof(serveradrss)))<0){
    	s_port--;
    	serveradrss.sin_port= htons(s_port);}          
    bzero(&cliaddr, sizeof(cliaddr));
    cliaddr.sin_family=AF_INET;
    cliaddr.sin_port=htons(c_port);
 if (inet_pton(AF_INET,c_ip,&cliaddr.sin_addr)<=0){
    perror("Eroare");
    return -1;}
 if (connect(*fd,(struct sockaddr *)&cliaddr,sizeof(cliaddr))<0){
    perror("Eroare la conectare");
    return -1;}
return 1;}

int g_fname(char *it,char *fileptr){
    char *filename = NULL;
      filename = strtok(it, " ");
      filename = strtok(NULL, " ");
      if(filename == NULL){
        return -1;
            }else{
    strncpy(fileptr,filename,strlen(filename));
        return 1; }
                                 }
int gcom(char *command){
	  char cpy[1024];
	strcpy(cpy,command);
	   char *str=strtok(cpy," ");
	    int val;
    if(strcmp(str,"LIST")==0){val=1;}
    else if(strcmp(str,"RETR")==0){val=2;}
    else if(strcmp(str,"STOR")==0){val=3;}
    else if(strcmp(str,"SKIP")==0){val=4;}
    else if(strcmp(str,"ABOR")==0){val=5;}
   return val;}
int afis_list(int ctrfd,int datafd,char *it){
	char filelist[1024],sendline[MAXIM+1],str[MAXIM+1];
	bzero(filelist, (int)sizeof(filelist));
	if(g_fname(it, filelist) > 0){
		printf("Filelist Detected\n");
		sprintf(str, "ls %s",filelist);
		printf("Filelist: %s\n",filelist);
		taie(filelist);
    	DIR *dir = opendir(filelist);
    	if(!dir){
    		sprintf(sendline, "Nu exista\n");
    		write(ctrfd, sendline, strlen(sendline));
    		return -1;}
       else{closedir(dir);}
	}else{sprintf(str, "ls");}
    FILE *fls;
    extern FILE *popen();
if(!(fls=popen(str,"r"))) {
    	sprintf(sendline, "Eroare");
    	write(ctrfd, sendline, strlen(sendline));
        return -1;}
   while (fgets(sendline, MAXIM,fls) != NULL) {
        write(datafd, sendline, strlen(sendline));
        printf("%s", sendline);
        bzero(sendline, (int)sizeof(sendline)); }
    sprintf(sendline, "Comanda acceptata!");
    write(ctrfd,sendline,strlen(sendline));
    pclose(fls);
return 1;}

int makeretr(int ctrfd, int datafd, char *it){
	char filename[1024], sendline[MAXIM+1], str[MAXIM+1];
	bzero(filename, (int)sizeof(filename));
	bzero(sendline, (int)sizeof(sendline));
	bzero(str, (int)sizeof(str));
	if(g_fname(it, filename) > 0){
		sprintf(str, "cat %s", filename);
         if((access(filename, F_OK)) != 0){
			sprintf(sendline, "Nu exista\n");
    		write(ctrfd, sendline, strlen(sendline));
    		return -1;}}
            else{
		printf("Nedetectat\n");
		sprintf(sendline, "Fa ceva.\nNedetectat\n");
    	write(ctrfd,sendline,strlen(sendline));
		return -1;}
	FILE *in;
    extern FILE *popen();
    if (!(in = popen(str, "r"))) {
    	sprintf(sendline, "Eroare\n");
    	write(ctrfd,sendline,strlen(sendline));
        return -1;}
while (fgets(sendline, MAXIM, in) != NULL) {
        write(datafd, sendline, strlen(sendline));
          bzero(sendline, (int)sizeof(sendline)); }
    sprintf(sendline, "Comanda acceptata!");
    write(ctrfd, sendline, strlen(sendline));
    pclose(in);
    return 1;}

int makestor(int ctrfd, int datafd, char *it){
	char filename[1024], sendline[MAXIM+1], recvline[MAXIM+1], str[MAXIM+1], template[1024];
	bzero(filename,(int)sizeof(filename));
	bzero(sendline,(int)sizeof(sendline));
	bzero(recvline,(int)sizeof(recvline));
	bzero(str, (int)sizeof(str));
	int n = 0, p = 0;
	if(g_fname(it, filename) > 0){
		sprintf(str, "%s-out", filename);
	}else
{printf("Filename Not Detected\n");
 sprintf(sendline, "450 Requested file action not taken.\n");
    	write(ctrfd, sendline, strlen(sendline));
		return -1;}
sprintf(template,"%s-out",filename);
	FILE *fp;
    if((fp = fopen(template,"w"))==NULL){
        return -1;}
 while((n = read(datafd,recvline,MAXIM))> 0){
        fseek(fp, p, SEEK_SET);
        fwrite(recvline,1,n, fp);
        p =p +n;
        bzero(recvline,(int)sizeof(recvline)); }
   sprintf(sendline,"Comanda acceptata!");
    write(ctrfd,sendline,strlen(sendline));
    fclose(fp);
    return 1;}

int main(int argc,char **argv){
	int listenfd, connfd, port;
	struct sockaddr_in servaddr;
	pid_t pid;
	if(argc != 2){
		printf("Invalid Number of Arguments...\n");
		printf("Usage: ./ftpserver <listen-port>\n");
		exit(-1);}
	
	sscanf(argv[1], "%d", &port);
	listenfd = socket(AF_INET, SOCK_STREAM, 0);
	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family      = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port        = htons(port);
	bind(listenfd, (struct sockaddr*) &servaddr, sizeof(servaddr));
	listen(listenfd, LISQ);
	while(1){
		connfd = accept(listenfd, (struct sockaddr*)NULL, NULL);
		printf("Client nou.....\n");
		//procesul copil-----------------------------------------------
		if((pid = fork()) == 0){
			close(listenfd);
			int datafd, code, x = 0, c_port = 0;
			char recvline[MAXIM+1];
			char c_ip[50], command[1024];
while(1){
				bzero(recvline, (int)sizeof(recvline));
				bzero(command, (int)sizeof(command));
//ia datele de conctare a portului client
    			if((x = read(connfd, recvline, MAXIM)) < 0){
    				break;}
    			printf("**********\n%s \n", recvline);
                if(strcmp(recvline,"QUIT")==0){
                    printf("Finish...\n");
                    char bye[1024];
                    sprintf(bye,"Proces terminat");
                    write(connfd,bye,strlen(bye));
                    close(connfd);
                    break;   }
     c_ip_port(recvline,c_ip,&c_port);
if((setup_connection(&datafd,c_ip, c_port,port))< 0){
    				break;}
if((x = read(connfd, command,MAXIM)) < 0){
    				break;}
printf("-----------------\n%s \n", command);
code = gcom(command);
    if(code == 1){
    afis_list(connfd, datafd, command);
    			}else if(code == 2){
    				makeretr(connfd, datafd, command);
    			}else if(code == 3){
    				makestor(connfd, datafd, command);
    			}else if(code == 4){
                    char reply[1024];
                    sprintf(reply, "NU exista");
                    write(connfd, reply, strlen(reply));
                    close(datafd);
                    continue; }              
    			close(datafd);}
    		printf("Iesit din procesul copil\n");
    		close(connfd);
    		_exit(1);}
		//sfrasit proces copil-------------------------------
		close(connfd);}}
