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
}

void ACV_Widget::set_camera(const QCameraDevice &cameraDevice)
{
  m_camera.reset(new QCamera(cameraDevice));
  m_captureSession.setCamera(m_camera.data());
  m_camera->start();
}

void ACV_Widget::predict_size_up()
{
  m_part_of_screen *= 1.25;
}

void ACV_Widget::predict_size_down()
{
  m_part_of_screen /= 1.25;
}

void ACV_Widget::start()
{
  // загружаем модель предсказания центра
  connect(&m_timer, &QTimer::timeout, this, &ACV_Widget::on_capture_timer);

  m_model = tflite::FlatBufferModel::BuildFromFile(m_center_predict_file.data());
  tflite::InterpreterBuilder builder(*m_model, m_resolver);
#ifdef Q_OS_ANDROID
  auto* delegate = TfLiteGpuDelegateV2Create(/*default options=*/nullptr);
  builder.AddDelegate(delegate);

  builder.AddDelegate(tflite::NnApiDelegate());
#endif

  m_status = builder(&m_interpreter);

  if (m_status == kTfLiteOk)
    {
      // получаем размерность входа
      const std::vector<int>& t_inputs = m_interpreter->inputs();
      TfLiteTensor* tensor = m_interpreter->tensor(t_inputs[0]);
      int batch = tensor->dims->data[0];
      m_input_width = tensor->dims->data[1];
      m_input_height = tensor->dims->data[2];
      m_canal_count = tensor->dims->data[3];
      m_point_count = m_input_width*m_input_height;
      m_data_size = m_point_count*m_canal_count;
      m_input_size = cv::Size{m_input_width, m_input_height};

      // получаем размерность выхода
      const std::vector<int>& t_outputs = m_interpreter->outputs();
      TfLiteTensor* out_tensor = m_interpreter->tensor(t_outputs[0]);
      m_output_width = out_tensor->dims->data[1];
      m_output_height = out_tensor->dims->data[2];
      m_output_size = cv::Size{m_output_width, m_output_height};

#ifndef Q_OS_ANDROID
      m_interpreter->SetNumThreads(16); // для desctop не используем аппаратное ускорение поэтому делаем много потоков
#endif
      // модель настроена на работу со строго 1 batch(для корректной работы android delegate)
      if (batch == 1)
        {
          m_interpreter->AllocateTensors();
          m_timer.start(CAPTURE_DELAY);
        }
      else
        {
          m_status = kTfLiteError;
          std::cout << "Размер батча в модели должен быть 1: " << std::endl;
        }
    }
  else
    std::cout << "LoadSavedModel Failed: " << std::endl;
}

cv::Mat ACV_Widget::predict_center(const cv::Mat &frame, Size output_size)
{
  if (m_status == kTfLiteOk)
    {
      // загружаем данные на входной слой
      float* input = m_interpreter->typed_input_tensor<float>(0);

      auto *from_data = (uint8_t*)frame.data;
      for(ulong i = 0; i < m_data_size; ++i)
        input[i] = float(from_data[i])/255;

      // делаем инференс
      auto status = m_interpreter->Invoke();

      // разбираем данные с выходного слоя
      float* output = m_interpreter->typed_output_tensor<float>(0);

      if (status == kTfLiteOk)
        {
          cv::Mat center(output_size, CV_8UC3);
          for(int i = 0; i < output_size.width*output_size.height*m_canal_count; ++i)
            {
              center.data[i] = output[i]*255.0;
            }
          return center;
        }
      else
        return cv::Mat{};
    }
  else
    return cv::Mat{};
}

// Получаем очередной фрейм от камеры, обрезаем его так чтобы соотношение сторон камеры и экрана были одинаковыми,
// скалируем и запоминаем после чего вызываем прорисовку окна
void ACV_Widget::on_capture_timer()
{
  QElapsedTimer timer;
  timer.start();
  // делаем одинаковым соотношение сторон
  double out_ratio = double(height())/double(width());
  int in_height = m_video_sink->videoFrame().height();
  int in_width = m_video_sink->videoFrame().width();
  double in_ratio = double(in_height)/double(in_width);
  int need_height = 0, need_width = 0;
  int cut_x, cut_y;
  // выбираем что обрезать ширину или высоту
  if (in_ratio > out_ratio)
    {
      //обрезаем высоту
      need_width = in_width;
      need_height = in_width*out_ratio;
      cut_x = 0;
      cut_y = (in_height - need_height)/2;
    }
  else
    {
      //обрезаем ширину
      need_height = in_height;
      need_width = in_height/out_ratio;
      cut_y = 0;
      cut_x = (in_width - need_width)/2;
    }

  // собственно обрезаем
  m_curr_image = m_video_sink->videoFrame().toImage().copy(cut_x, cut_y, need_width, need_height);
  // масштабируем
  m_curr_image = m_curr_image.scaled(size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

  m_timer.stop();
  if (!m_curr_image.isNull())
    {
      // получаем фрейм с вебки
      timer.start();
      m_frame = QImage2Mat(m_curr_image);

      //Запускаем перерисовку экрана, при этом будет происходить вызов paintEvent и предсказание центра
      update();
    }
  m_timer.start();
}

// в середине экрана заменяем область OUTPUT_SIZE пикселей предсказанной областью по области INPUT_SIZE вокруг центра
void ACV_Widget::paintEvent(QPaintEvent *event)
{
  QPainter painter(this);

  if (!m_frame.empty())
    {
      // Получаем координаты области по которой предсказываем центр (прямоугольник в центре экрана)
      int screen_frame_width = m_frame.cols * m_part_of_screen;
      double in_ratio = double(m_input_height) / double(m_input_width);
      int screen_frame_height = double(screen_frame_width) * in_ratio;
      int left = m_frame.cols/2 - screen_frame_width/2;
      int top = m_frame.rows/2 - screen_frame_height/2;
      cv::Rect in_rect_screen{left, top, screen_frame_width, screen_frame_height};
      cv::Mat in_screen_frame;
      m_frame(in_rect_screen).copyTo(in_screen_frame);

      // Получаем координаты предсказываемой области, того самого центра
      double kx = double(screen_frame_width) / double(m_input_width);
      double ky = double(screen_frame_height) / double(m_input_height);
      int screen_out_width = m_output_size.width*kx;
      int screen_out_height = m_output_size.height*ky;
      left = m_frame.cols/2 - screen_out_width/2;
      top = m_frame.rows/2 - screen_out_height/2;
      cv::Rect out_rect{left, top, screen_out_width, screen_out_height};

      // Вырезаем и преобразуем изображение
      cv::Mat in_frame;
      cv::resize(in_screen_frame, in_frame, m_input_size);
      int x = m_input_size.width/2 - m_output_size.width/2;
      in_frame(cv::Rect(x,x,m_output_size.width,m_output_size.width))=0;

      // предсказываем центер
      cv::Mat center = predict_center(in_frame, m_output_size);

      // обозначаем области на экране
      cv::rectangle(m_frame, in_rect_screen, Scalar(0,0,255,0), 2);
      cv::rectangle(m_frame, out_rect, Scalar(0,0,255,0), 2);

      // рисуем предсказанный фрейм
      painter.drawImage(rect(), Mat2QImage(m_frame));
      cv::Mat out_screen_frame;
      cv::resize(center, out_screen_frame, cv::Size(screen_frame_width, screen_frame_height));
      painter.drawImage(QRect(out_rect.x, out_rect.y, out_rect.width, out_rect.height), Mat2QImage(out_screen_frame));
    }

  event->accept();
}
