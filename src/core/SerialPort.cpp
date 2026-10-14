#include "SerialPort.h"

#include <QtSerialPort/QSerialPortInfo>

std::vector<AvailablePort> getAvailablePorts() {

  std::vector<AvailablePort> ports;

  const auto serialPortInfos = QSerialPortInfo::availablePorts();

  for (const QSerialPortInfo &portInfo : serialPortInfos) {
    ports.push_back({portInfo.portName(), portInfo.systemLocation(),
                     portInfo.description()});
  }

  return ports;
}

bool openSerialPort(QSerialPort &port, const QString &systemLocation,
                    qint32 baudRate, QString &errorMessage) {

  port.close();
  port.setPortName(systemLocation);
  port.setBaudRate(baudRate);
  port.setDataBits(QSerialPort::Data8);
  port.setParity(QSerialPort::NoParity);
  port.setStopBits(QSerialPort::OneStop);
  port.setFlowControl(QSerialPort::NoFlowControl);

  if (!port.open(QIODevice::ReadWrite)) {
    errorMessage = port.errorString();
    return false;
  }

  return true;
}
