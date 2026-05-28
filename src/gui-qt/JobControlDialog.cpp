#include "JobControlDialog.h"

JobControlDialog::JobControlDialog(QWidget* parent) : QDialog(parent) {
    setWindowTitle("VirtualDub - Job Control");
    resize(400, 300);

    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    jobList = new QListWidget(this);
    mainLayout->addWidget(jobList);

    QHBoxLayout* btnLayout = new QHBoxLayout();
    startButton = new QPushButton("Start", this);
    closeButton = new QPushButton("Close", this);

    btnLayout->addWidget(startButton);
    btnLayout->addWidget(closeButton);
    mainLayout->addLayout(btnLayout);

    connect(startButton, &QPushButton::clicked, this, &JobControlDialog::onStartClicked);
    connect(closeButton, &QPushButton::clicked, this, &QWidget::hide);
}

void JobControlDialog::addJob(const QString& source, const QString& dest, bool isDirectStream) {
    QString jobDesc = QString("Source: %1 -> Dest: %2 [%3]").arg(source).arg(dest).arg(isDirectStream ? "Direct Copy" : "Full Processing");
    jobList->addItem(jobDesc);
}

void JobControlDialog::onStartClicked() {
    emit startJobs();
    jobList->clear();
}
