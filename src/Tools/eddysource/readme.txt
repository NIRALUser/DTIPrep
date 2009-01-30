The codes in this folder is used to register diffusion weighted images(DWI) to B0 image. After the registration, the distortion caused by head motion and eddy current will be reduced.

I. Files
        The programs include the following files.
        demo.cxx
	itkGradientSteepestDescentBaseOptimizer.cxx
	itkGradientSteepestDescentBaseOptimizer.h
	itkGradientSteepestDescentOptimizer.cxx
	itkGradientSteepestDescentOptimizer.h
	itkLinearEddyCurrentTransform.h
	itkLinearEddyCurrentTransform.txx
	itkLinearHeadEddy3DCorrection.h
	itkLinearHeadEddy3DCorrection.txx
	itkDWIHeadMotionEddyCurrentCorrection.h
	itkDWIHeadMotionEddyCurrentCorrection.txx
	CmakeLists.txt

II.How to compile?
	The compilation needs CmakeLists.txt.  The directory is called source_file_DIR containing files. Run the following command to compile the code.
ccmake source_file_DIR 
            
III.How to run it?
After input demo, the program will prompt the usage.
Usage: demo  -i inputImageFile  -o outputImagefile [-h histogram_size] [-s samples] [-r translation_scale] [-l  initial_stepLength] [-e factor] [-n maxNumberOfIterations]

 	For example, we can run 
 	demo  -i AD29088.nrrd  -o out_AD29088_12.nrrd