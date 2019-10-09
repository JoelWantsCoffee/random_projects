#define MAX(a,b) (((a)>(b))?(a):(b))
#define MIN(a,b) (((a)<(b))?(a):(b))

typedef struct _outFunc {
    long double (*f)(long double);
    long double (*d)(long double);
} outFunc;

typedef struct _Neuron Neuron;

typedef struct _Neuron {
    Neuron * *in;
    Neuron * *out;
    long double * weight;
    long double bias;
    long double value;
    long double error;
    int comped;
    outFunc *func;
    size_t len;
    size_t outLen;
} Neuron;

typedef struct _Network {
    Neuron * neuron;
    Neuron * *in;
    Neuron * *out;
    size_t neuronLen;
    size_t inLen;
    size_t outLen;
} Network;

typedef float (*numGen)();

float zero() {
    return 0.0f;
}

float frand() {
    return ((((float) rand())/RAND_MAX) - 0.5) * 2;
}

long double sig(long double x) {
    return (1.0f / (1.0f + ((long double) pow(2.71828182846, -x))));
}

long double dsig(long double x) {
    return (x*(1.0f-x));
}

outFunc sigmoid = {
    .f = sig,
    .d = dsig,
};

long double fxx(long double x) {
    return x;
}


long double fxg0(long double x) {
    return MAX(x,0);
}

long double dfxx(long double x) {
    return 1;
}

outFunc fx = {
    .f = fxx,
    .d = dfxx,
};

outFunc relu = {
    .f = fxg0,
    .d = dfxx,
};


int initNeuron(Neuron * n, numGen w, outFunc *f, Neuron * *cons, Neuron * *outcons, int len, int outLen) {
    n->value = 0;
    n->error = 0;
    n->comped = 0;
    n->len = len;
    n->outLen = outLen;
    n->func = f;
    if (outLen) {

        n->out = malloc(sizeof(Neuron*) * outLen);
            for (int i = 0; i<outLen; i++) {    
            n->out[i] = outcons[i]; 
        };
    }

    if (len) {

        n->in = malloc(sizeof(Neuron*) * len);
        n->weight = malloc(sizeof(long double) * len);
        for (int i = 0; i<len; i++) {    
            n->in[i] = cons[i];
            n->weight[i] = (long double) w();
        }
    }

    n->bias = w();

    return 0;
}

int initFCNetwork(int * layerSizes, int numLayers, Network *net, numGen w, outFunc *f) {
    int numNeurons = 0;
    for (int i = 0; i < numLayers; i++) {
        numNeurons += layerSizes[i];
    }

    net->neuronLen = numNeurons;
    net->inLen = layerSizes[0];
    net->outLen = layerSizes[numLayers - 1];

    net->neuron = (Neuron*) malloc(sizeof(Neuron) * numNeurons);
    net->in = (Neuron**) malloc(sizeof(Neuron*) * layerSizes[0]);
    net->out = (Neuron**) malloc(sizeof(Neuron*) * layerSizes[numLayers - 1]);

    Neuron * ltptr [layerSizes[1]];
    for (int i = 0; i<layerSizes[1]; i++ ) ltptr[i] = &net->neuron[i + layerSizes[0]];
    for (int i = 0; i<layerSizes[0]; i++) {
        initNeuron(&net->neuron[i], w, f, 0, ltptr, 0, layerSizes[1]);
    }
    int tally = layerSizes[0];
    for (int i = 1; i < numLayers - 1; i++) {
        Neuron * inptr [layerSizes[i - 1]];
        Neuron * outptr [layerSizes[i + 1]];
        for (int j = 0; j<layerSizes[i - 1]; j++) inptr[j] = &(net->neuron[tally - layerSizes[i - 1] + j]);
        for (int j = 0; j<layerSizes[i]; j++) {
            initNeuron(&net->neuron[tally + j], w, f, inptr, outptr, layerSizes[i - 1], layerSizes[i + 1]);
        }
        tally += layerSizes[i];
    }
    Neuron * lsptr [layerSizes[numLayers - 2]];
    for (int i = 0; i<layerSizes[numLayers - 2]; i++ ) lsptr[i] = &net->neuron[i + tally - layerSizes[numLayers - 2]];
    for (int i = 0; i<layerSizes[numLayers - 1]; i++) {
        initNeuron(&net->neuron[numNeurons - layerSizes[numLayers - 1] + i], w, f, lsptr, 0, layerSizes[numLayers - 2], 0);
        net->out[i] = &net->neuron[numNeurons - layerSizes[numLayers - 1] + i];
    }
    tally = 0;
    for (int i = 0; i<numLayers-1; i++) {
        for (int j = 0; j < layerSizes[i]; j++) {
            for (int k = 0; k < layerSizes[i + 1]; k++) net->neuron[tally + j].out[k] = &(net->neuron[tally + layerSizes[i] + k]);
        }
        tally += layerSizes[i];
    }
    for (int i = 0; i<layerSizes[numLayers - 2]; i++) {
        for (int j = 0; j<layerSizes[numLayers - 1]; j++) {
            net->neuron[i + tally - layerSizes[numLayers - 2]].out[j] = &net->neuron[tally + j];
        }
    }
    for (int i = 0; i<layerSizes[0]; i++) net->in[i] = &net->neuron[i];

    return 1;
}

