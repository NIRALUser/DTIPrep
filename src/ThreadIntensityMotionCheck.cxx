#include "ThreadIntensityMotionCheck.h"

#include <string>
#include <math.h>
#include <QtGui>

CThreadIntensityMotionCheck::CThreadIntensityMotionCheck(QObject *parentLocal) :
  QThread(parentLocal)
{
  DWINrrdFilename = "";
  XmlFilename = "";
  protocol = NULL;
  qcResult = NULL;
  m_IntensityMotionCheck = new CIntensityMotionCheck;
  m_Recompute = false ;
  m_RecomputeOutputFileName = "" ;
  m_hasComputedOnce = false ;
}

CThreadIntensityMotionCheck::~CThreadIntensityMotionCheck()
{
}

void CThreadIntensityMotionCheck::SetRecompute( bool val )
{
  m_Recompute = val ;
}

bool CThreadIntensityMotionCheck::HasComputed()
{
  return m_hasComputedOnce ;
}

void CThreadIntensityMotionCheck::SetRecomputeOutputFileName( std::string filename )
{
    m_RecomputeOutputFileName = filename ;
}

void CThreadIntensityMotionCheck::run()
{
  if( m_Recompute && m_hasComputedOnce )
  {
    emit f_StartProgressSignal() ;
    if( m_RecomputeOutputFileName.empty() )
    {
      std::string dwiFileName = m_IntensityMotionCheck->GetDwiFileName() ;
      m_RecomputeOutputFileName =  dwiFileName.substr( 0, dwiFileName.find_last_of('.') ) ;
      m_RecomputeOutputFileName.append("_VC.nrrd") ;
    }
    m_IntensityMotionCheck->BrainMask(m_RecomputeOutputFileName , m_RecomputeOutputFileName , true );
    m_IntensityMotionCheck->DTIComputing( m_RecomputeOutputFileName , m_IntensityMotionCheck->GetLastComputedMask() , true );
    emit f_StopProgressSignal();
    emit allDone("Recomputing mask and DTI scalar measurements from VC done");
    emit SignalRecomputationDone();
    return ;
  }
  if( DWINrrdFilename.length() == 0 )
    {
    std::cout << "DWI file name not set!" << std::endl;
    return;
    }

  if( !protocol )
    {
    std::cout << "protocol not set!" << std::endl;
    return;
    }

  if( !qcResult )
    {
    std::cout << "qcResult not set!" << std::endl;
    return;
    }
  emit allDone("checking ...");

  qcResult->Clear();

  m_IntensityMotionCheck->SetDwiFileName(DWINrrdFilename);
  m_IntensityMotionCheck->SetXmlFileName(XmlFilename);
  m_IntensityMotionCheck->SetProtocol( protocol);
  m_IntensityMotionCheck->SetQCResult( qcResult);
  m_IntensityMotionCheck->GetImagesInformation();
  // std::cout << "TEST2:" << qcResult->GetIntensityMotionCheckResult().size() << std::endl;
  emit                StartProgressSignal(); // start showing progress bar
  const unsigned char result = m_IntensityMotionCheck->RunPipelineByProtocol();

  // std::cout <<"qcResult_TESTTHREAD " << qcResult.GetIntensityMotionCheckResult()[0].processing << std::endl;
  // std::cout <<"qcResult_TESTTHREAD " << qcResult.GetIntensityMotionCheckResult()[46].processing << std::endl;
  // Set_result( m_IntensityMotionCheck->RunPipelineByProtocol() );

  unsigned char out = result;
  std::cout << "--------------------------------" << std::endl;

  out = result;
  // out = out >> 7;
  out = out >> 9; // because of adding two processes in the QC pipeline ( brainmask and dominant directional artifact
  if( out )
    {
    std::cout << "QC FAILURE: too many gradients excluded!" << std::endl;
    }

  out = result;
  // out = out << 1;
  // out = out >> 7;
  out = out << 1;
  out = out >> 9; // because of adding two processes in the QC pipeline ( brainmask and dominant directional artifact
  if( out )
    {
    std::cout << "QC FAILURE: Single b-value DWI without a b0/baseline!"
              << std::endl;
    }

  out = result;
  // out = out << 2;
  // out = out >> 7;
  out = out << 2;
  out = out >> 9; // because of adding two processes in the QC pipeline ( brainmask and dominant directional artifact
  if( out )
    {
    std::cout << "QC FAILURE: Gradient direction #is less than 6!"
              << std::endl;
    }

  out = result;
  // out = out << 7;
  // out = out >> 7;
  out = out << 9;
  out = out >> 9; // because of adding two processes in the QC pipeline ( brainmask and dominant directional artifact
  if( out  )
    {
    std::cout << "Image information check:\tFAILURE" << std::endl;
    }
  else
    {
    std::cout << "Image information check:\tPASS or Not Set" << std::endl;
    }

  out = result;
  // out = out << 6;
  // out = out >> 7;
  out = out << 8;
  out = out >> 9; // because of adding two processes in the QC pipeline ( brainmask and dominant directional artifact
  if( out )
    {
    std::cout << "Diffusion information check:\tFAILURE" << std::endl;
    }
  else
    {
    std::cout << "Diffusion information check:\tPASS or Not Set" << std::endl;
    }

  out = result;
  // out = out << 5;
  // out = out >> 7;
  out = out << 7;
  out = out >> 9; // because of adding two processes in the QC pipeline ( brainmask and dominant directional artifact
  if( out  )
    {
    std::cout << "Slice-wise check:\t\tFAILURE" << std::endl;
    }
  else
    {
    std::cout << "Slice-wise check:\t\tPASS or Not Set" << std::endl;
    }

  out = result;
  // out = out << 4;
  // out = out >> 7;
  out = out << 6;
  out = out >> 9; // because of adding two processes in the QC pipeline ( brainmask and dominant directional artifact
  if( out  )
    {
    std::cout << "Interlace-wise check:\t\tFAILURE" <<  std::endl;
    }
  else
    {
    std::cout << "Interlace-wise check:\t\tPASS or Not Set" << std::endl;
    }

  out = result;
  // out = out << 3;
  // out = out >> 7;
  out = out << 5;
  out = out >> 9; // because of adding two processes in the QC pipeline ( brainmask and dominant directional artifact
  if( out )
    {
    std::cout << "Gradient-wise check:\t\tFAILURE" << std::endl;
    }
  else
    {
    std::cout << "Gradient-wise check:\t\tPASS or Not Set" << std::endl;
    }

  out = result;
  out = out << 4;
  out = out >> 9; // because of adding two processes in the QC pipeline ( brainmask and dominant directional artifact
  if( out )
    {
    std::cout << "Brain mask check:\t\tFAILURE" << std::endl;
    }
  else
    {
    std::cout << "Brain mask check:\t\tPASS or Not Set" << std::endl;
    }

  out = result;
  out = out << 3;
  out = out >> 9; // because of adding two processes in the QC pipeline ( brainmask and dominant directional artifact
  if( out )
    {
    std::cout << "Dominant Directional Detector:\tFAILURE" << std::endl;
    }
  else
    {
    std::cout << "Dominant Directional Detector:\tPASS or Not Set" << std::endl;
    }

  emit StopProgressSignal();  // hiding progress bar

  emit allDone("Checking Thread ended");
  emit Set_QCedDWI();                            // set QCed DWI in DTIPnale
  emit Set_Original_ForcedConformance_Mapping(); // set mapping in DTIPanel
  emit ResultUpdate();
  // emit Building_Mapping_XML();
  emit LoadQCedDWI( QString::fromStdString( m_IntensityMotionCheck->GetOutputDWIFileName() ) );
  emit QCedResultUpdate();
  emit Set_VCStatus();  // initialize the VC_Status in DTIPanel
  m_hasComputedOnce = true ;
}
