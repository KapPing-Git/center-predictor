QT       += core gui

#greaterThan(QT_MAJOR_VERSION, 4): QT += widgets androidextras multimedia multimediawidgets
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets multimedia multimediawidgets

CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    acv_widget.cpp \
    main.cpp \
    mainwindow.cpp \
    qt_cv_funcs.cpp

HEADERS += \
    acv_widget.h \
    mainwindow.h \
    qt_cv_funcs.h

FORMS += \
    mainwindow.ui

TRANSLATIONS += \
    diplom_computer_vision_tf_lite_ru_RU.ts
CONFIG += lrelease
CONFIG += embed_translations

!android {
    INCLUDEPATH += /home/kap/compiled_libs/desktop/include/opencv4
    LIBS += -L/home/kap/compiled_libs/desktop/lib \
#    -lopencv_stitching \
#    -lopencv_aruco \
#    -lopencv_bgsegm \
#    -lopencv_bioinspired \
#    -lopencv_ccalib \
#    -lopencv_dnn_objdetect \
#    -lopencv_dnn_superres \
#    -lopencv_dpm \
#    -lopencv_highgui \
#    -lopencv_face \
#    -lopencv_fuzzy \
#    -lopencv_hfs \
#    -lopencv_img_hash \
#    -lopencv_line_descriptor \
#    -lopencv_quality \
#    -lopencv_reg \
#    -lopencv_rgbd \
#    -lopencv_saliency \
#    -lopencv_shape \
#    -lopencv_stereo \
#    -lopencv_structured_light \
#    -lopencv_phase_unwrapping \
#    -lopencv_superres \
#    -lopencv_optflow \
#    -lopencv_surface_matching \
#    -lopencv_tracking \
#    -lopencv_datasets \
#    -lopencv_text \
    -lopencv_dnn \
#    -lopencv_plot \
#    -lopencv_ml \
#    -lopencv_videostab \
    -lopencv_videoio \
#    -lopencv_ximgproc \
#    -lopencv_video \
#    -lopencv_xobjdetect \
    -lopencv_objdetect \
    -lopencv_calib3d \
    -lopencv_imgcodecs \
    -lopencv_features2d \
    -lopencv_flann \
#    -lopencv_xphoto \
#    -lopencv_photo \
    -lopencv_imgproc \
    -lopencv_core

    INCLUDEPATH += /home/kap/source_code/tensorflow/tensorflow
    INCLUDEPATH += /home/kap/source_code/tensorflow/tensorflow/bazel-bin/
    INCLUDEPATH += /home/kap/source_code/tensorflow/tensorflow/bazel-tensorflow/external

    LIBS += -L/home/kap/source_code/tensorflow/tensorflow/bazel-bin/tensorflow/lite -ltensorflowlite
}

#OPENCV_ANDROID = /home/kap/Android/OpenCV-android-sdk-4.5.2
#OPENCV_ANDROID = /home/kap/Android/OpenCV-android-sdk
OPENCV_ANDROID = /home/kap/compiled_libs/android
#include($$OPENCV_ANDROID/sdk/native/jni/OpenCV.mk)
android {
    include(/home/kap/Android/Sdk/android_openssl/openssl.pri)
#    contains(ANDROID_TARGET_ARCH,armeabi-v7a) {
#        isEmpty(OPENCV_ANDROID) {
#            error("Let OPENCV_ANDROID point to the opencv-android-sdk, recommended: v4.0")
#        }
#error($$ANDROID_TARGET_ARCH)
        INCLUDEPATH += "$$OPENCV_ANDROID/sdk/native/jni/include"
#        LIBS += -L/home/kap/Android/Sdk/ndk/21.4.7075529/toolchains/llvm/prebuilt/linux-x86_64/sysroot/usr/lib/arm-linux-androideabi/30/ -lmediandk
#        LIBS += -lmediandk
        LIBS += \
            -L"$$OPENCV_ANDROID/sdk/native/libs/armeabi-v7a" \
            -L"$$OPENCV_ANDROID/sdk/native/staticlibs/armeabi-v7a" \
            -L"$$OPENCV_ANDROID/sdk/native/3rdparty/libs/armeabi-v7a" \
            -lade \
            -ltbb \
            -littnotify \
            -llibjpeg-turbo \
            -llibwebp \
            -llibpng \
            -llibtiff \
            -llibopenjp2 \
            -lIlmImf \
            -lquirc \
            -ltegra_hal \
            -lopencv_dnn \
            -lopencv_objdetect \
            -lopencv_calib3d \
            -lopencv_imgcodecs \
            -lopencv_features2d \
            -lopencv_flann \
            -lopencv_imgproc \
            -lopencv_core \
            -lopencv_videoio \
            -lcpufeatures \
            -llibprotobuf \
#            -lc++_shared \

        ANDROID_EXTRA_LIBS = $$OPENCV_ANDROID/sdk/native/libs/armeabi-v7a/libopencv_java4.so
        ANDROID_EXTRA_LIBS = /home/kap/source_code/tensorflow/tensorflow_arm/bazel-bin/tensorflow/lite/libtensorflowlite.so
        ANDROID_PACKAGE_SOURCE_DIR=$$_PRO_FILE_PWD_/android

        INCLUDEPATH += /home/kap/source_code/tensorflow/tensorflow_arm
        INCLUDEPATH += /home/kap/source_code/tensorflow/tensorflow_arm/bazel-bin/
        INCLUDEPATH += /home/kap/source_code/tensorflow/tensorflow_arm/bazel-tensorflow/external
#        INCLUDEPATH += /usr/local/include
        INCLUDEPATH += /home/kap/source_code/tensorflow/tensorflow_arm/bazel-bin/external/flatbuffers/_virtual_includes/flatbuffers

        LIBS += -L/home/kap/source_code/tensorflow/tensorflow_arm/bazel-bin/tensorflow/lite -ltensorflowlite
#    }
}

DISTFILES += \
    android/AndroidManifest.xml \
    android/build.gradle \
    android/gradle.properties \
    android/gradle/wrapper/gradle-wrapper.jar \
    android/gradle/wrapper/gradle-wrapper.properties \
    android/gradlew \
    android/gradlew.bat \
    android/res/haarcascade_frontalface_default.xml \
    android/res/values/libs.xml

contains(ANDROID_TARGET_ARCH,armeabi-v7a) {
    ANDROID_PACKAGE_SOURCE_DIR = \
        $$PWD/android
}

RESOURCES += \
    res.qrc

contains(ANDROID_TARGET_ARCH,arm64-v8a) {
    ANDROID_PACKAGE_SOURCE_DIR = \
        $$PWD/android

}
