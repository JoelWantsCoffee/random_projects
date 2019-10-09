#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <string.h>
#include <pthread.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>
#include <signal.h>

#include "../libs/utils.h"
#include "../libs/vecmath.h"
#include "../libs/surfaces.h"
#include "../libs/cam3d.h"
#include "../libs/shells.h"


void loop(input *ins, int  fc, void ** arg) {
    camera c;
    initCamera(&c, 200, 80);
    mesh me;

    dupeMesh((mesh *) arg[1], &me);

    vector mv = {
        .x = 0,
        .y = 0,
        .z = 0,
        .w = 0,
    };

    rotateMesh(&me, ((float) fc)/10, ((float) fc)/30, 0);
    //rotateMesh(&me, -3.141592/2, 0, 0);

    if (ins->on[4]) ((vector*) *arg)->y -= 0.1;
    if (ins->on[6]) ((vector*) *arg)->y += 0.1;

    // if (((vector*) *arg)->y > 3.1) ((vector*) *arg)->y = -3.1;
    // if (((vector*) *arg)->y < -3.1) ((vector*) *arg)->y = 3.1;

    // if (((vector*) *arg)->w > 3.1) ((vector*) *arg)->w = -3.1;
    // if (((vector*) *arg)->w < -3.1) ((vector*) *arg)->w = 3.1;

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
    c.vrot = ((vector*) *arg)->y;
    c.hrot = ((vector*) *arg)->w;

    // c.flags = 0;

    image img;

    #include "img.h"

    for (int i = 0; i<me.faceCount - 1; i += 2) {

    me.faces[i + 0].info->loc[0].pos.x = 0;
    me.faces[i + 0].info->loc[0].pos.y = vars - 1;

    me.faces[i + 0].info->loc[1].pos.x = vars - 1;
    me.faces[i + 0].info->loc[1].pos.y = 0;

    me.faces[i + 0].info->loc[2].pos.x = vars - 1;
    me.faces[i + 0].info->loc[2].pos.y = vars - 1;

    me.faces[i + 1].info->loc[0].pos.x = 0;
    me.faces[i + 1].info->loc[0].pos.y = vars - 1;

    me.faces[i + 1].info->loc[1].pos.x = 0;
    me.faces[i + 1].info->loc[1].pos.y = 0;

    me.faces[i + 1].info->loc[2].pos.x = vars - 1;
    me.faces[i + 1].info->loc[2].pos.y = 0;

    me.faces[i + 0].info->texture = &img;
    me.faces[i + 0].info->flags = 2;
    me.faces[i + 1].info->texture = &img;
    me.faces[i + 1].info->flags = 2;

    }
    mesh3d(&c, &me);

    freeImage(&img);

    fputs("\033[1;1H", stdout);

    c.pic.flags = 0;

    //drawStipple(&c.pic, 8);
    draw(&c.pic);

    // glutDisplayFunc(drawGL);
    // glutMainLoop();

    freeCamera(&c);
    freeMesh(&me);

    clearIns(ins);
}

void *gi(void *ins) {
    while (1) getIns((input *) ins);
}

void sigintHandler(int sig_num) {
    resetKeys();
    exit(0);
}

int main(int argc, char ** argv) {

    mesh me;

    vector pos = {
        .x = 0,
        .y = 0.5,
        .z = -10,
        .w = 0,
    };

    void * arg [3];

    if (argc == 2) { 
	inportObj(&me, argv[1]);
    } else {
	inportObj(&me, "monkey.obj");
    }

    struct osn_context * con;
    //open_simplex_noise(77374, &con);

    arg[0] = (void*) &pos;

    // int d = 40;

    // vector pts[d][d];

    // initMesh(&me, d*d, d*d*2);

    // me.faceCount = (d-1)*(d-1)*2;

    // for (int i = 0; i < d; i++) {
    //     for (int j = 0; j < d; j++) {
    //         pts[i][j].x = i - d/2;
    //         pts[i][j].z = j - d/2;
    //         float y = 10;
    //         int am = 20;
    //         for (int k = 1; k<am; k++) {
    //             y += open_simplex_noise3(con, ((float) i) * 0.3, ((float) j)*0.3, k*10) * map(k, 1, am, 1, 0);
    //         }
    //         pts[i][j].y = y;
    //         pts[i][j].v = intC(cLerp(fColour(216, 255, 188), fColour(106, 124, 3), map(pts[i][j].y, 9, 11, 0, 1)));
    //         me.pts[i + j*d] = pts[i][j];
    //     }
    // }

    // for (int i = 0; i < d - 1; i++) {
    //     for (int j = 0; j < d - 1; j++) {
    //         int index = i + j*(d-1);

    //         me.faces[index * 2].p1 = &me.pts[i + j*d];
    //         me.faces[index * 2].p2 = &me.pts[i + 1 + j*d];
    //         me.faces[index * 2].p3 = &me.pts[i + (j+1)*d];

    //         me.faces[index * 2 + 1].p2 = &me.pts[i + 1 + (j+1)*d];
    //         me.faces[index * 2 + 1].p1 = &me.pts[i + 1 + j*d];
    //         me.faces[index * 2 + 1].p3 = &me.pts[i + (j+1)*d];
    //     }
    // }

    arg[1] = (void*) &me;

    pthread_t getInputs;

    shell sh;

    pthread_create(&getInputs, NULL, gi, &(sh.ins));

    initShell(&sh, &loop);

    //sh.flags = 1;

    liveInputs(&sh);

    int fc = 0;

    atexit(resetKeys);
    //on_exit(resetKeys);

    signal(SIGINT, sigintHandler);

    while (1) {
        //getIns(&sh.ins);
        //pthread_create(&getInputs, NULL, &gi, &(sh.ins));
        run(&sh, fc, arg, 15);
        fc++;
    }
    return 0;
}
