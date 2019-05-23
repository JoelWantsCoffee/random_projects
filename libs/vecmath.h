#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))

//Structs
typedef struct _mat {
    float m[4][4];
} mat;

typedef struct _vector {
    float x;
    float y;
    float z;
    float w;
} vector;

typedef struct _tri {
    vector p1;
    vector p2;
    vector p3;
    long var;
} tri;

typedef struct _face {
    vector *p1;
    vector *p2;
    vector *p3;
    long var;
} face;

typedef struct _mesh {
    vector * pts;
    size_t ptCount;
    face * faces;
    size_t faceCount;
} mesh;


//Functions
void initMat(mat * in, float scale) {
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            if (i == j) {
                in->m[i][j] = scale;
            } else {
                in->m[i][j] = 0;
            }
        }
    }
}

void initMesh(mesh * in, int ptLen, int faLen) {
    in->ptCount = ptLen;
    in->faceCount = faLen;
    in->pts = malloc(sizeof(vector) * MAX(ptLen,1) );
    in->faces = malloc(sizeof(face) * MAX(faLen, 1) );
}

void freeMesh(mesh * me) {
    free(me->pts);
    me->pts = NULL;
    free(me->faces);
    me->faces = NULL;
}

mat matmultmat(mat one, mat two) {
    mat out;
    for (int i = 0; i<4; i++) {
        for (int j = 0; j<4; j++) {
            out.m[i][j] = 0;
            for (int k = 0; k<4; k++) {
                out.m[i][j] +=  one.m[k][j] * two.m[i][k];
            }
        }
    }
    return out;
}

vector vectormultmat(vector vec, mat ma) {
    vector out = {
        .x = vec.x * ma.m[0][0] + vec.y * ma.m[1][0] + vec.z * ma.m[2][0] + ma.m[3][0],
        .y = vec.x * ma.m[0][1] + vec.y * ma.m[1][1] + vec.z * ma.m[2][1] + ma.m[3][1],
        .z = vec.x * ma.m[0][2] + vec.y * ma.m[1][2] + vec.z * ma.m[2][2] + ma.m[3][2],
        .w = vec.x * ma.m[0][3] + vec.y * ma.m[1][3] + vec.z * ma.m[2][3] + ma.m[3][3],
    };
    return out;
}

vector crossProduct(vector v1, vector v2) {
    vector out = {
        .x = v1.y*v2.z - v1.z*v2.y,
        .y = v1.z*v2.x - v1.x*v2.z,
        .z = v1.x*v2.y - v1.y*v2.x,
    };
    return out;
}

float dotProduct(vector v1, vector v2) {
    return (v1.x * v2.x + v1.y * v2.y + v1.z * v2.z);
}

vector vecSubtract(vector v1, vector v2) {
    vector out = {
        .x = v1.x - v2.x,
        .y = v1.y - v2.y,
        .z = v1.z - v2.z,
    };
    return out;
}

vector vecAdd(vector v1, vector v2) {
    vector out = {
        .x = v1.x + v2.x,
        .y = v1.y + v2.y,
        .z = v1.z + v2.z,
        .w = v1.w + v2.w,
    };
    return out;
}

vector scale(vector v, float s) {
    vector out = {
        .x = v.x * s,
        .y = v.y * s,
        .z = v.z * s,
        .w = v.w * s,
    };
    return out;
}

void vecNormalise(vector * v) {
    float largestValue = MAX( abs(v->x), abs(v->y));
    largestValue = MAX(largestValue, abs(v->z));
    v->x /= largestValue;
    v->y /= largestValue;
    v->z /= largestValue;
}

tri ftot(face in) {
    tri out = {
        .p1 = *in.p1,
        .p2 = *in.p2,
        .p3 = *in.p3,
        .var = in.var,
    };
    return out;
}

//second degree functions
vector vectormultmatScaled(vector vec, mat ma) {
    vector out = vectormultmat(vec, ma);
    out = scale(out, 1/(out.w));
    return out;
}

