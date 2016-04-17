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

    int buf_in;
    int buf_out;
}share;

void producer(){
    int i;

    for(i = 0 ;i < M; i++){
        sem_wait(share.mutex);
        sem_wait(share.empty);

        printf("produce a new item %d\n",i);
        lseek(share.file, (share.buf_in%NBUFF)*sizeof(int), SEEK_SET);
        write(share.file,&i,sizeof(int));
	    share.buf_in++;
	 fflush(stdout);

        sem_post(share.full);
        sem_post(share.mutex);
    }


}

void consumer(){

    int num;

    sem_wait(share.mutex);
    sem_wait(share.full);

    lseek(share.file, (share.buf_out%NBUFF)*sizeof(int), SEEK_SET);//将目的文件的读写指针移到起始位置
    read(share.file, &num, sizeof(int));
   printf("%d:\t%d\n", getpid(), num);
   fflush(stdout);
    share.buf_out = share.buf_out + 1;

    sem_post(share.empty);
    sem_post(share.mutex);
    //printf("%d:  %d\n",getpid(),num);

}

int main(){

    sem_unlink("SEM_MUTEX");
    sem_unlink("SEM_EMPTY");
    sem_unlink("SEM_FULL");
    share.buf_in = 0;
    share.buf_out = 0;
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



    for(int i = 0; i < CHILD; i++){
        if(fork() == 0 ){
            consumer();
        }
    }
    if(fork() == 0){
	producer();
    }


    wait(NULL);
    close(share.file);
    sem_unlink("SEM_MUTEX");
    sem_unlink("SEM_EMPTY");
    sem_unlink("SEM_FULL");
    return 0;

}
