#define _USE_MATH_DEFINES

#include "myutility.h"
#include <cmath>
#include <iostream>

//Triangle
Triangle::Triangle() :
  r1(0), r2(0), costh(0), sinth(0),
  alpha0(0), beta0(0), gamma0(0),
  signa(1), signb(1), signg(1),
  omegax{0}, omegay{0}, omegaz{0},
  currentIndex(0),
  adaptive(true) {}

void Triangle::setR1(double R1) {
  r1 = R1;
}

void Triangle::setR2(double R2) {
  r2 = R2;
}

void Triangle::setCosth(double Costh) {
  costh = Costh;
}

void Triangle::setSinth(double Sinth) {
  sinth = Sinth;
}

void Triangle::setAdaptive(bool flag) {
  adaptive = flag;
}

void Triangle::rotate(double x1, double y1, double x2, double y2, double dt) {

  if(adaptive && r1 <= std::pow(x1*x1 + y1*y1, 0.5) && r2 <= std::pow(x2*x2 + y2*y2, 0.5)) {
    r1 = std::pow(x1*x1 + y1*y1, 0.5);
    r2 = std::pow(x2*x2 + y2*y2, 0.5);
    costh = (x1*x2 + y1*y2) / (r1*r2);
    sinth = std::pow(1 - costh*costh, 0.5);
  }

  double alpha = 0;
  double beta = 0;
  double gamma = 0;

  double fault;
  int count;

  double step;
  //alpha
  for(int i = 1; i < 2; i++) {
    step = 0.001;
    step = (std::abs(std::abs(r1*std::cos(beta0)*std::cos(alpha0 + alpha)*x1 + r1*std::cos(beta0)*std::sin(alpha0 + alpha)*y1) - r1*std::pow(x1*x1 + y1*y1, 0.5)) -
        std::abs(std::abs(r1*std::cos(beta0)*std::cos(alpha0 + alpha + step)*x1 + r1*std::cos(beta0)*std::sin(alpha0 + alpha + step)*y1) - r1*std::pow(x1*x1 + y1*y1, 0.5))) / step;

    switch(i) {
      case 0:
        step = 0.1*signa;
        break;
      case 1:
        if(step != 0)
          step = step*0.001 / std::abs(step);
        break;
    }

    count = 0;
    while(true) {
      if(count > 1000)
        break;
      fault = std::abs(std::abs(r1*std::cos(beta0)*std::cos(alpha0 + alpha)*x1 + r1*std::cos(beta0)*std::sin(alpha0 + alpha)*y1) - r1*std::pow(x1*x1 + y1*y1, 0.5));
      if(std::abs(std::abs(r1*std::cos(beta0)*std::cos(alpha0 + alpha + step)*x1 + r1*std::cos(beta0)*std::sin(alpha0 + alpha + step)*y1) - r1*std::pow(x1*x1 + y1*y1, 0.5)) < fault) {
        alpha += step;
        step = (std::abs(std::abs(r1*std::cos(beta0)*std::cos(alpha0 + alpha)*x1 + r1*std::cos(beta0)*std::sin(alpha0 + alpha)*y1) - r1*std::pow(x1*x1 + y1*y1, 0.5)) -
            std::abs(std::abs(r1*std::cos(beta0)*std::cos(alpha0 + alpha + step)*x1 + r1*std::cos(beta0)*std::sin(alpha0 + alpha + step)*y1) - r1*std::pow(x1*x1 + y1*y1, 0.5))) / step;
        if(std::abs(step) != 0)
          step = step*0.001 / std::abs(step);
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
  if(int(beta0 / M_PI)*M_PI - M_PI*0.05 <= beta0 &&
     beta0 <= int(beta0 / M_PI)*M_PI + M_PI*0.05 && signb != 0) {
    beta = signb;
  }

  if(int(beta0 / M_PI + 1)*M_PI - M_PI*0.05 <= beta0 &&
     beta0 <= int(beta0 / M_PI + 1)*M_PI + M_PI*0.05 && signb != 0) {
    beta = signb;
  }

  for(int i = 1; i < 2; i++) {
    step = 0.001;
    step = (std::abs(r1*std::cos(beta0 + beta)*std::cos(alpha0 + alpha) - x1) + std::abs(r1*std::cos(beta0 + beta)*std::sin(alpha0 + alpha) - y1) -
            std::abs(r1*std::cos(beta0 + beta + step)*std::cos(alpha0 + alpha) - x1) - std::abs(r1*std::cos(beta0 + beta + step)*std::sin(alpha0 + alpha) - y1)) / step;
    if(step != 0) {
      switch(i) {
        case 0:
          step = 0.1*signb;
          break;
        case 1:
          step = step*0.001 / std::abs(step);
          break;
      }
    }

    count = 0;
    while(true) {
      if(count > 1000)
        break;
      fault = std::abs(r1*cos(beta0 + beta)*std::cos(alpha0 + alpha) - x1) + std::abs(r1*std::cos(beta0 + beta)*std::sin(alpha0 + alpha) - y1);
        if(std::abs(r1*std::cos(beta0 + beta + step)*std::cos(alpha0 + alpha) - x1) + std::abs(r1*std::cos(beta0 + beta + step)*std::sin(alpha0 + alpha) - y1) < fault) {
          beta += step;
          step = (std::abs(r1*std::cos(beta0 + beta)*std::cos(alpha0 + alpha) - x1) + std::abs(r1*std::cos(beta0 + beta)*std::sin(alpha0 + alpha) - y1) -
                  std::abs(r1*std::cos(beta0 + beta + step)*std::cos(alpha0 + alpha) - x1) - std::abs(r1*std::cos(beta0 + beta + step)*std::sin(alpha0 + alpha) - y1)) / step;
          if(step != 0)
            step = step*0.001/std::abs(step);
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
  if(int(gamma0 / M_PI)*M_PI - M_PI*0.05 <= gamma0 &&
     gamma0 <= int(gamma0 / M_PI)*M_PI + M_PI*0.05 && signg != 0) {
    gamma = signg;
  }

  if(int(gamma0 / M_PI + 1)*M_PI - M_PI*0.05 <= gamma0 &&
     gamma0 <= int(gamma0 / M_PI + 1)*M_PI + M_PI*0.05 && signg != 0) {
    gamma = signg;
  }
  for(int i = 1; i < 2; i++) {
    step = 0.001;
    step = (std::abs((r2*costh*std::cos(beta0 + beta) + r2*sinth*std::sin(beta0 + beta)*std::sin(gamma0 + gamma))*std::cos(alpha0 + alpha) -
                r2*sinth*std::sin(alpha0 + alpha)*std::cos(gamma0 + gamma) - x2) +
            std::abs((r2*costh*std::cos(beta0 + beta) + r2*sinth*std::sin(beta0 + beta)*std::sin(gamma0 + gamma))*std::sin(alpha0 + alpha) +
                r2*sinth*std::cos(alpha0 + alpha)*std::cos(gamma0 + gamma) - y2) -
            std::abs((r2*costh*std::cos(beta0 + beta) + r2*sinth*std::sin(beta0 + beta)*std::sin(gamma0 + gamma + step))*std::cos(alpha0 + alpha) -
                r2*sinth*std::sin(alpha0 + alpha)*std::cos(gamma0 + gamma + step) - x2) -
            std::abs((r2*costh*std::cos(beta0 + beta) + r2*sinth*std::sin(beta0 + beta)*std::sin(gamma0 + gamma + step))*std::sin(alpha0 + alpha) +
                r2*sinth*std::cos(alpha0 + alpha)*std::cos(gamma0 + gamma + step) - y2)) / step;

    switch(i) {
      case 0:
        step = 0.1*signg;
        break;
      case 1:
        if(step != 0)
          step = step*0.001 / std::abs(step);
        break;
    }

    count = 0;
    while(true) {
      if(count > 1000)
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
            step = step*0.001/std::abs(step);
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

  currentIndex = (currentIndex + 1) % 10;

  omegax[currentIndex] = (omegax[(10 + currentIndex - 1) % 10] + omegax[(10 + currentIndex - 2) % 10] +
      gamma*std::cos(beta0 + beta)*std::cos(alpha0 + alpha) / dt - std::sin(alpha0 + alpha)*beta / dt) / 3;

  omegay[currentIndex] = (omegay[(10 + currentIndex - 1) % 10] + omegay[(10 + currentIndex - 2) % 10] +
      gamma*std::cos(beta0 + beta)*std::sin(alpha0 + alpha) / dt + std::cos(alpha0 + alpha)*beta / dt) / 3;

  omegaz[currentIndex] = (omegaz[(10 + currentIndex - 1) % 10] + omegaz[(10 + currentIndex - 2) % 10] +
      alpha / dt - std::sin(beta0 + beta)*gamma / dt) / 3;

  signa = alpha; signb = beta; signg = gamma;

  alpha0 += alpha; beta0 += beta; gamma0 += gamma;

}

double Triangle::getOmegaX() {
  return omegax[currentIndex];
}

double Triangle::getOmegaY() {
  return omegay[currentIndex];
}

double Triangle::getOmegaZ() {
  return omegaz[currentIndex];
}

//Approximator
Approximator::Approximator() : coefficientsx(), coefficientsy() {}

void Approximator::approximate(std::vector<PointData> data, int n) {
  double systemeq[n + 1][n + 2];
  for(int component = 0;component < 2; component++) {
    for(int i = 0; i < n + 1; i++) {
      for(int j = 0; j < n + 2; j++) {
        systemeq[i][j] = 0;
        if(j == n + 1) {
          for(int k = 0; k < int(data.size()); k++) {
            if(component == 0)
              systemeq[i][j] += std::pow(data[k].t, n - i)*data[k].x;
            else
              systemeq[i][j] += std::pow(data[k].t, n - i)*data[k].y;
          }
        } else {
          for(int k = 0; k < int(data.size()); k++) {
              systemeq[i][j] += std::pow(data[k].t, 2*n - i - j);
          }
        }
      }
    }

    //Forward Elimination
    for(int i = 0; i < n + 1; i++) {
      for(int j = i + 1; j < n + 1; j++) {
        for(int k = n + 1; k >= 0; k--) {
          if(systemeq[i][i] == 0) {
            break;
          }
          systemeq[j][k] -= (systemeq[i][k]*systemeq[j][i])/systemeq[i][i];
        }
      }
      for(int j = n + 1; j >= 0; j--) {
        if(systemeq[i][i] == 0) {
          break;
        }
        systemeq[i][j] /= systemeq[i][i];
      }
    }

    //Back Substitution
    for(int i = n - 1; i >= 0; i--) {
      for(int j = i; j >= 0; j--) {
        for(int k = n + 1; k >= i + 1; k--) {
          systemeq[j][k] -= systemeq[i + 1][k]*systemeq[j][i+1];
        }
      }
    }
    if(component == 0) {
      coefficientsx.clear(); coefficientsx.shrink_to_fit();
      for(int i = n; i >= 0; i--) {
        coefficientsx.push_back(systemeq[i][n + 1]);
      }
    } else {
      coefficientsy.clear(); coefficientsy.shrink_to_fit();
      for(int i = n; i >= 0; i--) {
        coefficientsy.push_back(systemeq[i][n + 1]);
      }
    }
  }
}

double Approximator::getX(double t) {
  double result = 0;
  for(size_t i = 0; i < coefficientsx.size(); i++) {
    result += coefficientsx[i]*std::pow(t, i);
  }

  return result;
}

double Approximator::getY(double t) {
  double result = 0;
  for(size_t i = 0; i < coefficientsy.size(); i++) {
    result += coefficientsy[i]*std::pow(t, i);
  }

  return result;
}
