#ifndef POINT_H
#define POINT_H

/*
 *	By Yinpeng Li, mousquetaires@unc.edu
 */

#include <iostream>

namespace fiberodf{

class Point{
protected:
	double m_x;
	double m_y;
	double m_z;
public:
	explicit Point(const double x = 0, const double y = 0, const double z = 0) ;
	double getX() const {return m_x ;}
	double getY() const {return m_y ;}
	double getZ() const {return m_z ;}
	double operator[](const int index) const ;
	double &getRef(const int index) ;
} ;

std::ostream &operator<<(std::ostream &os, const Point &p) ;

class Vector ;
Point operator+(const Point &p, const Vector &v) ;
bool operator==(const Point &p1, const Point &p2) ;
double distance(const Point &p1, const Point &p2) ;

}
#endif
