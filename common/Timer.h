// Timer.h
// nativeGraphics

#ifndef TIMER_H
#define TIMER_H

//#include <cstdlib>
#include <sys/time.h>

class Timer {
public:
    Timer() {
        reset();
    }

    void reset() {
        gettimeofday(&start, NULL);
    }

    float getSeconds() {
        struct timeval current;
        gettimeofday(&current, NULL);
        return current.tv_sec - start.tv_sec + (current.tv_usec  - start.tv_usec) / 1000000.0;
    }

private:
    struct timeval start;

};

#endif // TIMER_H
