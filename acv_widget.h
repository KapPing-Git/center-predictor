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

#include <QMediaRecorder>
#include <QVideoWidget>
#include <QCameraDevice>
#include <QMediaMetaData>
#include <QMediaDevices>
#include <QAudioDevice>
#include <QAudioInput>
#include <QVideoSink>

#include <QMessageBox>
#include <QPalette>
#include <QImage>

#include <QtWidgets>
#include <QMediaDevices>
#include <QMediaFormat>


#include <opencv2/opencv.hpp>


#include "tensorflow/lite/interpreter.h"
#include "tensorflow/lite/kernels/register.h"
#include "tensorflow/lite/model.h"
#include "tensorflow/lite/optional_debug_tools.h"
#include "tensorflow/lite/c/c_api_types.h"

class ACV_Widget : public QWidget
{
  Q_OBJECT
public:
  explicit ACV_Widget(QWidget *parent = nullptr);

private:
  cv::VideoCapture m_cam {};
#ifdef Q_OS_ANDROID
  const std::string m_face_detector_qml {(QDir::homePath() + QDir::separator() + "haarcascade_frontalface_default.xml").toStdString()};
  const std::string m_emotion_detector_file {(QDir::homePath() + QDir::separator() + "best_model.tflite").toStdString()};
#else
  const std::string m_face_detector_qml {"/home/kap/source_code/python/sample/diplom/haarcascade_frontalface_default.xml"};
  const std::string m_emotion_detector_file {"/home/kap/source_code/II/diplom_computer_vision_tf_lite/best_model.tflite"};
#endif

  cv::CascadeClassifier m_face_detector{m_face_detector_qml};

  const char *filename {"/home/kap/Загрузки/best_model.tflite"};
  std::unique_ptr<tflite::FlatBufferModel> m_model;

  // Build the interpreter
  tflite::ops::builtin::BuiltinOpResolver resolver;
  std::unique_ptr<tflite::Interpreter> interpreter;
  TfLiteStatus m_status {};

  std::string img_to_emotion(const cv::Mat &frame);

  const int INPUT_WIDTH {224};
  const int INPUT_HEIGTH {224};
  const int INPUT_CANAL_COUNT {3};
  const size_t POINT_COUNT = {static_cast<size_t>(INPUT_WIDTH * INPUT_HEIGTH)};
  const size_t DATA_SIZE {static_cast<size_t>(POINT_COUNT * INPUT_CANAL_COUNT)};

  QRandomGenerator rand_gen{13494895};

  QScopedPointer<QCamera> m_camera;
  QImageCapture *m_imageCapture;
  QMediaCaptureSession m_captureSession;
  QImage m_curr_image;
  int m_id {-1};
  QTimer m_timer;

  void on_imageCaptured(int id, const QImage &img);
  void on_errorOccurred(int id, QImageCapture::Error error, const QString &errorString);
  void on_capture_timer();
  void on_readyForCapture(bool ready);
  QTextEdit *m_te;

//signals:


  // QObject interface
protected:
  virtual void paintEvent(QPaintEvent *event) override;

};

#endif // ACV_WIDGET_H
