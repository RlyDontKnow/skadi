#include "ui_connection.h"
#include "ui_node.h"
#include "ui_scene.h"

#include <numeric>
#include <stdexcept>

#include "QtGui/QPainter"
#include "QtWidgets/QGraphicsDropShadowEffect"
#include "QtWidgets/QGraphicsSceneMouseEvent"
#include "QtWidgets/QStyleOptionGraphicsItem"

namespace skadi
{

namespace constants
{
  static qreal const opacity = 0.8;

  static qreal const text_vertical_spacing = 10;
  static qreal const text_horizontal_spacing = 10;
  static qreal const text_inner_spacing = 40;

  static QColor const shadow_color(20, 20, 20);
  static QPointF const shadow_offset(4, 4);
  static qreal const shadow_blur_radius = 20;

  static qreal const connection_radius = 5;
  static qreal const connection_interaction_radius = 40;

  static QColor const boundary_color_default(255, 255, 255);
  static QColor const boundary_color_selected(255, 165, 0);
  static qreal const boundary_radius = 3;

  static QColor const caption_color(255, 255, 255);

  static QColor const port_font_color_default(255, 255, 255);
  static QColor const port_font_color_empty(160, 160, 160);

  static qreal const pen_width_default = 1;
  static qreal const pen_width_hovered = 3;

  static std::pair<qreal, QColor> const gradient_colors[]
  {
    {0.00, {160, 160, 160}}
  , {0.03, { 80,  80,  80}}
  , {0.97, { 64,  64,  64}}
  , {1.00, { 58,  58,  58}}
  };
}

namespace
{
  auto calculate_text_width(QFontMetrics const &metrics, std::string const &s)
  {
    return metrics.boundingRect(QString::fromStdString(s)).width();
  }

  template<typename C>
  auto calculate_text_width(QFontMetrics const &metrics, C const &container)
  {
    int result{};
    for(auto &&v : container)
    {
      result = std::max(result, calculate_text_width(metrics, v.name));
    }
    return result;
  }

  QLinearGradient create_gradient(QRectF bounds)
  {
    QLinearGradient result(QPointF(0, 0), QPointF(2.0, bounds.height()));
    for(auto &&p : constants::gradient_colors)
    {
      result.setColorAt(p.first, p.second);
    }
    return result;
  }

