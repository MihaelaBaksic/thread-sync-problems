#include<stdio.h>
#include<pthread.h>
#include<unistd.h>
#include<stdlib.h>
#include<semaphore.h>
#include<sys/shm.h>
#include<sys/types.h>
#include<sys/ipc.h>
#include<sys/wait.h>

#define N 3

sem_t *semaphores;
sem_t *approach_table;
sem_t *table_empty;
int *table_items;

//generates two new different items
void getNewItems( int *it1, int *it2){
    *it1 = rand()%N;
    *it2 = rand()%N;
    while((*it1)==(*it2)){
        *it1 = rand()%N;
        *it2 = rand()%N;
    }
    return;    
}

//returns missing items for certain smoker
void getMissingItems( int *it1, int *it2, int smoker_number){
    switch (smoker_number)
    {
    case 0:
        *it1=1;
        *it2=2;
        break;
    case 1:
        *it1=0;
        *it2=2;
        break;
    case 2:
        *it1=1;
        *it2=0;
        break;
    default:
        break;
    }
    return;
}


void salesman(void){
    srand((unsigned) time(NULL));

    char items[3][6] = {"papir", "duhan", "sibice" };
    
    while(1){
        //waiting to be able to approach table
        sem_wait(approach_table);

        //putting new items on the table
        getNewItems(table_items, table_items+1);
        printf("\nTrgovac postavlja %s i %s\n", items[table_items[0]], items[table_items[1]]);

        //enabling approach to the table and letting smokers know items are at the table
        sem_post(approach_table);
        for(int i=0; i<N; i++){
            sem_post(semaphores+i);
        }
        //waiting for smokers to consume products
        sem_wait(table_empty);
    }
}

void smoker(int smoker_num){
    int missing_items[2];
    //defining missing items for this specific smoker
    getMissingItems(&missing_items[0], &missing_items[1], smoker_num);

    while(1){
        //waiting for items to be put on the table
        sem_wait(&semaphores[smoker_num]);
        //waiting for their turn to approach the table
        sem_wait(approach_table);

        //if items are the ones this smoker needs, take them, let salesman know the table is empty
        if( (missing_items[0]==table_items[0] && missing_items[1]==table_items[1]) ||
            (missing_items[0]==table_items[1] && missing_items[1]==table_items[0]) ){
                
                printf("Pusac %d uzima i ...\n", (smoker_num+1));
                sem_post(approach_table);
                sem_post(table_empty);
                sleep(3);
        }
        else{
            sem_post(approach_table);
        }
    }
}

int main(void){

    //shared memory allocation
    int id;
    id = shmget(IPC_PRIVATE, sizeof(sem_t)*5 + sizeof(int), 0600);
    semaphores = (sem_t*) shmat(id, NULL, 0);
    approach_table= semaphores + 3;
    table_empty = semaphores +4;
    table_items = (int*) (semaphores+5);
    shmctl(id, IPC_RMID, NULL);  



    //Defining posessions of smokers
    printf("Pušač 1 ima papir\n"); // index 0
    printf("Pušač 2 ima duhan\n"); // index 1
    printf("Pušač 3 ima šibice\n"); // index 2


    //semaphore initialization
    sem_init(approach_table, 1, 1);
    sem_init(table_empty, 1, 0);
    for(int i=0; i<N; i++){
        sem_init(&semaphores[i], 1, 0);
    }

    pid_t pid = fork();
    if(pid==0){
        salesman();
        exit(0);
    }else if(pid==-1){
        exit(1);
    }
    
    for(int i=0; i<N; i++){
        pid=fork();
        if(pid==0){
            smoker(i);
            exit(0);
        }
        else if(pid==-1) exit(1);
    }
    
    for(int i=0; i<(N+1); i++){
        (void) wait(NULL);
    }
 
    return 0;
}