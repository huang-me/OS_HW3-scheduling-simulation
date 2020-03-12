#include "resource.h"
#include "task.h"
#include "ucontext.h"
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
extern int ready_num;

extern void preempt();

status_type get_resource(resource_type id)
{
    status_type result = STATUS_OK;
    //don't allow the task get resource twice or more
    if(running_task.res[id]==1)
    {
        result=STATUS_ERROR;
        return result;
    }
    //test if anyone else have the resource
    for(int i=0; i<ready_num; i++)
    {
        if(ready_queue[i].res[id]==1)
        {
            result=STATUS_ERROR;
            return result;
        }
    }
    //give the task resource
    running_task.res[id]=1;
    //give the task same priority as the resource
    if(running_task.current_pri<resources_priority[id])
    {
        running_task.current_pri=resources_priority[id];
    }
    return result;
}

status_type release_resource(resource_type id)
{
    status_type result = STATUS_OK;
    //don't allow release the resource the task don't have
    if(running_task.res[id]==0)
    {
        result=STATUS_ERROR;
        return result;
    }
    else
    {
        //withdraw the resource
        running_task.res[id]=0;
        //make the priority of the task same as other resouces it owned or back to task's priority
        task_priority_type max=running_task.static_pri;
        for(int i=0; i<RESOURCES_COUNT; i++)
        {
            if(running_task.res[i]==1)
            {
                if(max<resources_priority[i])
                {
                    max=resources_priority[i];
                }
            }
        }
        running_task.current_pri=max;
        //get what task should execute and store the position
        getcontext(running_task.task);
        task_type id_before=running_task.id;
        preempt();
        if(id_before!=running_task.id)
        {
            setcontext(running_task.task);
        }
    }
    return result;
}
