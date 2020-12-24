#include "VideoMaker.h"

//
#include "QmitkAnimationWidget.h"

#include "QmitkAnimationItemDelegate.h"
#include "QmitkOrbitAnimationItem.h"
#include "QmitkOrbitAnimationWidget.h"

#include "QmitkRenderWindow.h"
#include "QmitkSliceWidget.h"

// #include <mitkGL.h>

#include "QmitkFFmpegWriter2.h"
#include <mitkRenderingManager.h>

#include <mitkBaseRenderer.h>
//


#include "mitkProperties.h"
#include "mitkImageAccessByItk.h"
#include <mitkIOUtil.h>

#include <mitkTransferFunction.h>
#include <mitkTransferFunctionProperty.h>
#include <mitkTransferFunctionPropertySerializer.h>

#include "vtkRenderer.h"
#include "vtkImageWriter.h"
#include "vtkJPEGWriter.h"
#include "vtkPNGWriter.h"
#include "vtkRenderLargeImage.h"
#include <vtkCamera.h>
#include <QString>
#include <QDir>



VideoMaker::VideoMaker(int argc, char *argv[]){
  Load(argc, argv);
}

VideoMaker::~VideoMaker(){
}

void VideoMaker::Initialize(){
  //Setting the data

  mitk::StandaloneDataStorage::SetOfObjects::ConstPointer dataNodes = this->m_DataStorageOriginal->GetAll();
  
  //Reading each image and setting TransferFunction 
  for(int image_index = 0; image_index < dataNodes->Size(); image_index++){
    mitk::Image::Pointer currentMitkImage = dynamic_cast<mitk::Image *>(dataNodes->at(image_index)->GetData());
    
    this->m_ResultNode = mitk::DataNode::New();
    this->m_DataStorageModified->Add(this->m_ResultNode);
    this->m_ResultNode->SetData(currentMitkImage);
    // set some additional properties
    this->m_ResultNode->SetProperty("binary", mitk::BoolProperty::New(false));

    // Volume Rendering and Transfer function
    this->m_ResultNode->SetProperty("volumerendering", mitk::BoolProperty::New(true));
    this->m_ResultNode->SetProperty("volumerendering.usegpu", mitk::BoolProperty::New(true));
    this->m_ResultNode->SetProperty("layer", mitk::IntProperty::New(1));

    mitk::TransferFunction::Pointer tf = mitk::TransferFunction::New();
    tf->InitializeByMitkImage(currentMitkImage);

    if(this->m_TransferFunctionFile.isEmpty())
      this->SetDefaultTransferFunction(tf);
    else
      tf = mitk::TransferFunctionPropertySerializer::DeserializeTransferFunction(this->m_TransferFunctionFile.toLatin1());

    this->m_ResultNode->SetProperty("TransferFunction", mitk::TransferFunctionProperty::New(tf.GetPointer()));
    mitk::LevelWindowProperty::Pointer levWinProp = mitk::LevelWindowProperty::New();
    mitk::LevelWindow levelwindow;
    levelwindow.SetAuto(currentMitkImage);
    levWinProp->SetLevelWindow(levelwindow);
    this->m_ResultNode->SetProperty("levelwindow", levWinProp);
  }

  this->InitializeRenderWindow();

}

