#include <itksys/SystemTools.hxx>
#include <libgen.h>
#include <iostream>




int main( int argc , char* argv[] )
{
  std::string filename = "/test/path/filename.cxx" ;
  std::cout << "'filename' variable original value: \n" << filename << std::endl << std::endl ;
  std::string fileNameBaseName =  basename( const_cast<char *>(filename.c_str() ) );
  std::cout << "'filename' variable value after 'basename()': \n" << filename << std::endl << std::endl ;
  std::string fileNameGetFilenameName = itksys::SystemTools::GetFilenameName( filename ) ;
  std::cout << "fileNameBaseName: " << fileNameBaseName << std::endl ;
  std::cout << "fileNameGetFilenameName: " << fileNameGetFilenameName << std::endl ;
  if( fileNameBaseName != fileNameGetFilenameName )
  {
    return -1 ;
  }
  return 0 ;
}
