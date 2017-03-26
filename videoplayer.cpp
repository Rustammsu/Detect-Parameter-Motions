#include "videoplayer.h"

#include <QtWidgets>
#include <qvideosurfaceformat.h>

#include <iostream>
#include <vector>
#include <string>
#include <cmath>
#include <limits>

#define ALMOST_ZERO 0.00001

VideoPlayer::VideoPlayer(QWidget *parent)
  : QWidget(parent)
  , mediaPlayer(0, QMediaPlayer::VideoSurface)
  , playButton(0)
  , positionSlider(0)
  , errorLabel(0)
  , time{0}
  , iter(0)
{
  QAbstractButton *openButton = new QPushButton(tr("Open..."));
  connect(openButton, &QAbstractButton::clicked, this, &VideoPlayer::openFile);

  playButton = new QPushButton;
  playButton->setEnabled(false);
  playButton->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));

  connect(playButton, &QAbstractButton::clicked,
          this, &VideoPlayer::play);

  positionSlider = new QSlider(Qt::Horizontal);
  positionSlider->setRange(0, 0);

  connect(positionSlider, &QAbstractSlider::sliderMoved,
          this, &VideoPlayer::setPosition);

  errorLabel = new QLabel;
  errorLabel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum);

  QBoxLayout *controlLayout = new QHBoxLayout;
  controlLayout->setMargin(0);
  controlLayout->addWidget(openButton);
  controlLayout->addWidget(playButton);
  controlLayout->addWidget(positionSlider);

  QAbstractButton *point1Button = new QPushButton(tr("red point"));
  connect(point1Button, &QAbstractButton::clicked, this, &VideoPlayer::openPoint1);

  QAbstractButton *point2Button = new QPushButton(tr("green point"));
  connect(point2Button, &QAbstractButton::clicked, this, &VideoPlayer::openPoint2);

  QAbstractButton *point3Button = new QPushButton(tr("blue point"));
  connect(point3Button, &QAbstractButton::clicked, this, &VideoPlayer::openPoint3);

  QAbstractButton *plotCoordinatesButton = new QPushButton(tr("cordinates"));
  connect(plotCoordinatesButton, &QAbstractButton::clicked, this, &VideoPlayer::drawCoordinates);

  QAbstractButton *plotVelocitiesButton = new QPushButton(tr("velocities"));
  connect(plotVelocitiesButton, &QAbstractButton::clicked, this, &VideoPlayer::drawVelocities);

  QAbstractButton *plotAngularVelocitiesButton = new QPushButton(tr("angular velocities"));
  connect(plotAngularVelocitiesButton, &QAbstractButton::clicked, this, &VideoPlayer::drawAngularVelocities);

  QBoxLayout *pointLayout = new QHBoxLayout;
  pointLayout->setMargin(0);
  pointLayout->addWidget(point1Button);
  pointLayout->addWidget(point2Button);
  pointLayout->addWidget(point3Button);
  pointLayout->addWidget(plotCoordinatesButton);
  pointLayout->addWidget(plotVelocitiesButton);
  pointLayout->addWidget(plotAngularVelocitiesButton);

  QBoxLayout *layout = new QVBoxLayout;
  layout->addLayout(controlLayout);
  layout->addLayout(pointLayout);
  layout->addWidget(errorLabel);

  imagelabel = new QLabel;
  imagelabel->setScaledContents(true);
  imagelabel->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
  layout->addWidget(imagelabel);

  timelabel = new QLabel;
  timelabel->setScaledContents(true);
  timelabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Ignored);
  timelabel->setMinimumHeight(16);
  timelabel->setMaximumHeight(16);
  layout->addWidget(timelabel);

  coordinatelabel = new QLabel;
  coordinatelabel->setScaledContents(true);
  coordinatelabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Ignored);
  coordinatelabel->setMinimumHeight(16);
  coordinatelabel->setMaximumHeight(16);
  layout->addWidget(coordinatelabel);

  setLayout(layout);

  surface = new myQAbstractVideoSurface();
  mediaPlayer.setPlaybackRate(0.1);
  mediaPlayer.setVideoOutput(surface);

  point1Color = QColor(255, 0, 0, 255);
  point1Color = QColor(0, 255, 0, 255);
  point1Color = QColor(0, 0, 255, 255);

  plotxy21 = new QCustomPlot;
  plotxy21->addGraph(); plotxy21->addGraph();
  plotxy21->graph(0)->setPen(QPen(Qt::red));
  plotxy21->graph(1)->setPen(QPen(Qt::green));
  plotxy21->xAxis2->setVisible(true);
  plotxy21->xAxis2->setTickLabels(false);
  plotxy21->yAxis2->setVisible(true);
  plotxy21->yAxis2->setTickLabels(false);
  connect(plotxy21->xAxis, SIGNAL(rangeChanged(QCPRange)), plotxy21->xAxis2, SLOT(setRange(QCPRange)));
  connect(plotxy21->yAxis, SIGNAL(rangeChanged(QCPRange)), plotxy21->yAxis2, SLOT(setRange(QCPRange)));
  plotxy21->graph(0)->setLineStyle((QCPGraph::LineStyle)QCPGraph::lsLine);
  plotxy21->graph(0)->selectionDecorator()->setPen(QPen(Qt::red));
  plotxy21->graph(0)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle, 5));
  plotxy21->graph(1)->setLineStyle((QCPGraph::LineStyle)QCPGraph::lsLine);
  plotxy21->graph(1)->selectionDecorator()->setPen(QPen(Qt::green));
  plotxy21->graph(1)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle, 5));
  plotxy21->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectPlottables);

  plotxy31 = new QCustomPlot;
  plotxy31->addGraph(); plotxy31->addGraph();
  plotxy31->graph(0)->setPen(QPen(Qt::red));
  plotxy31->graph(1)->setPen(QPen(Qt::blue));
  plotxy31->xAxis2->setVisible(true);
  plotxy31->xAxis2->setTickLabels(false);
  plotxy31->yAxis2->setVisible(true);
  plotxy31->yAxis2->setTickLabels(false);
  connect(plotxy31->xAxis, SIGNAL(rangeChanged(QCPRange)), plotxy31->xAxis2, SLOT(setRange(QCPRange)));
  connect(plotxy31->yAxis, SIGNAL(rangeChanged(QCPRange)), plotxy31->yAxis2, SLOT(setRange(QCPRange)));
  plotxy31->graph(0)->setLineStyle((QCPGraph::LineStyle)QCPGraph::lsLine);
  plotxy31->graph(0)->selectionDecorator()->setPen(QPen(Qt::red));
  plotxy31->graph(0)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle, 5));
  plotxy31->graph(1)->setLineStyle((QCPGraph::LineStyle)QCPGraph::lsLine);
  plotxy31->graph(1)->selectionDecorator()->setPen(QPen(Qt::blue));
  plotxy31->graph(1)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle, 5));
  plotxy31->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectPlottables);

  plotvelocities = new QCustomPlot;
  plotvelocities->addGraph(); plotvelocities->addGraph(); plotvelocities->addGraph();
  plotvelocities->addGraph(); plotvelocities->addGraph(); plotvelocities->addGraph();
  plotvelocities->graph(0)->setPen(QPen(Qt::red));
  plotvelocities->graph(1)->setPen(QPen(Qt::magenta));
  plotvelocities->graph(2)->setPen(QPen(Qt::green));
  plotvelocities->graph(3)->setPen(QPen(Qt::black));
  plotvelocities->graph(4)->setPen(QPen(Qt::blue));
  plotvelocities->graph(5)->setPen(QPen(Qt::gray));
  plotvelocities->xAxis2->setVisible(true);
  plotvelocities->xAxis2->setTickLabels(false);
  plotvelocities->yAxis2->setVisible(true);
  plotvelocities->yAxis2->setTickLabels(false);
  connect(plotvelocities->xAxis, SIGNAL(rangeChanged(QCPRange)), plotvelocities->xAxis2, SLOT(setRange(QCPRange)));
  connect(plotvelocities->yAxis, SIGNAL(rangeChanged(QCPRange)), plotvelocities->yAxis2, SLOT(setRange(QCPRange)));
  plotvelocities->graph(0)->setLineStyle((QCPGraph::LineStyle)QCPGraph::lsLine);
  plotvelocities->graph(0)->selectionDecorator()->setPen(QPen(Qt::red));
  plotvelocities->graph(0)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle, 5));
  plotvelocities->graph(1)->setLineStyle((QCPGraph::LineStyle)QCPGraph::lsLine);
  plotvelocities->graph(1)->selectionDecorator()->setPen(QPen(Qt::magenta));
  plotvelocities->graph(1)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle, 5));
  plotvelocities->graph(2)->setLineStyle((QCPGraph::LineStyle)QCPGraph::lsLine);
  plotvelocities->graph(2)->selectionDecorator()->setPen(QPen(Qt::green));
  plotvelocities->graph(2)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle, 5));
  plotvelocities->graph(3)->setLineStyle((QCPGraph::LineStyle)QCPGraph::lsLine);
  plotvelocities->graph(3)->selectionDecorator()->setPen(QPen(Qt::black));
  plotvelocities->graph(3)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle, 5));
  plotvelocities->graph(4)->setLineStyle((QCPGraph::LineStyle)QCPGraph::lsLine);
  plotvelocities->graph(4)->selectionDecorator()->setPen(QPen(Qt::blue));
  plotvelocities->graph(4)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle, 5));
  plotvelocities->graph(5)->setLineStyle((QCPGraph::LineStyle)QCPGraph::lsLine);
  plotvelocities->graph(5)->selectionDecorator()->setPen(QPen(Qt::gray));
  plotvelocities->graph(5)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle, 5));
  plotvelocities->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectPlottables);

  plotangularvelocities = new QCustomPlot;
  plotangularvelocities->addGraph(); plotangularvelocities->addGraph(); plotangularvelocities->addGraph();
  plotangularvelocities->graph(0)->setPen(QPen(Qt::red));
  plotangularvelocities->graph(1)->setPen(QPen(Qt::green));
  plotangularvelocities->graph(2)->setPen(QPen(Qt::blue));
  plotangularvelocities->xAxis2->setVisible(true);
  plotangularvelocities->xAxis2->setTickLabels(false);
  plotangularvelocities->yAxis2->setVisible(true);
  plotangularvelocities->yAxis2->setTickLabels(false);
  connect(plotangularvelocities->xAxis, SIGNAL(rangeChanged(QCPRange)), plotangularvelocities->xAxis2, SLOT(setRange(QCPRange)));
  connect(plotangularvelocities->yAxis, SIGNAL(rangeChanged(QCPRange)), plotangularvelocities->yAxis2, SLOT(setRange(QCPRange)));
  plotangularvelocities->graph(0)->setLineStyle((QCPGraph::LineStyle)QCPGraph::lsLine);
  plotangularvelocities->graph(0)->selectionDecorator()->setPen(QPen(Qt::red));
  plotangularvelocities->graph(0)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle, 5));
  plotangularvelocities->graph(1)->setLineStyle((QCPGraph::LineStyle)QCPGraph::lsLine);
  plotangularvelocities->graph(1)->selectionDecorator()->setPen(QPen(Qt::green));
  plotangularvelocities->graph(1)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle, 5));
  plotangularvelocities->graph(2)->setLineStyle((QCPGraph::LineStyle)QCPGraph::lsLine);
  plotangularvelocities->graph(2)->selectionDecorator()->setPen(QPen(Qt::blue));
  plotangularvelocities->graph(2)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle, 5));
  plotangularvelocities->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectPlottables);

  connect(&mediaPlayer, &QMediaPlayer::stateChanged,
          this, &VideoPlayer::mediaStateChanged);
  connect(&mediaPlayer, &QMediaPlayer::positionChanged, this, &VideoPlayer::positionChanged);
  connect(&mediaPlayer, &QMediaPlayer::durationChanged, this, &VideoPlayer::durationChanged);
  typedef void (QMediaPlayer::*ErrorSignal)(QMediaPlayer::Error);
  connect(&mediaPlayer, static_cast<ErrorSignal>(&QMediaPlayer::error),
          this, &VideoPlayer::handleError);

  connect(surface, SIGNAL(newImage(QPixmap)),
            this, SLOT(setNewImage(QPixmap)),Qt::QueuedConnection);
  connect(this, SIGNAL(ClearPixmap()),
              surface, SLOT(ClearPixmap()),Qt::QueuedConnection);
}