void VideoMaker::SetDefaultTransferFunction(mitk::TransferFunction::Pointer tf){
  // Set the color transfer function AddRGBPoint(double x, double r, double g, double b)
  // grayvalue->opacity                                                                                                                               
  tf->GetScalarOpacityFunction()->AddPoint(-1024, 0);
  tf->GetScalarOpacityFunction()->AddPoint(-907.30622617534937, 0.0031847133757961776);
  tf->GetScalarOpacityFunction()->AddPoint(-625.76944704779748, 0);
  tf->GetScalarOpacityFunction()->AddPoint(-549.05275779376507, 0.06687898089171973);
  tf->GetScalarOpacityFunction()->AddPoint(-217.28959700093719, 0.089171974522292974);
  tf->GetScalarOpacityFunction()->AddPoint(-216.70009372071229, 0);
  tf->GetScalarOpacityFunction()->AddPoint(115.90065604498591, 0);
  tf->GetScalarOpacityFunction()->AddPoint(1689.4210526315787, 0.0059880239520958556);
                                                                   
  // gradient at grayvalue->opacity                                                                                                                   
  tf->GetGradientOpacityFunction()->AddPoint(560.69500000000005, 1.000000);                                                                    

  // grayvalue->color                                                                                                                                 
  tf->GetColorTransferFunction()->AddRGBPoint(-903.32333645735707, 0.90588235294117647, 0, 0.050980392156862744);
  tf->GetColorTransferFunction()->AddRGBPoint(-857.09746954076854, 1, 0.28235294117647053, 0.10196078431372546);
  tf->GetColorTransferFunction()->AddRGBPoint(-507.02530459231491, 0.45490196078431372, 0.67843137254901964, 0.81960784313725488);
  tf->GetColorTransferFunction()->AddRGBPoint(-307.96626054358012, 1, 1, 1);
  tf->GetColorTransferFunction()->AddRGBPoint(-38.56138706654167, 1, 1, 0.74901960784313726);
}

QString VideoMaker::GetNewFileName(QString filePath, QString fileName, QString secondFileName){
  int c = 1;
  while (QFile::exists(filePath+fileName)){
    fileName = secondFileName; //QString("/3D_View1_");
    fileName += QString::number(c);
    fileName += ".png";
    c++;
  }
  return fileName;
}



void ReadPixels(std::unique_ptr<unsigned char[]>& frame, vtkRenderWindow* renderWindow, int x, int y, int width, int height){ 
  if (nullptr == renderWindow)  return;
    renderWindow->MakeCurrent();
    glReadPixels(x, y, width, height, GL_RGB, GL_UNSIGNED_BYTE, frame.get());
}

QmitkAnimationItem* CreateDefaultAnimation(const QString& widgetKey, double time){
  if (widgetKey == "Orbit"){
    //Total duration: 10 seg
    //return new QmitkOrbitAnimationItem(4140, false, 120.0, 0.0, false);
    return new QmitkOrbitAnimationItem(360, false, time, 0.0, false);
  }
  return nullptr;
}


void VideoMaker::CalculateTotalDuration(int fpsSpinBox)
{
  const int rowCount = m_AnimationModel->rowCount();

  double totalDuration = 0.0;
  double previousStart = 0.0;

  for (int i = 0; i < rowCount; ++i)
  {
   auto item = dynamic_cast<QmitkAnimationItem*>(m_AnimationModel->item(i, 1));

    if (nullptr == item)
      continue;

    if (item->GetStartWithPrevious())
    {
      totalDuration = std::max(totalDuration, previousStart + item->GetDelay() + item->GetDuration());
    }
    else
    {
      previousStart = totalDuration;
      totalDuration += item->GetDelay() + item->GetDuration();
    }
  }

  m_TotalDuration = totalDuration;
  m_NumFrames = static_cast<int>(totalDuration * fpsSpinBox);
  cout<<"Duration: "<<m_TotalDuration<<", Frames"<<m_NumFrames<<endl;
}

