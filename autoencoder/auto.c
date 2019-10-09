#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "lodepng.h"
#include "../libs/utils.h"
#include "../libs/surfaces.h"
#include "../libs/ai.h"

#define SIZEE 361

int main() {
    //create network
    system("clear");

    Network n;
    int layers[5] = {SIZEE, 56, 10, 56, SIZEE};
    initFCNetwork(layers, 5, &n, frand, &sigmoid);
    int count = 0;



    system("(find imgs | grep .png) > img.txt");

    FILE * files;

    files = fopen("img.txt", "r");

    int l;

    char temp [80];
    for (l = 0; fgets(temp, 80, files); l++);

    rewind(files);

    image * imgs = malloc(sizeof(image) * l);

    for (int i = 0; i<l; i++) {
        char buff [80];

        fgets(buff,80,files);

        buff[strlen(buff) - 1] = '\0';

        printf("%s\n", buff);

        unsigned char* im = 0;
        unsigned width, height;

        lodepng_decode32_file(&im, &width, &height, buff);

        initImage(imgs + i, width, height);
        lodeImage(imgs + i, im);

        mv(0,0);
        printImage(imgs + i);
    }

    system("clear");

    while (1) {
    	for (int j = 0; j<100; j++) {
    	    //train
            image * inimg = imgs + (rand() % l);
            long double * in = encodeImage(inimg);
            long double * out = malloc(sizeof(long double) * SIZEE);

            for (int i = 0; i < SIZEE; i++) {
                out[i] = in[i];
            }

            for (int i = 0; i < SIZEE; i++) {
                if (!(rand() % 8)) {
                    in[i] = 0;
                }
            }

    	    setNetwork(in, &n);
            computeNetwork(&n);
            backprop(in, &n, 0.001);
    	    free(in);
            free(out);
            count++;
    	}


		//demo
        mv(0,0);
        image * inimg = imgs + (rand() % l);
	    long double * in = encodeImage(inimg);
    	setNetwork(in, &n);
        computeNetwork(&n);
    	long double out [SIZEE];
    	for (int i = 0; i<SIZEE;i++) out[i] = n.out[i]->value;
    	fflush(stdout);

        image outimg = decodeImage(out, 19, 19);
        printImage(inimg);
        printf("\n");
        printImage(&outimg);

        freeImage(&outimg);

        free(in);
        printf("\n");
    }

    return 0;
}
