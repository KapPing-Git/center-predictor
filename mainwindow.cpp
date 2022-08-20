#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <iostream>

#include <QtCore>
#include <QDir>

using namespace std;

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
      QFile res("assets:/best_model.tflite");
      res.copy(emotion_detect_file_name);
    }
#endif

  update_cameras();

}

MainWindow::~MainWindow()
{
  delete ui;
}

void MainWindow::update_cameras()
{
  ui->cameras_combo_box->clear();
  const QList<QCameraDevice> availableCameras = QMediaDevices::videoInputs();
  for (const QCameraDevice &cameraDevice : availableCameras)
    {
      ui->cameras_combo_box->addItem(cameraDevice.description(), QVariant::fromValue(cameraDevice));
    }
}

void MainWindow::on_cameras_combo_box_currentIndexChanged(int index)
{
  ui->cv_widget->set_camera(qvariant_cast<QCameraDevice>(ui->cameras_combo_box->currentData()));
}

