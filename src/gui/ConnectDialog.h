#pragma once

#include <QDialog>

class QComboBox;

class ConnectDialog : public QDialog {
  Q_OBJECT
public:
  explicit ConnectDialog(QWidget *parent = nullptr);

  QString getControlPortAddress() const;
  int getControlPortBaudRate() const;
  QString getVideoDevicePath() const;
  uint32_t getVideoPixelFormat() const;

private slots:
  void handleRefresh_();

private:
  QComboBox *controlPortCombo_;
  QComboBox *controlPortBaudCombo_;
  QComboBox *videoDeviceCombo_;
  QComboBox *videoFormatCombo_;
};
