#Find external tools

macro( PrintNotFound name )
  message( STATUS "${name} not found. Searching for it at runtime" )
endmacro()

macro( FindToolMacro path name )
  find_program(${path} ${name} )
  if(NOT ${path} )
    PrintNotFound( ${name} )
  endif(NOT ${path} )
endmacro( FindToolMacro )

FindToolMacro( BRAINSFitTOOL BRAINSFit )
FindToolMacro( BRAINSDemonWarpTOOL BRAINSDemonWarp )
FindToolMacro(dtiprocessTOOL dtiprocess )
FindToolMacro(ANTSTOOL ANTS)
FindToolMacro(WarpImageMultiTransformTOOL WarpImageMultiTransform)
FindToolMacro(ResampleDTIlogEuclideanTOOL ResampleDTIlogEuclidean)
FindToolMacro(ITKTransformToolsTOOL ITKTransformTools)

######################################################################
##To add DTITK registration##
######################################################################
#FindToolMacro(TVtoolTOOL TVtool)#DTI-TK - scaling tensor unit
#FindToolMacro(DTIConvertTOOL DTIConvert)#DTI-TK - convert from NRRD to NIFTI
#FindToolMacro(dti_affine_reg_TOOL dti_affine_reg)#DTI_TK - Affine registration
#FindToolMacro(dti_diffeomorphic_reg_TOOL dti_diffeomorphic_reg)#DTI_TK - diffeomorphic registration
######################################################################
