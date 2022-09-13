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
        struct task* task;
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
struct task_list ready_list;
struct event_list list;

struct event* new_event(int time, int type, void (*func)(struct event*), struct task* task, void* data);
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
        struct task_item *ti = ready_list.head;
        // remove task item
        printf("%d\tT_%d_%d\tE\n", e->time, ti->task->id, ti->task->instance);
        remove_task_item(&ready_list, ti);
        free(ti);
        // new scheduling event
        add_scheduling(e->time);
}

void schedule(struct event* e) {
        if (ready_list.head) {
                struct task* t = ready_list.head->task;
                printf("%d\tT_%d_%d\tS\n", e->time, t->id, t->instance);
                if (e->time + t->ret <= e->next->time)
                        add_terminate(e->time + t->ret);
                t->ret = t->ret - (e->next->time - e->time);
        } 
}

int last_task_arrival = -1;

void task_arrival(struct event* e) {
        struct task *t = e->task;

        // Restore remnant execution time
        t->ret = t->wcet;
        t->instance = t->instance + 1;

        // Add task to ready list
        struct task_item* ti = (struct task_item*) malloc(sizeof(struct task_item));
        ti->prio = t->period; // RM
        ti->task = t;
        ti->next = NULL;
        insert_task_item(&ready_list, ti);

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

struct event* new_event(int time, int type, void (*func)(struct event*), struct task* task, void* data)
{
        struct event* e = (struct event*) malloc(sizeof(struct event));
        e->time = time;
        e->type = type;
        e->func = func;
        e->task = task;
        e->data = data;
        e->next = NULL;
        return e;
}

void sim(struct event_list* list)
{
        struct event* head = list->head;
        do {
                switch(head->type) {
                        case END:
                                return;
                        case ARRIVAL:
                                task_arrival(head);
                                break;
                        case TERMINATE:
                                terminate(head);
                                break;
                        case SCHEDULING:
                                schedule(head);
                                break;
                        default:
                                fprintf(stderr, "Error: unknown event.\n");
                                exit(EXIT_FAILURE);
                }
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
        insert_event(&list, new_event(time, ARRIVAL, NULL, task, NULL));
}

void add_scheduling(int time)
{
        insert_event(&list, new_event(time, SCHEDULING, NULL, NULL, NULL));
}

void add_terminate(int time)
{
        insert_event(&list, new_event(time, TERMINATE, NULL, NULL, NULL));
}

void set_simulation_end(int time)
{
        insert_event(&list, new_event(time, END, NULL, NULL, NULL));
}

void load_rts_from_file(FILE* f, struct task_list *l)
{
        int n, i;

        fscanf(f, "%d\n", &n);

        for (i = 0; i < n; i++) {
                struct task *t = (struct task*) malloc(sizeof(struct task));
                t->id = i + 1;
                fscanf(f, "%d\n", &(t->wcet));
                fscanf(f, "%d\n", &(t->period));
                fscanf(f, "%d\n", &(t->deadline));
                t->ret = t->wcet;
                t->instance = 0;

                // Add task to list
                struct task_item* ti = (struct task_item*) malloc(sizeof(struct task_item));
                ti->prio = 0;
                ti->task = t;
                ti->next = NULL;
                insert_task_item(l, ti);
        }
}

int main(int argc, char* argv[])
{
        FILE* f;
        if (argc == 2) { 
            f = fopen(argv[1], "r");
        } else {
            f = stdin;
        }
        load_rts_from_file(f, &task_list);

        struct task_item* head = task_list.head;
        do {
                add_arrival(head->task, 0);
                head = head->next;
        } while(head);
        
        set_simulation_end(25);
        sim(&list);
        exit(EXIT_SUCCESS);
}
