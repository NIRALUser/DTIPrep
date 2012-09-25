/*
 *	By Yinpeng Li, mousquetaires@unc.edu
 */

#include "Sphere.h"
#include <cmath>

namespace fiberodf
{

Sphere::Sphere(const Point & center, const double radius)
{
  this->m_center = center;
  this->m_radius = radius;
}

bool Sphere::intersect(const Point & raySource, const Point & pointOnRay, Point & intersectPoint) const
{
  if( raySource == pointOnRay )
    {
    return false;
    }
  Vector direction(raySource, pointOnRay);
  return intersect(raySource, direction, intersectPoint);
}

bool Sphere::intersect(const Point & raySource, const Vector & direction, Point & intersectPoint) const
{
  Vector direc = direction;

  direc.normalize();
  // Turn the ray function into: raySource + t * direc
  // Turn the intersection function into a * sqr(t) + b * t + c = 0 => sqr(|S + t * v - C|) = sqr(r)
  // =>sqr(tv) + 2tv(S - C) + sqr(S - C) - sqr(r)
  // a = 1
  const double b = 2.0 * direc * Vector(m_center, raySource);
  const double c = (Vector(m_center, raySource) ).magnitudeSquare() - m_radius * m_radius;

  if( (b * b - 4.0 * c) < 0.0 )
    {
    return false;
    }
  else if( (b * b - 4.0 * c) == 0.0 )
    {
    const double t = -b * 0.5;
    if( t < 0.0 )
      {
      return false;
      }
    else
      {
      intersectPoint = raySource + t * direc;
      return true;
      }
    }
  else
    {
    const double t1 = (-b - sqrt(b * b - 4.0 * c) ) * 0.5;
    const double t2 = (-b + sqrt(b * b - 4.0 * c) ) * 0.5;
    if( t2 < 0.0 )
      {
      return false;
      }
    else if( t1 < 0.0 )
      {
      intersectPoint = raySource + t2 * direc;
      return true;
      }
    else
      {
      intersectPoint = raySource + t1 * direc;
      return true;
      }
    }
}

Vector Sphere::normal(const Point & pointOnSurface) const
{
  Vector n(m_center, pointOnSurface);

  n.normalize();
  return n;
}

}
