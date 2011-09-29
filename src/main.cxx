#include <iostream>
#include <fstream>
#include <string>

#include <QApplication>
#include <QPushButton>
#include <QString>
#include <QFileInfo>
#include <QTreeWidget>

#include "GMainWindow.h"
// #include "main.h"

#include <stddef.h> /* size_t */
#include <string.h> /* strcmp */
#include <iostream>

#include "metaCommand.h"
#include "IntensityMotionCheck.h"

#include "Protocol.h"
#include "XmlStreamReader.h"
#include "XmlStreamWriter.h"

// Defining Checking bits
#define ImageCheckBit 1
#define DiffusionCheckBit 2
#define SliceWiseCheckBit 4
#define InterlaceWiseCheckBit 8
#define GradientWiseCheckBit 16

// #include <QStyleFactory>
using namespace std;

int r_SliceWiseCkeck; // the overall SliceWiseChecking result
int r_InterlaceWiseCheck; // the overall InterlaceWiseChecking result
int r_GradWiseCheck; // the overall GradientWiseChecking result

int main ( int argc, char **argv )
{
// BUG: When loading the DTIPrep GUI, the following error would appear: "Qt internal error: qt_menu.nib could not be loaded. The .nib file should be placed in QtGui.framework/Versions/Current/Resources/ or in the resources directory of your applicaiton bundle." Qt is aware of this problem (http://bugreports.qt.nokia.com/browse/QTBUG-5952) and has provided the following fix that was pasted in main.cxx and got rid of the error.

#if 0
QT_MANGLE_NAMESPACE(QCocoaMenuLoader) *qtMenuLoader = [[QT_MANGLE_NAMESPACE(QCocoaMenuLoader) alloc] init];
if ([NSBundle loadNibNamed:@"qt_menu" owner:qtMenuLoader] == false) { qFatal("Qt internal error: qt_menu.nib could not be loaded. The .nib file" " should be placed in QtGui.framework/Versions/Current/Resources/ " " or in the resources directory of your application bundle."); }

[cocoaApp setMenu:[qtMenuLoader menu]];
[newDelegate setMenuLoader:qtMenuLoader];
[qtMenuLoader release];
#endif
  if ( argc == 1 )
  {
    QApplication app (argc, argv);
    GMainWindow  *MainWindow = new GMainWindow;
    MainWindow->show();
    return app.exec();
  }
  else
 {
   
   MetaCommand command;
    
    // DWI filename
    command.SetOption(
      "DWIFileName",
      "w",
      true,
      "DWI file name to convert dicom image series into or to be checked (nhdr)");
    command.SetOptionLongTag(  "DWIFileName", "DWINrrdFile");
    command.AddOptionField(    "DWIFileName",
      "DWIFileName",
      MetaCommand::STRING,
      true);
    // protocol file name 
    command.SetOption(      "xmlSetting", "p", true, 
      "protocol xml file containing all the parameters");
    command.SetOptionLongTag(  "xmlSetting", "xmlProtocol");
    command.AddOptionField(    "xmlSetting",
      "xmlFileName",
      MetaCommand::STRING,
      true);
   // result QC XML
    command.SetOption(      "resultXML", "q", false, "QCresult XML file");
    command.SetOptionLongTag(  "resultXML", "resultXMLFile");
    command.AddOptionField(    "resultXML",
      "xmlQCFile",
      MetaCommand::STRING,
      true);
    // to create a default protocol 
    command.SetOption(      "createDefaultProtocol", "d", false, "create default protocol xml file");
    command.SetOptionLongTag(  "createDefaultProtocol", "default");
    // to check by the protocol 
    command.SetOption(      "checkByProtocol", "c", false, "check by protocol xml file. Default operatoin.");
    command.SetOptionLongTag(  "checkByProtocol", "check");

    // result notes
    command.SetOption(      "resultNotes", "n", false, "result notes");
    command.SetOptionLongTag(  "resultNotes", "resultNotesFile");
    command.AddOptionField(    "resultNotes",
      "NotesFile",
      MetaCommand::STRING,
      true);

    
    // result folder
    command.SetOption(      "outputFolder", "f", false, "output folder name");
    command.SetOptionLongTag(  "outputFolder", "outputFolder");
    command.AddOptionField(    "outputFolder",
      "OutputFolder",
      MetaCommand::STRING,
      true);

    if ( !command.Parse(argc, argv) )
    {
      return EXIT_FAILURE;
    }

    string DWIFileName  = command.GetValueAsString("DWIFileName", "DWIFileName");
    string xmlFileName  = command.GetValueAsString("xmlSetting", "xmlFileName");
    string resultNotes  = command.GetValueAsString("resultNotes", "NotesFile");
    string resultFolder = command.GetValueAsString("outputFolder","OutputFolder");
    string resultXMLFile = command.GetValueAsString("resultXML","xmlQCFile");
    
        
    bool bCreateDefaultProtocol = command.GetOptionWasSet("createDefaultProtocol");
    bool bcheckByProtocol       = command.GetOptionWasSet("checkByProtocol");

    // check with  xml
    if ( command.GetOptionWasSet("xmlSetting") && xmlFileName.length() > 0 )
    {

      Protocol protocol;
      QCResult qcResult;
      qcResult.Clear();
      protocol.clear();
      CIntensityMotionCheck IntensityMotionCheck;
      IntensityMotionCheck.SetDwiFileName(DWIFileName);
      QString  str( xmlFileName.c_str() );
      QFileInfo xmlFile( str);//QString::fromStdString(xmlFileName) );
      XmlStreamReader XmlReader(NULL);
      XmlReader.setProtocol( &protocol);
      if( xmlFile.exists() )
                  XmlReader.readFile(str, XmlStreamReader::ProtocolWise);
      protocol.GetQCOutputDirectory() = resultFolder;
      protocol.printProtocols();
      IntensityMotionCheck.SetProtocol( &protocol);
      IntensityMotionCheck.SetQCResult( &qcResult);
      IntensityMotionCheck.GetImagesInformation();

      if( bCreateDefaultProtocol || !xmlFile.exists() )
      {              
        IntensityMotionCheck.MakeDefaultProtocol( &protocol );
        protocol.Save( xmlFileName.c_str() );
      }
      
      if( bcheckByProtocol || ( !bCreateDefaultProtocol && ! bcheckByProtocol ) )
      {
        if( !bCreateDefaultProtocol && ! bcheckByProtocol )
        {
          std::cout<< "Create default protocol or check by protocol, at least one of them should be set." << std::endl;
          std::cout<< "Neither of them was set. Check by default." << std::endl;
        }

        const unsigned char result = IntensityMotionCheck.RunPipelineByProtocol();
        unsigned char out = result;
        std::cout << "qcResult.GetIntensityMotionCheckResult()[0].processing" << qcResult.GetIntensityMotionCheckResult()[0].processing << std::endl;
	std::cout << "qcResult.GetIntensityMotionCheckResult()[0].processing" << qcResult.GetIntensityMotionCheckResult()[46].processing << std::endl;
	
  
        //.........................................................................................
        // ...............Building XML QCResult........................................
        //.........................................................................................
	
        QString Result_xmlFile(resultXMLFile.c_str());
  	
        QFile file(Result_xmlFile);

	if ( !file.open(QFile::WriteOnly | QFile::Text) )
	{
	std::cerr << "Error: Cannot write file "
		<< qPrintable(Result_xmlFile) << ": "
		<< qPrintable( file.errorString() ) << std::endl;
	}
	
	QXmlStreamWriter xmlWriter(&file);
	xmlWriter.setAutoFormatting(true);
	xmlWriter.writeStartDocument();
	xmlWriter.writeStartElement("QCResultSettings");
	// ImageInformation	
        xmlWriter.writeStartElement("entry");
  	xmlWriter.writeAttribute( "parameter", "ImageInformation" );
	if ( protocol.GetImageProtocol().bCheck )
	{
	if ( ( protocol.GetImageProtocol().bQuitOnCheckSizeFailure && ( (qcResult.Get_result()  & ImageCheckBit) !=  0)) || (    protocol.GetImageProtocol().bQuitOnCheckSpacingFailure && ( (qcResult.Get_result()  & ImageCheckBit) !=  0)) ){
		xmlWriter.writeTextElement("value","Fail Pipeline Terminated");
		xmlWriter.writeStartElement("entry");
		xmlWriter.writeAttribute( "parameter", "file name"  );
		xmlWriter.writeTextElement("value",qcResult.GetImageInformationCheckResult().info);
		xmlWriter.writeEndElement();
	        xmlWriter.writeEndElement();
		xmlWriter.writeEndDocument();	 
   		return 0;
 	}
        xmlWriter.writeStartElement("entry");
        xmlWriter.writeAttribute( "parameter", "file name"  );
	xmlWriter.writeTextElement("value",qcResult.GetImageInformationCheckResult().info);
	xmlWriter.writeEndElement();
	xmlWriter.writeStartElement("entry");
  	xmlWriter.writeAttribute( "parameter", "origin"  );
        if ( qcResult.GetImageInformationCheckResult().origin )
	{
	  xmlWriter.writeTextElement("value","Pass");
	}
	else
	{
	  xmlWriter.writeTextElement("value","Fail");
	}
        xmlWriter.writeEndElement();
	xmlWriter.writeStartElement("entry");
  	xmlWriter.writeAttribute( "parameter", "size"  );
	if ( qcResult.GetImageInformationCheckResult().size )
	{
	  xmlWriter.writeTextElement("value","Pass");
	}
	else
	{
	  xmlWriter.writeTextElement("value","Fail");
	}
        xmlWriter.writeEndElement();
	xmlWriter.writeStartElement("entry");
  	xmlWriter.writeAttribute( "parameter", "space"  );
	if ( qcResult.GetImageInformationCheckResult().space )
	{
	  xmlWriter.writeTextElement("value","Pass");
	}
	else
	{
	  xmlWriter.writeTextElement("value","Fail");
	}
	xmlWriter.writeEndElement();
	xmlWriter.writeStartElement("entry");
  	xmlWriter.writeAttribute( "parameter", "spacedirection"  );
	if ( qcResult.GetImageInformationCheckResult().spacedirection )
	{
	  xmlWriter.writeTextElement("value","Pass");
	}
	else
	{
	  xmlWriter.writeTextElement("value","Fail");
	}
	xmlWriter.writeEndElement();
	xmlWriter.writeStartElement("entry");
  	xmlWriter.writeAttribute( "parameter", "spacing"  );
	if ( qcResult.GetImageInformationCheckResult().spacing )
	{
      	  xmlWriter.writeTextElement("value","Pass");
  	}
	else
	{
	   xmlWriter.writeTextElement("value","Fail");
	}
        xmlWriter.writeEndElement();
        xmlWriter.writeEndElement();
	}
	else if ( !protocol.GetImageProtocol().bCheck )
	{
	  xmlWriter.writeTextElement("value","Info Not Check");
	  xmlWriter.writeStartElement("entry");
	  xmlWriter.writeAttribute( "parameter", "file name"  );
	  xmlWriter.writeTextElement("value",qcResult.GetImageInformationCheckResult().info);
	  xmlWriter.writeEndElement();
          xmlWriter.writeEndElement();
	} 
	
	// DiffusionInformationCheckResult
        xmlWriter.writeStartElement("entry");
  	xmlWriter.writeAttribute( "parameter", "DiffusionInformation"  );
	if ( protocol.GetDiffusionProtocol().bCheck )
	{
	xmlWriter.writeStartElement("entry");
  	xmlWriter.writeAttribute( "parameter", "b value"  );	
	if ( qcResult.GetDiffusionInformationCheckResult().b )
	{
           xmlWriter.writeTextElement("value","Pass");
   	}
	else
	{
	   xmlWriter.writeTextElement("value","Fail");
	}
	xmlWriter.writeEndElement();
        xmlWriter.writeStartElement("entry");
  	xmlWriter.writeAttribute( "parameter", "gradient"  );	
	if ( qcResult.GetDiffusionInformationCheckResult().gradient )
	{
	   xmlWriter.writeTextElement("value","Pass");
	}
	else
	{
	  xmlWriter.writeTextElement("value","Fail");
	}
        xmlWriter.writeEndElement();
        xmlWriter.writeStartElement("entry");
  	xmlWriter.writeAttribute( "parameter", "measurementFrame"  );	
	if ( qcResult.GetDiffusionInformationCheckResult().measurementFrame )
	{
	  xmlWriter.writeTextElement("value","Pass");
	}
	else
	{
	  xmlWriter.writeTextElement("value","Fail");
	}
	xmlWriter.writeEndElement();
	}
	else if ( !protocol.GetDiffusionProtocol().bCheck ){
		xmlWriter.writeTextElement("value","Info Not Check");
        	
	}
	xmlWriter.writeEndElement();
	// itemIntensityMotionInformation
        // Overall assesment 
        // computing the overall results of the SliceWise check, InterlaceWise check and GradientWise check
	int num_SliceWiseCheckExc = 0;
	int num_InterlaceWiseCheckExc = 0;
	int num_GradientWiseCheckExc = 0;
	for ( unsigned int i = 0; i< qcResult.GetIntensityMotionCheckResult().size();
	i++ )
	{
	
	if ( qcResult.GetIntensityMotionCheckResult()[i].processing == QCResult::GRADIENT_EXCLUDE_SLICECHECK )
	num_SliceWiseCheckExc++;
	if ( qcResult.GetIntensityMotionCheckResult()[i].processing == QCResult::GRADIENT_EXCLUDE_INTERLACECHECK )
	num_InterlaceWiseCheckExc++;
	if ( qcResult.GetIntensityMotionCheckResult()[i].processing == QCResult::GRADIENT_EXCLUDE_GRADIENTCHECK )
	num_GradientWiseCheckExc++;
	}
	r_SliceWiseCkeck = num_SliceWiseCheckExc/qcResult.GetIntensityMotionCheckResult().size();
	r_InterlaceWiseCheck = num_InterlaceWiseCheckExc/(qcResult.GetIntensityMotionCheckResult().size()-num_SliceWiseCheckExc);
	r_GradWiseCheck = num_GradientWiseCheckExc/(qcResult.GetIntensityMotionCheckResult().size()-num_SliceWiseCheckExc-num_InterlaceWiseCheckExc);

	xmlWriter.writeStartElement("entry");
  	xmlWriter.writeAttribute( "parameter", "DWI Check"  );	

        xmlWriter.writeStartElement("entry");
	xmlWriter.writeAttribute( "parameter", "SliceWiseCheck"  );

        if ( protocol.GetSliceCheckProtocol().bCheck )   // Check protocol whether run SliceWiseChecking
	{
		if ( (qcResult.Get_result()  & SliceWiseCheckBit) == SliceWiseCheckBit )
		{
		if ( protocol.GetSliceCheckProtocol().bQuitOnCheckFailure )
		{
			xmlWriter.writeTextElement("value","Fail Pipeline Termination");
		}
                else
		     if (r_SliceWiseCkeck > protocol.GetInterlaceCheckProtocol().correlationThresholdGradient)
		     {
			
			xmlWriter.writeTextElement("value","Fail");
		     }
		     else
		     {
			xmlWriter.writeTextElement("value","Pass");
		     }
		}
		else
		     if (r_SliceWiseCkeck > protocol.GetInterlaceCheckProtocol().correlationThresholdGradient)
		     {
			
			xmlWriter.writeTextElement("value","Fail");
			
		     }
		     else
		     {
			xmlWriter.writeTextElement("value","Pass");
		     }
	}
	else if ( !protocol.GetSliceCheckProtocol().bCheck )
	{
		xmlWriter.writeTextElement("value","Not Set");
	}

	xmlWriter.writeEndElement();

	xmlWriter.writeStartElement("entry");
	xmlWriter.writeAttribute( "parameter", "InterlaceWiseCheck"  );

	if ( !(( (qcResult.Get_result()  & SliceWiseCheckBit) == SliceWiseCheckBit ) && protocol.GetSliceCheckProtocol().bQuitOnCheckFailure) && protocol.GetInterlaceCheckProtocol().bCheck )  // Check protocol whether run InterlaceWiseChecking
	{
	if (  (qcResult.Get_result() & InterlaceWiseCheckBit) == InterlaceWiseCheckBit )
	{
	if ( protocol.GetInterlaceCheckProtocol().bQuitOnCheckFailure )
	{
		xmlWriter.writeTextElement("value","Fail Pipeline Termination");
	}
	else
	{
		if ( r_InterlaceWiseCheck > protocol.GetInterlaceCheckProtocol().correlationThresholdGradient)
		{
		xmlWriter.writeTextElement("value","Fail");
		}
		else 
		{
		xmlWriter.writeTextElement("value","Pass");
		}
	}
	}
	else 
	{
		if ( r_InterlaceWiseCheck > protocol.GetInterlaceCheckProtocol().correlationThresholdGradient)
		{
		xmlWriter.writeTextElement("value","Fail");
		}
		else 
		{
		xmlWriter.writeTextElement("value","Pass");
		}
	}
	}
	else if ( !protocol.GetInterlaceCheckProtocol().bCheck )
	{
		xmlWriter.writeTextElement("value","Not Set");
	}	

	xmlWriter.writeEndElement();

	xmlWriter.writeStartElement("entry");
	xmlWriter.writeAttribute( "parameter", "GradientWiseCheck"  );

	if ( !protocol.GetSliceCheckProtocol().bQuitOnCheckFailure &&  !protocol.GetInterlaceCheckProtocol().bQuitOnCheckFailure && protocol.GetGradientCheckProtocol().bCheck )   // Check protocol whether run GradientWiseChecking
	{
	if  ( (qcResult.Get_result() & GradientWiseCheckBit) == GradientWiseCheckBit )
	{
	if (protocol.GetGradientCheckProtocol().bQuitOnCheckFailure)
	{
		
		xmlWriter.writeTextElement("value","Fail Pipeline Terminatio");
	}
	else
	{
		if ( r_GradWiseCheck > protocol.GetInterlaceCheckProtocol().correlationThresholdGradient)
		{
		xmlWriter.writeTextElement("value","Fail");
		}
		else 
		{
		xmlWriter.writeTextElement("value","Pass");
		}		
	}
	}
	else
	{
		if ( r_GradWiseCheck > protocol.GetInterlaceCheckProtocol().correlationThresholdGradient)
		{
		xmlWriter.writeTextElement("value","Fail");
		}
		else 
		{
		xmlWriter.writeTextElement("value","Pass");
		}	
	}

	}
	else if ( ! protocol.GetGradientCheckProtocol().bCheck )
	{
		xmlWriter.writeTextElement("value","Not Set");
	}
	xmlWriter.writeEndElement();

	
/**** Old

	if ( protocol.GetSliceCheckProtocol().bCheck )   // Check protocol whether run SliceWiseChecking
	{
		if ( (qcResult.Get_result()  & SliceWiseCheckBit) == SliceWiseCheckBit )
		{
		if ( protocol.GetSliceCheckProtocol().bQuitOnCheckFailure )
		{
			xmlWriter.writeStartElement("entry");
			xmlWriter.writeAttribute( "parameter", "SliceWiseCheck"  );	
			xmlWriter.writeTextElement("value","Fail Pipeline Termination");
			xmlWriter.writeEndElement();
			xmlWriter.writeStartElement("entry");
			xmlWriter.writeAttribute( "parameter", "InterlaceWiseCheck"  );	
			xmlWriter.writeTextElement("value","NA");
			xmlWriter.writeEndElement();
			xmlWriter.writeStartElement("entry");
			xmlWriter.writeAttribute( "parameter", "GradientWiseCheck"  );	
			xmlWriter.writeTextElement("value","NA");
			xmlWriter.writeEndElement();
		}
                else
		     if (r_SliceWiseCkeck > protocol.GetInterlaceCheckProtocol().correlationThresholdGradient)
		     {
			xmlWriter.writeStartElement("entry");
			xmlWriter.writeAttribute( "parameter", "InterlaceWiseCheck"  );	
			xmlWriter.writeTextElement("value","Fail");
			xmlWriter.writeEndElement();
		     }
		     else
		     {
			xmlWriter.writeStartElement("entry");
			xmlWriter.writeAttribute( "parameter", "InterlaceWiseCheck"  );	
			xmlWriter.writeTextElement("value","Pass");
			xmlWriter.writeEndElement();
		     }
		}
		else
		     if (r_SliceWiseCkeck > protocol.GetInterlaceCheckProtocol().correlationThresholdGradient)
		     {
			xmlWriter.writeStartElement("entry");
			xmlWriter.writeAttribute( "parameter", "InterlaceWiseCheck"  );	
			xmlWriter.writeTextElement("value","Fail");
			xmlWriter.writeEndElement();
		     }
		     else
		     {
			xmlWriter.writeStartElement("entry");
			xmlWriter.writeAttribute( "parameter", "InterlaceWiseCheck"  );	
			xmlWriter.writeTextElement("value","Pass");
			xmlWriter.writeEndElement();
		     }
	}
	else if ( !protocol.GetSliceCheckProtocol().bCheck )
	{
		xmlWriter.writeStartElement("entry");
		xmlWriter.writeAttribute( "parameter", "SliceWiseCheck"  );	
		xmlWriter.writeTextElement("value","Not Set");
		xmlWriter.writeEndElement();
	}


////////////////////////////////////////////


	if ( !(( (qcResult.Get_result()  & SliceWiseCheckBit) == SliceWiseCheckBit ) && protocol.GetSliceCheckProtocol().bQuitOnCheckFailure) && protocol.GetInterlaceCheckProtocol().bCheck )  // Check protocol whether run InterlaceWiseChecking
	{
	if (  (qcResult.Get_result() & InterlaceWiseCheckBit) == InterlaceWiseCheckBit )
	{
	if ( protocol.GetInterlaceCheckProtocol().bQuitOnCheckFailure )
	{
		xmlWriter.writeStartElement("entry");
		xmlWriter.writeAttribute( "parameter", "InterlaceWiseCheck"  );	
		xmlWriter.writeTextElement("value","Fail Pipeline Termination");
		xmlWriter.writeEndElement();
		xmlWriter.writeStartElement("entry");
		xmlWriter.writeAttribute( "parameter", "GradientWiseCheck"  );	
		xmlWriter.writeTextElement("value","NA");
		xmlWriter.writeEndElement();
	}
	else
	{
		if ( r_InterlaceWiseCheck > protocol.GetInterlaceCheckProtocol().correlationThresholdGradient)
		{
		xmlWriter.writeStartElement("entry");
		xmlWriter.writeAttribute( "parameter", "InterlaceWiseCheck"  );	
		xmlWriter.writeTextElement("value","Fail");
		xmlWriter.writeEndElement();
		}
		else 
		{
		xmlWriter.writeStartElement("entry");
		xmlWriter.writeAttribute( "parameter", "InterlaceWiseCheck"  );	
		xmlWriter.writeTextElement("value","Pass");
		xmlWriter.writeEndElement();	
		}
	}
	}
	else 
	{
		if ( r_InterlaceWiseCheck > protocol.GetInterlaceCheckProtocol().correlationThresholdGradient)
		{
		xmlWriter.writeStartElement("entry");
		xmlWriter.writeAttribute( "parameter", "InterlaceWiseCheck"  );	
		xmlWriter.writeTextElement("value","Fail");
		xmlWriter.writeEndElement();
		}
		else 
		{
		xmlWriter.writeStartElement("entry");
		xmlWriter.writeAttribute( "parameter", "InterlaceWiseCheck"  );	
		xmlWriter.writeTextElement("value","Pass");
		xmlWriter.writeEndElement();	
		}
	}
	}
	else if ( !protocol.GetInterlaceCheckProtocol().bCheck )
	{
		xmlWriter.writeStartElement("entry");
		xmlWriter.writeAttribute( "parameter", "InterlaceWiseCheck"  );	
		xmlWriter.writeTextElement("value","Not Set");
		xmlWriter.writeEndElement();	
	}


	

/////////////////////////////
	
	
	if ( !protocol.GetSliceCheckProtocol().bQuitOnCheckFailure &&  !protocol.GetInterlaceCheckProtocol().bQuitOnCheckFailure && protocol.GetGradientCheckProtocol().bCheck )   // Check protocol whether run GradientWiseChecking
	{
	if  ( (qcResult.Get_result() & GradientWiseCheckBit) == GradientWiseCheckBit )
	{
	if (protocol.GetGradientCheckProtocol().bQuitOnCheckFailure)
	{
		xmlWriter.writeStartElement("entry");
		xmlWriter.writeAttribute( "parameter", "GradientWiseCheck"  );	
		xmlWriter.writeTextElement("value","Fail Pipeline Terminatio");
		xmlWriter.writeEndElement();
	}
	else
	{
		if ( r_GradWiseCheck > protocol.GetInterlaceCheckProtocol().correlationThresholdGradient)
		{
		xmlWriter.writeStartElement("entry");
		xmlWriter.writeAttribute( "parameter", "GradientWiseCheck"  );	
		xmlWriter.writeTextElement("value","Fail");
		xmlWriter.writeEndElement();
		}
		else 
		{
		xmlWriter.writeStartElement("entry");
		xmlWriter.writeAttribute( "parameter", "GradientWiseCheck"  );	
		xmlWriter.writeTextElement("value","Pass");
		xmlWriter.writeEndElement();
		}		
	}
	}
	else
	{
		if ( r_GradWiseCheck > protocol.GetInterlaceCheckProtocol().correlationThresholdGradient)
		{
		xmlWriter.writeStartElement("entry");
		xmlWriter.writeAttribute( "parameter", "GradientWiseCheck"  );	
		xmlWriter.writeTextElement("value","Fail");
		xmlWriter.writeEndElement();
		}
		else 
		{
		xmlWriter.writeStartElement("entry");
		xmlWriter.writeAttribute( "parameter", "GradientWiseCheck"  );	
		xmlWriter.writeTextElement("value","Pass");
		xmlWriter.writeEndElement();
		}	
	}

	}
	else if ( ! protocol.GetGradientCheckProtocol().bCheck )
	{
		xmlWriter.writeStartElement("entry");
		xmlWriter.writeAttribute( "parameter", "GradientWiseCheck"  );	
		xmlWriter.writeTextElement("value","Not Set");
		xmlWriter.writeEndElement();	

	}





/*		xmlWriter.writeStartElement("entry");
		xmlWriter.writeAttribute( "parameter", "GradientWiseCheck"  );	
		xmlWriter.writeTextElement("value","Fail Pipeline Terminatio");
		xmlWriter.writeEndElement();	
	}
	}
	else if ( ! protocol.GetGradientCheckProtocol().bCheck )
	{
		xmlWriter.writeStartElement("entry");
		xmlWriter.writeAttribute( "parameter", "GradientWiseCheck"  );	
		xmlWriter.writeTextElement("value","Not Set");
		xmlWriter.writeEndElement();	

	}

	if ((qcResult.Get_result()  & SliceWiseCheckBit) == 0 )
	{
	if ( protocol.GetSliceCheckProtocol().bCheck )
	{
	if ( r_SliceWiseCkeck > protocol.GetInterlaceCheckProtocol().correlationThresholdGradient )
	{
		xmlWriter.writeStartElement("entry");
		xmlWriter.writeAttribute( "parameter", "SliceWiseCheck"  );	
		xmlWriter.writeTextElement("value","Fail");
		xmlWriter.writeEndElement();
	}
	else 
	{
	 	xmlWriter.writeStartElement("entry");
		xmlWriter.writeAttribute( "parameter", "SliceWiseCheck"  );	
		xmlWriter.writeTextElement("value","Pass");
		xmlWriter.writeEndElement();
	}
	}
	if ((qcResult.Get_result() & InterlaceWiseCheckBit) == 0)
	{
	if ( protocol.GetInterlaceCheckProtocol().bCheck )
	{
	if ( r_InterlaceWiseCheck > protocol.GetInterlaceCheckProtocol().correlationThresholdGradient)
	{
		xmlWriter.writeStartElement("entry");
		xmlWriter.writeAttribute( "parameter", "InterlaceWiseCheck"  );	
		xmlWriter.writeTextElement("value","Fail");
		xmlWriter.writeEndElement();
	}
	else 
	{
		xmlWriter.writeStartElement("entry");
		xmlWriter.writeAttribute( "parameter", "InterlaceWiseCheck"  );	
		xmlWriter.writeTextElement("value","Pass");
		xmlWriter.writeEndElement();	
	}
	}
	if ((qcResult.Get_result() & GradientWiseCheckBit) == 0)
	{
	if (protocol.GetGradientCheckProtocol().bCheck)
	{
	if ( r_GradWiseCheck > protocol.GetInterlaceCheckProtocol().correlationThresholdGradient)
	{
		xmlWriter.writeStartElement("entry");
		xmlWriter.writeAttribute( "parameter", "GradientWiseCheck"  );	
		xmlWriter.writeTextElement("value","Fail");
		xmlWriter.writeEndElement();
	}
	else 
	{
		xmlWriter.writeStartElement("entry");
		xmlWriter.writeAttribute( "parameter", "GradientWiseCheck"  );	
		xmlWriter.writeTextElement("value","Pass");
		xmlWriter.writeEndElement();
	}	
	}
	}
	}
	
	}
*///



        for ( unsigned int i = 0;
	i < qcResult.GetIntensityMotionCheckResult().size();
	i++ )
	{

	// gradient
	bool EXCLUDE_SliceWiseCheck = false;
	bool EXCLUDE_InterlaceWiseCheck = false;
	bool EXCLUDE_GreadientWiseCheck= false;
	
	xmlWriter.writeStartElement("entry");
  	xmlWriter.writeAttribute( "parameter",  QString("gradient_%1").arg( i, 4, 10, QLatin1Char( '0' ) ) );	
	
	switch ( qcResult.GetIntensityMotionCheckResult()[i].processing )
	{
	case QCResult::GRADIENT_BASELINE_AVERAGED:
        xmlWriter.writeTextElement("processing","BASELINE_AVERAGED");
	break;
	case QCResult::GRADIENT_EXCLUDE_SLICECHECK:
	{
        xmlWriter.writeTextElement("processing","EXCLUDE_SLICECHECK");
	EXCLUDE_SliceWiseCheck=true;
	}
	break;
	case QCResult::GRADIENT_EXCLUDE_INTERLACECHECK:
	{
        xmlWriter.writeTextElement("processing","EXCLUDE_INTERLACECHECK");
	EXCLUDE_InterlaceWiseCheck=true;
	}
	break;
	case QCResult::GRADIENT_EXCLUDE_GRADIENTCHECK:
	{
        xmlWriter.writeTextElement("processing","EXCLUDE_GRADIENTCHECK");
	EXCLUDE_GreadientWiseCheck=true;
	}
	break;
	case QCResult::GRADIENT_EXCLUDE_MANUALLY:
        xmlWriter.writeTextElement("processing","EXCLUDE");
	break;
	case QCResult::GRADIENT_EDDY_MOTION_CORRECTED:
        xmlWriter.writeTextElement("processing","EDDY_MOTION_CORRECTED");
	break;
	case QCResult::GRADIENT_INCLUDE:
	default:
        xmlWriter.writeTextElement("processing","INCLUDE");
	break;
	}
	
        xmlWriter.writeStartElement("entry");
  	xmlWriter.writeAttribute( "parameter",  "OriginalDir" );
        xmlWriter.writeTextElement("value",QString("%1 %2 %3")
	.arg(qcResult.GetIntensityMotionCheckResult()[i].OriginalDir[0], 0, 'f',
	6)
	.arg(qcResult.GetIntensityMotionCheckResult()[i].OriginalDir[1], 0, 'f',
	6)
	.arg(qcResult.GetIntensityMotionCheckResult()[i].OriginalDir[2], 0, 'f',
	6));	
      	xmlWriter.writeEndElement();
	xmlWriter.writeStartElement("entry");
  	xmlWriter.writeAttribute( "parameter",  "ReplacedDir" );
        xmlWriter.writeTextElement("value",QString("%1 %2 %3")
	.arg(qcResult.GetIntensityMotionCheckResult()[i].ReplacedDir[0], 0, 'f',
	6)
	.arg(qcResult.GetIntensityMotionCheckResult()[i].ReplacedDir[1], 0, 'f',
	6)
	.arg(qcResult.GetIntensityMotionCheckResult()[i].ReplacedDir[2], 0, 'f',
	6));		
      	xmlWriter.writeEndElement();
	xmlWriter.writeStartElement("entry");
  	xmlWriter.writeAttribute( "parameter",  "CorrectedDir" );
        xmlWriter.writeTextElement("value",QString("%1 %2 %3")
	.arg(qcResult.GetIntensityMotionCheckResult()[i].CorrectedDir[0], 0, 'f',
	6)
	.arg(qcResult.GetIntensityMotionCheckResult()[i].CorrectedDir[1], 0, 'f',
	6)
	.arg(qcResult.GetIntensityMotionCheckResult()[i].CorrectedDir[2], 0, 'f',
	6));		
      	xmlWriter.writeEndElement();

	xmlWriter.writeStartElement("entry");
  	xmlWriter.writeAttribute( "parameter",  "SliceWiseCheck" );

	if (protocol.GetSliceCheckProtocol().bCheck )
  	{
	if (EXCLUDE_SliceWiseCheck==true)
	{
	xmlWriter.writeTextElement("processing","EXCLUDE");
	for (unsigned int S_index=0; S_index<qcResult.GetSliceWiseCheckResult().size(); S_index++)
	{
	if (i==qcResult.GetSliceWiseCheckResult()[S_index].GradientNum)
	{
	xmlWriter.writeStartElement("entry");
  	xmlWriter.writeAttribute( "parameter",  "Slice#" );
        xmlWriter.writeTextElement("value",QString("%1").arg(qcResult.GetSliceWiseCheckResult()[S_index].SliceNum));
      	xmlWriter.writeEndElement();
	xmlWriter.writeStartElement("entry");
  	xmlWriter.writeAttribute( "parameter",  "Correlation" );
        xmlWriter.writeTextElement("value",QString("%1").arg(qcResult.GetSliceWiseCheckResult()[S_index].Correlation));
      	xmlWriter.writeEndElement();
	}
	}
	}
	if (EXCLUDE_SliceWiseCheck==false){
        xmlWriter.writeTextElement("processing","INCLUDE");
	}
	}
	xmlWriter.writeEndElement();
	xmlWriter.writeStartElement("entry");
  	xmlWriter.writeAttribute( "parameter",  "InterlaceWiseCheck" );
	if ( !(( (qcResult.Get_result()  & SliceWiseCheckBit) == SliceWiseCheckBit ) && protocol.GetSliceCheckProtocol().bQuitOnCheckFailure) )
	{ 
        if ( protocol.GetInterlaceCheckProtocol().bCheck )
        {
	if (EXCLUDE_InterlaceWiseCheck==true )
           xmlWriter.writeTextElement("processing","EXCLUDE");
	else 
	if (EXCLUDE_SliceWiseCheck==true)
           xmlWriter.writeTextElement("processing","NA");
	else 
           xmlWriter.writeTextElement("processing","INCLUDE");
	
	xmlWriter.writeStartElement("entry");
  	xmlWriter.writeAttribute( "parameter",  "InterlaceAngleX" );
        if( qcResult.GetInterlaceWiseCheckResult()[i].AngleX < protocol.GetInterlaceCheckProtocol().rotationThreshold )
	        xmlWriter.writeTextElement("green",QString("%1").arg(qcResult.GetInterlaceWiseCheckResult()[i].AngleX));// rotation threshold in protocol
        else 
      	        xmlWriter.writeTextElement("red",QString("%1").arg(qcResult.GetInterlaceWiseCheckResult()[i].AngleX));// rotation threshold in protocol
      	xmlWriter.writeEndElement();
	xmlWriter.writeStartElement("entry");
  	xmlWriter.writeAttribute( "parameter",  "InterlaceAngleY" );
	if( qcResult.GetInterlaceWiseCheckResult()[i].AngleY < protocol.GetInterlaceCheckProtocol().rotationThreshold )
	        xmlWriter.writeTextElement("green",QString("%1").arg(qcResult.GetInterlaceWiseCheckResult()[i].AngleY));// rotation threshold in protocol
        else 
      	        xmlWriter.writeTextElement("red",QString("%1").arg(qcResult.GetInterlaceWiseCheckResult()[i].AngleY));// rotation threshold in protocol
      	xmlWriter.writeEndElement();
	xmlWriter.writeStartElement("entry");
  	xmlWriter.writeAttribute( "parameter",  "InterlaceAngleZ" );
	if( qcResult.GetInterlaceWiseCheckResult()[i].AngleZ < protocol.GetInterlaceCheckProtocol().rotationThreshold )
	        xmlWriter.writeTextElement("green",QString("%1").arg(qcResult.GetInterlaceWiseCheckResult()[i].AngleZ));// rotation threshold in protocol
        else 
      	        xmlWriter.writeTextElement("red",QString("%1").arg(qcResult.GetInterlaceWiseCheckResult()[i].AngleZ));// rotation threshold in protocol
      	xmlWriter.writeEndElement();
        xmlWriter.writeStartElement("entry");
  	xmlWriter.writeAttribute( "parameter",  "InterlaceTranslationX" );
	if(qcResult.GetInterlaceWiseCheckResult()[i].TranslationX < protocol.GetInterlaceCheckProtocol().translationThreshold )
	        xmlWriter.writeTextElement("green",QString("%1").arg(qcResult.GetInterlaceWiseCheckResult()[i].TranslationX)); // translation threshold in protocol
        else 
      	        xmlWriter.writeTextElement("red",QString("%1").arg(qcResult.GetInterlaceWiseCheckResult()[i].TranslationX)); // translation threshold in protocol
      	xmlWriter.writeEndElement();
        xmlWriter.writeStartElement("entry");
  	xmlWriter.writeAttribute( "parameter",  "InterlaceTranslationY" );
	if( qcResult.GetInterlaceWiseCheckResult()[i].TranslationY < protocol.GetInterlaceCheckProtocol().translationThreshold )
	        xmlWriter.writeTextElement("green",QString("%1").arg(qcResult.GetInterlaceWiseCheckResult()[i].TranslationY)); // translation threshold in protocol
        else 
      	        xmlWriter.writeTextElement("red",QString("%1").arg(qcResult.GetInterlaceWiseCheckResult()[i].TranslationY)); // translation threshold in protocol
      	xmlWriter.writeEndElement();
        xmlWriter.writeStartElement("entry");
  	xmlWriter.writeAttribute( "parameter",  "InterlaceTranslationZ" );
	if( qcResult.GetInterlaceWiseCheckResult()[i].TranslationZ < protocol.GetInterlaceCheckProtocol().translationThreshold )
	        xmlWriter.writeTextElement("green",QString("%1").arg(qcResult.GetInterlaceWiseCheckResult()[i].TranslationZ)); // translation threshold in protocol
        else 
      	        xmlWriter.writeTextElement("red",QString("%1").arg(qcResult.GetInterlaceWiseCheckResult()[i].TranslationZ)); // translation threshold in protocol
      	xmlWriter.writeEndElement();
	xmlWriter.writeStartElement("entry");
  	xmlWriter.writeAttribute( "parameter",  "InterlaceMetric(MI)" );
	xmlWriter.writeTextElement("value",QString("%1").arg(qcResult.GetInterlaceWiseCheckResult()[i].Metric));
      	xmlWriter.writeEndElement();
	xmlWriter.writeStartElement("entry");
        if (i==0) //baseline
        {
  	    xmlWriter.writeAttribute( "parameter",  "InterlaceCorrelation_Baseline" );
            if (qcResult.GetInterlaceWiseCheckResult()[i].Correlation < protocol.GetInterlaceCheckProtocol().correlationThresholdBaseline)   //correlation threshold for baseline in protocol
               xmlWriter.writeTextElement("green",QString("%1").arg(qcResult.GetInterlaceWiseCheckResult()[i].Correlation));
            else 
               xmlWriter.writeTextElement("red",QString("%1").arg(qcResult.GetInterlaceWiseCheckResult()[i].Correlation));

        }
        else
        {
            xmlWriter.writeAttribute( "parameter",  "InterlaceCorrelation" );
	    if (qcResult.GetInterlaceWiseCheckResult()[i].Correlation < protocol.GetInterlaceCheckProtocol().correlationThresholdGradient)   //correlation threshold gradients in protocol 
               xmlWriter.writeTextElement("green",QString("%1").arg(qcResult.GetInterlaceWiseCheckResult()[i].Correlation));
            else 
               xmlWriter.writeTextElement("red",QString("%1").arg(qcResult.GetInterlaceWiseCheckResult()[i].Correlation));
        }
      	xmlWriter.writeEndElement();	
	}
	xmlWriter.writeEndElement();	
	xmlWriter.writeStartElement("entry");
  	xmlWriter.writeAttribute( "parameter",  "GradientWiseCheck" );
	if ( (((!qcResult.Get_result()  & InterlaceWiseCheckBit) == InterlaceWiseCheckBit ) || (!(protocol.GetSliceCheckProtocol().bQuitOnCheckFailure || protocol.GetInterlaceCheckProtocol().bQuitOnCheckFailure) )) )
        {
         if ( protocol.GetGradientCheckProtocol().bCheck )
         {
	if (EXCLUDE_GreadientWiseCheck==true )
	    xmlWriter.writeTextElement("value","EXCLUDE");
	else if (EXCLUDE_SliceWiseCheck==true)
            xmlWriter.writeTextElement("value","NA");
	else 
	xmlWriter.writeTextElement("value","INCLUDE");	
	xmlWriter.writeStartElement("entry");
  	xmlWriter.writeAttribute( "parameter",  "GradientAngleX" );
	if (qcResult.GetGradientWiseCheckResult()[i].AngleX < protocol.GetGradientCheckProtocol().rotationThreshold)   // rotation threshold in protocol
            xmlWriter.writeTextElement("green",QString("%1").arg(qcResult.GetGradientWiseCheckResult()[i].AngleX));
        else 
            xmlWriter.writeTextElement("red",QString("%1").arg(qcResult.GetGradientWiseCheckResult()[i].AngleX));
      	xmlWriter.writeEndElement();
	xmlWriter.writeStartElement("entry");
  	xmlWriter.writeAttribute( "parameter",  "GradientAngleY" );
	if (qcResult.GetGradientWiseCheckResult()[i].AngleY < protocol.GetGradientCheckProtocol().rotationThreshold)   // rotation threshold in protocol
            xmlWriter.writeTextElement("green",QString("%1").arg(qcResult.GetGradientWiseCheckResult()[i].AngleY));
        else 
            xmlWriter.writeTextElement("red",QString("%1").arg(qcResult.GetGradientWiseCheckResult()[i].AngleY));
      	xmlWriter.writeEndElement();
	xmlWriter.writeStartElement("entry");
  	xmlWriter.writeAttribute( "parameter",  "GradientAngleZ" );
	if (qcResult.GetGradientWiseCheckResult()[i].AngleZ < protocol.GetGradientCheckProtocol().rotationThreshold)   // rotation threshold in protocol
            xmlWriter.writeTextElement("green",QString("%1").arg(qcResult.GetGradientWiseCheckResult()[i].AngleZ));
        else 
            xmlWriter.writeTextElement("red",QString("%1").arg(qcResult.GetGradientWiseCheckResult()[i].AngleZ));
      	xmlWriter.writeEndElement();
	xmlWriter.writeStartElement("entry");
  	xmlWriter.writeAttribute( "parameter",  "GradientTranslationX" );
	if (qcResult.GetGradientWiseCheckResult()[i].TranslationX < protocol.GetGradientCheckProtocol().translationThreshold)    // translation threshold in protocol
            xmlWriter.writeTextElement("green",QString("%1").arg(qcResult.GetGradientWiseCheckResult()[i].TranslationX));
        else 
            xmlWriter.writeTextElement("red",QString("%1").arg(qcResult.GetGradientWiseCheckResult()[i].TranslationX));
      	xmlWriter.writeEndElement();
	xmlWriter.writeStartElement("entry");
  	xmlWriter.writeAttribute( "parameter",  "GradientTranslationY" );
	if (qcResult.GetGradientWiseCheckResult()[i].TranslationY < protocol.GetGradientCheckProtocol().translationThreshold)    // translation threshold in protocol
            xmlWriter.writeTextElement("green",QString("%1").arg(qcResult.GetGradientWiseCheckResult()[i].TranslationY));
        else 
            xmlWriter.writeTextElement("red",QString("%1").arg(qcResult.GetGradientWiseCheckResult()[i].TranslationY));
      	xmlWriter.writeEndElement();
	xmlWriter.writeStartElement("entry");
  	xmlWriter.writeAttribute( "parameter",  "GradientTranslationZ" );
	if (qcResult.GetGradientWiseCheckResult()[i].TranslationZ < protocol.GetGradientCheckProtocol().translationThreshold)    // translation threshold in protocol
            xmlWriter.writeTextElement("green",QString("%1").arg(qcResult.GetGradientWiseCheckResult()[i].TranslationZ));
        else 
            xmlWriter.writeTextElement("red",QString("%1").arg(qcResult.GetGradientWiseCheckResult()[i].TranslationZ));
      	xmlWriter.writeEndElement();
	xmlWriter.writeStartElement("entry");
  	xmlWriter.writeAttribute( "parameter",  "GradientMetric(MI)" );
	xmlWriter.writeTextElement("value",QString("%1").arg(qcResult.GetGradientWiseCheckResult()[i].MutualInformation));
      	xmlWriter.writeEndElement();
	}
	}
	else {
	xmlWriter.writeTextElement("processing","NA");
        }
      	xmlWriter.writeEndElement();

	}
	else{
	xmlWriter.writeTextElement("processing","NA");
	xmlWriter.writeEndElement();	
	xmlWriter.writeStartElement("entry");
  	xmlWriter.writeAttribute( "parameter",  "GradientWiseCheck" );
	xmlWriter.writeTextElement("processing","NA");
	xmlWriter.writeEndElement();
	}

	xmlWriter.writeStartElement("entry");
  	xmlWriter.writeAttribute( "parameter",  "Visual Check" );
        xmlWriter.writeStartElement("entry");
  	xmlWriter.writeAttribute( "parameter",  QString("VC_Status_%1").arg( i, 4, 10, QLatin1Char( '0' ) ) );
	switch ( qcResult.GetIntensityMotionCheckResult()[i].VisualChecking )
	{
	case QCResult::GRADIENT_BASELINE_AVERAGED:
        xmlWriter.writeTextElement("value","BASELINE_AVERAGED");
	break;
	case QCResult::GRADIENT_EXCLUDE_SLICECHECK:
	{
	xmlWriter.writeTextElement("value","EXCLUDE_SLICECHECK");
	EXCLUDE_SliceWiseCheck=true;
	}
	break;
	case QCResult::GRADIENT_EXCLUDE_INTERLACECHECK:
	{
        xmlWriter.writeTextElement("value","EXCLUDE_INTERLACECHECK");
	EXCLUDE_InterlaceWiseCheck=true;
	}
	break;
	case QCResult::GRADIENT_EXCLUDE_GRADIENTCHECK:
	{
        xmlWriter.writeTextElement("value","EXCLUDE_GRADIENTCHECK");
	EXCLUDE_GreadientWiseCheck=true;
	}
	break;
	case QCResult::GRADIENT_EXCLUDE_MANUALLY:
        xmlWriter.writeTextElement("value","EXCLUDE");
	break;
	case QCResult::GRADIENT_EDDY_MOTION_CORRECTED:
        xmlWriter.writeTextElement("value","EDDY_MOTION_CORRECTED");
	break;
	case QCResult::GRADIENT_INCLUDE:
	xmlWriter.writeTextElement("value","INCLUDE");
	break;
	default:
        xmlWriter.writeTextElement("value","NoChange");
	break;
	}
        xmlWriter.writeEndElement();
      	xmlWriter.writeEndElement();
      	xmlWriter.writeEndElement();
     }
     xmlWriter.writeEndElement();

     xmlWriter.writeEndDocument();	
     file.close();
     if ( file.error() )
     {
        std::cerr << "Error: Cannot write file "
	<< qPrintable(Result_xmlFile) << ": "
	<< qPrintable( file.errorString() ) << std::endl;
     } 
        




        if ( resultNotes.length() > 0 )
        {
          std::ofstream outfile;

          QFileInfo noteFile( QString::fromStdString(resultNotes) );
          if ( !noteFile.exists() )
          {
            outfile.open( resultNotes.c_str(),  std::ios::app);
            outfile  <<
              "DWI\t#AllGrad/#AllGradLeft\t#b0/#b0Left\t#Grad/#GradLeft\t#GradDir/#GradDirLeft\t#Rep/#RepLeft\tImageInfo\tDiffusionInfo\tSliceWise\tInterlaceWise\tGradWise\tGradDirLess6\tSingleBValueNoB0\tTooManyBadDirs";
          }
          else
          {
            outfile.open( resultNotes.c_str(),  std::ios::app);
          }

          if ( DWIFileName.rfind('/') != string::npos )
          {
            std::cout << DWIFileName.substr( DWIFileName.rfind(
              '/') + 1, DWIFileName.length() - DWIFileName.rfind('/')
              - 1) << std::endl;
            outfile << std::endl << DWIFileName.substr( DWIFileName.rfind(
              '/') + 1, DWIFileName.length() - DWIFileName.rfind('/') - 1);
          }
          else
          {
            std::cout << "DWI file name: " << DWIFileName << std::endl;
            outfile << std::endl << DWIFileName;
          }

          std::cout  << "GradientTotal#/LeftGradientTotal#: "
            << IntensityMotionCheck.getGradientNumber()
            + IntensityMotionCheck.getBaselineNumber() << "/"
            << IntensityMotionCheck.
            getGradientLeftNumber()
            + IntensityMotionCheck.getBaselineLeftNumber() << std::endl;
          outfile   << "\t" << IntensityMotionCheck.getGradientNumber()
            + IntensityMotionCheck.getBaselineNumber() << "/"
            << IntensityMotionCheck.
            getGradientLeftNumber()
            + IntensityMotionCheck.getBaselineLeftNumber();

          std::cout << "Baseline#/LeftBaseline#: "
            << IntensityMotionCheck.getBaselineNumber() << "/"
            <<  IntensityMotionCheck.getBaselineLeftNumber()
            << std::endl;
          outfile   << "\t" << IntensityMotionCheck.getBaselineNumber() << "/"
            <<  IntensityMotionCheck.getBaselineLeftNumber();

          std::cout << "Gradient#/LeftGradient#: "
            << IntensityMotionCheck.getGradientNumber() << "/"
            << IntensityMotionCheck.getGradientLeftNumber() << std::endl;
          outfile   << "\t" << IntensityMotionCheck.getGradientNumber() << "/"
            << IntensityMotionCheck.getGradientLeftNumber();

          std::cout << "GradientDir#/LeftGradientDir#: "
            << IntensityMotionCheck.getGradientDirNumber() << "/"
            << IntensityMotionCheck.getGradientDirLeftNumber()
            << std::endl;
          outfile   << "\t" << IntensityMotionCheck.getGradientDirNumber()
            << "/" << IntensityMotionCheck.getGradientDirLeftNumber();

          std::cout << "Repetition#/RepetitionLeft#: "
            << IntensityMotionCheck.getRepetitionNumber() << "/-"
            << std::endl;
          outfile   << "\t" << IntensityMotionCheck.getRepetitionNumber() << "/-";

          out = result;
          out = out << 7;
          out = out >> 7;
          if ( out )
          {
            std::cout << "Image information check: FAILURE" << std::endl;
            outfile << "\tFAILURE";
          }
          else
          {
            std::cout << "Image information check: PASS" << std::endl;
            outfile << "\tPASS";
          }

          out = result;
          out = out << 6;
          out = out >> 7;
          if ( out )
          {
            std::cout << "Diffusion information check: FAILURE" << std::endl;
            outfile << "\tFAILURE";
          }
          else
          {
            std::cout << "Diffusion information check: PASS" << std::endl;
            outfile << "\tPASS";
          }

          out = result;
          out = out << 5;
          out = out >> 7;
          if ( out  )
          {
            std::cout << "Slice-wise check: FAILURE" << std::endl;
            outfile << "\tFAILURE";
          }
          else
          {
            std::cout << "Slice-wise check: PASS" << std::endl;
            outfile << "\tPASS";
          }

          out = result;
          out = out << 4;
          out = out >> 7;
          if ( out  )
          {
            std::cout << "Interlace-wise check: FAILURE" <<  std::endl;
            outfile << "\tFAILURE";
          }
          else
          {
            std::cout << "Interlace-wise check: PASS" << std::endl;
            outfile << "\tPASS";
          }

          out = result;
          out = out << 3;
          out = out >> 7;
          if ( out )
          {
            std::cout << "Gradient-wise check: FAILURE" << std::endl;
            outfile << "\tFAILURE";
          }
          else
          {
            std::cout << "Gradient-wise check: PASS" << std::endl;
            outfile << "\tPASS";
          }

          // ZYXEDCBA:
          // X QC;Too many bad gradient directions found!
          // Y QC; Single b-value DWI without a b0/baseline!
          // Z QC: Gradient direction # is less than 6!

          out = result;
          out = out >> 7;
          if ( out )
          {
            std::cout << "Too many bad gradient directions found!" << std::endl;
            outfile << "\tToo many bad gradient directions found!";
          }
          else
          {
            outfile << "\t";
          }

          out = result;
          out = out << 1;
          out = out >> 7;
          if ( out )
          {
            std::cout << "Single b-value DWI without a b0/baseline!" << std::endl;
            outfile << "\tSingle b-value DWI without a b0/baseline!";
          }
          else
          {
            outfile << "\t";
          }

          out = result;
          out = out << 2;
          out = out >> 7;
          if ( out )
          {
            std::cout << "Gradient direction # is less than 6!" << std::endl;
            outfile << "\tGradient direction # is less than 6!";
          }
          else
          {
            outfile << "\t";
          }

          outfile.close();
        }
        return result;
      }
    }
    return 0;
  }
}


