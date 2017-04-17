#include "videoplayer.h"

#include <QtWidgets>
#include <qvideosurfaceformat.h>

#include <iostream>
#include <vector>
#include <string>
#include <cmath>
#include <limits>
#include <algorithm>
#include <thread>

#define ALMOST_ZERO 0.00001

VideoPlayer::VideoPlayer(QWidget *parent)
  : QWidget(parent)
  , mediaPlayer(0, QMediaPlayer::VideoSurface)
  , playButton(0)
  , positionSlider(0)
  , errorLabel(0)
  , pixelTriangle()
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

  QAbstractButton *plotAngularVelocitiesButton = new QPushButton(tr("angular velocities"));
  connect(plotAngularVelocitiesButton, &QAbstractButton::clicked, this, &VideoPlayer::drawAngularVelocities);

  QAbstractButton *approximateButton = new QPushButton(tr("approximate"));
  connect(approximateButton, &QAbstractButton::clicked, this, &VideoPlayer::approximateClicked);

  QAbstractButton *TestButton = new QPushButton(tr("test"));
  connect(TestButton, &QAbstractButton::clicked, this, &VideoPlayer::testFunction);

  QBoxLayout *pointLayout = new QHBoxLayout;
  pointLayout->setMargin(0);
  pointLayout->addWidget(point1Button);
  pointLayout->addWidget(point2Button);
  pointLayout->addWidget(point3Button);
  pointLayout->addWidget(plotCoordinatesButton);
  pointLayout->addWidget(plotAngularVelocitiesButton);
  pointLayout->addWidget(approximateButton);
  pointLayout->addWidget(TestButton);
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

  plotxy21Approximation = new QCustomPlot;
  plotxy21Approximation->addGraph(); plotxy21Approximation->addGraph();
  plotxy21Approximation->addGraph(); plotxy21Approximation->addGraph();
  plotxy21Approximation->graph(0)->setPen(QPen(Qt::red));
  plotxy21Approximation->graph(1)->setPen(QPen(Qt::green));
  plotxy21Approximation->graph(2)->setPen(QPen(Qt::magenta));
  plotxy21Approximation->graph(3)->setPen(QPen(Qt::cyan));
  plotxy21Approximation->xAxis2->setVisible(true);
  plotxy21Approximation->xAxis2->setTickLabels(false);
  plotxy21Approximation->yAxis2->setVisible(true);
  plotxy21Approximation->yAxis2->setTickLabels(false);
  connect(plotxy21Approximation->xAxis, SIGNAL(rangeChanged(QCPRange)), plotxy21Approximation->xAxis2, SLOT(setRange(QCPRange)));
  connect(plotxy21Approximation->yAxis, SIGNAL(rangeChanged(QCPRange)), plotxy21Approximation->yAxis2, SLOT(setRange(QCPRange)));
  plotxy21Approximation->graph(0)->setLineStyle((QCPGraph::LineStyle)QCPGraph::lsLine);
  plotxy21Approximation->graph(0)->selectionDecorator()->setPen(QPen(Qt::red));
  plotxy21Approximation->graph(0)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle, 5));
  plotxy21Approximation->graph(1)->setLineStyle((QCPGraph::LineStyle)QCPGraph::lsLine);
  plotxy21Approximation->graph(1)->selectionDecorator()->setPen(QPen(Qt::green));
  plotxy21Approximation->graph(1)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle, 5));
  plotxy21Approximation->graph(2)->setLineStyle((QCPGraph::LineStyle)QCPGraph::lsLine);
  plotxy21Approximation->graph(2)->selectionDecorator()->setPen(QPen(Qt::magenta));
  plotxy21Approximation->graph(2)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle, 5));
  plotxy21Approximation->graph(3)->setLineStyle((QCPGraph::LineStyle)QCPGraph::lsLine);
  plotxy21Approximation->graph(3)->selectionDecorator()->setPen(QPen(Qt::cyan));
  plotxy21Approximation->graph(3)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle, 5));
  plotxy21Approximation->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectPlottables);

  plotxy31Approximation = new QCustomPlot;
  plotxy31Approximation->addGraph(); plotxy31Approximation->addGraph();
  plotxy31Approximation->addGraph(); plotxy31Approximation->addGraph();
  plotxy31Approximation->graph(0)->setPen(QPen(Qt::red));
  plotxy31Approximation->graph(1)->setPen(QPen(Qt::blue));
  plotxy31Approximation->graph(2)->setPen(QPen(Qt::magenta));
  plotxy31Approximation->graph(3)->setPen(QPen(Qt::cyan));
  plotxy31Approximation->xAxis2->setVisible(true);
  plotxy31Approximation->xAxis2->setTickLabels(false);
  plotxy31Approximation->yAxis2->setVisible(true);
  plotxy31Approximation->yAxis2->setTickLabels(false);
  connect(plotxy31Approximation->xAxis, SIGNAL(rangeChanged(QCPRange)), plotxy31Approximation->xAxis2, SLOT(setRange(QCPRange)));
  connect(plotxy31Approximation->yAxis, SIGNAL(rangeChanged(QCPRange)), plotxy31Approximation->yAxis2, SLOT(setRange(QCPRange)));
  plotxy31Approximation->graph(0)->setLineStyle((QCPGraph::LineStyle)QCPGraph::lsLine);
  plotxy31Approximation->graph(0)->selectionDecorator()->setPen(QPen(Qt::red));
  plotxy31Approximation->graph(0)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle, 5));
  plotxy31Approximation->graph(1)->setLineStyle((QCPGraph::LineStyle)QCPGraph::lsLine);
  plotxy31Approximation->graph(1)->selectionDecorator()->setPen(QPen(Qt::blue));
  plotxy31Approximation->graph(1)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle, 5));
  plotxy31Approximation->graph(2)->setLineStyle((QCPGraph::LineStyle)QCPGraph::lsLine);
  plotxy31Approximation->graph(2)->selectionDecorator()->setPen(QPen(Qt::magenta));
  plotxy31Approximation->graph(2)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle, 5));
  plotxy31Approximation->graph(3)->setLineStyle((QCPGraph::LineStyle)QCPGraph::lsLine);
  plotxy31Approximation->graph(3)->selectionDecorator()->setPen(QPen(Qt::cyan));
  plotxy31Approximation->graph(3)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle, 5));
  plotxy31Approximation->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectPlottables);

  plotangularvelocitiesApproximation = new QCustomPlot;
  plotangularvelocitiesApproximation->addGraph(); plotangularvelocitiesApproximation->addGraph();
  plotangularvelocitiesApproximation->addGraph();
  plotangularvelocitiesApproximation->graph(0)->setPen(QPen(Qt::red));
  plotangularvelocitiesApproximation->graph(1)->setPen(QPen(Qt::green));
  plotangularvelocitiesApproximation->graph(2)->setPen(QPen(Qt::blue));
  plotangularvelocitiesApproximation->xAxis2->setVisible(true);
  plotangularvelocitiesApproximation->xAxis2->setTickLabels(false);
  plotangularvelocitiesApproximation->yAxis2->setVisible(true);
  plotangularvelocitiesApproximation->yAxis2->setTickLabels(false);
  connect(plotangularvelocitiesApproximation->xAxis, SIGNAL(rangeChanged(QCPRange)),
          plotangularvelocitiesApproximation->xAxis2, SLOT(setRange(QCPRange)));
  connect(plotangularvelocitiesApproximation->yAxis, SIGNAL(rangeChanged(QCPRange)),
          plotangularvelocitiesApproximation->yAxis2, SLOT(setRange(QCPRange)));
  plotangularvelocitiesApproximation->graph(0)->setLineStyle((QCPGraph::LineStyle)QCPGraph::lsLine);
  plotangularvelocitiesApproximation->graph(0)->selectionDecorator()->setPen(QPen(Qt::red));
  plotangularvelocitiesApproximation->graph(0)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle, 5));
  plotangularvelocitiesApproximation->graph(1)->setLineStyle((QCPGraph::LineStyle)QCPGraph::lsLine);
  plotangularvelocitiesApproximation->graph(1)->selectionDecorator()->setPen(QPen(Qt::green));
  plotangularvelocitiesApproximation->graph(1)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle, 5));
  plotangularvelocitiesApproximation->graph(2)->setLineStyle((QCPGraph::LineStyle)QCPGraph::lsLine);
  plotangularvelocitiesApproximation->graph(2)->selectionDecorator()->setPen(QPen(Qt::blue));
  plotangularvelocitiesApproximation->graph(2)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle, 5));
  plotangularvelocitiesApproximation->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectPlottables);

  plotshapesofVectors = new QCustomPlot;
  plotshapesofVectors->addGraph(); plotshapesofVectors->addGraph(); plotshapesofVectors->addGraph();
  plotshapesofVectors->addGraph();
  plotshapesofVectors->graph(0)->setPen(QPen(Qt::red));
  plotshapesofVectors->graph(1)->setPen(QPen(Qt::green));
  plotshapesofVectors->graph(2)->setPen(QPen(Qt::blue));
  plotshapesofVectors->graph(3)->setPen(QPen(Qt::black));
  plotshapesofVectors->xAxis2->setVisible(true);
  plotshapesofVectors->xAxis2->setTickLabels(false);
  plotshapesofVectors->yAxis2->setVisible(true);
  plotshapesofVectors->yAxis2->setTickLabels(false);
  connect(plotshapesofVectors->xAxis, SIGNAL(rangeChanged(QCPRange)),
          plotshapesofVectors->xAxis2, SLOT(setRange(QCPRange)));
  connect(plotshapesofVectors->yAxis, SIGNAL(rangeChanged(QCPRange)),
          plotshapesofVectors->yAxis2, SLOT(setRange(QCPRange)));
  plotshapesofVectors->graph(0)->setLineStyle((QCPGraph::LineStyle)QCPGraph::lsLine);
  plotshapesofVectors->graph(0)->selectionDecorator()->setPen(QPen(Qt::red));
  plotshapesofVectors->graph(0)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle, 5));
  plotshapesofVectors->graph(1)->setLineStyle((QCPGraph::LineStyle)QCPGraph::lsLine);
  plotshapesofVectors->graph(1)->selectionDecorator()->setPen(QPen(Qt::green));
  plotshapesofVectors->graph(1)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle, 5));
  plotshapesofVectors->graph(2)->setLineStyle((QCPGraph::LineStyle)QCPGraph::lsLine);
  plotshapesofVectors->graph(2)->selectionDecorator()->setPen(QPen(Qt::blue));
  plotshapesofVectors->graph(2)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle, 5));
  plotshapesofVectors->graph(3)->setLineStyle((QCPGraph::LineStyle)QCPGraph::lsLine);
  plotshapesofVectors->graph(3)->selectionDecorator()->setPen(QPen(Qt::black));
  plotshapesofVectors->graph(3)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle, 5));
  plotshapesofVectors->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectPlottables);

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

