#include "ControlPort.h"
#include "SerialPort.h"

#include <QDebug>

#include <algorithm>
#include <cstring>

#define BOSON_FRAME_START_BYTE 0x8E
#define BOSON_FRAME_END_BYTE 0xAE

ControlPort::ControlPort(QObject *parent) : QObject(parent) {
  connect(&port_, &QSerialPort::readyRead, this, &ControlPort::onReadyRead);
}

bool ControlPort::connectPort(const QString &systemLocation, qint32 baudRate,
                              QString &errorMessage) {
  rxBuffer_.clear();
  return openSerialPort(port_, systemLocation, baudRate, errorMessage);
}

void ControlPort::disconnectPort() {
  port_.close();
  rxBuffer_.clear();
}

void ControlPort::sendCommand(uint32_t functionId,
                              const std::vector<uint8_t> &data) {
  if (!isConnected()) {
    qWarning() << "Control port is not connected.";
    return;
  }

  Boson_Build_FSLP(&ctx_, functionId, data.data(), data.size());
  Boson_BitStuffing(&ctx_);

  qDebug() << "[TX] seq" << ctx_.sequence << "fn" << Qt::hex << Qt::showbase
           << functionId << "|"
           << QByteArray::fromRawData(
                  reinterpret_cast<const char *>(ctx_.boson_stuffed_package),
                  ctx_.boson_stuffed_package_len)
                  .toHex(' ');

  const qint64 written =
      port_.write(reinterpret_cast<const char *>(ctx_.boson_stuffed_package),
                  ctx_.boson_stuffed_package_len);
  if (written != ctx_.boson_stuffed_package_len) {
    qWarning() << "[TX] write failed:" << port_.errorString();
  }
}

void ControlPort::onReadyRead() {
  const QByteArray data = port_.readAll();
  qDebug() << "[RX] raw" << data.toHex(' ').toUpper();
  rxBuffer_.insert(rxBuffer_.end(), data.cbegin(), data.cend());

  while (true) {
    const auto start =
        std::find(rxBuffer_.begin(), rxBuffer_.end(), BOSON_FRAME_START_BYTE);
    if (start != rxBuffer_.begin()) {
      qWarning() << "[RX] discarded" << std::distance(rxBuffer_.begin(), start)
                 << "bytes outside a frame";
      rxBuffer_.erase(rxBuffer_.begin(), start);
    }
    if (rxBuffer_.empty()) {
      return;
    }
    const auto end =
        std::find(rxBuffer_.begin() + 1, rxBuffer_.end(), BOSON_FRAME_END_BYTE);
    if (end == rxBuffer_.end()) {
      return;
    }
    const std::vector<uint8_t> frame(rxBuffer_.begin(), end + 1);
    rxBuffer_.erase(rxBuffer_.begin(), end + 1);
    handleFrame_(frame);
  }
}

void ControlPort::handleFrame_(const std::vector<uint8_t> &stuffedFrame) {
  if (stuffedFrame.size() > sizeof(ctx_.boson_stuffed_package)) {
    return;
  }
  std::memcpy(ctx_.boson_stuffed_package, stuffedFrame.data(),
              stuffedFrame.size());
  ctx_.boson_stuffed_package_len =
      static_cast<unsigned short>(stuffedFrame.size());

  Boson_BitUnstuffing(&ctx_);
  const int ret = Boson_Check_Received_Frame(&ctx_, ctx_.aux_boson_package,
                                             ctx_.aux_boson_package_len);

  const unsigned char *pkg = ctx_.aux_boson_package;
  const uint32_t functionId = ctx_.aux_boson_package_len >= 10
                                  ? (uint32_t(pkg[6]) << 24) |
                                        (uint32_t(pkg[7]) << 16) |
                                        (uint32_t(pkg[8]) << 8) | pkg[9]
                                  : 0;
  qDebug() << "[RX] ret" << ret << "fn" << Qt::hex << Qt::showbase << functionId
           << "|"
           << QByteArray::fromRawData(reinterpret_cast<const char *>(pkg),
                                      ctx_.aux_boson_package_len)
                  .toHex(' ');

  if (ret != 0) { // Check FAILS
    if (ret == -1) {
      qWarning() << "Error : Boson Answer didn't come through CHANNEL 0";
    } else if (ret == -3) {
      qWarning() << "Error : Boson Sequence mismatch";
    } else if (ret == -4) {
      qWarning() << "Error : Boson Bad CRC Received";
    } else if (ret > 0) {
      Boson_Status_Error_Codes(ret);
    } else {
      qWarning() << "Error : Boson Undefined Error";
    }
    return;
  }

  emit responseReceived(
      functionId, std::vector<uint8_t>(ctx_.BosonData,
                                       ctx_.BosonData + ctx_.BosonData_count));
}
