#pragma once

#include <QMainWindow>
#include <QtSerialPort/QSerialPort>

#include "../core/RawFrame.h"
#include "../core/VideoDevice.h"

class QLabel;
class RawFrameView;

class MainWindow : public QMainWindow {
  Q_OBJECT
public:
  explicit MainWindow(QWidget *parent = nullptr);

private slots:
  void handleConnect_();
  void handleDisconnect_();
  void handleIncomingFrame_(const RawFrame &frame);
  void handleVideoError_();

private:
  void setConnectedUi_(bool connected);

  RawFrameView *rawFrameView_;
  QLabel *frameLabel_;

  QAction *connectAction_;
  QAction *disconnectAction_;

  QSerialPort controlPort_;
  VideoDevice videoDevice_;
};
