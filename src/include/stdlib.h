#ifndef STDLIB_H
#define STDLIB_H

#define NULL 0
typedef enum { false, true } bool;

typedef unsigned int size_t;

#define sgn(x) ((x<0)?-1:((x>0)?1:0))
#define abs(N) ((N<0)?(-N):(N))

#endif
