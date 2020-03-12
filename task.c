#include "task.h"
#include "ucontext.h"
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

extern node ready_queue[TASKS_COUNT];
extern ucontext_t schedule;
extern node running_task;
extern node temp;
extern int time_count;
extern int ready_num;

extern void preempt(void);
extern void scheduler(void);

status_type activate_task(task_type id)
{
    //increase the time_count when activate any task
    time_count++;
    status_type result = STATUS_OK;
    //don't allow activate task which has already been activated
    if(running_task.id==id)
    {
        result=STATUS_ERROR;
        return result;
    }
    for(int i=0; i<ready_num; i++)
    {
        if(ready_queue[i].id==id)
        {
            result=STATUS_ERROR;
            return result;
        }
    }
    //set the task's context
    ucontext_t *new;
    new=malloc(sizeof(ucontext_t));

    char tstack[SIZE];
    getcontext(new);
    new->uc_stack.ss_sp=tstack;
    new->uc_stack.ss_size=sizeof(tstack);
    new->uc_stack.ss_flags=0;
    new->uc_link=&schedule;
    makecontext(new,task_const[id].entry,0);
    //initialize the index of the new task
    ready_queue[ready_num].task=new;
    for(int j=0; j<RESOURCES_COUNT; j++)
    {
        ready_queue[ready_num].res[j]=0;
    }
    ready_queue[ready_num].entry=task_const[id].entry;
    ready_queue[ready_num].id=task_const[id].id;
    ready_queue[ready_num].static_pri=task_const[id].static_priority;
    ready_queue[ready_num].current_pri=ready_queue[ready_num].static_pri;
    ready_queue[ready_num].state=READY;
    ready_queue[ready_num].time=time_count;
    ready_num++;
    //get what task should execute and store the position
    getcontext(running_task.task);
    task_type id_before=running_task.id;
    preempt();
    if(id_before!=running_task.id)
    {
        setcontext(running_task.task);
    }
    return result;
}

status_type terminate_task(void)
{
    status_type result = STATUS_OK;
    //if the task still hold any resource return STATUS_ERROR from terminate_task
    for(int i=0; i<RESOURCES_COUNT; i++)
    {
        if(running_task.res[i]==1)
        {
            result=STATUS_ERROR;
            return result;
        }
    }
    //make the running task suspended
    running_task.state=SUSPENDED;
    //find the highest task from ready queue
    preempt();
    //run the highest task from its task memories
    setcontext(running_task.task);
    return result;
}
