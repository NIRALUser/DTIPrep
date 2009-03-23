
#include <iostream>
#include <fstream>
#include <string>


#include <QApplication>
#include <QPushButton>
#include <QString>

//#include <ui_MainWindow.h>
#include "GMainWindow.h"
#include "main.h"


// Work-around CMake dependency scanning limitation.  This must
// duplicate the above list of headers.

//# include "CommandLineArguments.hxx.in"
//# include "kwsys_ios_iostream.h.in"

#include <stddef.h> /* size_t */
#include <string.h> /* strcmp */

#include "metaCommand.h"

void usage()
{
  cerr << "Usage: dtiprep  [in] [out] <options>\n" << endl;
  cerr << "-dicom2nrrd directory nrrdfilename \n" << endl;
  cerr << "-intravolumecheck correctionThreshold RotationThreshold TranslationThreshold \n" << endl;
  cerr << "-intervolumecheck RotationThreshold TranslationThreshold \n" << endl;
 exit(1);

}

using namespace std;
int main ( int argc, char ** argv )
{
	

	if(argc==1)
	{
		QApplication app (argc, argv);
		GMainWindow *MainWindow = new GMainWindow;
		MainWindow ->show();
		return app.exec();
	}else
	{
		char   c;
		string line,line1;

		string strtemp;
		double FA, MD, pos;

		bool header=0;

		ofstream outfiletemp; 
		ofstream outfile; 
		ifstream infiletemp,infiletemp1;

		string str;
		str.append("dir "); //windows "ls " for linux
		str.append(argv[1]);
		str.append( "/b   >filelist.txt");
		cout<<str<<endl;
		system(str.c_str());
		//system("dir   D:\\Rebecca\\NeoFibers\\arcuate_L5_clean_inf  /b   >filelist.txt");
		ifstream list;
		list.open("filelist.txt");

		string filename,prefilename;


		int m=-1;
		list.seekg(0, ios::beg);
		while (! list.eof() )
		{
			++m;
			getline(list, filename); 

			string path;		
			path.append(argv[1]);
			path.append("\\");
			path.append(filename);
			cout<<path<<endl;

			if(m==0)
			{
				outfiletemp.open((path+string(".txt")).c_str());
				outfiletemp<<"\t\t"<<filename.substr(filename.find_last_of("\\")+1, 10)<<endl;

				infiletemp.open(path.c_str());
				if (infiletemp.is_open())
				{
					infiletemp.seekg(0, ios::beg);
					int i=0;
					while (! infiletemp.eof() )
					{
						infiletemp>>strtemp;
						i++;

						if(i>1000000)
						{
							if((i-1000000)%7==1) {	cout<<strtemp<<"\t"; outfiletemp<<strtemp<<"\t"; }
							if((i-1000000)%7==2) {	cout<<strtemp<<"\t"; }//outfiletemp<<strtemp<<"\t"; }
							if((i-1000000)%7==3) {	cout<<strtemp<<endl; outfiletemp<<strtemp<<endl; }
						}	
						if(strcmp(strtemp.c_str(),"Lambda3")==0)
						{
							i=1000000;
						}
					}
					infiletemp.close();
					outfiletemp.close();
				}
				else cout << "Unable to open input file"; 
			}
			else
			{
				cout<<filename<<endl;

				infiletemp1.open((prefilename+string(".txt")).c_str());
				infiletemp.open(path.c_str());
				outfiletemp.open((path+string(".txt")).c_str());

				if (!infiletemp.is_open())
					cout<<endl<<filename<<"Not open"<<endl;
				if (!infiletemp1.is_open())
					cout<<endl<<prefilename+string(".txt")<<"Not open"<<endl;
				if (!outfiletemp.is_open())
					cout<<endl<<filename+string(".txt")<<"Not open"<<endl;

				if (infiletemp.is_open() && infiletemp1.is_open() && outfiletemp.is_open())
				{


					infiletemp.seekg(0, ios::beg);
					infiletemp1.seekg(0, ios::beg);

					string linetemp;
					getline(infiletemp1, linetemp);
					outfiletemp<<linetemp<< "\t"<<filename.substr(filename.find_last_of("\\")+1, 10)<<endl;


					int i=0;
					//cout <<i<<"  ";
					while (! infiletemp.eof() )
					{
						infiletemp>>strtemp;
						i++;
						//cout <<i<<"  ";
						if(i>1000000)
						{
							if((i-1000000)%7==1) {	cout<<strtemp<<"\t"; }
							if((i-1000000)%7==2) {	cout<<strtemp<<"\t"; }
							if((i-1000000)%7==3) {  cout<<strtemp<<endl;  getline(infiletemp1,line1);	 line1+="\t"; line1+=strtemp; 
							outfiletemp << line1 << endl;}	
						}
						if(strcmp(strtemp.c_str(),"Lambda3")==0)
						{
							i=1000000;
						}
					}
					infiletemp1.close();
					infiletemp.close();
					outfiletemp.close();
				}
				else cout << "Unable to open  file"; 			
			}
			prefilename=path;

		}

		list.close();
///////////////////////////////////////////////////////////////////////
	//system("dir   D:\\Rebecca\\NeoFibers\\arcuate_L5_clean_inf  /s/b   >filelist.txt");

/*

		string fvp[13]={
			"D:\\Rebecca\\NeoFibers\\arcuate_L5_clean_inf\\neo-0019-2-1-deformed-arcuate_L5_clean_inf-FA-MD.fvp",
			"D:\\Rebecca\\NeoFibers\\arcuate_L5_clean_inf\\neo-0075-1-deformed-arcuate_L5_clean_inf-FA-MD.fvp",
			"D:\\Rebecca\\NeoFibers\\arcuate_L5_clean_inf\\neo-0133-1-deformed-arcuate_L5_clean_inf-FA-MD.fvp",
			"D:\\Rebecca\\NeoFibers\\arcuate_L5_clean_inf\\neo-0146-1-deformed-arcuate_L5_clean_inf-FA-MD.fvp",
			"D:\\Rebecca\\NeoFibers\\arcuate_L5_clean_inf\\neo-0147-1-deformed-arcuate_L5_clean_inf-FA-MD.fvp",
			"D:\\Rebecca\\NeoFibers\\arcuate_L5_clean_inf\\neo-0161-1-deformed-arcuate_L5_clean_inf-FA-MD.fvp",
			"D:\\Rebecca\\NeoFibers\\arcuate_L5_clean_inf\\neo-0187-1-deformed-arcuate_L5_clean_inf-FA-MD.fvp",
			"D:\\Rebecca\\NeoFibers\\arcuate_L5_clean_inf\\neo-0189-1-deformed-arcuate_L5_clean_inf-FA-MD.fvp",
			"D:\\Rebecca\\NeoFibers\\arcuate_L5_clean_inf\\TS03-deformed-arcuate_L5_clean_inf-FA-MD.fvp",
			"D:\\Rebecca\\NeoFibers\\arcuate_L5_clean_inf\\TS04-deformed-arcuate_L5_clean_inf-FA-MD.fvp",
			"D:\\Rebecca\\NeoFibers\\arcuate_L5_clean_inf\\TS05-deformed-arcuate_L5_clean_inf-FA-MD.fvp",
			"D:\\Rebecca\\NeoFibers\\arcuate_L5_clean_inf\\TS07-deformed-arcuate_L5_clean_inf-FA-MD.fvp",
			"D:\\Rebecca\\NeoFibers\\arcuate_L5_clean_inf\\TS17-deformed-arcuate_L5_clean_inf-FA-MD.fvp"
		};
	
	for(int m=0;m<13;m++)
	{
		if(m==0)
		{
			outfiletemp.open((fvp[0]+string(".txt")).c_str());

			outfiletemp<<"\t\t"<<fvp[0].substr(fvp[0].find_last_of("\\")+1, 10)<<endl;

			infiletemp.open(fvp[0].c_str());
			if (infiletemp.is_open())
			{
				infiletemp.seekg(0, ios::beg);
				int i=0;
				while (! infiletemp.eof() )
				{
					infiletemp>>strtemp;
					i++;

					if(i>1000000)
					{
						if((i-1000000)%7==1) {	cout<<strtemp<<"\t"; outfiletemp<<strtemp<<"\t"; }
						if((i-1000000)%7==2) {	cout<<strtemp<<"\t"; }//outfiletemp<<strtemp<<"\t"; }
						if((i-1000000)%7==3) {	cout<<strtemp<<endl; outfiletemp<<strtemp<<endl; }
					}	
					if(strcmp(strtemp.c_str(),"Lambda3")==0)
					{
						i=1000000;
					}
				}
				infiletemp.close();
				outfiletemp.close();
			}
			else cout << "Unable to open input file"; 
		}
		else
		{
			cout<<fvp[m]<<endl;

			infiletemp1.open((fvp[m-1]+string(".txt")).c_str());
			infiletemp.open(fvp[m].c_str());
			outfiletemp.open((fvp[m]+string(".txt")).c_str());

			if (!infiletemp.is_open())
				cout<<endl<<fvp[m]<<"Not open"<<endl;
			if (!infiletemp1.is_open())
				cout<<endl<<fvp[m-1]+string(".txt")<<"Not open"<<endl;
			if (!outfiletemp.is_open())
				cout<<endl<<fvp[m]+string(".txt")<<"Not open"<<endl;
			
			if (infiletemp.is_open() && infiletemp1.is_open() && outfiletemp.is_open())
			{


				infiletemp.seekg(0, ios::beg);
				infiletemp1.seekg(0, ios::beg);

				string linetemp;
				getline(infiletemp1, linetemp);
				outfiletemp<<linetemp<< "\t"<<fvp[m].substr(fvp[m].find_last_of("\\")+1, 10)<<endl;
				

				int i=0;
				//cout <<i<<"  ";
				while (! infiletemp.eof() )
				{
					infiletemp>>strtemp;
					i++;
					//cout <<i<<"  ";
					if(i>1000000)
					{
						if((i-1000000)%7==1) {	cout<<strtemp<<"\t"; }
						if((i-1000000)%7==2) {	cout<<strtemp<<"\t"; }
						if((i-1000000)%7==3) {  cout<<strtemp<<endl;  getline(infiletemp1,line1);	 line1+="\t"; line1+=strtemp; 
						outfiletemp << line1 << endl;}	
					}
					if(strcmp(strtemp.c_str(),"Lambda3")==0)
					{
						i=1000000;
					}
				}
				infiletemp1.close();
				infiletemp.close();
				outfiletemp.close();
			}
			else cout << "Unable to open  file"; 			
		}		
	}


	*/
///////////////////////////////////////////////////////////

/*
  if (infile.is_open())
  {
    infile.seekg(0, ios::beg);
    int i=0;
    while (! infile.eof() )
    {
	infile>>strtemp;
	i++;
	
	if(i>1000000)
	{
		if((i-1000000)%7==1) {	cout<<strtemp<<"\t"; outfile<<strtemp<<"\t"; }
		if((i-1000000)%7==2) {	cout<<strtemp<<"\t"; outfile<<strtemp<<"\t"; }
		if((i-1000000)%7==3) {	cout<<strtemp<<endl; outfile<<strtemp<<endl; }
	}	
	if(strcmp(strtemp.c_str(),"Lambda3")==0)
	{
		i=1000000;
		//cout<<strtemp<<endl;
	
	}
    }
    infile.close();
  }
  else cout << "Unable to open file"; 

   outfile.close();  

   ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  ifstream tempinFile;
  tempinFile.open(  "D:\\Rebecca\\NeoFibers\\arcuate_L5_clean_inf-FA-MD.txt");
  outfiletemp.open(  "D:\\Rebecca\\NeoFibers\\arcuate_L5_clean_inf\\neo-0075-1-deformed-arcuate_L5_clean_inf-FA-MD.txt");
  ifstream infile1("D:\\Rebecca\\NeoFibers\\arcuate_L5_clean_inf\\neo-0075-1-deformed-arcuate_L5_clean_inf-FA-MD.fvp");
  if (infile1.is_open())
  {
    infile1.seekg(0, ios::beg);
	tempinFile.seekg(0, ios::beg);
	
    int i=0;
    while (! infile1.eof() )
    {
	infile1>>strtemp;
	i++;
	
	if(i>1000000)
	{
		
		if((i-1000000)%7==1) {	cout<<strtemp<<"\t"; outfile<<strtemp<<"\t";}
		if((i-1000000)%7==2) {	cout<<strtemp<<"\t"; outfile<<strtemp<<"\t"; }
		if((i-1000000)%7==3) {getline(tempinFile,line1);	cout<<strtemp<<endl; outfile<<strtemp<<endl; line1+="\t"; line1+=strtemp; 
		outfiletemp << line1 << endl;
		cout<<line1 << endl;}
	}
	
	if(strcmp(strtemp.c_str(),"Lambda3")==0)
	{
		i=1000000;
		//cout<<strtemp<<endl;
	
	}
    }
    infile1.close();
  }
  else cout << "Unable to open file"; 

*/

/*
  if (infile.is_open())
  {
    infile.seekg(0, ios::beg);
    while (! infile.eof() )
    {
	getline(infile,line);
	
	//if(header) cout << line << endl;
	cout << line << endl;
	

       if(strcmp(line.c_str(), "X 	 nfibpts 	 FA 	 MD 	 GA 	 Lambda1 	 Lambda2 	 Lambda3")==0)
	{header=1;}

       
	
      //cout << line << endl;
      //txtfile << line << endl;
    }
    infile.close();
  }
  else cout << "Unable to open file"; 
*/
 

		/*
		int ok;
		double corrationThreshold, intraRTh, intraTTh;//intraVolume check
		char *dicom_dir = NULL, *nrrd_filename = NULL;
		double interRTh, interTTh;//intraVolume check
		

		// Parse filenames

		while (argc > 1) {
			ok = false;
			if ((ok == false) && (strcmp(argv[1], "-dicom2nrrd") == 0))
			{
				std::cout<<argv[1]<<"	";
				//std::cout<<endl;
				argc--;
				if(argc<=2)
				{
					std::cout<<"no enought parameters"<<std::endl;
					return 0;
				}
				argv++;
				dicom_dir=argv[1];
				std::cout<<argv[1]<<"	";
				//std::cout<<endl;

				argc--;
				argv++;
				if(argc==0) std::cout<<"no enought parameters"<<std::endl;
				nrrd_filename=argv[1];
				std::cout<<argv[1];
				std::cout<<endl;

				argc--;
				argv++;

				ok = true;
			}
			if ((ok == false) && (strcmp(argv[1], "-intravolumecheck") == 0))
			{
				std::cout<<argv[1]<<"	";
				//std::cout<<endl;
				argc--;
				if(argc<=3)
				{
					std::cout<<"no enought parameters"<<std::endl;
					return 0;
				}

				argv++;
				corrationThreshold = atof(argv[1]);
				std::cout<<argv[1]<<"	";
				//std::cout<<endl;

				argc--;
				argv++;
				intraRTh = atof(argv[1]);
				std::cout<<argv[1]<<"	";
				//std::cout<<endl;
				argc--;
				argv++;
				intraTTh = atof(argv[1]);
				std::cout<<argv[1]<<"	";
				std::cout<<endl;
				argc--;
				argv++;
				ok = true;
			}
			if ((ok == false) && (strcmp(argv[1], "-intervolumecheck") == 0)) 
			{
				std::cout<<argv[1]<<"	";
				//std::cout<<endl;
				argc--;
				if(argc<=2)
				{
					std::cout<<"no enought parameters"<<std::endl;
					return 0;
				}

				argv++;
				interRTh = atof(argv[1]);
				std::cout<<argv[1]<<"	";
				//std::cout<<endl;
				argc--;
				argv++;
				interTTh = atof(argv[1]);
				std::cout<<argv[1];
				std::cout<<endl;
				argc--;
				argv++;
				ok = true;
			}
			//if ((ok == false) && (strcmp(argv[1], "-eddycurrentmotion") == 0)) 
			//{
				//argc--;
				//argv++;
				//pos[0] = atof(argv[1]);
				//argc--;
				//argv++;
				//pos[1] = atof(argv[1]);
				//argc--;
				//argv++;
				//pos[2] = atof(argv[1]);
				//argc--;
				//argv++;
				//ok = true;
			//}
			if (ok == false)
			{
				cout << "Can't parse argument: " << argv[1] << endl;
				usage();
			}
		}




//		for(int i=0;i<argc;++i)
//			std::cout<<argv[i]<<" ";
//		std::cout<<endl;

//		int res = 0;
//		kwsys::CommandLineArguments arg;
//		arg.Initialize(argc, argv);
*/

		return 0;
	}

}
