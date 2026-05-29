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
#include <QTimer>
#include "FFmpegDecoder.h"
#include "CudaFilter.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void openVideo();
    void updateFrame();
    void playVideo();
    void stopVideo();
    void exportImageSequence();

private:
    QLabel *inputVideoLabel;
    QLabel *outputVideoLabel;
    QSlider *timelineSlider;
    QPushButton *playButton;
    QPushButton *stopButton;

    FFmpegDecoder decoder;
    CudaFilter cudaFilter;
    QTimer *playbackTimer;
    bool isPlaying = false;
    QString currentFile;
};

#endif // MAINWINDOW_H
