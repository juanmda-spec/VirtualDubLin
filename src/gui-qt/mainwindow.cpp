#include <QInputDialog>
#include <QProgressDialog>
#include <QCoreApplication>
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

    fileMenu->addSeparator();
    QAction *exportImgAct = new QAction(tr("Export &Image sequence..."), this);
    fileMenu->addAction(exportImgAct);
    connect(exportImgAct, &QAction::triggered, this, &MainWindow::exportImageSequence);
    fileMenu->addSeparator();
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
    connect(timelineSlider, &QSlider::sliderMoved, this, &MainWindow::sliderMoved);

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
        currentFile = fileName;
        if (decoder.openFile(fileName)) {
            qDebug() << "Archivo abierto con éxito:" << fileName;
            qDebug() << "Resolución:" << decoder.getVideoWidth() << "x" << decoder.getVideoHeight();

            // Iniciar filtro CUDA
            cudaFilter.init(decoder.getVideoWidth(), decoder.getVideoHeight());

            // Configurar Timeline Slider
            int totalFrames = decoder.getTotalFrames();
            timelineSlider->setRange(0, totalFrames > 0 ? totalFrames - 1 : 0);
            timelineSlider->setValue(0);

            // Mostrar primer frame
            updateFrame();
        } else {
            QMessageBox::critical(this, "Error", "No se pudo abrir el archivo de video con FFmpeg.");
        }
    }
}

void MainWindow::playVideo() {
    if (decoder.getVideoWidth() > 0) {
        if (decoder.getCurrentFrameIndex() >= decoder.getTotalFrames() - 1) {
            decoder.seekToFrame(0);
        }
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
        timelineSlider->setValue(decoder.getCurrentFrameIndex());

        // Mostrar Original
        QPixmap pixmapIn = QPixmap::fromImage(img).scaled(inputVideoLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
        inputVideoLabel->setPixmap(pixmapIn);

        // Procesar con CUDA (Opcional)
        QImage outImg = img.copy(); // Crear imagen de destino
        if (enableCudaFilter) {
            cudaFilter.processFrame(img.constBits(), outImg.bits());
        }

        // Mostrar Filtrado
        QPixmap pixmapOut = QPixmap::fromImage(outImg).scaled(outputVideoLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
        outputVideoLabel->setPixmap(pixmapOut);

    } else {
        // Fin del video o error
        stopVideo();
    }
}

#include <QInputDialog>
#include <QProgressDialog>
#include <QCoreApplication>

void MainWindow::exportImageSequence() {
    if (decoder.getVideoWidth() == 0 || currentFile.isEmpty()) {
        QMessageBox::warning(this, "Error", "No input video loaded.");
        return;
    }

    QString dir = QFileDialog::getExistingDirectory(this, tr("Select Destination Directory"), "", QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    if (dir.isEmpty()) return;

    bool ok;
    QString prefix = QInputDialog::getText(this, tr("Image Prefix"), tr("Enter filename prefix:"), QLineEdit::Normal, "frame_", &ok);
    if (!ok || prefix.isEmpty()) return;

    stopVideo();

    // Rewind video to start by reopening the decoder
    if (!decoder.openFile(currentFile)) {
        QMessageBox::critical(this, "Error", "Failed to reopen video for export.");
        return;
    }

    QImage img;
    int count = 0;
    int totalFrames = decoder.getTotalFrames();
    if (totalFrames <= 0) totalFrames = 0; // Unknown

    QProgressDialog progress("Exporting frames to PNG...", "Cancel", 0, totalFrames, this);
    progress.setWindowModality(Qt::WindowModal);

    while(decoder.decodeNextFrame(img)) {
        if (progress.wasCanceled()) break;

        QImage outImg = img.copy();
        if (enableCudaFilter) {
            cudaFilter.processFrame(img.constBits(), outImg.bits());
        }

        QString filename = QString("%1/%2%3.png").arg(dir).arg(prefix).arg(count, 5, 10, QChar('0'));
        outImg.save(filename, "PNG");

        count++;
        if (totalFrames > 0) progress.setValue(count);
        QCoreApplication::processEvents();
    }
    progress.setValue(totalFrames);

    QMessageBox::information(this, "Done", QString("Exported %1 images to %2").arg(count).arg(dir));
}

void MainWindow::sliderMoved(int position) {
    if (decoder.getVideoWidth() > 0) {
        decoder.seekToFrame(position);
        updateFrame();
    }
}