void VideoPlayer::approximateClicked() {
  plotxy21Approximation->graph(0)->data()->clear();
  plotxy21Approximation->graph(1)->data()->clear();
  plotxy21Approximation->graph(2)->data()->clear();
  plotxy21Approximation->graph(3)->data()->clear();

  plotxy31Approximation->graph(0)->data()->clear();
  plotxy31Approximation->graph(1)->data()->clear();
  plotxy31Approximation->graph(2)->data()->clear();
  plotxy31Approximation->graph(3)->data()->clear();

  plotangularvelocitiesApproximation->graph(0)->data()->clear();
  plotangularvelocitiesApproximation->graph(1)->data()->clear();
  plotangularvelocitiesApproximation->graph(2)->data()->clear();

  plotshapesofVectors->graph(0)->data()->clear();
  plotshapesofVectors->graph(1)->data()->clear();
  plotshapesofVectors->graph(2)->data()->clear();
  plotshapesofVectors->graph(3)->data()->clear();

  double t1, t2;
  t1 = QInputDialog::getDouble(0, "Input t1", "Input t1");
  t2 = QInputDialog::getDouble(0, "Input t2", "Input t2");
  if(t1 >= t2) {
    return;
  }
  std::vector<PointData> greenred;
  std::vector<PointData> bluered;
  for(size_t i = 0; i < red.size(); i++) {
    if(t1 <= red[i].t && red[i].t <= t2) {
      greenred.push_back(PointData(green[i].x - red[i].x, green[i].y - red[i].y, red[i].t));
      bluered.push_back(PointData(blue[i].x - red[i].x, blue[i].y - red[i].y, red[i].t));
    }
  }

  int n;
  n = QInputDialog::getInt(0, "Input degree", "Input degree");
  if(n < 0) {
    return;
  }
  double t;

  QRect availableGeometry = QApplication::desktop()->availableGeometry(plotxy21Approximation);
  plotxy21Approximation->resize(availableGeometry.width() / 3, availableGeometry.height() / 2);
  plotxy21Approximation->show();

  greenredApproximator.approximate(greenred, n);
  for(size_t i = 0; i < greenred.size(); i++) {
    t = greenred[i].t;

    plotxy21Approximation->graph(0)->addData(t, greenred[i].x);
    plotxy21Approximation->graph(1)->addData(t, greenred[i].y);

    plotxy21Approximation->graph(2)->addData(t, greenredApproximator.getX(t));
    plotxy21Approximation->graph(3)->addData(t, greenredApproximator.getY(t));

    plotxy21Approximation->rescaleAxes();
    plotxy21Approximation->replot();
  }

  availableGeometry = QApplication::desktop()->availableGeometry(plotxy31Approximation);
  plotxy31Approximation->resize(availableGeometry.width() / 3, availableGeometry.height() / 2);
  plotxy31Approximation->show();

  blueredApproximator.approximate(bluered, n);
  for(size_t i = 0; i < bluered.size(); i++) {
    t = greenred[i].t;

    plotxy31Approximation->graph(0)->addData(t, bluered[i].x);
    plotxy31Approximation->graph(1)->addData(t, bluered[i].y);

    plotxy31Approximation->graph(2)->addData(t, blueredApproximator.getX(t));
    plotxy31Approximation->graph(3)->addData(t, blueredApproximator.getY(t));

    plotxy31Approximation->rescaleAxes();
    plotxy31Approximation->replot();
  }

  availableGeometry = QApplication::desktop()->availableGeometry(plotangularvelocitiesApproximation);
  plotangularvelocitiesApproximation->resize(availableGeometry.width() / 3, availableGeometry.height() / 2);
  plotangularvelocitiesApproximation->show();

  approximatePixelTriangle.setR1(0); approximatePixelTriangle.setR2(0);
  for(t = t1; t <= t2; t += 0.01) {
    double dt = 0.01;
    approximatePixelTriangle.rotate(greenredApproximator.getX(t), greenredApproximator.getY(t),
                                    blueredApproximator.getX(t), blueredApproximator.getY(t), dt);

    plotangularvelocitiesApproximation->graph(0)->addData(t, approximatePixelTriangle.getOmegaX());
    plotangularvelocitiesApproximation->graph(1)->addData(t, approximatePixelTriangle.getOmegaY());
    plotangularvelocitiesApproximation->graph(2)->addData(t, approximatePixelTriangle.getOmegaZ());
    plotangularvelocitiesApproximation->rescaleAxes();
    plotangularvelocitiesApproximation->replot();
  }
  
  //high
  availableGeometry = QApplication::desktop()->availableGeometry(plotshapesofVectors);
  plotshapesofVectors->resize(availableGeometry.width() / 3, availableGeometry.height() / 2);
  plotshapesofVectors->show();
  for(t = t1; t <= t2; t += 0.01) {
    double x21 = greenredApproximator.getX(t), y21 = greenredApproximator.getY(t),
        dx21 = (greenredApproximator.getX(t + 0.001) - greenredApproximator.getX(t)) / 0.001, 
        dy21 = (greenredApproximator.getY(t + 0.001) - greenredApproximator.getY(t)) / 0.001;
    double x31 = blueredApproximator.getX(t), y31 = blueredApproximator.getY(t),
        dx31 = (blueredApproximator.getX(t + 0.001) - blueredApproximator.getX(t)) / 0.001, 
        dy31 = (blueredApproximator.getY(t + 0.001) - blueredApproximator.getY(t)) / 0.001;
    double a = dx31*x31 + dy31*y31, b = -(dx21*x31 + dy21*y31 + dx31*x21 + dy31*y21), c = dx21*x21 + dy21*y21;
    
    if(std::abs(a) > 5 && (x21*y31 - y21*x31) > 5) {
      double x21p = greenredApproximator.getX(t - 0.001), y21p = greenredApproximator.getY(t - 0.001),
          dx21p = (greenredApproximator.getX(t) - greenredApproximator.getX(t - 0.001)) / 0.001,
          dy21p = (greenredApproximator.getY(t) - greenredApproximator.getY(t - 0.001)) / 0.001;
      double x31p = blueredApproximator.getX(t - 0.001), y31p = blueredApproximator.getY(t - 0.001),
          dx31p = (blueredApproximator.getX(t) - blueredApproximator.getX(t - 0.001)) / 0.001,
          dy31p = (blueredApproximator.getY(t) - blueredApproximator.getY(t - 0.001)) / 0.001;
      double ap = dx31p*x31p + dy31p*y31p, bp = -(dx21p*x31p + dy21p*y31p + dx31p*x21p + dy31p*y21p), cp = dx21p*x21p + dy21p*y21p;

      double A = (-b + std::pow(std::pow(b, 2) - 4*a*c, 0.5)) / (2*a);
      double Ap = (-bp + std::pow(std::pow(bp, 2) - 4*ap*cp, 0.5)) / (2*ap);
      double dA = (A - Ap) / 0.001;
      double omega3 = (dx21*x31 + dy21*y31 - A*a) / (x21*y31 - y21*x31);
      double z31 = std::pow(((omega3*x31 - dy31)*(y21 - A*y31) - (dx31 + omega3*y31)*(x21 - A*x31)) / dA, 0.5);
      double z21 = A*z31;

      plotshapesofVectors->graph(0)->addData(t, std::pow(x21*x21 + y21*y21 + z21*z21, 0.5));
      plotshapesofVectors->graph(1)->addData(t, std::pow(x31*x31 + y31*y31 + z31*z31, 0.5));
      plotshapesofVectors->graph(2)->addData(t, approximatePixelTriangle.getR1());
      plotshapesofVectors->graph(3)->addData(t, approximatePixelTriangle.getR2());
      plotshapesofVectors->rescaleAxes();
      plotshapesofVectors->replot();
    } else {
      double A = (-b + std::pow(std::pow(b, 2) - 4*a*c, 0.5)) / (2*c);
    }
  }
}

