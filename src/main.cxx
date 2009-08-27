
#include <iostream>
#include <fstream>
#include <string>

#include <QApplication>
#include <QPushButton>
#include <QString>
#include <QFileInfo>

#include "GMainWindow.h"
//#include "main.h"

#include <stddef.h> /* size_t */
#include <string.h> /* strcmp */
#include <iostream>

#include "metaCommand.h"
#include "IntensityMotionCheck.h"

#include "Protocal.h"
#include "XmlStreamReader.h"

// #include <QStyleFactory>
using namespace std;
int main ( int argc, char ** argv )
{
	if(argc==1)
	{
		QApplication app (argc, argv);
		GMainWindow *MainWindow = new GMainWindow;
		MainWindow ->show();
		return app.exec();
	}
	else
	{
		
		MetaCommand command;

		// checking input or DicomToNrrd Output
		command.SetOption(			"DWIFileName", "w", true,"DWI file name to convert dicom image series into or to be checked (nhdr)");
		command.SetOptionLongTag(	"DWIFileName","DWINrrdFile");
		command.AddOptionField(		"DWIFileName","DWIFileName",MetaCommand::STRING, true);

		// dicom to nrrd
		command.SetOption(			"Dicom2Nrrd", "d", false,"Directory containing dicom images to be converted");
		command.SetOptionLongTag(	"Dicom2Nrrd","DicomDirectory");
		command.AddOptionField(		"Dicom2Nrrd","InputDicomDir",MetaCommand::STRING,true);

		// xml based check
		command.SetOption(			"xmlSetting","p", false,"protocol xml file");
		command.SetOptionLongTag(	"xmlSetting","xmlProtocol");
		command.AddOptionField(		"xmlSetting","xmlFileName",MetaCommand::STRING, true);

		// result notes
		command.SetOption(			"resultNotes","n", false,"result notes");
		command.SetOptionLongTag(	"resultNotes","resultNotesFile");
		command.AddOptionField(		"resultNotes","NotesFile",MetaCommand::STRING, true);

		// result folder
		command.SetOption(			"outputFolder","f", false,"output folder name");
		command.SetOptionLongTag(	"outputFolder","outputFolder");
		command.AddOptionField(		"outputFolder","OutputFolder",MetaCommand::STRING, true);

		if( !command.Parse(argc,argv) )
			return EXIT_FAILURE;

		string DWIFileName	= command.GetValueAsString("DWIFileName","DWIFileName");
		string InputDicomDir= command.GetValueAsString("Dicom2Nrrd","InputDicomDir");
		string xmlFileName	= command.GetValueAsString("xmlSetting","xmlFileName");
		string resultNotes  = command.GetValueAsString("resultNotes","NotesFile");
		string resultFolder = command.GetValueAsString("outputFolder","OutputFolder");

// convert
		if(command.GetOptionWasSet("Dicom2Nrrd") && InputDicomDir.length()!=0 && DWIFileName.length()>0  )
		{
			std::string str,str1,str2;
			str += string("DicomToNrrdConverter");			
			str += string("  ");
			str += InputDicomDir;
			str += string("  ");
			str += DWIFileName;
			
			cout<<"Dicom To Nrrd convert command: "<< str <<endl;
			system(const_cast<char *>(str.c_str())); 
		}

// check with  xml
		if(command.GetOptionWasSet("xmlSetting") && xmlFileName.length()>0 )
		{			
			CIntensityMotionCheck IntensityMotionCheck( DWIFileName );
			Protocal protocal;			
			QCResult qcResult;
			QString str( xmlFileName.c_str() );
			XmlStreamReader XmlReader(NULL);
			XmlReader.setProtocal( &protocal);
			XmlReader.readFile(str, XmlStreamReader::ProtocalWise);
			protocal.GetQCOutputDirectory() = resultFolder;
			protocal.printProtocals();
			IntensityMotionCheck.SetProtocal( &protocal);
			IntensityMotionCheck.SetQCResult( &qcResult);
			IntensityMotionCheck.GetImagesInformation();
			unsigned char  result = IntensityMotionCheck.CheckByProtocal();
			unsigned char out;	
			out = result;
			std::cout << "--------------------------------" << std::endl;
			
			if(resultNotes.length()>0)
			{		
				std::ofstream outfile; 
				
				QFileInfo noteFile(QString::fromStdString(resultNotes));
				if(!noteFile.exists())
				{
					outfile.open( resultNotes.c_str(),  std::ios::app);
					outfile   << "DWI\t#AllGrad/#AllGradLeft\t#b0/#b0Left\t#Grad/#GradLeft\t#GradDir/#GradDirLeft\t#Rep/#RepLeft\tImageInfo\tDiffusionInfo\tSliceWise\tInterlaceWise\tGradWise\tGradDirLess6\tSingleBValueNoB0\tTooManyBadDirs";
				}
				else
				{
					outfile.open( resultNotes.c_str(),  std::ios::app);
				}


				if( DWIFileName.rfind('/') != string::npos)
				{
					std::cout << DWIFileName.substr( DWIFileName.rfind('/')+1, DWIFileName.length()- DWIFileName.rfind('/')-1) <<std::endl;
					outfile <<std::endl<< DWIFileName.substr( DWIFileName.rfind('/')+1, DWIFileName.length()- DWIFileName.rfind('/')-1)  ;
				}
				else
				{
					std::cout <<"DWI file name: "<< DWIFileName <<std::endl;
					outfile <<std::endl<<DWIFileName ;
				}		

				std::cout	<<"GradientTotal#/LeftGradientTotal#: "
					<< IntensityMotionCheck.getGradientNumber()+IntensityMotionCheck.getBaselineNumber()<<"/"
					<< IntensityMotionCheck.getGradientLeftNumber()+IntensityMotionCheck.getBaselineLeftNumber()<<std::endl;
				outfile   <<"\t"<< IntensityMotionCheck.getGradientNumber()+IntensityMotionCheck.getBaselineNumber()<<"/"
					<< IntensityMotionCheck.getGradientLeftNumber()+IntensityMotionCheck.getBaselineLeftNumber();

				std::cout <<"Baseline#/LeftBaseline#: "<< IntensityMotionCheck.getBaselineNumber()<<"/"<<  IntensityMotionCheck.getBaselineLeftNumber()  <<std::endl;
				outfile   <<"\t"<< IntensityMotionCheck.getBaselineNumber() <<"/"<<  IntensityMotionCheck.getBaselineLeftNumber();

				std::cout <<"Gradient#/LeftGradient#: "<< IntensityMotionCheck.getGradientNumber()<<"/"<< IntensityMotionCheck.getGradientLeftNumber()<<std::endl;
				outfile   <<"\t"<< IntensityMotionCheck.getGradientNumber() <<"/"<< IntensityMotionCheck.getGradientLeftNumber();

				std::cout <<"GradientDir#/LeftGradientDir#: "<< IntensityMotionCheck.getGradientDirNumber()<<"/"<< IntensityMotionCheck.getGradientDirLeftNumber()  <<std::endl;
				outfile   <<"\t"<< IntensityMotionCheck.getGradientDirNumber() <<"/"<< IntensityMotionCheck.getGradientDirLeftNumber();

				std::cout <<"Repetition#/RepetitionLeft#: "<< IntensityMotionCheck.getRepetitionNumber()<<"/-" <<std::endl;
				outfile   <<"\t"<< IntensityMotionCheck.getRepetitionNumber() <<"/-";

				out = result;
				out = out<<7;
				out = out>>7;
				if( out )
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
				out = out<<6;
				out = out>>7;
				if( out )
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
				out = out<<5;
				out = out>>7;
				if( out  )
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
				out = out<<4;
				out = out>>7;
				if( out  )
				{
					std::cout << "Interlace-wise check: FAILURE"<<  std::endl;
					outfile << "\tFAILURE";
				}
				else 
				{
					std::cout << "Interlace-wise check: PASS" << std::endl;
					outfile << "\tPASS";
				}

				out = result;
				out = out<<3;
				out = out>>7;
				if( out )
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
				out = out>>7;
				if( out )
				{
					std::cout << "Too many bad gradient directions found!" << std::endl;
					outfile << "\tToo many bad gradient directions found!";
				}
				else
					outfile << "\t";

				out = result;
				out = out<<1;
				out = out>>7;
				if( out )
				{
					std::cout << "Single b-value DWI without a b0/baseline!" << std::endl;
					outfile << "\tSingle b-value DWI without a b0/baseline!";
				}
				else
					outfile << "\t";

				out = result;
				out = out<<2;
				out = out>>7;
				if( out )
				{
					std::cout << "Gradient direction # is less than 6!" << std::endl;
					outfile << "\tGradient direction # is less than 6!";
				}
				else
					outfile << "\t";

				outfile.close();
			}
			return result;
		}		
		return 0;
	}
}
