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
        int wcet;
        int period;
        int deadline;
        int ret;
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
void add_task_arrival(struct task* task, int time);
void set_simulation_end(int time);
void schedule(struct event* e);

void terminate(struct event* e) {
        printf("%d\tE\t%d\n", e->time, task_list.head->prio);
        // remove task item
        struct task_item *ti = task_list.head;
        remove_task_item(&task_list, ti);
        free(ti);
        // new scheduling event
        insert_event(&list, new_event(e->time, SCHEDULING, schedule, NULL));
}

void schedule(struct event* e) {
        if (task_list.head) {
                struct task* t = task_list.head->task;
                printf("%d\tS\t%d\n", e->time, task_list.head->prio);
                if (e->time + t->ret <= e->next->time) {
                        insert_event(&list, new_event(e->time + t->ret, 
                                                TERMINATE, terminate, (void*) NULL));
                }
                t->ret = t->ret - (e->next->time - e->time);
        } 
}

int last_task_arrival = -1;

void task_arrival(struct event* e) {
        struct task *t = (struct task*) e->data;
        printf("%d\tA\t%d\n", e->time, t->period);

        // Restore remanent execution time
        t->ret = t->wcet;

        // Add task to ready list
        struct task_item* ti = (struct task_item*) malloc(sizeof(struct task_item));
        ti->prio = t->period; // RM
        ti->task = t;
        ti->next = NULL;
        insert_task_item(&task_list, ti);

        // Call scheduling
        if (last_task_arrival < e->time) {
                last_task_arrival = e->time;
                insert_event(&list, new_event(e->time, SCHEDULING, schedule, NULL));
        }

        // Next arrival
        add_task_arrival(t, e->time + t->period);
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

void add_task_arrival(struct task* task, int time)
{
        insert_event(&list, new_event(time, ARRIVAL, task_arrival, (void*) task));
}

void set_simulation_end(int time)
{
        insert_event(&list, new_event(time, END, terminate, (void*) 0));
}

int main(int argc, char* argv[])
{
        struct task *t1 = (struct task*) malloc(sizeof(struct task));
        t1->wcet = 1;
        t1->period = 3;
        t1->deadline = 3;
        t1->ret = t1->wcet;
        struct task *t2 = (struct task*) malloc(sizeof(struct task));
        t2->wcet = 1;
        t2->period = 4;
        t2->deadline = 4;
        t2->ret = t2->wcet;
        struct task *t3 = (struct task*) malloc(sizeof(struct task));
        t3->wcet = 2;
        t3->period = 6;
        t3->deadline = 6;
        t3->ret = t3->wcet;
        struct task *t4 = (struct task*) malloc(sizeof(struct task));
        t4->wcet = 1;
        t4->period = 12;
        t4->deadline = 12;
        t4->ret = t4->wcet;
        add_task_arrival(t1, 0);
        add_task_arrival(t2, 0);
        add_task_arrival(t3, 0);
        add_task_arrival(t4, 0);
        set_simulation_end(25);
        sim(&list);
        exit(EXIT_SUCCESS);
}
