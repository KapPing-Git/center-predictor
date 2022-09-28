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
  // настраиваем камеру
  set_camera(QMediaDevices::defaultVideoInput());
  m_captureSession.setVideoSink(m_video_sink);

  // загружаем модель детектирования лиц
  if (!QFile::exists(QString::fromStdString(m_face_detector_graf)) ||
      !QFile::exists(QString::fromStdString(m_face_detector_weights)) )
    return;
  m_face_detect_model = cv::dnn::readNetFromCaffe(m_face_detector_graf, m_face_detector_weights);

  // загружаем модель классификации эмоций
  connect(&m_timer, &QTimer::timeout, this, &ACV_Widget::on_capture_timer);
  m_model = tflite::FlatBufferModel::BuildFromFile(m_emotion_detector_file.data());
//  tflite::InterpreterBuilder(*m_model, resolver).AddDelegate();
  m_status = tflite::InterpreterBuilder(*m_model, resolver)(&interpreter);
  if (m_status == kTfLiteOk)
    {
      m_timer.start(2000);
    }
  else
    std::cout << "LoadSavedModel Failed: " << std::endl;
}

void ACV_Widget::set_camera(const QCameraDevice &cameraDevice)
{
  m_camera.reset(new QCamera(cameraDevice));
  m_captureSession.setCamera(m_camera.data());
  m_camera->start();
}

std::string ACV_Widget::img_to_emotion(const cv::Mat &frame)
{
  if (m_status == kTfLiteOk)
    {
      interpreter->AllocateTensors();
      float* input = interpreter->typed_input_tensor<float>(0); // TODO применить алгоритм многопоточного копирования
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
}

void ACV_Widget::on_capture_timer()
{
  QElapsedTimer timer;
  timer.start();
  QSize frame_size = m_video_sink->videoFrame().size();
  double devider = frame_size.width()*frame_size.height()/500000.0;
  if (devider < 1) devider = 1;
  m_img_width = frame_size.width()/devider;
  m_img_height = frame_size.height()/devider;
  m_curr_image = m_video_sink->videoFrame().toImage().scaled(m_img_width, m_img_height);
  m_image_scale_time = timer.elapsed();
  if (!m_curr_image.isNull())
    {
      // получаем фрейм с вебки
      timer.start();
      m_frame = QImage2Mat(m_curr_image);
      m_i_to_m_time = timer.elapsed();

      // находим лица
      m_face_coords = face_detect(m_frame);
//      if (!m_face_coords.empty())
        update();
    }
}

std::vector<Rect> ACV_Widget::face_detect(const cv::Mat &frame)
{
  std::vector<Rect> coords;

  QElapsedTimer timer;
  timer.start();
//  cv::Mat gray_scale_frame{};
//  timer.start();
//  cv::cvtColor(frame, gray_scale_frame, cv::COLOR_BGR2GRAY);

//  m_face_detector.detectMultiScale(gray_scale_frame, coords, 1.3, 5);
//  m_face_detect_time = timer.elapsed();

  //ssd
  auto prepared_frame = cv::dnn::blobFromImage(frame, 1.0, Size(300,300), Scalar(104.0, 177.0, 123.0));
  m_face_detect_model.setInput(prepared_frame);
  Mat output = m_face_detect_model.forward();
  const int SHIFT = 7;
  using currTp = Vec<float,SHIFT>;
  auto it = output.begin<currTp>();
  while(it != output.end<currTp>())
    {
//      cout << (*it)[2] << endl;
      currTp pred = *it;
      if (pred[2] < 0.5)
        break;

      int x = pred[3]*m_img_width;
      int y = pred[4]*m_img_height;
      int width = (pred[5] - pred[3])*m_img_width;
      int height = (pred[6] - pred[4])*m_img_height;
      coords.push_back(Rect{x, y, width, height});
      it+=SHIFT;
    }

  m_face_detect_time = timer.elapsed();
  return coords;
}

void ACV_Widget::paintEvent(QPaintEvent *event)
{
  QPainter painter(this);

  QElapsedTimer timer_all;
  timer_all.start();
  QElapsedTimer timer;
  if (!m_frame.empty())
    {
      // рисуем эмоции на фрейме
      int64_t resize_time {0};
      int64_t m_emo_time {0};
      for (const Rect &face_rect : m_face_coords)
        {
          // Вырезаем и преобразуем изображение
          timer.start();
          cv::Mat face_frame = m_frame(face_rect);
          cv::Mat face_frame_resized;
          cv::resize(face_frame, face_frame_resized, cv::Size(INPUT_WIDTH, INPUT_HEIGTH));
          resize_time = timer.elapsed();

          // предсказываем эмоцию
          timer.start();
          string emotion_name {img_to_emotion(face_frame_resized)};
          m_emo_time = timer.elapsed();
          // печатаем эмоцию на экране
          cv::putText(m_frame, emotion_name, Point(face_rect.x, face_rect.y-5), cv::FONT_HERSHEY_COMPLEX, 1.5, Scalar(0,255,0,0),2);
          cv::rectangle(m_frame, face_rect, Scalar(0,0,255,0), 2);
        }

      // рисуем фрейм на виджете
      timer.start();
      painter.drawImage(rect(), Mat2QImage(m_frame));
      painter.setBrush(Qt::white);
      painter.drawRect(10,40,180,400);
      int64_t draw_time = timer.elapsed();
      int add = 30;
      int y = 40;
      y += add; painter.drawText(10, y , QString::number(m_curr_image.width()) + "   " +
                                 QString::number(m_curr_image.height()) + "   ");

      painter.drawText(10, 10 , QString::number(m_curr_image.width()) + "   ");

      y += add; painter.drawText(10, y, "qimage " + QString::number(m_image_scale_time));
      y += add; painter.drawText(10, y, "m_i_to_m_time " + QString::number(m_i_to_m_time));
      y += add; painter.drawText(10, y, "m_face_detect_time " + QString::number(m_face_detect_time));
      y += add; painter.drawText(10, y, "resize_time " + QString::number(resize_time));
      y += add; painter.drawText(10, y, "m_emo_time " + QString::number(m_emo_time));
      y += add; painter.drawText(10, y, "draw_time " + QString::number(draw_time));
      y += add; painter.drawText(10, y, "painter " + QString::number(timer_all.elapsed()));
    }

  event->accept();
}
