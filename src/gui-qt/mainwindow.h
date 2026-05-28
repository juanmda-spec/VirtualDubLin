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
#include <QStatusBar>
#include <QToolBar>
#include "FFmpegDecoder.h"
#include "FFmpegEncoder.h"
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
    void showFiltersDialog();
    void showCompressionDialog();
    void exportVideo();

private:
    void createMenus();
    void createControls();

    // UI Elements
    QLabel *inputVideoLabel;
    QLabel *outputVideoLabel;
    QSlider *timelineSlider;
    QLabel *positionLabel;

    // Transport controls
    QPushButton *btnStop;
    QPushButton *btnPlayInput;
    QPushButton *btnPlayOutput;
    QPushButton *btnPrevFrame;
    QPushButton *btnNextFrame;
    QPushButton *btnPrevKeyframe;
    QPushButton *btnNextKeyframe;
    QPushButton *btnSceneRev;
    QPushButton *btnSceneFwd;
    QPushButton *btnMarkIn;
    QPushButton *btnMarkOut;

    // Menus
    QMenu *fileMenu;
    QMenu *editMenu;
    QMenu *viewMenu;
    QMenu *goMenu;
    QMenu *videoMenu;
    QMenu *audioMenu;
    QMenu *optionsMenu;
    QMenu *toolsMenu;
    QMenu *helpMenu;

    // Engine
    FFmpegDecoder decoder;
    FFmpegEncoder encoder;
    QString selectedCodec = "libx264";
    int selectedBitrate = 4000000;
    bool isExporting = false;
    CudaFilter cudaFilter;
    QTimer *playbackTimer;
    bool isPlaying = false;
};

#endif // MAINWINDOW_H
