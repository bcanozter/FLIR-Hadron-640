#include "RawFrameView.h"

#include <QPainter>

RawFrameView::RawFrameView(QWidget *parent) : QWidget(parent) {
  setMinimumSize(320, 256);
}

// https://github.com/FLIR/BosonUSB/blob/master/BosonUSB.cpp#L88
void RawFrameView::AGC_Basic_Linear(const RawFrame &input_16, QImage &output_8,
                                    int h, int w) {

  uint16_t min1 = 0xFFFF;
  uint16_t max1 = 0x0000;

  for (int i = 0; i < h; i++) {
    const uint16_t *row = input_16.pixels.data() + i * w;
    for (int j = 0; j < w; j++) {
      uint16_t v = row[j];
      if (v < min1)
        min1 = v;
      if (v > max1)
        max1 = v;
    }
  }

  if (max1 <= min1) {
    output_8.fill(0);
    return;
  }

  for (int i = 0; i < h; i++) {
    const uint16_t *in_row = input_16.pixels.data() + i * w;
    uint8_t *out_row = output_8.scanLine(i);

    for (int j = 0; j < w; j++) {
      uint16_t v = in_row[j];
      uint32_t scaled = (255u * (v - min1)) / (max1 - min1);
      out_row[j] = static_cast<uint8_t>(scaled);
    }
  }
}

void RawFrameView::setFrame(const RawFrame &frame) {

  image_ = QImage(frame.width, frame.height, QImage::Format_Grayscale8);
  if (image_.isNull()) {
    return;
  }
  AGC_Basic_Linear(frame, image_, frame.height, frame.width);

  update();
}

void RawFrameView::clear() {

  image_ = QImage();

  update();
}

void RawFrameView::paintEvent(QPaintEvent *) {

  QPainter painter(this);

  painter.fillRect(rect(), Qt::black);

  QRect target(QPoint(0, 0), image_.size().scaled(size(), Qt::KeepAspectRatio));

  target.moveCenter(rect().center());

  painter.drawImage(target, image_);
}
