#include "IntensityMotionCheck.h"

#include "itkMetaDataDictionary.h"
#include "itkMetaDataObject.h"
#include "itkVectorIndexSelectionCastImageFilter.h"

#include "IntraGradientRigidRegistration.h"
#include "itkExtractImageFilter.h"
#include "RigidRegistration.h"
#include "itkImageRegionIterator.h"
#include "vnl/vnl_inverse.h"

#include <iomanip>
#include <iostream>
#include <fstream>
#include <string>
#include <math.h>


#include <vcl_algorithm.h>




//The version of DTI prep should be incremented with each algorithm changes
static const std::string DTIPREP_VERSION("1.5");

vnl_matrix_fixed<double, 3, 3>
CIntensityMotionCheck::GetMeasurementFrame(
  DwiImageType::Pointer DwiImageExtractMF)
{
  vnl_matrix_fixed<double, 3, 3> imageMeasurementFrame;
  imageMeasurementFrame.set_identity(); // Default is an indentity matrix;

  //  measurement frame
  if ( DwiImageExtractMF->GetMetaDataDictionary().HasKey(
    "NRRD_measurement frame") )
  {
    std::vector<std::vector<double> > nrrdmf;
    itk::ExposeMetaData<std::vector<std::vector<double> > >(
      DwiImageExtractMF->GetMetaDataDictionary(),
      "NRRD_measurement frame",
      nrrdmf);

    // Image frame
    for ( unsigned int row = 0; row < 3; ++row )
    {
      for ( unsigned int col = 0; col < 3; ++col )
      {
        //nrrdmf is a vector of vectors where the
        //left most index is for row, and rightmost is for col
        imageMeasurementFrame(row, col) = nrrdmf[row][col];
      }
    }
  }

  return imageMeasurementFrame;
}

CIntensityMotionCheck::CIntensityMotionCheck()
{
  m_baselineNumber    = 0;
  m_bValueNumber    = 1;
  m_repetitionNumber  = 1;
  m_gradientDirNumber  = 0;

  // Flags with _FurtherQC name are related to the Further QC process
  m_DwiOriginalImage = NULL;
  m_bDwiLoaded = false;
  m_bDwiLoaded_FurtherQC = false;		// this flag is set when new dwi loaded for Further QC process 
  m_bGetGradientDirections = false;
  m_bGetGradientDirections_FurtherQC = false;	
  // bGetGradientImages=false;

  m_DwiFileName = "";
  m_XmlFileName = "";

  protocol_load = false;



}


CIntensityMotionCheck::~CIntensityMotionCheck(){}

bool CIntensityMotionCheck::LoadDwiImage()
{
  // use with windows
  // std::string str;

  // str=m_DwiFileName.substr(0,m_DwiFileName.find_last_of('\\')+1);
  // std::cout<< str<<std::endl;
  // ::SetCurrentDirectory(str.c_str());


  if ( m_DwiFileName.length() == 0 )
  {
    std::cout << "Dwi file name not set" << std::endl;
    m_DwiOriginalImage = NULL;
    m_bDwiLoaded = false;
    return false;
  }
  else
  {
    DwiReaderType::Pointer DwiReader;
    DwiReader = DwiReaderType::New();
    try
    {
      itk::NrrdImageIO::Pointer myNrrdImageIO = itk::NrrdImageIO::New();
      DwiReader->SetImageIO(myNrrdImageIO);
      DwiReader->SetFileName(m_DwiFileName);
      std::cout << "Loading in CIntensityMotionCheck: " << m_DwiFileName
        << " ... ";
      DwiReader->Update();
    }
    catch ( itk::ExceptionObject & e )
    {
      std::cout << e.GetDescription() << std::endl;
      m_DwiOriginalImage = NULL;
      m_bDwiLoaded = false;
      return false;
    }

    std::cout << "Done " << std::endl;

    m_DwiOriginalImage = DwiReader->GetOutput();
    m_DwiForcedConformanceImage = m_DwiOriginalImage;

    m_bDwiLoaded = true;

    GetGradientDirections();

    if ( m_bGetGradientDirections )
    {
      collectDiffusionStatistics();
    }
    //   else
    //   {
    //     std::cout<< "Diffusion information read error"<<std::endl;
    //     return false;
    //   }
    //   std::cout<<"Image size"<<
    // m_DwiOriginalImage->GetLargestPossibleRegion().GetSize().GetSizeDimension()<<": ";
    //   std::cout<<m_DwiOriginalImage->GetLargestPossibleRegion().GetSize()[0]<<" ";
    //   std::cout<<m_DwiOriginalImage->GetLargestPossibleRegion().GetSize()[1]<<" ";
    //   std::cout<<m_DwiOriginalImage->GetLargestPossibleRegion().GetSize()[2]<<std::endl;

    this->m_numGradients = m_DwiOriginalImage->GetVectorLength();
    //   std::cout<<"Pixel Vector Length:
    // "<<m_DwiOriginalImage->GetVectorLength()<<std::endl;

    // Create result
    GradientIntensityMotionCheckResult result;
    result.processing = QCResult::GRADIENT_INCLUDE;
    result.VisualChecking = -1;

    qcResult->Clear();
    for ( unsigned int j = 0; j < m_DwiOriginalImage->GetVectorLength(); j++ )
    {
      result.OriginalDir[0] = this->m_GradientDirectionContainer->ElementAt(j)[0];
      result.OriginalDir[1] = this->m_GradientDirectionContainer->ElementAt(j)[1];
      result.OriginalDir[2] = this->m_GradientDirectionContainer->ElementAt(j)[2];

      result.ReplacedDir[0] = this->m_GradientDirectionContainer->ElementAt(j)[0];
      result.ReplacedDir[1] = this->m_GradientDirectionContainer->ElementAt(j)[1];
      result.ReplacedDir[2] = this->m_GradientDirectionContainer->ElementAt(j)[2];

      result.CorrectedDir[0] = this->m_GradientDirectionContainer->ElementAt(j)[0];
      result.CorrectedDir[1] = this->m_GradientDirectionContainer->ElementAt(j)[1];
      result.CorrectedDir[2] = this->m_GradientDirectionContainer->ElementAt(j)[2];

      qcResult->GetIntensityMotionCheckResult().push_back(result);
    }
    // std::cout<<"initilize the result.OriginalDir[0] and result.CorrectedDir[0]
    // "<<std::endl;
  }
  return true;
}

bool CIntensityMotionCheck::LoadDwiImage_FurtherQC( DwiImageType::Pointer  new_dwi )
{
    // This function lead the new dwi after doing visual checking 

    m_DwiOriginalImage = new_dwi;
    m_DwiForcedConformanceImage = m_DwiOriginalImage;

    m_bDwiLoaded_FurtherQC = true;

    GetGradientDirections_FurtherQC();

    if ( m_bGetGradientDirections_FurtherQC )
    {
      collectDiffusionStatistics();
    }
    //   else
    //   {
    //     std::cout<< "Diffusion information read error"<<std::endl;
    //     return false;
    //   }
    //   std::cout<<"Image size"<<
    // m_DwiOriginalImage->GetLargestPossibleRegion().GetSize().GetSizeDimension()<<": ";
    //   std::cout<<m_DwiOriginalImage->GetLargestPossibleRegion().GetSize()[0]<<" ";
    //   std::cout<<m_DwiOriginalImage->GetLargestPossibleRegion().GetSize()[1]<<" ";
    //   std::cout<<m_DwiOriginalImage->GetLargestPossibleRegion().GetSize()[2]<<std::endl;

    this->m_numGradients = m_DwiOriginalImage->GetVectorLength();
       std::cout<<"Pixel Vector Length:"<<m_DwiOriginalImage->GetVectorLength()<<std::endl;

    // Create result
    GradientIntensityMotionCheckResult result;
    result.processing = QCResult::GRADIENT_INCLUDE;
    result.VisualChecking = -1;

    qcResult->Clear();
    std::cout << "intensityMotionCheckResult.size()" << qcResult->GetIntensityMotionCheckResult().size() << std::endl;

    for ( unsigned int j = 0; j < m_DwiOriginalImage->GetVectorLength(); j++ )
    {
      result.OriginalDir[0] = this->m_GradientDirectionContainer->ElementAt(j)[0];
      result.OriginalDir[1] = this->m_GradientDirectionContainer->ElementAt(j)[1];
      result.OriginalDir[2] = this->m_GradientDirectionContainer->ElementAt(j)[2];

      result.ReplacedDir[0] = this->m_GradientDirectionContainer->ElementAt(j)[0];
      result.ReplacedDir[1] = this->m_GradientDirectionContainer->ElementAt(j)[1];
      result.ReplacedDir[2] = this->m_GradientDirectionContainer->ElementAt(j)[2];

      result.CorrectedDir[0] = this->m_GradientDirectionContainer->ElementAt(j)[0];
      result.CorrectedDir[1] = this->m_GradientDirectionContainer->ElementAt(j)[1];
      result.CorrectedDir[2] = this->m_GradientDirectionContainer->ElementAt(j)[2];

      qcResult->GetIntensityMotionCheckResult().push_back(result);
    }
    // std::cout<<"initilize the result.OriginalDir[0] and result.CorrectedDir[0]
    // "<<std::endl;
  return true;
}

bool CIntensityMotionCheck::GetGradientDirections()
{
  if ( !m_bDwiLoaded )
  {
    LoadDwiImage();
  }
  if ( !m_bDwiLoaded )
  {
    std::cout << "DWI load error, no Gradient Direction Loaded" << std::endl;
    m_bGetGradientDirections = false;
    return false;
  }

  itk::MetaDataDictionary imgMetaDictionary
    = m_DwiOriginalImage->GetMetaDataDictionary();                                            //
  std::vector<std::string> imgMetaKeys
    = imgMetaDictionary.GetKeys();
  std::vector<std::string>::const_iterator itKey = imgMetaKeys.begin();
  std::string                              metaString;

  // int numberOfImages=0;
  TensorReconstructionImageFilterType::GradientDirectionType vect3d;

  m_GradientDirectionContainer = GradientDirectionContainerType::New();
  m_GradientDirectionContainer->clear();

  for (; itKey != imgMetaKeys.end(); itKey++ )
  {
    // double x,y,z;
    itk::ExposeMetaData<std::string>(imgMetaDictionary, *itKey, metaString);
    if ( itKey->find("DWMRI_gradient") != std::string::npos )
    {
      std::istringstream iss(metaString);
      iss >> vect3d[0] >> vect3d[1] >> vect3d[2];
      // sscanf(metaString.c_str(), "%lf %lf %lf\n", &x, &y, &z);
      // vect3d[0] = x; vect3d[1] = y; vect3d[2] = z;
      m_GradientDirectionContainer->push_back(vect3d);
    }
    else if ( itKey->find("DWMRI_b-value") != std::string::npos )
    {
      m_readb0 = true;
      m_b0 = atof( metaString.c_str() );
      // std::cout<<"b Value: "<<b0<<std::endl;
    }
  }

  if ( !m_readb0 )
  {
    std::cout << "BValue not specified in header file" << std::endl;
    return false;
  }
  if ( m_GradientDirectionContainer->size() <= 6 )
  {
    std::cout << "Gradient Images Less than 7" << std::endl;
    // bGetGradientDirections=false;
    return false;
  }

  m_bGetGradientDirections = true;
  return true;
}

bool CIntensityMotionCheck::GetGradientDirections_FurtherQC()
{
  
  itk::MetaDataDictionary imgMetaDictionary
    = m_DwiOriginalImage->GetMetaDataDictionary();                                            //
  std::vector<std::string> imgMetaKeys
    = imgMetaDictionary.GetKeys();
  std::vector<std::string>::const_iterator itKey = imgMetaKeys.begin();
  std::string                              metaString;

  // int numberOfImages=0;
  TensorReconstructionImageFilterType::GradientDirectionType vect3d;

  m_GradientDirectionContainer = GradientDirectionContainerType::New();
  m_GradientDirectionContainer->clear();

  for (; itKey != imgMetaKeys.end(); itKey++ )
  {
    // double x,y,z;
    itk::ExposeMetaData<std::string>(imgMetaDictionary, *itKey, metaString);
    if ( itKey->find("DWMRI_gradient") != std::string::npos )
    {
      std::istringstream iss(metaString);
      iss >> vect3d[0] >> vect3d[1] >> vect3d[2];
      // sscanf(metaString.c_str(), "%lf %lf %lf\n", &x, &y, &z);
      // vect3d[0] = x; vect3d[1] = y; vect3d[2] = z;
      m_GradientDirectionContainer->push_back(vect3d);
    }
    else if ( itKey->find("DWMRI_b-value") != std::string::npos )
    {
      m_readb0_FurtherQC = true;
      m_b0_FurtherQC = atof( metaString.c_str() );
      // std::cout<<"b Value: "<<b0<<std::endl;
    }
  }

  if ( !m_readb0_FurtherQC )
  {
    std::cout << "BValue not specified in header file (in Further QC Process)" << std::endl;
    return false;
  }
  if ( m_GradientDirectionContainer->size() <= 6 )
  {
    std::cout << "Gradient Images Less than 7(in Further QC Process)" << std::endl;
    // bGetGradientDirections=false;
    return false;
  }

  m_bGetGradientDirections_FurtherQC = true;
  return true;
}

bool CIntensityMotionCheck::GetGradientDirections( 
  DwiImageType::Pointer dwi,
  double & bValue,
  GradientDirectionContainerType::Pointer GradDireContainer )
{
  if ( !dwi )
  {
    std::cout << "DWI error, no Gradient Direction Loaded" << std::endl;
    return false;
  }

  itk::MetaDataDictionary imgMetaDictionary
    = dwi->GetMetaDataDictionary();                                            //
  std::vector<std::string> imgMetaKeys
    = imgMetaDictionary.GetKeys();
  std::vector<std::string>::const_iterator itKey = imgMetaKeys.begin();
  std::string  metaString;

  // int numberOfImages=0;
  TensorReconstructionImageFilterType::GradientDirectionType vect3d;

  for (; itKey != imgMetaKeys.end(); itKey++ )
  {
    // double x,y,z;
    itk::ExposeMetaData<std::string>(imgMetaDictionary, *itKey, metaString);
    if ( itKey->find("DWMRI_gradient") != std::string::npos )
    {
      std::istringstream iss(metaString);
      iss >> vect3d[0] >> vect3d[1] >> vect3d[2];
      GradDireContainer->push_back(vect3d);
    }
    else if ( itKey->find("DWMRI_b-value") != std::string::npos )
    {
      bValue = atof( metaString.c_str() );
    }
  }

  if ( bValue < 0 )
  {
    std::cout << "BValue not specified in header file" << std::endl;
    return false;
  }
  if ( GradDireContainer->size() <= 6 )
  {
    std::cout << "Gradient Images Less than 7" << std::endl;
    return false;
  }

  return true;
}

void CIntensityMotionCheck::GetImagesInformation()
{
  if ( !m_bDwiLoaded )
  {
    LoadDwiImage();
  }
  if ( !m_bDwiLoaded )
  {
    std::cout << "DWI load error, no Gradient Direction Loaded" << std::endl;
    m_bGetGradientDirections = false;
    return;
  }

  itk::MetaDataDictionary imgMetaDictionary
    = m_DwiOriginalImage->GetMetaDataDictionary();                                            //
  std::vector<std::string> imgMetaKeys
    = imgMetaDictionary.GetKeys();
  std::vector<std::string>::const_iterator itKey = imgMetaKeys.begin();
  std::string metaString;

  TensorReconstructionImageFilterType::GradientDirectionType vect3d;

  GradientDirectionContainerType::Pointer GradientContainer
    = GradientDirectionContainerType::New();
  GradientContainer->clear();

  DwiImageType::SpacingType   spacing =  m_DwiOriginalImage->GetSpacing();
  DwiImageType::PointType     origin  =  m_DwiOriginalImage->GetOrigin();
  DwiImageType::DirectionType direction = m_DwiOriginalImage->GetDirection();

  int space;

  for (; itKey != imgMetaKeys.end(); itKey++ )
  {
    // double x,y,z;
    itk::ExposeMetaData<std::string>(imgMetaDictionary, *itKey, metaString);

    if ( itKey->find("DWMRI_gradient") != std::string::npos )
    {
      std::istringstream iss(metaString);
      iss >> vect3d[0] >> vect3d[1] >> vect3d[2];
      GradientContainer->push_back(vect3d);
    }
    else if ( itKey->find("DWMRI_b-value") != std::string::npos )
    {
      m_readb0 = true;
      m_b0 = atof( metaString.c_str() );
    }
    else if ( itKey->find("space") != std::string::npos )
    {
      if ( metaString.compare("right-anterior-superior") )
      {
        space = Protocol::SPACE_RAS;
      }
      else if ( metaString.compare("left-posterior-inferior") )
      {
        space = Protocol::SPACE_LPI;
      }
      else
      {
        space = Protocol::SPACE_UNKNOWN;
      }
    }
    else if ( itKey->find("modality") != std::string::npos )
    {
      if ( metaString != "DWMRI" )
      {
        std::cout << "Not a DWMRI modality!" << std::endl;
        return;
      }
    }
    else
    {
      ;
    }
  }
}


unsigned char CIntensityMotionCheck::ImageCheck( DwiImageType::Pointer localDWIImageToCheck )
// 0000 0000: ok; 
// 0000 0001: size mismatch; 
// 0000 0010: spacing mismatch; 
// 0000 0100: origins mismatch
// 0000 1000: space mismatch
// 0001 0000: Space directions mismatch        
{
  // First check
  unsigned char  returnValue = 0;
  bool        bReport = false;
  std::string ImageCheckReportFileName;

  if ( protocol->GetImageProtocol().reportFileNameSuffix.length() > 0 )
  {
    if ( protocol->GetQCOutputDirectory().length() > 0 )
    {
      if ( protocol->GetQCOutputDirectory().at( protocol->GetQCOutputDirectory()
        .length() - 1 ) == '\\'
        || protocol->GetQCOutputDirectory().at( protocol->
        GetQCOutputDirectory().length() - 1 ) == '/'     )
      {
        ImageCheckReportFileName = protocol->GetQCOutputDirectory().substr(
          0, protocol->GetQCOutputDirectory().find_last_of("/\\") );
      }
      else
      {
        ImageCheckReportFileName = protocol->GetQCOutputDirectory();
      }

      ImageCheckReportFileName.append( "/" );

      std::string str = m_DwiFileName.substr( 0, m_DwiFileName.find_last_of('.') );
      str = str.substr( str.find_last_of("/\\") + 1);

      ImageCheckReportFileName.append( str );
      ImageCheckReportFileName.append(
        protocol->GetImageProtocol().reportFileNameSuffix );
    }
    else
    {
      ImageCheckReportFileName = m_DwiFileName.substr(
        0, m_DwiFileName.find_last_of('.') );
      ImageCheckReportFileName.append(
        protocol->GetImageProtocol().reportFileNameSuffix );
    }
  }

  //   std::cout << "m_DwiFileName: " << m_DwiFileName<<std::endl;
  //   std::cout << "ImageCheckReportFileName: " <<
  // ImageCheckReportFileName<<std::endl;

  std::ofstream outfile;

  if ( protocol->GetImageProtocol().reportFileMode == 1 )
  {
    outfile.open( ImageCheckReportFileName.c_str(), std::ios_base::app);
  }
  else
  {
    outfile.open( ImageCheckReportFileName.c_str() );
  }

  if ( outfile )
  {
    bReport = true;
  }

  if ( bReport )
  {
    outfile << std::endl;
    //    outfile << "================================" << std::endl;
    //    outfile << "  Image Information checking    " << std::endl;
    //    outfile << "================================" << std::endl;
  }
  else
  {
    std::cout << "Image information check report file open failed."
      << std::endl;
  }

  QString DWIFileName_n = QString::fromStdString(m_DwiFileName);
  DWIFileName_n = DWIFileName_n.section('/',-1); // set only dwi file name to DwiName
  qcResult->GetImageInformationCheckResult().info = DWIFileName_n;
  
  if ( !protocol->GetImageProtocol().bCheck )
  {
    std::cout << "Image information check NOT set." << std::endl;
    if ( bReport )
    {
      outfile << "Image information check NOT set." << std::endl;
    }

    return true;
  }
  else
  {
    if ( !localDWIImageToCheck  )
    {
      std::cout << "DWI image error." << std::endl;
      m_bGetGradientDirections = false;
      return false;
    }
    // size
    if ( protocol->GetImageProtocol().size[0] ==
      localDWIImageToCheck->GetLargestPossibleRegion().GetSize()[0]
    && protocol->GetImageProtocol().size[1] ==
      localDWIImageToCheck->GetLargestPossibleRegion().GetSize()[1]
    && protocol->GetImageProtocol().size[2] ==
      localDWIImageToCheck->GetLargestPossibleRegion().GetSize()[2] )
    {
      qcResult->GetImageInformationCheckResult(). size = true;
      if ( bReport )
      {
        outfile << "Image_information_checking"
          << "Image_size_check " << "OK" << std::endl;
      }
      std::cout << "Image size Check: " << "\t\tOK" << std::endl;
    }
    else
    {
      qcResult->GetImageInformationCheckResult(). size = false;
      if ( bReport )
      {
        outfile << "Image_information_checking"
          << "Image_size_check " << "FAILED" << std::endl;
      }
      std::cout << "Image size Check: " << "\t\tFAILED" << std::endl;
      returnValue |= 0x00000001;
    }

    // It does not make sense to fail based on image origins
    // Diffent scan session will have different origins.
    // we still need to report this mismatch for atlas building without terminating the pipeline
    // origin
    if ( protocol->GetImageProtocol().origin[0] ==
      localDWIImageToCheck->GetOrigin()[0]
    && protocol->GetImageProtocol().origin[1] ==
      localDWIImageToCheck->GetOrigin()[1]
    && protocol->GetImageProtocol().origin[2] ==
      localDWIImageToCheck->GetOrigin()[2] )
    {
      qcResult->GetImageInformationCheckResult(). origin = true;
      if ( bReport )
      {
        outfile << "Image_information_checking"
          << "Image_origin_check " << "OK" << std::endl;
      }
      std::cout << "Image origin Check: " << "\t\tOK" << std::endl;
    }
    else
    {
      qcResult->GetImageInformationCheckResult(). origin = false;
      if ( bReport )
      {
        outfile << "Image_information_checking"
          << "Image_origin_check " << "FAILED" << std::endl;
      }
      std::cout << "Image origin Check: " << "\t\tFAILED" << std::endl;
      returnValue |= 0x00000100;
    }

    // spacing
    // Now you can set in protocol whether to exit when spacings or sizes are found mismatched
    const double spacing_tolerance=0.01;   //The numbers in the nhdr file are written in ascii
              // and are extracted from the space direction.  The tolerance can not be very
              // large or false negatives will appear.
    if ( vcl_abs( protocol->GetImageProtocol().spacing[0]
    - localDWIImageToCheck->GetSpacing()[0] ) < spacing_tolerance
      && vcl_abs( protocol->GetImageProtocol().spacing[1]
    - localDWIImageToCheck->GetSpacing()[1] ) < spacing_tolerance
      && vcl_abs( protocol->GetImageProtocol().spacing[2]
    - localDWIImageToCheck->GetSpacing()[2] ) < spacing_tolerance )
    {
      qcResult->GetImageInformationCheckResult(). spacing = true;
      if ( bReport )
      {
        outfile << "Image_information_checking"
          << "Image_spacing_check " << "OK" << std::endl;
      }
      std::cout << "Image Spacing Check: " << "\t\tOK" << std::endl;
    }
    else
    {
      qcResult->GetImageInformationCheckResult(). spacing = false;
      if ( bReport )
      {
        outfile << "Image_information_checking"
          << "Image_spacing_check " << "FAILED" << std::endl;
      }
      std::cout << "Image Spacing Check: " << "\t\tFAILED" << std::endl;
      returnValue |= 0x00000010;
    }
    // Space directions are not required to be the same.
    // Many data sets are collected with an oblique scan direction
    // space direction


    // space
    itk::MetaDataDictionary imgMetaDictionary = localDWIImageToCheck->GetMetaDataDictionary();
    std::vector<std::string> imgMetaKeys = imgMetaDictionary.GetKeys();
    std::vector<std::string>::const_iterator itKey = imgMetaKeys.begin();
    std::string                              metaString;

    itk::ExposeMetaData<std::string>(imgMetaDictionary,
      "NRRD_space",
      metaString);

    int space;
    if (     metaString.compare( "left-posterior-superior") == 0 )
    {
      space = Protocol::SPACE_LPS;
    }
    else if ( metaString.compare( "left-posterior-inferior") == 0 )
    {
      space = Protocol::SPACE_LPI;
    }
    else if ( metaString.compare( "left-anterior-superior" ) == 0 )
    {
      space = Protocol::SPACE_LAS;
    }
    else if ( metaString.compare( "left-anterior-inferior" ) == 0 )
    {
      space = Protocol::SPACE_LAI;
    }
    else if ( metaString.compare( "right-posterior-superior") == 0 )
    {
      space = Protocol::SPACE_RPS;
    }
    else if ( metaString.compare( "right-posterior-inferior") == 0 )
    {
      space = Protocol::SPACE_RPI;
    }
    else if ( metaString.compare( "right-anterior-superior" ) == 0 )
    {
      space = Protocol::SPACE_RAS;
    }
    else if ( metaString.compare( "right-anterior-inferior" ) == 0 )
    {
      space = Protocol::SPACE_RAI;
    }
    else
    {
      space = Protocol::SPACE_UNKNOWN;
    }

    //HACK:  This is OK to check for now, but it would be better to
    //       address this issue and account for it
    if ( protocol->GetImageProtocol().space == space )
    {
      qcResult->GetImageInformationCheckResult(). space = true;
      if ( bReport )
      {
        outfile << "Image_information_checking"
          << "Image_space_check " << "OK" << std::endl;
      }
      std::cout << "Image space Check: " << "\t\tOK" << std::endl;
    }
    else
    {
      qcResult->GetImageInformationCheckResult(). space = false;
      if ( bReport )
      {
        outfile << "Image_information_checking"
          << "Image_space_check " << "FAILED" << std::endl;
      }
      std::cout << "Image space Check: " << "\t\tFAILED" << std::endl;
      returnValue |= 0x00001000;      
    }
  }
  // to do: check space direction and space at the same time
  // report the mismatch

  if ( bReport )
  {
    outfile.close();
  }
  //HACK:  This should really be controlled by a command line option
  //  Now, quit on size and spacing checking failures settings are in protocol:
  //  bQuitOnCheckSizeFailure
  //  bQuitOnCheckSpacingFailure
  if( (!qcResult->GetImageInformationCheckResult(). size) && protocol->GetImageProtocol().bQuitOnCheckSizeFailure)
  {
    std::cout << "Image sizes mismatch, quit check pipeline by your settings in protocol." << std::endl;
    return returnValue; // allow to write the summary information
    //exit(-1);
  }   

  if( (!qcResult->GetImageInformationCheckResult(). spacing) && protocol->GetImageProtocol().bQuitOnCheckSpacingFailure)
  {
    std::cout << "Image spacings mismatch, quit check pipeline by your settings in protocol." << std::endl;
    return returnValue;  // allow to write the summary information
    //exit(-1);
  }    

  if ( protocol->GetImageProtocol().bCrop
    && ( !qcResult->GetImageInformationCheckResult().size ) )
  {
    this->ForceCroppingOfImage(bReport, ImageCheckReportFileName);
  }
  else
  {
    m_DwiForcedConformanceImage = localDWIImageToCheck;
  }

  return returnValue;
}