VideoPlayer::~VideoPlayer()
{
}

void VideoPlayer::openPoint1() {
  QFileDialog fileDialog(this);
  fileDialog.setAcceptMode(QFileDialog::AcceptOpen);
  fileDialog.setWindowTitle(tr("Open point1"));

  fileDialog.setDirectory(QStandardPaths::standardLocations(QStandardPaths::HomeLocation).value(0, QDir::homePath()));
  QString pointFile = fileDialog.getOpenFileName();
  if(!pointFile.isEmpty()) {
    QImage pointImage(pointFile);
    long long r = 0, g = 0, b = 0, alpha = 0;
    for(int i = 0; i < pointImage.width(); i++) {
      for(int j = 0; j < pointImage.height(); j++) {
        r += pointImage.pixelColor(i, j).red();
        g += pointImage.pixelColor(i, j).green();
        b += pointImage.pixelColor(i, j).blue();
        alpha += pointImage.pixelColor(i, j).alpha();
      }
    }
    r /= pointImage.width()*pointImage.height();
    g /= pointImage.width()*pointImage.height();
    b /= pointImage.width()*pointImage.height();
    alpha /= pointImage.width()*pointImage.height();
    std::cout << r << ' ' << g << ' ' << b << ' ' << alpha << std::endl;
    point1Color = QColor(r, g, b, alpha);
  }
}