vector getNormal(tri in) {
    vector v1 = vecSubtract(in.p1, in.p2);
    vector v2 = vecSubtract(in.p1, in.p3);
    return crossProduct(v1, v2);
}

tri trianglemultmat(tri tr, mat ma) {
    tri out = {
        .p1 = vectormultmatScaled(tr.p1, ma),
        .p2 = vectormultmatScaled(tr.p2, ma),
        .p3 = vectormultmatScaled(tr.p3, ma),
    };
    return out;
}

void meshmultmat(mesh *me, mat ma) {
    for (int i = 0; i<me->ptCount; i++) me->pts[i] = vectormultmatScaled(me->pts[i], ma);
}

float magnitude(vector in) {
    return sqrt(in.x*in.x + in.y*in.y + in.z*in.z);
}

float sudoTheta(vector v1, vector v2) {
    return (dotProduct(v1, v2)/(magnitude(v1) * magnitude(v2)));
}

float distance(vector p, vector n, vector *v) {
    return (n.x)*(v->x) + (n.y)*(v->y) + (n.z)*(v->z) - dotProduct(n, p);
}

vector vecIntersectPlane(vector p1, vector p2, vector *l1, vector *l2) {
	vecNormalise(&p2);
	float pd = -dotProduct(p2, p1);
	float ad = dotProduct(*l1, p2);
	float bd = dotProduct(*l2, p2);
	float t = (-pd - ad) / (bd - ad);
	vector wl = vecSubtract(*l2, *l1);
	vector pl = scale(wl, t);
	return vecAdd(*l1, pl);
}

void translateMesh(mesh *me, float x, float y, float z) {
    mat m;
    initMat(&m, 1);
    m.m[3][0] = x;
    m.m[3][1] = y;
    m.m[3][2] = z;
    meshmultmat(me, m);
}

void rotateMesh(mesh *me, float xrot, float yrot, float zrot) {
    mat m;
    initMat(&m, 1);
    m.m[1][1] = cosf(xrot);
    m.m[2][2] = cosf(xrot);
    m.m[2][1] = -sinf(xrot);
    m.m[1][2] = sinf(xrot);
    meshmultmat(me, m);
    initMat(&m, 1);
    m.m[0][0] = cosf(yrot);
    m.m[2][2] = cosf(yrot);
    m.m[2][0] = sinf(yrot);
    m.m[0][2] = -sinf(yrot);
    meshmultmat(me, m);
    initMat(&m, 1);
    m.m[0][0] = cosf(zrot);
    m.m[1][0] = -sinf(zrot);
    m.m[1][1] = cosf(zrot);
    m.m[0][1] = sinf(zrot);
    meshmultmat(me, m);
}

int clipFaceNums(vector pl, vector pn, face *in, int *nPts) {
    vecNormalise(&pn);

    int ic = 0;
    int oc = 0;

    float d0 = distance(pl, pn, in->p1);
    float d1 = distance(pl, pn, in->p2);
    float d2 = distance(pl, pn, in->p3);

    if (d0 >= 0) {
	    ic++;
    } else {
	    oc++;
    }

    if (d1 >= 0) {
    	ic++;
    } else {
	    oc++;
    }
    
    if (d2 >= 0) {
    	ic++;
    } else {
	    oc++;
    }


    if (ic == 0) {
        *nPts = 0;
	    return 0;  
    } else if (ic == 3) {
        *nPts = 0;
        return 1;
    } else if (ic == 2) {
        *nPts = 2;
        return 2;
    } else if (ic == 1) {
        *nPts = 2;
        return 1;
    }
}

