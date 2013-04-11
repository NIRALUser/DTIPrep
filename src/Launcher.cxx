#include <stdlib.h>
#include "DTIPrepCLP.h"
#include <string>


int main( int argc , char* argv[] )
{
  PARSE_ARGS;
  std::string path = argv[0] ;
  unsigned int found = path.find_last_of( "/" ) ;
  path = path.substr( 0 , found ) ;
#ifdef WIN32 
  std::string cmd = path + "/../hidden-cli-modules/DTIPrep.exe" ;
#else
  std::string cmd = path + "/../hidden-cli-modules/DTIPrep" ;
#endif
  for( int i = 1 ; i < argc ; i++ )
  {
    cmd += " " ;
    cmd += argv[ i ] ;
  }
  std::cout<< cmd << std::endl ;
  int val = system( cmd.c_str() ) ;
  return val ;
}
