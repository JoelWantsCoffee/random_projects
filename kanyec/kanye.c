#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <string.h>
#include <pthread.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>

#include "../libs/vecmath.h"
#include "../libs/surfaces.h"
#include "../libs/cam3d.h"
#include "../libs/shells.h"

void loop(input *ins, int fc, void ** arg) {
    camera c;
    initCamera(&c, 400, 100);
    mesh me;

    dupeMesh((mesh *) arg[1], &me);

    //printf("%d", me.faceCount);

    //inportObj((mesh*) arg[1], "cube.obj");

    vector mv = {
        .x = 0,
        .y = 0,
        .z = 0,
        .w = 0,
    };

    rotateMesh(&me, 3.1415, ((float) fc)/10, 0);

    if (ins->on[4]) ((vector*) *arg)->y += 0.1;
    if (ins->on[6]) ((vector*) *arg)->y -= 0.1;

    if (ins->on[5]) ((vector*) *arg)->w += 0.1;
    if (ins->on[7]) ((vector*) *arg)->w -= 0.1;

    float hrot = ((vector*) *arg)->w;

    if (ins->on[1] + ins->on[3]) hrot += 3.1415926/2;


    mv.z = cos(hrot)*0.1;
    mv.x = sin(hrot)*0.1;

    if (ins->on[1] + ins->on[3]) hrot -= 3.1415926/2;

    if (ins->on[2] + ins->on[3]) {
        mv = scale(mv, -1);
    }

    if (!(ins->on[0] + ins->on[1] + ins->on[2] + ins->on[3])) mv = scale(mv, 0);

    *((vector*) *arg) = vecAdd(*((vector*) *arg), mv);

    c.pos.z = ((vector*) *arg)->z;
    c.pos.x = ((vector*) *arg)->x;
    c.pos.y = ((vector*) *arg)->y;
    c.hrot = ((vector*) *arg)->w;

    mesh3d(&c, me);

    system("clear");
    draw(c.image);

    //draw(c.image);

    freeCamera(&c);
    freeMesh(&me);

    clearIns(ins);
}

void *gi(void *ins) {
    while (1) getIns((input *) ins);
}

int main() {
    mesh me;

    vector pos = {
        .x = 0,
        .y = 0,
        .z = -5,
        .w = 0,
    };

    void * arg [2];

    inportObj(&me, "monkey.obj");

    arg[0] = (void*) &pos;
    arg[1] = (void*) &me;

    pthread_t getInputs;

    shell sh;

    pthread_create(&getInputs, NULL, gi, &(sh.ins));

    liveInputs();

    initShell(&sh, &loop);

    int fc = 0;

    atexit(resetKeys);
    //on_exit(resetKeys);

    while (1) {
        //getIns(&sh.ins);
        //pthread_create(&getInputs, NULL, &gi, &(sh.ins));
        run(&sh, fc, arg, 30);
        fc++;
    }
    return 0;
}
