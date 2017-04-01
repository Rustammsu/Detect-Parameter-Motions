#ifndef UTILITY_H
#define UTILITY_H

#include <vector>

class PointData {
public:
  PointData(double x = 0, double y = 0, double t = 0) : x(x), y(y), t(t) {}
  double x;
  double y;
  double t;
};

class Triangle {
public:
  Triangle();
  void setR1(double R1);
  void setR2(double R2);
  void setCosth(double Costh);
  void setSinth(double Sinth);
  void setAdaptive(bool flag);
  void rotate(double x1, double y1, double x2, double y2, double dt);
  double getOmegaX();
  double getOmegaY();
  double getOmegaZ();

private:
  double r1, r2;
  double costh, sinth;
  double alpha0, beta0, gamma0;
  double signa, signb, signg;
  double omegax[10], omegay[10], omegaz[10];

  int currentIndex;

  bool adaptive;

};

class Approximator {
public:
  Approximator();
  void approximate(std::vector<PointData> data, int n);
  double getX(double t);
  double getY(double t);
private:
  std::vector<double> coefficientsx;
  std::vector<double> coefficientsy;
};

#endif // UTILITY_H