void VideoPlayer::openPoint2() {
  QFileDialog fileDialog(this);
  fileDialog.setAcceptMode(QFileDialog::AcceptOpen);
  fileDialog.setWindowTitle(tr("Open point2"));

  fileDialog.setDirectory(QStandardPaths::standardLocations(QStandardPaths::HomeLocation).value(0, QDir::homePath()));
  QString pointFile = fileDialog.getOpenFileName();
  if(!pointFile.isEmpty()) {
    QImage pointImage(pointFile);
    long long r = 0, g = 0, b = 0, alpha = 0;
    for(int i = 0; i < pointImage.width(); i++) {
      for(int j = 0; j < pointImage.height(); j++) {
        r += pointImage.pixelColor(i, j).red();
        g += pointImage.pixelColor(i, j).green();
        b += pointImage.pixelColor(i, j).blue();
        alpha += pointImage.pixelColor(i, j).alpha();
      }
    }
    r /= pointImage.width()*pointImage.height();
    g /= pointImage.width()*pointImage.height();
    b /= pointImage.width()*pointImage.height();
    alpha /= pointImage.width()*pointImage.height();
    std::cout << r << ' ' << g << ' ' << b << ' ' << alpha << std::endl;
    point2Color = QColor(r, g, b, alpha);
  }
}


