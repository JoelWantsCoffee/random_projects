typedef struct _vec2d {
    float x;
    float y;
} vec2d;


float lerp(float a, float b, float dist) {
    return (a*(1 - dist) + b*dist);
}

float map(float a, float min, float max, float nmin, float nmax) {
    return ((a - min)/(max - min))*nmax + nmin;
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
