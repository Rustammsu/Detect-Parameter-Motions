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

  plotcoordintates = new QCustomPlot;
  plotcoordintates->addGraph(); plotcoordintates->addGraph(); plotcoordintates->addGraph();
  plotcoordintates->addGraph(); plotcoordintates->addGraph(); plotcoordintates->addGraph();
  plotcoordintates->graph(0)->setPen(QPen(Qt::red));
  plotcoordintates->graph(1)->setPen(QPen(Qt::magenta));
  plotcoordintates->graph(2)->setPen(QPen(Qt::green));
  plotcoordintates->graph(3)->setPen(QPen(Qt::black));
  plotcoordintates->graph(4)->setPen(QPen(Qt::blue));
  plotcoordintates->graph(5)->setPen(QPen(Qt::gray));
  plotcoordintates->xAxis2->setVisible(true);
  plotcoordintates->xAxis2->setTickLabels(false);
  plotcoordintates->yAxis2->setVisible(true);
  plotcoordintates->yAxis2->setTickLabels(false);
  connect(plotcoordintates->xAxis, SIGNAL(rangeChanged(QCPRange)), plotcoordintates->xAxis2, SLOT(setRange(QCPRange)));
  connect(plotcoordintates->yAxis, SIGNAL(rangeChanged(QCPRange)), plotcoordintates->yAxis2, SLOT(setRange(QCPRange)));
  plotcoordintates->graph(0)->setLineStyle((QCPGraph::LineStyle)QCPGraph::lsLine);
  plotcoordintates->graph(0)->selectionDecorator()->setPen(QPen(Qt::red));
  plotcoordintates->graph(0)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle, 5));
  plotcoordintates->graph(1)->setLineStyle((QCPGraph::LineStyle)QCPGraph::lsLine);
  plotcoordintates->graph(1)->selectionDecorator()->setPen(QPen(Qt::magenta));
  plotcoordintates->graph(1)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle, 5));
  plotcoordintates->graph(2)->setLineStyle((QCPGraph::LineStyle)QCPGraph::lsLine);
  plotcoordintates->graph(2)->selectionDecorator()->setPen(QPen(Qt::green));
  plotcoordintates->graph(2)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle, 5));
  plotcoordintates->graph(3)->setLineStyle((QCPGraph::LineStyle)QCPGraph::lsLine);
  plotcoordintates->graph(3)->selectionDecorator()->setPen(QPen(Qt::black));
  plotcoordintates->graph(3)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle, 5));
  plotcoordintates->graph(4)->setLineStyle((QCPGraph::LineStyle)QCPGraph::lsLine);
  plotcoordintates->graph(4)->selectionDecorator()->setPen(QPen(Qt::blue));
  plotcoordintates->graph(4)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle, 5));
  plotcoordintates->graph(5)->setLineStyle((QCPGraph::LineStyle)QCPGraph::lsLine);
  plotcoordintates->graph(5)->selectionDecorator()->setPen(QPen(Qt::gray));
  plotcoordintates->graph(5)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle, 5));
  plotcoordintates->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectPlottables);

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
  const QRect availableGeometry = QApplication::desktop()->availableGeometry(plotcoordintates);
  plotcoordintates->resize(availableGeometry.width() / 3, availableGeometry.height() / 2);
  plotcoordintates->show();

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

  plotcoordintates->graph(0)->data()->clear();
  plotcoordintates->graph(1)->data()->clear();
  plotcoordintates->graph(2)->data()->clear();
  plotcoordintates->graph(3)->data()->clear();
  plotcoordintates->graph(4)->data()->clear();
  plotcoordintates->graph(5)->data()->clear();
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
  plotcoordintates->graph(0)->addData(time[iter] * 10, red[iter].first);
  plotcoordintates->graph(0)->rescaleAxes();
  plotcoordintates->graph(1)->addData(time[iter] * 10, red[iter].second);
  plotcoordintates->graph(1)->rescaleAxes();

  if(5 < red[iter].first && red[iter].first < image.width() - 5 &&
     5 < red[iter].second && red[iter].second < image.height() - 5) {
    for(int i = red[iter].first - 5; i <= red[iter].first + 5; i++) {
      for(int j = red[iter].second - 5; j <= red[iter].second + 5; j++) {
        image.setPixelColor(i, j, QColor(0, 0, 0, 0));
      }
    }
  }

  green[iter] = std::pair<int, int>(mean_green[0]/mean_green[2], mean_green[1]/mean_green[2]);
  plotcoordintates->graph(2)->addData(time[iter] * 10, green[iter].first);
  plotcoordintates->graph(2)->rescaleAxes();
  plotcoordintates->graph(3)->addData(time[iter] * 10, green[iter].second);
  plotcoordintates->graph(3)->rescaleAxes();

  if(5 < green[iter].first && green[iter].first < image.width() - 5 &&
     5 < green[iter].second && green[iter].second < image.height() - 5) {
    for(int i = green[iter].first - 5; i <= green[iter].first + 5; i++) {
      for(int j = green[iter].second - 5; j <= green[iter].second + 5; j++) {
        image.setPixelColor(i, j, QColor(0, 0, 0, 0));
      }
    }
  }

  blue[iter] = std::pair<int, int>(mean_blue[0]/mean_blue[2], mean_blue[1]/mean_blue[2]);
  plotcoordintates->graph(4)->addData(time[iter] * 10, blue[iter].first);
  plotcoordintates->graph(4)->rescaleAxes();
  plotcoordintates->graph(5)->addData(time[iter] * 10, blue[iter].second);
  plotcoordintates->graph(5)->rescaleAxes();

  if(5 < blue[iter].first && blue[iter].first < image.width() - 5 &&
     5 < blue[iter].second && blue[iter].second < image.height() - 5) {
    for(int i = blue[iter].first - 5; i <= blue[iter].first + 5; i++) {
      for(int j = blue[iter].second - 5; j <= blue[iter].second + 5; j++) {
        image.setPixelColor(i, j, QColor(0, 0, 0, 0));
      }
    }
  }

  plotcoordintates->replot();
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

  if(flag && iter % 2 == 0) {
    getAngularVelocity();
  }

  iter = (iter + 1) % 3;
}

