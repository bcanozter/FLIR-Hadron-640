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

  connect(connectAction_, &QAction::triggered, this,
          &MainWindow::handleConnect_);
  connect(disconnectAction_, &QAction::triggered, this,
          &MainWindow::handleDisconnect_);

  QToolBar *toolbar = addToolBar(tr("Connection"));

  toolbar->addAction(connectAction_);
  toolbar->addAction(disconnectAction_);

  connect(&videoDevice_, &VideoDevice::frameReceived, this,
          &MainWindow::handleIncomingFrame_);
  connect(&videoDevice_, &VideoDevice::deviceError, this,
          &MainWindow::handleVideoError_);

  setConnectedUi_(false);
}

void MainWindow::setConnectedUi_(bool connected) {
  connectAction_->setEnabled(!connected);
  disconnectAction_->setEnabled(connected);
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
  if (!openSerialPort(controlPort_, controlPortPath,
                      dialog.getControlPortBaudRate(), error)) {
    qWarning() << "Unable to open control port" << controlPortPath << ":"
               << error;
    QMessageBox::warning(this, tr("Connect"),
                         tr("Control port %1: %2").arg(controlPortPath, error));
    return;
  }
  if (!videoDevice_.open(videoPath)) {
    controlPort_.close();
    QMessageBox::warning(this, tr("Connect"),
                         tr("Unable to open video device %1.").arg(videoPath));
    return;
  }

  setConnectedUi_(true);
}

void MainWindow::handleDisconnect_() {
  videoDevice_.close();
  controlPort_.close();
  rawFrameView_->clear();
  setConnectedUi_(false);
}

void MainWindow::handleIncomingFrame_(const RawFrame &frame) {
  rawFrameView_->setFrame(frame);
}

void MainWindow::handleVideoError_() {
  handleDisconnect_();
  QMessageBox::critical(this, tr("Video"), tr("Video stream stopped."));
}
