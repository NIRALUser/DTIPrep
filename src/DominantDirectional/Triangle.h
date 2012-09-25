#ifndef TRIANGLE_H
#define TRIANGLE_H

/*
 *	By Yinpeng Li, mousquetaires@unc.edu
 */

#include "Point.h"
#include "Vector.h"

namespace fiberodf{

struct TriangleBarycentricCoords{
	double w1, w2, w3 ;
	TriangleBarycentricCoords(const double u, const double v) ;
} ;

std::ostream &operator<<(std::ostream &os, const TriangleBarycentricCoords &baryCentric) ;

class Triangle{
protected:
	Point m_p1, m_p2, m_p3 ;
public:
	Triangle(const Point &p1, const Point &p2, const Point &p3) ;
	Vector normal() const ;
	bool intersect(const Point &raySource, const Point &pointOnRay, Point &intersectPoint) const ;
	bool intersect(const Point &raySource, const Vector &direction, Point &intersectPoint) const ;
	TriangleBarycentricCoords barycentric(const Point &p) const ;
} ;

}
#endif
