#pragma once

#include "ui_scene.h"

#include "QtWidgets/QGraphicsView"

namespace skadi
{

class ui_view
  : public QGraphicsView
{
  Q_OBJECT

public:
  ui_view(ui_scene *);
  ~ui_view();

  ui_view(ui_view const &) = delete;
  ui_view &operator=(ui_view const &) = delete;

private:
  void drawBackground(QPainter *painter, QRectF const &r) override;
  void showEvent(QShowEvent *event) override;

  ui_scene *scene;
};

} // namespace skadi
