#pragma once

#include <QObject>
#include <QString>

#include <vector>

#include "RawFrame.h"

class QSocketNotifier;

// verified from the device
static constexpr int kBosonWidth = 640;
static constexpr int kBosonHeight = 512;

struct AvailableVideoDevice {
  QString path;
  QString name;
};

std::vector<AvailableVideoDevice> getAvailableVideoDevices();

class VideoDevice : public QObject {
  Q_OBJECT
public:
  explicit VideoDevice(QObject *parent = nullptr);
  ~VideoDevice() override;


  bool open(const QString &path);
  void close();

signals:
  void frameReceived(const RawFrame &frame);
  void deviceError();

private slots:
  void onReadable_();

private:
  struct MappedBuffer {
    void *start;
    size_t length;
  };

  int fd_ = -1;
  QSocketNotifier *notifier_ = nullptr;
  std::vector<MappedBuffer> buffers_;

};
