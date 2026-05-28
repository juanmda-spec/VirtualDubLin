#include "mainwindow.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QDebug>
#include <QPixmap>
#include <QInputDialog>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle("VirtualDub - Linux Native");
    resize(1024, 768);

    createMenus();

    // Central Widget
    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setContentsMargins(0,0,0,0);
    mainLayout->setSpacing(0);

    // Video Panels
    QWidget *videoContainer = new QWidget(this);
    QHBoxLayout *videoLayout = new QHBoxLayout(videoContainer);
    videoLayout->setContentsMargins(5,5,5,5);

    inputVideoLabel = new QLabel("Input Video", this);
    inputVideoLabel->setStyleSheet("background-color: #202020; color: #808080; border: 1px solid #404040;");
    inputVideoLabel->setAlignment(Qt::AlignCenter);
    inputVideoLabel->setMinimumSize(320, 240);

    outputVideoLabel = new QLabel("Output Video", this);
    outputVideoLabel->setStyleSheet("background-color: #202020; color: #808080; border: 1px solid #404040;");
    outputVideoLabel->setAlignment(Qt::AlignCenter);
    outputVideoLabel->setMinimumSize(320, 240);

    videoLayout->addWidget(inputVideoLabel, 1);
    videoLayout->addWidget(outputVideoLabel, 1);

    mainLayout->addWidget(videoContainer, 1);

    createControls();
    mainLayout->addWidget(timelineSlider);

    // Playback timer
    playbackTimer = new QTimer(this);
    connect(playbackTimer, &QTimer::timeout, this, &MainWindow::updateFrame);

    statusBar()->showMessage("Frame 0 (0:00:00.000) [K]   |   Video: Uncompressed RGB   |   Audio: No audio");
}

MainWindow::~MainWindow() {}

void MainWindow::createMenus() {
    fileMenu = menuBar()->addMenu(tr("&File"));
    QAction *openAct = new QAction(tr("&Open video file..."), this);
    openAct->setShortcut(QKeySequence::Open);
    fileMenu->addAction(openAct);
    connect(openAct, &QAction::triggered, this, &MainWindow::openVideo);

    fileMenu->addSeparator();
    QAction *exportAct = new QAction(tr("&Export video..."), this);
    fileMenu->addAction(exportAct);
    connect(exportAct, &QAction::triggered, this, &MainWindow::exportVideo);

    QAction *exitAct = new QAction(tr("E&xit"), this);
    fileMenu->addAction(exitAct);
    connect(exitAct, &QAction::triggered, this, &QWidget::close);

    editMenu = menuBar()->addMenu(tr("&Edit"));
    viewMenu = menuBar()->addMenu(tr("&View"));
    goMenu = menuBar()->addMenu(tr("&Go"));

    videoMenu = menuBar()->addMenu(tr("V&ideo"));
    QAction *filtersAct = new QAction(tr("&Filters..."), this);
    filtersAct->setShortcut(QKeySequence("Ctrl+F"));
    videoMenu->addAction(filtersAct);
    connect(filtersAct, &QAction::triggered, this, &MainWindow::showFiltersDialog);

    QAction *compressionAct = new QAction(tr("&Compression..."), this);
    compressionAct->setShortcut(QKeySequence("Ctrl+P"));
    videoMenu->addAction(compressionAct);
    connect(compressionAct, &QAction::triggered, this, &MainWindow::showCompressionDialog);

    audioMenu = menuBar()->addMenu(tr("&Audio"));
    optionsMenu = menuBar()->addMenu(tr("&Options"));
    toolsMenu = menuBar()->addMenu(tr("&Tools"));
    helpMenu = menuBar()->addMenu(tr("&Help"));
}

void MainWindow::createControls() {
    QToolBar *transportBar = new QToolBar(this);
    transportBar->setMovable(false);
    transportBar->setIconSize(QSize(24, 24));

    btnStop = new QPushButton("[]", this);
    btnPlayInput = new QPushButton(">I", this);
    btnPlayOutput = new QPushButton(">O", this);
    btnPrevFrame = new QPushButton("<|", this);
    btnNextFrame = new QPushButton("|>", this);
    btnPrevKeyframe = new QPushButton("<<", this);
    btnNextKeyframe = new QPushButton(">>", this);
    btnSceneRev = new QPushButton("<-", this);
    btnSceneFwd = new QPushButton("->", this);
    btnMarkIn = new QPushButton("[<", this);
    btnMarkOut = new QPushButton(">]", this);

    connect(btnStop, &QPushButton::clicked, this, &MainWindow::stopVideo);
    connect(btnPlayInput, &QPushButton::clicked, this, &MainWindow::playVideo);
    connect(btnPlayOutput, &QPushButton::clicked, this, &MainWindow::playVideo);

    transportBar->addWidget(btnStop);
    transportBar->addWidget(btnPlayInput);
    transportBar->addWidget(btnPlayOutput);
    transportBar->addSeparator();
    transportBar->addWidget(btnPrevFrame);
    transportBar->addWidget(btnNextFrame);
    transportBar->addWidget(btnPrevKeyframe);
    transportBar->addWidget(btnNextKeyframe);
    transportBar->addWidget(btnSceneRev);
    transportBar->addWidget(btnSceneFwd);
    transportBar->addSeparator();
    transportBar->addWidget(btnMarkIn);
    transportBar->addWidget(btnMarkOut);

    addToolBar(Qt::BottomToolBarArea, transportBar);

    timelineSlider = new QSlider(Qt::Horizontal, this);
    timelineSlider->setMinimumHeight(24);

    positionLabel = new QLabel("Frame 0", this);
    positionLabel->setMinimumWidth(80);
    positionLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

    QToolBar *posBar = new QToolBar(this);
    posBar->setMovable(false);
    posBar->addWidget(timelineSlider);
    posBar->addWidget(positionLabel);
    addToolBar(Qt::BottomToolBarArea, posBar);

}

