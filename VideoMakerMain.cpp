/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkRegisterClasses.h"
#include "VideoMaker.h"

#include "mitkDataStorage.h"

#include <QApplication>
#include <itksys/SystemTools.hxx>
int main(int argc, char *argv[])
{
  QApplication qtapplication(argc, argv);

  if (argc < 2)
  {
    fprintf(
      stderr, "Usage:   %s -i [filename1] [filename2] ... -tf [filename3]\n\n", itksys::SystemTools::GetFilenameName(argv[0]).c_str());
    return 1;
  }

  // Register Qmitk-dependent global instances
  QmitkRegisterClasses();

  VideoMaker mainWidget(argc, argv);
  mainWidget.Initialize();

  return 0;// qtapplication.exec();
}
/**
\example VideoMakerMain.cpp
*/
