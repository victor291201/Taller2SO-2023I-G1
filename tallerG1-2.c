#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <wait.h>
#include <string.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <time.h>

int main( int argc, char **argv ){
    
    FILE *ptr_file; int c=0,n=0;char l;
    ptr_file = fopen("comands.txt","rt");
    if (ptr_file == NULL)
    {
        puts("Error: No se encontró el archivo.");

    }
    while((c = fgetc(ptr_file)) != EOF){
        if( c == '\n')
            n++;
        putchar(c);
    }
    n ++;
    printf ("\nNúmero de líneas del archivo: %d\n",n);
    fclose(ptr_file);

    //declarando los datos que utilizaremos para guardar los pids
    pid_t pidp = getpid();
    pid_t* pidsh = (pid_t*) malloc(sizeof(pid_t)*n);
    for(int i  = 0; i<n;i++){
        pidsh[i]=-1;
    }
    //declarando los bufers de lectura y escritura
   char Buffer_out[1024];
   char Buffer_in[1024];
    //creando los hijos 
    //abriendo el archivo de texto en una matriz 
    char **matrix = NULL;
    char* line =NULL;
    int ind=0;
    int max_line = 1024;
    int shm_id;
    char *memh[n];
    char *memdim;
    for(int i = 0;i<n;i++){
        shm_id = shmget(IPC_PRIVATE,max_line*sizeof(char), IPC_CREAT | S_IRUSR | S_IWUSR);
        char *memdim = shmat(shm_id, 0, 0);
        memh[i]= memdim;
    }
    line = (char*) malloc(max_line* sizeof(char));
    matrix = (char**) malloc(max_line* sizeof(char*));
    FILE* fp = fopen("comands.txt","r");
    if(fp == NULL){perror("error al abrir el archivo \n");exit(EXIT_FAILURE);}
    while (fgets(line, max_line*sizeof(char), fp) ){
        matrix[ind] = line;
        line = (char*) malloc(max_line * sizeof(char));
        ind++;
    }
    fclose(fp);
    
    int h;
    for(h=0;h<n;h++){
        pidsh[h]=fork();
        if(pidsh[h] == 0 ){
            break;
        }
    }
    int bol = 1;
    
        void * oldhandler;
        void sighandler( int sig ){
            printf("\n \n \n señal ctrl + c recibida \n  \n");
            bol = 0;
        }
    //creando la logica para cada proceso
    if(pidp == getpid()){
        oldhandler = signal( SIGINT, sighandler); 
        //cerrar pipes escritura
        //logica del padre
        while(bol){
            sleep(1);
            for(int i=0;i<n;i++){
                printf("la salida del hijo [%d] es \n %s \n",i,memh[i]);
            }
        }
        for(int i = 0;i<n;i++){
            kill(pidsh[i],SIGKILL);
        }
        for(int i=0;i<n;i++){
            wait(NULL);
        }
        //cerrando las pipes restantes
        //terminando el programa
        printf("terminando el padre con el boleano en  %d.... \n",bol);
        
    }else{
        int tub[2];
        pid_t child;
        if(pipe(tub) == -1){return -1;}
        child = fork();
        switch (child){
        case -1:
            return -1;
        case 0:
            while(1){
                close(tub[0]);
                dup2(tub[1],STDOUT_FILENO);
                execl("/bin/sh","sh","-c",matrix[h],NULL);
            }
        default:
            while (1)
            {
                read(tub[0], Buffer_in, sizeof(Buffer_in));
                strcpy(memh[h], Buffer_in);
            }
            
            break;
        }

    }
        if(signal(SIGINT, oldhandler) == SIG_ERR){
            perror("signal:");
            exit(EXIT_FAILURE);
        }
    return 0;
}