#ifndef ACV_WIDGET_H
#define ACV_WIDGET_H

//#pragma once

#include <QWidget>
#include <QDir>
#include <QRandomGenerator>
#include <QCamera>
#include <QMediaCaptureSession>
#include <QMediaDevices>
#include <QImageCapture>
#include <QVideoWidget>
#include <QTimer>
#include <QTextEdit>
#include <QVideoSink>
#include <QMessageBox>
#include <QPalette>
#include <QImage>

#include <QtWidgets>

#include <opencv2/opencv.hpp>

#include "tensorflow/lite/interpreter.h"
#include "tensorflow/lite/kernels/register.h"
//#include "tensorflow/lite/nnapi/sl/include/SupportLibrary.h"
//#include "tensorflow/lite/model.h"
//#include "tensorflow/lite/optional_debug_tools.h"
//#include "tensorflow/lite/c/c_api_types.h"

#if defined(Q_OS_ANDROID)
#include "tensorflow/lite/delegates/nnapi/nnapi_delegate.h"
#include "tensorflow/lite/delegates/gpu/delegate.h"
#endif  // ANDROID

#include "constants.h"

class ACV_Widget : public QWidget
{
  Q_OBJECT
public:
  explicit ACV_Widget(QWidget *parent = nullptr);

  void set_camera(const QCameraDevice &cameraDevice);
  void predict_size_up();
  void predict_size_down();
  void start();

private:
  cv::VideoCapture m_cam {};
#ifdef Q_OS_ANDROID
  const std::string m_baze_dir {(QDir::homePath()+QDir::separator()).toStdString()};
#else
  const std::string m_baze_dir {"assets/"};
#endif
  const std::string m_center_predict_file {m_baze_dir + g_model_name.toStdString()};
const int CAPTURE_DELAY {10};

//  cv::dnn::Net m_face_detect_model;

  // Build the interpreter
  std::unique_ptr<tflite::FlatBufferModel> m_model;
  tflite::ops::builtin::BuiltinOpResolver m_resolver;
  std::unique_ptr<tflite::Interpreter> m_interpreter;
  TfLiteStatus m_status {};

  cv::Mat predict_center(const cv::Mat &frame, cv::Size output_size);

//  const int INPUT_WIDTH {150};
//  const int INPUT_HEIGTH {150};
  int m_input_width {128};
  int m_input_height {128};
  cv::Size m_input_size;
  int m_output_width {128};
  int m_output_height {128};
  cv::Size m_output_size;
//  cv::Size INPUT_SIZE{m_input_width, m_input_height};
//  const cv::Size OUTPUT_SIZE{50, 50};
//  const cv::Size OUTPUT_SIZE{32, 32};

  int m_canal_count {3};
//  const size_t POINT_COUNT = {static_cast<size_t>(m_input_width * m_input_height)};
//  const size_t DATA_SIZE {static_cast<size_t>(POINT_COUNT * m_input_canal_count)};
  size_t m_point_count;
  size_t m_data_size;
  double m_part_of_screen = 1.0/4.0;

  QRandomGenerator rand_gen{13494895};

  QScopedPointer<QCamera> m_camera;
  QVideoSink *m_video_sink{new QVideoSink{this}};
  QMediaCaptureSession m_captureSession;
  QImage m_curr_image;
  int m_id {-1};
  QTimer m_timer;
  cv::Mat m_frame{};
  std::vector<cv::Rect> m_face_coords;

  void on_capture_timer();
  void on_readyForCapture(bool ready);

  int m_img_height {0};
  int m_img_width {0};

//signals:


  // QObject interface
protected:
  virtual void paintEvent(QPaintEvent *event) override;

};

#endif // ACV_WIDGET_H
