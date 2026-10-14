#pragma once

#include <QString>
#include <QtSerialPort/QSerialPort>
#include <vector>

static constexpr int kBosonBaudRate = 921600;

// lsusb -v
/*
  idVendor           0x09cb FLIR Systems
  idProduct          0x4007 Breach
*/
static constexpr quint16 kFlirVendorId = 0x09cb;
static constexpr quint16 kBosonProductId = 0x4007;

struct AvailablePort {
  QString portName;
  QString systemLocation;
  QString description;
};

std::vector<AvailablePort> getAvailablePorts();

bool openSerialPort(QSerialPort &port, const QString &systemLocation,
                    qint32 baudRate, QString &errorMessage);
