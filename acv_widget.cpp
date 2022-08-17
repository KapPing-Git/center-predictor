#include "acv_widget.h"

#include <QPainter>
#include <QImage>
#include <QTimerEvent>
#include <QPaintEvent>

#include "qt_cv_funcs.h"

using namespace cv;
using namespace std;

ACV_Widget::ACV_Widget(QWidget *parent)
  : QWidget{parent}
{
//  if (m_cam.open(0))
//    {
//      m_cam.set(CAP_PROP_FRAME_WIDTH, 800);
//      m_cam.set(CAP_PROP_FRAME_HEIGHT, 600);
////      m_status = tflite::InterpreterBuilder(*m_model, resolver)(&interpreter);
////      if (m_status == kTfLiteOk)
////        {
////          startTimer(70);
////        }
////      else
////        std::cout << "LoadSavedModel Failed: " << std::endl;
////      startTimer(70); // TODO убрать

//    }
//  else
//    cerr << "cam is not opened error" << endl;
////  startTimer(70);

  m_camera.reset(new QCamera(QMediaDevices::defaultVideoInput()));
  m_captureSession.setCamera(m_camera.data());
  m_imageCapture = new QImageCapture;
  connect(m_imageCapture, &QImageCapture::imageCaptured, this, &ACV_Widget::on_imageCaptured);
  connect(m_imageCapture, &QImageCapture::errorOccurred, this, &ACV_Widget::on_errorOccurred);
//  connect(m_imageCapture, &QImageCapture::imageAvailable, this, &ACV_Widget::on_imageAvailable);
  m_captureSession.setImageCapture(m_imageCapture);
//  m_imageCapture->setQuality(QImageCapture::VeryLowQuality);
  m_imageCapture->setResolution(480, 640);
//  auto v = new QVideoWidget(this);
//  v->setGeometry(10,10,400,400);
//  v->show();
//  m_captureSession.setVideoOutput(v);
  auto empty_view = new QVideoWidget;
  m_captureSession.setVideoOutput(empty_view);
//  m_video_sink = new QVideoSink;
//  m_captureSession.setVideoSink(m_video_sink);
//  connect(m_video_sink, &QVideoSink::videoFrameChanged, this, &ACV_Widget::on_videoFrameChanged);
  m_camera->start();
//  m_imageCapture->capture();
//  m_te = new QTextEdit(this);
//  m_te->setGeometry(20,20,200,20);
//  m_te->show();

  connect(&m_timer, &QTimer::timeout, this, &ACV_Widget::on_capture_timer);
  connect(m_imageCapture, &QImageCapture::readyForCaptureChanged, this, &ACV_Widget::on_readyForCapture);
//  m_timer.start(200);
  m_model = tflite::FlatBufferModel::BuildFromFile(m_emotion_detector_file.data());
  auto bilder = tflite::InterpreterBuilder(*m_model, resolver);
  m_status = bilder(&interpreter);
//  m_status = tflite::InterpreterBuilder(*m_model, resolver)(&interpreter);
  if (m_status == kTfLiteOk)
    {
      m_timer.start(400);
    }
  else
    std::cout << "LoadSavedModel Failed: " << std::endl;
}

std::string ACV_Widget::img_to_emotion(const cv::Mat &frame)
{
  if (m_status == kTfLiteOk)
    {
      interpreter->AllocateTensors();
      float* input = interpreter->typed_input_tensor<float>(0);
      auto *from_data = (uint8_t*)frame.data;
      for (size_t i = 0;i < DATA_SIZE; ++i)
        {
          input[i] = float(from_data[i]);
        }

      auto status = interpreter->Invoke();

      float* output = interpreter->typed_output_tensor<float>(0);
      if (status == kTfLiteOk)
        {
          auto size = 9;
          int max_idx {0};
          float max = output[0];
//          static const vector<string> emo_names = {"anger", "contempt", "disgust", "fear", "happy", "neutral", "sad",
//                                            "surprise", "uncertain"};
          static const vector<string> emo_names = {"злость", "презрение", "отвращение", "страх", "радость", "норма", "печаль",
                                            "удивление", "неуверенность"};
          string emotions;
          for (int i = 0; i < size; ++i)
            {
              float curr_val = output[i];
              if (curr_val > 0.2)
                emotions.append(emo_names[i] + " ");
              if (curr_val > max)
                {
                  max_idx = i;
                  max = curr_val;
                }
//              cout << curr_val << endl;
            }
//          cout << endl;


//          return emo_names[max_idx];
          return emotions;
        }
      else
        return "predict error";
    }
  else
    return "Model doesn`t exist";
//  return "Эмоция";
}


