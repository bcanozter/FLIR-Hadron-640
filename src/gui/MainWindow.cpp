#include "MainWindow.h"

#include <QAction>
#include <QDebug>
#include <QLabel>
#include <QMessageBox>
#include <QToolBar>

#include "../core/SerialPort.h"
#include "ConnectDialog.h"
#include "RawFrameView.h"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {

  setWindowTitle(tr("FLIR Hadron 640"));
  resize(1000, 860);

  rawFrameView_ = new RawFrameView(this);
  setCentralWidget(rawFrameView_);

  connectAction_ = new QAction(tr("Connect"), this);
  disconnectAction_ = new QAction(tr("Disconnect"), this);
  blackHotOutButton_ = new QPushButton("BlackHot", this);
  whiteHotOutButton_ = new QPushButton("WhiteHot", this);
  serialNumberButton_ = new QPushButton("SerialNo", this);
  runFfcButton_ = new QPushButton("FLAT Field", this);

  connect(connectAction_, &QAction::triggered, this,
          &MainWindow::handleConnect_);
  connect(disconnectAction_, &QAction::triggered, this,
          &MainWindow::handleDisconnect_);
  connect(blackHotOutButton_, &QPushButton::clicked, this,
          &MainWindow::handleBlackHotConfig_);
  connect(whiteHotOutButton_, &QPushButton::clicked, this,
          &MainWindow::handleWhiteHotConfig_);
  connect(serialNumberButton_, &QPushButton::clicked, this,
          &MainWindow::handleSerialNumber_);
  connect(runFfcButton_, &QPushButton::clicked, this,
          &MainWindow::handleRunFfc_);
  connect(&controlPort_, &ControlPort::responseReceived, this,
          &MainWindow::handleControlResponse_);

  QToolBar *toolbar = addToolBar(tr("Connection"));

  toolbar->addAction(connectAction_);
  toolbar->addAction(disconnectAction_);

  // TODO: remove command buttons from this toolbar
  toolbar->addWidget(blackHotOutButton_);
  toolbar->addWidget(whiteHotOutButton_);
  toolbar->addWidget(serialNumberButton_);
  toolbar->addWidget(runFfcButton_);

  connect(&videoDevice_, &VideoDevice::Y16FrameReceived, this,
          &MainWindow::handleIncomingFrame_);
  connect(&videoDevice_, &VideoDevice::YU12FrameReceived, this,
          &MainWindow::handleIncomingYuvFrame_);
  connect(&videoDevice_, &VideoDevice::deviceError, this,
          &MainWindow::handleVideoError_);

  setConnectedUi_(false);
}

void MainWindow::setConnectedUi_(bool connected) {
  connectAction_->setEnabled(!connected);
  disconnectAction_->setEnabled(connected);
  blackHotOutButton_->setEnabled(connected);
  whiteHotOutButton_->setEnabled(connected);
  serialNumberButton_->setEnabled(connected);
  runFfcButton_->setEnabled(connected);
}

void MainWindow::handleConnect_() {
  ConnectDialog dialog(this);
  if (dialog.exec() != QDialog::Accepted) {
    return;
  }

  const QString controlPortPath = dialog.getControlPortAddress();
  const QString videoPath = dialog.getVideoDevicePath();
  if (controlPortPath.isEmpty() || videoPath.isEmpty()) {
    QMessageBox::warning(this, tr("Connect"),
                         tr("Select both a control port and a video device."));
    return;
  }

  QString error;
  if (!controlPort_.connectPort(controlPortPath,
                                dialog.getControlPortBaudRate(), error)) {
    qWarning() << "Unable to open control port" << controlPortPath << ":"
               << error;
    QMessageBox::warning(this, tr("Connect"),
                         tr("Control port %1: %2").arg(controlPortPath, error));
    return;
  }
  if (!videoDevice_.open(videoPath, dialog.getVideoPixelFormat())) {
    controlPort_.disconnectPort();
    QMessageBox::warning(this, tr("Connect"),
                         tr("Unable to open video device %1.").arg(videoPath));
    return;
  }

  setConnectedUi_(true);
}

void MainWindow::handleDisconnect_() {
  videoDevice_.close();
  controlPort_.disconnectPort();
  rawFrameView_->clear();
  setConnectedUi_(false);
}

void MainWindow::handleIncomingFrame_(const Y16Frame &frame) {
  rawFrameView_->setFrame(frame);
}

void MainWindow::handleIncomingYuvFrame_(const YU12Frame &frame) {
  rawFrameView_->setYU12Frame(frame);
}

void MainWindow::handleVideoError_() {
  handleDisconnect_();
  QMessageBox::critical(this, tr("Video"), tr("Video stream stopped."));
}

void MainWindow::handleBlackHotConfig_() {
  qDebug() << "send blackhot config command..";
  controlPort_.sendCommand(boson::cmd::kBosonOutput, {0x00, 0x00, 0x00, 0x01});
}

void MainWindow::handleWhiteHotConfig_() {
  qDebug() << "send whitehot config command...";
  controlPort_.sendCommand(boson::cmd::kBosonOutput, {0x00, 0x00, 0x00, 0x00});
}

void MainWindow::handleSerialNumber_() {
  controlPort_.sendCommand(boson::cmd::kBosonSerialNumber);
}

void MainWindow::handleRunFfc_() {
  controlPort_.sendCommand(boson::cmd::kBosonFLATField);
}

void MainWindow::handleControlResponse_(uint32_t functionId,
                                        const std::vector<uint8_t> &data) {
  if (functionId == boson::cmd::kBosonSerialNumber && data.size() == 4) {
    qInfo() << "Camera serial number:"
            << ((uint32_t(data[0]) << 24) | (uint32_t(data[1]) << 16) |
                (uint32_t(data[2]) << 8) | data[3]);
  }
}
