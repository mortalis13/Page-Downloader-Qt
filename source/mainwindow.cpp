#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QtDebug>
#include <QFileDialog>
#include <QMessageBox>
#include <QShortcut>
#include <QRegExp>


MainWindow::MainWindow(QWidget *parent) :
  QMainWindow(parent),
  ui(new Ui::MainWindow)
{
  ui->setupUi(this);

  proc=new QProcess();

  connect( ui->bDownload, SIGNAL(clicked()), this, SLOT(downloadClicked()) );
  connect( ui->bSearch, SIGNAL(clicked()), this, SLOT(searchClicked()) );
  connect( ui->bOpen, SIGNAL(clicked()), this, SLOT(openClicked()) );

  addShortcuts();
}

MainWindow::~MainWindow()
{
  delete ui;
  delete proc;
}


void MainWindow::addShortcuts() {
  QShortcut *bDownload=new QShortcut(QKeySequence("F1"), this);
  connect( bDownload, SIGNAL(activated()), ui->bDownload, SLOT(click()) );

  QShortcut *bSearch=new QShortcut(QKeySequence("F2"), this);
  connect( bSearch, SIGNAL(activated()), ui->bSearch, SLOT(click()) );

  QShortcut *bOpen=new QShortcut(QKeySequence("F3"), this);
  connect( bOpen, SIGNAL(activated()), ui->bOpen, SLOT(click()) );

  QShortcut *quit=new QShortcut(QKeySequence("Esc"), this);
  connect( quit, SIGNAL(activated()), this, SLOT(close()) );

  connect(ui->leURL, SIGNAL(returnPressed()), ui->bDownload, SLOT(click()) );
  connect(ui->leSearch, SIGNAL(returnPressed()), ui->bSearch, SLOT(click()) );
}


// ----------------------------------------- actions -----------------------------------------

void MainWindow::openClicked(){
  QString procVal;
  QString prog=ui->leBrowserPath->text();
  QString page=path+filename;

  procVal="\""+prog+"\" "+page;
  qDebug() << "val:"<<procVal;
  proc->start(procVal);
}

void MainWindow::downloadClicked(){
   downloadPage();
}

void MainWindow::searchClicked(){
  downloadPage(true);
}

void MainWindow::downloadPage(bool search){
  log("Wait...\n");
  
  QString urlText;
  bool filenameEmpty=false;

  ui->statusBar->showMessage("");
  ui->teLog->clear();

  if(search){
    QString searchText=ui->leSearch->text();
    urlText="https://www.google.com/search?q="+searchText+"&gbv=1";
  }
  else{
    urlText=ui->leURL->text();
  }
  
  if(urlText.indexOf(QRegExp("^http"))==-1){
    urlText="http://"+urlText;
  }

  // QUrl url( urlText, QUrl::StrictMode );
  // QUrl url( urlText, QUrl::TolerantMode );
  QUrl url = QUrl::fromEncoded(urlText.toUtf8());

  filename=ui->leFile->text();
  if(filename.trimmed().length()==0){
    // filename.replace(QRegExp("/$"), "");
    
    // filename=url.toString();
    // QString pattern="/[^/]+$";
    // filename=regexFind(pattern, filename);
    // filename.replace(QRegExp("^/"), "");
    
//    filename=url.fileName();
    filename=urlFileName(url);
    
    if(filename.length()==0){
      filename="unnamed_file";
    }
    filenameEmpty=true;
  }
  fixFilename(filenameEmpty);
  
  log("File Name: " + filename + "\n");

  // qDebug() << "fileName:" << filename;
  // qDebug() << "url.path():" << url.path();
  // qDebug() << "url.toString()" << url.toString();
  // qDebug() << "url.toLocalFile()" << url.toLocalFile();
  // qDebug() << "url.fromAce()" << QUrl::fromAce(url.toString().toUtf8());
  // qDebug() << "url.fromPercentEncoding()" << QUrl::fromPercentEncoding(url.toString().toUtf8());


  path=ui->lePath->text();

  QNetworkAccessManager *manager = new QNetworkAccessManager(this);
  connect(manager, SIGNAL(finished(QNetworkReply*)), this, SLOT(replyFinished(QNetworkReply*)));

  QNetworkReply *reply=manager->get(QNetworkRequest(url));
  connect(reply, SIGNAL(downloadProgress(qint64,qint64)), this, SLOT(showProgress(qint64,qint64)) );

  qDebug() << "Start Reading\n";
  log("Start Reading...\n");
}


