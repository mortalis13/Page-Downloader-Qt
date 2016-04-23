#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include <QDialog>
#include <QFile>

#include <QtNetwork>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
  Q_OBJECT

public:
  explicit MainWindow(QWidget *parent = 0);
  ~MainWindow();

  void writeFile(QString path, QString text);
  void log(QString msg);
  void addShortcuts();
  void downloadPage(bool search=false);
  void fixFilename(bool filenameEmpty);

  static QString regexFind(QString pattern, QString text, int group = 0);
  QString urlFileName(QUrl url) const;
  
  void test();

  void printBytes(QByteArray bytes);
  void printBytesHex(QByteArray bytes);
public slots:
  void downloadClicked();
  void replyFinished(QNetworkReply *nr);
  void showProgress(qint64 done, qint64 total);
  void searchClicked();
  void openClicked();

private:
  Ui::MainWindow *ui;

  QProcess *proc;

  QString filename;
  QString path;
};

#endif // MAINWINDOW_H
