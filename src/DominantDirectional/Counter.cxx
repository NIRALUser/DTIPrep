/*
 *	By Yinpeng Li, mousquetaires@unc.edu
 */

#include "Counter.h"
#include "Triangle.h"
#include <cassert>
#include <iostream>
#include <iomanip>
#include <vtkPointData.h>
#include <vtkDoubleArray.h>

namespace fiberodf
{

void CounterSerializer_TXT::SerializeBins(std::ostream & os, const std::vector<AccumulateType> & bins) const
{
  os << std::fixed;
  for( unsigned int i = 0; i < bins.size(); i++ )
    {
    os << bins[i];
    if( i % 4 == 3 )
      {
      os << std::endl;
      }
    else
      {
      os << '\t';
      }
    }
  if( bins.size() % 4 != 0 )
    {
    os << std::endl;
    }
}

void CounterSerializer_BIN::SerializeBins(std::ostream & os, const std::vector<AccumulateType> & bins) const
{
  os.write(reinterpret_cast<const char *>(&bins[0]), sizeof(AccumulateType) * bins.size() );
}

// *******************************************************************************

SphereIkosahedronType::Pointer Counter::icosahedron = SphereIkosahedronType::New();

void Counter::Initialize(const short subdivisionLevel)
{

  if( subdivisionLevel < 0 )
    {
    std::cout << "The subdivision level must be at least 0!: FAILURE" << std::endl;
    std::cout << "FAILURE IN:" <<  __FILE__ << " at " <<  __LINE__ << std::endl;
    exit(1);
    }

  Counter::icosahedron->SetSubdivisionLevel(subdivisionLevel);
  Counter::icosahedron->Initialize();
}

Counter::Counter(CounterSerializer & s) : serializer(s)
{
  bins.resize(GetSize() );
  for( int i = 0; i < GetSize(); i++ )
    {
    bins[i] = 0;
    }
}

int Counter::GetSize() const
{
  return icosahedron->GetNumberOfVertices();
}

std::vector<double> Counter::GetFrequency() const
{
  std::vector<double> frequency(bins.size() );
  double              total = 0;
  for( unsigned int i = 0; i < frequency.size(); i++ )
    {
    total += bins[i];
    frequency[i] = 0;
    }
  if( total > 0 )
    {
    for( unsigned int i = 0; i < frequency.size(); i++ )
      {
      frequency[i] = bins[i] / total;
      }
    }
  return frequency;
}

vtkSmartPointer<vtkPolyData> Counter::GetVTKPolyData() const
{
  vtkSmartPointer<vtkPolyData>  poly = icosahedron->CreateVTKPolyData();
  vtkSmartPointer<vtkPointData> pointData = poly->GetPointData();

  vtkSmartPointer<vtkDoubleArray> scalars = vtkSmartPointer<vtkDoubleArray>::New();
  scalars->SetName("Frequencies");
  scalars->SetNumberOfComponents(1);
  std::vector<double> frequencies = GetFrequency();
  scalars->SetNumberOfValues(static_cast<vtkIdType>(frequencies.size() ) );
  for( unsigned int i = 0; i < frequencies.size(); i++ )
    {
    scalars->SetValue(static_cast<vtkIdType>(i), frequencies[i]);
    }

  pointData->SetScalars(scalars);

  return poly;
}

void Counter::WriteCounterToVTKFile(const char *fname) const
{
  vtkSmartPointer<vtkPolyData> data = GetVTKPolyData();

  vtkSmartPointer<vtkPolyDataWriter> writer = vtkSmartPointer<vtkPolyDataWriter>::New();
  writer->SetInput(data);
  writer->SetFileName(fname);
  std::stringstream ss;
  ss << "Counter";
  writer->SetHeader(ss.str().c_str() );
  writer->Write();
}

std::vector<AccumulateType> Counter::Getbins()
{
  return bins;
}

void Counter::Serialize(std::ostream & os) const
{
  serializer.SerializeBins(os, bins);
}

std::ostream & operator<<(std::ostream & os, const Counter & c)
{
  c.Serialize(os);
  return os;
}

// **************************************************************************

Counter_NearestNeighborVertex::Counter_NearestNeighborVertex(CounterSerializer & s) : Counter(s)
{
}

void Counter_NearestNeighborVertex::Add(Vector direction, const double weight)
{
  if( direction.isZero() )
    {
    return;
    }
  else
    {
    direction.normalize();
    }

  double temp = -1;
  short  nearest = -1;
  for( short i = 0; i < GetSize(); i++ )
    {
    VectorType vertex = icosahedron->GetCoordinateTableatIndex(i);
    assert(vertex.size() == 3);
    Vector v_vertex(vertex[0], vertex[1], vertex[2]);
    if( v_vertex * direction > temp )
      {
      nearest = i;
      temp = v_vertex * direction;
      }
    }

  bins[nearest] += 1 * weight;
}

// **************************************************************************

Counter_WeightedVertices::Counter_WeightedVertices(CounterSerializer & s) : Counter(s)
{
}

void Counter_WeightedVertices::Add(Vector direction, const double weight)
{
  if( direction.isZero() )
    {
    return;
    }
  else
    {
    direction.normalize();
    }

  double temp = -1;
  short  nearest = -1;
  for( short i = 0; i < GetSize(); i++ )
    {
    VectorType vertex = icosahedron->GetCoordinateTableatIndex(i);
    assert(vertex.size() == 3);
    Vector v_vertex(vertex[0], vertex[1], vertex[2]);
    if( v_vertex * direction > temp )
      {
      nearest = i;
      temp = v_vertex * direction;
      }
    }

  IndexList surroundingTriangles = icosahedron->GetSurroundingTriangles(nearest);

  assert(surroundingTriangles.size() == 5 || surroundingTriangles.size() == 6);
  for( unsigned int i = 0; i < surroundingTriangles.size(); i++ )
    {
    const std::vector<VectorType> & triangle = icosahedron->GetTriangle(surroundingTriangles[i]);
    assert(triangle.size() == 3);
    Triangle t_triangle(
      Point(triangle[0][0], triangle[0][1], triangle[0][2]),
      Point(triangle[1][0], triangle[1][1], triangle[1][2]),
      Point(triangle[2][0], triangle[2][1], triangle[2][2])
      );

    Point intersectionPoint;

    if( t_triangle.intersect(Point(0, 0, 0), direction, intersectionPoint) )
      {
      IndexList triangleVertices = icosahedron->GetTriangleVertices(surroundingTriangles[i]);
      assert(triangleVertices.size() == 3);
      TriangleBarycentricCoords baryCentric = t_triangle.barycentric(intersectionPoint);
      bins[triangleVertices[0]] += baryCentric.w1 * weight;
      bins[triangleVertices[1]] += baryCentric.w2 * weight;
      bins[triangleVertices[2]] += baryCentric.w3 * weight;
      break;    // Stop the iteration
      }
    }
}

// **************************************************************************

double Counter_NearestNeighborVertex::Calculation_SubAreaBin( std::vector<double> Sides, IndexList indices,
                                                              short Source)
{

  // Short is the index of the vertex for Bin
  // Calculation of diameter of circumcircle of the triangle with Sides a b b
  // acb/(2.sqrt(S(S-a)(S-b)(S-c)))
  // S=(a+b+c)/2

  // Calculate sub area of Bin which contains of two traingles : 1) a ( sqrt (d*d-(a/2)*(a/2)) )/4
  //								2) b ( sqrt (d*d-(b/2)*(b/2)) )/4

  double     diameter;
  double     a = Sides[0];
  double     b = Sides[1];
  double     c = Sides[2];
  short      A1 = -3;
  short      A2 = -3;
  short      A3 = Source;
  double     Semiperimeter;
  VectorType v1, v2, v3;

  Semiperimeter = ( a + b + c ) / 2;
  diameter = ( a * b * c ) / (2 * sqrt(Semiperimeter * (Semiperimeter - a) * (Semiperimeter - b) * (Semiperimeter - c) ) );
  std::cout << "Source " << Source << "diameter " << diameter << std::endl;
  for( short i = 0; i < 3; i++ )
    {
    if( indices[i] != A3 && A1 == -3 )
      {
      A1 = indices[i];
      }
    else if( indices[i] != A3 )
      {
      A2 = indices[i];
      }
    }
  for( short k = 0; k < 3; k++ )
    {
    v3.push_back(icosahedron->GetCoordinateTableatIndex( A3 )[k]);
    }
  for( short k = 0; k < 3; k++ )
    {
    v2.push_back(icosahedron->GetCoordinateTableatIndex( A2 )[k]);
    }
  for( short k = 0; k < 3; k++ )
    {
    v1.push_back(icosahedron->GetCoordinateTableatIndex( A1 )[k]);
    }

  double a_adj = sqrt(
      (double) (v3[0] - v2[0]) * (v3[0] - v2[0]) + (v3[1] - v2[1]) * (v3[1] - v2[1]) + (v3[2] - v2[2]) * (v3[2] - v2[2]) );
  double b_adj = sqrt(
      (double) (v1[0] - v3[0]) * (v1[0] - v3[0]) + (v1[1] - v3[1]) * (v1[1] - v3[1]) + (v1[2] - v3[2]) * (v1[2] - v3[2]) );
  std::cout << "a " << a_adj << " b " << b_adj << std::endl;

  double SubArea1 = (double) a_adj * ( (double) sqrt(diameter * diameter - (a_adj / 2) * (a_adj / 2) ) ) / 4;
  double SubArea2 = (double) b_adj * ( (double) sqrt(diameter * diameter - (b_adj / 2) * (b_adj / 2) ) ) / 4;
  std::cout << "SubArea1 " << SubArea1 << " SubArea2 " << SubArea2 << std::endl;

  double SubArea = SubArea1 + SubArea2;
  return SubArea;

}

// **************************************************************************

void Counter_NearestNeighborVertex::Calculation_AreaBin()
{

  // std::vector < IndexList >	AdjacentVertex;
  std::ofstream outfile;
  std::string   ReportFile = "/biomed-resimg/work/mahshid/Data/Entropy/AreaBins_12.txt";

  outfile.open( ReportFile.c_str() );

  std::vector<double> SideTriangle;
  std::vector<double> AreaBins;
  for( short i = 0; i < icosahedron->GetNumberOfVertices(); i++ )
    {
    double AreaBin = 0;
    for( short j = 0; j < icosahedron->GetNumberOfTriangle(); j++ )
      {
      for( short k = 0; k < 3; k++ )
        {
        if( i == icosahedron->GetTriangleVertices(j)[k] )
          {
          for( short m = 0; m < 3; m++ )
            {
            SideTriangle.push_back( (double) icosahedron->GetTriangleLength(j)[m] );

            }
          outfile << "index " << i << " Triangle " << j << " " << Calculation_SubAreaBin(
            SideTriangle, icosahedron->GetTriangleVertices(j), i ) << std::endl;
          AreaBin = AreaBin + Calculation_SubAreaBin( SideTriangle, icosahedron->GetTriangleVertices(j), i );
          break;
          // AdjacentVertex.push_back( GetTriangleVertices(j) );
          }
        }
      }
    outfile << "index " << i << " AreaBin " << AreaBin << std::endl;
    AreaBins.push_back( AreaBin );
    }
  for( short h = 0; h < icosahedron->GetNumberOfVertices(); h++ )
    {
    outfile << "Bin " << h << ": " <<  AreaBins[h] << std::endl;
    }

  outfile << "Num Triangle" << icosahedron->GetNumberOfTriangle() << std::endl;

}

// **************************************************************************

void Counter_NearestNeighborVertex::Printout_m_all_triangs()
{
  std::ofstream outfile;

  std::string ReportFile = "/biomed-resimg/work/mahshid/Data/Entropy/Triangle_Vectores.txt";

  outfile.open( ReportFile.c_str() );

  outfile << " Triangle points " << std::endl;
  for( short i = 0; i < icosahedron->GetNumberOfTriangle(); i++ )
    {
    outfile << "Triangle" << i << " : " << std::endl;
    for( unsigned int j = 0; j < 3; j++ )
      {
      for( unsigned int k = 0; k < 3; k++ )
        {

        outfile << icosahedron->GetTriangle(i)[j][k] << " ";
        }
      outfile << std::endl;
      }
    }

  outfile << " Triangle vertex indices " << std::endl;
  for( short i = 0; i < icosahedron->GetNumberOfTriangle(); i++ )
    {
    outfile << "Triangle" << i << " : " << std::endl;
    for( unsigned int j = 0; j < 3; j++ )
      {
      outfile << icosahedron->GetTriangleVertices(i)[j] << " ";

      }
    outfile << std::endl;
    }

  outfile << " Triangle lengths " << std::endl;
  for( short i = 0; i < icosahedron->GetNumberOfTriangle(); i++ )
    {
    outfile << "Triangle" << i << " : " << std::endl;
    for( unsigned int j = 0; j < 3; j++ )
      {
      outfile << icosahedron->GetTriangleLength(i)[j] << " ";

      }
    outfile << std::endl;
    }

  outfile.close();
}

}
