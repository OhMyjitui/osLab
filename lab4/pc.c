#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <semaphore.h>
#include <errno.h>
#include <fcntl.h>

#define NBUFF 10
#define FILE_MODE  (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)
#define CHILD 2
#define M 500

sem_t *mutex,*empty,*full;
int file;

int buf_in; //记录缓冲输入的位置
int buf_out; //记录缓冲输出的位置
void producer(){
    int i;

    for(i = 0 ;i < M; i++){
        sem_wait( empty);
        sem_wait( mutex);
        lseek( file, 11*sizeof(int), SEEK_SET);
        read( file,&buf_in,sizeof(int));

        printf("produce a new item %d\n",i);
        lseek( file, (buf_in%NBUFF)*sizeof(int), SEEK_SET);
        write( file,&i,sizeof(int));
	    buf_in++;
        lseek( file, 11*sizeof(int), SEEK_SET);
        write( file,&buf_in,sizeof(int));
	    fflush(stdout);
        sem_post( mutex);
        sem_post( full);

    }


}

void consumer(){

    int num;
    sem_wait( full);
    sem_wait( mutex);


    lseek( file, 10*sizeof(int), SEEK_SET);
    read( file,&buf_out,sizeof(int));
    lseek( file, (buf_out%NBUFF)*sizeof(int), SEEK_SET);//将目的文件的读写指针移到起始位置
    read( file, &num, sizeof(int));
   printf("%d:\t%d\n", getpid(), num);
   fflush(stdout);
    buf_out = buf_out + 1;
    lseek( file, 10*sizeof(int), SEEK_SET);
    write( file,&buf_out,sizeof(int));
    sem_post( mutex);
    sem_post( empty);

    //printf("%d:  %d\n",getpid(),num);

}

int main(){

    sem_unlink("SEM_MUTEX");
    sem_unlink("SEM_EMPTY");
    sem_unlink("SEM_FULL");

    if(( mutex = sem_open("SEM_MUTEX",O_CREAT,FILE_MODE,1)) == SEM_FAILED){
        perror("sem_open() error");
        exit(-1);
    }
    if(( empty = sem_open("SEM_EMPTY",O_CREAT,FILE_MODE,NBUFF)) == SEM_FAILED){
        perror("sem_open() error");
        exit(-1);
    }
    if(( full = sem_open("SEM_FULL",O_CREAT,FILE_MODE,0)) == SEM_FAILED){
        perror("sem_open() error");
        exit(-1);
    }



     file = open("buff.txt", O_CREAT|O_RDWR|O_TRUNC,0666);

     lseek( file, 10*sizeof(int), SEEK_SET);
     write( file,&buf_out,sizeof(int));
     lseek( file, 11*sizeof(int), SEEK_SET);
     write( file,&buf_in,sizeof(int));
// produce

     if(fork() == 0){
         int i= 0;

         while(i < M){
             sem_wait( empty);
             sem_wait( mutex);
             lseek( file, 11*sizeof(int), SEEK_SET);
             read( file,&buf_in,sizeof(int));

             printf("produce a new item %d\n",i);
             lseek( file, (buf_in%NBUFF)*sizeof(int), SEEK_SET);
             write( file,&i,sizeof(int));
             buf_in++;
             lseek( file, 11*sizeof(int), SEEK_SET);
             write( file,&buf_in,sizeof(int));
             fflush(stdout);
             sem_post( mutex);
             sem_post( full);
             i++;
         }
     }
     //consumer();
    for(int i = 0; i < CHILD; i++){
        if(fork() == 0 ){
            for(int  k = 0; k < M/CHILD; k++ )
            {
                int num;
                sem_wait( full);
                sem_wait( mutex);


                lseek( file, 10*sizeof(int), SEEK_SET);
                read( file,&buf_out,sizeof(int));
                lseek(file, (buf_out%NBUFF)*sizeof(int), SEEK_SET);//将目的文件的读写指针移到起始位置
                read(file, &num, sizeof(int));
                printf("%d:\t%d\n", getpid(), num);
                fflush(stdout);
                buf_out++;
                lseek( file, 10*sizeof(int), SEEK_SET);
                write( file,&buf_out,sizeof(int));
                sem_post( mutex);
                sem_post( empty);
            }
        }
    }



    wait(NULL);
    close( file);
    sem_unlink("SEM_MUTEX");
    sem_unlink("SEM_EMPTY");
    sem_unlink("SEM_FULL");
    return 0;

}
