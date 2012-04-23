file(READ ${fixfile} code)
string(REPLACE "REQUIRED PATH " "REQUIRED PATHS "
  code "${code}")
string(REPLACE "GenerateCLP(DTIReg_SOURCE DTI-Reg.xml)"
"GenerateCLP(DTIReg_SOURCE DTI-Reg.xml)
link_directories(${linkdir})"
code "${code}")

file(WRITE ${fixfile} "${code}")
