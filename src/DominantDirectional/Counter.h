#ifndef COUNTER_H
#define COUNTER_H

/*
 *	By Yinpeng Li, mousquetaires@unc.edu
 */

#include "SphereIkosahedron.h"
#include "Vector.h"

namespace fiberodf
{

typedef itk::SphereIkosahedron<double> SphereIkosahedronType;

typedef double AccumulateType;

// *******************************************************************************

class CounterSerializer
{
public:
  virtual void SerializeBins(std::ostream & os, const std::vector<AccumulateType> & bins) const = 0;

};

class CounterSerializer_TXT : public CounterSerializer    // Text format serializer
{
public:
  void SerializeBins(std::ostream & os, const std::vector<AccumulateType> & bins) const;

};

class CounterSerializer_BIN : public CounterSerializer    // Binary format serializer
{
public:
  void SerializeBins(std::ostream & os, const std::vector<AccumulateType> & bins) const;

};

// *******************************************************************************

class Counter
{
public:

  static void Initialize(const short subdivisionLevel);

  Counter(CounterSerializer & s);

  virtual void Add(Vector direction, const double weight = 1) = 0;

  void Serialize(std::ostream & os) const;

  int GetSize() const;

  std::vector<double> GetFrequency() const;

  vtkSmartPointer<vtkPolyData> GetVTKPolyData() const;

  void WriteCounterToVTKFile(const char *fname = "Counter.vtk") const;

  std::vector<AccumulateType> Getbins();

  virtual ~Counter()
  {
  }

protected:

  static SphereIkosahedronType::Pointer icosahedron;
  std::vector<AccumulateType>           bins;
  CounterSerializer &                   serializer;
};

std::ostream & operator<<(std::ostream & os, const Counter & c);

// *******************************************************************************

class Counter_NearestNeighborVertex : public Counter
{
public:
  Counter_NearestNeighborVertex(CounterSerializer & s);

  void Add(Vector direction, const double weight = 1);

  void Printout_m_all_triangs();

  void Calculation_AreaBin();

  double Calculation_SubAreaBin( std::vector<double> Sides, IndexList indices, short Source);

  ~Counter_NearestNeighborVertex()
  {
  }

};

// *******************************************************************************

class Counter_WeightedVertices : public Counter
{
public:
  Counter_WeightedVertices(CounterSerializer & s);

  void Add(Vector direction, const double weight = 1);

  ~Counter_WeightedVertices()
  {
  }

};

}
#endif
