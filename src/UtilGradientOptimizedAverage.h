#ifndef __UTILGRADIENTOPTIMIZEDAVERAGE_H__
#define __UTILGRADIENTOPTIMIZEDAVERAGE_H__
namespace itk {
/**
 *
 */
template <class TImageType>
void
  DWIBaselineAverager<TImageType>
::GradientOptimizedAverage()
{
  // 1: create idwi
  // 2: register all gradients into idwi
  // iteratively do  1 and 2 until idwi doesn't change any more(by threshold)

  // register all baselines into the final idwi then average.
  InputImageConstPointer inputPtr = this->GetInput();

  typedef itk::VectorIndexSelectionCastImageFilter<TImageType, UnsignedImageType> FilterType;
  typename FilterType::Pointer componentExtractor = FilterType::New();
  componentExtractor->SetInput(inputPtr);

  // first do direct averaging idwi
  this->computeIDWI();   // init idwi with the original DW gradients

  // resampledGradient: used in registration(), containing the resampled image
  // after transformation from registration.
  m_tempIDWI = doubleImageType::New();
  m_tempIDWI->CopyInformation(inputPtr);
  m_tempIDWI->SetRegions( inputPtr->GetLargestPossibleRegion() );
  m_tempIDWI->Allocate();
  m_tempIDWI->FillBuffer(1);

  // iteratively register each DW gradient onto the idwi
  std::vector<struRigidRegResult> allResults;
  struRigidRegResult              result;

  for ( unsigned int i = 0; i < this->m_GradientDirectionContainer->Size(); i++ )
    {
      if ( this->GradientDirectionIsB0Image(i) == false )
      {
      allResults.push_back(result);
      }
    }

  bool         bRegister = true;
  unsigned int iterationCount = 0;
  std::cout << "Gradient optimized averaging baseline ...";
  do
    {
    iterationCount++;
    std::cout << " iteration: " << iterationCount << "...";

    double oldMISum = 0.0;
    double newMISum = 0.0;

    double oldTransSum = 0.0;
    double newTransSum = 0.0;

    double oldRotateSum = 0.0;
    double newRotateSum = 0.0;

    int gradientNum = 0;
    for ( unsigned int i = 0;
      i < this->m_GradientDirectionContainer->Size();
      i++ )
      {
      if ( 0.0 !=  this->m_GradientDirectionContainer->ElementAt(i)[0]
        || 0.0 !=  this->m_GradientDirectionContainer->ElementAt(i)[1]
        || 0.0 !=  this->m_GradientDirectionContainer->ElementAt(i)[2]    )
        {
        componentExtractor->SetIndex( i );
        componentExtractor->Update();

        // register and replace the DW gradient volumes with the idwi
        //           std::cout<<"Registering DW gradient #: "<<i<<" to current
        // idwi ... ";
        struRigidRegResult resultLocal;
        exit(-1); //HACK:  Not valid testing yet.
        typename UnsignedImageType::Pointer dummy=NULL;
        rigidRegistration<UnsignedImageType,itk::Image<double,3> >( this->idwi,
          componentExtractor->GetOutput(), 25, 0.1, 1, resultLocal,
          1 /* DW gradient */,
          dummy,
          this->m_tempIDWI
        );
        std::cout << " done " << std::endl;

        allResults.at(gradientNum) = resultLocal;
        gradientNum++;

        //           std::cout<<"DW gradient #: "<<i<<std::endl;
        //           std::cout<<"Angles: "<<resultLocal.AngleX<<"
        // "<<resultLocal.AngleY<<" "<<resultLocal.AngleZ<<std::endl;
        //           std::cout<<"Trans : "<<resultLocal.TranslationX<<"
        // "<<resultLocal.TranslationY<<"
        // "<<resultLocal.TranslationZ<<std::endl;
        //           std::cout<<"MI    :
        // "<<resultLocal.MutualInformation<<std::endl;
        }
      }

    if ( m_StopCriteria == TotalTransformationBased )
      {
      //         std::cout<<"StopCriteriaEnum:
      // TotalTransformationBased"<<std::endl;
      //         std::cout<<"StopThreshold: "<<m_StopThreshold<<std::endl;

      if ( ( vcl_abs( newTransSum  - oldTransSum  )  < m_StopThreshold )
        && ( vcl_abs( newRotateSum - oldRotateSum ) < m_StopThreshold ) )                                                          //
        //
        // 0.01
        {
        bRegister = false;
        }
      }

    if ( m_StopCriteria == MetricSumBased )
      {
      //         std::cout<<"StopCriteriaEnum: MetricSumBased"<<std::endl;
      //         std::cout<<"StopThreshold: "<<m_StopThreshold<<std::endl;

      if ( vcl_abs(newMISum - oldMISum)  < m_StopThreshold ) // 0.001
        {
        bRegister = false;
        }
      }


    double         meanSquarediff = 0.0;
    double         meanIntensity = 0.0;
      {
      // compute the idwi with registered DW gradient
      typedef ImageRegionIteratorWithIndex<UnsignedImageType> idwiIterator;
      idwiIterator idwiIt( idwi, idwi->GetLargestPossibleRegion() );
      idwiIt.GoToBegin();
      while ( !idwiIt.IsAtEnd() )
        {
        const typename UnsignedImageType::IndexType pixelIndex= idwiIt.GetIndex();
        const unsigned short pixelValue = idwiIt.Get();
        meanIntensity += pixelValue;
        idwiIt.Set( vcl_pow( m_tempIDWI->GetPixel(pixelIndex),
            1.0 / this->getGradientNumber() ) );

        const double diffToPrevious=( pixelValue - idwiIt.Get() );
        meanSquarediff += diffToPrevious*diffToPrevious;
        ++idwiIt;
        }
      }
    m_tempIDWI->FillBuffer(1);

    //const UnsignedImageType::SizeType sizes = m_averagedBaseline->GetLargestPossibleRegion().GetSize();
    const long voxelCount  =m_averagedBaseline->GetLargestPossibleRegion().GetNumberOfPixels();
    meanSquarediff = vcl_sqrt(meanSquarediff / voxelCount);
    meanIntensity = meanIntensity / voxelCount;
    const double ratio = meanSquarediff / meanIntensity;

    //       std::cout<<"voxelCount                    :
    // "<<voxelCount<<std::endl;
    //       std::cout<<"meanVoxelIntensity            :
    // "<<meanIntensity<<std::endl;
    //       std::cout<<"meanVoxelSquareRootDifference :
    // "<<meanSquarediff<<std::endl;
    //       std::cout<<"meanVoxelSquareRootDifference/meanVoxelIntensity:
    // "<<ratio<<std::endl;

    if ( m_StopCriteria == IntensityMeanSquareDiffBased )
      {
      //         std::cout<<"StopCriteriaEnum:
      // IntensityMeanSquareDiffBased"<<std::endl;
      //         std::cout<<"StopThreshold: "<<m_StopThreshold<<std::endl;

      if ( ratio < m_StopThreshold )  // 0.02
        {
        bRegister = false;
        }
      }
    } while ( bRegister && iterationCount <= GetMaxIteration() );

  // registered all the baseline into idwi, then average
  // ////////////////////////////////////////////////////////////////////////
  UnsignedImageTypePointer temporaryDeformedImage = UnsignedImageType::New();
  temporaryDeformedImage->CopyInformation(inputPtr);
  temporaryDeformedImage->SetRegions( inputPtr->GetLargestPossibleRegion() );
  temporaryDeformedImage->Allocate();
  temporaryDeformedImage->FillBuffer(0);

  for ( unsigned int i = 0; i < this->m_GradientDirectionContainer->Size(); i++ )
    {
      if ( this->GradientDirectionIsB0Image(i) == true )
      {
      componentExtractor->SetIndex( i );
      componentExtractor->Update();

      struRigidRegResult resultLocal;

      // register and replace the DW gradient volumes with the idwi
      //         std::cout<<"Registering baseline #: "<<i<<" to idwi ... ";
      rigidRegistration<UnsignedImageType,itk::Image<double,3> >( this->idwi,
        componentExtractor->GetOutput(), 25, 0.1, 1, resultLocal,
        0 /* baseline */,
        temporaryDeformedImage,
        NULL
      );
      //         std::cout<<" done "<<std::endl;
      }
    }

  // average the registered baselines into m_averagedBaseline
  typedef ImageRegionIteratorWithIndex<UnsignedImageType> averagedBaselineIterator;
  averagedBaselineIterator aIt( m_averagedBaseline, m_averagedBaseline->GetLargestPossibleRegion() );
  aIt.GoToBegin();

  while ( !aIt.IsAtEnd() )
    {
    const typename UnsignedImageType::IndexType pixelIndex= aIt.GetIndex();
    //const unsigned short pixelValue = aIt.Get();
    aIt.Set( static_cast<unsigned short>(
        temporaryDeformedImage->GetPixel(pixelIndex)
        / ( static_cast<double>( this->getGradientNumber() ) ) 
    )
    );
    ++aIt;
    }

  //     std::cout<<"DW gradient based optimized done"<<std::endl;
}

} //end namespace itk
#endif // __UTILGRADIENTOPTIMIZEDAVERAGE_H__