void VideoPlayer::openPoint3() {
  QFileDialog fileDialog(this);
  fileDialog.setAcceptMode(QFileDialog::AcceptOpen);
  fileDialog.setWindowTitle(tr("Open point3"));

  fileDialog.setDirectory(QStandardPaths::standardLocations(QStandardPaths::HomeLocation).value(0, QDir::homePath()));
  QString pointFile = fileDialog.getOpenFileName();
  if(!pointFile.isEmpty()) {
    QImage pointImage(pointFile);
    long long r = 0, g = 0, b = 0, alpha = 0;
    for(int i = 0; i < pointImage.width(); i++) {
      for(int j = 0; j < pointImage.height(); j++) {
        r += pointImage.pixelColor(i, j).red();
        g += pointImage.pixelColor(i, j).green();
        b += pointImage.pixelColor(i, j).blue();
        alpha += pointImage.pixelColor(i, j).alpha();
      }
    }
    r /= pointImage.width()*pointImage.height();
    g /= pointImage.width()*pointImage.height();
    b /= pointImage.width()*pointImage.height();
    alpha /= pointImage.width()*pointImage.height();
    std::cout << r << ' ' << g << ' ' << b << ' ' << alpha << std::endl;
    point3Color = QColor(r, g, b, alpha);
  }
}

void VideoPlayer::drawCoordinates() {
  const QRect availableGeometry21 = QApplication::desktop()->availableGeometry(plotxy21);
  plotxy21->resize(availableGeometry21.width() / 3, availableGeometry21.height() / 2);
  plotxy21->show();

  const QRect availableGeometry31 = QApplication::desktop()->availableGeometry(plotxy31);
  plotxy31->resize(availableGeometry31.width() / 3, availableGeometry31.height() / 2);
  plotxy31->show();
}

void VideoPlayer::drawVelocities() {
  const QRect availableGeometry = QApplication::desktop()->availableGeometry(plotvelocities);
  plotvelocities->resize(availableGeometry.width() / 3, availableGeometry.height() / 2);
  plotvelocities->show();
}

void VideoPlayer::drawAngularVelocities() {
  const QRect availableGeometry = QApplication::desktop()->availableGeometry(plotangularvelocities);
  plotangularvelocities->resize(availableGeometry.width() / 3, availableGeometry.height() / 2);
  plotangularvelocities->show();
}

void VideoPlayer::openFile() {
  QFileDialog fileDialog(this);
  fileDialog.setAcceptMode(QFileDialog::AcceptOpen);
  fileDialog.setWindowTitle(tr("Open Movie"));
  QStringList supportedMimeTypes = mediaPlayer.supportedMimeTypes();
  if (!supportedMimeTypes.isEmpty())
      fileDialog.setMimeTypeFilters(supportedMimeTypes);
  fileDialog.setDirectory(QStandardPaths::standardLocations(QStandardPaths::HomeLocation).value(0, QDir::homePath()));
  if (fileDialog.exec() == QDialog::Accepted)
      setUrl(fileDialog.selectedUrls().constFirst());

  plotxy21->graph(0)->data()->clear();
  plotxy21->graph(1)->data()->clear();
  
  plotxy31->graph(0)->data()->clear();
  plotxy31->graph(1)->data()->clear();

  plotvelocities->graph(0)->data()->clear();
  plotvelocities->graph(1)->data()->clear();
  plotvelocities->graph(2)->data()->clear();
  plotvelocities->graph(3)->data()->clear();
  plotvelocities->graph(4)->data()->clear();
  plotvelocities->graph(5)->data()->clear();
  
  plotangularvelocities->graph(0)->data()->clear();
  plotangularvelocities->graph(1)->data()->clear();
  plotangularvelocities->graph(2)->data()->clear();
}

void VideoPlayer::setUrl(const QUrl &url) {
  errorLabel->setText(QString());
  setWindowFilePath(url.isLocalFile() ? url.toLocalFile() : QString());
  mediaPlayer.setMedia(url);
  playButton->setEnabled(true);
}

void VideoPlayer::play() {
  switch(mediaPlayer.state()) {
  case QMediaPlayer::PlayingState:
      mediaPlayer.pause();
      break;
  default:
      mediaPlayer.play();
      break;
  }
}

void VideoPlayer::mediaStateChanged(QMediaPlayer::State state) {
  switch(state) {
  case QMediaPlayer::PlayingState:
      playButton->setIcon(style()->standardIcon(QStyle::SP_MediaPause));
      break;
  default:
      playButton->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
      break;
  }
}

void VideoPlayer::positionChanged(qint64 position) {
  positionSlider->setValue(position);
}

void VideoPlayer::durationChanged(qint64 duration) {
  positionSlider->setRange(0, duration);
}

