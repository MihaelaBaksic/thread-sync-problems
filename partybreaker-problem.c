#include<stdio.h>
#include<pthread.h>
#include<unistd.h>
#include<stdlib.h>
#include<stdbool.h>

#define N 3

int num_stud_in_room;
int number_of_students;
bool partybreaker_in_room;
int sys_counter;
pthread_mutex_t monitor;
pthread_cond_t stud_queue;
pthread_cond_t breaker_queue;
pthread_cond_t breaker_leaving;



void *partybreaker( void *x){
    
    while(1){
        
        usleep( (rand()%901 + 100)*1000);

        //locking mutex to access shared memory
        pthread_mutex_lock(&monitor);
        //if condition for continuation is no longer valid, break and return
        if(sys_counter==N*number_of_students) break;
        //wait for number of students in the room to be 3 or more
        if(num_stud_in_room < 3){
            pthread_cond_wait(&breaker_queue, &monitor);
        }
        //checking if it was freed from the queue due to all students finishing and leaving the system
        if(sys_counter==N*number_of_students) break;

        //entering the room and unlocking the mutex so students can exit
        printf("Partibrejker je usao u sobu.\n");
        partybreaker_in_room=true;
        pthread_mutex_unlock(&monitor);

        //waiting for all students to exit
        pthread_mutex_lock(&monitor);
        while( num_stud_in_room!=0)
            pthread_cond_wait(&breaker_leaving, &monitor);
        //after all students have exited the room, partybreaker exits
        printf("Partibrejker izasao iz sobe.\n");
        partybreaker_in_room=false;
        //letting students know partybreaker has exited
        pthread_cond_broadcast(&stud_queue);
        pthread_mutex_unlock(&monitor);    
    }
    return NULL;
}

void *student( void *x){
    
    usleep( (rand()%401 + 100)*1000);

    for(int i=0; i<N; i++){

        //waiting until partybreaker is not in the room
        pthread_mutex_lock(&monitor);
        while(partybreaker_in_room)
            pthread_cond_wait(&stud_queue, &monitor);
        //entering the room
        printf("Student %d je usao u sobu.\n", (*((int*)x) + 1));
        num_stud_in_room++;
        //if number of students in the room is three or more, partybreaker can enter
        if(num_stud_in_room >= 3) pthread_cond_broadcast(&breaker_queue); 
        pthread_mutex_unlock(&monitor);

        usleep( 1000*(rand()%1001 + 1000));

        //leaving the room
        pthread_mutex_lock(&monitor);
        printf("Student %d izasao iz sobe.\n", (*((int*)x) +1));
        num_stud_in_room--;
        sys_counter++;
        //if all students have been to room 3 times, free partybreaker from waiting to enter
        if(sys_counter==N*number_of_students){
            pthread_cond_broadcast(&breaker_queue);
        }
        //letting partybreaker know he can check for leaving the room
        pthread_cond_broadcast(&breaker_leaving);
        pthread_mutex_unlock(&monitor);

        usleep(( rand()%1001 + 1000)*1000);
    }
    return NULL;
}

int main(int argc, char*argv[]){

    //getting total number of students
    number_of_students = atoi(argv[1]);
    if(number_of_students <=3 ) {
        printf("Number of students must be larger than 3.\n");
        exit(1);
    }
    sys_counter=0;

    //initially 0 students in the room
    //partybreaker not in the room
    num_stud_in_room=0;
    partybreaker_in_room=false;

    //setting random seed
    srand((unsigned )time(NULL));

    //defining mutex
    pthread_mutex_init(&monitor, NULL);
    pthread_cond_init(&stud_queue, NULL);
    pthread_cond_init(&breaker_queue, NULL);
    pthread_cond_init(&breaker_leaving, NULL);

    //creating threads;
    
    pthread_t student_threads[number_of_students];
    int students[number_of_students];
    for(int i=0; i< number_of_students; i++){
        students[i]=i;
        if( pthread_create(&student_threads[i], NULL, student, &students[i]) != 0){
            printf("Thread not created\n");
            exit(1);
        };
    }

    pthread_t partybreaker_thread;
    if(pthread_create(&partybreaker_thread, NULL, partybreaker, NULL) != 0){
        printf("Thread not created\n");
        exit(1);
    };

    //joining threads
    for(int i=0; i<number_of_students; i++){
        pthread_join(student_threads[i], NULL);
    }
    pthread_join(partybreaker_thread, NULL);

    //deleting mutex
    pthread_mutex_destroy(&monitor);
    pthread_cond_destroy(&stud_queue);
    pthread_cond_destroy(&breaker_queue);  
    pthread_cond_destroy(&breaker_leaving);

    return 0;
}