#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <iostream>

#include <QtCore>
#include <QDir>
//#include <QtAndroid>

//#include "tensorflow/lite/interpreter.h"
//#include "tensorflow/lite/kernels/register.h"
//#include "tensorflow/lite/model.h"
//#include "tensorflow/lite/optional_debug_tools.h"

using namespace std;

bool checkPermission(const QString &permission)
{
//#ifdef Q_OS_ANDROID
//  QtAndroid::PermissionResult r = QtAndroid::checkPermission(permission);
//  if(r != QtAndroid::PermissionResult::Granted)
//    {
//      QtAndroid::requestPermissionsSync( QStringList() << permission );
//      r = QtAndroid::checkPermission(permission);
//      if(r == QtAndroid::PermissionResult::Denied)
//        {
//          return false;
//        }
//      else
//        {
//          return true;
//        }
//    }
//  else
//    return(true);

//#else
//  return true;
//#endif
  return false;
}


MainWindow::MainWindow(QWidget *parent)
  : QMainWindow(parent)
  , ui(new Ui::MainWindow)
{
  ui->setupUi(this);

#ifdef Q_OS_ANDROID
  // Копируем файл модели нахождения лиц в домашнюю директорию, чтобы обращаться к нему как к файлу а не ресурсу
  QString face_detect_file_name {QDir::homePath() + QDir::separator() + "haarcascade_frontalface_default.xml"};
  QFile file(face_detect_file_name);
  if (!file.exists())
    {
      QFile res("assets:/haarcascade_frontalface_default.xml");
      res.copy(face_detect_file_name);
    }

  // Копируем файл модели определения эмоций в домашнюю директорию, чтобы обращаться к нему как к файлу а не ресурсу
  QString emotion_detect_file_name {QDir::homePath() + QDir::separator() + "best_model.tflite"};
  QFile emo_file(emotion_detect_file_name);
  if (!emo_file.exists())
    {
//      ui->textEdit->setText("not exist");
      QFile res("assets:/best_model.tflite");
      res.copy(emotion_detect_file_name);
    }
//  else
//    ui->textEdit->setText("exist");

  QDir dir {QDir::homePath()};
  for (auto elem : dir.entryList())
    ui->textEdit->append(elem);

//  if (checkPermission("android.permission.CAMERA"))
//    {
//      ui->textEdit->append("android.permission.CAMERA выдано");
//    }
//  else
//    {
//      ui->textEdit->append("android.permission.CAMERA не выдано");
//    }
#endif

//  m_camera.reset(new QCamera(QMediaDevices::defaultVideoInput()));
//  m_captureSession.setCamera(m_camera.data());
//  m_captureSession.setVideoOutput(ui->video_widget);
//  m_camera->start();

}

MainWindow::~MainWindow()
{
  delete ui;
}