//Force the cropping of a DTI image to make it fit.
void CIntensityMotionCheck::ForceCroppingOfImage(const bool bReport, const std::string ImageCheckReportFileName )
{
  CropperType::Pointer Cropper = CropperType::New();
  Cropper->SetInput( m_DwiForcedConformanceImage );
  {
    int sizePara[3];
    sizePara[0]  = protocol->GetImageProtocol().size[0];
    sizePara[1]  = protocol->GetImageProtocol().size[1];
    sizePara[2]  = protocol->GetImageProtocol().size[2];
    Cropper->SetSize(sizePara);
  }

  if ( bReport )
  {
    Cropper->SetReportFileName( ImageCheckReportFileName ); //
    // protocol->GetImageProtocol().reportFileNameSuffix);
    Cropper->SetReportFileMode( protocol->GetImageProtocol().reportFileMode);
  }
  Cropper->Update();

  m_DwiForcedConformanceImage = Cropper->GetOutput();

  if ( protocol->GetImageProtocol().croppedDWIFileNameSuffix.length() > 0 )
  {
    try
    {
      std::string CroppedFileName;
      if ( protocol->GetImageProtocol().reportFileNameSuffix.length() > 0 )
      {
        if ( protocol->GetQCOutputDirectory().length() > 0 )
        {
          if ( protocol->GetQCOutputDirectory().at( protocol->
            GetQCOutputDirectory().length() - 1 ) == '\\'
            || protocol->GetQCOutputDirectory().at( protocol->
            GetQCOutputDirectory().length() - 1 ) == '/'     )
          {
            CroppedFileName = protocol->GetQCOutputDirectory().substr(
              0, protocol->GetQCOutputDirectory().find_last_of("/\\") );
          }
          else
          {
            CroppedFileName = protocol->GetQCOutputDirectory();
          }

          CroppedFileName.append( "/" );

          std::string str
            = m_DwiFileName.substr( 0, m_DwiFileName.find_last_of('.') );
          str = str.substr( str.find_last_of("/\\") + 1);

          CroppedFileName.append( str );
          CroppedFileName.append(
            protocol->GetImageProtocol().croppedDWIFileNameSuffix );
        }
        else
        {
          CroppedFileName
            = m_DwiFileName.substr( 0, m_DwiFileName.find_last_of('.') );
          CroppedFileName.append(
            protocol->GetImageProtocol().croppedDWIFileNameSuffix );
        }
      }

      std::cout << "Saving cropped DWI: " << CroppedFileName << " ... ";

      DwiWriterType::Pointer    DwiWriter = DwiWriterType::New();
      itk::NrrdImageIO::Pointer myNrrdImageIO = itk::NrrdImageIO::New();
      DwiWriter->SetFileName( CroppedFileName ); //
      // protocol->GetImageProtocol().croppedDWIFileNameSuffix
      // );
      DwiWriter->SetInput( m_DwiForcedConformanceImage );
      DwiWriter->UseCompressionOn();
      DwiWriter->Update();
    }
    catch ( itk::ExceptionObject & e )
    {
      std::cout << e.GetDescription() << std::endl;
      // return -1;
    }
    std::cout << "DONE." << std::endl;
  }
}


bool CIntensityMotionCheck::SliceWiseCheck( DwiImageType::Pointer dwi )
{
  bool ret = true;
  if ( protocol->GetSliceCheckProtocol().bCheck )
  {
    std::string ReportFileName;
    if ( protocol->GetSliceCheckProtocol().reportFileNameSuffix.length() > 0 )
    {
      if ( protocol->GetQCOutputDirectory().length() > 0 )
      {
        if ( protocol->GetQCOutputDirectory().at( protocol->
          GetQCOutputDirectory().length() - 1 ) == '\\'
          || protocol->GetQCOutputDirectory().at( protocol->
          GetQCOutputDirectory().length() - 1 ) == '/'     )
        {
          ReportFileName = protocol->GetQCOutputDirectory().substr(
            0, protocol->GetQCOutputDirectory().find_last_of("/\\") );
        }
        else
        {
          ReportFileName = protocol->GetQCOutputDirectory();
        }

        ReportFileName.append( "/" );

        std::string str = m_DwiFileName.substr( 0, m_DwiFileName.find_last_of('.') );
        str = str.substr( str.find_last_of("/\\") + 1);

        ReportFileName.append( str );
        ReportFileName.append(
          protocol->GetSliceCheckProtocol().reportFileNameSuffix );
      }
      else
      {
        ReportFileName = m_DwiFileName.substr( 0, m_DwiFileName.find_last_of('.') );
        ReportFileName.append(
          protocol->GetSliceCheckProtocol().reportFileNameSuffix );
      }

      //       ReportFileName=m_DwiFileName.substr(0,m_DwiFileName.find_last_of('.')
      // );
      //       ReportFileName.append(
      // protocol->GetSliceCheckProtocol().reportFileNameSuffix );
    }

    SliceCheckerType::Pointer SliceChecker = SliceCheckerType::New();
    SliceChecker->SetInput( dwi );
    SliceChecker->SetCheckTimes( protocol->GetSliceCheckProtocol().checkTimes );
    SliceChecker->SetHeadSkipRatio(
      protocol->GetSliceCheckProtocol().headSkipSlicePercentage );
    SliceChecker->SetTailSkipRatio(
      protocol->GetSliceCheckProtocol().tailSkipSlicePercentage );
    SliceChecker->SetBaselineStdevTimes(
      protocol->GetSliceCheckProtocol().correlationDeviationThresholdbaseline );
    SliceChecker->SetGradientStdevTimes(
      protocol->GetSliceCheckProtocol().correlationDeviationThresholdgradient );
    SliceChecker->SetReportFileName( ReportFileName );
    SliceChecker->SetReportFileMode(
      protocol->GetSliceCheckProtocol().reportFileMode );
    SliceChecker->SetReportType(protocol->GetReportType() );

    SliceChecker->SetQuadFit(0);

    SliceChecker->SetSubRegionalCheck( protocol->GetSliceCheckProtocol().bSubregionalCheck );
    SliceChecker->SetSubregionalCheckRelaxationFactor( protocol->GetSliceCheckProtocol().subregionalCheckRelaxationFactor );
    // results get worse using smoothing
    //     SliceChecker->SetSmoothing( false );
    //     SliceChecker->SetGaussianVariance( 4.0 );
    //     SliceChecker->SetMaxKernelWidth( 8.0 );

    try
    {
      SliceChecker->Update();
    }
    catch ( itk::ExceptionObject & e )
    {
      std::cout << e.GetDescription() << std::endl;
    }

    m_DwiForcedConformanceImage = SliceChecker->GetOutput();

    // .......Mapping between input gradeints and DWIForcedComformance gradeints
    // New : 16 Jun 2011: 
    /*Original_ForcedConformance_Mapping t_ForcedMapping;
    int id_Forced = 0;
    for ( int id = 0 ; id < SliceChecker->getQCResults().size() ; id++ )
    {
      if ( SliceChecker->getQCResults()[id] )
      {
	std::cout << "id included SliceWise: " << id << std::endl;
        std::vector <int> m_id;
	m_id.push_back( id );
	t_ForcedMapping.index_original = m_id;
	t_ForcedMapping.index_ForcedConformance = id_Forced;
        m_Original_ForcedConformance_Mapping.push_back( t_ForcedMapping );
	id_Forced++;
      }
    }*/
    std::vector<bool>	tem_vector = SliceChecker->getQCResults();

    //std::cout << "SliceChecker_qcResult Size:" << SliceChecker->getQCResults().size() << std::endl;
    //for ( int jj=0 ; jj< SliceChecker->getQCResults().size() ; jj++ )
    //{
    //std::cout << "SliceChecker_qcResult:" << SliceChecker->getQCResults()[jj] << std::endl;
    //}

    unsigned int id = 0;
    while (  id < tem_vector.size()  )
    {
      if ( tem_vector[id] == 0 )
      {
	this->qcResult->GetIntensityMotionCheckResult()[(m_Original_ForcedConformance_Mapping[id].index_original)[0]].processing
                = QCResult::GRADIENT_EXCLUDE_SLICECHECK;
	tem_vector.erase( tem_vector.begin()+id );
        m_Original_ForcedConformance_Mapping.erase( m_Original_ForcedConformance_Mapping.begin()+id );
	id = -1;
      }
      id++;
    }
    //........

    SliceWiseCheckResult SliceWise;    //Updating qcresult for SliceWise cheking information
    
    for (unsigned int i=0; i < SliceChecker->GetSliceWiseCheckResult().size(); i++)
    {
    SliceWise.GradientNum=SliceChecker->GetSliceWiseCheckResult()[i].GradientNum;
    SliceWise.SliceNum=SliceChecker->GetSliceWiseCheckResult()[i].SliceNum;
    SliceWise.Correlation=SliceChecker->GetSliceWiseCheckResult()[i].Correlation;

    qcResult->GetSliceWiseCheckResult().push_back(SliceWise);
    
    }
    
    // update the QCResults
    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    //Wrong: This parts must be changed since the results of Interlace QC are not matched with this->qcResult --> Look at the New: June 2011
    /*for ( unsigned int i = 0;
      i < SliceChecker->GetGradientDirectionContainer()->size();
      i++ )
    {
      for ( unsigned int j = 0;
        j < this->qcResult->GetIntensityMotionCheckResult().size();
        j++ )
      {
        if ( SliceChecker->GetGradientDirectionContainer()->at(i)[0] ==
          this->qcResult->GetIntensityMotionCheckResult()[j].ReplacedDir[0]
        && SliceChecker->GetGradientDirectionContainer()->at(i)[1] ==
          this->qcResult->GetIntensityMotionCheckResult()[j].ReplacedDir[1]
        && SliceChecker->GetGradientDirectionContainer()->at(i)[2] ==
          this->qcResult->GetIntensityMotionCheckResult()[j].ReplacedDir[2]  )
        {
          if ( this->qcResult->GetIntensityMotionCheckResult()[j].processing !=
            QCResult::GRADIENT_INCLUDE )
          {
            // std::cout<< "gradient " << i << " has been excluded!"
            // <<std::endl;
          }
          else
          {
            if (  !SliceChecker->getQCResults()[i] )      
            {
              this->qcResult->GetIntensityMotionCheckResult()[j].processing
                = QCResult::GRADIENT_EXCLUDE_SLICECHECK;
            }
          }
	  
        }
      }
    }*/

    // save slicechecked DWI
    if ( protocol->GetSliceCheckProtocol().outputDWIFileNameSuffix.length() > 0 )
    {
      std::string SliceWiseOutput;
      if ( protocol->GetQCOutputDirectory().length() > 0 )
      {
        if ( protocol->GetQCOutputDirectory().at( protocol->
          GetQCOutputDirectory().length() - 1 ) == '\\'
          || protocol->GetQCOutputDirectory().at( protocol->
          GetQCOutputDirectory().length() - 1 ) == '/'     )
        {
          SliceWiseOutput = protocol->GetQCOutputDirectory().substr(
            0, protocol->GetQCOutputDirectory().find_last_of("/\\") );
        }
        else
        {
          SliceWiseOutput = protocol->GetQCOutputDirectory();
        }

        SliceWiseOutput.append( "/" );

        std::string str = m_DwiFileName.substr( 0, m_DwiFileName.find_last_of('.') );
        str = str.substr( str.find_last_of("/\\") + 1);

        SliceWiseOutput.append( str );
        SliceWiseOutput.append(
          protocol->GetSliceCheckProtocol().outputDWIFileNameSuffix );
      }
      else
      {
        SliceWiseOutput = m_DwiFileName.substr( 0, m_DwiFileName.find_last_of('.') );
        SliceWiseOutput.append(
          protocol->GetSliceCheckProtocol().outputDWIFileNameSuffix );
      }

      try
      {
        std::cout << "Saving output of slice check: " << SliceWiseOutput
          << " ... ";
        DwiWriterType::Pointer DwiWriter = DwiWriterType::New();
        itk::NrrdImageIO::Pointer myNrrdImageIO = itk::NrrdImageIO::New();
        DwiWriter->SetImageIO(myNrrdImageIO);
        DwiWriter->SetFileName( SliceWiseOutput );
        DwiWriter->SetInput( m_DwiForcedConformanceImage );
        DwiWriter->UseCompressionOn();
        DwiWriter->Update();
      }
      catch ( itk::ExceptionObject & e )
      {
        std::cout << e.GetDescription() << std::endl;
      }
      std::cout << "DONE" << std::endl;
    }

    // save the excluded gradients to a nrrd file
    if ( ! SliceChecker->GetExcludedGradients()  )
    {
      std::cout << "No excluded gradient file created." << std::endl;   
    }
    else
    {
      if ( protocol->GetSliceCheckProtocol().excludedDWINrrdFileNameSuffix.length() > 0 )
      {
        std::string SliceWiseExcludeOutput;
        if ( protocol->GetQCOutputDirectory().length() > 0 )
        {
          if ( protocol->GetQCOutputDirectory().at( protocol->
            GetQCOutputDirectory().length() - 1 ) == '\\'
            || protocol->GetQCOutputDirectory().at( protocol->
            GetQCOutputDirectory().length() - 1 ) == '/'     )
          {
            SliceWiseExcludeOutput = protocol->GetQCOutputDirectory().substr(
              0, protocol->GetQCOutputDirectory().find_last_of("/\\") );
          }
          else
          {
            SliceWiseExcludeOutput = protocol->GetQCOutputDirectory();
          }

          SliceWiseExcludeOutput.append( "/" );

          std::string str = m_DwiFileName.substr( 0, m_DwiFileName.find_last_of('.') );
          str = str.substr( str.find_last_of("/\\") + 1);

          SliceWiseExcludeOutput.append( str );
          SliceWiseExcludeOutput.append(
            protocol->GetSliceCheckProtocol().excludedDWINrrdFileNameSuffix );
        }
        else
        {
          SliceWiseExcludeOutput = m_DwiFileName.substr( 0, m_DwiFileName.find_last_of('.') );
          SliceWiseExcludeOutput.append(
            protocol->GetSliceCheckProtocol().excludedDWINrrdFileNameSuffix );
        }

        try
        {
          std::cout << "Saving excluded gradients of slice check: " << SliceWiseExcludeOutput
            << " ... ";
          DwiWriterType::Pointer DwiWriter = DwiWriterType::New();
          itk::NrrdImageIO::Pointer myNrrdImageIO = itk::NrrdImageIO::New();
          DwiWriter->SetImageIO(myNrrdImageIO);
          DwiWriter->SetFileName( SliceWiseExcludeOutput );
          DwiWriter->SetInput( SliceChecker->GetExcludedGradients() );
          DwiWriter->UseCompressionOn();
          DwiWriter->Update();
        }
        catch ( itk::ExceptionObject & e )
        {
          std::cout << e.GetDescription() << std::endl;
        }
        std::cout << "DONE" << std::endl;
      }
    }

    //validate the SliceWise output
    std::ofstream outfile; 
    outfile.open(ReportFileName.c_str(), std::ios::app);

    //outfile<<"=="<<std::endl;
    //outfile<<"SliceWisw check summary:"<<std::endl;

    //std::cout<<"=="<<std::endl;
    //std::cout<<"SliceWisw check summary:"<<std::endl;

    if(SliceChecker->getGradientDirLeftNumber()<6)
    {
      outfile<<"  Gradient direction # is less than 6!"<<std::endl;
      std::cout<<"  Gradient direction # is less than 6!"<<std::endl;
      ret = false;
    }

    if(SliceChecker->getBaselineLeftNumber()==0 && SliceChecker->getBValueLeftNumber()==1)
    {
      outfile<<"  Single b-value DWI without a b0/baseline!"<<std::endl;
      std::cout<<"  Single b-value DWI without a b0/baseline!"<<std::endl;
      ret = false;
    }

    if( ((SliceChecker->getGradientDirNumber())-(SliceChecker->getGradientDirLeftNumber())) 
    > protocol->GetBadGradientPercentageTolerance()* (SliceChecker->getGradientDirNumber()))
    {
      outfile<<"  Too many bad gradient directions found!"<<std::endl;
      std::cout<<"  Too many bad gradient directions found!"<<std::endl;
      ret = false;
    }
  }
  else
  {
    std::cout << "Slice-wise check NOT set." << std::endl;
  }

  return ret;
}

