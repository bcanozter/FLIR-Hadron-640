#pragma once

#include <QImage>
#include <QWidget>

#include "../core/RawFrame.h"

class RawFrameView : public QWidget {
  Q_OBJECT
public:
  explicit RawFrameView(QWidget *parent = nullptr);

  void AGC_Basic_Linear(const RawFrame &input_16, QImage &output_8, int h,
                        int w);
  void setFrame(const RawFrame &frame);
  void clear();

protected:
  void paintEvent(QPaintEvent *event) override;

private:
  QImage image_;
};
