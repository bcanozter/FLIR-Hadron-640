#pragma once

#include "../core/ControlPort.h"
#include "../core/RawFrame.h"
#include "../core/VideoDevice.h"
#include <QMainWindow>
#include <QPushButton>
#include <QtSerialPort/QSerialPort>

class QLabel;
class RawFrameView;

class MainWindow : public QMainWindow {
  Q_OBJECT
public:
  explicit MainWindow(QWidget *parent = nullptr);

private slots:
  void handleConnect_();
  void handleDisconnect_();
  void handleIncomingFrame_(const Y16Frame &frame);
  void handleIncomingYuvFrame_(const YU12Frame &frame);
  void handleVideoError_();

  void handleBlackHotConfig_();
  void handleWhiteHotConfig_();
  void handleSerialNumber_();
  void handleRunFfc_();
  void handleControlResponse_(uint32_t functionId,
                              const std::vector<uint8_t> &data);

private:
  void setConnectedUi_(bool connected);

  RawFrameView *rawFrameView_;
  QLabel *frameLabel_;

  QAction *connectAction_;
  QAction *disconnectAction_;
  QPushButton *blackHotOutButton_;
  QPushButton *whiteHotOutButton_;
  QPushButton *serialNumberButton_;
  QPushButton *runFfcButton_;

  ControlPort controlPort_;
  VideoDevice videoDevice_;
};