bool CIntensityMotionCheck::InterlaceWiseCheck( DwiImageType::Pointer dwi )
{
  bool ret = true;
  if ( protocol->GetInterlaceCheckProtocol().bCheck )
  {
    std::string ReportFileName;
    if ( protocol->GetInterlaceCheckProtocol().reportFileNameSuffix.length() >
      0 )
    {
      if ( protocol->GetQCOutputDirectory().length() > 0 )
      {
        if ( protocol->GetQCOutputDirectory().at( protocol->
          GetQCOutputDirectory().length() - 1 ) == '\\'
          || protocol->GetQCOutputDirectory().at( protocol->
          GetQCOutputDirectory().length() - 1 ) == '/'     )
        {
          ReportFileName = protocol->GetQCOutputDirectory().substr(
            0, protocol->GetQCOutputDirectory().find_last_of("/\\") );
        }
        else
        {
          ReportFileName = protocol->GetQCOutputDirectory();
        }

        ReportFileName.append( "/" );

        std::string str = m_DwiFileName.substr( 0, m_DwiFileName.find_last_of('.') );
        str = str.substr( str.find_last_of("/\\") + 1);

        ReportFileName.append( str );
        ReportFileName.append(
          protocol->GetInterlaceCheckProtocol().reportFileNameSuffix );
      }
      else
      {
        ReportFileName = m_DwiFileName.substr( 0, m_DwiFileName.find_last_of('.') );
        ReportFileName.append(
          protocol->GetInterlaceCheckProtocol().reportFileNameSuffix );
      }

      //       ReportFileName=m_DwiFileName.substr(0,m_DwiFileName.find_last_of('.')
      // );
      //       ReportFileName.append(
      // protocol->GetInterlaceCheckProtocol().reportFileNameSuffix );
    }

    InterlaceCheckerType::Pointer InterlaceChecker = InterlaceCheckerType::New();
    InterlaceChecker->SetInput( dwi );
    InterlaceChecker->SetCorrelationThresholdBaseline(
      protocol->GetInterlaceCheckProtocol().correlationThresholdBaseline );
    InterlaceChecker->SetCorrelationThresholdGradient(
      protocol->GetInterlaceCheckProtocol().correlationThresholdGradient );
    InterlaceChecker->SetCorrelationStedvTimesBaseline(
      protocol->GetInterlaceCheckProtocol().correlationDeviationBaseline );
    InterlaceChecker->SetCorrelationStdevTimesGradient(
      protocol->GetInterlaceCheckProtocol().correlationDeviationGradient );
    InterlaceChecker->SetTranslationThreshold(
      protocol->GetInterlaceCheckProtocol().translationThreshold );
    InterlaceChecker->SetRotationThreshold( protocol->GetInterlaceCheckProtocol(
      ).rotationThreshold );
    InterlaceChecker->SetReportFileName( ReportFileName );
    InterlaceChecker->SetReportFileMode(
      protocol->GetInterlaceCheckProtocol().reportFileMode );
    InterlaceChecker->SetReportType(protocol->GetReportType());

    try
    {
      InterlaceChecker->Update();
    }
    catch ( itk::ExceptionObject & e )
    {
      std::cout << e.GetDescription() << std::endl;
      return -1;
    }

    m_DwiForcedConformanceImage = InterlaceChecker->GetOutput();

    // .......Mapping between input gradeints and DWIForcedComformance gradeints
    // New : 16 Jun 2011
    std::vector<bool>	tem_vector = InterlaceChecker->getQCResults();

    //std::cout << "InterlaceWise_qcResult Size:" << InterlaceChecker->getQCResults().size() << std::endl;
    //for ( int jj=0 ; jj< InterlaceChecker->getQCResults().size() ; jj++ )
    //{
    //std::cout << "InterlaceWise_qcResult:" << InterlaceChecker->getQCResults()[jj] << std::endl;
    //}

    unsigned int id = 0;
    while ( id < tem_vector.size() )
    {
      if ( tem_vector[id] == 0 )
      {
	this->qcResult->GetIntensityMotionCheckResult()[(m_Original_ForcedConformance_Mapping[id].index_original)[0]].processing
                = QCResult::GRADIENT_EXCLUDE_INTERLACECHECK;
	std::cout << "Original Excluded id: " << (m_Original_ForcedConformance_Mapping[id].index_original)[0] << "id: " << id << std::endl;
	tem_vector.erase( tem_vector.begin()+id );
        m_Original_ForcedConformance_Mapping.erase( m_Original_ForcedConformance_Mapping.begin()+id );
	id = -1;
      }
      id ++;
    }
    //........

    InterlaceWiseCheckResult InterlaceResult;	////Updating qcresult for Interlace cheking information


    for (unsigned int i=0; i < InterlaceChecker->GetResultsContainer().size(); i++)
    {
    InterlaceResult.AngleX=InterlaceChecker->GetResultsContainer()[i].AngleX;
    InterlaceResult.AngleY=InterlaceChecker->GetResultsContainer()[i].AngleY;
    InterlaceResult.AngleZ=InterlaceChecker->GetResultsContainer()[i].AngleZ;
    InterlaceResult.TranslationX=InterlaceChecker->GetResultsContainer()[i].TranslationX;
    InterlaceResult.TranslationY=InterlaceChecker->GetResultsContainer()[i].TranslationY;
    InterlaceResult.TranslationZ=InterlaceChecker->GetResultsContainer()[i].TranslationZ;
    InterlaceResult.Metric=InterlaceChecker->GetResultsContainer()[i].Metric;
    InterlaceResult.Correlation=InterlaceChecker->GetResultsContainer()[i].Correlation;

    qcResult->GetInterlaceWiseCheckResult().push_back(InterlaceResult);
    
    }

    // update the QCResults
    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%/
    //Wrong: This parts must be changed since the results of Interlace QC are not matched with this->qcResult look at New jun 2011
    /*for ( unsigned int i = 0;
      i < InterlaceChecker->GetGradientDirectionContainer()->size();
      i++ )
    {
      for ( unsigned int j = 0;
        j < this->qcResult->GetIntensityMotionCheckResult().size();
        j++ )
      {
        if ( InterlaceChecker->GetGradientDirectionContainer()->at(i)[0] ==
          this->qcResult->GetIntensityMotionCheckResult()[j].ReplacedDir[0]
        && InterlaceChecker->GetGradientDirectionContainer()->at(i)[1] ==
          this->qcResult->GetIntensityMotionCheckResult()[j].ReplacedDir[1]
        && InterlaceChecker->GetGradientDirectionContainer()->at(i)[2] ==
          this->qcResult->GetIntensityMotionCheckResult()[j].ReplacedDir[2]  )
        {
          if ( this->qcResult->GetIntensityMotionCheckResult()[j].processing !=
            QCResult::GRADIENT_INCLUDE )
          {
            // std::cout<< "gradient " << i << " has been excluded!"
            // <<std::endl;
          }
          else
          {
            if (  !InterlaceChecker->getQCResults()[i] )
            {
              this->qcResult->GetIntensityMotionCheckResult()[j].processing     //??????????????????????/
                = QCResult::GRADIENT_EXCLUDE_INTERLACECHECK;
            }
          }
        }
      }
    } */
    

    //     for( unsigned int i=0;
    // i<this->qcResult->GetIntensityMotionCheckResult().size(); i++)
    //     {
    //       if(  !InterlaceChecker->getQCResults()[i])
    //         this->qcResult->GetIntensityMotionCheckResult()[i].processing =
    // QCResult::GRADIENT_EXCLUDE_INTERLACECHECK;
    //     }

    // save the output 
    if ( protocol->GetInterlaceCheckProtocol().outputDWIFileNameSuffix.length()
        > 0 )
    {
      std::string outputDWIFileName;
      //
      //
      //
      //
      //    outputDWIFileName=m_DwiFileName.substr(0,m_DwiFileName.find_last_of('.')
      // );
      //       outputDWIFileName.append(
      // protocol->GetInterlaceCheckProtocol().outputDWIFileNameSuffix );
      if ( protocol->GetQCOutputDirectory().length() > 0 )
      {
        if ( protocol->GetQCOutputDirectory().at( protocol->
          GetQCOutputDirectory().length() - 1 ) == '\\'
          || protocol->GetQCOutputDirectory().at( protocol->
          GetQCOutputDirectory().length() - 1 ) == '/'     )
        {
          outputDWIFileName = protocol->GetQCOutputDirectory().substr(
            0, protocol->GetQCOutputDirectory().find_last_of("/\\") );
        }
        else
        {
          outputDWIFileName = protocol->GetQCOutputDirectory();
        }

        outputDWIFileName.append( "/" );

        std::string str = m_DwiFileName.substr( 0, m_DwiFileName.find_last_of('.') );
        str = str.substr( str.find_last_of("/\\") + 1);

        outputDWIFileName.append( str );
        outputDWIFileName.append(
          protocol->GetInterlaceCheckProtocol().outputDWIFileNameSuffix );
      }
      else
      {
        outputDWIFileName = m_DwiFileName.substr( 0, m_DwiFileName.find_last_of('.') );
        outputDWIFileName.append(
          protocol->GetInterlaceCheckProtocol().outputDWIFileNameSuffix );
      }
      try
      {
        std::cout << "Saving output of interlace check: "
          << outputDWIFileName << " ... ";
        DwiWriterType::Pointer DwiWriter = DwiWriterType::New();
        itk::NrrdImageIO::Pointer myNrrdImageIO = itk::NrrdImageIO::New();
        DwiWriter->SetImageIO(myNrrdImageIO);
        DwiWriter->SetFileName( outputDWIFileName );
        DwiWriter->SetInput( m_DwiForcedConformanceImage );
        DwiWriter->UseCompressionOn();
        DwiWriter->Update();
      }
      catch ( itk::ExceptionObject & e )
      {
        std::cout << e.GetDescription() << std::endl;
        return -1;
      }
      std::cout << "DONE" << std::endl;
    }

    // save the excluded gradients to a nrrd file
    if ( ! InterlaceChecker->GetExcludedGradients() )
    {
      std::cout << "No excluded gradient file created." << std::endl;   
    }
    else
    {
      if ( protocol->GetInterlaceCheckProtocol().excludedDWINrrdFileNameSuffix.length() > 0 )
      {
        std::string InterlaceWiseExcludeOutput;
        if ( protocol->GetQCOutputDirectory().length() > 0 )
        {
          if ( protocol->GetQCOutputDirectory().at( protocol->
            GetQCOutputDirectory().length() - 1 ) == '\\'
            || protocol->GetQCOutputDirectory().at( protocol->
            GetQCOutputDirectory().length() - 1 ) == '/'     )
          {
            InterlaceWiseExcludeOutput = protocol->GetQCOutputDirectory().substr(
              0, protocol->GetQCOutputDirectory().find_last_of("/\\") );
          }
          else
          {
            InterlaceWiseExcludeOutput = protocol->GetQCOutputDirectory();
          }

          InterlaceWiseExcludeOutput.append( "/" );

          std::string str = m_DwiFileName.substr( 0, m_DwiFileName.find_last_of('.') );
          str = str.substr( str.find_last_of("/\\") + 1);

          InterlaceWiseExcludeOutput.append( str );
          InterlaceWiseExcludeOutput.append(
            protocol->GetInterlaceCheckProtocol().excludedDWINrrdFileNameSuffix );
        }
        else
        {
          InterlaceWiseExcludeOutput = m_DwiFileName.substr( 0, m_DwiFileName.find_last_of('.') );
          InterlaceWiseExcludeOutput.append(
            protocol->GetInterlaceCheckProtocol().excludedDWINrrdFileNameSuffix );
        }

        try
        {
          std::cout << "Saving excluded gradients of interlace check: " << InterlaceWiseExcludeOutput
            << " ... ";
          DwiWriterType::Pointer DwiWriter = DwiWriterType::New();
          itk::NrrdImageIO::Pointer myNrrdImageIO = itk::NrrdImageIO::New();
          DwiWriter->SetImageIO(myNrrdImageIO);
          DwiWriter->SetFileName( InterlaceWiseExcludeOutput );
          DwiWriter->SetInput( InterlaceChecker->GetExcludedGradients() );
          DwiWriter->UseCompressionOn();
          DwiWriter->Update();
        }
        catch ( itk::ExceptionObject & e )
        {
          std::cout << e.GetDescription() << std::endl;
        }
        std::cout << "DONE" << std::endl;
      }
    }

    //validate the interlace Wise output
    std::ofstream outfile; 
    outfile.open(ReportFileName.c_str(), std::ios::app);

    //outfile<<"=="<<std::endl;
    //outfile<<"InterlaceWisw check summary:"<<std::endl;

    //std::cout<<"=="<<std::endl;
    //std::cout<<"InterlaceWisw check summary::"<<std::endl;

    if(InterlaceChecker->getGradientDirLeftNumber()<6)
    {
      outfile<<"  Gradient direction # is less than 6!"<<std::endl;
      std::cout<<"  Gradient direction # is less than 6!"<<std::endl;
      ret = false;
    }

    if(InterlaceChecker->getBaselineLeftNumber()==0 && InterlaceChecker->getBValueLeftNumber()==1)
    {
      outfile<<"  Single b-value DWI without a b0/baseline!"<<std::endl;
      std::cout<<"  Single b-value DWI without a b0/baseline!"<<std::endl;
      ret = false;
    }

    if( ((InterlaceChecker->getGradientDirNumber())-(InterlaceChecker->getGradientDirLeftNumber())) 
    > protocol->GetBadGradientPercentageTolerance()* (InterlaceChecker->getGradientDirNumber()))
    {
      outfile<<"  Too many bad gradient directions found!"<<std::endl;
      std::cout<<"  Too many bad gradient directions found!"<<std::endl;
      ret = false;
    }
  }
  else
  {
    std::cout << "Interlace-wise check NOT set." << std::endl;
  }

  return ret;
}

bool CIntensityMotionCheck::BaselineAverage( DwiImageType::Pointer dwi )
{
  if ( protocol->GetBaselineAverageProtocol().bAverage )
  {
    std::string ReportFileName;
    if ( protocol->GetBaselineAverageProtocol().reportFileNameSuffix.length() >
      0 )
    {
      //       ReportFileName=m_DwiFileName.substr(0,m_DwiFileName.find_last_of('.')
      // );
      //       ReportFileName.append(
      // protocol->GetBaselineAverageProtocol().reportFileNameSuffix );
      if ( protocol->GetQCOutputDirectory().length() > 0 )
      {
        if ( protocol->GetQCOutputDirectory().at( protocol->
          GetQCOutputDirectory().length() - 1 ) == '\\'
          || protocol->GetQCOutputDirectory().at( protocol->
          GetQCOutputDirectory().length() - 1 ) == '/'     )
        {
          ReportFileName = protocol->GetQCOutputDirectory().substr(
            0, protocol->GetQCOutputDirectory().find_last_of("/\\") );
        }
        else
        {
          ReportFileName = protocol->GetQCOutputDirectory();
        }

        ReportFileName.append( "/" );

        std::string str = m_DwiFileName.substr( 0, m_DwiFileName.find_last_of('.') );
        str = str.substr( str.find_last_of("/\\") + 1);

        ReportFileName.append( str );
        ReportFileName.append(
          protocol->GetBaselineAverageProtocol().reportFileNameSuffix );
      }
      else
      {
        ReportFileName = m_DwiFileName.substr( 0, m_DwiFileName.find_last_of('.') );
        ReportFileName.append(
          protocol->GetBaselineAverageProtocol().reportFileNameSuffix );
      }
    }

    BaselineAveragerType::Pointer BaselineAverager = BaselineAveragerType::New();
    BaselineAverager->SetInput( dwi );
    BaselineAverager->SetReportFileName( ReportFileName );
    BaselineAverager->SetReportFileMode(
      protocol->GetBaselineAverageProtocol().reportFileMode);
    BaselineAverager->SetAverageMethod(
      protocol->GetBaselineAverageProtocol().averageMethod );
    BaselineAverager->SetStopThreshold(
      protocol->GetBaselineAverageProtocol().stopThreshold );
    BaselineAverager->SetMaxIteration( 15 );
    BaselineAverager->SetReportType(protocol->GetReportType() );

    try
    {
      BaselineAverager->Update();
    } 
    catch ( itk::ExceptionObject & e )
    {
      std::cout << e.GetDescription() << std::endl;
      return -1;
    }

    Setm_DwiForcedConformanceImage(BaselineAverager->GetOutput());

    // .......Mapping between input gradeints and DWIForcedComformance gradeints
    //New : jun 2011
    if ( BaselineAverager->getBaselineNumber()>1 )	// multiple baselines
    {
	std::vector<bool>	tem_vector = BaselineAverager->getGradient_indx_Baselines();
	std::vector<int>	B0_indices;
	Original_ForcedConformance_Mapping  m_B0s;
	int id = 0;
    	while ( id < tem_vector.size() )
    	{
      		if ( tem_vector[id] == 1 )
      		{
		if ( this->qcResult->GetIntensityMotionCheckResult()[(m_Original_ForcedConformance_Mapping[id].index_original)[0]].processing == 		     QCResult::GRADIENT_INCLUDE )
		{
			this->qcResult->GetIntensityMotionCheckResult()[(m_Original_ForcedConformance_Mapping[id].index_original)[0]].processing
			= QCResult::GRADIENT_BASELINE_AVERAGED;		
		}
		tem_vector.erase( tem_vector.begin()+id );
		B0_indices.push_back(m_Original_ForcedConformance_Mapping[ id ].index_original[0]);
        	m_Original_ForcedConformance_Mapping.erase( m_Original_ForcedConformance_Mapping.begin()+id );
		id = -1;
      		}
	id++;
    	}
	m_B0s.index_original = B0_indices;
	m_B0s.index_ForcedConformance = -1;
	m_Original_ForcedConformance_Mapping.insert( m_Original_ForcedConformance_Mapping.begin(), m_B0s );
    }
    // .....

    //Updating qcResult
    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%/
    //Wrong: This parts must be changed since the results of BaselibeAvg QC are not matched with this->qcResult (Report) look at New jun 2011
    /*for ( unsigned int j = 0;
      j < this->qcResult->GetIntensityMotionCheckResult().size();
      j++ )
    {
      if ( 0.0 ==
        this->qcResult->GetIntensityMotionCheckResult()[j].ReplacedDir[0]
      && 0.0 ==
        this->qcResult->GetIntensityMotionCheckResult()[j].ReplacedDir[1]
      && 0.0 ==
        this->qcResult->GetIntensityMotionCheckResult()[j].ReplacedDir[2]  )
      {
        if ( this->qcResult->GetIntensityMotionCheckResult()[j].processing ==
          QCResult::GRADIENT_INCLUDE )
        {
          this->qcResult->GetIntensityMotionCheckResult()[j].processing
            = QCResult::GRADIENT_BASELINE_AVERAGED;
        }
      }
    }*/

    if ( protocol->GetBaselineAverageProtocol().outputDWIFileNameSuffix.length()
      > 0 )
    {
      std::string outputDWIFileName;
      //
      //
      //
      //
      //    outputDWIFileName=m_DwiFileName.substr(0,m_DwiFileName.find_last_of('.')
      // );
      //       outputDWIFileName.append(
      // protocol->GetBaselineAverageProtocol().outputDWIFileNameSuffix );
      if ( protocol->GetQCOutputDirectory().length() > 0 )
      {
        if ( protocol->GetQCOutputDirectory().at( protocol->
          GetQCOutputDirectory().length() - 1 ) == '\\'
          || protocol->GetQCOutputDirectory().at( protocol->
          GetQCOutputDirectory().length() - 1 ) == '/'     )
        {
          outputDWIFileName = protocol->GetQCOutputDirectory().substr(
            0, protocol->GetQCOutputDirectory().find_last_of("/\\") );
        }
        else
        {
          outputDWIFileName = protocol->GetQCOutputDirectory();
        }

        outputDWIFileName.append( "/" );

        std::string str = m_DwiFileName.substr( 0, m_DwiFileName.find_last_of('.') );
        str = str.substr( str.find_last_of("/\\") + 1);

        outputDWIFileName.append( str );
        outputDWIFileName.append(
          protocol->GetBaselineAverageProtocol().outputDWIFileNameSuffix );
      }
      else
      {
        outputDWIFileName = m_DwiFileName.substr( 0, m_DwiFileName.find_last_of('.') );
        outputDWIFileName.append(
          protocol->GetBaselineAverageProtocol().outputDWIFileNameSuffix );
      }

      try
      {
        std::cout << "Saving output of baseline average: "
          << outputDWIFileName << " ... ";
        DwiWriterType::Pointer DwiWriter = DwiWriterType::New();
        itk::NrrdImageIO::Pointer myNrrdImageIO = itk::NrrdImageIO::New();
        DwiWriter->SetImageIO(myNrrdImageIO);
        DwiWriter->SetFileName( outputDWIFileName );
        DwiWriter->SetInput( m_DwiForcedConformanceImage );
        DwiWriter->UseCompressionOn();
        DwiWriter->Update();
      }
      catch ( itk::ExceptionObject & e )
      {
        std::cout << e.GetDescription() << std::endl;
        return -1;
      }
      std::cout << "DONE" << std::endl;
    }
  }
  else
  {
    std::cout << "Baseline average NOT set." << std::endl;
  }

  return true;
}

