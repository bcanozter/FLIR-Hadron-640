#include "ConnectDialog.h"

#include <QComboBox>
#include <QDebug>
#include <QDialogButtonBox>
#include <QElapsedTimer>
#include <QFormLayout>
#include <QPushButton>
#include <QVBoxLayout>

#include "../core/SerialPort.h"
#include "../core/VideoDevice.h"

const int kCommonBaudRates[] = {9600,   19200,  38400,  57600,
                                115200, 230400, 460800, 921600};

QComboBox *makeBaudCombo(QWidget *parent, int defaultBaud) {
  auto *combo = new QComboBox(parent);

  for (int baudRate : kCommonBaudRates) {
    combo->addItem(QString::number(baudRate), baudRate);
  }

  const int idx = combo->findData(defaultBaud);
  combo->setCurrentIndex(idx >= 0 ? idx : 0);

  return combo;
}

void restoreSelection(QComboBox *combo, const QString &previous) {

  if (previous.isEmpty()) {
    return;
  }

  const int idx = combo->findData(previous);

  if (idx >= 0) {
    combo->setCurrentIndex(idx);
  }
}

ConnectDialog::ConnectDialog(QWidget *parent) : QDialog(parent) {
  setWindowTitle(tr("Connect to Device"));
  resize(520, 200);

  controlPortCombo_ = new QComboBox(this);
  controlPortBaudCombo_ = makeBaudCombo(this, kBosonBaudRate);

  videoDeviceCombo_ = new QComboBox(this);

  auto *refreshButton = new QPushButton(tr("Refresh"), this);
  connect(refreshButton, &QPushButton::clicked, this,
          &ConnectDialog::handleRefresh_);

  auto *form = new QFormLayout;
  form->addRow(tr("Control port (serial):"), controlPortCombo_);
  form->addRow(tr("Baud rate:"), controlPortBaudCombo_);
  form->addRow(tr("Video device (V4L2):"), videoDeviceCombo_);

  auto *buttons = new QDialogButtonBox(
      QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
  connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
  connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);

  auto *layout = new QVBoxLayout(this);
  layout->addLayout(form);
  layout->addWidget(refreshButton);
  layout->addWidget(buttons);

  handleRefresh_();
}

void ConnectDialog::handleRefresh_() {
  const QString prevControlPort = controlPortCombo_->currentData().toString();
  const QString prevVideoDevice = videoDeviceCombo_->currentData().toString();

  controlPortCombo_->clear();
  videoDeviceCombo_->clear();

  for (const AvailablePort &p : getAvailablePorts()) {
    controlPortCombo_->addItem(
        QStringLiteral("%1 (%2)").arg(p.systemLocation, p.description),
        p.systemLocation);
  }

  bool bosonVideoSelected = false;

  for (const AvailableVideoDevice &d : getAvailableVideoDevices()) {
    videoDeviceCombo_->addItem(QStringLiteral("%1 (%2)").arg(d.path, d.name),
                               d.path);

    if (!bosonVideoSelected && d.name.contains(QStringLiteral("Boson"))) {
      videoDeviceCombo_->setCurrentIndex(videoDeviceCombo_->count() - 1);
      bosonVideoSelected = true;
    }
  }

  restoreSelection(controlPortCombo_, prevControlPort);
  restoreSelection(videoDeviceCombo_, prevVideoDevice);
}

QString ConnectDialog::getControlPortAddress() const {
  return controlPortCombo_->currentData().toString();
}

int ConnectDialog::getControlPortBaudRate() const {
  return controlPortBaudCombo_->currentData().toInt();
}

QString ConnectDialog::getVideoDevicePath() const {
  return videoDeviceCombo_->currentData().toString();
}
