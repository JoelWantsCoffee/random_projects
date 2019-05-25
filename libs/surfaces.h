#define sign(a) (((a)<(0))?(-1):(1))
#define MAX(a,b) (((a)>(b))?(a):(b))
#define MIN(a,b) (((a)<(b))?(a):(b))

//structs
typedef struct _colour {
    int r;
    int g;
    int b;
} colour;

typedef struct _surface {
    colour lastCol;
    int width;
    int height;
    size_t pixelsLength;
    char cols [9];
    int colLen; 
    colour * pixels;
} surface;

typedef struct _dvec2d {
    int x;
    int y;
} dvec2d;

//functions
float lerp(float a, float b, float dist) {
    return (a*(1 - dist) + b*dist);
}

void initSurf(surface *surf, int w, int h) {
    strcpy(surf->cols, " .,>xX#@@");
    surf->colLen = 9;
    surf->pixelsLength = w*h;
    surf->width = w;
    surf->height = h;
    surf->pixels = malloc(w * h * sizeof(colour));
}  

void freeSurf(surface *surf) {
    free(surf->pixels);
}

int eq(colour a, colour b) {
    return ((a.r == b.r) && (a.g == b.g) && (a.b == b.b)); 
}

void draw(surface surf) {
    for (int j = 0; j<surf.height; j++) {
        for (int i = 0; i<surf.width; i++) {
            colour col = surf.pixels[i + j*surf.width];
            //float tone = MIN(MAX( ((float)fpcol.r + col.g + col.b)/(3*256), 0), 255);	

            
            if (!(eq(col, surf.pixels[i + j*surf.width - 1]))) {
                fprintf(stdout, "\033[48;2;%d;%d;%dm ", col.r, col.g, col.b);
            } else {
                fprintf(stdout, " ");
            }



            //sprintf(buff + c, "%s", temp);

            // sprintf(buff + c, "%s", temp);

	        //putchar(surf.cols[(int) floor(tone*(surf.colLen - 1)) ]);
        }
        fprintf(stdout, "\n");

        // c++;
        //sprintf(buff + c, "\n");
    }
    fflush(stdout);
    //printf("%s", buff);
} 

int getIndex(surface *surf, int x, int y) {
    return x + y*surf->width;
}


colour cNeg(colour c) {
    colour out = {
	.r = -c.r,
	.g = -c.g,
	.b = -c.b,
    };
    return out;
}

void cPlus(colour *c, colour col) {
    c->r += col.r;
    c->g += col.b;
    c->b += col.g;
}

colour mColour(int);

colour cMult(colour c, float a) {
    colour out = {
        .r = c.r * a,
        .g = c.g * a,
        .b = c.b * a,	
    };
    return out;
}

colour cLerp(colour c1, colour c2, float d) {
    d = MAX(MIN(d, 1), 0);
    colour out = {
        .r = lerp(c1.r, c2.r, d),
        .g = lerp(c1.g, c2.g, d),
        .b = lerp(c1.b, c2.b, d),
    };
    return out;
}

colour cInt(int a) {
    colour out = {
        .r = (a & 0xFF0000) >> 8*2,
        .g = (a & 0x00FF00) >> 8,
        .b = (a & 0x0000FF),
    };
    return out;
}

int intC(colour c) {
    return (((c.r & 0x0000FF) << 8*2) | ((c.g & 0x0000FF) << 8) | (c.b & 0x0000FF));
}

