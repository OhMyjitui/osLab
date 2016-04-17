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
struct{
    sem_t *mutex,*empty,*full;
    int file;
}share;
int buf_in; //记录缓冲输入的位置
int buf_out; //记录缓冲输出的位置
void producer(){
    int i;

    for(i = 0 ;i < M; i++){
        sem_wait(share.empty);
        sem_wait(share.mutex);
        lseek(share.file, 11*sizeof(int), SEEK_SET);
        read(share.file,&buf_in,sizeof(int));

        printf("produce a new item %d\n",i);
        lseek(share.file, (buf_in%NBUFF)*sizeof(int), SEEK_SET);
        write(share.file,&i,sizeof(int));
	    buf_in++;
        lseek(share.file, 11*sizeof(int), SEEK_SET);
        write(share.file,&buf_in,sizeof(int));
	    fflush(stdout);
        sem_post(share.mutex);
        sem_post(share.full);

    }


}

void consumer(){

    int num;
    sem_wait(share.full);
    sem_wait(share.mutex);


    lseek(share.file, 10*sizeof(int), SEEK_SET);
    read(share.file,&buf_out,sizeof(int));
    lseek(share.file, (buf_out%NBUFF)*sizeof(int), SEEK_SET);//将目的文件的读写指针移到起始位置
    read(share.file, &num, sizeof(int));
   printf("%d:\t%d\n", getpid(), num);
   fflush(stdout);
    buf_out = buf_out + 1;
    lseek(share.file, 10*sizeof(int), SEEK_SET);
    write(share.file,&buf_out,sizeof(int));
    sem_post(share.mutex);
    sem_post(share.empty);

    //printf("%d:  %d\n",getpid(),num);

}

int main(){

    sem_unlink("SEM_MUTEX");
    sem_unlink("SEM_EMPTY");
    sem_unlink("SEM_FULL");

    if((share.mutex = sem_open("SEM_MUTEX",O_CREAT,FILE_MODE,1)) == SEM_FAILED){
        perror("sem_open() error");
        exit(-1);
    }
    if((share.empty = sem_open("SEM_EMPTY",O_CREAT,FILE_MODE,NBUFF)) == SEM_FAILED){
        perror("sem_open() error");
        exit(-1);
    }
    if((share.full = sem_open("SEM_FULL",O_CREAT,FILE_MODE,0)) == SEM_FAILED){
        perror("sem_open() error");
        exit(-1);
    }



    share.file = open("buff.txt", O_CREAT|O_RDWR|O_TRUNC,0666);

     lseek(share.file, 10*sizeof(int), SEEK_SET);
     write(share.file,&buf_out,sizeof(int));
     lseek(share.file, 11*sizeof(int), SEEK_SET);
     write(share.file,&buf_in,sizeof(int));
// produce

     if(fork() == 0){
         int i;

         for(i = 0 ;i < M; i++){
             sem_wait(share.empty);
             sem_wait(share.mutex);
             lseek(share.file, 11*sizeof(int), SEEK_SET);
             read(share.file,&buf_in,sizeof(int));

             printf("produce a new item %d\n",i);
             lseek(share.file, (buf_in%NBUFF)*sizeof(int), SEEK_SET);
             write(share.file,&i,sizeof(int));
             buf_in++;
             lseek(share.file, 11*sizeof(int), SEEK_SET);
             write(share.file,&buf_in,sizeof(int));
             fflush(stdout);
             sem_post(share.mutex);
             sem_post(share.full);

         }
     }
     //consumer();
    for(int i = 0; i < CHILD; i++){
        if(fork() == 0 ){
            int num;
            sem_wait(share.full);
            sem_wait(share.mutex);


            lseek(share.file, 10*sizeof(int), SEEK_SET);
            read(share.file,&buf_out,sizeof(int));
            lseek(share.file, (buf_out%NBUFF)*sizeof(int), SEEK_SET);//将目的文件的读写指针移到起始位置
            read(share.file, &num, sizeof(int));
           printf("%d:\t%d\n", getpid(), num);
           fflush(stdout);
            buf_out++;
            lseek(share.file, 10*sizeof(int), SEEK_SET);
            write(share.file,&buf_out,sizeof(int));
            sem_post(share.mutex);
            sem_post(share.empty);
        }
    }



    wait(NULL);
    close(share.file);
    sem_unlink("SEM_MUTEX");
    sem_unlink("SEM_EMPTY");
    sem_unlink("SEM_FULL");
    return 0;

}
