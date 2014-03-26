
#include "UtilRegister.h"


void
itk::struRigidRegResult::Print(void) const
{
  // Print out results
  std::cout << "Result = " << std::endl;
  std::cout << " AngleX (radians) = " << this->GetFinalAngleInRadiansX() << " (degrees) = "
    << this->GetFinalAngleInDegreesX() << std::endl;
  std::cout << " AngleY (radians) = " << this->GetFinalAngleInRadiansY() << " (degrees) = "
    << this->GetFinalAngleInDegreesY()  << std::endl;
  std::cout << " AngleZ (radians) = " << this->GetFinalAngleInRadiansZ() << " (degrees) = "
    << this->GetFinalAngleInDegreesZ()  << std::endl;
  std::cout << " Translation X = " << this->GetTranslationX() << std::endl;
  std::cout << " Translation Y = " << this->GetTranslationY() << std::endl;
  std::cout << " Translation Z = " << this->GetTranslationZ() << std::endl;
  std::cout << " Metric value  = " << this->GetMutualInformation() << std::endl;
}