void VideoMaker::InitializeRenderWindow()
{
  mitk::StandaloneDataStorage::Pointer ds = mitk::StandaloneDataStorage::New();
  QmitkRenderWindow* renderWindow = new QmitkRenderWindow();

  // Tell the renderwindow which (part of) the tree to render
  renderWindow->GetRenderer()->SetDataStorage(m_DataStorageModified);

  // Use it as a 3D view
  renderWindow->GetRenderer()->SetMapperID(mitk::BaseRenderer::Standard3D);
  //renderWindow->resize(1280, 1125);
  renderWindow->resize(this->m_Width+6, this->m_Height+6);

  //NOAMLR
  renderWindow->setAttribute(Qt::WA_DontShowOnScreen);
  renderWindow->show();


  m_renderWindow2 = renderWindow->GetVtkRenderWindow();
  mitk::BaseRenderer* baserenderer = mitk::BaseRenderer::GetInstance(m_renderWindow2);
  auto renderer = baserenderer->GetVtkRenderer();

  
  renderer->ResetCamera();
  vtkCamera* vtkcam = renderer->GetActiveCamera();
  vtkcam->Zoom(2.1);

  vtkcam->Roll( 90 );

  vtkcam->Azimuth( 90 );
  vtkcam->Roll( -90 );

  if (nullptr != renderWindow){
    int fpsSpinBox = this->m_Fps;
    
    //m_FFmpegWriter = new QmitkFFmpegWriter(nullptr); 
    m_FFmpegWriter = new QmitkFFmpegWriter2(); 

    const QString ffmpegPath =  "/usr/bin/ffmpeg"; // GetFFmpegPath();
    m_FFmpegWriter->SetFFmpegPath(ffmpegPath);

    const int border = 3;
    const int x = border;
    const int y = border;
    int width = m_renderWindow2->GetSize()[0] - border * 2; 
    int height = m_renderWindow2->GetSize()[1] - border * 2; 

    if (width & 1) --width;
    if (height & 1) --height;
    if (width < 16 || height < 16) return;


    m_FFmpegWriter->SetSize(width, height);
    m_FFmpegWriter->SetFramerate(fpsSpinBox);

    QString saveFileName = m_OutputDir; //"PassNameByValue.mp4";
    if(!saveFileName.endsWith(".mp4"))  saveFileName += ".mp4";

    m_FFmpegWriter->SetOutputPath(saveFileName);

    //Animation

    m_AnimationModel = new QStandardItemModel(nullptr);
    const auto key = "Orbit"; //action->text();
    m_AnimationModel->appendRow(QList<QStandardItem*>() << new QStandardItem(key) << CreateDefaultAnimation(key, this->m_Time));

    this->CalculateTotalDuration(fpsSpinBox);

    try{
      auto frame = std::make_unique<unsigned char[]>(width * height * 3);
      m_FFmpegWriter->Start();

      for (m_CurrentFrame = 0; m_CurrentFrame < m_NumFrames; ++m_CurrentFrame){
        this->RenderCurrentFrame();
        ReadPixels(frame, m_renderWindow2, x, y, width, height);
        m_FFmpegWriter->WriteFrame(frame.get());
      }
      m_FFmpegWriter->Stop();

      m_CurrentFrame = 0;
      this->RenderCurrentFrame();
    }
    catch (const mitk::Exception& exception){
      cout<< "Movie Maker: "<< exception.GetDescription()<<endl;
    }
  }
}


void VideoMaker::RenderCurrentFrame(){
  double deltaT = m_TotalDuration / (m_NumFrames - 1);
  // cout<<"CurrentFrame: "<<m_CurrentFrame<<endl;
  // cout<<"Just 7.1-deltaT: "<<deltaT<<", m_NumFrames: "<<m_NumFrames<<endl;
  const auto activeAnimations = this->GetActiveAnimations(m_CurrentFrame * deltaT);

  for (const auto& animation : activeAnimations){
    const auto nextActiveAnimations = this->GetActiveAnimations((m_CurrentFrame + 1) * deltaT);
    bool lastFrameForAnimation = true;
    for (const auto& nextAnimation : nextActiveAnimations){
      if (nextAnimation.first == animation.first){
        lastFrameForAnimation = false;
        break;
      }                                                                                                                                                 
    }
    animation.first->Animate2(!lastFrameForAnimation ? animation.second : 1.0, m_renderWindow2);
  }
  mitk::RenderingManager::GetInstance()->ForceImmediateUpdateAll();
}

