#include "resource.h"
#include "task.h"
#include "config.h"
#include "task_set.h"
#include "ucontext.h"
#include "typedefine.h"
#include "stdlib.h"
#include "stdio.h"
#define SIZE 64000

typedef struct
{
    ucontext_t *task;
    int res[RESOURCES_COUNT];
    int id;
    int static_pri;
    entry_point_type entry;
    int current_pri;
    task_state_type state;
    int time;
} node;

node ready_queue[TASKS_COUNT];
ucontext_t schedule;
node running_task;
node temp;
int time_count;
int ready_num = 0;

void preempt(void);
void scheduler(void);

int main()
{
    //intiailize auto start tasks
    time_count=0;
    for(int i=0; i<AUTO_START_TASKS_COUNT; i++)
    {
        ucontext_t *new;
        new=malloc(sizeof(ucontext_t));

        char tstack[SIZE];
        getcontext(new);

        new->uc_stack.ss_sp=tstack;
        new->uc_stack.ss_size=sizeof(tstack);
        new->uc_stack.ss_flags=0;
        new->uc_link=&schedule;

        for(int j=0; j<RESOURCES_COUNT; j++)
        {
            ready_queue[i].res[j]=0;
        }
        ready_queue[i].entry=task_const[auto_start_tasks_list[i]].entry;
        ready_queue[i].id=task_const[auto_start_tasks_list[i]].id;
        ready_queue[i].static_pri=task_const[auto_start_tasks_list[i]].static_priority;
        ready_queue[i].current_pri=ready_queue[i].static_pri;
        ready_queue[i].state=READY;
        ready_queue[i].time=time_count;

        makecontext(new,ready_queue[i].entry,0);
        ready_queue[i].task=new;
    }
    ready_num=AUTO_START_TASKS_COUNT;
    //initialize the context of schedule
    getcontext(&schedule);
    schedule.uc_link=0;
    schedule.uc_stack.ss_sp=malloc(SIZE);
    schedule.uc_stack.ss_size=SIZE;
    getcontext(&schedule);
    //reschedule and run the highest priority task
    preempt();
    swapcontext(&schedule,running_task.task);
    scheduler();
    return 0;
}

void scheduler()   //when the running task is suspended
{
    //find the one have highest priority
    int max=0;
    for(int i=0; i<ready_num; i++)
    {
        if(ready_queue[i].current_pri>ready_queue[max].current_pri)
        {
            max=i;
        }
        else if(ready_queue[i].current_pri==ready_queue[max].current_pri)
        {
            if(ready_queue[i].time<=ready_queue[max].time)
            {
                max=i;
            }
        }
    }
    //set the index of the running task
    running_task.task=ready_queue[max].task;
    for(int j=0; j<RESOURCES_COUNT; j++)
    {
        running_task.res[j]=ready_queue[max].res[j];
    }
    running_task.entry= ready_queue[max].entry;
    running_task.id=ready_queue[max].id;
    running_task.static_pri=ready_queue[max].static_pri;
    running_task.current_pri=ready_queue[max].current_pri;
    running_task.time=ready_queue[max].time;
    running_task.state=RUNNING;
    //move the tasks behind front for 1
    for(int k=max; k<ready_num; k++)
    {
        ready_queue[k].task=ready_queue[k+1].task;
        for(int j=0; j<RESOURCES_COUNT; j++)
        {
            ready_queue[k].res[j]=ready_queue[k+1].res[j];
        }
        ready_queue[k].entry=ready_queue[k+1].entry;
        ready_queue[k].id=ready_queue[k+1].id;
        ready_queue[k].static_pri=ready_queue[k+1].static_pri;
        ready_queue[k].current_pri=ready_queue[k+1].current_pri;
        ready_queue[k].time=ready_queue[k+1].time;
        ready_queue[k].state=READY;
    }
    //decrease the number of tasks in ready queue
    ready_num--;
    return;
}

void preempt()      //when there's another task with higher priority
{
    if(running_task.state == SUSPENDED)
    {
        scheduler();
        return;
    }
    else
    {
        int max=0;
        //find the highest priority task
        for(int i=0; i<ready_num; i++)
        {
            if(ready_queue[i].current_pri>ready_queue[max].current_pri)
            {
                max=i;
            }
            else if(ready_queue[i].current_pri==ready_queue[max].current_pri)
            {
                if(ready_queue[i].time<ready_queue[max].time)
                {
                    max=i;
                }
            }
        }
        //compare the task have highest priority in ready queue and the running one
        if(running_task.current_pri>ready_queue[max].current_pri)
        {
            return;
        }
        //if have same priority compare the time when it was put in the queue
        else if(running_task.current_pri==ready_queue[max].current_pri)
        {
            if(running_task.time<=ready_queue[max].time)
            {
                return;
            }
        }
        //if the one in ready queue should execute first, move it to running task, and put running one back ready queue
        temp.task=running_task.task;
        for(int j=0; j<RESOURCES_COUNT; j++)
        {
            temp.res[j]=running_task.res[j];
        }
        temp.entry=running_task.entry;
        temp.id=running_task.id;
        temp.static_pri=running_task.static_pri;
        temp.current_pri=running_task.current_pri;
        temp.time=running_task.time;
        temp.state=READY;

        running_task.task=ready_queue[max].task;
        for(int j=0; j<RESOURCES_COUNT; j++)
        {
            running_task.res[j]=ready_queue[max].res[j];
        }
        running_task.entry= ready_queue[max].entry;
        running_task.id=ready_queue[max].id;
        running_task.static_pri=ready_queue[max].static_pri;
        running_task.current_pri=ready_queue[max].current_pri;
        running_task.time=ready_queue[max].time;
        running_task.state=RUNNING;

        ready_num--;

        for(int k=max; k<ready_num; k++)
        {
            ready_queue[k].task=ready_queue[k+1].task;
            for(int j=0; j<RESOURCES_COUNT; j++)
            {
                ready_queue[k].res[j]=ready_queue[k+1].res[j];
            }
            ready_queue[k].entry=ready_queue[k+1].entry;
            ready_queue[k].id=ready_queue[k+1].id;;
            ready_queue[k].static_pri=ready_queue[k+1].static_pri;
            ready_queue[k].current_pri=ready_queue[k+1].current_pri;
            ready_queue[k].time=ready_queue[k+1].time;
            ready_queue[k].state=READY;
        }

        ready_queue[ready_num].task=temp.task;

        for(int j=0; j<RESOURCES_COUNT; j++)
        {
            ready_queue[ready_num].res[j]=temp.res[j];
        }
        ready_queue[ready_num].entry=temp.entry;
        ready_queue[ready_num].id=temp.id;
        ready_queue[ready_num].static_pri=temp.static_pri;
        ready_queue[ready_num].current_pri=temp.current_pri;
        ready_queue[ready_num].time=temp.time;
        ready_queue[ready_num].state=READY;

        ready_num++;
        return;
    }
}
