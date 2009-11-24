#include "ThreadIntensityMotionCheck.h"
#include "IntensityMotionCheck.h"

#include <string>
#include <math.h>
#include <QtGui>

CThreadIntensityMotionCheck::CThreadIntensityMotionCheck(QObject *parentLocal) :
  QThread(parentLocal)
  {
  DWINrrdFilename = "";
  protocol = NULL;
  qcResult = NULL;
  }

CThreadIntensityMotionCheck::~CThreadIntensityMotionCheck()
     {}

void CThreadIntensityMotionCheck::run()
{
  if ( DWINrrdFilename.length() == 0 )
    {
    std::cout << "DWI file name not set!" << std::endl;
    return;
    }

  if ( !protocol )
    {
    std::cout << "protocol not set!" << std::endl;
    return;
    }

  if ( !qcResult )
    {
    std::cout << "qcResult not set!" << std::endl;
    return;
    }
  emit allDone("checking ...");

  std::cout << "Checking Thread begins here " << std::endl;
  qcResult->Clear();
  CIntensityMotionCheck IntensityMotionCheck(DWINrrdFilename);
  IntensityMotionCheck.SetProtocol( protocol);
  IntensityMotionCheck.SetQCResult( qcResult);
  IntensityMotionCheck.GetImagesInformation();
  unsigned char result = IntensityMotionCheck.CheckByProtocol();

  unsigned char out = result;
  std::cout << "--------------------------------" << std::endl;

  out = result;
  out = out >> 7;
  if ( out )
    {
    std::cout << "QC FAILURE: too many gradients excluded!" << std::endl;
    }

  out = result;
  out = out << 1;
  out = out >> 7;
  if ( out )
    {
    std::cout << "QC FAILURE: Single b-value DWI without a b0/baseline!"
              << std::endl;
    }

  out = result;
  out = out << 2;
  out = out >> 7;
  if ( out )
    {
    std::cout << "QC FAILURE: Gradient direction # is less than 6!"
              << std::endl;
    }

  out = result;
  out = out << 7;
  out = out >> 7;
  if ( out  )
    {
    std::cout << "Image information check:\tFAILURE" << std::endl;
    }
  else
    {
    std::cout << "Image information check:\tPASS" << std::endl;
    }

  out = result;
  out = out << 6;
  out = out >> 7;
  if ( out )
    {
    std::cout << "Diffusion information check:\tFAILURE" << std::endl;
    }
  else
    {
    std::cout << "Diffusion information check:\tPASS" << std::endl;
    }

  out = result;
  out = out << 5;
  out = out >> 7;
  if ( out  )
    {
    std::cout << "Slice-wise check:\t\tFAILURE" << std::endl;
    }
  else
    {
    std::cout << "Slice-wise check:\t\tPASS" << std::endl;
    }

  out = result;
  out = out << 4;
  out = out >> 7;
  if ( out  )
    {
    std::cout << "Interlace-wise check:\t\tFAILURE" <<  std::endl;
    }
  else
    {
    std::cout << "Interlace-wise check:\t\tPASS" << std::endl;
    }

  out = result;
  out = out << 3;
  out = out >> 7;
  if ( out )
    {
    std::cout << "Gradient-wise check:\t\tFAILURE" << std::endl;
    }
  else
    {
    std::cout << "Gradient-wise check:\t\tPASS" << std::endl;
    }

  // emit kkk(100);
  // emit QQQ(10);

  emit allDone("Checking Thread ended");
  emit ResultUpdate();
}

