file(READ ${fixfile} code)
string(REPLACE "ITKCommon ITKIO" "\${ITK_LIBRARIES}"
  code "${code}")

file(WRITE ${fixfile} "${code}")
