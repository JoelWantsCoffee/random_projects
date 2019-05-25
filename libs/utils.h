float map(float a, float min, float max, float nmin, float nmax) {
    return ((a - min)/(max - min))*nmax + nmin;
}