#ifndef VIDEOPLAYER_H
#define VIDEOPLAYER_H


#include "videowidgetsurface.h"
#include "qcustomplot.h"
#include "myutility.h"

#include <qmediaplayer.h>

#include <QtGui/QMovie>
#include <QtWidgets/QWidget>

class QAbstractButton;
class QSlider;
class QLabel;
class QUrl;

class VideoPlayer : public QWidget
{
    Q_OBJECT
public:
    VideoPlayer(QWidget *parent = 0);
    ~VideoPlayer();

    void setUrl(const QUrl &url);

public slots:
    void openFile();
    void play();
    void openPoint1();
    void openPoint2();
    void openPoint3();

private slots:
    void mediaStateChanged(QMediaPlayer::State state);
    void positionChanged(qint64 position);
    void durationChanged(qint64 duration);
    void setPosition(int position);
    void handleError();

    void setNewImage(QPixmap pix);

Q_SIGNALS:
    void ClearPixmap();

private:
    void capturePoints(QPixmap &pix);
    void drawCoordinates();
    void drawAngularVelocities();
    void approximateClicked();
    void testFunction();

    QMediaPlayer mediaPlayer;
    QAbstractButton *playButton;
    QSlider *positionSlider;
    QLabel *errorLabel;
    QLabel *imagelabel;
    QLabel *timelabel;
    QLabel *coordinatelabel;
    QCustomPlot *plotxy21;
    QCustomPlot *plotxy31;
    QCustomPlot *plotangularvelocities;
    QCustomPlot *plotxy21Approximation;
    QCustomPlot *plotxy31Approximation;
    QCustomPlot *plotangularvelocitiesApproximation;
    QCustomPlot *plotshapesofVectors;
    myQAbstractVideoSurface *surface;

    QColor point1Color;
    QColor point2Color;
    QColor point3Color;

    std::vector<PointData>red;
    std::vector<PointData>green;
    std::vector<PointData>blue;

    Triangle pixelTriangle;
    Triangle approximatePixelTriangle;
    Approximator greenredApproximator, blueredApproximator;
};

#endif
