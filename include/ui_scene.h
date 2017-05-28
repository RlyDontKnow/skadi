#pragma once

#include "graph.h"
#include "picojson.h"

#include <unordered_map>
#include <vector>

#include "QtWidgets/QgraphicsScene"
#include "QtCore/QPointF"

namespace skadi
{

class ui_node;
class ui_connection;

struct node_layout
{
  node_instance_id node;
  QPointF position;
};

struct graph_layout
{
  std::vector<node_layout> node_layouts;
};

picojson::value save(graph_layout);
graph_layout load_graph_layout(picojson::value);

class ui_scene
  : public QGraphicsScene
{
  Q_OBJECT

public:
  ui_scene(type_registry registry);
  ~ui_scene();

  ui_scene(ui_scene const &) = delete;
  ui_scene &operator=(ui_scene const &) = delete;

  void clear();

  graph get_content() const;
  void set_content(graph);

  graph_layout get_layout() const;
  void set_layout(graph_layout);

  ui_connection *create_connection(ui_node *source, int source_port);

  bool is_input_connected(ui_node *node, int port);
  bool is_output_connected(ui_node *node, int port);

public slots:
  void remove_connection(connection_instance_id);
  void remove_node(node_instance_id);

private:
  void add_connection(connection_instance_id, ui_connection *);
  void add_node(node_instance_id, ui_node *);

  type_registry registry;
  std::map<node_instance_id, ui_node *> nodes;
  std::map<connection_instance_id, ui_connection *> connections;

  int64_t last_uid;
};

} // namespace skadi