std::vector<std::pair<QmitkAnimationItem*, double>> VideoMaker::GetActiveAnimations(double t) const{
  const int rowCount = m_AnimationModel->rowCount();
  std::vector<std::pair<QmitkAnimationItem*, double>> activeAnimations;

  double totalDuration = 0.0;
  double previousStart = 0.0;
  for (int i = 0; i < rowCount; ++i){
    QmitkAnimationItem* item = dynamic_cast<QmitkAnimationItem*>(m_AnimationModel->item(i, 1));
    if (item == nullptr) continue;
    if (item->GetDuration() > 0.0){
      double start = item->GetStartWithPrevious() ? previousStart + item->GetDelay() : totalDuration + item->GetDelay();
      if (start <= t && t <= start + item->GetDuration())
        activeAnimations.emplace_back(item, (t - start) / item->GetDuration());
    }

    if (item->GetStartWithPrevious()){
      totalDuration = std::max(totalDuration, previousStart + item->GetDelay() + item->GetDuration());
    }
    else{
      previousStart = totalDuration;
      totalDuration += item->GetDelay() + item->GetDuration();
    }
  }
  return activeAnimations;
}


void VideoMaker::Load(int argc, char *argv[])
{
  //*************************************************************************
  // Part I: Basic initialization
  //*************************************************************************

  this->m_DataStorageOriginal = mitk::StandaloneDataStorage::New();
  this->m_DataStorageModified = mitk::StandaloneDataStorage::New();
  bool readImage = false;
  bool readTF = false;
  bool readOuputDir = false;
  bool readWidth = false;
  bool readHeight = false;
  bool readTime = false;
  bool readFps = false;
  
  int i;
  for (i = 1; i < argc; ++i){
    if (strcmp(argv[i], "-i") == 0){
      readImage = true;
      readTF = false;
      readOuputDir = false;
      readWidth = false;
      readHeight = false;
      readTime = false;
      readFps = false;
      continue;
    } 
    if (strcmp(argv[i], "-tf") == 0){
      readImage = false;
      readTF = true;
      readOuputDir = false;
      readWidth = false;
      readHeight = false;
      readTime = false;
      readFps = false;
      continue;
    }
    if (strcmp(argv[i], "-o") == 0){
      readImage = false;
      readTF = false;
      readOuputDir = true;
      readWidth = false;
      readHeight = false;
      readTime = false;
      readFps = false;
      continue;
    }
    if (strcmp(argv[i], "-w") == 0){
      readImage = false;
      readTF = false;
      readOuputDir = false;
      readWidth = true;
      readHeight = false;
      readTime = false;
      readFps = false;
      continue;
    }
    if (strcmp(argv[i], "-h") == 0){
      readImage = false;
      readTF = false;
      readOuputDir = false;
      readWidth = false;
      readHeight = true;
      readTime = false;
      readFps = false;
      continue;
    }
    if (strcmp(argv[i], "-t") == 0){
      readImage = false;
      readTF = false;
      readOuputDir = false;
      readWidth = false;
      readHeight = false;
      readTime = true;
      readFps = false;
      continue;
    }
    if (strcmp(argv[i], "-f") == 0){
      readImage = false;
      readTF = false;
      readOuputDir = false;
      readWidth = false;
      readHeight = false;
      readTime = false;
      readFps = true;
      continue;
    }



    if(readImage){
      mitk::StandaloneDataStorage::SetOfObjects::Pointer dataNodes = mitk::IOUtil::Load(argv[i], *this->m_DataStorageOriginal);
      if (dataNodes->empty()){
        fprintf(stderr, "Could not open file %s \n\n", argv[i]);
        exit(2);
      }
    }
    if(readTF){
      this->m_TransferFunctionFile=argv[i];
    }
    if(readOuputDir){
      this->m_OutputDir=argv[i];
    }
    if(readWidth){
      this->m_Width=atoi(argv[i]);
    }
    if(readHeight){
      this->m_Height=atoi(argv[i]);
    }
    if(readTime){
      this->m_Time=atof(argv[i]);
    }
    if(readFps){
      this->m_Fps=atoi(argv[i]);
    }
  }
}

