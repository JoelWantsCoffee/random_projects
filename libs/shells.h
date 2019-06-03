//structs
typedef struct _input {
    char * keys;
    int on [8];
} input;

typedef struct _shell {
    void (*fp)(input*, int, void **);
    input ins;
    int flags;
} shell;

//functions
void initShell(shell *in, void (*fp)(input *, int, void **)) {
    in->fp = fp;
    in->ins.keys = ("wasdijkl");
    in->flags = 0;
}

void run(shell *f, int frameCount, void ** arg, float frameRate) {
    clock_t mt;
    mt = clock();
    clock_t t;
    t = clock();
    f->fp(&(f->ins), frameCount, arg);
    while ( ((double)t - (double)mt)/CLOCKS_PER_SEC < 1/frameRate ) {
	    t = clock();
    }
}

void liveInputs(shell *sh) {
    if (sh->flags & 1) system("xset r rate 10 100");
    static struct termios told, tnew;
    tcgetattr( STDIN_FILENO, &told);
    tnew = told;
    tnew.c_lflag &= ~(ICANON | ECHO);
    tcsetattr( STDIN_FILENO, TCSANOW, &tnew);
}

void* getIns(input *in) {
    char b[5];
    b[0] = ' ';
    int size = sizeof(in->on);
    read(STDIN_FILENO, b, 5);
    for (int i = 0; i<size; i++) {
        if (b[0] == in->keys[i]) {
            in->on[i] = 1;
        } else {
            in->on[i] = 0;
        }
    }
    fflush(stdout);
}

void resetKeys() {
    static struct termios t;
    tcgetattr( STDIN_FILENO, &t);
    t.c_lflag |= (ICANON | ECHO);
    tcsetattr( STDIN_FILENO, TCSANOW, &t);
    system("xset r rate 220 20");
}

void clearIns(input *in) {
    for (int i = 0; i<sizeof(in->on); i++) in->on[i] = 0;
}
