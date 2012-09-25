/*
 *	By Yinpeng Li, mousquetaires@unc.edu
 */

#include "Point.h"
#include "Vector.h"
#include "Geometry_Common.h"
#include <cstdlib>
#include <iomanip>

namespace fiberodf{

Point::Point(const double x, const double y, const double z){
	this->x = x ;
	this->y = y ;
	this->z = z ;
}

double Point::operator[](const int index) const{
  switch (index){
  case X:
    return x ;
    break ;
  case Y:
    return y ;
    break ;
  case Z:
    return z ;
    break ;
  default:
    std::cout << "Index out of range!" << std::endl ;
    exit(-1) ;
    break ;
  } ;
}

double &Point::getRef(const int index){
     switch (index){
  case X:
    return x ;
    break ;
  case Y:
    return y ;
    break ;
  case Z:
    return z ;
    break ;
  default:
    std::cout << "Index out of range!" << std::endl ;
    exit(-1) ;
    break ;
  } ; 
}

std::ostream &operator<<(std::ostream &os, const Point &p){
	os << "Point: (" << std::fixed << p.getX() << ", " << p.getY() << ", " << p.getZ() << ")" ;
	return os ;
}

Point operator+(const Point &p, const Vector &v){
	return Point(p.getX() + v.getX(), p.getY() + v.getY(), p.getZ() + v.getZ()) ;
}

bool operator==(const Point &p1, const Point &p2){
	return p1.getX() == p2.getX() && p1.getY() == p2.getY() && p1.getZ() == p2.getZ() ;
}

double distance(const Point &p1, const Point &p2){
	return Vector(p1, p2).magnitude() ;
}

}
