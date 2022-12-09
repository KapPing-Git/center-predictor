#ifndef ACV_WIDGET_H
#define ACV_WIDGET_H

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
#include "tensorflow/lite/c/c_api_types.h"

#if defined(Q_OS_ANDROID)
#include "tensorflow/lite/delegates/nnapi/nnapi_delegate.h"
#include "tensorflow/lite/delegates/gpu/delegate.h"
#endif  // ANDROID

class ACV_Widget : public QWidget
{
  Q_OBJECT
public:
  explicit ACV_Widget(QWidget *parent = nullptr);

  void set_camera(const QCameraDevice &cameraDevice);

private:
  cv::VideoCapture m_cam {};
#ifdef Q_OS_ANDROID
  const std::string m_baze_dir {(QDir::homePath()+QDir::separator()).toStdString()};
  const int CAPTURE_DELAY {300};
#else
  const std::string m_baze_dir {"assets/"};
  const int CAPTURE_DELAY {100};
#endif
  const std::string m_emotion_detector_file {m_baze_dir + "best_model.tflite"};
  const std::string m_face_detector_graf {m_baze_dir + "deploy.prototxt"};
  const std::string m_face_detector_weights {m_baze_dir + "weights.caffemodel"};

  cv::dnn::Net m_face_detect_model;

  // Build the interpreter
  std::unique_ptr<tflite::FlatBufferModel> m_model;
  tflite::ops::builtin::BuiltinOpResolver resolver;
  std::unique_ptr<tflite::Interpreter> interpreter;
  TfLiteStatus m_status {};

  std::vector<std::string> img_to_emotion(const cv::Mat &frame);

  const int INPUT_WIDTH {224};
  const int INPUT_HEIGTH {224};
  const int INPUT_CANAL_COUNT {3};
  const size_t POINT_COUNT = {static_cast<size_t>(INPUT_WIDTH * INPUT_HEIGTH)};
  const size_t DATA_SIZE {static_cast<size_t>(POINT_COUNT * INPUT_CANAL_COUNT)};

  QRandomGenerator rand_gen{13494895};

  QScopedPointer<QCamera> m_camera;
  QVideoSink *m_video_sink{new QVideoSink{this}};
  QMediaCaptureSession m_captureSession;
  QImage m_curr_image;
  int m_id {-1};
  QTimer m_timer;
  cv::Mat m_frame{};
  std::vector<cv::Rect> m_face_coords;

//  void on_video_frame_changed(const QVideoFrame &frame);
  void on_capture_timer();
  void on_readyForCapture(bool ready);

  std::vector<cv::Rect> face_detect(const cv::Mat &frame);
//  QTextEdit *m_te;
  int64_t m_image_scale_time {0};
  int64_t m_i_to_m_time {0};
  int64_t m_face_detect_time {0};
  int m_img_height {0};
  int m_img_width {0};

//signals:


  // QObject interface
protected:
  virtual void paintEvent(QPaintEvent *event) override;

};

#endif // ACV_WIDGET_H
