#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <iostream>

#include <QtCore>
#include <QDir>

//#include "constants.h"

using namespace std;

MainWindow::MainWindow(QWidget *parent)
  : QMainWindow(parent)
  , ui(new Ui::MainWindow)
{
  ui->setupUi(this);

#ifdef Q_OS_ANDROID

  // Копируем файл модели определения эмоций в домашнюю директорию, чтобы обращаться к нему как к файлу а не ресурсу
//  QString model_name {"pred_center_model3.tflite"};
//  QString model_name {"pred_center_model2.tflite"};
  QString emotion_detect_file_name {QDir::homePath() + QDir::separator() + g_model_name};
//  QString emotion_detect_file_name {"/data/user/0/kap.center_predictor.example/files/pred_center_model.tflite"};
  QFile emo_file(emotion_detect_file_name);
  if (emo_file.exists())
    {
      emo_file.remove();
    }
  QFile res("assets:/" + g_model_name);
  res.copy(emotion_detect_file_name);

//  QDir dir{QDir::home()};
//  for (const auto &entry : dir.entryList())
//    ui->textEdit->append(entry);

#endif

  update_cameras();
#ifdef Q_OS_ANDROID
  ui->cameras_combo_box->setCurrentIndex(1);
#else
  ui->cameras_combo_box->setCurrentIndex(0);
#endif
//  ui->cv_widget->set_camera(qvariant_cast<QCameraDevice>(ui->cameras_combo_box->itemData(1)));

  ui->cv_widget->start();
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


void MainWindow::on_increase_button_clicked()
{
  ui->cv_widget->predict_size_up();
}


void MainWindow::on_decrease_button_clicked()
{
  ui->cv_widget->predict_size_down();
}

