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
  OVR::Camprop RequestedFormat(OVR::OV_CAM20HD_FULL);
  bool CameraSync(true);

  //Create a renderer, render window, and interactor
  vtkSmartPointer<vtkRenderer> renderer = vtkSmartPointer<vtkRenderer>::New();
  vtkSmartPointer<vtkRenderWindow> renderWindow = vtkSmartPointer<vtkRenderWindow>::New();
  renderWindow->AddRenderer(renderer);

  vtkSmartPointer<vtkRenderWindowInteractor> renderWindowInteractor = vtkSmartPointer<vtkRenderWindowInteractor>::New();
  renderWindowInteractor->SetRenderWindow(renderWindow);

  vtkOpenVRRenderWindow* vrWindow = dynamic_cast<vtkOpenVRRenderWindow*>(renderWindow.Get());

	// OVRVision video
  unsigned int frameSize[2]={1280, 960};
  unsigned char *LeftFrameRGB = new unsigned char[frameSize[0] * frameSize[1] * sizeof( unsigned char ) * 3];
  unsigned char *RightFrameRGB = new unsigned char[frameSize[0] * frameSize[1] * sizeof( unsigned char ) * 3];

  if ( !OvrvisionProHandle.Open( 0, RequestedFormat ) ) // We don't need to share it with OpenGL/D3D, but in the future we could access the images in GPU memory
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

  imageImportLeft->SetDataScalarTypeToUnsignedChar();
  imageImportLeft->SetNumberOfScalarComponents(3);
  imageImportLeft->SetImportVoidPointer(LeftFrameRGB);
  imageImportLeft->SetWholeExtent(0, frameSize[0]-1, 0, frameSize[1]-1, 0, 0);

  imageImportRight->SetDataScalarTypeToUnsignedChar();
  imageImportRight->SetNumberOfScalarComponents(3);
  imageImportRight->SetImportVoidPointer(RightFrameRGB);
  imageImportRight->SetWholeExtent(0, frameSize[0]-1, 0, frameSize[1]-1, 0, 0);

  //textureLeft->SetInputConnection(imageImportLeft->GetOutputPort());
  //textureRight->SetInputConnection(imageImportRight->GetOutputPort());

  //vrWindow->SetLeftBackgroundTexture(textureLeft);
  //vrWindow->SetRightBackgroundTexture(textureRight);

  // New
  vtkSmartPointer<vtkInteractorStyleImage> style = vtkSmartPointer<vtkInteractorStyleImage>::New();
  vrWindow->AddRenderer(renderer);
  renderWindowInteractor->SetInteractorStyle(style);
  renderer->ResetCamera();

  while(/*someexitcondition*/ true)
  {
    // Query the SDK for the latest frames
    OvrvisionProHandle.PreStoreCamData( OVR::OV_CAMQT_DMSRMP );
    /*
    unsigned char* leftFrameBGRA = OvrvisionProHandle.GetCamImageBGRA( OVR::OV_CAMEYE_LEFT );
    unsigned char* rightFrameBGRA = OvrvisionProHandle.GetCamImageBGRA( OVR::OV_CAMEYE_RIGHT );

    unsigned char* leftTargetPixel = LeftFrameRGB;
    unsigned char* leftSourcePixel = leftFrameBGRA;
    unsigned char* rightTargetPixel = RightFrameRGB;
    unsigned char* rightSourcePixel = rightFrameBGRA;
    const unsigned int targetPixelStride = 3 * sizeof( unsigned char );
    const unsigned int sourcePixelStride = 4 * sizeof( unsigned char );
    for ( unsigned int y = 0; y < frameSize[1]; ++y )
    {
      for ( unsigned int x = 0; x < frameSize[0]; ++x )
      {
        leftTargetPixel[2] = leftSourcePixel[0]; // blue
        leftTargetPixel[1] = leftSourcePixel[1]; // green
        leftTargetPixel[0] = leftSourcePixel[2]; // red

        rightTargetPixel[2] = rightSourcePixel[0]; // blue
        rightTargetPixel[1] = rightSourcePixel[1]; // green
        rightTargetPixel[0] = rightSourcePixel[2]; // red

        leftTargetPixel += targetPixelStride;
        rightTargetPixel += targetPixelStride;
        leftSourcePixel += sourcePixelStride;
        rightSourcePixel += sourcePixelStride;
      }
    }
    */

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
    imageImportLeft->Update();

    imageImportRight->SetDataSpacing( 1, 1, 1);
    imageImportRight->SetDataOrigin( 0, 0, 0);
    imageImportRight->SetWholeExtent( 0, finalMatRight.size().width-1, 0, finalMatRight.size().height - 1, 0 ,0 );
    imageImportRight->SetDataExtentToWholeExtent();
    imageImportRight->SetDataScalarTypeToUnsignedChar();
    imageImportRight->SetNumberOfScalarComponents(finalMatRight.channels());
    imageImportRight->SetImportVoidPointer(finalMatRight.data);
    imageImportRight->Update();

    textureLeft->SetInputConnection(imageImportLeft->GetOutputPort());
    textureRight->SetInputConnection(imageImportRight->GetOutputPort());

    textureLeft->Update();
    textureRight->Update();

    vrWindow->SetLeftBackgroundTexture(textureLeft);
    vrWindow->SetRightBackgroundTexture(textureRight);

    //vrWindow->SetTexturedBackground(true);

    // Create a plane
    vtkSmartPointer<vtkPlaneSource> plane = vtkSmartPointer<vtkPlaneSource>::New();
    plane->SetCenter(0.0, 0.0, 0.0);
    plane->SetNormal(0.0, 0.0, 1.0);
 
    vtkSmartPointer<vtkTextureMapToPlane> texturePlane = vtkSmartPointer<vtkTextureMapToPlane>::New();
    texturePlane->SetInputConnection(plane->GetOutputPort());
 
    vtkSmartPointer<vtkPolyDataMapper> planeMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    planeMapper->SetInputConnection(texturePlane->GetOutputPort());

    vtkSmartPointer<vtkActor> leftActor = vtkSmartPointer<vtkActor>::New();
    vtkSmartPointer<vtkActor> rightActor = vtkSmartPointer<vtkActor>::New();

    leftActor->SetMapper(planeMapper);
    leftActor->SetTexture(textureLeft);
 
    rightActor->SetMapper(planeMapper);
    rightActor->SetTexture(textureRight);

    // Visualize the textured plane
    renderer->AddActor(leftActor);
    renderer->AddActor(rightActor);
    renderer->ResetCamera();
    vrWindow->AddRenderer(renderer);
    vrWindow->Render();
    //vrWindow->Render();
    //renderWindowInteractor->SetRenderWindow(vrWindow);
    //renderWindowInteractor->Initialize();
    //renderWindowInteractor->Start();

  }

  delete[] LeftFrameRGB;
  LeftFrameRGB = NULL;
  delete[] RightFrameRGB;
  RightFrameRGB = NULL;

  if ( OvrvisionProHandle.isOpen() )
  {
    OvrvisionProHandle.Close();
  }
}