int initRNN(int * layerSizes, int numLayers, Network *net, numGen w, outFunc *f) {
    if ((numLayers < 3) || (layerSizes[numLayers - 1] > layerSizes[numLayers - 2])) {
        return 0;
    }
    initFCNetwork(layerSizes, numLayers - 1, net, w, f);
    return 1;
}

long double compute(Neuron * n);

long double compute(Neuron * n) {
    if (!(n->comped)) {
        n->comped = 1;
        n->value = 0;
        for (int i = 0; i<n->len; i++) {
            n->value += n->weight[i] * compute(n->in[i]);
        }
        n->value = n->func->f(n->value + n->bias);
    }
    return n->value;
}

int setNetwork(long double * in, Network *n) {
    for (int i = n->inLen; i<n->neuronLen; i++) {
        n->neuron[i].comped = 0;
    }
    for (int i = 0; i<n->inLen; i++) {
        n->in[i]->comped = 1;
        n->in[i]->value = n->in[i]->func->f(in[i]); 
    }
    return 1;
}

int computeNetwork(Network *n) {
    for (int i = 0; i<n->outLen; i++) {
        compute(n->out[i]);
    }
    return 1;
}

int getError(Neuron *n) {
    long double netError = 0;
    for (int i = 0; i<n->outLen; i++) {
        long double w = 0;
        for (int j = 0; j<n->out[i]->len; j++) {
            if (n->out[i]->in[j] == n) w = n->out[i]->weight[j];
        }
        netError += n->out[i]->error * w;
    }
    n->error = n->func->d(n->value) * netError;
    return 1;
}

int gdescent(Neuron *n, float s) {
    for (int i = 0; i < n->len; i++) {
        n->weight[i] -= s * n->in[i]->value * n->error;
    }
    n->bias -= s * n->error;
}

int backprop(long double * vals, Network *n, float s) {
    for (int i = 0; i<n->outLen; i++) {
        n->out[i]->error = n->out[i]->func->d(n->out[i]->value) * (n->out[i]->value - vals[i]);
    }
    for (int i = n->neuronLen - n->outLen - 1; i >= 0; i--) {
        getError(&n->neuron[i]);
    }
    for (int i = 0; i < n->neuronLen; i++) {
        gdescent(&n->neuron[i], s);
    }
    return 1;
}

long double *encodeChar(char c) {
    char * possibleChars;
    possibleChars = malloc(sizeof(char) * 28);
    strcpy(possibleChars, " abcdefghijklmnopqrstuvwxyz\n");
    long double * out = malloc(sizeof(long double) * 28);
    for (int i = 0; i<28; i++) {
        if (c == possibleChars[i]) {
            out[i] = 1;
        } else {
            out[i] = 0;
        }
    }
    free(possibleChars);
    return out;
}

char decodeChar(long double *in) {
    char * possibleChars;
    possibleChars = malloc(sizeof(char) * 28);
    strcpy(possibleChars, " abcdefghijklmnopqrstuvwxyz\n");
    long double * out = malloc(sizeof(long double) * 28);
    int max = 0;
    for (int i = 0; i<28; i++) {
        if (in[i] > in[max]) {
	    max = i;
	}
    }
    char c = possibleChars[max];
    free(possibleChars);
    return c;
}

long double *oneHot(char c, int *l) {
    char * possibleChars;
    *l = 66;
    possibleChars = malloc(sizeof(char) * *l);
    strcpy(possibleChars, " ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz,.?!;:-_()\'\n\r");
    long double * out = malloc(sizeof(long double) * *l);
    for (int i = 0; i<*l; i++) {
        if (c == possibleChars[i]) {
            out[i] = 1;
        } else {
            out[i] = 0;
        }
    }
    free(possibleChars);
    return out;
}

char hotOne(long double * d) {
    char * possibleChars;
    int l = 66;
    possibleChars = malloc(sizeof(char) * l);
    strcpy(possibleChars, " ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz,.?!;:-_()\'\n\r");
    int max = 0;
    for (int i = 0; i<l; i++) {
        if (d[i] > d[max]) {
            max = i;
        }
    }
    char c = possibleChars[max];
    free(possibleChars);
    return c;
}

char * inportText(char * fileName, int * l) {
    FILE *fp;

    fp = fopen(fileName, "r");

    int len = 0;

    char ch;

    while((ch = fgetc(fp)) != EOF) {
        len++;
    }

    char * t = malloc(sizeof(char) * len);
    int c = 0;

    rewind(fp);

    while((ch = fgetc(fp)) != EOF) {
        t[c] = ch;
        c++;
    }

    t[c] = '\0';

    fclose(fp);

    *l = len + 1;
    return t;
}
