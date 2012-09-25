#ifndef GEOMETRY_COMMON_H
#define GEOMETRY_COMMON_H

/*
 *	By Yinpeng Li, mousquetaires@unc.edu
 */

namespace fiberodf{

enum {X = 0, Y = 1, Z = 2} ;

#define MIN(a, b) (((a) < (b))?(a) : (b))
#define MAX(a, b) (((a) > (b))?(a) : (b))

extern const double EPSILON ;

}
#endif
