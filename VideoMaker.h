/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef VIDEOMAKER_H
#define VIDEOMAKER_H

//#include <qmainwindow.h>
// #include <QWidget>
#include <mitkImage.h>
#include <mitkPointSet.h>
#include <mitkStandaloneDataStorage.h>

#include "vtkRenderer.h"
#include <mitkTransferFunction.h>
#include <vtkRenderWindow.h>

#include <itkImage.h>
#include <QString>
#include <QColor>

#include <QProcess>

#ifndef DOXYGEN_IGNORE


class QmitkAnimationItem;
class QmitkAnimationWidget;
//class QmitkFFmpegWriter;
class QmitkFFmpegWriter2;
class QMenu;
class QStandardItemModel;
class QTimer;
class QItemSelection;


class VideoMaker
{
  public:
    VideoMaker(int argc, char *argv[]);
    ~VideoMaker();
    virtual void Initialize();

  protected:
    void Load(int argc, char *argv[]);
    virtual void InitializeRenderWindow();

    mitk::StandaloneDataStorage::Pointer m_DataStorageOriginal;
    mitk::StandaloneDataStorage::Pointer m_DataStorageModified;

    mitk::DataNode::Pointer m_ResultNode;

    QColor m_BackgroundColor;

    QString           m_TransferFunctionFile;
    QString           m_OutputDir;   
    QString           m_LastFile;
    QString           m_PNGExtension = "PNG File (*.png)";
    QString           m_JPGExtension = "JPEG File (*.jpg)";

  private:
    void SetDefaultTransferFunction(mitk::TransferFunction::Pointer tf);
    void TakeScreenshot(vtkRenderer* renderer, unsigned int magnificationFactor, QString fileName, 
      QString filter = "", QColor backgroundColor = QColor(255,255,255));
    QString GetNewFileName(QString filePath, QString fileName, QString secondFileName);
    void CalculateTotalDuration(int fpsSpinBox);
    void RenderCurrentFrame();
    std::vector<std::pair<QmitkAnimationItem*, double>> GetActiveAnimations(double t) const;


    QmitkFFmpegWriter2* m_FFmpegWriter;
    // Ui::QmitkMovieMakerView* m_Ui;
    QStandardItemModel* m_AnimationModel;
    std::map<QString, QmitkAnimationWidget*> m_AnimationWidgets;
    // QMenu* m_AddAnimationMenu;
    // QMenu* m_RecordMenu;
    QTimer* m_Timer;
    double m_TotalDuration;
    int m_NumFrames;
    int m_CurrentFrame;
    vtkRenderWindow* m_renderWindow2;
    QProcess *m_Process;
    
};
#endif // DOXYGEN_IGNORE

#endif // STEP6_H

/**
\example VideoMaker.h
*/
