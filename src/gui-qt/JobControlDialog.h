#ifndef JOBCONTROLDIALOG_H
#define JOBCONTROLDIALOG_H

#include <QDialog>
#include <QListWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>

class JobControlDialog : public QDialog {
    Q_OBJECT
public:
    JobControlDialog(QWidget* parent = nullptr);
    void addJob(const QString& source, const QString& dest, bool isDirectStream);

signals:
    void startJobs();

private slots:
    void onStartClicked();

private:
    QListWidget* jobList;
    QPushButton* startButton;
    QPushButton* closeButton;
};

#endif