bool CIntensityMotionCheck::EddyMotionCorrectIowa( DwiImageType::Pointer dwi )
{
  if ( protocol->GetEddyMotionCorrectionProtocol().bCorrect )
  {
    std::cout << "Eddy-current and head motion correction using IOWA tool."
      << std::endl;

    std::string ReportFileName;
    if ( protocol->GetEddyMotionCorrectionProtocol().reportFileNameSuffix.
      length() > 0 )
    {
      //       ReportFileName=m_DwiFileName.substr(0,m_DwiFileName.find_last_of('.')
      // );
      //       ReportFileName.append(
      // protocol->GetEddyMotionCorrectionProtocol().reportFileNameSuffix );
      if ( protocol->GetQCOutputDirectory().length() > 0 )
      {
        if ( protocol->GetQCOutputDirectory().at( protocol->
          GetQCOutputDirectory().length() - 1 ) == '\\'
          || protocol->GetQCOutputDirectory().at( protocol->
          GetQCOutputDirectory().length() - 1 ) == '/'     )
        {
          ReportFileName = protocol->GetQCOutputDirectory().substr(
            0, protocol->GetQCOutputDirectory().find_last_of("/\\") );
        }
        else
        {
          ReportFileName = protocol->GetQCOutputDirectory();
        }

        ReportFileName.append( "/" );

        std::string str = m_DwiFileName.substr( 0, m_DwiFileName.find_last_of('.') );
        str = str.substr( str.find_last_of("/\\") + 1);

        ReportFileName.append( str );
        ReportFileName.append(
          protocol->GetEddyMotionCorrectionProtocol().reportFileNameSuffix );
      }
      else
      {
        ReportFileName = m_DwiFileName.substr( 0, m_DwiFileName.find_last_of('.') );
        ReportFileName.append(
          protocol->GetEddyMotionCorrectionProtocol().reportFileNameSuffix );
      }
    }

    //    std::cout<<"ReportFileName: "<< ReportFileName <<std::endl;

    // get the inputgradDir
    itk::MetaDataDictionary imgMetaDictionary = dwi->GetMetaDataDictionary();    //
    std::vector<std::string> imgMetaKeys = imgMetaDictionary.GetKeys();
    std::vector<std::string>::const_iterator itKey = imgMetaKeys.begin();
    std::string metaString;
    TensorReconstructionImageFilterType::GradientDirectionType vect3d;

    GradientDirectionContainerType::Pointer inputGradDirContainer
      = GradientDirectionContainerType::New();
    inputGradDirContainer->clear();

    int baselineNumberLocal = 0;
    itKey = imgMetaKeys.begin();
    for (; itKey != imgMetaKeys.end(); itKey++ )
    {
      itk::ExposeMetaData<std::string>(imgMetaDictionary, *itKey, metaString);
      if ( itKey->find("DWMRI_gradient") != std::string::npos )
      {
        std::istringstream iss(metaString);
        iss >> vect3d[0] >> vect3d[1] >> vect3d[2];
        inputGradDirContainer->push_back(vect3d);
        if ( vect3d[0] == 0.0 && vect3d[1] == 0.0 && vect3d[2] == 0.0 )
        {
          baselineNumberLocal++;
        }
      }
    }

    //    std::cout<<"baselineNumberLocal: "<< baselineNumberLocal <<std::endl;

    // eddy-motion Iowa
    EddyMotionCorrectorTypeIowa::Pointer EddyMotionCorrectorIowa
      = EddyMotionCorrectorTypeIowa::New();
    EddyMotionCorrectorIowa->SetInput( dwi );

    // find the gradient with the smallest b-value
    int smallestGradient = -1;
    if ( baselineNumberLocal == 0 )
    {
      double smallestBValue = 9999999999999999.0;
      for ( unsigned int i = 0; i < inputGradDirContainer->size(); i++ )
      {
        double temp;
        temp =  inputGradDirContainer->ElementAt(i)[0]
        * inputGradDirContainer->ElementAt(i)[0]
        + inputGradDirContainer->ElementAt(i)[1]
        * inputGradDirContainer->ElementAt(i)[1]
        + inputGradDirContainer->ElementAt(i)[2]
        * inputGradDirContainer->ElementAt(i)[2];
        if ( temp < smallestBValue )
        {
          smallestBValue = temp;
          smallestGradient = i;
        }
      }

      //      std::cout<<"smallestGradient: "<< smallestGradient <<std::endl;

      typedef itk::VectorIndexSelectionCastImageFilter<DwiImageType,
        GradientImageType> FilterType;
      FilterType::Pointer componentExtractor = FilterType::New();

      componentExtractor->SetInput( dwi);
      componentExtractor->SetIndex( smallestGradient );
      componentExtractor->Update();
      EddyMotionCorrectorIowa->SetFixedImage( componentExtractor->GetOutput() );
    }
    else
    {
      GradientImageType::Pointer fixed = GradientImageType::New();

      fixed->CopyInformation( dwi );
      fixed->SetRegions( dwi->GetLargestPossibleRegion() );
      fixed->Allocate();
      fixed->FillBuffer(0);

      typedef itk::ImageRegionIteratorWithIndex<GradientImageType>
        averagedBaselineIterator;
      averagedBaselineIterator aIt( fixed, fixed->GetLargestPossibleRegion() );
      aIt.GoToBegin();

      typedef DwiImageType::ConstPointer dwiImageConstPointer;
      dwiImageConstPointer inputPtr = static_cast<dwiImageConstPointer>( dwi );
      GradientImageType::IndexType pixelIndex;
      double pixelValue;

      while ( !aIt.IsAtEnd() )
      {
        // determine the index of the output pixel
        pixelIndex = aIt.GetIndex();
        pixelValue = 0.0;
        for ( unsigned int i = 0; i < inputPtr->GetVectorLength(); i++ )
        {
          if ( inputGradDirContainer->ElementAt(i)[0] == 0.0
            && inputGradDirContainer->ElementAt(i)[1] == 0.0
            && inputGradDirContainer->ElementAt(i)[2] == 0.0     )
          {
            pixelValue += inputPtr->GetPixel(pixelIndex)[i]
            / ( static_cast<double>( baselineNumberLocal ) );
          }
        }
        aIt.Set( static_cast<unsigned short>( pixelValue ) );
        ++aIt;
      }
      EddyMotionCorrectorIowa->SetFixedImage(fixed);
    }

    //
    // EddyMotionCorrectorIowa->SetNumberOfBins(protocol->GetEddyMotionCorrectionProtocol().numberOfBins
    // );
    //     EddyMotionCorrectorIowa->SetSamples(
    // protocol->GetEddyMotionCorrectionProtocol().numberOfSamples );
    //     EddyMotionCorrectorIowa->SetTranslationScale(
    // protocol->GetEddyMotionCorrectionProtocol().translationScale );
    //
    // EddyMotionCorrectorIowa->SetStepLength(protocol->GetEddyMotionCorrectionProtocol().stepLength
    // );
    //     EddyMotionCorrectorIowa->SetFactor(
    // protocol->GetEddyMotionCorrectionProtocol().relaxFactor );
    //     EddyMotionCorrectorIowa->SetMaxNumberOfIterations(
    // protocol->GetEddyMotionCorrectionProtocol().maxNumberOfIterations );

    //     float m_TranslationScale;
    //     float m_MaximumStepLength;
    //     float m_MinimumStepLength;
    //     float m_RelaxationFactor;
    //     int   m_NumberOfSpatialSamples;
    //     int   m_NumberOfIterations;
    //     std::string m_OutputParameterFile;

    try
    {
      EddyMotionCorrectorIowa->Update();
    }
    catch ( itk::ExceptionObject & e )
    {
      std::cout << e.GetDescription() << std::endl;
    }

    this->m_DwiForcedConformanceImage = EddyMotionCorrectorIowa->GetOutput();

    // read the meta info from output and fill in qcResult
    itk::MetaDataDictionary outputMetaDictionary
      = EddyMotionCorrectorIowa->GetOutput()->GetMetaDataDictionary();
    std::vector<std::string> outputImgMetaKeys = outputMetaDictionary.GetKeys();
    std::vector<std::string>::const_iterator outputItKey
      = outputImgMetaKeys.begin();

    GradientDirectionContainerType::Pointer outputGradDirContainer
      = GradientDirectionContainerType::New();
    outputGradDirContainer->clear();

    outputItKey = outputImgMetaKeys.begin();
    for (; outputItKey != outputImgMetaKeys.end(); outputItKey++ )
    {
      itk::ExposeMetaData<std::string>(outputMetaDictionary,
        *outputItKey,
        metaString);
      if ( outputItKey->find("DWMRI_gradient") != std::string::npos )
      {
        std::istringstream iss(metaString);
        iss >> vect3d[0] >> vect3d[1] >> vect3d[2];
        outputGradDirContainer->push_back(vect3d);
      }
    }

    if ( inputGradDirContainer->size() != outputGradDirContainer->size() )
    {
      std::cout
        << "error: Gradient number mismatch between input and output."
        << std::endl;
      return false;
    }

/*for ( unsigned int i = 0; i < inputGradDirContainer->size(); i++ )
    {
      for ( unsigned int j = 0;
        j < this->qcResult->GetIntensityMotionCheckResult().size();
        j++ )
      {
        if ( 0.0 ==
          this->qcResult->GetIntensityMotionCheckResult()[j].ReplacedDir[0]
        && 0.0 ==
          this->qcResult->GetIntensityMotionCheckResult()[j].ReplacedDir[1]
        && 0.0 ==
          this->qcResult->GetIntensityMotionCheckResult()[j].ReplacedDir[2]  )
        {
          this->qcResult->GetIntensityMotionCheckResult()[j].processing
            = QCResult::GRADIENT_BASELINE_AVERAGED;
          continue;
        }

        if ( inputGradDirContainer->at(i)[0] ==
          this->qcResult->GetIntensityMotionCheckResult()[j].ReplacedDir[0]
        && inputGradDirContainer->at(i)[1] ==
          this->qcResult->GetIntensityMotionCheckResult()[j].ReplacedDir[1]
        && inputGradDirContainer->at(i)[2] ==
          this->qcResult->GetIntensityMotionCheckResult()[j].ReplacedDir[2]  )
        {
          if ( this->qcResult->GetIntensityMotionCheckResult()[j].processing >
            QCResult::GRADIENT_EDDY_MOTION_CORRECTED )                                                               //
            //
            // GRADIENT_EXCLUDE_SLICECHECK,
            // GRADIENT_EXCLUDE_INTERLACECHECK,
            // GRADIENT_EXCLUDE_GRADIENTCHECK,
            // GRADIENT_EXCLUDE_MANUALLY,
          {
            // std::cout<< "gradient " << i << " has been excluded!"
            // <<std::endl;
          }
          else
          {
            this->qcResult->GetIntensityMotionCheckResult()[j].processing
              = QCResult::GRADIENT_EDDY_MOTION_CORRECTED;
            this->qcResult->GetIntensityMotionCheckResult()[j].CorrectedDir[0]
            = outputGradDirContainer->at(i)[0];
            this->qcResult->GetIntensityMotionCheckResult()[j].CorrectedDir[1]
            = outputGradDirContainer->at(i)[1];
            this->qcResult->GetIntensityMotionCheckResult()[j].CorrectedDir[2]
            = outputGradDirContainer->at(i)[2];
          }
        }
      }
    }
*/

     for ( unsigned int j = 0;
        j < this->qcResult->GetIntensityMotionCheckResult().size();
        j++ )
      {
        if ( 0.0 ==
          this->qcResult->GetIntensityMotionCheckResult()[j].ReplacedDir[0]
        && 0.0 ==
          this->qcResult->GetIntensityMotionCheckResult()[j].ReplacedDir[1]
        && 0.0 ==
          this->qcResult->GetIntensityMotionCheckResult()[j].ReplacedDir[2]  )
        {
          this->qcResult->GetIntensityMotionCheckResult()[j].processing
            = QCResult::GRADIENT_BASELINE_AVERAGED;
          continue;
        }
    }

    for ( unsigned int i = 0; i < inputGradDirContainer->size(); i++ )
    {
	//std::cout << "Gradient Input Iowa No " << i << " : " << inputGradDirContainer->at(i)[0] << "," << inputGradDirContainer->at(i)[1] << "," << inputGradDirContainer->at(i)[2] << std:: endl;
	}	

    for ( unsigned int j = 0;
        j < this->qcResult->GetIntensityMotionCheckResult().size();
        j++ )
      {
	//std::cout << "Gradient QC replaced dir " << j << " : " << this->qcResult->GetIntensityMotionCheckResult()[j].ReplacedDir[0] << "," << this->qcResult->GetIntensityMotionCheckResult()[j].ReplacedDir[1] << "," << this->qcResult->GetIntensityMotionCheckResult()[j].ReplacedDir[2] << std:: endl;
	}

    for ( unsigned int i = 0; i < inputGradDirContainer->size(); i++ )
    {
      bool gradientFound = false;
      for ( unsigned int j = 0;
        j < this->qcResult->GetIntensityMotionCheckResult().size() && !gradientFound;
        j++ )
      {
        if ( inputGradDirContainer->at(i)[0] ==
          this->qcResult->GetIntensityMotionCheckResult()[j].ReplacedDir[0]
        && inputGradDirContainer->at(i)[1] ==
          this->qcResult->GetIntensityMotionCheckResult()[j].ReplacedDir[1]
        && inputGradDirContainer->at(i)[2] ==
          this->qcResult->GetIntensityMotionCheckResult()[j].ReplacedDir[2]  )
        {
          if ( this->qcResult->GetIntensityMotionCheckResult()[j].processing !=
            QCResult::GRADIENT_EDDY_MOTION_CORRECTED && this->qcResult->GetIntensityMotionCheckResult()[j].processing !=
            QCResult::GRADIENT_BASELINE_AVERAGED)
          {
            this->qcResult->GetIntensityMotionCheckResult()[j].processing
              = QCResult::GRADIENT_EDDY_MOTION_CORRECTED;
            this->qcResult->GetIntensityMotionCheckResult()[j].CorrectedDir[0]
            = outputGradDirContainer->at(i)[0];
            this->qcResult->GetIntensityMotionCheckResult()[j].CorrectedDir[1]
            = outputGradDirContainer->at(i)[1];
            this->qcResult->GetIntensityMotionCheckResult()[j].CorrectedDir[2]
            = outputGradDirContainer->at(i)[2];
	    gradientFound = true;
	    //std::cout << "Matching " << i <<" th Iowa gradient to " << j << " th original gradient" << std:: endl;
	  }
        }
      }
    }


    if ( protocol->GetGradientCheckProtocol().outputDWIFileNameSuffix.length()
    > 0 )
    {
      std::string outputDWIFileName;
      //    outputDWIFileName=m_DwiFileName.substr(0,m_DwiFileName.find_last_of('.')
      // );
      //       outputDWIFileName.append(
      // protocol->GetEddyMotionCorrectionProtocol().outputDWIFileNameSuffix );
      if ( protocol->GetQCOutputDirectory().length() > 0 )
      {
        if ( protocol->GetQCOutputDirectory().at( protocol->
          GetQCOutputDirectory().length() - 1 ) == '\\'
          || protocol->GetQCOutputDirectory().at( protocol->
          GetQCOutputDirectory().length() - 1 ) == '/'     )
        {
          outputDWIFileName = protocol->GetQCOutputDirectory().substr(
            0, protocol->GetQCOutputDirectory().find_last_of("/\\") );
        }
        else
        {
          outputDWIFileName = protocol->GetQCOutputDirectory();
        }

        outputDWIFileName.append( "/" );

        std::string str = m_DwiFileName.substr( 0, m_DwiFileName.find_last_of('.') );
        str = str.substr( str.find_last_of("/\\") + 1);

        outputDWIFileName.append( str );
        outputDWIFileName.append(
          protocol->GetEddyMotionCorrectionProtocol().outputDWIFileNameSuffix );
      }
      else
      {
        outputDWIFileName = m_DwiFileName.substr( 0, m_DwiFileName.find_last_of('.') );
        outputDWIFileName.append(
          protocol->GetEddyMotionCorrectionProtocol().outputDWIFileNameSuffix );
      }

      try
      {
        std::cout << "Saving output of eddy current motion correction: "
          << outputDWIFileName << " ... ";
        DwiWriterType::Pointer DwiWriter = DwiWriterType::New();
        itk::NrrdImageIO::Pointer myNrrdImageIO = itk::NrrdImageIO::New();
        DwiWriter->SetImageIO(myNrrdImageIO);
        DwiWriter->SetFileName( outputDWIFileName );
        DwiWriter->SetInput( m_DwiForcedConformanceImage);
        DwiWriter->UseCompressionOn();
        DwiWriter->Update();
      }
      catch ( itk::ExceptionObject & e )
      {
        std::cout << e.GetDescription() << std::endl;
      }
      std::cout << "DONE" << std::endl;
    }
  }
  else
  {
    std::cout << "Eddy-current and head motion correction NOT set."
      << std::endl;
  }
  return true;
}

bool CIntensityMotionCheck::EddyMotionCorrect( DwiImageType::Pointer dwi )
{
  if ( protocol->GetEddyMotionCorrectionProtocol().bCorrect )
  {
    std::string ReportFileName;
    if ( protocol->GetEddyMotionCorrectionProtocol().reportFileNameSuffix.
      length() > 0 )
    {
      if ( protocol->GetQCOutputDirectory().length() > 0 )
      {
        if ( protocol->GetQCOutputDirectory().at( protocol->
          GetQCOutputDirectory().length() - 1 ) == '\\'
          || protocol->GetQCOutputDirectory().at( protocol->
          GetQCOutputDirectory().length() - 1 ) == '/'     )
        {
          ReportFileName = protocol->GetQCOutputDirectory().substr(
            0, protocol->GetQCOutputDirectory().find_last_of("/\\") );
        }
        else
        {
          ReportFileName = protocol->GetQCOutputDirectory();
        }

        ReportFileName.append( "/" );

        std::string str = m_DwiFileName.substr( 0, m_DwiFileName.find_last_of('.') );
        str = str.substr( str.find_last_of("/\\") + 1);

        ReportFileName.append( str );
        ReportFileName.append(
          protocol->GetEddyMotionCorrectionProtocol().reportFileNameSuffix );
      }
      else
      {
        ReportFileName = m_DwiFileName.substr( 0, m_DwiFileName.find_last_of('.') );
        ReportFileName.append(
          protocol->GetEddyMotionCorrectionProtocol().reportFileNameSuffix );
      }
    }

    // eddy-motion Utah
    EddyMotionCorrectorType::Pointer EddyMotionCorrector
      = EddyMotionCorrectorType::New();
    EddyMotionCorrector->SetInput( dwi);
    EddyMotionCorrector->SetReportFileName( ReportFileName );
    EddyMotionCorrector->SetReportFileMode(
      protocol->GetEddyMotionCorrectionProtocol().reportFileMode );

    EddyMotionCorrector->SetNumberOfBins(
      protocol->GetEddyMotionCorrectionProtocol().numberOfBins );
    EddyMotionCorrector->SetSamples(
      protocol->GetEddyMotionCorrectionProtocol().numberOfSamples );
    EddyMotionCorrector->SetTranslationScale(
      protocol->GetEddyMotionCorrectionProtocol().translationScale );
    EddyMotionCorrector->SetStepLength(
      protocol->GetEddyMotionCorrectionProtocol().stepLength );
    EddyMotionCorrector->SetFactor(
      protocol->GetEddyMotionCorrectionProtocol().relaxFactor );
    EddyMotionCorrector->SetMaxNumberOfIterations(
      protocol->GetEddyMotionCorrectionProtocol().maxNumberOfIterations );

    try
    {
      EddyMotionCorrector->Update();
    }
    catch ( itk::ExceptionObject & e )
    {
      std::cout << e.GetDescription() << std::endl;
    }

    this->m_DwiForcedConformanceImage = EddyMotionCorrector->GetOutput();

    for ( unsigned int i = 0;
      i < EddyMotionCorrector->GetOutputGradientDirectionContainer()->size();
      i++ )
    {
      for ( unsigned int j = 0;
        j < this->qcResult->GetIntensityMotionCheckResult().size();
        j++ )
      {
        if ( 0.0 ==
          this->qcResult->GetIntensityMotionCheckResult()[j].ReplacedDir[0]
        && 0.0 ==
          this->qcResult->GetIntensityMotionCheckResult()[j].ReplacedDir[1]
        && 0.0 ==
          this->qcResult->GetIntensityMotionCheckResult()[j].ReplacedDir[2]  )
        {
          this->qcResult->GetIntensityMotionCheckResult()[j].processing
            = QCResult::GRADIENT_BASELINE_AVERAGED;
          continue;
        }

        if ( EddyMotionCorrector->GetFeedinGradientDirectionContainer()->at(i)[
          0] ==
            this->qcResult->GetIntensityMotionCheckResult()[j].ReplacedDir[0]
          && EddyMotionCorrector->GetFeedinGradientDirectionContainer()->at(
            i)[1] ==
            this->qcResult->GetIntensityMotionCheckResult()[j].ReplacedDir[1]
          && EddyMotionCorrector->GetFeedinGradientDirectionContainer()->at(
            i)[2] ==
            this->qcResult->GetIntensityMotionCheckResult()[j].ReplacedDir[2]  )
          {
            if ( this->qcResult->GetIntensityMotionCheckResult()[j].processing >
              QCResult::GRADIENT_EDDY_MOTION_CORRECTED )                                                               //
              //
              // GRADIENT_EXCLUDE_SLICECHECK,
              // GRADIENT_EXCLUDE_INTERLACECHECK,
              // GRADIENT_EXCLUDE_GRADIENTCHECK,
              // GRADIENT_EXCLUDE_MANUALLY,
            {
              // std::cout<< "gradient " << i << " has been excluded!"
              // <<std::endl;
            }
            else
            {
              this->qcResult->GetIntensityMotionCheckResult()[j].processing
                = QCResult::GRADIENT_EDDY_MOTION_CORRECTED;
              this->qcResult->GetIntensityMotionCheckResult()[j].CorrectedDir[0]
              = EddyMotionCorrector->GetOutputGradientDirectionContainer()->at(
                i)[
                  0];
                  this->qcResult->GetIntensityMotionCheckResult()[j].CorrectedDir[1]
                  = EddyMotionCorrector->GetOutputGradientDirectionContainer()->at(
                    i)[
                      1];
                      this->qcResult->GetIntensityMotionCheckResult()[j].CorrectedDir[2]
                      = EddyMotionCorrector->GetOutputGradientDirectionContainer()->at(
                        i)[
                          2];
            }
          }
      }
    }

    if ( protocol->GetGradientCheckProtocol().outputDWIFileNameSuffix.length()
      > 0 )
    {
      std::string outputDWIFileName;
      if ( protocol->GetQCOutputDirectory().length() > 0 )
      {
        if ( protocol->GetQCOutputDirectory().at( protocol->
          GetQCOutputDirectory().length() - 1 ) == '\\'
          || protocol->GetQCOutputDirectory().at( protocol->
          GetQCOutputDirectory().length() - 1 ) == '/'     )
        {
          outputDWIFileName = protocol->GetQCOutputDirectory().substr(
            0, protocol->GetQCOutputDirectory().find_last_of("/\\") );
        }
        else
        {
          outputDWIFileName = protocol->GetQCOutputDirectory();
        }

        outputDWIFileName.append( "/" );

        std::string str = m_DwiFileName.substr( 0, m_DwiFileName.find_last_of('.') );
        str = str.substr( str.find_last_of("/\\") + 1);

        outputDWIFileName.append( str );
        outputDWIFileName.append(
          protocol->GetEddyMotionCorrectionProtocol().outputDWIFileNameSuffix );
      }
      else
      {
        outputDWIFileName = m_DwiFileName.substr( 0, m_DwiFileName.find_last_of('.') );
        outputDWIFileName.append(
          protocol->GetEddyMotionCorrectionProtocol().outputDWIFileNameSuffix );
      }

      try
      {
        std::cout << "Saving output of eddy current motion correction: "
          << outputDWIFileName << " ... ";
        DwiWriterType::Pointer DwiWriter = DwiWriterType::New();
        itk::NrrdImageIO::Pointer myNrrdImageIO = itk::NrrdImageIO::New();
        DwiWriter->SetImageIO(myNrrdImageIO);
        DwiWriter->SetFileName( outputDWIFileName );
        DwiWriter->SetInput( m_DwiForcedConformanceImage);
        DwiWriter->UseCompressionOn();
        DwiWriter->Update();
      }
      catch ( itk::ExceptionObject & e )
      {
        std::cout << e.GetDescription() << std::endl;
      }
      std::cout << "DONE" << std::endl;
    }
  }
  else
  {
    std::cout << "Eddy-current and head motion correction NOT set."
      << std::endl;
  }
  return true;
}

