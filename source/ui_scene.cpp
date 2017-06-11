#include "ui_connection.h"
#include "ui_node.h"
#include "ui_scene.h"

#include "boost/range/algorithm.hpp"
#include "boost/range/adaptor/map.hpp"
#include "boost/range/adaptor/transformed.hpp"

namespace skadi
{

bool operator<(node_instance_id const &lhs, node_instance_id const &rhs)
{
  return (lhs.id < rhs.id);
}

bool operator<(connection_instance_id const &lhs, connection_instance_id const &rhs)
{
  return (lhs.id < rhs.id);
}

static picojson::value save(QPointF p)
{
  picojson::array a{};
  a.push_back(picojson::value(p.x()));
  a.push_back(picojson::value(p.y()));
  return picojson::value(a);
}

static QPointF load_point(picojson::value v)
{
  auto a = v.get<picojson::array>();
  return
  {
    a.at(0).get<double>()
  , a.at(1).get<double>()
  };
}

picojson::value save(graph_layout layout)
{
  picojson::object o{};

  picojson::array node_layout{};
  for(auto &&l : layout.node_layouts)
  {
    picojson::object entry{};
    entry["uid"] = picojson::value(l.node.id);
    entry["position"] = save(l.position);
    node_layout.emplace_back(entry);
  }
  o["node_layout"] = picojson::value(node_layout);

  return picojson::value(o);
}

graph_layout load_graph_layout(picojson::value v)
{
  graph_layout layout{};

  auto o = v.get<picojson::object>();
  for(auto &&entryValue : o["node_layout"].get<picojson::array>())
  {
    auto entry = entryValue.get<picojson::object>();

    node_layout l{};
    l.node.id = entry["uid"].get<int64_t>();
    l.position = load_point(entry["position"]);
    layout.node_layouts.emplace_back(l);
  }

  return layout;
}

ui_scene::ui_scene(type_registry registry)
  : registry(registry)
  , last_uid()
{
}

ui_scene::~ui_scene()
{
  clear();
}

void ui_scene::clear()
{
  // clear connections first as they reference nodes
  for(auto &&[unused, connection] : connections)
  {
    Q_UNUSED(unused);
    removeItem(connection);
  }
  connections.clear();
  nodes.clear();
  QGraphicsScene::clear();
}

graph ui_scene::get_content() const
{
  graph result{};

  for(auto &&[id, ui_node] : nodes)
  {
    node n{};
    n.uid = id;
    n.type = ui_node->get_type_info().guid;
    result.nodes.emplace_back(std::move(n));
  }

  auto find_node_id = [&](ui_node *node)
  {
    using namespace boost::range;

    auto it = find_if(nodes, [=](auto &&v)
    {
      return (v.second == node);
    });
    assert(it != end(nodes));
    return it->first;
  };
  for(auto &&[id, ui_connection] : connections)
  {
    connection c{};
    c.uid = id;
    
    auto &&[source_node, signal] = ui_connection->get_source();
    c.source = find_node_id(source_node);
    c.signal = source_node->get_type_info().outputs[signal].name;

    auto &&[destination_node, slot] = ui_connection->get_destination();
    c.destination = find_node_id(destination_node);
    c.slot = destination_node->get_type_info().inputs[slot].name;

    result.connections.emplace_back(std::move(c));
  }

  return result;
}

void ui_scene::set_content(graph content)
try
{
  auto first = begin(registry.node_types);
  auto last = end(registry.node_types);

  for(auto &&node : content.nodes)
  {
    auto it = find_if(first, last, [&](node_type const &t)
    {
      return (t.guid.guid == node.type.guid);
    });

    if(it == last)
    {
      throw std::runtime_error("unknown node_type: " + std::to_string(node.type.guid));
    }

    auto graphics_item = new ui_node(*it);
    addItem(graphics_item);
    nodes.emplace(node.uid, graphics_item);
  }

  auto find_index = [](auto &&range, std::string port_name)
  {
    using namespace boost::range;

    auto it = find_if(range, [&](auto &&port)
    {
      return (port.name == port_name);
    });
    if(it == end(range))
    {
      throw std::runtime_error("unknown port: " + port_name);
    }
    return distance(begin(range), it);
  };

  for(auto &&connection : content.connections)
  {
    auto source = nodes.at(connection.source);
    auto source_port = find_index(source->get_type_info().outputs, connection.signal);

    auto destination = nodes.at(connection.destination);
    auto destination_port = find_index(destination->get_type_info().inputs, connection.slot);

    last_uid = std::max(last_uid, connection.uid.id);
    add_connection(connection.uid, new ui_connection(source, source_port, destination, destination_port));
  }
}
catch(std::runtime_error &)
{
  clear();
  throw;
}

graph_layout ui_scene::get_layout() const
{
  graph_layout layout{};
  for(auto &&[id, ui_node] : nodes)
  {
    node_layout l{};
    l.node = id;
    l.position = ui_node->scenePos();
    layout.node_layouts.emplace_back(l);
  }
  return layout;
}

void ui_scene::set_layout(graph_layout layout)
{
  for(auto &&l : layout.node_layouts)
  {
    nodes.at(l.node)->setPos(l.position);
  }
}

ui_connection *ui_scene::create_connection(ui_node *source, int source_port)
{
  if(last_uid == std::numeric_limits<int64_t>::max())
  {
    throw std::runtime_error("ui_scene: exceeded id space");
  }
  else
  {
    ++last_uid;
  }

  connection_instance_id id{last_uid};

  auto connection = new ui_connection(source, source_port);
  add_connection(id, connection);
  return connection;
}

bool ui_scene::is_input_connected(ui_node *node, int port)
{
  using namespace boost::adaptors;

  auto get_value = [](ui_connection *connection)
  {
    return connection->get_destination();
  };
  auto range = connections | map_values | transformed(std::ref(get_value));
  return find(range, std::make_pair(node, port)) != end(range);
}

bool ui_scene::is_output_connected(ui_node *node, int port)
{
  using namespace boost::adaptors;

  auto get_value = [](ui_connection *connection)
  {
    return connection->get_source();
  };
  auto range = connections | map_values | transformed(std::ref(get_value));
  return find(range, std::make_pair(node, port)) != end(range);
}

void ui_scene::remove_connection(connection_instance_id id)
{
  connections.erase(id);
}

void ui_scene::remove_node(node_instance_id id)
{
  nodes.erase(id);
}

void ui_scene::add_connection(connection_instance_id id, ui_connection *connection)
{
  addItem(connection);
  connections.emplace(id, connection);
  connect(connection, &QObject::destroyed, std::bind(&ui_scene::remove_connection, this, id));
}

void ui_scene::add_node(node_instance_id id, ui_node *node)
{
  addItem(node);
  nodes.emplace(id, node);
  connect(node, &QObject::destroyed, std::bind(&ui_scene::remove_node, this, id));
}

} // namespace skadi
