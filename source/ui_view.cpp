#include "ui_view.h"

#include "QtCore/QMimeData"
#include "QtGui/QDropEvent"

namespace skadi
{

namespace constants
{
  static QColor background_color(53, 53, 53);
  static QColor grid_color_fine(60, 60, 60);
  static QColor grid_color_coarse(25, 25, 25);
}

ui_view::ui_view(ui_scene *scene)
  : QGraphicsView(scene)
  , scene(scene)
{
  setBackgroundBrush(constants::background_color);

  setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
  setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
  
  setDragMode(QGraphicsView::ScrollHandDrag);
  setRenderHint(QPainter::Antialiasing);
  
  setTransformationAnchor(QGraphicsView::AnchorUnderMouse);

  setCacheMode(QGraphicsView::CacheBackground);
}

ui_view::~ui_view() = default;

void ui_view::dragEnterEvent(QDragEnterEvent *event)
{
  if(event->mimeData()->formats().contains("application/x-skadinodetype"))
  {
    event->accept();
  }
}

void ui_view::dragMoveEvent(QDragMoveEvent *event)
{
  if(event->mimeData()->formats().contains("application/x-skadinodetype"))
  {
    event->accept();
  }
}

void ui_view::dropEvent(QDropEvent *event)
{
  if(!(event->possibleActions() & Qt::CopyAction))
  {
    return;
  }

  auto const mime_data = event->mimeData();
  auto const id_data = mime_data->data("application/x-skadinodetype");
  node_type_id id{};
  if(id_data.size() != sizeof(id))
  {
    return;
  }
  std::copy(id_data.begin(), id_data.end(), reinterpret_cast<char *>(&id));

  event->setDropAction(Qt::CopyAction);
  event->accept();

  auto const pos = event->pos();
  auto const scene_pos = mapToScene(pos);

  scene->create_node(id, pos);
}

void ui_view::drawBackground(QPainter *painter, QRectF const &r)
{
  QGraphicsView::drawBackground(painter, r);

  auto drawGrid =
    [&](double gridStep)
  {
    QRect   windowRect = rect();
    QPointF tl = mapToScene(windowRect.topLeft());
    QPointF br = mapToScene(windowRect.bottomRight());

    double left   = std::floor(tl.x() / gridStep - 0.5);
    double right  = std::floor(br.x() / gridStep + 1.0);
    double bottom = std::floor(tl.y() / gridStep - 0.5);
    double top    = std::floor (br.y() / gridStep + 1.0);

    // vertical lines
    for (int xi = int(left); xi <= int(right); ++xi)
    {
      QLineF line(xi * gridStep, bottom * gridStep,
                  xi * gridStep, top * gridStep );

      painter->drawLine(line);
    }

    // horizontal lines
    for (int yi = int(bottom); yi <= int(top); ++yi)
    {
      QLineF line(left * gridStep, yi * gridStep,
                  right * gridStep, yi * gridStep );
      painter->drawLine(line);
    }
  };

  QPen pfine(constants::grid_color_fine, 1.0);

  painter->setPen(pfine);
  drawGrid(15);

  QPen p(constants::grid_color_coarse, 1.0);

  painter->setPen(p);
  drawGrid(150);
}

void ui_view::showEvent(QShowEvent *event)
{
  scene->setSceneRect(this->rect());
  QGraphicsView::showEvent(event);
}

} // namespace skadi
