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
  QString model_file_name {QDir::homePath() + QDir::separator() + g_model_name};
  QFile emo_file(model_file_name);
  if (emo_file.exists())
    {
      emo_file.remove();
    }
  QFile res("assets:/" + g_model_name);
  res.copy(model_file_name);
#endif

  update_cameras(); // получаем список камер
#ifdef Q_OS_ANDROID
  ui->cameras_combo_box->setCurrentIndex(1); // для Android выбираем вронтальную по умолчанию
#else
  ui->cameras_combo_box->setCurrentIndex(0);
#endif
  // запускаем цикл предсказания центра
  ui->cv_widget->start();
}

MainWindow::~MainWindow()
{
  delete ui;
}

// получаем список камер
void MainWindow::update_cameras()
{
  ui->cameras_combo_box->clear();
  const QList<QCameraDevice> availableCameras = QMediaDevices::videoInputs();
  for (const QCameraDevice &cameraDevice : availableCameras)
    {
      ui->cameras_combo_box->addItem(cameraDevice.description(), QVariant::fromValue(cameraDevice));
    }
}

// Меняем камеру по выбору пользователя
void MainWindow::on_cameras_combo_box_currentIndexChanged(int)
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

