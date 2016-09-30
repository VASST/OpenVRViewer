PROJECT( OPENVRVIEWER )

CMAKE_MINIMUM_REQUIRED( VERSION 2.2 )

FIND_PACKAGE( vtk REQUIRED )
INCLUDE( ${VTK_USE_FILE} )

FIND_PACKAGE( OvrvisionPro REQUIRED)
FIND_PACKAGE( OpenVR REQUIRED )
FIND_PACKAGE( SDL REQUIRED)
FIND_PACKAGE(OpenCV REQUIRED)

ADD_EXECUTABLE( OpenVRViewer
  main.cpp
)

target_include_directories( OpenVRViewer
							PRIVATE ${SDL_INCLUDE_DIR} ${OPENVR_INCLUDE_DIRS})
							
target_link_libraries( OpenVRViewer
					OvrvisionPro
					vtkRenderingOpenVR
					vtkRenderingFreeType
					vtkIOImage
					vtkFiltersTexture
					vtkFiltersSources
					vtkRenderingGL2PS${VTK_RENDERING_BACKEND})