void ACV_Widget::on_imageCaptured(int id, const QImage &img)
{
  m_curr_image = img.scaled(QSize(400, 800),
                            Qt::KeepAspectRatio,
                            Qt::SmoothTransformation);
  m_curr_image = img;
  m_id = id;
  update();
}
void ACV_Widget::on_errorOccurred(int id, QImageCapture::Error error, const QString &errorString)
{
  m_te->setText(errorString + QString::number(id));
}

void ACV_Widget::on_capture_timer()
{
  if (m_imageCapture->isReadyForCapture())
    m_imageCapture->capture();
}

void ACV_Widget::on_readyForCapture(bool ready)
{
//  m_te->setText(ready ? "ready" : "not");
//  if (m_imageCapture->isReadyForCapture())
//    m_imageCapture->capture();
//  m_timer.stop();
}




void ACV_Widget::paintEvent(QPaintEvent *event)
{
  QPainter painter(this);

//  if (m_cam.isOpened())
  if (!m_curr_image.isNull())
    {
      const int DIMS = 3;
      // получаем фрейм с вебки
      cv::Mat frame{};
//      m_cam >> frame;
      frame = QImage2Mat(m_curr_image);

      // находим лица
      vector<Rect> face_coords;
      cv::Mat gray_scale_frame{};
      cv::cvtColor(frame, gray_scale_frame, cv::COLOR_BGR2GRAY);
      m_face_detector.detectMultiScale(gray_scale_frame, face_coords, 1.3, 5);

      // рисуем эмоции на фрейме
//      cv::Mat face_frame_resized;
      for (const Rect &face_rect : face_coords)
        {
          // Вырезаем и преобразуем изображение
          cv::Mat face_frame = frame(face_rect);
          cv::Mat face_frame_resized;
          cv::resize(face_frame, face_frame_resized, cv::Size(INPUT_WIDTH, INPUT_HEIGTH));

          // предсказываем эмоцию
          string emotion_name {img_to_emotion(face_frame_resized)};

          // печатаем эмоцию на экране
          cv::putText(frame, emotion_name, Point(face_rect.x, face_rect.y-5), cv::FONT_HERSHEY_COMPLEX, 1.5, Scalar(0,255,0,0),2);
          cv::rectangle(frame, face_rect, Scalar(0,0,255,0), 2);
        }

      // рисуем фрейм на виджете
//      frame = gray_scale_frame;
//      int num_point {0};
//      QImage image(frame.cols, frame.rows, QImage::Format_RGB32);
//      for (auto it {frame.begin<Vec<uint8_t, DIMS>>()}; it != frame.end<Vec<uint8_t, DIMS>>(); ++it)
//        {
//          int y = num_point / frame.cols;
//          int x = num_point % frame.cols;
//          auto vec_color = *it;

//          // преобразуем точку в формат для image.setPixel
//          uint color {255};
//          color = color << 8;
//          color = color | vec_color[2];
//          color = color << 8;
//          color = color | vec_color[1];
//          color = color << 8;
//          color = color | vec_color[0];

//          image.setPixel(x, y, color);
//          num_point++;
//        }
//      painter.drawImage(rect(), image);

      painter.drawImage(rect(), Mat2QImage(frame));
//      painter.drawImage(rect(), Mat2QImage(face_frame_resized));

    }

  event->accept();
}