// ----------------------------------------- service -----------------------------------------

/*
 From Qt 5.3.1 QUrl.cpp source
*/
QString MainWindow::urlFileName(QUrl url) const {
  QString ourPath = url.path();
  ourPath.replace(QRegExp("/$"), "");
  
  const int slash = ourPath.lastIndexOf(QLatin1Char('/'));
  if (slash == -1)
      return ourPath;
  return ourPath.mid(slash + 1);
}

void MainWindow::fixFilename(bool filenameEmpty){
  QByteArray bytes = filename.toUtf8();
  filename = QUrl::fromPercentEncoding(bytes);
  
  if(!filenameEmpty){
    if(ui->chIncrement->isChecked()){
      QString pattern="\\d+$";

      QString num=regexFind(pattern, filename);
      if(num.length()!=0){
        int numVal=num.toInt();
        numVal++;
        num=QString::number(numVal);
        filename=filename.replace(QRegExp(pattern), num);
      }
      else{
        filename+="1";
      }
      ui->leFile->setText(filename);
    }
  }

  if(!ui->chNotHtml->isChecked()){
    if(filename.indexOf(".htm")==-1)
      filename+=".html";
  }
}

QString MainWindow::regexFind(QString pattern, QString text, int group) {
  QString res;

  QRegExp rx(pattern);
  if(rx.indexIn(text)!=-1)
    res=rx.cap(group);

  return res;
}

void MainWindow::replyFinished(QNetworkReply* nr){
  qDebug() << "\n=Start Reply=";

  QByteArray ba=nr->readAll();
  QString content(ba);
  writeFile(path+filename, content);

  qDebug() << "=End Reply=\n";
  log("Finish\n");
  ui->statusBar->showMessage("Download Finished");
}

void MainWindow::showProgress(qint64 done, qint64 total){
//  qDebug() << "Progress:" << done << "\\" << total;

  QString msg=QString("Progress: %1 \\ %2").arg(done).arg(total);
  log(msg);
}

void MainWindow::writeFile(QString path, QString text) {
  QFile file(path);

  if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
    qWarning("Couldn't open file.");
    return;
  }

  file.write(text.toStdString().c_str());
}

void MainWindow::log(QString msg) {
  ui->teLog->appendPlainText(msg);
}


// ----------------------------------------- add -----------------------------------------

// void MainWindow::printBytes(QByteArray bytes){
//   foreach(char byte,bytes){
//     qDebug() << byte;
//   }
//   qDebug();
// }

void MainWindow::printBytes(QByteArray bytes){
    QString res = "";
    QString sep = " ";

    for(int i=0; i<bytes.length(); i++){
        if(i == bytes.length()-1)
            sep="";
        res += QString::number((int) bytes.at(i)) + sep;
    }

    qDebug() << res;
    qDebug();
}

void MainWindow::printBytesHex(QByteArray bytes){
    QString res = "";
    QString sep = " ";

    bytes = bytes.toHex();

    for(int i=0; i<bytes.length(); i++){
        if(i == bytes.length()-1)
            sep="";
        res += bytes[i];
        if(i%2!=0)
            res += sep;
    }

    qDebug() << res;
    qDebug();
}

void MainWindow::test(){
  QString word=ui->leURL->text();
  QUrl url( word, QUrl::TolerantMode );
  
  // qDebug() << "word:" << word;
  // qDebug() << "url:" << url.toString();
  
  QByteArray bytes = word.toUtf8();
  QString res = QUrl::fromPercentEncoding(bytes);
  QByteArray bytes1 = res.toUtf8();
  
  ui->teLog->appendPlainText(res);
  
  printBytes(bytes);
  printBytesHex(bytes);
  
  printBytes(bytes1);
  printBytesHex(bytes1);
}