int clipFace(vector pl, vector pn, face *in, face *out [2], vector *op [2], int *nPts, mesh *meIn, mesh *meOut) {
    vecNormalise(&pn);

    out[0]->var = in->var;
    out[1]->var = in->var;

    vector *ipts [3];
    vector *opts [3];
    int ic = 0;
    int oc = 0;

    float d0 = distance(pl, pn, in->p1);
    float d1 = distance(pl, pn, in->p2);
    float d2 = distance(pl, pn, in->p3);



    vector * inDex = meIn->pts;

    if (d0 >= 0) {
	    ipts[ic++] = &meOut->pts[in->p1 - inDex];
    } else {
	    opts[oc++] = &meOut->pts[in->p1 - inDex];
    }

    if (d1 >= 0) {
    	ipts[ic++] = &meOut->pts[in->p2 - inDex];
    } else {
	    opts[oc++] = &meOut->pts[in->p2 - inDex];
    }
    
    if (d2 >= 0) {
    	ipts[ic++] = &meOut->pts[in->p3 - inDex];
    } else {
	    opts[oc++] = &meOut->pts[in->p3 - inDex];
    }

    

    if (ic == 0) {
        *nPts = 0;
	    return 0;  
    } else if (ic == 3) {
        out[0]->p1 = &meOut->pts[in->p1 - inDex];
        out[0]->p2 = &meOut->pts[in->p2 - inDex];
        out[0]->p3 = &meOut->pts[in->p3 - inDex];
        *nPts = 0;

        return 1;
    } else if (ic == 2) {
        *op[0] = vecIntersectPlane(pl,pn,ipts[0],opts[0]);
        *op[1] = vecIntersectPlane(pl,pn,ipts[1],opts[0]);


        out[0]->p1 = ipts[0];
        out[0]->p2 = op[0];
        out[0]->p3 = ipts[1];

        out[1]->p1 = ipts[1];
        out[1]->p2 = op[0];
        out[1]->p3 = op[1];

        *nPts = 2;

        return 2;
    } else if (ic == 1) {
        *op[0] = vecIntersectPlane(pl,pn,ipts[0],opts[0]);
        *op[1] = vecIntersectPlane(pl,pn,ipts[0],opts[1]);

        out[0]->p1 = ipts[0];
        out[0]->p2 = op[0];
        out[0]->p3 = op[1];

        *nPts = 2;

        return 1;
    }
    return 0;
}

void copyMesh(mesh *in, mesh *out) {
    out->pts = in->pts;
    out->ptCount = in->ptCount;
    out->faces = in->faces;
    out->faceCount = in->faceCount;
}

void dupeMesh(mesh *in, mesh *out) {
    initMesh(out, in->ptCount, in->faceCount);
    for (int i = 0; i<in->ptCount; i++) {
        vector p = {
            .x = in->pts[i].x,
            .y = in->pts[i].y,
            .z = in->pts[i].z
        };
        out->pts[i] = p;
    }
    for (int i = 0; i<in->faceCount; i++) {
        out->faces[i].p1 = &out->pts[in->faces[i].p1 - in->pts];
        out->faces[i].p2 = &out->pts[in->faces[i].p2 - in->pts];
        out->faces[i].p3 = &out->pts[in->faces[i].p3 - in->pts];
    }
}

void clipMesh(mesh *me, vector pl, vector pn) {
    int ptCount = 0;
    int faCount = 0;
    face o [2];
    face *po [2];
    po[0] = o;
    po[1] = o+1;
    vector p [2];
    vector *pp [2];
    pp[0] = p;
    pp[1] = p+1;
    int t = 0;
    for (int i = 0; i<me->faceCount; i++) {
        faCount += clipFaceNums(pl, pn, &me->faces[i], &t);
        ptCount += t;
    }

    mesh out;

    if (faCount > 0) {
        
        
        initMesh(&out, me->ptCount + ptCount, faCount);
        
        ptCount = 0;
        faCount = 0;

        for (int i = 0; i<me->ptCount; i++) out.pts[i] = me->pts[i];
        
        for (int i = 0; i<me->faceCount; i++) {
            pp[0] = &out.pts[me->ptCount + ptCount];
            pp[1] = &out.pts[me->ptCount + ptCount + 1];

            po[0] = &out.faces[faCount];
            po[1] = &out.faces[faCount + 1];

            int faCountInc = clipFace(pl, pn, &me->faces[i], po, pp, &t, me, &out);

            faCount += faCountInc;
            ptCount += t;
        }


        //freeMesh(me);

        copyMesh(&out, me);

    }
    
    // *me = out;
}