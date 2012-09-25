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



#include "DominantDirectional_Detector.h"

#include <iostream>
#include <fstream>
#include <string>
#include <math.h>

#include "itkMetaDataDictionary.h"
#include "itkNrrdImageIO.h"
#include "itkMetaDataDictionary.h"
#include "itkMetaDataObject.h"
#include "itkNrrdImageIO.h"

#include "Counter.h"

/*******************************************************************************************************************************/
DiffusionTensorEstimation::DiffusionTensorEstimation(){
	subdivisionLevel = 0;
	//m_entropy = 0;
	bResultReport = false;
}

/*******************************************************************************************************************************/
DiffusionTensorEstimation::~DiffusionTensorEstimation()
{}

/*******************************************************************************************************************************/
/* labelfilename, QCedDWI, Reportfilename, DTI ==  buffer1, buffer3,  buffer2,  buffer4 */
double DiffusionTensorEstimation::EstimateTensor_Whitematter_GrayMatter(std::string buffer1, std::string buffer3, std::string buffer2, std::string buffer4)
{

			std::string buffer_tensor;
			std::string DTI_name;
			DTI_name.append("tensor.nrrd");
			if ( buffer4.length()==0 )
			{
			std::cout << "buffer1" << buffer1 << std::endl;
			std::string str8;
			str8.append("/tools/bin_linux64/dtiestim");
			str8.append(" ");
			str8.append("--dwi_image ");
			str8.append(buffer3);
			str8.append(" -m ");
			str8.append(" wls ");
			str8.append(" -M ");
			str8.append(buffer1);
			buffer_tensor.append(DTI_name);
			//buffer_tensor.replace(buffer3.end()-1,buffer3.end()-6,"_LabeledTensor.nrrd");
			str8.append(" --tensor_output ");
			str8.append(buffer_tensor);
			str8.append(" -t 0 " );
			std::cout << "dtiestim command: " << str8.c_str() << std::endl;
			system( str8.c_str() );
			}
			else
				buffer_tensor = buffer4;


			FileReaderType_Tensor::Pointer tensorReader4 = FileReaderType_Tensor::New();
			tensorReader4->SetFileName( buffer_tensor.c_str() );
			try
			{
				//if(VERBOSE)
				std::cout << "Reading Data Labeled Tensor" << std::endl;
				tensorReader4->Update();
			}
			catch (itk::ExceptionObject & e)
			{
				std::cerr << e <<std::endl;
			}

			std::cout << "===============  Starting Computing Principal Direction ===============" << std::endl;
			
			TensorImageType::Pointer tensor4 = TensorImageType::New();
			PDImageType::Pointer PDImage4 = PDImageType::New();
			TensorPrincipalEigenvector::Pointer m_tensorPrincipalEigenvector4 = TensorPrincipalEigenvector::New();
		
			tensor4 = tensorReader4->GetOutput();
			m_tensorPrincipalEigenvector4->SetInput(tensor4);
			m_tensorPrincipalEigenvector4->Update();
			PDImage4 = m_tensorPrincipalEigenvector4->GetOutput();
			
			std::cout << "===============  Computing the histogram ===============" << std::endl;
			// For each pixel, the the best correspondance vertex on sphere is found --> the histogram of PD distrubution using their frequencies
	
			m_entropy=0;
			//subdivisionLevel = 8 ;
			fiberodf::Counter::Initialize(subdivisionLevel);
			fiberodf::CounterSerializer_TXT counterTXT4 ;
			fiberodf::Counter_NearestNeighborVertex counter4(counterTXT4) ;
			//fiberodf::Counter_WeightedVertices counter2(counterTXT) ;
		
			
			typedef itk::ImageRegionConstIteratorWithIndex<PDImageType>	ConstIteratorType;
			ConstIteratorType oit4( PDImage4, PDImage4->GetLargestPossibleRegion() );
			VPixelType VectorPD4;
			oit4.GoToBegin();
			while ( !oit4.IsAtEnd() )
			{
				VectorPD4[0] = oit4.Get()[0];
				VectorPD4[1] = oit4.Get()[1];
				VectorPD4[2] = oit4.Get()[2];
				counter4.Add(fiberodf::Vector(VectorPD4[0],VectorPD4[1], VectorPD4[2])) ;
				counter4.Add(fiberodf::Vector(-VectorPD4[0],-VectorPD4[1], -VectorPD4[2])) ;
				//counter2.Add(fiberodf::Vector(VectorPD[0],VectorPD[1], VectorPD[2])) ;
				//counter2.Add(fiberodf::Vector(-VectorPD[0],-VectorPD[1], -VectorPD[2])) ;
				++oit4;
			}
			std::string buffer_counter = "_PD_Counter.vtk";
			//buffer_counter.replace(buffer3.end()-1,buffer3.end()-6,"_PD_Counter.vtk");
			counter4.WriteCounterToVTKFile( buffer_counter.c_str() ) ;
			double total4 = 0;
			for ( int i = 0; i < counter4.Getbins().size() ; i++)
			{
				total4+=counter4.Getbins()[i];
			}
			for ( int i = 0; i < counter4.Getbins().size() ; i++)
			{
				if ( (double) counter4.Getbins()[i] != 0 )
				{
					m_entropy += ((double) counter4.Getbins()[i]/total4)*(log ((double) counter4.Getbins()[i]/total4));
				}
			}
			m_entropy = -m_entropy;
			Setm_bins( counter4.Getbins() );
			std::cout << "entropy:" << m_entropy <<std::endl;
			std::cout << "bins" << counter4 << std::endl;
			SaveEntropyResult_beta( buffer2, buffer3, buffer1 );	
			
			std::string rm_DTI_name;
			rm_DTI_name.append("rm ");
			rm_DTI_name.append(DTI_name);
			std::cout << rm_DTI_name << std::endl;
			system(rm_DTI_name.c_str());
			
			return m_entropy;
			
	
}



