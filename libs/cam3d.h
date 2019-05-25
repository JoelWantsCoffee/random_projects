// #include "surfaces.h"
// #include "vecmat.h"
#define sign(a) (((a)<(0))?(-1):(1))
//Structs
typedef struct _camera {
    vector pos;
    float hrot;
    float vrot;
    float pNear;
    float pFar;
    float fov;
    float fovRad;
    
    int phong;

    float charRatio;
    float drawDist;
    int width;
    int height;

    surface image;

    float * dists;
} camera;

//functions
void wipe(camera *cam) {
    background(&(cam->image), mColour(0));
    for (int i = 0; i<cam->width * cam->height; i++) cam->dists[i] = cam->drawDist;
}

void initCamera(camera *cam, int w, int h) {
    cam->hrot = 0;
    cam->vrot = 0;
    cam->pos.x = 0;
    cam->pos.y = 0;
    cam->pos.z = 0;
    cam->fov = 3.141592/4;
    cam->pFar = 1000;
    cam->pNear = 0.1f;
    cam->fovRad = 1/tanf((cam->fov)/2);
    cam->drawDist = 1000;

    cam->width = w;
    cam->height = h;
    cam->charRatio = 2/1;

    cam->phong = 1;

    initSurf(&(cam->image), w, h);
    cam->dists = malloc(sizeof(float) * (w * h));

    wipe(cam);
}

void freeCamera(camera *cam) {
    free(cam->dists);
    freeSurf(&cam->image);
}

void getProjectionMat(camera cam, mat * out) {
    initMat(out, 0);
    float aspectRatio = ((float) cam.width) / ((float) cam.height);
    out->m[0][0] = cam.fovRad/aspectRatio;
    out->m[1][1] = cam.fovRad;
    out->m[2][2] = cam.pFar/(cam.pFar - cam.pNear);
    out->m[3][2] = -(cam.pFar * cam.pNear)/(cam.pFar - cam.pNear);
    out->m[2][3] = 1;
}

void set3d(camera *cam, float x, float y, float z, colour col) {
    int w = cam->width;
    int h = cam->height;
    if (((x >= 0) && (y >= 0)) && ((x < w) && (y < h))) {
        int i = ((int) x) + ((int) y) * w;
        if (cam->dists[i] > z) {
            cam->dists[i] = z;
            cam->image.pixels[i] = col;
        }
    }
}

void line3d(camera *cam, float x1, float y1, float z1, float x2, float y2, float z2) {
    float xmove = (x2 - x1);
    float ymove = (y2 - y1);
    float zmove = (z2 - z1);

    int dist;

    if (abs(xmove) > abs(ymove)) {
        ymove = ymove / abs(xmove);
        zmove = zmove / abs(xmove);
        dist = (int) xmove;
        xmove = sign(xmove);
    } else {
        xmove = xmove / abs(ymove);
        zmove = zmove / abs(ymove);
        dist = (int) ymove;
        ymove = sign(ymove);
    }

    float tx = x1;
    float ty = y1;
    float tz = z1;

    for (int i = 0; i<abs(dist); i++) {
        set3d(cam, tx, ty, tz, mColour(255));
        tx += xmove;
        ty += ymove;
        tz += zmove;
    }
}

void fillTri3d(camera * cam, vector v1, vector v2, vector v3, int phong) {
    v1.x = (int) v1.x;
    v1.y = (int) v1.y;
    v2.x = (int) v2.x;
    v2.y = (int) v2.y;
    v3.x = (int) v3.x;
    v3.y = (int) v3.y;

    vector *miny = &v1;
    vector *maxy = &v1;
    vector *midy = &v1;

    if (v2.y < miny->y) {
	    miny = &v2;
    } else {
	    maxy = &v2;
    }

    if (v3.y < miny->y) {
        midy = miny;
        miny = &v3;
    } else if (v3.y > maxy->y) {
        midy = maxy;
        maxy = &v3;
    } else {
	    midy = &v3;
    }

    colour c1, c2, cminy, cmidy, cmaxy;
    colour c = cLerp(cLerp(cInt(miny->v), cInt(midy->v), 0.5), cInt(maxy->v), 0.33333);

    if (phong) {
        colour cminy = cInt(miny->v);
        colour cmidy = cInt(midy->v);
        colour cmaxy = cInt(maxy->v);
    }

    for (int y = miny->y; y < midy->y; y++) {
        float p1 = ((float) y - miny->y) / (midy->y - miny->y);
        float p2 = ((float) y - miny->y) / (maxy->y - miny->y);

        int x1 = (int) lerp(miny->x, midy->x, p1);
        int x2 = (int) lerp(miny->x, maxy->x, p2);

        float z1 = lerp(miny->z, midy->z, p1);
        float z2 = lerp(miny->z, maxy->z, p2);


        if (phong) {
            c1 = cLerp(cminy, cmidy, p1);
            c2 = cLerp(cminy, cmaxy, p2);
        }
        
        int minx = MIN(x1, x2);
        int maxx = MAX(x1, x2);

        float fcol = 0;

        for (int x = MIN(x1, x2); x<MAX(x1, x2); x++) {

            float pp = ((float) x - x1) / (x2 - x1);

            float z = lerp(z1, z2, pp);

            if (phong) c = cLerp(c1, c2, pp);

            set3d(cam, x, y, z, c);
        }
    } 

    for (int y = midy->y; y < maxy->y; y++) {
        float p1 = ((float) y - midy->y) / (maxy->y - midy->y);
        float p2 = ((float) y - miny->y) / (maxy->y - miny->y);
        
        int x1 = (int) lerp(midy->x, maxy->x, p1);
        int x2 = (int) lerp(miny->x, maxy->x, p2);
        
        float z1 = lerp(midy->z, maxy->z, p1);
        float z2 = lerp(miny->z, maxy->z, p2);

        if (phong) {
            c1 = cLerp(cmidy, cmaxy, p1);
            c2 = cLerp(cminy, cmaxy, p2);
        }

        int bcol = 0;

        int minx = MIN(x1, x2);
        int maxx = MAX(x1, x2);

        for (int x = MIN(x1, x2); x<MAX(x1, x2); x++) {

            float pp = ((float) x - x1) / (x2 - x1);

            float z = lerp(z1, z2, pp);

            if (phong) c = cLerp(c1, c2, pp);

            set3d(cam, x, y, z, c);    
        }
    }
}

