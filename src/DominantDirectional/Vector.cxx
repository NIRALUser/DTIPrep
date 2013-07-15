/*
 *	By Yinpeng Li, mousquetaires@unc.edu
 */

#include "Vector.h"
#include "Point.h"
#include "Geometry_Common.h"
#include <cstdlib>
#include <cmath>
#include <iomanip>

namespace fiberodf
{

Vector::Vector(const double x, const double y, const double z)
{
  this->m_x = x;
  this->m_y = y;
  this->m_z = z;
}

Vector::Vector(const Point & tail, const Point & tip)
{
  m_x = tip.getX() - tail.getX();
  m_y = tip.getY() - tail.getY();
  m_z = tip.getZ() - tail.getZ();
}

bool operator==(const Vector & v1, const Vector & v2)
{
  return v1.getX() == v2.getX() && v1.getY() == v2.getY() && v1.getZ() == v2.getZ();
}

double Vector::magnitude() const
{
  const double squareMagnitude = m_x * m_x + m_y * m_y + m_z * m_z;

  return sqrt(squareMagnitude);
}

double Vector::magnitudeSquare() const
{
  return m_x * m_x + m_y * m_y + m_z * m_z;
}

bool Vector::isZero() const
{
  return 0 == magnitude();
}

void Vector::normalize()
{
  double mag = magnitude();

  if( mag > 0 )
    {
    m_x = m_x / mag;
    m_y = m_y / mag;
    m_z = m_z / mag;
    }
  else
    {
    std::cout << "Zero vector, can not normalize!" << std::endl;
    std::cout << "FAILURE IN:" <<  __FILE__ << " at " <<  __LINE__ << std::endl;
    exit(-1);
    }
}

Vector Vector::operator -() const
{
  return Vector(-m_x, -m_y, -m_z);
}

double Vector::operator[](const int index) const
{
  switch( index )
    {
    case X:
      return m_x;
      break;
    case Y:
      return m_y;
      break;
    case Z:
      return m_z;
      break;
    default:
      std::cout << "Index out of range!" << std::endl;
      std::cout << "FAILURE IN:" <<  __FILE__ << " at " <<  __LINE__ << std::endl;
      exit(-1);
      break;
    }
  ;
}

std::ostream & operator<<(std::ostream & os, const Vector & v)
{
  os << "Vector: (" << std::fixed << v.getX() << ", " << v.getY() << ", " << v.getZ() << ")";
  return os;
}

Vector operator*(const Vector & v, const double factor)
{
  return Vector(v.getX() * factor, v.getY() * factor, v.getZ() * factor);
}

Vector operator*(const double factor, const Vector & v)
{
  return v * factor;
}

Vector operator+(const Vector & v1, const Vector & v2)
{
  return Vector(v1.getX() + v2.getX(), v1.getY() + v2.getY(), v1.getZ() + v2.getZ() );
}

Vector operator-(const Vector & v1, const Vector & v2)
{
  return Vector(v1.getX() - v2.getX(), v1.getY() - v2.getY(), v1.getZ() - v2.getZ() );
}

double operator*(const Vector & v1, const Vector & v2)
{
  return v1.getX() * v2.getX() + v1.getY() * v2.getY() + v1.getZ() * v2.getZ();
}

Vector crossProduct(const Vector & v1, const Vector & v2)
{
  return Vector(  v1.getY() * v2.getZ() - v1.getZ() * v2.getY(),
                  v1.getZ() * v2.getX() - v1.getX() * v2.getZ(),
                  v1.getX() * v2.getY() - v1.getY() * v2.getX() );
}

Vector symmetric(const Vector & a, const Vector & bisector)
{
  //   a    b    c    a,c are tails of r,v. Connect ac, ac intersects n at b, and |ob| = cos(theta)|v|
  //   ^\   ^n  /^
  //  r  \  |  / v
  //      \ | /
  //       \|/
  //        o
  // |n| = 1, namely a unit vector. r = v + (r - v) = v + 2 * (ob - v) = v + 2 * ((n * v)n - v)
  Vector n = bisector;

  n.normalize();
  return 2 * ( (n * a) * n) - a;
}

}
