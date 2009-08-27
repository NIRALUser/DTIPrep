
#ifndef _itkLinearHeadEddy3DCorrection_txx
#define _itkLinearHeadEddy3DCorrection_txx

#include "itkNumericTraits.h"
#include "itkLinearHeadEddy3DCorrection.h"
#include "vnl/algo/vnl_matrix_inverse.h"


namespace itk
{

// Constructor with default arguments
template<class TScalarType, unsigned int NInputDimensions,
                            unsigned int NOutputDimensions>
LinearHeadEddy3DCorrection<TScalarType, NInputDimensions, NOutputDimensions>
::LinearHeadEddy3DCorrection()
  : Superclass(OutputSpaceDimension, ParametersDimension)
{
 
  m_Center.Fill( 0 );
 
  this->m_FixedParameters.SetSize ( NInputDimensions );
  this->m_FixedParameters.Fill ( 0.0 );


  //create transform pointer
  m_head_motion_transform = HeadMotionTransformType::New();
  m_eddy_current_transform = EddyCurrentTransformType::New();

}


// Destructor
template<class TScalarType, unsigned int NInputDimensions,
                            unsigned int NOutputDimensions>
LinearHeadEddy3DCorrection<TScalarType, NInputDimensions, NOutputDimensions>
::~LinearHeadEddy3DCorrection()
{
  return;
}



// Print self
template<class TScalarType, unsigned int NInputDimensions,
                            unsigned int NOutputDimensions>
void
LinearHeadEddy3DCorrection<TScalarType, NInputDimensions, NOutputDimensions>
::PrintSelf(std::ostream &os, Indent indent) const
{

  Superclass::PrintSelf(os,indent);

  unsigned int i, j;
  
  os << indent << "Versor Matrix: " << std::endl;
  MatrixType rigid_Matrix = m_head_motion_transform->GetRotationMatrix();

  for (i = 0; i < NInputDimensions; i++) 
    {
    os << indent.GetNextIndent();
    for (j = 0; j < NOutputDimensions; j++)
      {
      os << rigid_Matrix[i][j] << " ";
      }
    os << std::endl;
    }


  os << indent << "Center: " << m_Center << std::endl;
  
  TranslationType rigid_Translation = m_head_motion_transform->GetTranslation();
  os << indent << "Rigid Translation: " << rigid_Translation << std::endl;


  os << indent << "Eddy Current Matrix: " << std::endl;
  MatrixType eddy_Matrix = m_eddy_current_transform->GetMatrix();
  for (i = 0; i < NInputDimensions; i++) 
    {
    os << indent.GetNextIndent();
    for (j = 0; j < NOutputDimensions; j++)
      {
      os << eddy_Matrix[i][j] << " ";
      }
    os << std::endl;
    }
  
  TranslationType eddy_Translation = m_eddy_current_transform->GetTranslation();
  os << indent << "Eddy Current translation: " << eddy_Translation << std::endl;
}



/**  Transform a point  */
template<class TScalarType, unsigned int NInputDimensions,
                            unsigned int NOutputDimensions>
typename LinearHeadEddy3DCorrection<TScalarType,
                               NInputDimensions,
                               NOutputDimensions>::OutputPointType
LinearHeadEddy3DCorrection<TScalarType, NInputDimensions, NOutputDimensions>
::TransformPoint(const InputPointType &point) const 
{

  return  m_eddy_current_transform->TransformPoint( m_head_motion_transform->TransformPoint(point) );
}


/** Get fixed parameters  */
template<class TScalarType, unsigned int NInputDimensions,
                            unsigned int NOutputDimensions>
void
LinearHeadEddy3DCorrection<TScalarType, NInputDimensions, NOutputDimensions>
  ::SetFixedParameters( const ParametersType & fp )
{
  this->m_FixedParameters = fp;
  InputPointType c;
  for ( unsigned int i = 0; i < NInputDimensions; i++ )
    {
    c[i] = this->m_FixedParameters[i];
    }
  this->SetCenter ( c );
}

/** Get the Fixed Parameters. */
template<class TScalarType, unsigned int NInputDimensions,
                            unsigned int NOutputDimensions>
const typename LinearHeadEddy3DCorrection<TScalarType,
                                     NInputDimensions,
                                     NOutputDimensions>::ParametersType &
LinearHeadEddy3DCorrection<TScalarType, NInputDimensions, NOutputDimensions>
  ::GetFixedParameters(void) const
{
  this->m_FixedParameters.SetSize ( NInputDimensions );
  for ( unsigned int i = 0; i < NInputDimensions; i++ )
    {
    this->m_FixedParameters[i] = this->m_Center[i];
    }
  return this->m_FixedParameters;
}



/** Get parameters */
template<class TScalarType, unsigned int NInputDimensions,
                            unsigned int NOutputDimensions>
const typename LinearHeadEddy3DCorrection<TScalarType,
                                     NInputDimensions,
                                     NOutputDimensions>::ParametersType &
LinearHeadEddy3DCorrection<TScalarType, NInputDimensions, NOutputDimensions>
::GetParameters( void ) const
{

  HeadMotionTransformTypeParametersType head_motion_parameter = m_head_motion_transform->GetParameters();
  EddyCurrentTransformTypeParametersType eddy_current_parameter = m_eddy_current_transform->GetParameters();

  unsigned int par = 0;

  //Get Rigid Transformation Part Parameters
  for(unsigned int i=0; i<head_motion_parameter.Size(); i++)
  {
      this->m_Parameters[par] = head_motion_parameter[i];
      ++par;
  }

  //Get Eddy Current Transformation Part Parameters
  for(unsigned int i=0; i<eddy_current_parameter.Size(); i++) 
  {
    this->m_Parameters[par] = eddy_current_parameter[i];
    ++par;
  }

  return this->m_Parameters;
}

/**/
template<class TScalarType, unsigned int NInputDimensions,
                            unsigned int NOutputDimensions>
const typename LinearHeadEddy3DCorrection<TScalarType,
                                     NInputDimensions,
                                     NOutputDimensions>::RotationMatrixType &
LinearHeadEddy3DCorrection<TScalarType, NInputDimensions, NOutputDimensions>
::GetHeadMotionRotationMatrix(void) const
{
  return  m_head_motion_transform->GetRotationMatrix();
}
/**/

/** Set parameters */
template<class TScalarType, unsigned int NInputDimensions,
                            unsigned int NOutputDimensions>
void
LinearHeadEddy3DCorrection<TScalarType, NInputDimensions, NOutputDimensions>
::SetParameters( const ParametersType & parameters )
{
  HeadMotionTransformTypeParametersType head_motion_parameter = m_head_motion_transform->GetParameters();
  EddyCurrentTransformTypeParametersType eddy_current_parameter = m_eddy_current_transform->GetParameters();

  unsigned int par = 0;

  this->m_Parameters = parameters;

  //Set Rigid Transformation Part Parameters
  for(unsigned int i=0; i<head_motion_parameter.Size(); i++)
  {
      head_motion_parameter[i] = this->m_Parameters[par];
      ++par;
  }

  m_head_motion_transform->SetParameters(head_motion_parameter);
  m_head_motion_transform->SetCenter(m_Center);

  //Set Eddy Current Transformation Part Parameters
  for(unsigned int i=0; i<eddy_current_parameter.Size(); i++) 
  {
    eddy_current_parameter[i] = this->m_Parameters[par];
    ++par;
  }

  m_eddy_current_transform->SetParameters(eddy_current_parameter);
  m_eddy_current_transform->SetCenter(m_Center);
}


// Compute the Jacobian in one position 
template<class TScalarType, unsigned int NInputDimensions,
                            unsigned int NOutputDimensions>
const typename LinearHeadEddy3DCorrection<TScalarType, NInputDimensions, NOutputDimensions>::JacobianType & 
LinearHeadEddy3DCorrection<TScalarType, NInputDimensions, NOutputDimensions>
::GetJacobian( const InputPointType & p ) const
{

  this->m_Jacobian.Fill( 0.0 );

  const InputVectorType v0 = p - this->GetCenter(); //v0 is for head motion transformation Jacobian

  const InputPointType v1 = m_head_motion_transform->TransformPoint(p); //v1 is after head motion transformation
    
  const InputVectorType v2 = v1 - this->GetCenter(); //v2 is for eddy current transformation Jacobian

  EddyCurrentTransformTypeParametersType eddy_current_parameter = m_eddy_current_transform->GetParameters();
  //const double c0 = eddy_current_parameter[3];  //y axis global translation
  const double c1 = eddy_current_parameter[0];  //x axis shear
  const double c2 = eddy_current_parameter[1];  //y axis scale
  const double c3 = eddy_current_parameter[2];  //z axis shear


  //The following is for versor Jacobian

  // compute derivatives with respect to rotation
  const HeadMotionTransformValueType vx = m_head_motion_transform->GetVersor().GetX();
  const HeadMotionTransformValueType vy = m_head_motion_transform->GetVersor().GetY();
  const HeadMotionTransformValueType vz = m_head_motion_transform->GetVersor().GetZ();
  const HeadMotionTransformValueType vw = m_head_motion_transform->GetVersor().GetW();

  const double px = v0[0];
  const double py = v0[1];
  const double pz = v2[2];

  const double vxx = vx * vx;
  const double vyy = vy * vy;
  const double vzz = vz * vz;
  const double vww = vw * vw;

  const double vxy = vx * vy;
  const double vxz = vx * vz;
  const double vxw = vx * vw;

  const double vyz = vy * vz;
  const double vyw = vy * vw;

  const double vzw = vz * vw;


  // compute Jacobian with respect to quaternion parameters
  double J11 = 2.0 * (               (vyw+vxz)*py + (vzw-vxy)*pz)
                         / vw;
  double J21 = 2.0 * ((vyw-vxz)*px   -2*vxw   *py + (vxx-vww)*pz) 
                         / vw;
  double J31 = 2.0 * ((vzw+vxy)*px + (vww-vxx)*py   -2*vxw   *pz) 
                         / vw;

  double J12 = 2.0 * ( -2*vyw  *px + (vxw+vyz)*py + (vww-vyy)*pz) 
                         / vw;
  double J22 = 2.0 * ((vxw-vyz)*px                + (vzw+vxy)*pz) 
                         / vw;
  double J32 = 2.0 * ((vyy-vww)*px + (vzw-vxy)*py   -2*vyw   *pz) 
                         / vw;

  double J13 = 2.0 * ( -2*vzw  *px + (vzz-vww)*py + (vxw-vyz)*pz) 
                         / vw;
  double J23 = 2.0 * ((vww-vzz)*px   -2*vzw   *py + (vyw+vxz)*pz) 
                         / vw;
  double J33 = 2.0 * ((vxw+vyz)*px + (vyw-vxz)*py               ) 
                         / vw;

  //Fill the Jacobian Matrix
  this->m_Jacobian[0][0] = J11;	// x /Rx 
  this->m_Jacobian[0][1] = J12;	// x /Ry
  this->m_Jacobian[0][2] = J13;	// x /Rz  
  this->m_Jacobian[0][3] = 1.0;	// x /Tx 


  this->m_Jacobian[1][0] = c1 * J11 + c2 * J21 + c3 * J31;	// y /Rx 
  this->m_Jacobian[1][1] = c1 * J12 + c2 * J22 + c3 * J32;	// y /Ry
  this->m_Jacobian[1][2] = c1 * J13 + c2 * J23 + c3 * J33;	// y /Rz  
  this->m_Jacobian[1][3] = c1;					// y /Tx
  this->m_Jacobian[1][4] = c2;					// y /Ty
  this->m_Jacobian[1][5] = c3;					// y /Tz
  this->m_Jacobian[1][6] = v2[0];				// y /c1
  this->m_Jacobian[1][7] = v2[1];				// y /c2
  this->m_Jacobian[1][8] = v2[2];				// y /c3
  this->m_Jacobian[1][9] = 1.0;					// y /c0

  this->m_Jacobian[2][0] = J31; //z /Rx
  this->m_Jacobian[2][1] = J32; //z /Ry
  this->m_Jacobian[2][2] = J33; //z /Rz
  this->m_Jacobian[2][5] = 1.0; //z /Tz

  return this->m_Jacobian;

}


} // namespace

#endif

