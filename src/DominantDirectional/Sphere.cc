/*
 *	By Yinpeng Li, mousquetaires@unc.edu
 */

#include "Sphere.h"
#include <cmath>

namespace fiberodf{

Sphere::Sphere(const Point &center, const double radius){
	this->center = center ;
	this->radius = radius ;
}

bool Sphere::intersect(const Point &raySource, const Point &pointOnRay, Point &intersectPoint) const{
	if (raySource == pointOnRay){
		return false ;
	}
	Vector direction(raySource, pointOnRay) ;
	return intersect(raySource, direction, intersectPoint) ;
}

bool Sphere::intersect(const Point &raySource, const Vector &direction, Point &intersectPoint) const{
	Vector direc = direction ;
	direc.normalize() ;
	//Turn the ray function into: raySource + t * direc
	//Turn the intersection function into a * sqr(t) + b * t + c = 0 => sqr(|S + t * v - C|) = sqr(r)
	//=>sqr(tv) + 2tv(S - C) + sqr(S - C) - sqr(r)
	//a = 1
	double b = 2 * direc * Vector(center, raySource) ;
	double c = (Vector(center, raySource)).magnitudeSquare() - radius * radius ;

	if ((b * b - 4 * c) < 0)
		return false ;
	else if ((b * b - 4 * c) == 0){
		double t = - b / 2 ;
		if (t < 0){
			return false ;
		}
		else {
			intersectPoint = raySource + t * direc ;
			return true ;
		}
	}
	else{
		double t1 = (- b - sqrt(b * b - 4 * c)) / 2 ;
		double t2 = (- b + sqrt(b * b - 4 * c)) / 2 ;
		if (t2 < 0){
			return false ;
		}
		else if (t1 < 0){
			intersectPoint = raySource + t2 * direc ;
			return true ;
		}
		else{
			intersectPoint = raySource + t1 * direc ;
			return true ;
		}
	}
}

Vector Sphere::normal(const Point &pointOnSurface) const{
	Vector n(center, pointOnSurface) ;
	n.normalize() ;
	return n ;
}

}
