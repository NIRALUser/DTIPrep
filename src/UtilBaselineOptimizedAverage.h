#ifndef __UTILBASELINEOPTIMIZEDAVERAGE_H__
#define __UTILBASELINEOPTIMIZEDAVERAGE_H__

#include "UtilRegister.h"

namespace itk
{
/**
 *
 */
template <class TImageType>
bool BaselineOptimizedAverage(
  const std::vector<typename TImageType::Pointer> & baselineContainer,
  typename itk::Image<float, 3>::Pointer & InitialAndReturnedAverageBaseline,
  int OptimizationStopCriteria,
  float OptimiationStopThreshold,
  unsigned int MaxIterations
  )
{

  // Keep status of Baselines
  std::vector<bool> BaselineStatus;
  BaselineStatus.resize(baselineContainer.size() );

  // first do direct averaging baselines
  std::vector<struRigidRegResult> allImageRegistrations;
  allImageRegistrations.resize(baselineContainer.size() );
  std::vector<typename TImageType::Pointer> temporaryDeformedImages;
  temporaryDeformedImages.resize(baselineContainer.size() );
  for( unsigned int i = 0; i < baselineContainer.size(); i++ )
    {
    // iteratively register each baseline onto the averaged baseline
    struRigidRegResult result;
      {
      typedef typename itk::VersorRigid3DTransform<double> TransformType;
      typedef typename itk::CenteredTransformInitializer<TransformType,
                                                         itk::Image<float, 3>, TImageType> TransformInitializerType;
      typename TransformInitializerType::Pointer initializer = TransformInitializerType::New();
      initializer->SetTransform(   result.GetTransform() );
      initializer->SetFixedImage(  InitialAndReturnedAverageBaseline );
      initializer->SetMovingImage( baselineContainer[i] );
      initializer->MomentsOn();
      // initializer->GeometryOn(); //
      initializer->InitializeTransform();
      }
    allImageRegistrations[i] = (result);
      {
      temporaryDeformedImages[i] = TImageType::New();
      temporaryDeformedImages[i]->CopyInformation(InitialAndReturnedAverageBaseline);
      temporaryDeformedImages[i]->SetRegions( InitialAndReturnedAverageBaseline->GetLargestPossibleRegion() );
      temporaryDeformedImages[i]->Allocate();
      temporaryDeformedImages[i]->FillBuffer(0);
      }
    }
  bool         bRegister = true;
  unsigned int iterationCount = 0;

  std::cout << "Baseline optimized averaging baseline ..." << std::endl;

  do
    {
    iterationCount++;
    std::cout << " iteration: " << iterationCount <<  " of " << MaxIterations << "..." << std::endl;

    double oldMISum = 0.0;
    double newMISum = 0.0;

    double oldTransSum = 0.0;
    double newTransSum = 0.0;

    double oldRotateSum = 0.0;
    double newRotateSum = 0.0;

    double meanSquarediff = 0.0;
    double meanIntensity = 0.0;
      {     // Need to keep the last iteration of the the averageBaseline the same over time.
      for( unsigned int i = 0; i < baselineContainer.size(); i++ )
        {
        // register and replace the baseline volumes with the transformed ones
        //         std::cout<<"Registering baseline #: "<<i<<" to current averaged
        // baseline ... ";
        struRigidRegResult currentIterationCurrentImageRegistration =
          rigidRegistration<itk::Image<float, 3>, TImageType, TImageType>( InitialAndReturnedAverageBaseline,
                                                                           baselineContainer[i],
                                                                           25,
                                                                           0.1,
                                                                           1,
                                                                           allImageRegistrations[i],
                                                                           0 /* baseline */,
                                                                           temporaryDeformedImages[i],
                                                                           NULL
                                                                           );
        //         std::cout<<" done "<<std::endl;

        oldMISum += allImageRegistrations.at(i).GetMutualInformation();
        newMISum += currentIterationCurrentImageRegistration.GetMutualInformation();

        oldTransSum += ( vcl_abs(allImageRegistrations.at(i).GetTranslationX() )
                         + vcl_abs(allImageRegistrations.at(i).GetTranslationY() )
                         + vcl_abs(allImageRegistrations.at(i).GetTranslationZ() ) ) / 3.0;
        newTransSum +=
          ( vcl_abs(currentIterationCurrentImageRegistration.GetTranslationX() )
            + vcl_abs(currentIterationCurrentImageRegistration.GetTranslationY() )
            + vcl_abs(currentIterationCurrentImageRegistration.GetTranslationZ() ) ) / 3.0;

        oldRotateSum +=
          ( vcl_abs(allImageRegistrations.at(i).GetFinalAngleInRadiansX() )
            + vcl_abs(allImageRegistrations.at(i).GetFinalAngleInRadiansY() )
            + vcl_abs(allImageRegistrations.at(i).GetFinalAngleInRadiansZ() ) ) / 3.0;
        newRotateSum +=
          ( vcl_abs(currentIterationCurrentImageRegistration.GetFinalAngleInRadiansX() )
            + vcl_abs(currentIterationCurrentImageRegistration.GetFinalAngleInRadiansY() )
            + vcl_abs(currentIterationCurrentImageRegistration.GetFinalAngleInRadiansZ() ) ) / 3.0;

        std::cout << "Current metric: " << i << " "
                  << currentIterationCurrentImageRegistration.GetMutualInformation() << std::endl;
        // prepare for next iteration.
        allImageRegistrations.at(i) = currentIterationCurrentImageRegistration;
        }
      // Need to compute new averages.
      typedef itk::Image<float, 3> FloatImageType;
      typename FloatImageType::Pointer newAverageAccumulator = FloatImageType::New();
      newAverageAccumulator->CopyInformation(InitialAndReturnedAverageBaseline);
      newAverageAccumulator->SetRegions( InitialAndReturnedAverageBaseline->GetLargestPossibleRegion() );
      newAverageAccumulator->Allocate();
      newAverageAccumulator->FillBuffer(0.0);

        {
        // average the registered baselines into InitialAndReturnedAverageBaseline
        typedef ImageRegionIteratorWithIndex<FloatImageType> averagedBaselineIterator;
        averagedBaselineIterator aIt( newAverageAccumulator, newAverageAccumulator->GetLargestPossibleRegion() );
        aIt.GoToBegin();
        const float InvNumberOfBaselineContainers = 1.0F / static_cast<float>(baselineContainer.size() );
        while( !aIt.IsAtEnd() )
          {
          const typename FloatImageType::IndexType pixelIndex = aIt.GetIndex();
          float localAccum = 0;
          for( unsigned int i = 0; i < baselineContainer.size(); i++ )
            {
            localAccum += temporaryDeformedImages[i]->GetPixel(pixelIndex);
            }
          localAccum *= InvNumberOfBaselineContainers;
          newAverageAccumulator->SetPixel(pixelIndex, localAccum);
          ++aIt;
          }
        }
      // Now update the estimate of the average
      // average the registered baselines into InitialAndReturnedAverageBaseline
      typedef ImageRegionConstIteratorWithIndex<FloatImageType> averagedBaselineIterator;
      averagedBaselineIterator aIt( newAverageAccumulator, newAverageAccumulator->GetLargestPossibleRegion() );
      aIt.GoToBegin();
      while( !aIt.IsAtEnd() )
        {
        const unsigned short pixelValue = aIt.Get();
        const typename FloatImageType::IndexType currLoc = aIt.GetIndex();
        meanIntensity += pixelValue;
        const double diffPrevious = ( pixelValue - InitialAndReturnedAverageBaseline->GetPixel( currLoc ) );
        meanSquarediff += diffPrevious * diffPrevious;
        InitialAndReturnedAverageBaseline->SetPixel( currLoc, pixelValue);      // Replace the baseline with the new
                                                                                // estimate here.
        ++aIt;
        }

      const long voxelCount  = InitialAndReturnedAverageBaseline->GetLargestPossibleRegion().GetNumberOfPixels();
      meanSquarediff = vcl_sqrt(meanSquarediff / voxelCount);
      meanIntensity = meanIntensity / voxelCount;

      std::cout << " meanSquarediff " << meanSquarediff << std::endl;
      std::cout << " meanIntensity " << meanIntensity << std::endl;

        {
        InitialAndReturnedAverageBaseline = newAverageAccumulator;
        }
      }
    const double ratio = meanSquarediff / meanIntensity;

    std::cout << "OptimizationStopCriteria " << OptimizationStopCriteria << "IntensityMeanSquareDiffBased"
              << IntensityMeanSquareDiffBased << "ratio" << ratio << std::endl;

    if( OptimizationStopCriteria == IntensityMeanSquareDiffBased )
      {
      std::cout << "IntensityMeanSquareDiffBased: " << ratio << " < " << OptimiationStopThreshold << std::endl;
      if( ratio < OptimiationStopThreshold )       // 0.02
        {
        bRegister = false;
        break;
        }
      }

    if( iterationCount > MaxIterations )
      {
      std::cout << "Iterative registration seems not converging,doing direct averaging ..." << std::endl;
      // //exit(-1);//HACK:  This is not an acceptable work around. (Mahshid deleted this line! and changed the
      // BaselineOptimizedAverage() to return bool)
      // DirectAverage();
      bRegister = false;
      return false;
      }
    }
  while( bRegister );

  // if ( ratio > OptimiationStopThreshold ) && max interation number reached
  // register all the other baselines to the 1st one
  std::cout << "DONE" << std::endl;
  return true;
}

} // end namespace itk

#endif // __UTILBASELINEOPTIMIZEDAVERAGE_H__
