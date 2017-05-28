#pragma once

#include "QtWidgets/QGraphicsItem"

namespace skadi
{

class ui_node;

class ui_connection
  : public QObject
  , public QGraphicsItem
{
  Q_OBJECT
  Q_INTERFACES(QGraphicsItem)

public:
  ui_connection(ui_node *source, int source_port,
                ui_node *destination = nullptr, int destination_port = 0);
  ~ui_connection();

  ui_connection(ui_connection const &) = delete;
  ui_connection &operator=(ui_connection const &) = delete;

  void set_destination(ui_node *destination, int destination_port);

  std::pair<ui_node *, int> get_source() const;
  std::pair<ui_node *, int> get_destination() const;

public slots:
  void update_positions();

private:
  int type() const override;
  QRectF boundingRect() const override;
  QPainterPath shape() const override;
  void paint(QPainter *painter, QStyleOptionGraphicsItem const *option, QWidget *widget) override;
  QVariant itemChange(GraphicsItemChange change, const QVariant &value) override;

  void hoverEnterEvent(QGraphicsSceneHoverEvent *event) override;
  void hoverLeaveEvent(QGraphicsSceneHoverEvent *event) override;

  void keyPressEvent(QKeyEvent *event) override;

  void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
  void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;

  void update_destination(QGraphicsSceneMouseEvent *event);
  ui_node *find_node(QPointF) const;

  ui_node *source;
  int source_port;
  ui_node *destination;
  int destination_port;

  QPainterPath path;
  bool is_hovered;
  bool was_dragged;
};

} // namespace skadi