  QColor generate_color(data_type_id type)
  {
    auto hue = static_cast<int>((127U + 67U * static_cast<uint64_t>(type.guid)) % 256U);
    auto saturation = 120;
    auto lightness = 160;

    return QColor::fromHsl(hue, saturation, lightness);
  }
}

ui_node::ui_node(node_type const &type_info)
  : parent(nullptr)
  , type_info(type_info)
  , is_hovered()
  , font()
  , font_metrics(font)
{
  setFlag(QGraphicsItem::ItemDoesntPropagateOpacityToChildren, true);
  setFlag(QGraphicsItem::ItemIsMovable, true);
  setFlag(QGraphicsItem::ItemIsFocusable, true);
  setFlag(QGraphicsItem::ItemIsSelectable, true);
  setFlag(QGraphicsItem::ItemSendsScenePositionChanges, true);

  setCacheMode(QGraphicsItem::DeviceCoordinateCache);

  {
    auto effect = new QGraphicsDropShadowEffect;
    effect->setOffset(constants::shadow_offset);
    effect->setBlurRadius(constants::shadow_blur_radius);
    effect->setColor(constants::shadow_color);

    setGraphicsEffect(effect);
  }
  setOpacity(constants::opacity);

  setAcceptHoverEvents(true);
  setZValue(0);

  calculate_layout();
}

ui_node::~ui_node() = default;

int ui_node::get_input_index(QPointF pos) const
{
  auto input_count = static_cast<int>(type_info.inputs.size());
  if(0 == input_count)
  {
    return -1;
  }

  auto dist = [](QPointF p1, QPointF p2)
  {
    return abs(p1.y() - p2.y());
  };

  auto result = 0;
  auto result_dist = dist(pos, get_input_position(0));
  for(int i = 1; i < input_count; ++i)
  {
    auto d = dist(pos, get_input_position(i));
    if(d < result_dist)
    {
      result = i;
      result_dist = d;
    }
  }

  return result;
}

QPointF ui_node::get_input_position(int idx) const
{
  auto offset = input_offset;
  offset.rx() -= constants::text_horizontal_spacing + constants::connection_radius;
  offset.ry() += idx * (font_metrics.height() + constants::text_vertical_spacing) - 0.25 * font_metrics.height();
  return sceneTransform().map(offset);
}

QPointF ui_node::get_output_position(int idx) const
{
  auto offset = output_offset;
  offset.rx() += constants::text_horizontal_spacing + constants::connection_radius;
  offset.ry() += idx * (font_metrics.height() + constants::text_vertical_spacing) - 0.25 * font_metrics.height();
  return sceneTransform().map(offset);
}

QColor ui_node::get_output_color(int idx) const
{
  return generate_color(type_info.outputs.at(static_cast<size_t>(idx)).type);
}

node_type const &ui_node::get_type_info() const
{
  return type_info;
}

int ui_node::type() const
{
  return (UserType + 1);
}

QRectF ui_node::boundingRect() const
{
  auto rect = bounding_rect;
  auto border = std::max(2 * constants::connection_radius, constants::boundary_radius);
  return
  {
    rect.left() - border,
    rect.top() - border,
    rect.width() + 2 * border,
    rect.height() + 2 * border
  };
}

void ui_node::paint(QPainter *painter, QStyleOptionGraphicsItem const *option, QWidget *)
{
  parent = dynamic_cast<ui_scene *>(scene());
  
  painter->setClipRect(option->exposedRect);

  if(font != painter->font())
  {
    font = painter->font();
    calculate_layout();
  }

  { // draw bounds
    auto pen = painter->pen();
    pen.setColor(isSelected() ? constants::boundary_color_selected : constants::boundary_color_default);
    pen.setWidthF(is_hovered ? constants::pen_width_hovered : constants::pen_width_default);
    painter->setPen(pen);

    painter->setBrush(create_gradient(bounding_rect));

    QRectF boundary
    {
      -constants::connection_radius
    , -constants::connection_radius
    , 2.0 * constants::connection_radius + bounding_rect.width()
    , 2.0 * constants::connection_radius + bounding_rect.height()
    };

    painter->drawRoundedRect(boundary, constants::boundary_radius, constants::boundary_radius);
  }

  { // draw caption
    auto offset = caption_offset;

    auto bold_font = font;
    bold_font.setBold(true);
    painter->setFont(bold_font);

    auto pen = painter->pen();
    pen.setColor(constants::caption_color);
    painter->setPen(pen);

    painter->drawText(offset, QString::fromStdString(type_info.name));

    painter->setFont(font);
  }

  { // draw input
    auto offset = input_offset;
    auto pen = painter->pen();
    pen.setWidth(constants::pen_width_default);

    int port_index{};
    for(auto &&p : type_info.inputs)
    {
      bool is_port_connected = parent->is_input_connected(this, port_index++);

      pen.setColor(is_port_connected ? constants::port_font_color_default : constants::port_font_color_empty);
      painter->setPen(pen);
      painter->drawText(offset, QString::fromStdString(p.name));

      auto port_offset = 0.25 * font_metrics.height();
      offset.rx() -= constants::text_horizontal_spacing + constants::connection_radius;
      offset.ry() -= port_offset;

      auto color = generate_color(p.type);
      pen.setColor(isSelected() ? constants::boundary_color_selected : constants::boundary_color_default);
      painter->setPen(pen);
      painter->setBrush(color);
      painter->drawEllipse(offset, constants::connection_radius, constants::connection_radius);

      offset.rx() += constants::text_horizontal_spacing + constants::connection_radius;
      offset.ry() += port_offset + font_metrics.height() + constants::text_vertical_spacing;
    }
  }

  { // draw output
    auto offset = output_offset;
    auto pen = painter->pen();
    pen.setWidth(constants::pen_width_default);

    int port_index{};
    for(auto &&p : type_info.outputs)
    {
      bool is_port_connected = parent->is_output_connected(this, port_index++);
      auto text_rect = font_metrics.boundingRect(QString::fromStdString(p.name));

      pen.setColor(is_port_connected ? constants::port_font_color_default : constants::port_font_color_empty);
      painter->setPen(pen);
      offset.rx() -= text_rect.width();
      painter->drawText(offset, QString::fromStdString(p.name));

      auto port_offset = 0.25 * font_metrics.height();
      offset.rx() += text_rect.width() + constants::text_horizontal_spacing + constants::connection_radius;
      offset.ry() -= port_offset;

      auto color = generate_color(p.type);
      pen.setColor(isSelected() ? constants::boundary_color_selected : constants::boundary_color_default);
      painter->setPen(pen);
      painter->setBrush(color);
      painter->drawEllipse(offset, constants::connection_radius, constants::connection_radius);

      offset.rx() -= constants::text_horizontal_spacing + constants::connection_radius;
      offset.ry() += port_offset + font_metrics.height() + constants::text_vertical_spacing;
    }
  }
}

QVariant ui_node::itemChange(GraphicsItemChange change, QVariant const &value)
{
  if(QGraphicsItem::ItemPositionHasChanged == change)
  {
    emit positionChanged();
  }
  return QGraphicsItem::itemChange(change, value);
}

void ui_node::calculate_layout()
{
  font_metrics = QFontMetrics(font);
  auto bold_font = font;
  bold_font.setBold(true);
  QFontMetrics bold_font_metrics(font);

  auto text_height = font_metrics.height();
  auto row_count = std::max(type_info.inputs.size(), type_info.outputs.size());

  auto input_width = calculate_text_width(font_metrics, type_info.inputs);
  auto output_width = calculate_text_width(font_metrics, type_info.outputs);

  auto port_width = 2 * constants::text_horizontal_spacing + constants::text_inner_spacing + 2 * std::max(input_width, output_width);
  auto port_height = row_count * text_height + (row_count + 1) * constants::text_vertical_spacing;

  auto caption_text_width = calculate_text_width(bold_font_metrics, type_info.name);
  auto caption_text_height = bold_font_metrics.height();
  auto caption_height = 2 * constants::text_vertical_spacing + bold_font_metrics.height();
  auto caption_width = 2 * constants::text_horizontal_spacing + caption_text_width;

  auto width = std::max(caption_width, port_width);
  auto height = caption_height + port_height;

  bounding_rect = {0, 0, width, height};
  caption_offset = {0.5 * width - 0.5 * caption_text_width, caption_text_height + constants::text_vertical_spacing};
  input_offset = {constants::text_horizontal_spacing, caption_height + text_height};
  output_offset = {width - constants::text_horizontal_spacing, caption_height + text_height};

  prepareGeometryChange();
}

int ui_node::get_output_index(QPointF pos) const
{
  auto output_count = static_cast<int>(type_info.outputs.size());
  if(0 == output_count)
  {
    return -1;
  }

  auto dist = [](QPointF p1, QPointF p2)
  {
    return (p1 - p2).manhattanLength();
  };

  auto result = 0;
  auto result_dist = dist(pos, get_output_position(0));
  for(int i = 1; i < output_count; ++i)
  {
    auto d = dist(pos, get_output_position(i));
    if(d < result_dist)
    {
      result = i;
      result_dist = d;
    }
  }

  return ((result_dist <= constants::connection_interaction_radius) ? result : -1);
}

void ui_node::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
  QGraphicsItem::mousePressEvent(event);

  auto idx = get_output_index(event->scenePos());
  if(idx != -1)
  {
    if(nullptr == parent)
    {
      throw std::runtime_error("ui_node: must be attached to ui_scene!");
    }

    auto connection = parent->create_connection(this, idx);

    // pass mouse control to connection
    setSelected(false);
    connection->setSelected(true);
    connection->grabMouse();
  }
}

void ui_node::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
  is_hovered = true;
  update();
  event->accept();
}

void ui_node::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
  is_hovered = false;
  update();
  event->accept();
}


} // namespace skadi