void VideoPlayer::testFunction() {
  plotangularvelocities->graph(0)->data()->clear();
  plotangularvelocities->graph(1)->data()->clear();
  plotangularvelocities->graph(2)->data()->clear();

  int choose;
  choose = QInputDialog::getInt(0, "Choose test", "Choose test");
  if(choose < 0 || choose > 5)
    return;

  pixelTriangle.setR1(0);
  pixelTriangle.setR2(0);

  switch(choose) {
    case 0:
      for(double i = 0; i < 20; i += 0.05) {
        pixelTriangle.rotate(std::cos(i), 0, 0, 1, 0.05);
        plotangularvelocities->graph(0)->addData(i, pixelTriangle.getOmegaX());
        plotangularvelocities->graph(1)->addData(i, pixelTriangle.getOmegaY());
        plotangularvelocities->graph(2)->addData(i, pixelTriangle.getOmegaZ());
        plotangularvelocities->rescaleAxes();
        plotangularvelocities->replot();
      }
      break;
    case 1:
      for(double i = 0; i < 20; i += 0.05) {
        pixelTriangle.rotate(1, 0, 0, std::cos(i), 0.05);
        plotangularvelocities->graph(0)->addData(i, pixelTriangle.getOmegaX());
        plotangularvelocities->graph(1)->addData(i, pixelTriangle.getOmegaY());
        plotangularvelocities->graph(2)->addData(i, pixelTriangle.getOmegaZ());
        plotangularvelocities->rescaleAxes();
        plotangularvelocities->replot();
      }
      break;
    case 2:
      for(double i = 0; i < 20; i += 0.05) {
        pixelTriangle.rotate(std::cos(i), std::sin(i),
                             std::cos(3.14159265 / 2.0 + i), std::sin(3.14159265 / 2.0 + i), 0.05);
        plotangularvelocities->graph(0)->addData(i, pixelTriangle.getOmegaX());
        plotangularvelocities->graph(1)->addData(i, pixelTriangle.getOmegaY());
        plotangularvelocities->graph(2)->addData(i, pixelTriangle.getOmegaZ());
        plotangularvelocities->rescaleAxes();
        plotangularvelocities->replot();
      }
      break;
    case 3:
      for(double i = 0; i < 20; i += 0.05) {
        pixelTriangle.rotate(std::cos(i)*std::cos(i / 2.0), std::sin(i)*std::cos(i / 2.0),
                             std::cos(3.14159265 / 2.0 + i), std::sin(3.14159265 / 2.0 + i), 0.05);

        plotangularvelocities->graph(0)->addData(i, pixelTriangle.getOmegaX());
        plotangularvelocities->graph(1)->addData(i, pixelTriangle.getOmegaY());
        plotangularvelocities->graph(2)->addData(i, pixelTriangle.getOmegaZ());
        plotangularvelocities->rescaleAxes();
        plotangularvelocities->replot();
      }
      break;
    case 4:
      for(double i = 0; i < 20; i += 0.05) {
        pixelTriangle.rotate(std::cos(i*(-1)), std::sin(i*(-1)),
                             std::cos(3.14159265 / 2.0 - i)*std::cos((-1)*i / 2.0), std::sin(3.14159265 / 2.0 - i)*std::cos((-1)*i / 2.0), 0.05);

        plotangularvelocities->graph(0)->addData(i, pixelTriangle.getOmegaX());
        plotangularvelocities->graph(1)->addData(i, pixelTriangle.getOmegaY());
        plotangularvelocities->graph(2)->addData(i, pixelTriangle.getOmegaZ());
        plotangularvelocities->rescaleAxes();
        plotangularvelocities->replot();
      }
      break;
    case 5:
      for(double i = 0; i < 20; i += 0.05) {
        if(i < 10) {
          pixelTriangle.rotate(std::cos(i*(-1)), std::sin(i*(-1)),
                               std::cos(3.14159265 / 2.0 - i)*std::cos((-1)*i / 2.0), std::sin(3.14159265 / 2.0 - i)*std::cos((-1)*i / 2.0), 0.05);
        } else {
          pixelTriangle.rotate(std::cos(i*(-1)), std::sin(i*(-1)),
                               std::cos(3.14159265 / 2.0 - i)*std::cos(i / 2.0), std::sin(3.14159265 / 2.0 - i)*std::cos(i / 2.0), 0.05);
        }
        plotangularvelocities->graph(0)->addData(i, pixelTriangle.getOmegaX());
        plotangularvelocities->graph(1)->addData(i, pixelTriangle.getOmegaY());
        plotangularvelocities->graph(2)->addData(i, pixelTriangle.getOmegaZ());
        plotangularvelocities->rescaleAxes();
        plotangularvelocities->replot();
      }
      break;
  }
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

  plotangularvelocities->graph(0)->data()->clear();
  plotangularvelocities->graph(1)->data()->clear();
  plotangularvelocities->graph(2)->data()->clear();

  pixelTriangle.setR1(0); pixelTriangle.setR2(0);

  red.clear(); red.shrink_to_fit(); green.clear(); green.shrink_to_fit(); blue.clear(); blue.shrink_to_fit();
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
  QImage image = pix.toImage();

  if(image.isNull())
    return;

  double mean_red[3] = {0};
  std::vector<std::pair<double, double> > red_points;

  double mean_green[3] = {0};
  std::vector<std::pair<double, double> > green_points;

  double mean_blue[3] = {0};
  std::vector<std::pair<double, double> > blue_points;

  for(int i = 0; i < image.width(); i++) {
    for(int j = 0; j < image.height(); j++) {
      if(abs(point1Color.red() - image.pixelColor(i, j).red()) < 20 &&
         abs(point1Color.green() - image.pixelColor(i, j).green()) < 20 &&
         abs(point1Color.blue() - image.pixelColor(i, j).blue()) < 20) {
        image.setPixelColor(i, j, QColor(255, 0, 0, 0));
        mean_red[0] += i; mean_red[1] += j; mean_red[2]++;
        red_points.push_back(std::pair<double, double>(i, j));
      } else if(abs(point2Color.red() - image.pixelColor(i, j).red()) < 20 &&
         abs(point2Color.green() - image.pixelColor(i, j).green()) < 20 &&
         abs(point2Color.blue() - image.pixelColor(i, j).blue()) < 20) {
        image.setPixelColor(i, j, QColor(0, 255, 0, 0));
        mean_green[0] += i; mean_green[1] += j; mean_green[2]++;
        green_points.push_back(std::pair<double, double>(i, j));
      } else if(abs(point3Color.red() - image.pixelColor(i, j).red()) < 20 &&
         abs(point3Color.green() - image.pixelColor(i, j).green()) < 20 &&
         abs(point3Color.blue() - image.pixelColor(i, j).blue()) < 20) {
        image.setPixelColor(i, j, QColor(0, 0, 255, 0));
        mean_blue[0] += i; mean_blue[1] += j; mean_blue[2]++;
        blue_points.push_back(std::pair<double, double>(i, j));
      }
    }
  }

  //red
  for(auto it = red_points.begin(); it != red_points.end(); it++) {
    if(std::pow(it->first - mean_red[0] / mean_red[2], 2) + std::pow(it->second - mean_red[1] / mean_red[2], 2) > 8100) {
      it->first = -1; it->second = -1;
    }
  }
  red_points.erase(std::remove(red_points.begin(), red_points.end(), std::pair<double, double>(-1, -1)), red_points.end());
  mean_red[0] = 0; mean_red[1] = 0; mean_red[2] = 0;
  for(auto it = red_points.begin(); it != red_points.end(); it++) {
    mean_red[0] += it->first; mean_red[1] += it->second; mean_red[2]++;
  }

  //green
  for(auto it = green_points.begin(); it != green_points.end(); it++) {
    if(std::pow(it->first - mean_green[0] / mean_green[2], 2) + std::pow(it->second - mean_green[1] / mean_green[2], 2) > 8100) {
      it->first = -1; it->second = -1;
    }
  }
  green_points.erase(std::remove(green_points.begin(), green_points.end(), std::pair<double, double>(-1, -1)), green_points.end());
  mean_green[0] = 0; mean_green[1] = 0; mean_green[2] = 0;
  for(auto it = green_points.begin(); it != green_points.end(); it++) {
    mean_green[0] += it->first; mean_green[1] += it->second; mean_green[2]++;
  }

  //blue
  for(auto it = blue_points.begin(); it != blue_points.end(); it++) {
    if(std::pow(it->first - mean_blue[0] / mean_blue[2], 2) + std::pow(it->second - mean_blue[1] / mean_blue[2], 2) > 8100) {
      it->first = -1; it->second = -1;
    }
  }
  blue_points.erase(std::remove(blue_points.begin(), blue_points.end(), std::pair<double, double>(-1, -1)), blue_points.end());
  mean_blue[0] = 0; mean_blue[1] = 0; mean_blue[2] = 0;
  for(auto it = blue_points.begin(); it != blue_points.end(); it++) {
    mean_blue[0] += it->first; mean_blue[1] += it->second; mean_blue[2]++;
  }

  if(mean_red[2] == 0 || mean_green[2] == 0 || mean_blue[2] == 0) {
    pix = QPixmap::fromImage(image);
    return;
  }

  double capture_time = double(mediaPlayer.position())/1000.0;
  timelabel->setText((std::string("time: ") + std::to_string(capture_time)).c_str());

  red.push_back(PointData(mean_red[0] / mean_red[2], mean_red[1] / mean_red[2], capture_time));
  double tick = red.size() - 1;
  if(5 < red[tick].x && red[tick].x < image.width() - 5 &&
     5 < red[tick].y && red[tick].y < image.height() - 5) {
    for(int i = red[tick].x - 5; i <= red[tick].x + 5; i++) {
      for(int j = red[tick].y - 5; j <= red[tick].y + 5; j++) {
        image.setPixelColor(i, j, QColor(0, 0, 0, 0));
      }
    }
  }

  green.push_back(PointData(mean_green[0] / mean_green[2], mean_green[1] / mean_green[2], capture_time));
  tick = green.size() - 1;
  if(5 < green[tick].x && green[tick].y < image.width() - 5 &&
     5 < green[tick].y && green[tick].y < image.height() - 5) {
    for(int i = green[tick].x - 5; i <= green[tick].x + 5; i++) {
      for(int j = green[tick].y - 5; j <= green[tick].y + 5; j++) {
        image.setPixelColor(i, j, QColor(0, 0, 0, 0));
      }
    }
  }

  blue.push_back(PointData(mean_blue[0] / mean_blue[2], mean_blue[1] / mean_blue[2], capture_time));
  tick = blue.size() - 1;
  if(5 < blue[tick].x && blue[tick].x < image.width() - 5 &&
     5 < blue[tick].y && blue[tick].y < image.height() - 5) {
    for(int i = blue[tick].x - 5; i <= blue[tick].x + 5; i++) {
      for(int j = blue[tick].y - 5; j <= blue[tick].y + 5; j++) {
        image.setPixelColor(i, j, QColor(0, 0, 0, 0));
      }
    }
  }

  plotxy21->graph(0)->addData(capture_time, green[tick].x - red[tick].x);
  plotxy21->graph(1)->addData(capture_time, green[tick].y - red[tick].y);
  plotxy21->rescaleAxes();

  plotxy31->graph(0)->addData(capture_time, blue[tick].x - red[tick].x);
  plotxy31->graph(1)->addData(capture_time, blue[tick].y - red[tick].y);
  plotxy31->rescaleAxes();

  plotxy21->replot();
  plotxy31->replot();
  pix = QPixmap::fromImage(image);

  //angularVelocity
  if(red.size() > 2) {
    size_t tick = red.size() - 1;

    double x21 = green[tick].x - red[tick].x, y21 = green[tick].y - red[tick].y;
    double x31 = blue[tick].x - red[tick].x, y31 = blue[tick].y - red[tick].y;
    double dt = red[tick].t - red[tick - 1].t;

    pixelTriangle.rotate(x21, y21, x31, y31, dt);

    plotangularvelocities->graph(0)->addData(capture_time, pixelTriangle.getOmegaX());
    plotangularvelocities->graph(1)->addData(capture_time, pixelTriangle.getOmegaY());
    plotangularvelocities->graph(2)->addData(capture_time, pixelTriangle.getOmegaZ());
    plotangularvelocities->rescaleAxes();
    plotangularvelocities->replot();
  }
}
