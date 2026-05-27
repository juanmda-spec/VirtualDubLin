#include "mainwindow.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QDebug>
#include <QPixmap>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle("VirtualDub - Linux Native (Qt + FFmpeg)");
    resize(800, 600);

    // Menu
    QMenu *fileMenu = menuBar()->addMenu(tr("&Archivo"));
    QAction *openAct = new QAction(tr("&Abrir archivo de video..."), this);
    fileMenu->addAction(openAct);
    connect(openAct, &QAction::triggered, this, &MainWindow::openVideo);

    QAction *exitAct = new QAction(tr("&Salir"), this);
    fileMenu->addAction(exitAct);
    connect(exitAct, &QAction::triggered, this, &QWidget::close);

    // Central Widget
    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);

    // Video Panels
    QHBoxLayout *videoLayout = new QHBoxLayout();

    inputVideoLabel = new QLabel("Video de Entrada", this);
    inputVideoLabel->setStyleSheet("background-color: black; color: white;");
    inputVideoLabel->setAlignment(Qt::AlignCenter);
    inputVideoLabel->setMinimumSize(320, 240);

    outputVideoLabel = new QLabel("Video de Salida", this);
    outputVideoLabel->setStyleSheet("background-color: black; color: white;");
    outputVideoLabel->setAlignment(Qt::AlignCenter);
    outputVideoLabel->setMinimumSize(320, 240);

    videoLayout->addWidget(inputVideoLabel, 1);
    videoLayout->addWidget(outputVideoLabel, 1);

    mainLayout->addLayout(videoLayout, 1);

    // Timeline and controls
    QHBoxLayout *controlsLayout = new QHBoxLayout();

    playButton = new QPushButton("Play", this);
    stopButton = new QPushButton("Stop", this);
    timelineSlider = new QSlider(Qt::Horizontal, this);

    controlsLayout->addWidget(playButton);
    controlsLayout->addWidget(stopButton);
    controlsLayout->addWidget(timelineSlider);

    mainLayout->addLayout(controlsLayout);

    // Playback timer
    playbackTimer = new QTimer(this);
    connect(playbackTimer, &QTimer::timeout, this, &MainWindow::updateFrame);

    connect(playButton, &QPushButton::clicked, this, &MainWindow::playVideo);
    connect(stopButton, &QPushButton::clicked, this, &MainWindow::stopVideo);
}

MainWindow::~MainWindow()
{
}

void MainWindow::openVideo()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Abrir Video"), "", tr("Archivos de Video (*.avi *.mp4 *.mkv *.mov);;Todos los archivos (*.*)"));
    if (!fileName.isEmpty()) {
        if (decoder.openFile(fileName)) {
            qDebug() << "Archivo abierto con éxito:" << fileName;
            qDebug() << "Resolución:" << decoder.getVideoWidth() << "x" << decoder.getVideoHeight();

            // Mostrar primer frame
            updateFrame();
        } else {
            QMessageBox::critical(this, "Error", "No se pudo abrir el archivo de video con FFmpeg.");
        }
    }
}

void MainWindow::playVideo() {
    if (decoder.getVideoWidth() > 0) {
        isPlaying = true;
        playbackTimer->start(33); // Aprox 30fps
    }
}

void MainWindow::stopVideo() {
    isPlaying = false;
    playbackTimer->stop();
}

void MainWindow::updateFrame()
{
    QImage img;
    if (decoder.decodeNextFrame(img)) {
        // Redimensionar para encajar en el label manteniendo el aspecto
        QPixmap pixmap = QPixmap::fromImage(img).scaled(inputVideoLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
        inputVideoLabel->setPixmap(pixmap);
    } else {
        // Fin del video o error
        stopVideo();
    }
}
