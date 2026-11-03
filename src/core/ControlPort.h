#pragma once

#include <QObject>
#include <QtSerialPort/QSerialPort>

#include <cstdint>
#include <vector>

#include "BosonProtocol.h"

class ControlPort : public QObject {
  Q_OBJECT
public:
  explicit ControlPort(QObject *parent = nullptr);

  bool connectPort(const QString &systemLocation, qint32 baudRate,
                   QString &errorMessage);
  void disconnectPort();
  bool isConnected() const { return port_.isOpen(); }
  void sendCommand(uint32_t functionId, const std::vector<uint8_t> &data = {});

signals:
  void responseReceived(uint32_t functionId, const std::vector<uint8_t> &data);

private slots:
  void onReadyRead();

private:
  void handleFrame_(const std::vector<uint8_t> &stuffedFrame);

  QSerialPort port_;
  std::vector<uint8_t> rxBuffer_;
  BosonContext ctx_{};
};
