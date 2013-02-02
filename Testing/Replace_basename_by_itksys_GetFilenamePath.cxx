#include <itksys/SystemTools.hxx>
#include <libgen.h>
#include <iostream>




int main( int argc , char* argv[] )
{
  std::string filename = "/test/path/filename.cxx" ;
  std::cout << "'filename' variable original value: \n" << filename << std::endl << std::endl ;
  std::string fileNameGetFilenamePath = itksys::SystemTools::GetFilenamePath( filename ) ;
  std::string fileNameDirName = dirname( const_cast<char *>(filename.c_str() ) );
  //The value of 'filename' has been modified by 'dirname()'
  std::cout << "'filename' variable value after 'dirname()': \n" << filename << std::endl << std::endl ;
  std::cout << "fileNameBaseName: " << fileNameDirName << std::endl ;
  std::cout << "fileNameGetFilenamePath: " << fileNameGetFilenamePath << std::endl ;
  if( fileNameDirName != fileNameGetFilenamePath )
  {
    return -1 ;
  }
  return 0 ;
}
