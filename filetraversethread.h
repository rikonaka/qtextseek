#ifndef FILETRAVERSETHREAD_H
#define FILETRAVERSETHREAD_H

#include <QDir>
#include <QStandardItemModel>
#include <QRegularExpression>

class FileTraverseWorker : public QObject
{
    Q_OBJECT

public:
    bool stopRequested;
    explicit FileTraverseWorker(const QString &dirPath, const QString &target, const QList<QString> &extensions)
        : dirPath(dirPath), target(target), extensions(extensions) {}

    void run() {
        stopRequested = false;

        QStandardItemModel *model = new QStandardItemModel;
        model->setHorizontalHeaderLabels({tr("name"), tr("size"), tr("path"), "target"});
        emit updateFileInfo(model);
        dataModel = model;

        FileTraverseWorker::traverse(dirPath, target, extensions);
    }

signals:
    void updateProgress(QString file, int status);
    void updateProgressStoped();
    void updateFileInfo(QStandardItemModel *model);


private:
    QString dirPath;
    QString target;
    QList<QString> extensions;
    QStandardItemModel *dataModel;
    void traverse(const QString &dirPath, const QString &target, const QList<QString> &extensions)
    {
        QDir dir(dirPath);
        QFileInfoList files = dir.entryInfoList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot | QDir::Readable);

        for (QFileInfo &fileInfo : files) {
            if (!stopRequested) {
                if (fileInfo.isDir())
                    FileTraverseWorker::traverse(fileInfo.absoluteFilePath(), target, extensions);
                else {
                    if (fileInfo.isFile()) {
                        QString fileName = fileInfo.fileName();
                        QStringList words = fileName.split(".");
                        if (words.length() >1) {
                            QString ext = words.last();
                            ext = ext.trimmed();
                            if (extensions.contains(ext)) {
                                QString filePath = fileInfo.absoluteFilePath();
                                bool status = FileTraverseWorker::checkFileContent(filePath, target);
                                if (status) {
                                    QString fileName = fileInfo.fileName();
                                    QString fileSize = QString::number(fileInfo.size()) + " bytes";
                                    QList<QStandardItem *> newRow;
                                    newRow.append(new QStandardItem(fileName));
                                    newRow.append(new QStandardItem(fileSize));
                                    newRow.append(new QStandardItem(filePath));
                                    newRow.append(new QStandardItem(target));
                                    dataModel->appendRow(newRow);

                                    emit updateProgress(fileName, 0);
                                } else // not found target string in file
                                    emit updateProgress(fileName, 2);
                            } else
                               emit updateProgress(fileName, 1);
                        } else
                            emit updateProgress(fileName, 1);
                    }
                }
            } else
                emit updateProgressStoped();
        }
    }
    bool checkFileContent(const QString &filePath, const QString &target) {
        QFile file(filePath);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QString msg = "Can not open file: ";
            msg += filePath;
            qDebug() << msg;
            return false;
        }

        QTextStream in(&file);
        QString content;
        while (!in.atEnd()) {
            QString line = in.readLine();
            QString filteredLine;
            for (QChar &c : line) {
                if (c.isPrint()) {
                    filteredLine.append(c);
                }
            }
            content.append(filteredLine + "\n");
        }
        file.close();

        // regex work
        QRegularExpression re(target);
        QRegularExpressionMatch match = re.match(content);
        if (match.hasMatch())
            return true;
        else
            return false;
    }
};

#endif // FILETRAVERSETHREAD_H
