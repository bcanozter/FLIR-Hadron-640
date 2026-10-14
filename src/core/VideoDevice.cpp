// Reference: https://www.kernel.org/doc/html/v4.9/media/uapi/v4l/capture.c.html

#include "VideoDevice.h"

#include <QDebug>
#include <QDir>
#include <QFile>
#include <QSocketNotifier>

#include <cerrno>
#include <cstring>
#include <fcntl.h>
#include <linux/videodev2.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <unistd.h>

#define CLEAR(x) memset(&(x), 0, sizeof(x))

int xioctl(int fd, unsigned long request, void *arg) {

  int result;

  do {
    result = ioctl(fd, request, arg);
  } while (result == -1 && errno == EINTR);

  return result;
}

std::vector<AvailableVideoDevice> getAvailableVideoDevices() {

  std::vector<AvailableVideoDevice> devices;

  const QDir sysDir("/sys/class/video4linux");

  const QStringList entries =
      sysDir.entryList({"video*"}, QDir::Dirs | QDir::System, QDir::Name);

  for (const QString &entry : entries) {

    QFile nameFile(sysDir.filePath(entry + "/name"));

    QString name;

    if (nameFile.open(QIODevice::ReadOnly)) {
      name = QString::fromUtf8(nameFile.readAll()).trimmed();
    }

    devices.push_back({"/dev/" + entry, name});
  }
  return devices;
}

VideoDevice::VideoDevice(QObject *parent) : QObject(parent) {}

VideoDevice::~VideoDevice() { close(); }

bool VideoDevice::open(const QString &path) {

  close();

  fd_ = ::open(path.toLocal8Bit().constData(), O_RDWR | O_NONBLOCK);

  if (fd_ < 0) {
    qWarning() << "open" << path << "failed:" << strerror(errno);
    return false;
  }

  v4l2_capability cap{};

  CLEAR(cap);

  if (xioctl(fd_, VIDIOC_QUERYCAP, &cap) < 0) {
    qWarning() << "VIDIOC_QUERYCAP failed:" << strerror(errno);
    close();
    return false;
  }

  if (!(cap.capabilities & V4L2_CAP_DEVICE_CAPS) ||
      !(cap.device_caps & V4L2_CAP_VIDEO_CAPTURE) ||
      !(cap.device_caps & V4L2_CAP_STREAMING)) {
    qWarning() << path << "is not a streaming capture device";
    close();
    return false;
  }

  v4l2_format fmt{};

  CLEAR(fmt);

  fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  fmt.fmt.pix.width = kBosonWidth;
  fmt.fmt.pix.height = kBosonHeight;
  fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_Y16;
  fmt.fmt.pix.field = V4L2_FIELD_INTERLACED;
  if (xioctl(fd_, VIDIOC_S_FMT, &fmt) < 0) {
    qWarning() << "VIDIOC_S_FMT failed:" << strerror(errno);
    close();
    return false;
  }

  if (fmt.fmt.pix.pixelformat != V4L2_PIX_FMT_Y16 ||
      fmt.fmt.pix.width != kBosonWidth || fmt.fmt.pix.height != kBosonHeight) {
    qWarning() << "Driver did not accept Y16" << kBosonWidth << "x"
               << kBosonHeight << "(got" << fmt.fmt.pix.width << "x"
               << fmt.fmt.pix.height << ")";
    close();
    return false;
  }
  unsigned int min;

  min = fmt.fmt.pix.width * 2;
  if (fmt.fmt.pix.bytesperline < min)
    fmt.fmt.pix.bytesperline = min;
  min = fmt.fmt.pix.bytesperline * fmt.fmt.pix.height;
  if (fmt.fmt.pix.sizeimage < min)
    fmt.fmt.pix.sizeimage = min;

  v4l2_requestbuffers req{};

  CLEAR(req);

  req.count = 4;
  req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  req.memory = V4L2_MEMORY_MMAP;
  if (xioctl(fd_, VIDIOC_REQBUFS, &req) < 0) {
    qWarning() << "VIDIOC_REQBUFS failed:" << strerror(errno);
    close();
    return false;
  }
  if (req.count < 2) {
    qWarning() << "Driver granted only" << req.count << "buffer(s)";
    close();
    return false;
  }

  for (uint32_t i = 0; i < req.count; ++i) {
    v4l2_buffer buf{};
    buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buf.memory = V4L2_MEMORY_MMAP;
    buf.index = i;
    if (xioctl(fd_, VIDIOC_QUERYBUF, &buf) < 0) {
      qWarning() << "VIDIOC_QUERYBUF failed:" << strerror(errno);
      close();
      return false;
    }
    void *start = mmap(nullptr, buf.length, PROT_READ | PROT_WRITE, MAP_SHARED,
                       fd_, buf.m.offset);
    if (start == MAP_FAILED) {
      qWarning() << "mmap failed:" << strerror(errno);
      close();
      return false;
    }
    buffers_.push_back({start, buf.length});
    if (xioctl(fd_, VIDIOC_QBUF, &buf) < 0) {
      qWarning() << "VIDIOC_QBUF failed:" << strerror(errno);
      close();
      return false;
    }
  }

  v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  if (xioctl(fd_, VIDIOC_STREAMON, &type) < 0) {
    qWarning() << "VIDIOC_STREAMON failed:" << strerror(errno);
    close();
    return false;
  }

  notifier_ = new QSocketNotifier(fd_, QSocketNotifier::Read, this);
  connect(notifier_, &QSocketNotifier::activated, this,
          &VideoDevice::onReadable_);

  return true;
}

