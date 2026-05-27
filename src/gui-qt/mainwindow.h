#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLabel>
#include <QMenu>
#include <QMenuBar>
#include <QAction>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QSlider>

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void openVideo();

private:
    QLabel *inputVideoLabel;
    QLabel *outputVideoLabel;
    QSlider *timelineSlider;
    QPushButton *playButton;
    QPushButton *stopButton;
};

#endif // MAINWINDOW_H
