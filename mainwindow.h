#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFileInfo>
#include <QStandardItemModel>

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
    void updateProgress(QString file);
    void updateFileInfo(QStandardItemModel *model);

private:
    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