bool CIntensityMotionCheck::GradientWiseCheck( DwiImageType::Pointer dwi )
{
  bool ret = true;
  if ( protocol->GetGradientCheckProtocol().bCheck )
  {
    std::string ReportFileName;
    if ( protocol->GetGradientCheckProtocol().reportFileNameSuffix.length() > 0 )
    {
      if ( protocol->GetQCOutputDirectory().length() > 0 )
      {
        if ( protocol->GetQCOutputDirectory().at( protocol->
          GetQCOutputDirectory().length() - 1 ) == '\\'
          || protocol->GetQCOutputDirectory().at( protocol->
          GetQCOutputDirectory().length() - 1 ) == '/'     )
        {
          ReportFileName = protocol->GetQCOutputDirectory().substr(
            0, protocol->GetQCOutputDirectory().find_last_of("/\\") );
        }
        else
        {
          ReportFileName = protocol->GetQCOutputDirectory();
        }

        ReportFileName.append( "/" );

        std::string str = m_DwiFileName.substr( 0, m_DwiFileName.find_last_of('.') );
        str = str.substr( str.find_last_of("/\\") + 1);

        ReportFileName.append( str );
        ReportFileName.append(
          protocol->GetGradientCheckProtocol().reportFileNameSuffix );
      }
      else
      {
        ReportFileName = m_DwiFileName.substr( 0, m_DwiFileName.find_last_of('.') );
        ReportFileName.append(
          protocol->GetGradientCheckProtocol().reportFileNameSuffix );
      }
    }

    GradientCheckerType::Pointer GradientChecker = GradientCheckerType::New();
    GradientChecker->SetInput( dwi);
    GradientChecker->SetTranslationThreshold(
      protocol->GetGradientCheckProtocol().translationThreshold );
    GradientChecker->SetRotationThreshold(
      protocol->GetGradientCheckProtocol().rotationThreshold );
    GradientChecker->SetReportFileName( ReportFileName );
    GradientChecker->SetReportFileMode(
      protocol->GetGradientCheckProtocol().reportFileMode );
    GradientChecker->SetReportType( protocol->GetReportType());

    try
    {
      GradientChecker->Update();
    }
    catch ( itk::ExceptionObject & e )
    {
      std::cout << e.GetDescription() << std::endl;
      return -1;
    }

    m_DwiForcedConformanceImage = GradientChecker->GetOutput();

    // .......Mapping between input gradeints and DWIForcedComformance gradeints
    //New : jun 2011
    std::vector<bool>	tem_vector = GradientChecker->GetQCResults();
    unsigned int id = 0;
    while ( id < tem_vector.size() )
    {
      if ( tem_vector[id] == 0 )
      {
	if ( this->qcResult->GetIntensityMotionCheckResult()[(m_Original_ForcedConformance_Mapping[id].index_original)[0]].processing == QCResult::GRADIENT_INCLUDE || this->qcResult->GetIntensityMotionCheckResult()[(m_Original_ForcedConformance_Mapping[id].index_original)[0]].processing == QCResult::GRADIENT_BASELINE_AVERAGED || this->qcResult->GetIntensityMotionCheckResult()[(m_Original_ForcedConformance_Mapping[id].index_original)[0]].processing == QCResult::GRADIENT_EDDY_MOTION_CORRECTED ) 
	{
	this->qcResult->GetIntensityMotionCheckResult()[(m_Original_ForcedConformance_Mapping[id].index_original)[0]].processing
                = QCResult::GRADIENT_EXCLUDE_GRADIENTCHECK;
	}
	tem_vector.erase( tem_vector.begin()+id );
        m_Original_ForcedConformance_Mapping.erase( m_Original_ForcedConformance_Mapping.begin()+id );
	id = -1;
      }
      id++;
    }
    //........


    //updating qcResult
    GradientWiseCheckResult GradientWiseResult;

    for (unsigned int i=0; i < GradientChecker->GetResultsContainer().size(); i++)
    {
    GradientWiseResult.AngleX=GradientChecker->GetResultsContainer()[i].AngleX;
    GradientWiseResult.AngleY=GradientChecker->GetResultsContainer()[i].AngleY;
    GradientWiseResult.AngleZ=GradientChecker->GetResultsContainer()[i].AngleZ;
    GradientWiseResult.TranslationX=GradientChecker->GetResultsContainer()[i].TranslationX;
    GradientWiseResult.TranslationY=GradientChecker->GetResultsContainer()[i].TranslationY;
    GradientWiseResult.TranslationZ=GradientChecker->GetResultsContainer()[i].TranslationZ;
    GradientWiseResult.MutualInformation=GradientChecker->GetResultsContainer()[i].MutualInformation;
    
    qcResult->GetGradientWiseCheckResult().push_back(GradientWiseResult);
    
    }    


    // update the QCResults
    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    //Wrong: This parts must be changed since the results of GradientWiseChecking QC are not matched with this->qcResult (Report) look at New jun 2011
    
    /*for ( unsigned int i = 0;
      i < GradientChecker->GetGradientDirectionContainer()->size();
      i++ )
    {
      for ( unsigned int j = 0;
        j < this->qcResult->GetIntensityMotionCheckResult().size();
        j++ )
      {
        if ( vcl_abs(GradientChecker->GetGradientDirectionContainer()->at(i)[0]
        - this->qcResult->GetIntensityMotionCheckResult()[j].CorrectedDir[0]) 
          < 0.000001
          && vcl_abs(GradientChecker->GetGradientDirectionContainer()->at(i)[1]
        - this->qcResult->GetIntensityMotionCheckResult()[j].CorrectedDir[1]) 
          < 0.000001
          && vcl_abs(GradientChecker->GetGradientDirectionContainer()->at(i)[2]
        - this->qcResult->GetIntensityMotionCheckResult()[j].CorrectedDir[2])        
           < 0.000001    )
        {
          if ( this->qcResult->GetIntensityMotionCheckResult()[j].processing >
            QCResult::GRADIENT_EDDY_MOTION_CORRECTED )
          {
            // std::cout<< "gradient " << j << " has been excluded!"
            // <<std::endl;
          }
          else
          {
            if (  !GradientChecker->getQCResults()[i] )
            {
              // std::cout<< "gradient " << j << "
              // GRADIENT_EXCLUDE_GRADIENTCHECK!" <<std::endl;
              this->qcResult->GetIntensityMotionCheckResult()[j].processing
                = QCResult::GRADIENT_EXCLUDE_GRADIENTCHECK;
            }
          }
        }
      }
    }*/
 
    // save the output of gradient check
    if ( protocol->GetGradientCheckProtocol().outputDWIFileNameSuffix.length() > 0 )
    {
      std::string outputDWIFileName;
      if ( protocol->GetQCOutputDirectory().length() > 0 )
      {
        if ( protocol->GetQCOutputDirectory().at( protocol->
          GetQCOutputDirectory().length() - 1 ) == '\\'
          || protocol->GetQCOutputDirectory().at( protocol->
          GetQCOutputDirectory().length() - 1 ) == '/'     )
        {
          outputDWIFileName = protocol->GetQCOutputDirectory().substr(
            0, protocol->GetQCOutputDirectory().find_last_of("/\\") );
        }
        else
        {
          outputDWIFileName = protocol->GetQCOutputDirectory();
        }

        outputDWIFileName.append( "/" );

        std::string str = m_DwiFileName.substr( 0, m_DwiFileName.find_last_of('.') );
        str = str.substr( str.find_last_of("/\\") + 1);

        outputDWIFileName.append( str );
        outputDWIFileName.append(
          protocol->GetGradientCheckProtocol().outputDWIFileNameSuffix );
      }
      else
      {
        outputDWIFileName = m_DwiFileName.substr( 0, m_DwiFileName.find_last_of('.') );
        outputDWIFileName.append(
          protocol->GetGradientCheckProtocol().outputDWIFileNameSuffix );
      }

      try
      {
        std::cout << "Saving output of gradient check: "<< outputDWIFileName << " ... ";
        DwiWriterType::Pointer DwiWriter = DwiWriterType::New();
        itk::NrrdImageIO::Pointer myNrrdImageIO = itk::NrrdImageIO::New();
        DwiWriter->SetImageIO(myNrrdImageIO);
        DwiWriter->SetFileName( outputDWIFileName );
        DwiWriter->SetInput( m_DwiForcedConformanceImage);
        DwiWriter->UseCompressionOn();
        DwiWriter->Update();
      }
      catch ( itk::ExceptionObject & e )
      {
        std::cout << e.GetDescription() << std::endl;
        return -1;
      }
      std::cout << "DONE" << std::endl;
    }

    // save the excluded DWIs in gradient wise checking
    if ( ! GradientChecker->GetExcludedGradients() )
    {
      std::cout << "No excluded gradient file created." << std::endl;   
    }
    else
    {

      if ( protocol->GetInterlaceCheckProtocol().excludedDWINrrdFileNameSuffix.length() > 0 )
      {
        std::string GradientWiseExcludeOutput;
        if ( protocol->GetQCOutputDirectory().length() > 0 )
        {
          if ( protocol->GetQCOutputDirectory().at( protocol->
            GetQCOutputDirectory().length() - 1 ) == '\\'
            || protocol->GetQCOutputDirectory().at( protocol->
            GetQCOutputDirectory().length() - 1 ) == '/'     )
          {
            GradientWiseExcludeOutput = protocol->GetQCOutputDirectory().substr(
              0, protocol->GetQCOutputDirectory().find_last_of("/\\") );
          }
          else
          {
            GradientWiseExcludeOutput = protocol->GetQCOutputDirectory();
          }

          GradientWiseExcludeOutput.append( "/" );

          std::string str = m_DwiFileName.substr( 0, m_DwiFileName.find_last_of('.') );
          str = str.substr( str.find_last_of("/\\") + 1);

          GradientWiseExcludeOutput.append( str );
          GradientWiseExcludeOutput.append(
            protocol->GetInterlaceCheckProtocol().excludedDWINrrdFileNameSuffix );
        }
        else
        {
          GradientWiseExcludeOutput = m_DwiFileName.substr( 0, m_DwiFileName.find_last_of('.') );
          GradientWiseExcludeOutput.append(
            protocol->GetGradientCheckProtocol().excludedDWINrrdFileNameSuffix );
        }

        try
        {
          std::cout << "Saving excluded gradients of interlace check: " << GradientWiseExcludeOutput
          << " ... ";
          DwiWriterType::Pointer DwiWriter = DwiWriterType::New();
          itk::NrrdImageIO::Pointer myNrrdImageIO = itk::NrrdImageIO::New();
          DwiWriter->SetImageIO(myNrrdImageIO);
          DwiWriter->SetFileName( GradientWiseExcludeOutput );
          DwiWriter->SetInput( GradientChecker->GetExcludedGradients() );
          DwiWriter->UseCompressionOn();
          DwiWriter->Update();
        }
        catch ( itk::ExceptionObject & e )
        {
          std::cout << e.GetDescription() << std::endl;
        }
        std::cout << "DONE" << std::endl;
      }
    }

    //validate the gradient Wise output
    std::ofstream outfile; 
    outfile.open(ReportFileName.c_str(), std::ios::app);

    //outfile<<"=="<<std::endl;
    //outfile<<"GradientWisw check summary:"<<std::endl;

    //std::cout<<"=="<<std::endl;
    //std::cout<<"GradientWisw check summary::"<<std::endl;

    if(GradientChecker->getGradientDirLeftNumber()<6)
    {
      outfile<<"  Gradient direction # is less than 6!"<<std::endl;
      std::cout<<"  Gradient direction # is less than 6!"<<std::endl;
      ret = false;
    }

    if(GradientChecker->getBaselineLeftNumber()==0 && GradientChecker->getBValueLeftNumber()==1)
    {
      outfile<<"  Single b-value DWI without a b0/baseline!"<<std::endl;
      std::cout<<"  Single b-value DWI without a b0/baseline!"<<std::endl;
      ret = false;
    }

    if( ((GradientChecker->getGradientDirNumber())-(GradientChecker->getGradientDirLeftNumber())) 
    > protocol->GetBadGradientPercentageTolerance()* (GradientChecker->getGradientDirNumber()))
    {
      outfile<<"  Too many bad gradient directions found!"<<std::endl;
      std::cout<<"  Too many bad gradient directions found!"<<std::endl;
      ret = false;
    }    
  }
  else
  {
    std::cout << "Gradient-wise check NOT set." << std::endl;
  }
  return ret;
}

bool CIntensityMotionCheck::SaveDwiForcedConformanceImage(void) const
{
  if ( protocol->GetQCedDWIFileNameSuffix().length() > 0 )
  {
    std::string outputDWIFileName;
    if ( protocol->GetQCOutputDirectory().length() > 0 )
    {
      if ( protocol->GetQCOutputDirectory().at( protocol->GetQCOutputDirectory()
        .length() - 1 ) == '\\'
        || protocol->GetQCOutputDirectory().at( protocol->
        GetQCOutputDirectory().length() - 1 ) == '/'     )
      {
        outputDWIFileName = protocol->GetQCOutputDirectory().substr(
          0, protocol->GetQCOutputDirectory().find_last_of("/\\") );
      }
      else
      {
        outputDWIFileName = protocol->GetQCOutputDirectory();
      }

      outputDWIFileName.append( "/" );

      std::string str = m_DwiFileName.substr( 0, m_DwiFileName.find_last_of('.') );
      str = str.substr( str.find_last_of("/\\") + 1);

      outputDWIFileName.append( str );
      outputDWIFileName.append( protocol->GetQCedDWIFileNameSuffix() );
    }
    else
    {
      outputDWIFileName = m_DwiFileName.substr( 0, m_DwiFileName.find_last_of('.') );
      outputDWIFileName.append( protocol->GetQCedDWIFileNameSuffix() );
    }

    try
    {
      DwiWriterType::Pointer DwiWriter = DwiWriterType::New();
      itk::NrrdImageIO::Pointer myNrrdImageIO = itk::NrrdImageIO::New();
      DwiWriter->SetImageIO(myNrrdImageIO);
      DwiWriter->SetFileName( outputDWIFileName );
      DwiWriter->SetInput( this->m_DwiForcedConformanceImage );
      DwiWriter->UseCompressionOn();
      DwiWriter->Update();
    }
    catch ( itk::ExceptionObject & e )
    {
      std::cout << e.GetDescription() << std::endl;
      return false;
    }
    return true;
  }
  return true;
}

bool CIntensityMotionCheck::SaveDwiForcedConformanceImage_FurtherQC( void ) const
{
  if ( protocol->GetQCedDWIFileNameSuffix().length() > 0 )
  {
    std::string outputDWIFileName;
    if ( protocol->GetQCOutputDirectory().length() > 0 )
    {
      if ( protocol->GetQCOutputDirectory().at( protocol->GetQCOutputDirectory()
        .length() - 1 ) == '\\'
        || protocol->GetQCOutputDirectory().at( protocol->
        GetQCOutputDirectory().length() - 1 ) == '/'     )
      {
        outputDWIFileName = protocol->GetQCOutputDirectory().substr(
          0, protocol->GetQCOutputDirectory().find_last_of("/\\") );
      }
      else
      {
        outputDWIFileName = protocol->GetQCOutputDirectory();
      }

      outputDWIFileName.append( "/" );
      outputDWIFileName.append("FurtherQC_");

      std::string str = m_DwiFileName.substr( 0, m_DwiFileName.find_last_of('.') );
      str = str.substr( str.find_last_of("/\\") + 1);

      outputDWIFileName.append( str );
      outputDWIFileName.append( protocol->GetQCedDWIFileNameSuffix() );
    }
    else
    {
      outputDWIFileName = m_DwiFileName.substr( 0, m_DwiFileName.find_last_of('.') );
      outputDWIFileName.append("_FurtherQC");
      outputDWIFileName.append( protocol->GetQCedDWIFileNameSuffix() );
    }

    try
    {
      DwiWriterType::Pointer DwiWriter = DwiWriterType::New();
      itk::NrrdImageIO::Pointer myNrrdImageIO = itk::NrrdImageIO::New();
      DwiWriter->SetImageIO(myNrrdImageIO);
      std::cout << "OutPut File name: " << outputDWIFileName << std::endl;
      DwiWriter->SetFileName( outputDWIFileName );
      DwiWriter->SetInput( this->m_DwiForcedConformanceImage );
      DwiWriter->UseCompressionOn();
      DwiWriter->Update();
    }
    catch ( itk::ExceptionObject & e )
    {
      std::cout << e.GetDescription() << std::endl;
      return false;
    }
    return true;
  }
  return true;
}
  

unsigned char CIntensityMotionCheck::RunPipelineByProtocol_FurtherQC()
{
  
  
  if ( !protocol_load )
  {
    std::cout << "Protocol NOT set (in Further QC)." << std::endl;
    return -1;
  }

  std::cout << "New DWI gradients:" << m_DwiOriginalImage->GetVectorLength() << "and DWI Forced" << m_DwiForcedConformanceImage->GetVectorLength() << std::endl;
  if ( m_DwiOriginalImage->GetVectorLength() != m_GradientDirectionContainer->size() )
  {
    std::cout << "Bad DWI: mismatch between gradient image # and gradient vector # (in Further QC)"
      << std::endl;
    return -1;
  }

  bool bReport = false;
  std::ofstream outfile;

  std::string ReportFileName;
  if ( protocol->GetQCOutputDirectory().length() > 0 )
  {
    if ( protocol->GetQCOutputDirectory().at( protocol->GetQCOutputDirectory().
      length() - 1 ) == '\\'
      || protocol->GetQCOutputDirectory().at( protocol->GetQCOutputDirectory()
      .length() - 1 ) == '/'     )
    {
      ReportFileName = protocol->GetQCOutputDirectory().substr(
        0, protocol->GetQCOutputDirectory().find_last_of("/\\") );
    }
    else
    {
      ReportFileName = protocol->GetQCOutputDirectory();
    }

    ReportFileName.append( "/" );

    std::string str = m_DwiFileName.substr( 0, m_DwiFileName.find_last_of('.') );
    str = str.substr( str.find_last_of("/\\") + 1);

    ReportFileName.append( str );
    ReportFileName.append( "_QC_CheckReports_FurtherQC.txt");
    
  }
  else
  {
    ReportFileName = m_DwiFileName.substr( 0, m_DwiFileName.find_last_of('.') );
    
    ReportFileName.append( "_QC_CheckReports_FurtherQC.txt");
  }

  outfile.open( ReportFileName.c_str() );

  if ( outfile )
  {
    bReport = true;
  }

  if ( bReport )
  {
    outfile << "================================= " << std::endl;
    outfile << "* DWI Further QC Report ( DTIPrep " << DTIPREP_VERSION << " ) * " << std::endl;
    outfile << "================================= " << std::endl;
    outfile << "DWI File: " << m_DwiFileName << std::endl;
    outfile << "xml File: " << m_XmlFileName << std::endl;
    time_t rawtime; time( &rawtime );
    outfile << "Check Time: " << ctime(&rawtime) << std::endl;

    outfile.close();
  }

  Original_ForcedConformance_Mapping m_map;
  for ( unsigned int jj = 0; jj< m_DwiOriginalImage->GetVectorLength(); jj++ )
  {	
	std::vector<int> Bing;	
	Bing.push_back( jj );
	m_map.index_original = Bing;
	m_map.index_ForcedConformance = jj;
	m_Original_ForcedConformance_Mapping.push_back( m_map );
  }

  this-> qcResult->Set_result(0);	//unsigned char result = 0;	

  // ZYXEDCBA:
  // X  QC; Too many bad gradient directions found!
  // Y  QC; Single b-value DWI without a b0/baseline!
  // Z  QC: Gradient direction # is less than 6!
  // A: ImageCheck()
  // B: DiffusionCheckInternalDwiImage()
  // C: SliceCheck()
  // D: InterlaceCheck()
  // E:GradientCheck()

  // protocol->printProtocols();
#if 0
  DwiWriterType::Pointer DwiWriter = DwiWriterType::New();
  itk::NrrdImageIO::Pointer myNrrdImageIO = itk::NrrdImageIO::New();
  DwiWriter->SetImageIO(myNrrdImageIO);
#endif

  m_DwiForcedConformanceImage = m_DwiOriginalImage;
  // baseline average
  std::cout << "=====================" << std::endl;
  std::cout << "BaselineAverage in Further QC... " << std::endl;
  
  BaselineAverage( m_DwiForcedConformanceImage );
  std::cout << "BaselineAverage DONE " << std::endl;
 

  
  // EddyMotionCorrect
  std::cout << "=====================" << std::endl;
  std::cout << "EddyCurrentHeadMotionCorrectIowa in Further QC... " << std::endl;
  
  EddyMotionCorrectIowa(m_DwiForcedConformanceImage);
  std::cout << "EddyCurrentHeadMotionCorrect DONE " << std::endl;
  
  

  // GradientChecker
  std::cout << "=====================" << std::endl;
  std::cout << "GradientCheck in Further QC... " << std::endl;
  
  if ( !GradientWiseCheck( m_DwiForcedConformanceImage ) )
  {
    this-> qcResult->Set_result( this-> qcResult->Get_result() | 16 );
    printf("result of GradientCheck = %d",this-> qcResult->Get_result());
    if( protocol->GetGradientCheckProtocol().bQuitOnCheckFailure)
    {
      std::cout << "GradientWise Check failed, pipeline terminated. " << std::endl;
      std::cout << "GradientCheck DONE " << std::endl;
      return this-> qcResult->Get_result(); 
    }
  }
  std::cout << "GradientCheck DONE " << std::endl;

  // Save QC'ed DWI
  std::cout << "=====================" << std::endl;
  std::cout << "Save QC'ed DWI in Further QC... ";
  SaveDwiForcedConformanceImage_FurtherQC();

  std::cout << "DONE" << std::endl;
  
  // DTIComputing
  //std::cout << "=====================" << std::endl;
  //std::cout << "DTIComputing in Further QC... " << std::endl;
  //DTIComputing();
  //std::cout << "DTIComputing DONE" << std::endl;

  return this-> qcResult->Get_result();
  
}

