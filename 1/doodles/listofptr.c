#include <stdio.h>
#include <stdlib.h>

typedef struct thingy {
    struct list_item** l;
} thingy;

typedef struct list_item {
    int id;
    struct char* name;
} list_item;

list_item* a;
list_item* b;
list_item* c;
thingy* t;

int main() {
    a = (list_item*) malloc(sizeof(list_item));
    b = (list_item*) malloc(sizeof(list_item));
    c = (list_item*) malloc(sizeof(list_item));
    a->id = 10;
    a->name = "Something";
    b->id = 20;
    b->name = "Heh";
    c->id = 30;
    c->name = "SDF";

    
    t = (thingy*) malloc(sizeof(thingy));
    t->l = (list_item**) malloc(sizeof(list_item*) * 3);
    t[0] = a;
    t[1] = b;
    t[2] = c;
    return 0;
}
