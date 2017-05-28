#pragma once

#include "graph.h"

#include "QtCore/QRectF"
#include "QtGui/QFont"
#include "QtGui/QFontMetrics"
#include "QtWidgets/QGraphicsItem"

namespace skadi
{

class ui_scene;

class ui_node
  : public QObject
  , public QGraphicsItem
{
  Q_OBJECT
  Q_INTERFACES(QGraphicsItem)

public:
  ui_node(node_type const &);
  ~ui_node();

  ui_node(ui_node const &) = delete;
  ui_node &operator=(ui_node const &) = delete;

  int get_input_index(QPointF) const;
  QPointF get_input_position(int) const;
  QPointF get_output_position(int) const;

  QColor get_output_color(int) const;

  node_type const &get_type_info() const;

signals:
  void positionChanged();

private:
  int type() const override;

  QRectF boundingRect() const override;
  void paint(QPainter *painter, QStyleOptionGraphicsItem const *option, QWidget *widget) override;
  QVariant itemChange(GraphicsItemChange change, QVariant const &value) override;

  void calculate_layout();

  int get_output_index(QPointF pos) const;
  void mousePressEvent(QGraphicsSceneMouseEvent *event) override;

  void hoverEnterEvent(QGraphicsSceneHoverEvent *event) override;
  void hoverLeaveEvent(QGraphicsSceneHoverEvent *event) override;

  ui_scene *parent;
  node_type type_info;

  QFont font;
  QFontMetrics font_metrics;

  QRectF bounding_rect;
  QPointF caption_offset;
  QPointF input_offset;
  QPointF output_offset;

  bool is_hovered;
};

} // namespace skadi