unsigned char CIntensityMotionCheck::RunPipelineByProtocol()
{
 
  if ( !protocol )
  {
    std::cout << "Protocol NOT set." << std::endl;
    return -1;
  }

  if ( m_DwiOriginalImage->GetVectorLength() != m_GradientDirectionContainer->size() )
  {
    std::cout << "Bad DWI: mismatch between gradient image # and gradient vector #"
      << std::endl;
    return -1;
  }

  if ( !m_bDwiLoaded  )
  {
    LoadDwiImage();
  }

  bool bReport = false;
  std::ofstream outfile;

  std::string ReportFileName;
  if ( protocol->GetQCOutputDirectory().length() > 0 )
  {
    if ( protocol->GetQCOutputDirectory().at( protocol->GetQCOutputDirectory().
      length() - 1 ) == '\\'
      || protocol->GetQCOutputDirectory().at( protocol->GetQCOutputDirectory()
      .length() - 1 ) == '/'     )
    {
      ReportFileName = protocol->GetQCOutputDirectory().substr(
        0, protocol->GetQCOutputDirectory().find_last_of("/\\") );
    }
    else
    {
      ReportFileName = protocol->GetQCOutputDirectory();
    }

    ReportFileName.append( "/" );

    std::string str = m_DwiFileName.substr( 0, m_DwiFileName.find_last_of('.') );
    str = str.substr( str.find_last_of("/\\") + 1);

    ReportFileName.append( str );
    if ( protocol->GetReportFileNameSuffix().length() > 0 )
    {
      ReportFileName.append( protocol->GetReportFileNameSuffix() );
    }
    else
    {
      ReportFileName.append( "_QC_CheckReports.txt");
    }
  }
  else
  {
    ReportFileName = m_DwiFileName.substr( 0, m_DwiFileName.find_last_of('.') );
    if ( protocol->GetReportFileNameSuffix().length() > 0 )
    {
      ReportFileName.append( protocol->GetReportFileNameSuffix() );
    }
    else
    {
      ReportFileName.append( "_QC_CheckReports.txt");
    }
  }

  outfile.open( ReportFileName.c_str() );

  if ( outfile )
  {
    bReport = true;
  }

  if ( bReport )
  {
    outfile << "================================= " << std::endl;
    outfile << "* DWI QC Report ( DTIPrep " << DTIPREP_VERSION << " ) * " << std::endl;
    outfile << "================================= " << std::endl;
    outfile << "DWI File: " << m_DwiFileName << std::endl;
    outfile << "xml File: " << m_XmlFileName << std::endl;
    time_t rawtime; time( &rawtime );
    outfile << "Check Time: " << ctime(&rawtime) << std::endl;

    outfile.close();
  }

  Original_ForcedConformance_Mapping m_map;
  for ( unsigned int jj = 0; jj< m_DwiOriginalImage->GetVectorLength(); jj++ )
  {	
	std::vector<int> Bing;
	Bing.push_back( jj );
	m_map.index_original = Bing;
	m_map.index_ForcedConformance = jj;
	m_Original_ForcedConformance_Mapping.push_back( m_map );
  }

  this-> qcResult->Set_result(0);	//unsigned char result = 0;	

  // ZYXEDCBA:
  // X  QC; Too many bad gradient directions found!
  // Y  QC; Single b-value DWI without a b0/baseline!
  // Z  QC: Gradient direction # is less than 6!
  // A: ImageCheck()
  // B: DiffusionCheckInternalDwiImage()
  // C: SliceCheck()
  // D: InterlaceCheck()
  // E:GradientCheck()

  // protocol->printProtocols();
#if 0
  DwiWriterType::Pointer DwiWriter = DwiWriterType::New();
  itk::NrrdImageIO::Pointer myNrrdImageIO = itk::NrrdImageIO::New();
  DwiWriter->SetImageIO(myNrrdImageIO);
#endif

  m_DwiForcedConformanceImage = m_DwiOriginalImage;

  // image information check
  std::cout << "=====================" << std::endl;
  std::cout << "ImageCheck ... " << std::endl;
  unsigned char imageCheckResult = ImageCheck( m_DwiForcedConformanceImage );
  if ( imageCheckResult )
  {
    this-> qcResult->Set_result( this-> qcResult->Get_result() | 1 );
    printf("result of ImageCheck = %d",this-> qcResult->Get_result());
    if( protocol->GetImageProtocol().bQuitOnCheckSizeFailure && (imageCheckResult & 0x00000001 ))
    {
      std::cout << "Image size check failed, pipeline terminated. " << std::endl;
      std::cout << "ImageCheck DONE " << std::endl;
      return this-> qcResult->Get_result(); 
    }

    if( protocol->GetImageProtocol().bQuitOnCheckSpacingFailure && (imageCheckResult & 0x00000010 ))
    {
      std::cout << "Image spacing check failed, pipeline terminated. " << std::endl;
      std::cout << "ImageCheck DONE " << std::endl;
      return this-> qcResult->Get_result(); 
    }
  }
  std::cout << "ImageCheck DONE " << std::endl;
  
  
  
  // diffusion information check
  std::cout << "=====================" << std::endl;
  std::cout << "DiffusionCheck ... " << std::endl;
  if ( !DiffusionCheck( m_DwiForcedConformanceImage ) )
  {
    this-> qcResult->Set_result( this-> qcResult->Get_result() | 2 );
    printf("result of DiffusionCheck = %d",this-> qcResult->Get_result());
    if( protocol->GetDiffusionProtocol().bQuitOnCheckFailure )
    {
      std::cout << "Diffusion Check failed, pipeline terminated. " << std::endl;
      std::cout << "DiffusionCheck DONE " << std::endl;
      return this-> qcResult->Get_result(); 
    }
  }
  std::cout << "DiffusionCheck DONE " << std::endl;
  

  // SliceChecker
  std::cout << "=====================" << std::endl;
  std::cout << "SliceWiseCheck ... " << std::endl;
  
  if ( !SliceWiseCheck( m_DwiForcedConformanceImage ) )
  {
    this-> qcResult->Set_result( this-> qcResult->Get_result() | 4 );
    if( protocol->GetSliceCheckProtocol().bQuitOnCheckFailure)
    {
      std::cout << "SliceWise Check failed, pipeline terminated. " << std::endl;
      std::cout << "SliceWiseCheck DONE " << std::endl;
      return this-> qcResult->Get_result(); 
    }
  }
  std::cout << "SliceWiseCheck DONE " << std::endl;

  
  // InterlaceChecker
  std::cout << "=====================" << std::endl;
  std::cout << "InterlaceWiseCheck ... " << std::endl;
  
  if ( !InterlaceWiseCheck( m_DwiForcedConformanceImage ) )
  {
    this-> qcResult->Set_result( this-> qcResult->Get_result() | 8 );
    printf("result of InterlaceWiseCheck = %d",this-> qcResult->Get_result());
    if( protocol->GetInterlaceCheckProtocol().bQuitOnCheckFailure)
    {
      std::cout << "InterlaceWise Check failed, pipeline terminated. " << std::endl;
      std::cout << "InterlaceWiseCheck DONE " << std::endl;
      return this-> qcResult->Get_result(); 
    }
  }
  std::cout << "InterlaceWiseCheck DONE " << std::endl;
  
  std::cout << "Mapping Original and Comforce image: " << "length of Map: " << m_Original_ForcedConformance_Mapping.size() << std::endl;
  

  // baseline average
  std::cout << "=====================" << std::endl;
  std::cout << "BaselineAverage ... " << std::endl;
  
  BaselineAverage( m_DwiForcedConformanceImage );
  std::cout << "BaselineAverage DONE " << std::endl;
 

  
  // EddyMotionCorrect
  std::cout << "=====================" << std::endl;
  std::cout << "EddyCurrentHeadMotionCorrect ... " << std::endl;
  // EddyMotionCorrect( m_DwiForcedConformanceImage );
  std::cout << "EddyCurrentHeadMotionCorrectIowa ... " << std::endl;
  
  EddyMotionCorrectIowa(m_DwiForcedConformanceImage);
  std::cout << "EddyCurrentHeadMotionCorrect DONE " << std::endl;
  
  

  // GradientChecker
  std::cout << "=====================" << std::endl;
  std::cout << "GradientCheck ... " << std::endl;
  
  if ( !GradientWiseCheck( m_DwiForcedConformanceImage ) )
  {
    this-> qcResult->Set_result( this-> qcResult->Get_result() | 16 );
    printf("result of GradientCheck = %d",this-> qcResult->Get_result());
    if( protocol->GetGradientCheckProtocol().bQuitOnCheckFailure)
    {
      std::cout << "GradientWise Check failed, pipeline terminated. " << std::endl;
      std::cout << "GradientCheck DONE " << std::endl;
      return this-> qcResult->Get_result(); 
    }
  }
  std::cout << "GradientCheck DONE " << std::endl;

  for ( unsigned int k_ind = 0; k_ind < m_Original_ForcedConformance_Mapping.size() ; k_ind ++ )
  {
	if ( k_ind == 0 )
	{
	for ( unsigned int kk_ind = 0; kk_ind < m_Original_ForcedConformance_Mapping[k_ind].index_original.size(); kk_ind ++ )
 	{
	std::cout << "Baselines_indices:" << " " << m_Original_ForcedConformance_Mapping[k_ind].index_original[kk_ind] << std::endl;
	}
	}
	else
	std::cout << "Included Gradients_indices:" << " " << m_Original_ForcedConformance_Mapping[k_ind].index_original[0] << std::endl;
	
  }
  // Saving m_Original_ForcedConformance_Mapping in the QCResult
 /* Original_ForcedConformance_Map item_map;
  for ( unsigned int ind_map = 0; ind_map < m_Original_ForcedConformance_Mapping.size() ; ind_map++ )
  {
      item_map.index_ForcedConformance = m_Original_ForcedConformance_Mapping[ ind_map ].index_ForcedConformance;
      for ( unsigned int ind = 0; ind < (m_Original_ForcedConformance_Mapping[ ind_map ].index_original).size() ; ind ++ )
		item_map.index_original[ ind ] = (m_Original_ForcedConformance_Mapping[ ind_map ].index_original )[ ind ];

      this->qcResult->GetOriginal_ForcedConformance_Map().push_back( item_map );
  }  */


  // Save QC'ed DWI
  std::cout << "=====================" << std::endl;
  std::cout << "Save QC'ed DWI ... ";
  SaveDwiForcedConformanceImage();

  std::cout << "DONE" << std::endl;

  // DTIComputing
  std::cout << "=====================" << std::endl;
  std::cout << "DTIComputing ... " << std::endl;
  DTIComputing();
  std::cout << "DTIComputing DONE" << std::endl;

  // 00000CBA:
  // A: Gradient direction # is less than 6!
  // B: Single b-value DWI without a b0/baseline!
  // C: Too many bad gradient directions found!
  // 0: valid

  unsigned char ValidateResult;
  collectLeftDiffusionStatistics( m_DwiForcedConformanceImage, ReportFileName );
  ValidateResult = validateLeftDiffusionStatistics();

  this->qcResult-> Set_result( ( ValidateResult << 5 ) + this->qcResult->Get_result() );
  
  
  
  return this->qcResult->Get_result();

}


bool CIntensityMotionCheck::validateDiffusionStatistics()
{
  return true;
}

unsigned char CIntensityMotionCheck::validateLeftDiffusionStatistics()

// 00000CBA:
// A: Gradient direction # is less than 6!
// B: Single b-value DWI without a b0/baseline!
// C: Too many bad gradient directions found!
// 0: valid

{
  bool bReport = false;
  std::string ReportFileName;

  if ( protocol->GetImageProtocol().reportFileNameSuffix.length() > 0 )
  {
    ReportFileName = m_DwiFileName.substr( 0, m_DwiFileName.find_last_of('.') );
    ReportFileName.append( protocol->GetImageProtocol().reportFileNameSuffix );
  }

  std::ofstream outfile;
  outfile.open( ReportFileName.c_str(), std::ios_base::app);

  if ( outfile )
  {
    bReport = true;
  }

  if ( bReport )
  {
    outfile << "================================" << std::endl;
    outfile << "  QC result summary:" << std::endl;
    outfile << "================================" << std::endl;
  }

  std::cout << "================================" << std::endl;
  std::cout << "  QC result summary:" << std::endl;
  std::cout << "================================" << std::endl;

  unsigned char ret = 0;

  if ( this->m_gradientDirLeftNumber < 6 )
  {
    std::cout << "\tGradient direction # is less than 6!" << std::endl;
    if ( bReport )
    {
      outfile << "\tGradient direction # is less than 6!" << std::endl;
    }
    ret = ret | 1;
  }

  if ( this->m_baselineLeftNumber == 0 && this->m_bValueLeftNumber == 1 )
  {
    std::cout << "\tSingle b-value DWI without a b0/baseline!" << std::endl;
    if ( bReport )
    {
      outfile << "\tSingle b-value DWI without a b0/baseline!" << std::endl;
    }
    ret = ret | 2;
  }

  if ( ( ( this->m_gradientDirNumber ) - ( this->m_gradientDirLeftNumber ) ) >
    protocol->GetBadGradientPercentageTolerance() * ( this->m_gradientDirNumber ) )
  {
    std::cout << "\tToo many bad gradient directions found! " << std::endl;
    if ( bReport )
    {
      outfile  << "\tToo many bad gradient directions found! " << std::endl;
    }
    ret = ret | 4;
  }

  // std::cout<<"validateDiffusionStatistics(): ret "<<ret<<std::endl;
  outfile.close();

  return ret;
}

void CIntensityMotionCheck::collectLeftDiffusionStatistics(
  DwiImageType::Pointer dwi,
  std::string reportfilename )
{
  GradientDirectionContainerType::Pointer GradContainer
    = GradientDirectionContainerType::New();
  double bValue;

  this->GetGradientDirections( dwi, bValue, GradContainer);

  // ///////
  std::vector<DiffusionDir> DiffusionDirections;
  DiffusionDirections.clear();

  for ( unsigned int i = 0; i < GradContainer->size(); i++ )
  {
    if ( DiffusionDirections.size() > 0 )
    {
      bool newDir = true;
      for ( unsigned int j = 0; j < DiffusionDirections.size(); j++ )
      {
        if ( GradContainer->ElementAt(i)[0] ==
          DiffusionDirections[j].gradientDir[0]
        && GradContainer->ElementAt(i)[1] ==
          DiffusionDirections[j].gradientDir[1]
        && GradContainer->ElementAt(i)[2] ==
          DiffusionDirections[j].gradientDir[2] )
        {
          DiffusionDirections[j].repetitionNumber++;
          newDir = false;
        }
      }
      if ( newDir )
      {
        std::vector<double> dir;
        dir.push_back(GradContainer->ElementAt(i)[0]);
        dir.push_back(GradContainer->ElementAt(i)[1]);
        dir.push_back(GradContainer->ElementAt(i)[2]);

        DiffusionDir diffusionDir;
        diffusionDir.gradientDir = dir;
        diffusionDir.repetitionNumber = 1;

        DiffusionDirections.push_back(diffusionDir);
      }
    }
    else
    {
      std::vector<double> dir;
      dir.push_back(GradContainer->ElementAt(i)[0]);
      dir.push_back(GradContainer->ElementAt(i)[1]);
      dir.push_back(GradContainer->ElementAt(i)[2]);

      DiffusionDir diffusionDir;
      diffusionDir.gradientDir = dir;
      diffusionDir.repetitionNumber = 1;

      DiffusionDirections.push_back(diffusionDir);
    }
  }

  // std::cout<<"DiffusionDirections.size(): " << DiffusionDirections.size()
  // <<std::endl;

  std::vector<double> dirMode;
  dirMode.clear();

  this->m_baselineLeftNumber = 0;
  for ( unsigned int i = 0; i < DiffusionDirections.size(); i++ )
  {
    if ( DiffusionDirections[i].gradientDir[0] == 0.0
      && DiffusionDirections[i].gradientDir[1] == 0.0
      && DiffusionDirections[i].gradientDir[2] == 0.0 )
    {
      this->m_baselineLeftNumber = DiffusionDirections[i].repetitionNumber;
      // std::cout<<"DiffusionDirections[i].repetitionNumber: " <<i<<"
      //  "<<DiffusionDirections[i].repetitionNumber <<std::endl;
    }
    else
    {
      this->m_repetitionLeftNumber.push_back(
        DiffusionDirections[i].repetitionNumber);

      double modeSqr =  DiffusionDirections[i].gradientDir[0]
      * DiffusionDirections[i].gradientDir[0]
      + DiffusionDirections[i].gradientDir[1]
      * DiffusionDirections[i].gradientDir[1]
      + DiffusionDirections[i].gradientDir[2]
      * DiffusionDirections[i].gradientDir[2];

      //   std::cout<<"modeSqr: " <<modeSqr <<std::endl;
      if ( dirMode.size() > 0 )
      {
        bool newDirMode = true;
        for ( unsigned int j = 0; j < dirMode.size(); j++ )
        {
          if ( vcl_abs(modeSqr - dirMode[j]) < 0.001 )   // 1 DIFFERENCE for b
            // value
          {
            newDirMode = false;
            break;
          }
        }
        if ( newDirMode && DiffusionDirections[i].repetitionNumber > 0 )
        {
          dirMode.push_back(  modeSqr);
          //           std::cout<<" if(newDirMode) dirMode.size(): " <<
          //  dirMode.size() <<std::endl;
        }
      }
      else
      {
        if ( DiffusionDirections[i].repetitionNumber > 0 )
        {
          dirMode.push_back(  modeSqr);
        }
        // std::cout<<" else dirMode.size(): " <<  dirMode.size() <<std::endl;
      }
    }
  }

  //   std::cout<<" repetNum.size(): " <<  repetNum.size() <<std::endl;
  //   std::cout<<" dirMode.size(): " <<  dirMode.size() <<std::endl;

  this->m_gradientDirLeftNumber = 0;
  this->m_gradientLeftNumber = 0;
  for ( unsigned int i = 0; i < this->m_repetitionLeftNumber.size(); i++ )
  {
    this->m_gradientLeftNumber += this->m_repetitionLeftNumber[i];
    if ( this->m_repetitionLeftNumber[i] > 0 )
    {
      this->m_gradientDirLeftNumber++;
    }
  }

  this->m_bValueLeftNumber = dirMode.size();

  std::cout << "Left DWI Diffusion: "  << std::endl;
  std::cout << "\tbaselineLeftNumber: "  << m_baselineLeftNumber  << std::endl;
  std::cout << "\tbValueLeftNumber: "  << m_bValueLeftNumber    << std::endl;
  std::cout << "\tgradientDirLeftNumber: " << m_gradientDirLeftNumber
    << std::endl;

  if ( reportfilename.length() > 0 )
  {
    std::ofstream outfile;
    outfile.open( this->m_GlobalReportFileName.c_str(), std::ios::app);
    outfile << "--------------------------------" << std::endl;
    outfile << "Diffusion Gradient information:" << std::endl;

    outfile << "Left DWI Diffusion: "    << std::endl;
    outfile << "\tbaselineLeftNumber: "  << m_baselineLeftNumber  << std::endl;
    outfile << "\tbValueLeftNumber: "    << m_bValueLeftNumber    << std::endl;
    outfile << "\tgradientDirLeftNumber: " << m_gradientDirLeftNumber
      << std::endl;

    for ( unsigned int i = 0; i < DiffusionDirections.size(); i++ )
    {
      std::cout << "\t" << i << "\t[ "
        << std::setw(9) << std::setiosflags(std::ios::fixed)
        << std::setprecision(6) << std::setiosflags(std::ios::right)
        << DiffusionDirections[i].gradientDir[0] << ", "
        << std::setw(9) << std::setiosflags(std::ios::fixed)
        << std::setprecision(6) << std::setiosflags(std::ios::right)
        << DiffusionDirections[i].gradientDir[1] << ", "
        << std::setw(9) << std::setiosflags(std::ios::fixed)
        << std::setprecision(6) << std::setiosflags(std::ios::right)
        << DiffusionDirections[i].gradientDir[2] << " ]"
        << "\t" << DiffusionDirections[i].repetitionNumber << std::endl;

      outfile << "\t" << i << "\t[ "
        << std::setw(9) << std::setiosflags(std::ios::fixed)
        << std::setprecision(6) << std::setiosflags(std::ios::right)
        << DiffusionDirections[i].gradientDir[0] << ", "
        << std::setw(9) << std::setiosflags(std::ios::fixed)
        << std::setprecision(6) << std::setiosflags(std::ios::right)
        << DiffusionDirections[i].gradientDir[1] << ", "
        << std::setw(9) << std::setiosflags(std::ios::fixed)
        << std::setprecision(6) << std::setiosflags(std::ios::right)
        << DiffusionDirections[i].gradientDir[2] << " ]"
        << "\t" << DiffusionDirections[i].repetitionNumber << std::endl;
    }
    outfile.close();
  }

  return;
}

void CIntensityMotionCheck::collectDiffusionStatistics()
{
  std::vector<DiffusionDir> DiffusionDirections;
  DiffusionDirections.clear();

  //   std::cout<<"this->GetDiffusionProtocol().gradients.size(): " <<
  // this->GetDiffusionProtocol().gradients.size() <<std::endl;
  for ( unsigned int i = 0; i < this->m_GradientDirectionContainer->size(); i++ )
  {
    if ( DiffusionDirections.size() > 0 )
    {
      bool newDir = true;
      for ( unsigned int j = 0; j < DiffusionDirections.size(); j++ )
      {
        if ( this->m_GradientDirectionContainer->ElementAt(i)[0] ==
          DiffusionDirections[j].gradientDir[0]
        && this->m_GradientDirectionContainer->ElementAt(i)[1] ==
          DiffusionDirections[j].gradientDir[1]
        && this->m_GradientDirectionContainer->ElementAt(i)[2] ==
          DiffusionDirections[j].gradientDir[2] )
        {
          DiffusionDirections[j].repetitionNumber++;
          newDir = false;
        }
      }
      if ( newDir )
      {
        std::vector<double> dir;
        dir.push_back(this->m_GradientDirectionContainer->ElementAt(i)[0]);
        dir.push_back(this->m_GradientDirectionContainer->ElementAt(i)[1]);
        dir.push_back(this->m_GradientDirectionContainer->ElementAt(i)[2]);

        DiffusionDir diffusionDir;
        diffusionDir.gradientDir = dir;
        diffusionDir.repetitionNumber = 1;

        DiffusionDirections.push_back(diffusionDir);
      }
    }
    else
    {
      std::vector<double> dir;
      dir.push_back(this->m_GradientDirectionContainer->ElementAt(i)[0]);
      dir.push_back(this->m_GradientDirectionContainer->ElementAt(i)[1]);
      dir.push_back(this->m_GradientDirectionContainer->ElementAt(i)[2]);

      DiffusionDir diffusionDir;
      diffusionDir.gradientDir = dir;
      diffusionDir.repetitionNumber = 1;

      DiffusionDirections.push_back(diffusionDir);
    }
  }

  //  std::cout<<" DiffusionDirections.size(): " << DiffusionDirections.size()
  // <<std::endl;

  std::vector<int> repetNum;
  repetNum.clear();
  std::vector<double> dirMode;
  dirMode.clear();

  for ( unsigned int i = 0; i < DiffusionDirections.size(); i++ )
  {
    if ( DiffusionDirections[i].gradientDir[0] == 0.0
      && DiffusionDirections[i].gradientDir[1] == 0.0
      && DiffusionDirections[i].gradientDir[2] == 0.0 )
    {
      this->m_baselineNumber = DiffusionDirections[i].repetitionNumber;
      //      std::cout<<" DiffusionDirections[i].repetitionNumber: " <<i<<"
      //  "<<DiffusionDirections[i].repetitionNumber <<std::endl;
    }
    else
    {
      repetNum.push_back(DiffusionDirections[i].repetitionNumber);

      double modeSqr =  DiffusionDirections[i].gradientDir[0]
      * DiffusionDirections[i].gradientDir[0]
      + DiffusionDirections[i].gradientDir[1]
      * DiffusionDirections[i].gradientDir[1]
      + DiffusionDirections[i].gradientDir[2]
      * DiffusionDirections[i].gradientDir[2];

      //      std::cout<<" modeSqr: " <<modeSqr <<std::endl;
      if ( dirMode.size() > 0 )
      {
        bool newDirMode = true;
        for ( unsigned int j = 0; j < dirMode.size(); j++ )
        {
          if ( vcl_abs(modeSqr - dirMode[j]) < 0.001 )   // 1 DIFFERENCE for b
            // value
          {
            newDirMode = false;
            break;
          }
        }
        if ( newDirMode )
        {
          dirMode.push_back(  modeSqr);
          //           std::cout<<" if(newDirMode) dirMode.size(): " <<
          //  dirMode.size() <<std::endl;
        }
      }
      else
      {
        dirMode.push_back(  modeSqr);
        //         std::cout<<" else dirMode.size(): " <<  dirMode.size()
        // <<std::endl;
      }
    }
  }

  //     std::cout<<"  repetNum.size(): " <<  repetNum.size() <<std::endl;
  //     std::cout<<"  dirMode.size(): " <<  dirMode.size() <<std::endl;

  this->m_gradientDirNumber = repetNum.size();
  this->m_bValueNumber = dirMode.size();

  if ( repetNum.size() > 1 )
  {
    m_repetitionNumber = repetNum[0];
    for ( unsigned int i = 1; i < repetNum.size(); i++ )
    {
      if ( repetNum[i] != repetNum[0] )
      {
        std::cout   << "DWI data error. Not all the gradient directions have same repetition. "
                      << "GradientNumber= " << i << " " << repetNum[i] << " != " << repetNum[0]
            << std::endl;
        m_repetitionNumber = -1;
      }
    }
  }
  else
  {
    std::cout << " too less independent gradient dir detected in DWI "
      << std::endl;
  }

  this->m_gradientNumber = this->m_GradientDirectionContainer->size()
    - this->m_baselineNumber;

  //   std::cout<<"DWI Diffusion: "    <<std::endl;
  //  std::cout<<"  baselineNumber: "    <<baselineNumber  <<std::endl;
  //  std::cout<<"  bValueNumber: "    <<bValueNumber    <<std::endl;
  //  std::cout<<"  gradientDirNumber: "  <<gradientDirNumber  <<std::endl;
  //  std::cout<<"  gradientNumber: "    <<gradientNumber  <<std::endl;
  //  std::cout<<"  repetitionNumber: "  <<repetitionNumber  <<std::endl;
  return;
}

bool CIntensityMotionCheck::dtiestim()
{
  if ( !protocol->GetDTIProtocol().bCompute )
  {
    std::cout << "DTI computing NOT set" << std::endl;
    return true;
  }

  // dtiestim
  std::string str;
  str.append(protocol->GetDTIProtocol().dtiestimCommand);
  str.append(" ");

  str.append("--dwi_image");
  str.append(" ");

  std::string outputDWIFileName;
  if ( protocol->GetQCedDWIFileNameSuffix().length() > 0 )
  {
    if ( protocol->GetQCOutputDirectory().length() > 0 )
    {
      if ( protocol->GetQCOutputDirectory().at( protocol->GetQCOutputDirectory()
        .length() - 1 ) == '\\'
        || protocol->GetQCOutputDirectory().at( protocol->
        GetQCOutputDirectory().length() - 1 ) == '/'     )
      {
        outputDWIFileName = protocol->GetQCOutputDirectory().substr(
          0, protocol->GetQCOutputDirectory().find_last_of("/\\") );
      }
      else
      {
        outputDWIFileName = protocol->GetQCOutputDirectory();
      }

      outputDWIFileName.append( "/" );

      std::string strLocal
        = m_DwiFileName.substr( 0, m_DwiFileName.find_last_of('.') );
      strLocal = strLocal.substr( strLocal.find_last_of("/\\") + 1);

      outputDWIFileName.append( strLocal );
      outputDWIFileName.append( protocol->GetQCedDWIFileNameSuffix() );
    }
    else
    {
      outputDWIFileName = m_DwiFileName.substr( 0, m_DwiFileName.find_last_of('.') );
      outputDWIFileName.append( protocol->GetQCedDWIFileNameSuffix() );
    }
  }
  else
  {
    outputDWIFileName = m_DwiFileName.substr( 0, m_DwiFileName.find_last_of('.') );
    outputDWIFileName.append( protocol->GetQCedDWIFileNameSuffix() );
  }
  str.append(outputDWIFileName);
  str.append(" ");

  std::string OutputTensor;
  OutputTensor
    = outputDWIFileName.substr( 0, outputDWIFileName.find_last_of('.') );
  OutputTensor.append( protocol->GetDTIProtocol().tensorSuffix);
  str.append("--tensor_output");
  str.append(" ");
  str.append(OutputTensor);

  if ( protocol->GetDTIProtocol().method == Protocol::METHOD_WLS )
  {
    str.append(" -m wls ");
  }
  else if ( protocol->GetDTIProtocol().method == Protocol::METHOD_ML )
  {
    str.append(" -m ml ");
  }
  else if ( protocol->GetDTIProtocol().method == Protocol::METHOD_NLS )
  {
    str.append(" -m nls ");
  }
  else
  {
    str.append(" -m lls ");
  }

  if ( protocol->GetDTIProtocol().mask.length() > 0 )
  {
    str.append(" -M ");
    str.append( protocol->GetDTIProtocol().mask );
  }

  str.append(" -t ");
  char buffer[10];
  sprintf( buffer, "%d", protocol->GetDTIProtocol().baselineThreshold );
  str.append( buffer );

  if ( protocol->GetDTIProtocol().bidwi )
  {
    str.append(" --idwi ");
    std::string idwi;
    idwi = outputDWIFileName.substr( 0, outputDWIFileName.find_last_of('.') );
    idwi.append(protocol->GetDTIProtocol().idwiSuffix);
    str.append(idwi);
  }

  if ( protocol->GetDTIProtocol().bbaseline )
  {
    str.append(" --B0 ");
    std::string baseline;
    baseline = outputDWIFileName.substr( 0, outputDWIFileName.find_last_of('.') );
    baseline.append(protocol->GetDTIProtocol().baselineSuffix);
    str.append(baseline);
  }

  std::cout << "===============  Starting dtiestim command ===============" << std::endl;
  std::cout << "dtiestim command: " << str.c_str() << std::endl;
  system( str.c_str() );
  return true;
}

