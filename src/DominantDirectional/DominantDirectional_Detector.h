/*=========================================================================

Program:   NeuroLib
Language:  C++
Author:    Mahshid Farzinfar farzinfa@email.unc.edu

Copyright (c) NIRAL, UNC. All rights reserved.
See http://www.niral.unc.edu for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


#ifndef DominantDirectional_Detector_h
#define DominantDirectional_Detector_h
// ITK VTK
#include <iostream>
#include <itkDataObject.h>
#include <itkObjectFactory.h>
#include <itkImage.h>
#include <itkImageRegionConstIteratorWithIndex.h>
#include <itkVectorContainer.h>
#include <itkVector.h>
#include <itkCovariantVector.h>
#include <itkDiffusionTensor3D.h>
#include <itkVectorImage.h>
// IO
#include <itkImageFileReader.h>
#include <itkImageFileWriter.h>
//DTIProcess
#include "itkTensorPrincipalEigenvectorImageFilter.h"

// VNL Includes
#include <vnl/vnl_matrix.h>
#include <vnl/vnl_vector_fixed.h>
#include "itkImage.h"
#include <iostream>
#include <fstream>
#include <string>

#include "itkVectorImage.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkDiffusionTensor3DReconstructionImageFilter.h"

#include <QProcess>
#include <QObject>



const unsigned int DIM = 3;

class DiffusionTensorEstimation
{

	public:
		DiffusionTensorEstimation();
		~DiffusionTensorEstimation();

		
		typedef unsigned short 									ScalarPixelType;	//Data type of pixel in  image (DWI)
		typedef double											VectorPixelValueType;	//Data type of pixel in  image (PD)
		typedef itk::DiffusionTensor3D<double> 					TensorPixelType;	//Data type of pixel in  image (Tensor)
		typedef TensorPixelType::RealValueType					RealValueType;
		typedef itk::CovariantVector<VectorPixelValueType,3>  	VPixelType;

		typedef itk::VectorImage<ScalarPixelType, DIM> 			VectorImageType;
		typedef itk::Image<TensorPixelType, DIM> 				TensorImageType;
		typedef itk::Image<VPixelType,DIM>						PDImageType;
		typedef itk::Image< RealValueType,DIM >					FAImageType;
		
		typedef itk::DiffusionTensor3DReconstructionImageFilter	<ScalarPixelType,ScalarPixelType, double> 		TensorReconstructionImageFilterType;
		

		typedef itk::ImageFileReader<VectorImageType> 			FileReaderType_DWI;
		typedef itk::ImageFileReader<TensorImageType> 			FileReaderType_Tensor;
		typedef itk::ImageFileReader<PDImageType>				FileReaderType_PD;
		typedef itk::ImageFileReader<FAImageType>				FileReaderType_FA;

		typedef itk::TensorPrincipalEigenvectorImageFilter<TensorImageType,PDImageType>							TensorPrincipalEigenvector;
		typedef  TensorReconstructionImageFilterType::GradientDirectionContainerType    						GradientDirectionContainerType;	

	
		

		
		double EstimateTensor_Whitematter_GrayMatter(std::string buffer1, std::string buffer2, std::string buffer3 /*, std::string buffer2 */, std::string buffer4);
		
		short subdivisionLevel;
		void SaveEntropyResult_beta( std::string Reportfilename, std::string Labelfilename, std::string DWIfilename);
		

		void Set_subdivisionLevel( short level)
		{
			subdivisionLevel = level;
		}
		
		double Get_entropyValue()
		{
			return m_entropy;
		}

		
		

	protected:
		GradientDirectionContainerType::Pointer 	GradientDirectionContainer;	// container to hold gradient directions of the 'n' DW measurements
		bool 						bResultReport;			//report of Entropy Result
		double 						m_entropy;			// entropy result
		std::vector < double >  			m_bins;		// histogram

		int 						threshold_dtiestim;		
		std::string					current_image_correction;	// filename path of the result of Correction() function
		std::string					current_image_correction_round;	// filename path of the result of Correction_Round1() function
		std::string					current_filename_output; // filename of the result of Correction() function
		
		std::string					current_updated_path;		// path
		std::string					current_updated_outputfilename;		// updated image name
		std::string 					current_updated_image;			// updated image

		std::string					current_updated_path2;		// path
		std::string					current_updated_outputfilename2;		// updated image name
		std::string 					current_updated_image2;			// updated image
		




};

#endif

