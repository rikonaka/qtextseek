#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFileInfo>
#include <QStandardItemModel>
#include "filetraversethread.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_pushButton_folderSelect_clicked();
    void on_searchButton_clicked();
    void on_stopButton_clicked();
    /* my slots */
    void updateProgressFunc(QString file, bool skip);
    void updateFileInfoFunc(QStandardItemModel *model);
    void tableRowClicked(const QModelIndex &index);
    void stopSearchThread();

private:
    Ui::MainWindow *ui;
    QThread* searchThread;
    FileTraverseWorker *fileWorker;
};
#endif // MAINWINDOW_H