void VideoDevice::close() {

  if (notifier_) {
    notifier_->setEnabled(false);
    notifier_->deleteLater();
    notifier_ = nullptr;
  }

  if (fd_ < 0) {
    return;
  }

  v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

  // stop capturing
  if (xioctl(fd_, VIDIOC_STREAMOFF, &type) < 0) {
    qWarning() << "VIDIOC_STREAMOFF failed: " << strerror(errno);
  }

  // uninit
  for (const MappedBuffer &b : buffers_) {
    munmap(b.start, b.length);
  }

  buffers_.clear();
  ::close(fd_);
  fd_ = -1;
}

void VideoDevice::onReadable_() {
  v4l2_buffer buf{};

  buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  buf.memory = V4L2_MEMORY_MMAP;

  if (xioctl(fd_, VIDIOC_DQBUF, &buf) < 0) {
    if (errno == EAGAIN) {
      return;
    }
    qWarning() << "VIDIOC_DQBUF failed:" << strerror(errno);
    close();
    emit deviceError();
    return;
  }

  // capture.c: assert(buf.index < n_buffers)
  if (buf.index >= buffers_.size()) {
    qWarning() << "VIDIOC_DQBUF returned buffer index" << buf.index;
    close();
    emit deviceError();
    return;
  }

  const size_t frameBytes =
      size_t(kBosonWidth) * kBosonHeight * sizeof(uint16_t);

  if (buf.flags & V4L2_BUF_FLAG_ERROR || buf.bytesused < frameBytes) {
    qWarning() << "Dropping frame: bytesused" << buf.bytesused << "expected"
               << frameBytes;
  } else {

    RawFrame frame;

    frame.width = kBosonWidth;
    frame.height = kBosonHeight;
    frame.pixels.resize(size_t(kBosonWidth) * kBosonHeight);

    std::memcpy(frame.pixels.data(), buffers_[buf.index].start, frameBytes);

    emit frameReceived(frame);
  }

  if (fd_ < 0) {
    return;
  }

  // if (-1 == xioctl(fd, VIDIOC_QBUF, &buf))
  if (xioctl(fd_, VIDIOC_QBUF, &buf) < 0) {
    qWarning() << "VIDIOC_QBUF failed:" << strerror(errno);
    close();
    emit deviceError();
  }
}