void VideoPlayer::getAngularVelocity() {
  static double Cpast = std::numeric_limits<double>::max();

  double CpastCalc = 0;
  int count = 0;

  double omegax = 0;
  double omegay = 0;
  double omegaz = 0;
  double dt = time[iter] - time[(iter + 2) % 3];

  for(double fault21x = -10; fault21x <= 10; fault21x++) {
    for(double fault21y = -10; fault21y <= 10; fault21y++) {
      for(double fault31x = -10; fault31x <= 10; fault31x++) {
        for(double fault31y = -10; fault31y <= 10; fault31y++) {
          double x21 = green[iter].first - red[iter].first + fault21x, y21 = green[iter].second - red[iter].second + fault21y;
          double x31 = blue[iter].first - red[iter].first + fault31x, y31 = blue[iter].second - red[iter].second + fault31y;


          double dx21 = (x21 - (green[(iter + 2) % 3].first - red[(iter + 2) % 3].first)) / dt;
          double dy21 = (y21 - (green[(iter + 2) % 3].second - red[(iter + 2) % 3].second)) / dt;
          double dx31 = (x31 - (blue[(iter + 2) % 3].first - red[(iter + 2) % 3].first)) / dt;
          double dy31 = (y31 - (blue[(iter + 2) % 3].second - red[(iter + 2) % 3].second)) / dt;

          if(abs(x21*y31 - x31*y21) < ALMOST_ZERO) {
            continue;
          }
          double a = (y31 - y21)*(dx31*x31 + dy31*y31) / (x21*y31 - x31*y21);
          double b = -((y31 - y21)*(dx21*x31 + dy21*y31) / (x21*y31 - x31*y21) + dx31);
          double c = dx21;

          if(pow(b, 2) - 4*a*c < 0 || abs(a) < ALMOST_ZERO) {
            continue;
          }
          double Cpresent = (-b - pow((pow(b, 2) - 4*a*c), 0.5)) / (2*a);

          if(Cpast == std::numeric_limits<double>::max()) {
            CpastCalc += Cpresent;
            count++;
          } else {
            double dCpresent = (Cpresent - Cpast) / dt;
            if(abs(dCpresent) < ALMOST_ZERO)
              continue;

            double z31squared = (y21*(omegaz*x31 - dy31) - x21*(dx31 + omegaz*y31) +
                                 y31*(omegaz*x31 - dy31) - x31*(dx31 + omegaz*y31)) / dCpresent;
            if(z31squared < 0 || abs(z31squared) < ALMOST_ZERO)
              continue;

            double omega3 = (dx21*x31 + dy21*y31 - Cpresent*(dx31*x31 + dy31*y31)) / (x21*y31 - x31*y21);
            omegaz += omega3;
            omegax += ((omega3)*x31 - dy31) / pow(z31squared, 0.5);
            omegay += (dx31 + (omega3)*y31) / pow(z31squared, 0.5);

            CpastCalc += Cpresent;
            count++;
          }
        }
      }
    }
  }

  if(count != 0)
    Cpast = CpastCalc / count;
  else
    Cpast = std::numeric_limits<double>::max();

  plotangularvelocities->graph(0)->addData(time[iter], omegax / count);
  plotangularvelocities->graph(1)->addData(time[iter], omegay / count);
  plotangularvelocities->graph(2)->addData(time[iter], omegaz / count);
  plotangularvelocities->replot();
}
