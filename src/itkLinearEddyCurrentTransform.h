#ifndef __itkLinearEddyCurrentTransform_h
#define __itkLinearEddyCurrentTransform_h

#include <iostream>

#include "itkMatrix.h"
#include "itkTransform.h"
#include "itkExceptionObject.h"
#include "itkMacro.h"

/** For Linear Eddy Current Transformation **/

namespace itk
{
template <
  class TScalarType = double,         // Data type for scalars
  unsigned int NInputDimensions = 3,  // Number of dimensions in the input space
  unsigned int NOutputDimensions = 3>
// Number of dimensions in the output space
class LinearEddyCurrentTransform :
  public Transform<TScalarType, NInputDimensions, NOutputDimensions>
{
public:
  /** Standard typedefs   */
  typedef LinearEddyCurrentTransform Self;
  typedef Transform<TScalarType,
                    NInputDimensions,
                    NOutputDimensions>        Superclass;
  typedef SmartPointer<Self>       Pointer;
  typedef SmartPointer<const Self> ConstPointer;

  /** Run-time type information (and related methods).   */
  itkTypeMacro( LinearEddyCurrentTransform, Transform );

  /** New macro for creation of through a Smart Pointer   */
  itkNewMacro( Self );

  /** Dimension of the domain space. */
  itkStaticConstMacro(InputSpaceDimension, unsigned int, NInputDimensions);
  itkStaticConstMacro(OutputSpaceDimension, unsigned int, NOutputDimensions);

  itkStaticConstMacro(ParametersDimension, unsigned int,
                      NInputDimensions + 1);

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

  /** Standard matrix type for this class   */
  typedef Matrix<TScalarType, itkGetStaticConstMacro(OutputSpaceDimension),
                 itkGetStaticConstMacro(InputSpaceDimension)>
  MatrixType;

  /** Standard inverse matrix type for this class   */
  typedef Matrix<TScalarType, itkGetStaticConstMacro(InputSpaceDimension),
                 itkGetStaticConstMacro(OutputSpaceDimension)>
  InverseMatrixType;

  typedef InputPointType CenterType;

  typedef OutputVectorType OffsetType;

  typedef OutputVectorType TranslationType;

  /** Set the transformation to an Identity
   *
   * This sets the matrix to identity and the Offset to null. */
  virtual void SetIdentity( void );

  /** Set matrix of an LinearEddyCurrentTransform
   *
   * This method sets the matrix of an LinearEddyCurrentTransform to a
   * value specified by the user.
   *
   * This updates the Offset wrt to current translation
   * and center.  See the warning regarding offset-versus-translation
   * in the documentation for SetCenter.
   *
   * To define an affine transform, you must set the matrix,
   * center, and translation OR the matrix and offset */
  virtual void SetMatrix(const MatrixType & matrix)
  {
    m_Matrix = matrix; this->ComputeOffset();
    this->ComputeMatrixParameters();
    m_MatrixMTime.Modified(); this->Modified(); return;
  }

  /** Get matrix of an LinearEddyCurrentTransform
   *
   * This method returns the value of the matrix of the
   * LinearEddyCurrentTransform.
   * To define an affine transform, you must set the matrix,
   * center, and translation OR the matrix and offset */
  const MatrixType & GetMatrix() const
  {
    return m_Matrix;
  }

  /** Set offset (origin) of an MatrixOffset TransformBase.
   *
   * This method sets the offset of an LinearEddyCurrentTransform to a
   * value specified by the user.
   * This updates Translation wrt current center.  See the warning regarding
   * offset-versus-translation in the documentation for SetCenter.
   * To define an affine transform, you must set the matrix,
   * center, and translation OR the matrix and offset */
  void SetOffset(const OutputVectorType & offset)
  {
    m_Offset = offset; this->ComputeTranslation();
    this->Modified(); return;
  }

  /** Get offset of an LinearEddyCurrentTransform
   *
   * This method returns the offset value of the LinearEddyCurrentTransform.
   * To define an affine transform, you must set the matrix,
   * center, and translation OR the matrix and offset */
  const OutputVectorType & GetOffset(void) const
  {
    return m_Offset;
  }

  /** Set center of rotation of an LinearEddyCurrentTransform
   *
   * This method sets the center of rotation of an LinearEddyCurrentTransform
   * to a fixed point - for most transforms derived from this class,
   * this point is not a "parameter" of the transform - the exception is that
   * "centered" transforms have center as a parameter during optimization.
   *
   * This method updates offset wrt to current translation and matrix.
   * That is, changing the center changes the transform!
   *
   * WARNING: When using the Center, we strongly recommend only changing the
   * matrix and translation to define a transform.   Changing a transform's
   * center, changes the mapping between spaces - specifically, translation is
   * not changed with respect to that new center, and so the offset is updated
   * to * maintain the consistency with translation.   If a center is not used,
   * or is set before the matrix and the offset, then it is safe to change the
   * offset directly.
   *        As a rule of thumb, if you wish to set the center explicitly, set
   * before Offset computations are done.
   *
   * To define an affine transform, you must set the matrix,
   * center, and translation OR the matrix and offset */
  void SetCenter(const InputPointType & center)
  {
    m_Center = center; this->ComputeOffset();
    this->Modified(); return;
  }

  /** Get center of rotation of the LinearEddyCurrentTransform
   *
   * This method returns the point used as the fixed
   * center of rotation for the LinearEddyCurrentTransform.
   * To define an affine transform, you must set the matrix,
   * center, and translation OR the matrix and offset */
  const InputPointType & GetCenter() const
  {
    return m_Center;
  }

  /** Set translation of an LinearEddyCurrentTransform
   *
   * This method sets the translation of an LinearEddyCurrentTransform.
   * This updates Offset to reflect current translation.
   * To define an affine transform, you must set the matrix,
   * center, and translation OR the matrix and offset */
  void SetTranslation(const OutputVectorType & translation)
  {
    m_Translation = translation; this->ComputeOffset();
    this->Modified(); return;
  }

  /** Get translation component of the LinearEddyCurrentTransform
   *
   * This method returns the translation used after rotation
   * about the center point.
   * To define an affine transform, you must set the matrix,
   * center, and translation OR the matrix and offset */
  const OutputVectorType & GetTranslation(void) const
  {
    return m_Translation;
  }

  /** Set the transformation from a container of parameters.
   * The first (NOutputDimension x NInputDimension) parameters define the
   * matrix and the last NOutputDimension parameters the translation.
   * Offset is updated based on current center. */
  void SetParameters( const ParametersType & parameters );

  /** Get the Transformation Parameters. */
  const ParametersType & GetParameters(void) const;

  /** Set the fixed parameters and update internal transformation. */
  virtual void SetFixedParameters( const ParametersType & );

  /** Get the Fixed Parameters. */
  virtual const ParametersType & GetFixedParameters(void) const;

  /** Compose with another LinearEddyCurrentTransform
   *
   * This method composes self with another LinearEddyCurrentTransform of the
   * same dimension, modifying self to be the composition of self
   * and other.  If the argument pre is true, then other is
   * precomposed with self; that is, the resulting transformation
   * consists of first applying other to the source, followed by
   * self.  If pre is false or omitted, then other is post-composed
   * with self; that is the resulting transformation consists of
   * first applying self to the source, followed by other.
   * This updates the Translation based on current center. */
  void Compose(const Self *other, bool pre = 0);

  /** Transform by an affine transformation
   *
   * This method applies the affine transform given by self to a
   * given point or vector, returning the transformed point or
   * vector.  The TransformPoint method transforms its argument as
   * an affine point, whereas the TransformVector method transforms
   * its argument as a vector. */
  OutputPointType     TransformPoint(const InputPointType & point) const;

  OutputVectorType    TransformVector(const InputVectorType & vector) const;

  OutputVnlVectorType TransformVector(const InputVnlVectorType & vector) const;

  OutputCovariantVectorType TransformCovariantVector(const InputCovariantVectorType & vector) const;

  /** Compute the Jacobian of the transformation
   *
   * This method computes the Jacobian matrix of the transformation.
   * given point or vector, returning the transformed point or
   * vector. The rank of the Jacobian will also indicate if the transform
   * is invertible at this point. */
  const JacobianType & GetJacobian(const InputPointType & point ) const;

  /** Create inverse of an affine transformation
    *
    * This populates the parameters an affine transform such that
    * the transform is the inverse of self. If self is not invertible,
    * an exception is thrown.
    * Note that by default the inverese transform is centered at
    * the origin. If you need to compute the inverse centered at a point, p,
    *
    * \code
    * transform2->SetCenter( p );
    * transform1->GetInverse( transform2 );
    * \endcode
    *
    * transform2 will now contain the inverse of transform1 and will
    * with its center set to p. Flipping the two statements will produce an
    * incorrect transform. */
  bool GetInverse(Self *inverse) const;

  /** \deprecated Use GetInverse instead.
   *
   * Method will eventually be made a protected member function */
  const InverseMatrixType & GetInverseMatrix( void ) const;

  /** Indicates that this transform is linear. That is, given two
   * points P and Q, and scalar coefficients a and b, then
   *
   *           T( a*P + b*Q ) = a * T(P) + b * T(Q)
   */
  virtual bool IsLinear() const
  {
    return true;
  }

protected:
  /** Construct an LinearEddyCurrentTransform object
   *
   * This method constructs a new LinearEddyCurrentTransform object and
   * initializes the matrix and offset parts of the transformation
   * to values specified by the caller.  If the arguments are
   * omitted, then the LinearEddyCurrentTransform is initialized to an identity
   * transformation in the appropriate number of dimensions.   **/
  LinearEddyCurrentTransform(const MatrixType & matrix, const OutputVectorType & offset);
  LinearEddyCurrentTransform(unsigned int outputDims, unsigned int paramDims);
  LinearEddyCurrentTransform();

  /** Destroy an LinearEddyCurrentTransform object   **/
  virtual ~LinearEddyCurrentTransform();

  /** Print contents of an LinearEddyCurrentTransform */
  void PrintSelf(std::ostream & s, Indent indent) const;

  const InverseMatrixType & GetVarInverseMatrix( void ) const
  {
    return m_InverseMatrix;
  }

  void SetVarInverseMatrix(const InverseMatrixType & matrix) const
  {
    m_InverseMatrix = matrix; m_InverseMatrixMTime.Modified();
  }

  bool InverseMatrixIsOld(void) const
  {
    if( m_MatrixMTime != m_InverseMatrixMTime )
      {
      return true;
      }
    else
      {
      return false;
      }
  }

  virtual void ComputeMatrixParameters(void);

  virtual void ComputeMatrix(void);

  void SetVarMatrix(const MatrixType & matrix)
  {
    m_Matrix = matrix; m_MatrixMTime.Modified();
  }

  virtual void ComputeTranslation(void);

  void SetVarTranslation(const OutputVectorType & translation)
  {
    m_Translation = translation;
  }

  virtual void ComputeOffset(void);

  void SetVarOffset(const OutputVectorType & offset)
  {
    m_Offset = offset;
  }

  void SetVarCenter(const InputPointType & center)
  {
    m_Center = center;
  }

private:

  LinearEddyCurrentTransform(const Self & other);
  const Self & operator=( const Self & );

  MatrixType                m_Matrix;           // Matrix of the transformation
  OutputVectorType          m_Offset;           // Offset of the transformation
  mutable InverseMatrixType m_InverseMatrix;    // Inverse of the matrix
  mutable bool              m_Singular;         // Is m_Inverse singular?

  InputPointType   m_Center;
  OutputVectorType m_Translation;

  /** To avoid recomputation of the inverse if not needed */
  TimeStamp         m_MatrixMTime;
  mutable TimeStamp m_InverseMatrixMTime;
};   // class LinearEddyCurrentTransform
}  // namespace itk

// Define instantiation macro for this template.
#define ITK_TEMPLATE_LinearEddyCurrentTransform(_, EXPORT, x, y) \
  namespace itk { \
  _( 3 ( class EXPORT LinearEddyCurrentTransform<ITK_TEMPLATE_3 x> ) ) \
  namespace Templates { typedef LinearEddyCurrentTransform<ITK_TEMPLATE_3 x> \
                        LinearEddyCurrentTransform##y; } \
  }

#if ITK_TEMPLATE_EXPLICIT
#include "Templates/itkLinearEddyCurrentTransform+-.h"
#endif

#if ITK_TEMPLATE_TXX
#include "itkLinearEddyCurrentTransform.txx"
#endif

#endif /* __itkLinearEddyCurrentTransform_h */