/*
void CThreadIntensityMotionCheck::run()
{
  if(DWINrrdFilename.length()==0)
  {
    std::cout<<"DWI file name not set!"<<std::endl;
    return;
  }

  if( !protocol )
  {
    std::cout<<"protocol not set!"<<std::endl;
    return;
  }

  if( !qcResult )
  {
    std::cout<<"qcResult not set!"<<std::endl;
    return;
  }
  emit allDone("checking ...");

  emit kkk(0);

  std::cout<<"Checking Thread begins here "<<std::endl;
  qcResult->Clear();
  CIntensityMotionCheck IntensityMotionCheck(DWINrrdFilename);
  IntensityMotionCheck.SetProtocol( protocol);
  IntensityMotionCheck.SetQCResult( qcResult);
  IntensityMotionCheck.GetImagesInformation();
  //unsigned char  result = IntensityMotionCheck.CheckByProtocol();

  ///

  if( !protocol )
  {
    std::cout<<"Protocol NOT set."<<std::endl;
    return;
  }
  if( IntensityMotionCheck.GetDwiImage()->GetVectorLength() != IntensityMotionCheck.GetGradientDirectionContainer()->size() )
  {
    std::cout<< "Bad DWI: mismatch between gradient image # and gradient vector #" << std::endl;
    return;
  }

  if(!IntensityMotionCheck.GetDwiLoadStatus() ) IntensityMotionCheck.LoadDwiImage();

  bool bReport = false;
  std::ofstream outfile;

  std::string ReportFileName;
  std::string DwiFileName;
  DwiFileName = IntensityMotionCheck.GetDwiFileName();

  ReportFileName=DwiFileName.substr(0,DwiFileName.find_last_of('.') );

  if( protocol->GetReportFileNameSuffix().length() > 0)
    ReportFileName.append( protocol->GetReportFileNameSuffix() );
  else
    ReportFileName.append( "_QC_CheckReports.txt");

  outfile.open( ReportFileName.c_str() );

  if(outfile)
    bReport = true;

  if(bReport)
  {
    outfile<<"================================= "<< std::endl;
    outfile<<"* DWI QC Report ( DTIPrep 1.0 ) * "<< std::endl;
    outfile<<"================================= "<< std::endl;
    outfile<<"DWI File: "<< DwiFileName << std::endl;
    time_t rawtime; time( &rawtime );
    outfile<<"Check Time: "<< ctime(&rawtime) << std::endl;

    outfile.close();
  }

  emit kkk(2);

  unsigned char result=0;
  // ZYXEDCBA:
  // X  QC; Too many bad gradient directions found!
  // Y  QC; Single b-value DWI without a b0/baseline!
  // Z  QC: Gradient direction # is less than 6!
  // A: ImageCheck()
  // B: DiffusionCheck()
  // C: SliceCheck()
  // D: InterlaceCheck()
  // E:GradientCheck()

  //protocol->printProtocols();

  typedef unsigned short DwiPixelType;
  typedef itk::VectorImage<DwiPixelType, 3>  DwiImageType;
  typedef itk::ImageFileWriter<DwiImageType>  DwiWriterType;
  DwiWriterType::Pointer DwiWriter = DwiWriterType::New();
  itk::NrrdImageIO::Pointer  NrrdImageIO = itk::NrrdImageIO::New();

  DwiWriter  = DwiWriterType::New();
  NrrdImageIO  = itk::NrrdImageIO::New();
  DwiWriter->SetImageIO(NrrdImageIO);

  // image information check
  std::cout<<"====================="<<std::endl;
  std::cout<<"ImageCheck ... "<<std::endl;
  if(!IntensityMotionCheck.ImageCheck( ))
    result = result | 1;
  std::cout<<"ImageCheck DONE "<<std::endl;
  emit kkk(4);

  // diffusion information check
  std::cout<<"====================="<<std::endl;
  std::cout<<"DiffusionCheck ... "<<std::endl;
  if(!IntensityMotionCheck.DiffusionCheck( ))
    result = result | 2;
   std::cout<<"DiffusionCheck DONE "<<std::endl;
  emit kkk(6);

  // SliceChecker
  std::cout<<"====================="<<std::endl;
  std::cout<<"SliceWiseCheck ... "<<std::endl;
  if(!IntensityMotionCheck.SliceWiseCheck( ))
    result = result | 4;
  std::cout<<"SliceWiseCheck DONE "<<std::endl;
  emit kkk(10);

  // InterlaceChecker
  std::cout<<"====================="<<std::endl;
  std::cout<<"InterlaceWiseCheck ... "<<std::endl;
  if(!IntensityMotionCheck.InterlaceWiseCheck( ))
    result = result | 8;
  std::cout<<"InterlaceWiseCheck DONE "<<std::endl;
  emit kkk(25);

  // baseline average
  std::cout<<"====================="<<std::endl;
  std::cout<<"BaselineAverage ... "<<std::endl;
  IntensityMotionCheck.BaselineAverage( );
  std::cout<<"BaselineAverage DONE "<<std::endl;
  emit kkk(35);

  // EddyMotionCorrect
  std::cout<<"====================="<<std::endl;
  std::cout<<"EddyCurrentHeadMotionCorrect ... "<<std::endl;
  IntensityMotionCheck.EddyMotionCorrect();
  std::cout<<"EddyCurrentHeadMotionCorrect DONE "<<std::endl;
  emit kkk(60);

  // GradientChecker
  std::cout<<"====================="<<std::endl;
  std::cout<<"GradientCheck ... "<<std::endl;
  if(!IntensityMotionCheck.GradientWiseCheck( ))
    result = result | 16;
  std::cout<<"GradientCheck DONE "<<std::endl;
  emit kkk(85);

  // Save QC'ed DWI
  std::cout<<"====================="<<std::endl;
   std::cout<<"Save QC'ed DWI ... ";
  IntensityMotionCheck.SaveQCedDWI();
  std::cout<<" DONE "<<std::endl;

  emit kkk(90);


  // DTIComputing
  std::cout<<"====================="<<std::endl;
  std::cout<<"DTIComputing ... "<<std::endl;
  IntensityMotionCheck.DTIComputing();
  std::cout<<"DTIComputing DONE"<<std::endl;
  emit kkk(99);

  // 00000CBA:
  // A: Gradient direction # is less than 6!
  // B: Single b-value DWI without a b0/baseline!
  // C: Too many bad gradient directions found!
  // 0: valid

  unsigned char ValidateResult;
  IntensityMotionCheck.collectLeftDiffusionStatistics( 0 );
  ValidateResult = IntensityMotionCheck.validateLeftDiffusionStatistics( );

  result = ( ValidateResult << 5 ) + result;

  ///
  unsigned char out= result;
  std::cout << "--------------------------------" << std::endl;

  out = result;
  out = out>>7;
  if( out)
    std::cout << "QC FAILURE: Gradient direction # is less than 6!" << std::endl;

  out = result;
  out = out<<1;
  out = out>>7;
  if( out)
    std::cout << "QC FAILURE: Single b-value DWI without a b0/baseline!" << std::endl;

  out = result;
  out = out<<2;
  out = out>>7;
  if( out)
    std::cout << "QC FAILURE: too many gradients excluded!" << std::endl;

  out = result;
  out = out<<7;
  out = out>>7;
  if( out  )
    std::cout << "Image information check:\tFAILURE" << std::endl;
  else
    std::cout << "Image information check:\tPASS" << std::endl;

  out = result;
  out = out<<6;
  out = out>>7;
  if( out )
    std::cout << "Diffusion information check:\tFAILURE" << std::endl;
  else
    std::cout << "Diffusion information check:\tPASS" << std::endl;

  out = result;
  out = out<<5;
  out = out>>7;
  if( out  )
    std::cout << "Slice-wise check:\t\tFAILURE" << std::endl;
  else
    std::cout << "Slice-wise check:\t\tPASS" << std::endl;

  out = result;
  out = out<<4;
  out = out>>7;
  if( out  )
    std::cout << "Interlace-wise check:\t\tFAILURE"<<  std::endl;
  else
    std::cout << "Interlace-wise check:\t\tPASS" << std::endl;

  out = result;
  out = out<<3;
  out = out>>7;
  if( out )
    std::cout << "Gradient-wise check:\t\tFAILURE" << std::endl;
  else
    std::cout << "Gradient-wise check:\t\tPASS" << std::endl;

  emit kkk(100);
  //emit QQQ(10);

  emit allDone("Checking Thread ended");
  emit ResultUpdate();
}

*/
