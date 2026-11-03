#pragma once

#include <QImage>
#include <QWidget>

#include "../core/RawFrame.h"

class RawFrameView : public QWidget {
  Q_OBJECT
public:
  explicit RawFrameView(QWidget *parent = nullptr);

  void AGC_Basic_Linear(const Y16Frame &input_16, QImage &output_8, int h,
                        int w);
  void setFrame(const Y16Frame &frame);

  void setYU12Frame(const YU12Frame &frame);
  void clear();

protected:
  void paintEvent(QPaintEvent *event) override;

private:
  QImage image_;
};
