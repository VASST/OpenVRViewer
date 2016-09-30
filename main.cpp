// Main app file

// VTK
#include <vtkSmartPointer.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderer.h>
#include <vtkTexture.h>
#include <vtkImageImport.h>
#include <vtkImageMapper.h>
#include <vtkImageActor.h>
#include <vtkImageData.h>
#include <vtkInteractorStyleImage.h>
#include <vtkPNGWriter.h>
#include <vtkPolyDataMapper.h>
#include <vtkPolyData.h>
#include <vtkTextureMapToPlane.h>
#include <vtkPlaneSource.h>

// VTK OpenVR
#include <vtkOpenVRCamera.h>
#include <vtkOpenVRRenderer.h>
#include <vtkOpenVRRenderWindow.h>
#include <vtkOpenVRRenderWindowInteractor.h>

#include <ovrvision_pro.h>

// OpenCV
#include <opencv2/opencv.hpp>
#include <cv.h>
#include <highgui.h>
#include <opencv2/imgproc/imgproc.hpp>
#include <stdio.h>

int main(int argc, char *argv[])
{
  // Requested capture format
  OVR::OvrvisionPro OvrvisionProHandle;
  OVR::Camprop RequestedFormat(OVR::OV_CAMVR_FULL);
  bool CameraSync(true);

  //Create a renderer, render window, and interactor
  vtkSmartPointer<vtkRenderer> renderer = vtkSmartPointer<vtkRenderer>::New();
  vtkSmartPointer<vtkRenderWindow> renderWindow = vtkSmartPointer<vtkRenderWindow>::New();
  renderWindow->AddRenderer(renderer);

  vtkSmartPointer<vtkRenderWindowInteractor> renderWindowInteractor = vtkSmartPointer<vtkRenderWindowInteractor>::New();
  renderWindowInteractor->SetRenderWindow(renderWindow);

  vtkOpenVRRenderWindow* vrWindow = dynamic_cast<vtkOpenVRRenderWindow*>(renderWindow.Get());

  vrWindow->SetTexturedBackground(true);

  // 10DE is NVIDIA vendor ID
  auto vendor = "NVIDIA Corporation";
  if ( !OvrvisionProHandle.Open( 0, RequestedFormat, vendor) ) // We don't need to share it with OpenGL/D3D, but in the future we could access the images in GPU memory
  {
    printf( "Unable to connect to OvrvisionPro device." );
	exit(1);
  }

  OvrvisionProHandle.SetCameraSyncMode( CameraSync );

  //Render Window
  vtkSmartPointer<vtkTexture> textureLeft =
      vtkSmartPointer<vtkTexture>::New();
  vtkSmartPointer<vtkTexture> textureRight =
      vtkSmartPointer<vtkTexture>::New();

  vtkSmartPointer<vtkImageImport> imageImportLeft =
    vtkSmartPointer<vtkImageImport>::New();

  vtkSmartPointer<vtkImageImport> imageImportRight =
    vtkSmartPointer<vtkImageImport>::New();

  vrWindow->AddRenderer(renderer);

  // Grab Left and Right Images
  cv::Mat matLeft(OvrvisionProHandle.GetCamHeight(), OvrvisionProHandle.GetCamWidth(), CV_8UC4, OvrvisionProHandle.GetCamImageBGRA( OVR::OV_CAMEYE_LEFT ));
  cv::Mat matRight(OvrvisionProHandle.GetCamHeight(), OvrvisionProHandle.GetCamWidth(), CV_8UC4, OvrvisionProHandle.GetCamImageBGRA( OVR::OV_CAMEYE_RIGHT ));

  cv::Mat rgbMatLeft;
  cv::Mat finalMatLeft;
  cv::Mat rgbMatRight;
  cv::Mat finalMatRight;

  // Convert From BGRA to RGB
  cv::cvtColor(matLeft, rgbMatLeft, CV_BGRA2RGB);
  cv::cvtColor(matRight, rgbMatRight, CV_BGRA2RGB);

  // Flip Orientation for VTK
  cv::flip(rgbMatLeft, finalMatLeft, 0);
  cv::flip(rgbMatRight, finalMatRight, 0);

  // Convert image from opencv to vtk
  imageImportLeft->SetDataSpacing( 1, 1, 1);
  imageImportLeft->SetDataOrigin( 0, 0, 0);
  imageImportLeft->SetWholeExtent( 0, finalMatLeft.size().width-1, 0, finalMatLeft.size().height - 1, 0 ,0 );
  imageImportLeft->SetDataExtentToWholeExtent();
  imageImportLeft->SetDataScalarTypeToUnsignedChar();
  imageImportLeft->SetNumberOfScalarComponents(finalMatLeft.channels());
  imageImportLeft->SetImportVoidPointer(finalMatLeft.data);
  imageImportLeft->Modified();
  imageImportLeft->Update();

  imageImportRight->SetDataSpacing( 1, 1, 1);
  imageImportRight->SetDataOrigin( 0, 0, 0);
  imageImportRight->SetWholeExtent( 0, finalMatRight.size().width-1, 0, finalMatRight.size().height - 1, 0 ,0 );
  imageImportRight->SetDataExtentToWholeExtent();
  imageImportRight->SetDataScalarTypeToUnsignedChar();
  imageImportRight->SetNumberOfScalarComponents(finalMatRight.channels());
  imageImportRight->SetImportVoidPointer(finalMatRight.data);
  imageImportRight->Modified();
  imageImportRight->Update();

  textureLeft->SetInputConnection(imageImportLeft->GetOutputPort());
  textureRight->SetInputConnection(imageImportRight->GetOutputPort());

  vrWindow->SetLeftBackgroundTexture(textureLeft);
  vrWindow->SetRightBackgroundTexture(textureRight);

  while(/*someexitcondition*/ true)
  {
    // Query the SDK for the latest frames
    OvrvisionProHandle.PreStoreCamData( OVR::OV_CAMQT_DMSRMP );

    // Grab Left and Right Images
    cv::Mat matLeft(OvrvisionProHandle.GetCamHeight(), OvrvisionProHandle.GetCamWidth(), CV_8UC4, OvrvisionProHandle.GetCamImageBGRA( OVR::OV_CAMEYE_LEFT ));
    cv::Mat matRight(OvrvisionProHandle.GetCamHeight(), OvrvisionProHandle.GetCamWidth(), CV_8UC4, OvrvisionProHandle.GetCamImageBGRA( OVR::OV_CAMEYE_RIGHT ));

    // Convert From BGRA to RGB
    cv::cvtColor(matLeft, rgbMatLeft, CV_BGRA2RGB);
    cv::cvtColor(matRight, rgbMatRight, CV_BGRA2RGB);

    // Flip Orientation for VTK
    cv::flip(rgbMatLeft, finalMatLeft, 0);
    cv::flip(rgbMatRight, finalMatRight, 0);

    // Update
    imageImportLeft->Modified();
    imageImportLeft->Update();
    imageImportRight->Modified();
    imageImportRight->Update();

    textureLeft->Modified();
    textureRight->Modified();
    textureLeft->Update();
    textureRight->Update();

    // Render
    vrWindow->Render();
  }

  if ( OvrvisionProHandle.isOpen() )
  {
    OvrvisionProHandle.Close();
  }
}
