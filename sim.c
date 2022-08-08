#include <stdio.h>
#include <stdlib.h>

enum event_type {
        TERMINATE,
        ARRIVAL,
        SCHEDULING, 
        END
};

struct event {
        int time;
        int type;
        void (*func)(struct event*);
        void* data;
        struct event* next;
};

struct event_list { 
        struct event* head;
};

struct task {
        int id;
        int wcet;
        int period;
        int deadline;
        int ret;
        int instance;
};

struct task_item {
        int prio;
        struct task* task;
        struct task_item* next;
};

struct task_list {
        struct task_item* head;
};

struct task_list task_list;
struct event_list list;

struct event* new_event(int time, int type, void (*func)(struct event*), void* data);
void insert_event(struct event_list* list, struct event* event);
void insert_task_item(struct task_list* list, struct task_item* task_item);
void remove_task_item(struct task_list* list, struct task_item* task_item);
void print_list(struct event_list* list);
void add_arrival(struct task* task, int time);
void set_simulation_end(int time);
void add_scheduling(int time);
void add_terminate(int time);
void schedule(struct event* e);

void terminate(struct event* e) {
        struct task_item *ti = task_list.head;
        // remove task item
        printf("%d\tT_%d_%d\tE\n", e->time, ti->task->id, ti->task->instance);
        remove_task_item(&task_list, ti);
        free(ti);
        // new scheduling event
        add_scheduling(e->time);
}

void schedule(struct event* e) {
        if (task_list.head) {
                struct task* t = task_list.head->task;
                printf("%d\tT_%d_%d\tS\n", e->time, t->id, t->instance);
                if (e->time + t->ret <= e->next->time)
                        add_terminate(e->time + t->ret);
                t->ret = t->ret - (e->next->time - e->time);
        } 
}

int last_task_arrival = -1;

void task_arrival(struct event* e) {
        struct task *t = (struct task*) e->data;

        // Restore remnant execution time
        t->ret = t->wcet;
        t->instance = t->instance + 1;

        // Add task to ready list
        struct task_item* ti = (struct task_item*) malloc(sizeof(struct task_item));
        ti->prio = t->period; // RM
        ti->task = t;
        ti->next = NULL;
        insert_task_item(&task_list, ti);

        // Call scheduling
        if (last_task_arrival < e->time) {
                last_task_arrival = e->time;
                add_scheduling(e->time);
        }

        // Next arrival
        add_arrival(t, e->time + t->period);

        printf("%d\tT_%d_%d\tA\n", e->time, t->id, t->instance);
}

void insert_task_item(struct task_list* list, struct task_item* task_item)
{
        struct task_item** pp = &(list->head);
        struct task_item* entry = list->head;

        while (entry && entry->prio <= task_item->prio) {
                pp = &(entry->next);
                entry = entry->next;
        }
        
        *pp = task_item;
        task_item->next = entry;
}

void remove_task_item(struct task_list* list, struct task_item* task_item)
{
        struct task_item** pp = &(list->head);

        while (*pp != task_item) {
                pp = &(*pp)->next;
        }
        
        *pp = task_item->next;
}

void insert_event(struct event_list* list, struct event* event) {
        struct event** pp = &(list->head);
        struct event* entry = list->head;

        while (entry) {
                if (entry->time >= event->time)
                        break;
                pp = &(entry->next);
                entry = entry->next;
        }
        while (entry && entry->time == event->time) {
                if (entry->type >= event->type)
                        break;
                pp = &(entry->next);
                entry = entry->next;
        }
        *pp = event;
        event->next = entry;
}

struct event* new_event(int time, int type, void (*func)(struct event*), void* data)
{
        struct event* e = (struct event*) malloc(sizeof(struct event));
        e->time = time;
        e->type = type;
        e->func = func;
        e->data = data;
        e->next = NULL;
        return e;
}

void sim(struct event_list* list)
{
        struct event* head = list->head;
        do {
                head->func(head);
                if (head->type == END)
                        return;
                head = head->next;
        } while(head);
}

void print_list(struct event_list* list)
{
        struct event* head = list->head;
        do {
                printf("%d %d\n", head->time, head->type);
                head = head->next;
        } while(head);
}

void add_arrival(struct task* task, int time)
{
        insert_event(&list, new_event(time, ARRIVAL, task_arrival, (void*) task));
}

void add_scheduling(int time)
{
        insert_event(&list, new_event(time, SCHEDULING, schedule, (void*) NULL));
}

void add_terminate(int time)
{
        insert_event(&list, new_event(time, TERMINATE, terminate, (void*) NULL));
}

void set_simulation_end(int time)
{
        insert_event(&list, new_event(time, END, terminate, (void*) 0));
}

void load_rts_from_file(FILE* f, int n, struct task_list *l)
{

}

int main(int argc, char* argv[])
{
        struct task *t1 = (struct task*) malloc(sizeof(struct task));
        t1->id = 1;
        t1->wcet = 1;
        t1->period = 3;
        t1->deadline = 3;
        t1->ret = t1->wcet;
        t1->instance = 0;
        struct task *t2 = (struct task*) malloc(sizeof(struct task));
        t2->id = 2;
        t2->wcet = 1;
        t2->period = 4;
        t2->deadline = 4;
        t2->ret = t2->wcet;
        t2->instance = 0;
        struct task *t3 = (struct task*) malloc(sizeof(struct task));
        t3->id = 3;
        t3->wcet = 2;
        t3->period = 6;
        t3->deadline = 6;
        t3->ret = t3->wcet;
        t3->instance = 0;
        struct task *t4 = (struct task*) malloc(sizeof(struct task));
        t4->id = 4;
        t4->wcet = 1;
        t4->period = 12;
        t4->deadline = 12;
        t4->ret = t4->wcet;
        t4->instance = 0;
        add_arrival(t1, 0);
        add_arrival(t2, 0);
        add_arrival(t3, 0);
        add_arrival(t4, 0);
        set_simulation_end(25);
        sim(&list);
        exit(EXIT_SUCCESS);
}
