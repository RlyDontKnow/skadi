#include "ui_connection.h"
#include "ui_node.h"

#include <numeric>
#include <stdexcept>

#include "QtGui/QCursor"
#include "QtGui/QKeyEvent"
#include "QtGui/QPainter"
#include "QtWidgets/QGraphicsScene"
#include "QtWidgets/QGraphicsSceneHoverEvent"
#include "QtWidgets/QGraphicsSceneMouseEvent"
#include "QtWidgets/QGraphicsView"
#include "QtWidgets/QStyleOptionGraphicsItem"

namespace skadi
{

namespace constants
{
  static qreal const connection_radius = 5;
  static int const selected_color_factor = 200;
  static QColor const selected_color(255, 165, 0);
  static int const hover_color_factor = 100;
  static QColor const hover_color(255, 255, 255);
  static qreal const insertion_width = 20;
  static qreal const line_width_default = 3;
  static qreal const line_width_hilight = 6;
  static QColor const incomplete_color(100, 100, 100);
  static int const shape_stroker_width = 15;
}


ui_connection::ui_connection(ui_node *source, int source_port,
                             ui_node *destination, int destination_port)
  : source(source)
  , source_port(source_port)
  , destination(destination)
  , destination_port(destination_port)
  , is_hovered()
  , was_dragged()
{
  if(!source)
  {
    throw std::invalid_argument("ui_connection: source cannot be null");
  }

  update_positions();

  connect(source, &ui_node::positionChanged, this, &ui_connection::update_positions);
  if(destination)
  {
    connect(destination, &ui_node::positionChanged, this, &ui_connection::update_positions);
  }

  setFlag(QGraphicsItem::ItemIsMovable, true);
  setFlag(QGraphicsItem::ItemIsFocusable, true);
  setFlag(QGraphicsItem::ItemIsSelectable, true);

  setAcceptHoverEvents(true);

  setZValue(-1.0);
}

ui_connection::~ui_connection()
{
  source->update();
  if(destination)
  {
    destination->update();
  }
}

void ui_connection::set_destination(ui_node *new_destination, int new_destination_port)
{
  auto old_destination = destination;

  destination = new_destination;
  destination_port = new_destination_port;

  if(destination != old_destination)
  {
    if(destination)
    {
      connect(destination, &ui_node::positionChanged, this, &ui_connection::update_positions);
    }
    if(old_destination)
    {
      disconnect(old_destination, &ui_node::positionChanged, this, &ui_connection::update_positions);
      old_destination->update();
    }
  }
}

std::pair<ui_node *, int> ui_connection::get_source() const
{
  return{source, source_port};
}

std::pair<ui_node *, int> ui_connection::get_destination() const
{
  return{destination, destination_port};
}

void ui_connection::update_positions()
{
  if(!scene())
  {
    return;
  }

  auto scene_to_connection = sceneTransform().inverted();
  auto views = scene()->views();
  if(views.empty())
  {
    return;
  }
  auto view = views.front();

  auto source_position = scene_to_connection.map(source->get_output_position(source_port));
  auto destination_position = scene_to_connection.map((nullptr == destination)
    ? view->mapToScene(view->mapFromGlobal(QCursor::pos()))
    : destination->get_input_position(destination_port)
    );

  auto source_position_offset = source_position + QPointF{constants::insertion_width, 0};
  auto destination_position_offset = destination_position - QPointF{constants::insertion_width, 0};

  auto diff = destination_position_offset - source_position_offset;
  auto mid = source_position_offset + 0.5 * diff;

  path = QPainterPath{source_position};
  path.lineTo(source_position_offset);
  if(abs(diff.x()) > abs(diff.y()))
  {
    path.lineTo({mid.x(), source_position_offset.y()});
    path.lineTo({mid.x(), destination_position_offset.y()});
  }
  else
  {
    path.lineTo({source_position_offset.x(), mid.y()});
    path.lineTo({destination_position_offset.x(), mid.y()});
  }
  path.lineTo(destination_position_offset);
  path.lineTo(destination_position);

  prepareGeometryChange();
}

int ui_connection::type() const
{
  return (UserType + 2);
}

QRectF ui_connection::boundingRect() const
{
  return shape().boundingRect();
}

QPainterPath ui_connection::shape() const
{
  QPainterPathStroker stroker;
  stroker.setWidth(constants::shape_stroker_width);
  return stroker.createStroke(path);
}

void ui_connection::paint(QPainter *painter, QStyleOptionGraphicsItem const *option, QWidget *)
{
  if(path.isEmpty())
  {
    return;
  }

  painter->setClipRect(option->exposedRect);

  auto pen = painter->pen();

  bool is_complete = (destination != nullptr);

  auto color = source->get_output_color(source_port);
  if(isSelected())
  {
    color = color.darker(constants::selected_color_factor);
  }
  else if(is_hovered)
  {
    color = color.lighter(constants::hover_color_factor);
  }

  pen.setColor(color);

  if(is_complete)
  {
    // draw hilighting if needed
    if(isSelected() || is_hovered)
    {
      auto p = pen;
      p.setWidthF(constants::line_width_hilight);
      p.setColor(isSelected() ? constants::selected_color : constants::hover_color);
      painter->setPen(p);
      painter->setBrush(Qt::NoBrush);
      painter->drawPath(path);
    }
  }
  else
  {
    pen.setColor(constants::incomplete_color);
    pen.setStyle(Qt::DashLine);
  }

  // draw path
  pen.setWidthF(constants::line_width_default);
  painter->setPen(pen);
  painter->setBrush(Qt::NoBrush);
  painter->drawPath(path);

  // draw ports
  painter->setBrush(color);
  painter->drawEllipse(path.elementAt(0), constants::connection_radius, constants::connection_radius);
  painter->drawEllipse(path.elementAt(path.elementCount() - 1), constants::connection_radius, constants::connection_radius);
}

QVariant ui_connection::itemChange(GraphicsItemChange change, const QVariant &value)
{
  if((ItemSceneChange == change) && scene())
  {
    update_positions();
  }

  return QGraphicsItem::itemChange(change, value);
}

void ui_connection::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
  is_hovered = true;
  prepareGeometryChange();
  event->accept();
}