void tri3d(camera *cam, tri t) {
    float charRatio = cam->charRatio;
    float w = (float) cam->width;
    float h = (float) cam->height;

    // float x1 = (t.p1.x * charRatio + 1) * w/2;
    // float y1 = (t.p1.y + 1) * h/2;
    // float x2 = (t.p2.x * charRatio + 1) * w/2;
    // float y2 = (t.p2.y + 1) * h/2;
    // float x3 = (t.p3.x * charRatio + 1) * w/2;
    // float y3 = (t.p3.y + 1) * h/2;

    // vector p1 = {
    //     .x = x1,
    //     .y = y1,
    //     .z = t.p1.z,
    //     .v = t.p1.v,
    // };

    // vector p2 = {
    //     .x = x2,
    //     .y = y2,
    //     .z = t.p2.z,
    //     .v = t.p2.v,
    // };

    // vector p3 = {
    //     .x = x3,
    //     .y = y3,
    //     .z = t.p3.z,
    //     .v = t.p3.v,
    // };

    t.p1.x = (t.p1.x * charRatio + 1) * w/2;
    t.p1.y = (t.p1.y + 1) * h/2;
    t.p2.x = (t.p2.x * charRatio + 1) * w/2;
    t.p2.y = (t.p2.y + 1) * h/2;
    t.p3.x = (t.p3.x * charRatio + 1) * w/2;
    t.p3.y = (t.p3.y + 1) * h/2;

    fillTri3d(cam, t.p1, t.p2, t.p3, cam->phong);

    //setCol(&cam->image, mColour(t.var));

    //fillTriangle(&cam->image, x1, y1, x2, y2, x3, y3);

    // line3d(cam, x1, y1, t.p1.z, x2, y2, t.p2.z);
    // line3d(cam, x2, y2, t.p2.z, x3, y3, t.p3.z);
    // line3d(cam, x3, y3, t.p3.z, x1, y1, t.p1.z);
}

void clipMeshToCam(mesh *me) {
    vector pl1 = {.x = -0.5, .y = 0, .z = 0};
    vector pn1 = {.x = 1, .y = 0, .z = 0};
    vector pl2 = {.x = 0.5, .y = 0, .z = 0};
    vector pn2 = {.x = -1, .y = 0, .z = 0};
    vector pl3 = {.x = 0, .y = 1, .z = 0};
    vector pn3 = {.x = 0, .y = -1, .z = 0};
    vector pl4 = {.x = 0, .y = -1, .z = 0};
    vector pn4 = {.x = 0, .y = 1, .z = 0};
    clipMesh(me, pl1, pn1);
    clipMesh(me, pl2, pn2);
    clipMesh(me, pl3, pn3);
    clipMesh(me, pl4, pn4);
}

