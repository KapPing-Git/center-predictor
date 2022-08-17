#ifndef QT_CV_FUNCS_H
#define QT_CV_FUNCS_H

#include <QImage>
#include <opencv2/opencv.hpp>

QImage Mat2QImage(cv::Mat const& src);
cv::Mat QImage2Mat(QImage const& src);

#endif // QT_CV_FUNCS_H