bool CIntensityMotionCheck::DTIComputing()
{
  if ( !protocol->GetDTIProtocol().bCompute )
  {
    std::cout << "DTI computing NOT set" << std::endl;
    return true;
  }

  dtiestim();
  dtiprocess();
  return true;
}

bool CIntensityMotionCheck::dtiprocess()
{
  std::string string;

  string.append(protocol->GetDTIProtocol().dtiprocessCommand);
  string.append(" ");

  std::string outputDWIFileName;
  if ( protocol->GetQCedDWIFileNameSuffix().length() > 0 )
  {
    if ( protocol->GetQCOutputDirectory().length() > 0 )
    {
      if ( protocol->GetQCOutputDirectory().at( protocol->GetQCOutputDirectory()
        .length() - 1 ) == '\\'
        || protocol->GetQCOutputDirectory().at( protocol->
        GetQCOutputDirectory().length() - 1 ) == '/'     )
      {
        outputDWIFileName = protocol->GetQCOutputDirectory().substr(
          0, protocol->GetQCOutputDirectory().find_last_of("/\\") );
      }
      else
      {
        outputDWIFileName = protocol->GetQCOutputDirectory();
      }

      outputDWIFileName.append( "/" );

      std::string str = m_DwiFileName.substr( 0, m_DwiFileName.find_last_of('.') );
      str = str.substr( str.find_last_of("/\\") + 1);

      outputDWIFileName.append( str );
      outputDWIFileName.append( protocol->GetQCedDWIFileNameSuffix() );
    }
    else
    {
      outputDWIFileName = m_DwiFileName.substr( 0, m_DwiFileName.find_last_of('.') );
      outputDWIFileName.append( protocol->GetQCedDWIFileNameSuffix() );
    }
  }
  else
  {
    outputDWIFileName = m_DwiFileName.substr( 0, m_DwiFileName.find_last_of('.') );
    outputDWIFileName.append( protocol->GetQCedDWIFileNameSuffix() );
  }

  std::string dtiprocessInput;
  dtiprocessInput
    = outputDWIFileName.substr( 0, outputDWIFileName.find_last_of('.') );
  dtiprocessInput.append( protocol->GetDTIProtocol().tensorSuffix);
  
  string.append("--dti_image");
  string.append(" ");
  string.append(dtiprocessInput);

  if ( protocol->GetDTIProtocol().bfa )
  {
    string.append(" -f ");
    std::string fa;
    fa = dtiprocessInput.substr( 0, dtiprocessInput.find_last_of('.') );
    fa.append(protocol->GetDTIProtocol().faSuffix);
    string.append(fa);
  }

  if ( protocol->GetDTIProtocol().bmd )
  {
    string.append(" -m ");
    std::string md;
    md = dtiprocessInput.substr( 0, dtiprocessInput.find_last_of('.') );
    md.append(protocol->GetDTIProtocol().mdSuffix);
    string.append(md);
  }

  if ( protocol->GetDTIProtocol().bcoloredfa )
  {
    string.append(" --color_fa_output ");
    std::string cfa;
    cfa = dtiprocessInput.substr( 0, dtiprocessInput.find_last_of('.') );
    cfa.append(protocol->GetDTIProtocol().coloredfaSuffix);
    string.append(cfa);
  }

  if ( protocol->GetDTIProtocol().bfrobeniusnorm )
  {
    string.append(" --frobenius-norm-output ");
    std::string fn;
    fn = dtiprocessInput.substr( 0, dtiprocessInput.find_last_of('.') );
    fn.append(protocol->GetDTIProtocol().frobeniusnormSuffix);
    string.append(fn);
  }

  std::cout << "===============  Starting dtiprocess command ===============" << std::endl;
  std::cout << "dtiprocess command: " << string.c_str() << std::endl;
  system( string.c_str() );

  return true;
}




bool CIntensityMotionCheck::DiffusionCheck( DwiImageType::Pointer dwi)
{
  bool bReport = false;

  //HACK:  TODO:  Extracing the ReportFileName has a lot of copy and paste code
  // several time in the source.  This should be pulled out into it's own
  // function as part of the protocol class.  I did it for this one case,
  // but there are several  other cases in this file also need to be done
  //
  //This functionality is probably best suited to be in the protocol class
  //as member functions:  Instead of:
  //protocol->GetDiffusionProtocol().reportFileNameSuffix
  //USE
  //protocol->GetDiffusionProtocol().GetReportFileName();
  const std::string ReportFileName=protocol->GetDiffusionProtocolReportFileName(this->m_DwiFileName);

  std::ofstream outfile;
  if ( protocol->GetImageProtocol().reportFileMode == 1 )
  {
    outfile.open( ReportFileName.c_str(), std::ios_base::app);
  }
  else
  {
    outfile.open( ReportFileName.c_str() );
  }
  if ( outfile )
  {
    bReport = true;
  }

  if ( (bReport && protocol->GetReportType() == 1) || (protocol->GetReportType() == 0) )
  {
    outfile << std::endl;
    outfile << "================================" << std::endl;
    outfile << " Diffusion Information checking " << std::endl;
    outfile << "================================" << std::endl;
  }

  bool returnValue = true;

  outfile << std::endl;
  if ( !protocol->GetDiffusionProtocol().bCheck )
  {
    if ( bReport )
    {
      outfile << "Diffusion information check NOT set." << std::endl;
    }
    std::cout << "Diffusion information check NOT set." << std::endl;
    return true;
  }
  else
  {
    if ( !dwi )
    {
      std::cout << "DWI error." << std::endl;
      m_bGetGradientDirections = false;
      return false;
    }

    GradientDirectionContainerType::Pointer GradContainer
      = GradientDirectionContainerType::New();
    double bValue;
    this->GetGradientDirections( dwi, bValue, GradContainer);

    const double bValueAcceptablePercentageTolerance=0.005;
    if ( vcl_abs( ( protocol->GetDiffusionProtocol().bValue - bValue )/ protocol->GetDiffusionProtocol().bValue ) < bValueAcceptablePercentageTolerance )
    {
      qcResult->GetDiffusionInformationCheckResult().b = true;
      if ( bReport )
      {
        if(protocol->GetReportType() == 1 || protocol->GetReportType() == 0 )
          outfile << "DWMRI_bValue Check: " << "\t\tOK" << std::endl;
        if(protocol->GetReportType() == 2 )
          outfile << "Diffusion_information_checking"
          << "DWMRI_bValue_check " << "OK" << std::endl;
      }
      std::cout << "DWMRI_bValue check: " << "\t\tOK" << std::endl;
    }
    else
    {
      qcResult->GetDiffusionInformationCheckResult().b = false;
      if ( bReport )
      {
        if(protocol->GetReportType() == 1 || protocol->GetReportType() == 0 )
          outfile << "DWMRI_bValue Check: " << "\t\tFAILED" << std::endl;
        if(protocol->GetReportType() == 2 )
          outfile << "Diffusion_information_checking"
          << "DWMRI_bValue_check " << "FAILED" << std::endl;
        if(protocol->GetReportType() == 1)
          outfile << "DWMRI_bValue\t\tmismatch with DWI = " << this->m_b0
          << "\t\t\t\tprotocol = "
          << protocol->GetDiffusionProtocol().bValue << std::endl;
      }
      std::cout << "Diffusion b-value Check:\t\tFAILED" << std::endl;
      //       std::cout <<"DWMRI_bValue\t\tmismatch with DWI = " << this->b0
      //         << "\t\t\t\tprotocol = " <<
      // protocol->GetDiffusionProtocol().bValue <<std::endl;

      if ( protocol->GetDiffusionProtocol().bUseDiffusionProtocol )
      {
        std::ostringstream ossMetaString;
        ossMetaString << protocol->GetDiffusionProtocol().bValue;
        itk::EncapsulateMetaData<std::string>(
          dwi->GetMetaDataDictionary(), "DWMRI_b-value", ossMetaString.str() );
      }
      returnValue = false;
    }

    // HJJ -- measurement frame is not being properly used here.
    // measurement frame
    const vnl_matrix_fixed<double, 3, 3> imageMeasurementFrame 
      = GetMeasurementFrame(this->m_DwiForcedConformanceImage);
    
    // It is not required that the measurement frames are the same for all images.
    // Images collected at different oblique angles will likely have different measurement frames.
	  
    // JTM - START DEBUG: delete later
    std::cout << "Image measurement frame (scanner space) \n" <<  imageMeasurementFrame << std::endl;
    // JTM - END DEBUG: delete later
    
    // JTM - START DEBUG: delete later
    vnl_matrix_fixed<double, 3, 3> protocolMeasurementFrame
    = protocol->GetDiffusionProtocol().measurementFrame;
    std::cout << "Protocol measurement frame (scanner space) \n" <<  protocolMeasurementFrame << std::endl;
    // JTM - END DEBUG: delete later
    
    // JTM - Normalize protocol measurement frame
    vnl_vector_fixed<double, 3> tempMeasurementFrameFromProtocol1;
    tempMeasurementFrameFromProtocol1[0] = protocol->GetDiffusionProtocol().measurementFrame[0][0];
    tempMeasurementFrameFromProtocol1[1] = protocol->GetDiffusionProtocol().measurementFrame[0][1];
    tempMeasurementFrameFromProtocol1[2] = protocol->GetDiffusionProtocol().measurementFrame[0][2];
    vnl_vector_fixed<double, 3> tempMeasurementFrameFromProtocol2;
    tempMeasurementFrameFromProtocol2[0] = protocol->GetDiffusionProtocol().measurementFrame[1][0];
    tempMeasurementFrameFromProtocol2[1] = protocol->GetDiffusionProtocol().measurementFrame[1][1];
    tempMeasurementFrameFromProtocol2[2] = protocol->GetDiffusionProtocol().measurementFrame[1][2];
    vnl_vector_fixed<double, 3> tempMeasurementFrameFromProtocol3;
    tempMeasurementFrameFromProtocol3[0] = protocol->GetDiffusionProtocol().measurementFrame[2][0];
    tempMeasurementFrameFromProtocol3[1] = protocol->GetDiffusionProtocol().measurementFrame[2][1];
    tempMeasurementFrameFromProtocol3[2] = protocol->GetDiffusionProtocol().measurementFrame[2][2];  
    
    vnl_vector_fixed<double, 3> normMeasurementFrameFromProtocol1
    = tempMeasurementFrameFromProtocol1.normalize();
    vnl_vector_fixed<double, 3> normMeasurementFrameFromProtocol2
    =	tempMeasurementFrameFromProtocol2.normalize();
    vnl_vector_fixed<double, 3> normMeasurementFrameFromProtocol3
    = tempMeasurementFrameFromProtocol3.normalize();
    
    vnl_matrix_fixed<double, 3, 3> normMeasurementFrameFromProtocol;
    normMeasurementFrameFromProtocol[0][0] = normMeasurementFrameFromProtocol1[0];
    normMeasurementFrameFromProtocol[0][1] = normMeasurementFrameFromProtocol1[1];
    normMeasurementFrameFromProtocol[0][2] = normMeasurementFrameFromProtocol1[2];
    normMeasurementFrameFromProtocol[1][0] = normMeasurementFrameFromProtocol2[0];
    normMeasurementFrameFromProtocol[1][1] = normMeasurementFrameFromProtocol2[1];
    normMeasurementFrameFromProtocol[1][2] = normMeasurementFrameFromProtocol2[2];
    normMeasurementFrameFromProtocol[2][0] = normMeasurementFrameFromProtocol3[0];
    normMeasurementFrameFromProtocol[2][1] = normMeasurementFrameFromProtocol3[1];
    normMeasurementFrameFromProtocol[2][2] = normMeasurementFrameFromProtocol3[2];
    
    // JTM - START DEBUG: delete later
    std::cout << "Protocol measurement frame (normalized) \n" <<  normMeasurementFrameFromProtocol << std::endl;
    // JTM - END DEBUG: delete later
    
    // JTM - Take inverse of normalized protocol measurement frame
    const vnl_matrix_fixed<double, 3, 3> mfInverseFromProtocol
    = vnl_inverse(normMeasurementFrameFromProtocol);
    
    // JTM - START DEBUG: delete later
    std::cout << "Protocol measurement frame (normalized inverse) \n" <<  mfInverseFromProtocol << std::endl;
    // JTM - END DEBUG: delete later
    
    // JTM - Normalize image measurement frame
    vnl_vector_fixed<double, 3> tempMeasurementFrameFromImage1;
    tempMeasurementFrameFromImage1[0] = imageMeasurementFrame[0][0];
    tempMeasurementFrameFromImage1[1] = imageMeasurementFrame[0][1];
    tempMeasurementFrameFromImage1[2] = imageMeasurementFrame[0][2];
    vnl_vector_fixed<double, 3> tempMeasurementFrameFromImage2;
    tempMeasurementFrameFromImage2[0] = imageMeasurementFrame[1][0];
    tempMeasurementFrameFromImage2[1] = imageMeasurementFrame[1][1];
    tempMeasurementFrameFromImage2[2] = imageMeasurementFrame[1][2];
    vnl_vector_fixed<double, 3> tempMeasurementFrameFromImage3;
    tempMeasurementFrameFromImage3[0] = imageMeasurementFrame[2][0];
    tempMeasurementFrameFromImage3[1] = imageMeasurementFrame[2][1];
    tempMeasurementFrameFromImage3[2] = imageMeasurementFrame[2][2];
    
    vnl_vector_fixed<double, 3> normMeasurementFrameFromImage1
    = tempMeasurementFrameFromImage1.normalize();
    vnl_vector_fixed<double, 3> normMeasurementFrameFromImage2
    =	tempMeasurementFrameFromImage2.normalize();
    vnl_vector_fixed<double, 3> normMeasurementFrameFromImage3
    = tempMeasurementFrameFromImage3.normalize();
    
    vnl_matrix_fixed<double, 3, 3> normMeasurementFrameFromImage;
    normMeasurementFrameFromImage[0][0] = normMeasurementFrameFromImage1[0];
    normMeasurementFrameFromImage[0][1] = normMeasurementFrameFromImage1[1];
    normMeasurementFrameFromImage[0][2] = normMeasurementFrameFromImage1[2];
    normMeasurementFrameFromImage[1][0] = normMeasurementFrameFromImage2[0];
    normMeasurementFrameFromImage[1][1] = normMeasurementFrameFromImage2[1];
    normMeasurementFrameFromImage[1][2] = normMeasurementFrameFromImage2[2];
    normMeasurementFrameFromImage[2][0] = normMeasurementFrameFromImage3[0];
    normMeasurementFrameFromImage[2][1] = normMeasurementFrameFromImage3[1];
    normMeasurementFrameFromImage[2][2] = normMeasurementFrameFromImage3[2];
    
    // JTM - START DEBUG: delete later
    std::cout << "Image measurement frame (normalized) \n" <<  normMeasurementFrameFromImage << std::endl;
    // JTM - END DEBUG: delete later
    
    // JTM - Take inverse of normalized image measurement frame
    const vnl_matrix_fixed<double, 3, 3> mfInverseFromImage
    = vnl_inverse(normMeasurementFrameFromImage);
    
    // JTM - START DEBUG: delete later
    std::cout << "Image measurement frame (normalized inverse) \n" <<  mfInverseFromImage << std::endl;
    // JTM - END DEBUG: delete later
	  
    bool result = true;
    if ( GradContainer->size() !=
      protocol->GetDiffusionProtocol().gradients.size() )
    {
      qcResult->GetDiffusionInformationCheckResult().gradient = false;
      if ( bReport )
      {
        if(protocol->GetReportType() == 1 || protocol->GetReportType() == 0 )
          outfile << "Diffusion vector # mismatch with protocol = "
          << protocol->GetDiffusionProtocol().gradients.size()
          << " image = " << GradContainer->size() << std::endl;
        if(protocol->GetReportType() == 2) 
          outfile << "Diffusion_information_checking "
          << "Diffusion_vector_check "<< "FAILED" << std::endl;
      }

      std::cout  << "Diffusion vector # mismatch with protocol = "
        << protocol->GetDiffusionProtocol().gradients.size()
        << " image = " << GradContainer->size() << std::endl;

      result = false;
    }
    else // Diffusion vector # matched
    {
      qcResult->GetDiffusionInformationCheckResult().gradient = true;
      for ( unsigned int i = 0; i < GradContainer->size(); i++ )
      {
        // Allow for small differences in colinearity to be considered the same
        // direction.
        // const float gradientTolerancePercentOfSmallestAngle=0.2;
        const float gradientToleranceForSameness = 1; // Allow 1 degree
        // difference for sameness
        //(180.0/static_cast<float>(protocol->GetDiffusionProtocol().gradients.size()))
        //*gradientTolerancePercentOfSmallestAngle;

        // JTM - Gradient from protocol
        vnl_vector_fixed<double, 3> tempGradientFromProtocol;
        tempGradientFromProtocol[0] = protocol->GetDiffusionProtocol().gradients[i][0];
        tempGradientFromProtocol[1] = protocol->GetDiffusionProtocol().gradients[i][1];
        tempGradientFromProtocol[2] = protocol->GetDiffusionProtocol().gradients[i][2];
        
        // JTM - START DEBUG: delete later
        std::cout << "===================================================" << std::endl;
        std::cout << "Gradient " << i << std::endl;
        std::cout << "Gradient from protocol (scanner space) \n" <<  tempGradientFromProtocol << std::endl;
        // JTM - END DEBUG: delete later
		  
        vnl_vector_fixed<double, 3> tempGradientFromImage
          = GradContainer->ElementAt(i);
        
        // JTM - START DEBUG: delete later
        std::cout << "Gradient from image (scanner space) \n" <<  tempGradientFromImage << std::endl;
        // JTM - END DEBUG: delete later
        
        bool bColinear = false;
        //double gradientMinAngle = 90.0;
        double gradientMinAngle = 0.0;
        double gradMagnitude = 0.0;
        double gradProtocolMagnitude = 0.0;
        //if ( vcl_abs(protocol->GetDiffusionProtocol().gradients[i][0]
        //  - GradContainer->ElementAt(i)[0]) < 0.00001
        //  && vcl_abs(protocol->GetDiffusionProtocol().gradients[i][1]
        //  - GradContainer->ElementAt(i)[1]) < 0.00001
        //  && vcl_abs(protocol->GetDiffusionProtocol().gradients[i][2]
        //  - GradContainer->ElementAt(i)[2]) < 0.00001 )
        //  bColinear = true; 

        // to check baselines
        if ( vcl_abs( tempGradientFromImage.magnitude() ) < 1e-4 )
        {
          if( vcl_abs( tempGradientFromProtocol.magnitude() ) < 1e-4 )
            bColinear = true; // image: baseline   protocol: baseline
          else
            bColinear = false; // image: baseline  protocol: non-baseline; missing dir information
        }
        else
        {
          if( vcl_abs( tempGradientFromProtocol.magnitude() ) < 1e-4 )
            bColinear = false;  // image: non-baseline  protocol: baseline
          else   
          {
            // JTM - Already normalized
            //tempGradientFromProtocol.normalize(); 
            //tempGradientFromImage.normalize();
            // Sometimes this is not
            // normalized due to numerical precision problems.
          
            const vnl_vector_fixed<double, 3> gradientFromImage
              = mfInverseFromImage * tempGradientFromImage;
            
            const vnl_vector_fixed<double, 3> gradientFromProtocol
              = mfInverseFromProtocol * tempGradientFromProtocol;
			  
            // JTM - START DEBUG: delete later
            std::cout << "Gradient from image (anatomical space) \n" <<  gradientFromImage << std::endl;
            std::cout << "Gradient from protocol (anatomical space) \n" <<  gradientFromProtocol << std::endl;
            // JTM - END DEBUG: delete later
			  
            //Compute the dot product out of the normalize vectors ! Otherwise, if the bvalue of the current vector is not the max bvalue, the angle is wrong
            //double gradientDot = dot_product(gradientFromProtocol, gradientFromImage);
            //changed to:
            //	  double gradientDot = dot_product(tempGradientFromProtocol, gradientFromImage);
            // JTM - changed above to:
            double gradientDot = dot_product(gradientFromImage, gradientFromProtocol);            
            double magnitudesProduct = gradientFromProtocol.magnitude() * gradientFromImage.magnitude();
            double sendToArcCos = gradientDot / magnitudesProduct;
            
            sendToArcCos = ( sendToArcCos > 1 ) ? 1 : sendToArcCos;
            //Avoid numerical precision problems
            sendToArcCos = ( sendToArcCos < -1 ) ? -1 : sendToArcCos; 
            // Avoid numerical precision problems
	   
            const double gradientAngle = vcl_abs( vcl_acos(sendToArcCos) * 180.0 * vnl_math::one_over_pi);

            gradientMinAngle
              = vcl_min( gradientAngle, vcl_abs(180.0 - gradientAngle) );

            // Now see if the gradients are colinear in opposite directions;
            gradMagnitude = gradientFromImage.magnitude();
            gradProtocolMagnitude = gradientFromProtocol.magnitude();
            //std::cout << "gradProtocolMagnitude: " 
            //<< gradProtocolMagnitude << std::endl;
            if ( gradientMinAngle < gradientToleranceForSameness )
              bColinear = true;   
              // image: non-baseline  protocol: non-baseline --colinear
            else
              bColinear = false;   
              // image: non-baseline  protocol: non-baseline --non-colinear
            
            // JTM - START DEBUG: delete later
            std::cout << "Dot product of 2 vectors: " <<  gradientDot << std::endl;
            std::cout << "Product of vector magnitudes: " << magnitudesProduct << std::endl;
            std::cout << "Dot product of 2 vectors / Product of vector magnitudes: " << sendToArcCos << std::endl;
            std::cout << "Angle between vectors (raw): " <<  gradientAngle << std::endl;
            std::cout << "Minimum angle between vectors: " <<  gradientMinAngle << std::endl;
            // JTM - END DEBUG: delete later
          }
        }

        if( bColinear )
        {
          result = result && true;
        }
        else
        {
          if ( bReport )
          {
            outfile  << "DWMRI_gradient_" << std::setw(4)
              << std::setfill('0') << i << " mismatch! DWI: [ "
              << std::setw(9) << std::setiosflags(std::ios::fixed)
              << std::setprecision(6) << std::setiosflags(
              std::ios::right)
              << GradContainer->ElementAt(i)[0] << " "
              << std::setw(9) << std::setiosflags(std::ios::fixed)
              << std::setprecision(6) << std::setiosflags(
              std::ios::right)
              << GradContainer->ElementAt(i)[1] << " "
              << std::setw(9) << std::setiosflags(std::ios::fixed)
              << std::setprecision(6) << std::setiosflags(
              std::ios::right)
              << GradContainer->ElementAt(i)[2] << " ] protocol: [ "
              << std::setw(9) << std::setiosflags(std::ios::fixed)
              << std::setprecision(6) << std::setiosflags(
              std::ios::right)
              << protocol->GetDiffusionProtocol().gradients[i][0] << " "
              << std::setw(9) << std::setiosflags(std::ios::fixed)
              << std::setprecision(6) << std::setiosflags(
              std::ios::right)
              << protocol->GetDiffusionProtocol().gradients[i][1] << " "
              << std::setw(9) << std::setiosflags(std::ios::fixed)
              << std::setprecision(6) << std::setiosflags(
              std::ios::right)
              << protocol->GetDiffusionProtocol().gradients[i][2]
              << " ]"
              << "  Colinearity angle (degrees): "
              << gradientMinAngle << " > "
              << gradientToleranceForSameness  
              //<< " : "<< gradMagnitude << " - "
              //<< gradProtocolMagnitude 
              << std::endl;

              std::cout << "DWMRI_gradient_" << std::setw(4)
              << std::setfill('0') << i << " mismatch! DWI: [ "
              << std::setw(9) << std::setiosflags(std::ios::fixed)
              << std::setprecision(6) << std::setiosflags(
              std::ios::right)
              << GradContainer->ElementAt(i)[0] << " "
              << std::setw(9) << std::setiosflags(std::ios::fixed)
              << std::setprecision(6) << std::setiosflags(
              std::ios::right)
              << GradContainer->ElementAt(i)[1] << " "
              << std::setw(9) << std::setiosflags(std::ios::fixed)
              << std::setprecision(6) << std::setiosflags(
              std::ios::right)
              << GradContainer->ElementAt(i)[2] << " ] protocol: [ "
              << std::setw(9) << std::setiosflags(std::ios::fixed)
              << std::setprecision(6) << std::setiosflags(
              std::ios::right)
              << protocol->GetDiffusionProtocol().gradients[i][0]
              << " "
              << std::setw(9) << std::setiosflags(std::ios::fixed)
              << std::setprecision(6) << std::setiosflags(
              std::ios::right)
              << protocol->GetDiffusionProtocol().gradients[i][1]
              << " "
              << std::setw(9) << std::setiosflags(std::ios::fixed)
              << std::setprecision(6) << std::setiosflags(
              std::ios::right)
              << protocol->GetDiffusionProtocol().gradients[i][2]
              << " ]"
              << "  Colinearity angle (degrees): "
              <<  gradientMinAngle << " > "
              << gradientToleranceForSameness
              //<< " : "<< gradMagnitude << " - "
              //<< gradProtocolMagnitude 
              << std::endl;
          }

          if ( protocol->GetDiffusionProtocol().bUseDiffusionProtocol )
          {
            std::ostringstream ossMetaString, ossMetaKey;
            ossMetaKey << "DWMRI_gradient_" << std::setw(4)
              << std::setfill('0') << i;
            ossMetaString << std::setw(9)
              << std::setiosflags(std::ios::fixed)
              << std::setprecision(6)
              << std::setiosflags(std::ios::right)
              << protocol->GetDiffusionProtocol().gradients[i][0]
              << "    "
              << std::setw(9)
              << std::setiosflags(std::ios::fixed)
              << std::setprecision(6)
              << std::setiosflags(std::ios::right)
              << protocol->GetDiffusionProtocol().gradients[i][1]
              << "    "
              << std::setw(9)
              << std::setiosflags(std::ios::fixed)
              << std::setprecision(6)
              << std::setiosflags(std::ios::right)
              << protocol->GetDiffusionProtocol().gradients[i][2];

            itk::EncapsulateMetaData<std::string>(
              dwi->GetMetaDataDictionary(),
              ossMetaKey.str(),  ossMetaString.str() );

            qcResult->GetIntensityMotionCheckResult()[i].ReplacedDir[0]
              =  protocol->GetDiffusionProtocol().gradients[i][0];
            qcResult->GetIntensityMotionCheckResult()[i].ReplacedDir[1]
              =  protocol->GetDiffusionProtocol().gradients[i][1];
            qcResult->GetIntensityMotionCheckResult()[i].ReplacedDir[2]
              =  protocol->GetDiffusionProtocol().gradients[i][2];

            qcResult->GetIntensityMotionCheckResult()[i].CorrectedDir[0]
              = protocol->GetDiffusionProtocol().gradients[i][0];
            qcResult->GetIntensityMotionCheckResult()[i].CorrectedDir[1]
              = protocol->GetDiffusionProtocol().gradients[i][1];
            qcResult->GetIntensityMotionCheckResult()[i].CorrectedDir[2]
              = protocol->GetDiffusionProtocol().gradients[i][2];
          }

          qcResult->GetDiffusionInformationCheckResult().gradient = false;
          result = false;
        }
      }
    }

    if ( result )
    {
      qcResult->GetDiffusionInformationCheckResult().gradient = true;
      if ( bReport )
      {
        if( protocol->GetReportType() == 1 || protocol->GetReportType() == 0 )
          outfile << "Diffusion gradient Check: \tOK" << std::endl;
        if( protocol->GetReportType() == 2 )   
          outfile << "Diffusion_information_checking "
          << "Diffusion_vector_check "<< "OK" << std::endl;
      }
      std::cout << "Diffusion gradient Check: \tOK" << std::endl;
    }
    else
    {
      qcResult->GetDiffusionInformationCheckResult().gradient = false;
      if ( bReport )
      {
        if( protocol->GetReportType() == 1 || protocol->GetReportType() == 0 )   
          outfile << "Diffusion gradient Check: \tFAILED" << std::endl;
        if( protocol->GetReportType() == 2 )   
          outfile << "Diffusion_information_checking "
            << "Diffusion_vector_check "<< "FAILED" << std::endl;
        if(protocol->GetDiffusionProtocol().bUseDiffusionProtocol)
          outfile << "Mismatched information was replaced with that from protocol." 
            << std::endl;

      }
      std::cout << "Diffusion gradient Check: \tFAILED" << std::endl;
      if(protocol->GetDiffusionProtocol().bUseDiffusionProtocol)
        std::cout << "Mismatched information was replaced with that from protocol." 
            << std::endl;
    }

    returnValue = returnValue && result;

    if ( !returnValue )
    {
      std::cout
        << "Diffusion information Check FAILED." << std::endl;
      if ( bReport )
      {
        outfile
          << "Diffusion information Check FAILED." 
          << std::endl;
      }
    }
  }

  if ( bReport )
  {
    outfile.close();
  }

  // then save the updated DWI
  if ( protocol->GetDiffusionProtocol().diffusionReplacedDWIFileNameSuffix.length() > 0
    && protocol->GetDiffusionProtocol().bUseDiffusionProtocol
    && ( !qcResult->GetDiffusionInformationCheckResult().b
    || !qcResult->GetDiffusionInformationCheckResult().gradient
    || !qcResult->GetDiffusionInformationCheckResult().measurementFrame ) )
  {

    std::string DWIFileName;
    if ( protocol->GetQCOutputDirectory().length() > 0 )
    {
      if ( protocol->GetQCOutputDirectory().at( protocol->GetQCOutputDirectory()
        .length() - 1 ) == '\\'
        || protocol->GetQCOutputDirectory().at( protocol->
        GetQCOutputDirectory().length() - 1 ) == '/'     )
      {
        DWIFileName = protocol->GetQCOutputDirectory().substr(
          0, protocol->GetQCOutputDirectory().find_last_of("/\\") );
      }
      else
      {
        DWIFileName = protocol->GetQCOutputDirectory();
      }

      DWIFileName.append( "/" );

      std::string str = m_DwiFileName.substr( 0, m_DwiFileName.find_last_of('.') );
      str = str.substr( str.find_last_of("/\\") + 1);

      DWIFileName.append( str );
      DWIFileName.append(
        protocol->GetDiffusionProtocol().diffusionReplacedDWIFileNameSuffix );
    }
    else
    {
      DWIFileName = m_DwiFileName.substr( 0, m_DwiFileName.find_last_of('.') );
      DWIFileName.append(
        protocol->GetDiffusionProtocol().diffusionReplacedDWIFileNameSuffix );
    }

    try
    {
      std::cout << "Saving diffusion information updated DWI: "
        << DWIFileName << "...";
      DwiWriterType::Pointer DwiWriter = DwiWriterType::New();
      itk::NrrdImageIO::Pointer myNrrdImageIO = itk::NrrdImageIO::New();
      DwiWriter->SetFileName( DWIFileName );
      DwiWriter->SetInput( this->m_DwiForcedConformanceImage);
      DwiWriter->UseCompressionOn();
      DwiWriter->Update();
    }
    catch ( itk::ExceptionObject & e )
    {
      std::cout << e.GetDescription() << std::endl;
      // return false;
    }
    std::cout << "DONE" << std::endl;
  }

  return returnValue;
}

