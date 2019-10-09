#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "../libs/utils.h"
#include "../libs/surfaces.h"

typedef struct _cnum {
  long double a;
  long double b;
} cnum;

cnum cn(long double a, long double b) {
  cnum out = {
    .a = a,
    .b = b,
  };
  return out;
}

cnum square(cnum n) {
  return cn(n.a*n.a - n.b*n.b, 2*n.a*n.b);
}

cnum plus(cnum a, cnum b) {
  return cn(a.a + b.a, a.b + b.b);
}

cnum fx(cnum z, cnum c) {
  return plus(square(z), c);
}

float mag2(cnum n) {
  return ((n.a*n.a) + (n.b*n.b));
}

int main() {
  long double mag = 2;
  long double x = -0.7463;
  long double y = 0.1102;

  surface surf;
  initSurf(&surf, 200, 100);
  surf.flags = 0;
  long double aspect = surf.width / surf.height;
  while (1) {
    int reps = MIN(20 / sqrt(mag), 300) ;
    for (int j = 0; j<surf.height; j++) {
      for (int i = 0; i<surf.width; i++) {
        cnum pl = cn(map(i, 0, surf.width, x - mag, x + mag), map(j, 0, surf.height, y - mag, y + mag));
        cnum temp = pl;
        int k = 0;
        for (k = 0; k<reps; k++) {
          if (mag2(temp) > 16) break;
          temp = fx(temp, pl);
          //printf("%d ", (int) temp.a);
        }
        setCol(&surf, cLerp(fColour(0, 255, 255), fColour(255, 0, 255),  ((float) k)/reps));
        set(&surf, i, j);
      }
    }
    mv(0,0);
    //system("clear");
    draw(&surf);
    mag /= 1.01;
    printf("%d, %Lf\n", reps, mag);
  }
  return 0;
}