void ui_connection::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
  is_hovered = false;
  prepareGeometryChange();
  event->accept();
}

void ui_connection::keyPressEvent(QKeyEvent *event)
{
  QGraphicsItem::keyPressEvent(event);

  if(event->key() == Qt::Key_Delete)
  {
    deleteLater();
    event->accept();
  }
}

void ui_connection::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
  QGraphicsItem::mouseMoveEvent(event);

  update_destination(event);
  was_dragged = true;

  event->accept();
}

void ui_connection::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
  QGraphicsItem::mouseReleaseEvent(event);

  if(was_dragged)
  {
    update_destination(event);
    if(nullptr == destination)
    {
      deleteLater();
    }
  }
  was_dragged = false;
  ungrabMouse();

  event->accept();
}

void ui_connection::update_destination(QGraphicsSceneMouseEvent *event)
{
  auto pos = event->scenePos();

  auto node = find_node(pos);
  auto port = 0;
  if(node != nullptr)
  {
    port = node->get_input_index(pos);
    if(port == -1) // destination doesn't have inputs, so we can't connect to it
    {
      node = nullptr;
    }
  }
  set_destination(node, port);

  update_positions();
  event->accept();
}

ui_node *ui_connection::find_node(QPointF pos) const
{
  auto items = scene()->items(pos, Qt::IntersectsItemShape, Qt::DescendingOrder);
  return std::accumulate(std::begin(items), std::end(items), static_cast<ui_node *>(nullptr),
                         [&](ui_node *lhs, QGraphicsItem *rhs)
  {
    auto node = dynamic_cast<ui_node *>(rhs);
    if(node == source)
    {
      node = nullptr;
    }
    return ((lhs != nullptr) ? lhs : node);
  });
}

} // namespace skadi