bool CIntensityMotionCheck::MakeDefaultProtocol( Protocol *protocol )
{

  if ( !protocol )
  {
    std::cout << "protocol error." << std::endl;    
    return false;
  }

  if ( !m_bDwiLoaded  )
  {
    LoadDwiImage();
  }
  
    if ( !m_DwiOriginalImage )
  {
    std::cout << "DWI error." << std::endl;    
    return false;
  }

  protocol->clear();
  protocol->initDTIProtocol();

  protocol->GetQCOutputDirectory() = "";
  protocol->GetQCedDWIFileNameSuffix() = "_QCed.nhdr";
  protocol->GetReportFileNameSuffix() = "_QCReport.txt";
  protocol->SetBadGradientPercentageTolerance(0.2);
  protocol->SetReportType(0);

  // ***** image
  protocol->GetImageProtocol().bCheck = true;

  // size
  protocol->GetImageProtocol().size[0]
  = m_DwiOriginalImage->GetLargestPossibleRegion().GetSize()[0];
  protocol->GetImageProtocol().size[1]
  = m_DwiOriginalImage->GetLargestPossibleRegion().GetSize()[1];
  protocol->GetImageProtocol().size[2]
  = m_DwiOriginalImage->GetLargestPossibleRegion().GetSize()[2];

  // origin
  protocol->GetImageProtocol().origin[0] = m_DwiOriginalImage->GetOrigin()[0];
  protocol->GetImageProtocol().origin[1] = m_DwiOriginalImage->GetOrigin()[1];
  protocol->GetImageProtocol().origin[2] = m_DwiOriginalImage->GetOrigin()[2];

  // spacing
  protocol->GetImageProtocol().spacing[0] = m_DwiOriginalImage->GetSpacing()[0];
  protocol->GetImageProtocol().spacing[1] = m_DwiOriginalImage->GetSpacing()[1];
  protocol->GetImageProtocol().spacing[2] = m_DwiOriginalImage->GetSpacing()[2];

  // space
  itk::MetaDataDictionary imgMetaDictionary
    = m_DwiOriginalImage->GetMetaDataDictionary();
  std::vector<std::string> imgMetaKeys
    = imgMetaDictionary.GetKeys();
  std::vector<std::string>::const_iterator itKey = imgMetaKeys.begin();
  std::string                              metaString;

  itk::ExposeMetaData<std::string>(imgMetaDictionary, "NRRD_space", metaString);
  if ( metaString == "left-anterior-inferior" )
  {
    protocol->GetImageProtocol().space = Protocol::SPACE_LAI;
  }
  else if ( metaString == "left-anterior-superior" )
  {
    protocol->GetImageProtocol().space = Protocol::SPACE_LAS;
  }
  else if ( metaString == "left-posterior-inferior" )
  {
    protocol->GetImageProtocol().space = Protocol::SPACE_LPI;
  }
  else if ( metaString == "left-posterior-superior" )
  {
    protocol->GetImageProtocol().space = Protocol::SPACE_LPS;
  }
  else if ( metaString == "right-anterior-inferior" )
  {
    protocol->GetImageProtocol().space = Protocol::SPACE_RAI;
  }
  else if ( metaString == "right-anterior-superior" )
  {
    protocol->GetImageProtocol().space = Protocol::SPACE_RAS;
  }
  else if ( metaString == "right-posterior-inferior" )
  {
    protocol->GetImageProtocol().space = Protocol::SPACE_RPI;
  }
  else if ( metaString == "right-posterior-superior" )
  {
    protocol->GetImageProtocol().space = Protocol::SPACE_RPS;
  }
  else
  {
    protocol->GetImageProtocol().space = Protocol::SPACE_UNKNOWN;
  }

  protocol->GetImageProtocol().bCrop = true;
  protocol->GetImageProtocol().croppedDWIFileNameSuffix = "";

  protocol->GetImageProtocol().reportFileNameSuffix = "_QCReport.txt";
  protocol->GetImageProtocol().reportFileMode = 1; //append

  protocol->GetImageProtocol().bQuitOnCheckSpacingFailure = true;
  protocol->GetImageProtocol().bQuitOnCheckSizeFailure = false;

  // ***** diffusion
  GetGradientDirections();

  protocol->GetDiffusionProtocol().bCheck = true;
  protocol->GetDiffusionProtocol().bValue = this->m_b0;

  for ( unsigned int i = 0; i < m_GradientDirectionContainer->size(); i++ )
  {
    vnl_vector_fixed<double, 3> vect;
    vect[0] = ( m_GradientDirectionContainer->ElementAt(i)[0] );
    vect[1] = ( m_GradientDirectionContainer->ElementAt(i)[1] );
    vect[2] = ( m_GradientDirectionContainer->ElementAt(i)[2] );

    protocol->GetDiffusionProtocol().gradients.push_back(vect);
  }

  // imaging frame
  vnl_matrix_fixed<double, 3, 3> imgf;
  imgf = m_DwiOriginalImage->GetDirection().GetVnlMatrix();

  protocol->GetImageProtocol().spacedirection[0][0] = imgf(0, 0);
  protocol->GetImageProtocol().spacedirection[0][1] = imgf(0, 1);
  protocol->GetImageProtocol().spacedirection[0][2] = imgf(0, 2);
  protocol->GetImageProtocol().spacedirection[1][0] = imgf(1, 0);
  protocol->GetImageProtocol().spacedirection[1][1] = imgf(1, 1);
  protocol->GetImageProtocol().spacedirection[1][2] = imgf(1, 2);
  protocol->GetImageProtocol().spacedirection[2][0] = imgf(2, 0);
  protocol->GetImageProtocol().spacedirection[2][1] = imgf(2, 1);
  protocol->GetImageProtocol().spacedirection[2][2] = imgf(2, 2);

  // measurement frame 
  std::vector<std::vector<double> > nrrdmf;
  itk::ExposeMetaData<std::vector<std::vector<double> > >(
    imgMetaDictionary,
    "NRRD_measurement frame",
    nrrdmf);

  vnl_matrix_fixed<double, 3, 3> mf;
  for ( unsigned int i = 0; i < 3; ++i )
  {
    for ( unsigned int j = 0; j < 3; ++j )
    {
      mf(i, j) = nrrdmf[i][j];
    }
  }

  protocol->GetDiffusionProtocol().measurementFrame[0][0] = mf(0, 0);
  protocol->GetDiffusionProtocol().measurementFrame[0][1] = mf(0, 1);
  protocol->GetDiffusionProtocol().measurementFrame[0][2] = mf(0, 2);
  protocol->GetDiffusionProtocol().measurementFrame[1][0] = mf(1, 0);
  protocol->GetDiffusionProtocol().measurementFrame[1][1] = mf(1, 1);
  protocol->GetDiffusionProtocol().measurementFrame[1][2] = mf(1, 2);
  protocol->GetDiffusionProtocol().measurementFrame[2][0] = mf(2, 0);
  protocol->GetDiffusionProtocol().measurementFrame[2][1] = mf(2, 1);
  protocol->GetDiffusionProtocol().measurementFrame[2][2] = mf(2, 2);

  protocol->GetDiffusionProtocol().bUseDiffusionProtocol = true;
  protocol->GetDiffusionProtocol().diffusionReplacedDWIFileNameSuffix = "";

  protocol->GetDiffusionProtocol().reportFileNameSuffix = "_QCReport.txt";
  protocol->GetDiffusionProtocol().reportFileMode = 1;
  protocol->GetDiffusionProtocol().bQuitOnCheckFailure = true;

  // ***** slice check
  protocol->GetSliceCheckProtocol().bCheck = true;
  protocol->GetSliceCheckProtocol().bSubregionalCheck = false;
  protocol->GetSliceCheckProtocol().subregionalCheckRelaxationFactor = 1.1;
  protocol->GetSliceCheckProtocol().checkTimes = 0;
  protocol->GetSliceCheckProtocol().headSkipSlicePercentage = 0.1;
  protocol->GetSliceCheckProtocol().tailSkipSlicePercentage = 0.1;
  protocol->GetSliceCheckProtocol().
    correlationDeviationThresholdbaseline = 3.00;
  protocol->GetSliceCheckProtocol().
    correlationDeviationThresholdgradient = 3.50; 
  protocol->GetSliceCheckProtocol().outputDWIFileNameSuffix = "";
  protocol->GetSliceCheckProtocol().reportFileNameSuffix
    = "_QCReport.txt";
  protocol->GetSliceCheckProtocol().reportFileMode = 1;
  protocol->GetSliceCheckProtocol().excludedDWINrrdFileNameSuffix = "";
  protocol->GetSliceCheckProtocol().bQuitOnCheckFailure = true;

  // ***** interlace check
  protocol->GetInterlaceCheckProtocol().bCheck = true;
  protocol->GetInterlaceCheckProtocol().correlationThresholdBaseline
    = .85;
  protocol->GetInterlaceCheckProtocol().correlationDeviationBaseline
    = 2.50;
  protocol->GetInterlaceCheckProtocol().correlationThresholdGradient
    = .85;
  protocol->GetInterlaceCheckProtocol().correlationDeviationGradient
    = 3.00;
  protocol->GetInterlaceCheckProtocol().rotationThreshold = 0.5; 
  protocol->GetInterlaceCheckProtocol().translationThreshold
    = ( protocol->GetImageProtocol().spacing[0]
  + protocol->GetImageProtocol().spacing[1]
  + protocol->GetImageProtocol().spacing[2]   ) * 0.3333333333333;
  protocol->GetInterlaceCheckProtocol().outputDWIFileNameSuffix = "";
  protocol->GetInterlaceCheckProtocol().reportFileNameSuffix
    = "_QCReport.txt";
  protocol->GetInterlaceCheckProtocol().reportFileMode = 1;
  protocol->GetInterlaceCheckProtocol().excludedDWINrrdFileNameSuffix = "";
  protocol->GetInterlaceCheckProtocol().bQuitOnCheckFailure = true;

  // ***** gradient check
  protocol->GetGradientCheckProtocol().bCheck = true;
  protocol->GetGradientCheckProtocol().rotationThreshold = 0.5; // degree
  protocol->GetGradientCheckProtocol().translationThreshold
    = (  protocol->GetImageProtocol().spacing[0]
  + protocol->GetImageProtocol().spacing[1]
  + protocol->GetImageProtocol().spacing[2] ) * 0.3333333333333;
  protocol->GetGradientCheckProtocol().outputDWIFileNameSuffix = "";
  protocol->GetGradientCheckProtocol().reportFileNameSuffix
    = "_QCReport.txt";
  protocol->GetGradientCheckProtocol().reportFileMode = 1;
  protocol->GetGradientCheckProtocol().excludedDWINrrdFileNameSuffix = "";
  protocol->GetGradientCheckProtocol().bQuitOnCheckFailure = true;

  // ***** baseline average
  protocol->GetBaselineAverageProtocol().bAverage = true;
  protocol->GetBaselineAverageProtocol().averageMethod = 1;
  protocol->GetBaselineAverageProtocol().stopThreshold = 0.02;
  protocol->GetBaselineAverageProtocol().outputDWIFileNameSuffix = "";
  protocol->GetBaselineAverageProtocol().reportFileNameSuffix
    = "_QCReport.txt";

  // ***** Eddy motion correction
  protocol->GetEddyMotionCorrectionProtocol().bCorrect = true;
  protocol->GetEddyMotionCorrectionProtocol().numberOfBins =  24;
  protocol->GetEddyMotionCorrectionProtocol().numberOfSamples
    =  100000;
  protocol->GetEddyMotionCorrectionProtocol().translationScale
    =  0.001;
  protocol->GetEddyMotionCorrectionProtocol().stepLength    =  0.1;
  protocol->GetEddyMotionCorrectionProtocol().relaxFactor    =  0.5;
  protocol->GetEddyMotionCorrectionProtocol().maxNumberOfIterations
    =  500;
  protocol->GetEddyMotionCorrectionProtocol().outputDWIFileNameSuffix
    = "";
  protocol->GetEddyMotionCorrectionProtocol().reportFileNameSuffix
    = "_QCReport.txt";
  protocol->GetEddyMotionCorrectionProtocol().reportFileMode = 1;

  // ***** DTI
  protocol->GetDTIProtocol().bCompute = true;
  protocol->GetDTIProtocol().dtiestimCommand
    = "/tools/bin_linux64/dtiestim";
  protocol->GetDTIProtocol().dtiprocessCommand
    = "/tools/bin_linux64/dtiprocess";
  protocol->GetDTIProtocol().method = Protocol::METHOD_WLS;
  protocol->GetDTIProtocol().baselineThreshold = 50; 
  protocol->GetDTIProtocol().mask = "";
  protocol->GetDTIProtocol().tensorSuffix = "_DTI.nhdr";
  protocol->GetDTIProtocol().bbaseline = true;
  protocol->GetDTIProtocol().baselineSuffix = "_Baseline.nhdr";
  protocol->GetDTIProtocol().bidwi = true;
  protocol->GetDTIProtocol().idwiSuffix = "_IDWI.nhdr";
  protocol->GetDTIProtocol().bfa = true;
  protocol->GetDTIProtocol().faSuffix = "_FA.nhdr";
  protocol->GetDTIProtocol().bmd = true;
  protocol->GetDTIProtocol().mdSuffix = "_MD.nhdr";
  protocol->GetDTIProtocol().bcoloredfa = true;
  protocol->GetDTIProtocol().coloredfaSuffix = "_colorFA.nhdr";
  protocol->GetDTIProtocol().bfrobeniusnorm = true;
  protocol->GetDTIProtocol().frobeniusnormSuffix
    = "_frobeniusnorm.nhdr";

  protocol->GetDTIProtocol().reportFileNameSuffix = "_QCReport.txt";
  protocol->GetDTIProtocol().reportFileMode = 1;

  return true;
}