void VideoPlayer::setPosition(int position) {
  mediaPlayer.setPosition(position);
}

void VideoPlayer::handleError() {
  playButton->setEnabled(false);
  const QString errorString = mediaPlayer.errorString();
  QString message = "Error: ";
  if (errorString.isEmpty())
      message += " #" + QString::number(int(mediaPlayer.error()));
  else
      message += errorString;
  errorLabel->setText(message);
}

void VideoPlayer::setNewImage(QPixmap pix) {
  emit ClearPixmap();

  capturePoints(pix);
  imagelabel->setPixmap(pix.scaled(imagelabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
  imagelabel->update();
}

void VideoPlayer::capturePoints(QPixmap &pix) {
  static bool flag = false;
  QImage image = pix.toImage();

  if(image.isNull())
    return;

  int mean_red[3] = {0};
  int mean_green[3] = {0};
  int mean_blue[3] = {0};
  for(int i = 0; i < image.width(); i++) {
    for(int j = 0; j < image.height(); j++) {
      if(abs(point1Color.red() - image.pixelColor(i, j).red()) < 20 &&
         abs(point1Color.green() - image.pixelColor(i, j).green()) < 20 &&
         abs(point1Color.blue() - image.pixelColor(i, j).blue()) < 20) {
        image.setPixelColor(i, j, QColor(255, 0, 0, 0));
        mean_red[0] += i; mean_red[1] += j; mean_red[2] += 1;
      } else if(abs(point2Color.red() - image.pixelColor(i, j).red()) < 20 &&
         abs(point2Color.green() - image.pixelColor(i, j).green()) < 20 &&
         abs(point2Color.blue() - image.pixelColor(i, j).blue()) < 20) {
        image.setPixelColor(i, j, QColor(0, 255, 0, 0));
        mean_green[0] += i; mean_green[1] += j; mean_green[2] += 1;
      } else if(abs(point3Color.red() - image.pixelColor(i, j).red()) < 20 &&
         abs(point3Color.green() - image.pixelColor(i, j).green()) < 20 &&
         abs(point3Color.blue() - image.pixelColor(i, j).blue()) < 20) {
        image.setPixelColor(i, j, QColor(0, 0, 255, 0));
        mean_blue[0] += i; mean_blue[1] += j; mean_blue[2] += 1;
      }
    }
  }

  if(mean_red[2] == 0 || mean_green[2] == 0 || mean_blue[2] == 0) {
    pix = QPixmap::fromImage(image);
    return;
  }

  if(time[(iter + 2) % 3] == double(mediaPlayer.position())/1000.0) {
    pix = QPixmap::fromImage(image);
    return;
  }
  time[iter] = double(mediaPlayer.position())/1000.0;
  timelabel->setText((std::string("time: ") + std::to_string(time[iter])).c_str());

  red[iter] = std::pair<int, int>(mean_red[0]/mean_red[2], mean_red[1]/mean_red[2]);

  if(5 < red[iter].first && red[iter].first < image.width() - 5 &&
     5 < red[iter].second && red[iter].second < image.height() - 5) {
    for(int i = red[iter].first - 5; i <= red[iter].first + 5; i++) {
      for(int j = red[iter].second - 5; j <= red[iter].second + 5; j++) {
        image.setPixelColor(i, j, QColor(0, 0, 0, 0));
      }
    }
  }

  green[iter] = std::pair<int, int>(mean_green[0]/mean_green[2], mean_green[1]/mean_green[2]);

  if(5 < green[iter].first && green[iter].first < image.width() - 5 &&
     5 < green[iter].second && green[iter].second < image.height() - 5) {
    for(int i = green[iter].first - 5; i <= green[iter].first + 5; i++) {
      for(int j = green[iter].second - 5; j <= green[iter].second + 5; j++) {
        image.setPixelColor(i, j, QColor(0, 0, 0, 0));
      }
    }
  }

  blue[iter] = std::pair<int, int>(mean_blue[0]/mean_blue[2], mean_blue[1]/mean_blue[2]);

  if(5 < blue[iter].first && blue[iter].first < image.width() - 5 &&
     5 < blue[iter].second && blue[iter].second < image.height() - 5) {
    for(int i = blue[iter].first - 5; i <= blue[iter].first + 5; i++) {
      for(int j = blue[iter].second - 5; j <= blue[iter].second + 5; j++) {
        image.setPixelColor(i, j, QColor(0, 0, 0, 0));
      }
    }
  }

  plotxy21->graph(0)->addData(time[iter], green[iter].first - red[iter].first);
  plotxy21->graph(0)->rescaleAxes();
  plotxy21->graph(1)->addData(time[iter], green[iter].second - red[iter].second);
  plotxy21->graph(1)->rescaleAxes();


  plotxy31->graph(0)->addData(time[iter], blue[iter].first - red[iter].first);
  plotxy31->graph(0)->rescaleAxes();
  plotxy31->graph(1)->addData(time[iter], blue[iter].second - red[iter].second);
  plotxy31->graph(1)->rescaleAxes();

  plotxy21->replot();
  plotxy31->replot();
  pix = QPixmap::fromImage(image);

  if(iter == 2) {
    flag = true;
  }

  if(((red[iter].first - red[(iter + 2) % 3].first) / (time[iter] - time[(iter + 2) % 3])) > 10000 ||
     ((red[iter].second - red[(iter + 2) % 3].second) / (time[iter] - time[(iter + 2) % 3])) > 10000 ||
     ((green[iter].first - green[(iter + 2) % 3].first) / (time[iter] - time[(iter + 2) % 3])) > 10000 ||
     ((green[iter].second - green[(iter + 2) % 3].second) / (time[iter] - time[(iter + 2) % 3])) > 10000 ||
     ((blue[iter].first - blue[(iter + 2) % 3].first) / (time[iter] - time[(iter + 2) % 3])) > 10000 ||
     ((blue[iter].second - blue[(iter + 2) % 3].second) / (time[iter] - time[(iter + 2) % 3])) > 10000) {
    iter = (iter + 1) % 3;
    return;
  }
  plotvelocities->graph(0)->addData(time[iter] * 10, (red[iter].first - red[(iter + 2) % 3].first) /
      (time[iter] - time[(iter + 2) % 3]));
  plotvelocities->graph(0)->rescaleAxes();
  plotvelocities->graph(1)->addData(time[iter] * 10, (red[iter].second - red[(iter + 2) % 3].second) /
      (time[iter] - time[(iter + 2) % 3]));
  plotvelocities->graph(1)->rescaleAxes();

  plotvelocities->graph(2)->addData(time[iter] * 10, (green[iter].first - green[(iter + 2) % 3].first) /
      (time[iter] - time[(iter + 2) % 3]));
  plotvelocities->graph(2)->rescaleAxes();
  plotvelocities->graph(3)->addData(time[iter] * 10, (green[iter].second - green[(iter + 2) % 3].second) /
      (time[iter] - time[(iter + 2) % 3]));
  plotvelocities->graph(3)->rescaleAxes();

  plotvelocities->graph(4)->addData(time[iter] * 10, (blue[iter].first - blue[(iter + 2) % 3].first) /
      (time[iter] - time[(iter + 2) % 3]));
  plotvelocities->graph(4)->rescaleAxes();
  plotvelocities->graph(5)->addData(time[iter] * 10, (blue[iter].second - blue[(iter + 2) % 3].second) /
      (time[iter] - time[(iter + 2) % 3]));
  plotvelocities->graph(5)->rescaleAxes();

  plotvelocities->replot();

  if(flag) {
    getAngularVelocity();
  }

  iter = (iter + 1) % 3;
}

void VideoPlayer::getAngularVelocity() {
  static double omega1[3] = {0};
  static double omega2[3] = {0};
  static double omega3[3] = {0};

  double omegax = 0;
  double omegay = 0;
  double omegaz = 0;
  double dt = time[iter] - time[(iter + 2) % 3];

  double x21 = green[iter].first - red[iter].first, y21 = green[iter].second - red[iter].second;
  double x31 = blue[iter].first - red[iter].first, y31 = blue[iter].second - red[iter].second;

  //f(std::cos(time[iter]), 0, 0, 1, omegax, omegay, omegaz, dt);
  //f(1, 0, 0, std::cos(time[iter]), omegax, omegay, omegaz, dt);
  //f(std::cos(time[iter]), std::sin(time[iter]), std::cos(3.14159265 / 2.0 + time[iter]), std::sin(3.14159265 / 2.0 + time[iter]), omegax, omegay, omegaz, dt);
  //f(std::cos(time[iter])*std::cos(time[iter] / 2.0), std::sin(time[iter])*std::cos(time[iter] / 2.0), std::cos(3.14159265 / 2.0 + time[iter]), std::sin(3.14159265 / 2.0 + time[iter]), omegax, omegay, omegaz, dt);
  f(x21, y21, x31, y31, omegax, omegay, omegaz, dt);

  omega1[iter] = (omegax + omega1[0] + omega1[1] + omega1[2]) / 4;
  omega2[iter] = (omegay + omega2[0] + omega2[1] + omega2[2]) / 4;
  omega3[iter] = (omegaz + omega3[0] + omega3[1] + omega3[2]) / 4;

  plotangularvelocities->graph(0)->addData(time[iter], omega1[iter]);
  plotangularvelocities->graph(1)->addData(time[iter], omega2[iter]);
  plotangularvelocities->graph(2)->addData(time[iter], omega3[iter]);
  plotangularvelocities->replot();
}

void VideoPlayer::f(double x1, double y1, double x2, double y2, double &omegax, double &omegay, double &omegaz, double dt) {
  static double costh = 0, sinth = 0;
  static double r1 = 0, r2 = 0;

  if(r1 <= std::pow(x1*x1 + y1*y1, 0.5) && r2 <= std::pow(x2*x2 + y2*y2, 0.5)) {

    r1 = std::pow(x1*x1 + y1*y1, 0.5);
    r2 = std::pow(x2*x2 + y2*y2, 0.5);
    costh = (x1*x2 + y1*y2) / (r1*r2);
    sinth = std::pow(1 - costh*costh, 0.5);
  }

  static double alpha0 = 0;
  static double beta0 = 0;
  static double gamma0 = 0;

  static double signa = 1;
  static double signb = 1;
  static double signg = 1;

  double alpha = 0;
  double beta = 0;
  double gamma = 0;

  double fault;
  int count;

  double step;
  //alpha
  for(int i = 0; i < 2; i++) {
    step = 0.01;
    step = (std::abs(std::abs(r1*std::cos(beta0)*std::cos(alpha0 + alpha)*x1 + r1*std::cos(beta0)*std::sin(alpha0 + alpha)*y1) - r1*std::pow(x1*x1 + y1*y1, 0.5)) -
        std::abs(std::abs(r1*std::cos(beta0)*std::cos(alpha0 + alpha + step)*x1 + r1*std::cos(beta0)*std::sin(alpha0 + alpha + step)*y1) - r1*std::pow(x1*x1 + y1*y1, 0.5))) / step;
    if(step != 0) {
      switch(i) {
        case 0:
          step = 0.01*signa;
          break;
        case 1:
          step = step*0.01 / std::abs(step);
          break;
      }
    }

    count = 0;
    while(true) {
      if(count > 500)
        break;
      fault = std::abs(std::abs(r1*std::cos(beta0)*std::cos(alpha0 + alpha)*x1 + r1*std::cos(beta0)*std::sin(alpha0 + alpha)*y1) - r1*std::pow(x1*x1 + y1*y1, 0.5));
      if(std::abs(std::abs(r1*std::cos(beta0)*std::cos(alpha0 + alpha + step)*x1 + r1*std::cos(beta0)*std::sin(alpha0 + alpha + step)*y1) - r1*std::pow(x1*x1 + y1*y1, 0.5)) < fault) {
        alpha += step;
        step = (std::abs(std::abs(r1*std::cos(beta0)*std::cos(alpha0 + alpha)*x1 + r1*std::cos(beta0)*std::sin(alpha0 + alpha)*y1) - r1*std::pow(x1*x1 + y1*y1, 0.5)) -
            std::abs(std::abs(r1*std::cos(beta0)*std::cos(alpha0 + alpha + step)*x1 + r1*std::cos(beta0)*std::sin(alpha0 + alpha + step)*y1) - r1*std::pow(x1*x1 + y1*y1, 0.5))) / step;
        if(std::abs(step) != 0)
          step = step*0.01 / std::abs(step);
        count++;
      } else {
        step /= 2.0;
      }

      if(std::abs(step) < 0.00000001)
        break;
    }
    if(alpha != 0)
      break;
  }

  //beta
  for(int i = 0; i < 2; i++) {
    step = 0.01;
    step = (std::abs(r1*std::cos(beta0 + beta)*std::cos(alpha0 + alpha) - x1) + std::abs(r1*std::cos(beta0 + beta)*std::sin(alpha0 + alpha) - y1) -
            std::abs(r1*std::cos(beta0 + beta + step)*std::cos(alpha0 + alpha) - x1) - std::abs(r1*std::cos(beta0 + beta + step)*std::sin(alpha0 + alpha) - y1)) / step;
    if(step != 0) {
      if(step != 0) {
        switch(i) {
          case 0:
            step = 0.01*signb;
            break;
          case 1:
            step = step*0.01 / std::abs(step);
            break;
        }
      }
    }

    count = 0;
    while(true) {
      if(count > 500)
        break;
      fault = std::abs(r1*cos(beta0 + beta)*std::cos(alpha0 + alpha) - x1) + std::abs(r1*std::cos(beta0 + beta)*std::sin(alpha0 + alpha) - y1);
        if(std::abs(r1*std::cos(beta0 + beta + step)*std::cos(alpha0 + alpha) - x1) + std::abs(r1*std::cos(beta0 + beta + step)*std::sin(alpha0 + alpha) - y1) < fault) {
          beta += step;
          step = (std::abs(r1*std::cos(beta0 + beta)*std::cos(alpha0 + alpha) - x1) + std::abs(r1*std::cos(beta0 + beta)*std::sin(alpha0 + alpha) - y1) -
                  std::abs(r1*std::cos(beta0 + beta + step)*std::cos(alpha0 + alpha) - x1) - std::abs(r1*std::cos(beta0 + beta + step)*std::sin(alpha0 + alpha) - y1)) / step;
          if(step != 0)
            step = step*0.01/std::abs(step);
          count++;
        } else {
          step /= 2.0;
        }

        if(std::abs(step) < 0.00000001)
          break;
    }
    if(beta != 0)
      break;
  }

  //gamma
  for(int i = 0; i < 2; i++) {
    step = 0.01;
    step = (std::abs((r2*costh*std::cos(beta0 + beta) + r2*sinth*std::sin(beta0 + beta)*std::sin(gamma0 + gamma))*std::cos(alpha0 + alpha) -
                r2*sinth*std::sin(alpha0 + alpha)*std::cos(gamma0 + gamma) - x2) +
            std::abs((r2*costh*std::cos(beta0 + beta) + r2*sinth*std::sin(beta0 + beta)*std::sin(gamma0 + gamma))*std::sin(alpha0 + alpha) +
                r2*sinth*std::cos(alpha0 + alpha)*std::cos(gamma0 + gamma) - y2) -
            std::abs((r2*costh*std::cos(beta0 + beta) + r2*sinth*std::sin(beta0 + beta)*std::sin(gamma0 + gamma + step))*std::cos(alpha0 + alpha) -
                r2*sinth*std::sin(alpha0 + alpha)*std::cos(gamma0 + gamma + step) - x2) -
            std::abs((r2*costh*std::cos(beta0 + beta) + r2*sinth*std::sin(beta0 + beta)*std::sin(gamma0 + gamma + step))*std::sin(alpha0 + alpha) +
                r2*sinth*std::cos(alpha0 + alpha)*std::cos(gamma0 + gamma + step) - y2)) / step;
    if(step != 0) {
      switch(i) {
        case 0:
          step = 0.01*signg;
          break;
        case 1:
          step = step*0.01 / std::abs(step);
          break;
      }
    }

    count = 0;
    while(true) {
      if(count > 500)
        break;
      fault = std::abs(std::abs((r2*costh*std::cos(beta0 + beta) + r2*sinth*std::sin(beta0 + beta)*std::sin(gamma0 + gamma))*std::cos(alpha0 + alpha) -
                  r2*sinth*std::sin(alpha0 + alpha)*std::cos(gamma0 + gamma) - x2) +
              std::abs((r2*costh*std::cos(beta0 + beta) + r2*sinth*std::sin(beta0 + beta)*std::sin(gamma0 + gamma))*std::sin(alpha0 + alpha) +
                  r2*sinth*std::cos(alpha0 + alpha)*std::cos(gamma0 + gamma) - y2));
        if(std::abs(std::abs((r2*costh*std::cos(beta0 + beta) + r2*sinth*std::sin(beta0 + beta)*std::sin(gamma0 + gamma + step))*std::cos(alpha0 + alpha) -
               r2*sinth*std::sin(alpha0 + alpha)*std::cos(gamma0 + gamma + step) - x2) -
          std::abs((r2*costh*std::cos(beta0 + beta) + r2*sinth*std::sin(beta0 + beta)*std::sin(gamma0 + gamma + step))*std::sin(alpha0 + alpha) +
               r2*sinth*std::cos(alpha0 + alpha)*std::cos(gamma0 + gamma + step) - y2)) < fault) {
          gamma += step;
          step = (std::abs((r2*costh*std::cos(beta0 + beta) + r2*sinth*std::sin(beta0 + beta)*std::sin(gamma0 + gamma))*std::cos(alpha0 + alpha) -
                     r2*sinth*std::sin(alpha0 + alpha)*std::cos(gamma0 + gamma) - x2) +
                  std::abs((r2*costh*std::cos(beta0 + beta) + r2*sinth*std::sin(beta0 + beta)*std::sin(gamma0 + gamma))*std::sin(alpha0 + alpha) +
                      r2*sinth*std::cos(alpha0 + alpha)*std::cos(gamma0 + gamma) - y2) -
                  std::abs((r2*costh*std::cos(beta0 + beta) + r2*sinth*std::sin(beta0 + beta)*std::sin(gamma0 + gamma + step))*std::cos(alpha0 + alpha) -
                      r2*sinth*std::sin(alpha0 + alpha)*std::cos(gamma0 + gamma + step) - x2) -
                  std::abs((r2*costh*std::cos(beta0 + beta) + r2*sinth*std::sin(beta0 + beta)*std::sin(gamma0 + gamma + step))*std::sin(alpha0 + alpha) +
                     r2*sinth*std::cos(alpha0 + alpha)*std::cos(gamma0 + gamma + step) - y2)) / step;
          if(std::abs(step) != 0)
            step = step*0.01/std::abs(step);
          count++;
        } else {
          step /= 2.0;
        }

        if(std::abs(step) < 0.00000001)
          break;
    }
    if(gamma != 0)
      break;
  }
  omegax = gamma*std::cos(beta0 + beta)*std::cos(alpha0 + alpha) / dt - std::sin(alpha0 + alpha)*beta / dt;
  omegay = gamma*std::cos(beta0 + beta)*std::sin(alpha0 + alpha) / dt + std::cos(alpha0 + alpha)*beta / dt;
  omegaz = alpha / dt - std::sin(beta0 + beta)*gamma / dt;

  if(std::abs(alpha) >= 0.01)
    signa = alpha / std::abs(alpha);
  if(std::abs(beta) >= 0.01)
    signb = beta / std::abs(beta);
  if(std::abs(gamma) >= 0.01)
    signg = gamma / std::abs(gamma);

  alpha0 += alpha; beta0 += beta; gamma0 += gamma;
}
