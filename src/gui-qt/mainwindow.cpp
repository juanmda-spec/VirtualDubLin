#include "mainwindow.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle("VirtualDub - Linux Native (Qt)");
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

    outputVideoLabel = new QLabel("Video de Salida", this);
    outputVideoLabel->setStyleSheet("background-color: black; color: white;");
    outputVideoLabel->setAlignment(Qt::AlignCenter);

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
}

MainWindow::~MainWindow()
{
}

void MainWindow::openVideo()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Abrir Video"), "", tr("Archivos de Video (*.avi *.mp4 *.mkv *.mov);;Todos los archivos (*.*)"));
    if (!fileName.isEmpty()) {
        QMessageBox::information(this, "Archivo Seleccionado", "Has seleccionado:\n" + fileName + "\n\n(La integración con FFmpeg para abrir el archivo se implementará en la Fase 4).");
        qDebug() << "Archivo a abrir:" << fileName;
    }
}
