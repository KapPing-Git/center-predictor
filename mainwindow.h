#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
  Q_OBJECT

public:
  MainWindow(QWidget *parent = nullptr);
  ~MainWindow();

private slots:

  void on_cameras_combo_box_currentIndexChanged(int);

  void on_increase_button_clicked();

  void on_decrease_button_clicked();

private:
  Ui::MainWindow *ui;

  void update_cameras();
};
#endif // MAINWINDOW_H
