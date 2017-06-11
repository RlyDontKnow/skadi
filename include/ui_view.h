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
  explicit ui_view(ui_scene *);
  ~ui_view();

  ui_view(ui_view const &) = delete;
  ui_view &operator=(ui_view const &) = delete;

private:
  void dragEnterEvent(QDragEnterEvent *) override;
  void dragMoveEvent(QDragMoveEvent *) override;
  void dropEvent(QDropEvent *) override;
  void drawBackground(QPainter *, QRectF const &) override;
  void showEvent(QShowEvent *) override;
  void wheelEvent(QWheelEvent *) override;

  ui_scene *scene;
};

} // namespace skadi
