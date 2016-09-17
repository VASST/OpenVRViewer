PROJECT( VTK_RENDER_OVRVISION )

CMAKE_MINIMUM_REQUIRED( VERSION 2.2 )

FIND_PACKAGE( vtk REQUIRED )
INCLUDE( ${VTK_USE_FILE} )

FIND_PACKAGE( OvrvisionPro REQUIRED)
FIND_PACKAGE( OpenVR REQUIRED )
FIND_PACKAGE( SDL REQUIRED)
FIND_PACKAGE(OpenCV REQUIRED)

ADD_EXECUTABLE( vtk_render_ovrvision
  main.cpp
)

target_include_directories( vtk_render_ovrvision
							PRIVATE ${SDL_INCLUDE_DIR} ${OPENVR_INCLUDE_DIRS})
							
target_link_libraries( vtk_render_ovrvision
					OvrvisionPro
					vtkOpenVR
					vtkRenderingFreeType
					vtkIOImage
					vtkFiltersTexture
					vtkFiltersSources
					vtkRenderingGL2PS${VTK_RENDERING_BACKEND})