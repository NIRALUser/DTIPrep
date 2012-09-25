file(READ ${fixfile} code)
foreach(x vtkWidgets
  "  *vtkRendering"
  "  *vtkGraphics"
  "  *vtkImaging"
  "  *vtkIO"
  "  *vtkFiltering"
  "  *vtkCommon"
  "  *vtkHybrid"
  "  *vtksys"
  "  *vtkQtChart"
  "  *vtkViews"
  "  *vtkInfovis"
  "  *vtklibxml2"
  "  *vtkDICOMParser"
  "  *vtkpng"
  "  *vtkzlib"
  "  *vtkjpeg"
  "  *vtkalglib"
  "  *vtkexpat"
  "  *vtkverdict"
  "  *vtkmetaio"
  "  *vtkNetCDF"
  "  *vtksqlite"
  "  *vtkexoIIc"
  "  *vtkftgl"
  "  *vtkfreetype"
)
  string(REGEX REPLACE "${x}" "ABCX" code "${code}")
endforeach(x)

string(REGEX REPLACE "ABCX
" "" code "${code}")

string(REPLACE "target_link_libraries(FiberViewerLight"
"target_link_libraries(FiberViewerLight \${VTK_LIBRARIES}" code "${code}")

file(WRITE ${fixfile} "${code}")
