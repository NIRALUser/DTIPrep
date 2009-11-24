#ifndef __itkLinearHeadEddy3DCorrection_h
#define __itkLinearHeadEddy3DCorrection_h

#include <iostream>

#include "itkMatrix.h"
#include "itkTransform.h"
#include "itkExceptionObject.h"
#include "itkMacro.h"

#include "itkVersorRigid3DTransform.h"
#include "itkLinearEddyCurrentTransform.h"

namespace itk
{
template <
  class TScalarType = double,         // Data type for scalars
  unsigned int NInputDimensions = 3,  // Number of dimensions in the input space
  unsigned int NOutputDimensions = 3>
// Number of dimensions in the output space
class LinearHeadEddy3DCorrection :
  public Transform<TScalarType, NInputDimensions, NOutputDimensions>
  {
public:
  /** Standard typedefs   */
  typedef LinearHeadEddy3DCorrection Self;
  typedef Transform<TScalarType,
    NInputDimensions,
    NOutputDimensions>        Superclass;
  typedef SmartPointer<Self>       Pointer;
  typedef SmartPointer<const Self> ConstPointer;

  /** Run-time type information (and related methods).   */
  itkTypeMacro( LinearHeadEddy3DCorrection, Transform );

  /** New macro for creation of through a Smart Pointer   */
  itkNewMacro( Self );

  /** Dimension of the domain space. */
  itkStaticConstMacro(InputSpaceDimension, unsigned int, NInputDimensions);
  itkStaticConstMacro(OutputSpaceDimension, unsigned int, NOutputDimensions);

  itkStaticConstMacro(ParametersDimension, unsigned int,
    NOutputDimensions * NInputDimensions + 1);                // 10 parameters

  /** Parameters Type   */
  typedef typename Superclass::ParametersType ParametersType;

  /** Jacobian Type   */
  typedef typename Superclass::JacobianType JacobianType;

  /** Standard scalar type for this class */
  typedef typename Superclass::ScalarType ScalarType;

  /** Standard vector type for this class   */
  typedef Vector<TScalarType,
    itkGetStaticConstMacro(InputSpaceDimension)>  InputVectorType;
  typedef Vector<TScalarType,
    itkGetStaticConstMacro(OutputSpaceDimension)> OutputVectorType;

  /** Standard covariant vector type for this class   */
  typedef CovariantVector<TScalarType,
    itkGetStaticConstMacro(InputSpaceDimension)>
  InputCovariantVectorType;
  typedef CovariantVector<TScalarType,
    itkGetStaticConstMacro(OutputSpaceDimension)>
  OutputCovariantVectorType;

  /** Standard vnl_vector type for this class   */
  typedef vnl_vector_fixed<TScalarType,
    itkGetStaticConstMacro(InputSpaceDimension)>
  InputVnlVectorType;
  typedef vnl_vector_fixed<TScalarType,
    itkGetStaticConstMacro(OutputSpaceDimension)>
  OutputVnlVectorType;

  /** Standard coordinate point type for this class   */
  typedef Point<TScalarType,
    itkGetStaticConstMacro(InputSpaceDimension)>
  InputPointType;
  typedef Point<TScalarType,
    itkGetStaticConstMacro(OutputSpaceDimension)>
  OutputPointType;

  /** VersorRigid3DTransform as Head Motion Transform Type **/
  typedef itk::VersorRigid3DTransform<double> HeadMotionTransformType;

  /** LinearEddyCurrentTransform as Eddy Current Correction Transform Type **/
  typedef itk::LinearEddyCurrentTransform<
    double,
    3>  EddyCurrentTransformType;

  /** HeadMotionTransformType Parameters Type   */
  typedef typename HeadMotionTransformType::ParametersType
  HeadMotionTransformTypeParametersType;
  typedef typename HeadMotionTransformType::VersorType VersorType;
  typedef typename VersorType::ValueType
  HeadMotionTransformValueType;

  /** HeadMotionTransformType Parameters Type   */
  typedef typename EddyCurrentTransformType::ParametersType
  EddyCurrentTransformTypeParametersType;

  /** Standard matrix type for this class   */
  typedef Matrix<TScalarType, itkGetStaticConstMacro(OutputSpaceDimension),
    itkGetStaticConstMacro(InputSpaceDimension)>
  MatrixType;

  typedef InputPointType   CenterType;

  typedef OutputVectorType TranslationType;

  /** Set center of rotation of an LinearHeadEddy3DCorrection */
  void SetCenter(const InputPointType & center)
  {
    m_Center = center;
  }

  /** Get center of rotation of the LinearHeadEddy3DCorrection
   *
   * This method returns the point used as the fixed
   * center of rotation for the LinearHeadEddy3DCorrection.
   *
   */
  const InputPointType & GetCenter() const
  {
    return m_Center;
  }

  /** Set the transformation from a container of parameters.
   * The first (NOutputDimension x NInputDimension) parameters define the
   * matrix and the last NOutputDimension parameters the translation.
   * Offset is updated based on current center.
   */
  void SetParameters( const ParametersType & parameters );

  /** Get the Transformation Parameters. */
  const ParametersType & GetParameters(void) const;

  /** Set the fixed parameters and update internal transformation. */
  virtual void SetFixedParameters( const ParametersType & );

  /** Get the Fixed Parameters. */
  virtual const ParametersType & GetFixedParameters(void) const;

  /** Transform by an linearHeadEddy3DCorrection transformation
   *
   * This method applies the affine transform given by self to a
   * given point or vector, returning the transformed point or
   * vector.  The TransformPoint method transforms its argument as
   * an affine point,
   */
  OutputPointType     TransformPoint(const InputPointType & point) const;

  /** Compute the Jacobian of the transformation
   *
   * This method computes the Jacobian matrix of the transformation.
   * given point or vector, returning the transformed point or
   * vector. The rank of the Jacobian will also indicate if the transform
   * is invertible at this point.
   */
  const JacobianType & GetJacobian(const InputPointType & point ) const;

  typedef HeadMotionTransformType::MatrixType RotationMatrixType;
  const RotationMatrixType & GetHeadMotionRotationMatrix(void) const;

  // virtual bool IsLinear() const { return true; }
protected:
  /** Construct an LinearHeadEddy3DCorrection object
   *
   * This method constructs a new LinearHeadEddy3DCorrection object and
   * initializes the matrix and offset parts of the transformation
   * to values specified by the caller.  If the arguments are
   * omitted, then the LinearHeadEddy3DCorrection is initialized to an identity
   * transformation in the appropriate number of dimensions.   **/

  LinearHeadEddy3DCorrection();

  /** Destroy an LinearHeadEddy3DCorrection object   **/
  virtual ~LinearHeadEddy3DCorrection();

  /** Print contents of an LinearHeadEddy3DCorrection */
  void PrintSelf(std::ostream & s, Indent indent) const;

  void SetVarCenter(const InputPointType & center)
  {
    m_Center = center;
  }

private:

  LinearHeadEddy3DCorrection(const Self & other);
  const Self & operator=( const Self & );

  InputPointType m_Center;

  HeadMotionTransformType::Pointer  m_head_motion_transform;
  EddyCurrentTransformType::Pointer m_eddy_current_transform;
  }; // class LinearHeadEddy3DCorrection
}  // namespace itk

// Define instantiation macro for this template.
#define ITK_TEMPLATE_LinearHeadEddy3DCorrection(_, EXPORT, x, y) \
  namespace itk { \
  _( 3 ( class EXPORT LinearHeadEddy3DCorrection < ITK_TEMPLATE_3 x > ) ) \
  namespace Templates { typedef LinearHeadEddy3DCorrection < ITK_TEMPLATE_3 x > \
                        LinearHeadEddy3DCorrection ## y; } \
  }

#if ITK_TEMPLATE_EXPLICIT
# include "Templates/itkLinearHeadEddy3DCorrection+-.h"
#endif

#if ITK_TEMPLATE_TXX
# include "itkLinearHeadEddy3DCorrection.txx"
#endif

#endif /* __itkLinearHeadEddy3DCorrection_h */