void MainWindow::openVideo()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open Video File"), "", tr("Video Files (*.avi *.mp4 *.mkv *.mov);;All Files (*.*)"));
    if (!fileName.isEmpty()) {
        if (decoder.openFile(fileName)) {
            qDebug() << "File opened successfully:" << fileName;
            cudaFilter.init(decoder.getVideoWidth(), decoder.getVideoHeight());
            updateFrame();
            statusBar()->showMessage(QString("Opened %1 (%2x%3)").arg(fileName).arg(decoder.getVideoWidth()).arg(decoder.getVideoHeight()));
        } else {
            QMessageBox::critical(this, "Error", "Failed to open video file via FFmpeg.");
        }
    }
}

void MainWindow::playVideo() {
    if (decoder.getVideoWidth() > 0) {
        isPlaying = true;
        playbackTimer->start(33); // Aprox 30fps
        statusBar()->showMessage("Playing...");
    }
}

void MainWindow::stopVideo() {
    isPlaying = false;
    playbackTimer->stop();
    statusBar()->showMessage("Stopped.");
}

void MainWindow::updateFrame()
{
    QImage img;
    if (decoder.decodeNextFrame(img)) {
        // Output Original
        QPixmap pixmapIn = QPixmap::fromImage(img).scaled(inputVideoLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
        inputVideoLabel->setPixmap(pixmapIn);

        // Process with CUDA
        QImage outImg = img.copy();
        cudaFilter.processFrame(img.constBits(), outImg.bits());

        // Output Filtered
        QPixmap pixmapOut = QPixmap::fromImage(outImg).scaled(outputVideoLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
        outputVideoLabel->setPixmap(pixmapOut);
    } else {
        stopVideo();
    }
}

void MainWindow::showFiltersDialog() {
    QMessageBox::information(this, "Filters", "Filter chain configuration dialog will be implemented natively.");
}

void MainWindow::showCompressionDialog() {
    QStringList codecs = {"Uncompressed", "libx264", "libx265", "hevc_nvenc"};
    bool ok;
    QString item = QInputDialog::getItem(this, "Video Compression", "Select video codec:", codecs, 0, false, &ok);
    if (ok && !item.isEmpty()) {
        selectedCodec = item;
        selectedBitrate = QInputDialog::getInt(this, "Bitrate", "Enter bitrate (bps):", 4000000, 100000, 50000000, 100000, &ok);
        if (ok) {
            QMessageBox::information(this, "Settings Saved", "Codec: " + selectedCodec + "\nBitrate: " + QString::number(selectedBitrate) + " bps");
        }
    }
}

void MainWindow::exportVideo() {
    if (decoder.getVideoWidth() == 0) {
        QMessageBox::warning(this, "Error", "No input video loaded.");
        return;
    }

    QString saveName = QFileDialog::getSaveFileName(this, "Save Video", "", "Video (*.mp4 *.mkv)");
    if (saveName.isEmpty()) return;

    if (!encoder.init(saveName, decoder.getVideoWidth(), decoder.getVideoHeight(), selectedCodec, selectedBitrate)) {
        QMessageBox::critical(this, "Encoder Error", "Failed to initialize encoder with codec: " + selectedCodec);
        return;
    }

    isExporting = true;
    statusBar()->showMessage("Exporting...");

    // Export loop handling Audio and Video
    QImage img;
    uint8_t audioBuf[8192];
    int audioSize = 0;

    while(true) {
        bool hasVideo = decoder.decodeNextFrame(img);
        bool hasAudio = decoder.decodeNextAudioFrame(audioBuf, audioSize);

        if (!hasVideo && !hasAudio) break; // EOF

        if (hasVideo) {
            QImage outImg = img.copy();
            cudaFilter.processFrame(img.constBits(), outImg.bits());
            outImg = outImg.convertToFormat(QImage::Format_RGB32);
            encoder.encodeFrame(outImg);
        }

        if (hasAudio && audioSize > 0) {
            // A/V Sync would be maintained by PTS interleaving inside FFmpegEncoder
            encoder.encodeAudioFrame(audioBuf, audioSize);
        }
    }

    encoder.close();
    isExporting = false;
    statusBar()->showMessage("Export complete!");
    QMessageBox::information(this, "Done", "Video exported successfully to: " + saveName);
}
