#include<iostream>
#include<unistd.h>

using namespace std;


#define MAX_THREADS 5
#define NUM_OF_THREADS 6
#define MAX_PRIORITY 7

struct sim_thread {
    int id;
    int time_remaining;
    int priority;
    //Sched 1 is sched_RR, sched 0 is sched_FIFO
    int sched;
} ;

int new_threads[NUM_OF_THREADS][5] = 
{
    //moment of creation, id, time_remain, priority
    {1,3,5,3,1},
    {3,5,6,5,1},
	{7,2,3,5,0},
	{12,1,5,3,0},
	{20,6,3,6,0},
	{20,7,4,7,0}

};

//representing the queue of threads waiting to be executed, queue[0] is active
struct sim_thread* queue[MAX_THREADS];

//printing the state of the queue in moment time
void print(int t, int num_curr){
    cout << t << "  " ;
    for(int i=0; i< num_curr; i++ ){
        cout << queue[i]->id << "/" << queue[i]->priority << "/" << queue[i]->time_remaining << "   ";
    }
    for(int i=num_curr; i<MAX_THREADS; i++){
        cout << "-/-/-   ";
    }
    cout << endl;
}


int main(void){

    int num_current_threads = 0;
    int num_finished_threads = 0;
    int idx;

    int time = 0;
    struct sim_thread * created_thread;
    
    cout << "t    AKT   WAIT1   WAIT2   WAIT3   WAIT4" << endl;
    
    while(num_finished_threads < NUM_OF_THREADS){
        //if there are any threads in thw queue, time remaining for the currently executed thread is reduced by one
        // if the thread has finished (time remaining is zero) it is removed from the queue
        if(queue[0]!=nullptr){
            queue[0]->time_remaining--;
            if(queue[0]->time_remaining==0){

                printf( "Dretva %d zavrsila.\n", queue[0]->id );

                num_finished_threads++;
                num_current_threads--;
                free(queue[0]);

                //moving threads forward in the queue
                for(int i=0; i< (num_current_threads); i++){
                    queue[i]=queue[i+1];
                }
            }
            else{
                if( queue[0]->sched == 1){
                    //RR, move to the end of its priority
                    idx=0;
                    while(  (idx+1) < num_current_threads && queue[idx+1]->priority==queue[idx]->priority){
                        swap(queue[idx+1], queue[idx]);
                        idx++;
                    }

                }
            }
        }

        print(time, num_current_threads);

        //Checking if there are any new threads to be added to the queue
        //If the queue is full and new thread arrives, it cannot be stored in the queue and will be lost.
        for(int i=0; i< NUM_OF_THREADS; i++){
            if( new_threads[i][0]==time && (num_current_threads+1)<=MAX_THREADS){
                created_thread = new sim_thread;
                created_thread->id = new_threads[i][1];
                created_thread->time_remaining = new_threads[i][2];
                created_thread->priority = new_threads[i][3];
                created_thread->sched = new_threads[i][4];

                queue[num_current_threads] = created_thread;
                idx = num_current_threads;
                num_current_threads++;
            
                //Positioning the new thread to the last place in its priority queue
                while (idx >=1 && queue[idx]->priority > queue[idx-1]->priority){
                    swap(queue[idx], queue[idx-1]);
                    idx--;
                }
                //Informing the user about changes
                printf("%d  -- nova dretva id=%d trajanje=%d prioritet=%d\n", time, created_thread->id, created_thread->time_remaining, created_thread->priority);
                print(time, num_current_threads);

            }
            else if( (num_current_threads+1)>MAX_THREADS && new_threads[i][0]==time){
                cout << "WARNING : Congestion! Some threads may not have been added to the queue.\n This is illegal behaviour."<<endl;
            }

        }

        sleep(1);
        time++;
    }

    if(created_thread!=nullptr)
        free(created_thread);
    return 0;
}