void mesh3d(camera *cam, mesh m) {
    mesh me;
    dupeMesh(&m,&me);
    vector l = {.x = 1, .y = -1, .z = -1};
    vecNormalise(&l);

    for (int i = 0; i<me.faceCount; i++) me.faces[i].var = (sudoTheta(getNormal(ftot(me.faces[i])), l) + 1)*0.5*255;

    colour cols [me.ptCount];
    int totalFaces [me.ptCount];

    for (int i = 0; i<me.ptCount; i++) {
        cols[i] = mColour(0);
        totalFaces[i] = 0;
    }

    for (int i = 0; i < m.faceCount; i++) {
        int i1 = me.faces[i].p1 - me.pts;
        int i2 = me.faces[i].p2 - me.pts;
        int i3 = me.faces[i].p3 - me.pts;

        colour col = mColour(me.faces[i].var);

        cols[i1] = cLerp(cols[i1], col, 1.0 / ((float) ++totalFaces[i1]));
        cols[i2] = cLerp(cols[i2], col, 1.0 / ((float) ++totalFaces[i2]));
        cols[i3] = cLerp(cols[i3], col, 1.0 / ((float) ++totalFaces[i3]));
    }

    for (int i = 0; i<me.ptCount; i++) {
        me.pts[i].v = intC( cMult(cInt(me.pts[i].v) , (((float) cols[i].r)/255.0) ));
    }

    translateMesh(&me, cam->pos.x, cam->pos.y, -cam->pos.z);
    rotateMesh(&me, cam->vrot, cam->hrot, 0);
    
    mat proj; 
    vector pl = {.x = 0, .y = 0, .z = cam->pNear};
    vector pn = {.x = 0, .y = 0, .z = 1};

    for (int i = 0; i<me.faceCount; i++)  {
        if (sudoTheta(getNormal(ftot(me.faces[i])), *me.faces[i].p1) > 0) me.faces[i].var = -1;
    }

    getProjectionMat(*cam, &proj);
    clipMesh(&me, pl, pn);
    meshmultmat(&me, proj);
    
    //copyMesh(&newMesh, &me);

    clipMeshToCam(&me);

    for (int i = 0; i<me.faceCount; i++) {
        if (me.faces[i].var >= 0) tri3d(cam, ftot(me.faces[i]));
    }
}

int pullSubString(char * in, int inLen, char * out, char b, int index) {
    /*printf("\n---------------stat--------------\n");
    printf("Char: (%c)\n", b);
    printf("In len: (%d)\n", inLen);
    printf("Index: (%d)\n", index);
    printf("in: %s", in);*/
    int c = 0;
    for (int i = 0; i<inLen; i++) {
        //printf("(%c vs %c)\n", in[i], b);
        if (c == index) {
            int j;
            for (j = 0; (!(in[i + j] == b) && (j+i+1 < inLen)); j++) {
                out[j] = in[i+j];
            }
            out[j] = '\0';
            return j+1;
        }

        if (in[i] == b) c++;
    }
    return 0;
}

void inportObj(mesh *me, char * fileName) {
    FILE *fp;

    fp = fopen(fileName, "r");

    int len = 0;

    char ch;

    while((ch = fgetc(fp)) != EOF) {
        len++;
    }

    char t [len + 1];
    int c = 0;

    rewind(fp);

    while((ch = fgetc(fp)) != EOF) {
        t[c] = ch;
        c++;
    }

    fclose(fp);

    if (t[c-1] == '\n') c--;

    t[c] = '\0';
    int ptCount = 0;
    int faCount = 0;

    int lCount = 0;

    for (int c = 0; c<len+1; c++) {
        if (t[c] == '\n') lCount++;
    }

    for (int i = 0; i<lCount+1; i++) {
        char line [100];
        int l = pullSubString(t, len, line, '\n', i);
        if (l > 2) {
            if ((line[0] == 'v') && (line[1] = ' ')) ptCount++;
            if ((line[0] == 'f') && (line[1] = ' ')) faCount++;
        }
    }

    //printf("pt count: %d, f count %d", ptCount, faCount);

    initMesh(me, ptCount, faCount);

    //printf("whot %d", lCount);
    //fflush(stdout);

    int ptTally = 0;
    int faTally = 0;

    for (int i = 0; i<lCount+1; i++) {


        char line [100];
        int l = pullSubString(t, len, line, '\n', i);
        //printf("str: (%s)\n", line);
        //fflush(stdout);

        if (line[0] == 'v') {
            //printf("tally: %d", ptTally);
            char num [100];
            pullSubString(line, l, num, ' ', 1);
            me->pts[ptTally].x = (float) atof(num);
            pullSubString(line, l, num, ' ', 2);
            me->pts[ptTally].y = (float) atof(num);
            pullSubString(line, l, num, ' ', 3);
            me->pts[ptTally].z = (float) atof(num);
            me->pts[ptTally].w = 0;
            me->pts[ptTally].v = 0xFFFFFF;
            ptTally++;
        }
        if (line[0] == 'f') {
            char num [100];
            pullSubString(line, l, num, ' ', 1);
            me->faces[faTally].p1 = &(me->pts[atoi(num) - 1]);
            pullSubString(line, l, num, ' ', 2);
            me->faces[faTally].p2 = &(me->pts[atoi(num) - 1]);
            pullSubString(line, l, num, ' ', 3);
            me->faces[faTally].p3 = &(me->pts[atoi(num) - 1]);
            faTally++;
        }
    }
}
