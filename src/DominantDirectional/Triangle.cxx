/*
 *	By Yinpeng Li, mousquetaires@unc.edu
 */

#include "Triangle.h"
#include "Geometry_Common.h"
#include <iomanip>

namespace fiberodf
{

TriangleBarycentricCoords::TriangleBarycentricCoords(const double u, const double v)
{
  w2 = u;
  w3 = v;
  w1 = 1 - w2 - w3;
}

std::ostream & operator<<(std::ostream & os, const TriangleBarycentricCoords & baryCentric)
{
  os << "Barycentric: (" << std::fixed << baryCentric.w1 << ", " << baryCentric.w2 << ", " << baryCentric.w3 << ")";
  return os;
}

Triangle::Triangle(const Point & p1, const Point & p2, const Point & p3)
{
  this->m_p1 = p1;
  this->m_p2 = p2;
  this->m_p3 = p3;
}

bool Triangle::intersect(const Point & raySource, const Point & pointOnRay, Point & intersectPoint) const
{
  if( raySource == pointOnRay )
    {
    return false;
    }
  Vector direction(raySource, pointOnRay);
  return intersect(raySource, direction, intersectPoint);
}

bool Triangle::intersect(const Point & raySource, const Vector & direction, Point & intersectPoint) const
{
  Vector direc = direction;

  direc.normalize();
  Vector norm = normal();
  if( direc * norm == 0 )  // The ray is parallel to the plane
    {
    return false;
    }
  // Turn the ray function into: raySource + t * direc
  const double t = norm * Vector(raySource, m_p1) / (norm * direc);

  if( t < 0 )
    {
    return false;
    }
  else
    {
    Point planeIntersect = raySource + direc * t;

    // Judge if the intersection point with the plane is inside of the triangle
    // Barycentric Coordinate is used
    // If w1 or w2 or w3 < 0 then point is in the wrong direction and must be outside the triangle

    TriangleBarycentricCoords baryCoord = barycentric(planeIntersect);

    if( (baryCoord.w1 >= -EPSILON) && (baryCoord.w2 >= -EPSILON) && (baryCoord.w3 >= -EPSILON) )
      {
      intersectPoint = planeIntersect;
      return true;
      }
    else
      {
      return false;
      }
    }
}

Vector Triangle::normal() const
{
  Vector n = crossProduct(Vector(m_p1, m_p2), Vector(m_p1, m_p3) );

  n.normalize();
  return n;
}

TriangleBarycentricCoords Triangle::barycentric(const Point & p) const
{
  // Local 2D coordinate system is used--p1 is the origin
  // Let intersect point be P = p1 + u(p2 - p1) + v(p3 - p1), where u and v are real numbers
  Vector p1P(m_p1, p);
  Vector p1p2(m_p1, m_p2);
  Vector p1p3(m_p1, m_p3);

  // p1P = u * p1p2 + v * p1p3
  const double u =
    ( (p1P
       * p1p2)
     * (p1p3 * p1p3) - (p1P * p1p3) * (p1p2 * p1p3) ) / ( (p1p2 * p1p2) * (p1p3 * p1p3) - (p1p2 * p1p3) * (p1p2 * p1p3) );
  const double v =
    ( (p1P
       * p1p3)
     * (p1p2 * p1p2) - (p1P * p1p2) * (p1p2 * p1p3) ) / ( (p1p2 * p1p2) * (p1p3 * p1p3) - (p1p2 * p1p3) * (p1p2 * p1p3) );

  return TriangleBarycentricCoords(u, v);
}

}