void drawStipple(surface surf, int d) { 
    surface td;
    initSurf(&td, surf.width, surf.height);

    for (int i = 0; i<surf.pixelsLength; i++) td.pixels[i] = surf.pixels[i];

    for (int j = 0; j<surf.height; j ++) {
        for (int i = 0; i<surf.width;  i++) {
            colour col = td.pixels[getIndex(&td, i, j)];
	
	    int pCols = d;

            colour aCol = {
		//round((surf.colLen-1) * gCol / 255) * 255/(surf.colLen-1);
		.r = round((pCols - 1) * col.r / 255) * 255/(pCols - 1),
		.g = round((pCols - 1) * col.g / 255) * 255/(pCols - 1),
		.b = round((pCols - 1) * col.b / 255) * 255/(pCols - 1),
	    };
	    

            colour error = col;
	    
	    cPlus(&error, cNeg(aCol));
            
            td.pixels[getIndex(&td, i, j)] = aCol;

            cPlus(&td.pixels[ getIndex(&td, MIN(i + 1, td.width-1)  , j				) ], cMult(error , 7.0 / 16.0));
            cPlus(&td.pixels[ getIndex(&td, MAX(i - 1, 0)	    , MIN(j + 1, td.height-1)	) ], cMult(error , 3.0 / 16.0));
            cPlus(&td.pixels[ getIndex(&td, i			    , MIN(j + 1, td.height-1)	) ], cMult(error , 5.0 / 16.0));
            cPlus(&td.pixels[ getIndex(&td, MIN(i + 1, td.width-1)  , MIN(j + 1, td.height-1)	) ], cMult(error , 1.0 / 16.0));
        }
    } 
    draw(td);
    freeSurf(&td);
}

void setCol(surface *surf, colour col) {
    surf->lastCol = col;
}

void set(surface *surf, int x, int y) {
    if ((x >= 0) && (y >= 0) && (x < surf->width) && (y < surf->height)) {
        surf->pixels[(x + (y * surf->width))] = surf->lastCol;
    }
}

void line(surface *surf, float x1, float y1, float x2, float y2) {
    float xmove = (x2 - x1);
    float ymove = (y2 - y1);

    int dist;

    if ( abs(xmove) > abs(ymove)) {
        ymove = ymove / abs(xmove);
        dist = (int) xmove;
        xmove = sign(xmove);
    } else {
        xmove = xmove / abs(ymove);
        dist = (int) ymove;
        ymove = sign(ymove);
    }

    float tx = x1;
    float ty = y1;

    for (int i = 0; i<abs(dist); i++) {
        set(surf, tx, ty);
        tx += xmove;
        ty += ymove;
    }
}

void triangle(surface *surf, float x1, float y1, float x2, float y2, float x3, float y3) {
    line(surf, x1, y1, x2, y2);
    line(surf, x2, y2, x3, y3);
    line(surf, x3, y3, x1, y1);
}

void fillTriangle(surface *surf, float x1, float y1, float x2, float y2, float x3, float y3) {
    dvec2d v1 = {
        .x = (int) x1,
        .y = (int) y1,
    };
    dvec2d v2 = {
        .x = (int) x2,
        .y = (int) y2,
    };
    dvec2d v3 = {
        .x = (int) x3,
        .y = (int) y3,
    };

    dvec2d *miny = &v1;
    dvec2d *maxy = &v1;
    dvec2d *midy = &v1;

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

    for (int y = miny->y; y < midy->y; y++) {
        float p1 = ((float) y - miny->y) / (midy->y - miny->y);
        float p2 = ((float) y - miny->y) / (maxy->y - miny->y);

        int x1 = (int) lerp(miny->x, midy->x, p1);
        int x2 = (int) lerp(miny->x, maxy->x, p2);
        
        for (int x = MIN(x1, x2); x<MAX(x1, x2); x++) {
            set(surf, x, y);
        }
    } 

    for (int y = midy->y; y < maxy->y; y++) {
        float p1 = ((float) y - midy->y) / (maxy->y - midy->y);
        float p2 = ((float) y - miny->y) / (maxy->y - miny->y);
        
        int x1 = (int) lerp(midy->x, maxy->x, p1);
        int x2 = (int) lerp(miny->x, maxy->x, p2);
        
        for (int x = MIN(x1, x2); x<MAX(x1, x2); x++) {
            set(surf, x, y);	    
        }
    }
}

void background(surface *surf, colour col) {
    for (int i = 0; i<surf->pixelsLength; i++) {
        surf->pixels[i] = col;
    }
}

colour fColour(int r, int g, int b) {    
    r = MAX(MIN(r, 255), 0);    
    g = MAX(MIN(g, 255), 0);    
    b = MAX(MIN(b, 255), 0);    
    colour col = {
        .r = r,
        .g = g,
        .b = b,
    };
    return col;
}

colour mColour(int v) {
    colour col = {
        .r = v,
        .g = v,
        .b = v,
    };
    return col;
}

