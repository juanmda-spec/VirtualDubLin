#include <QInputDialog>
#include <QStatusBar>
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

    QMenu *videoMenu = menuBar()->addMenu(tr("&Video"));
    directStreamCopyAct = new QAction(tr("&Direct stream copy"), this);
    directStreamCopyAct->setCheckable(true);
    videoMenu->addAction(directStreamCopyAct);
    connect(directStreamCopyAct, &QAction::triggered, this, &MainWindow::setDirectStreamCopy);

    fullProcessingModeAct = new QAction(tr("&Full processing mode"), this);
    fullProcessingModeAct->setCheckable(true);
    fullProcessingModeAct->setChecked(true); // Default
    videoMenu->addAction(fullProcessingModeAct);
    connect(fullProcessingModeAct, &QAction::triggered, this, &MainWindow::setFullProcessingMode);

    QMenu *fileMenu = menuBar()->addMenu(tr("&Archivo"));
    QAction *openAct = new QAction(tr("&Abrir archivo de video..."), this);
    fileMenu->addAction(openAct);
    connect(openAct, &QAction::triggered, this, &MainWindow::openVideo);


    QAction *exportImgAct = new QAction(tr("Export &Image sequence..."), this);
    fileMenu->addAction(exportImgAct);
    connect(exportImgAct, &QAction::triggered, this, &MainWindow::exportImageSequence);

    fileMenu->addSeparator();

    QAction *queueAct = new QAction(tr("Save as AVI... (Queue job)"), this);
    fileMenu->addAction(queueAct);
    connect(queueAct, &QAction::triggered, this, &MainWindow::queueJob);

    QAction *jobControlAct = new QAction(tr("&Job Control..."), this);
    jobControlAct->setShortcut(QKeySequence("F4"));
    fileMenu->addAction(jobControlAct);
    connect(jobControlAct, &QAction::triggered, this, &MainWindow::showJobControl);

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
    jobControl = new JobControlDialog(this);
    connect(jobControl, &JobControlDialog::startJobs, this, &MainWindow::runJobs);
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
        // Mostrar Original
        QPixmap pixmapIn = QPixmap::fromImage(img).scaled(inputVideoLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
        inputVideoLabel->setPixmap(pixmapIn);

        // Procesar con CUDA
        QImage outImg = img.copy(); // Crear imagen de destino
        cudaFilter.processFrame(img.constBits(), outImg.bits());

        // Mostrar Filtrado
        QPixmap pixmapOut = QPixmap::fromImage(outImg).scaled(outputVideoLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
        outputVideoLabel->setPixmap(pixmapOut);

    } else {
        // Fin del video o error
        stopVideo();
    }
}

void MainWindow::exportImageSequence() {
    if (decoder.getVideoWidth() == 0) {
        QMessageBox::warning(this, "Error", "No input video loaded.");
        return;
    }

    QString dir = QFileDialog::getExistingDirectory(this, tr("Select Destination Directory"), "", QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    if (dir.isEmpty()) return;

    bool ok;
    QString prefix = QInputDialog::getText(this, tr("Image Prefix"), tr("Enter filename prefix:"), QLineEdit::Normal, "frame_", &ok);
    if (!ok || prefix.isEmpty()) return;

    isPlaying = false;
    if (playbackTimer) playbackTimer->stop();

    QImage img;
    int count = 0;
    while(decoder.decodeNextFrame(img)) {
        QString filename = QString("%1/%2%3.png").arg(dir).arg(prefix).arg(count, 5, 10, QChar('0'));
        img.save(filename, "PNG");
        count++;
    }

    QMessageBox::information(this, "Done", QString("Exported %1 images to %2").arg(count).arg(dir));
}

void MainWindow::setDirectStreamCopy() {
    directStreamCopy = true;
    directStreamCopyAct->setChecked(true);
    fullProcessingModeAct->setChecked(false);
}

void MainWindow::setFullProcessingMode() {
    directStreamCopy = false;
    directStreamCopyAct->setChecked(false);
    fullProcessingModeAct->setChecked(true);
}

void MainWindow::queueJob() {
    if (currentFile.isEmpty()) {
        QMessageBox::warning(this, "Error", "No input video loaded.");
        return;
    }
    QString saveName = QFileDialog::getSaveFileName(this, "Queue Video", "", "Video (*.mp4 *.mkv)");
    if (!saveName.isEmpty()) {
        jobControl->addJob(currentFile, saveName, directStreamCopy);
        QMessageBox::information(this, "Job Queued", "Job added to queue.");
    }
}

void MainWindow::showJobControl() {
    jobControl->show();
}

void MainWindow::runJobs() {
    QMessageBox::information(this, "Batch Processing", "Processing jobs... (MVP simulation)");
}
