#include "graph_io.h"

namespace skadi
{

picojson::value save(data_type t)
{
  picojson::object o;
  o["guid"] = picojson::value(t.guid.guid);
  o["name"] = picojson::value(t.name);
  return picojson::value(o);
}

data_type load_data_type(picojson::value v)
{
  auto o = v.get<picojson::object>();

  data_type t{};
  t.guid.guid = o["guid"].get<int64_t>();
  t.name = o["name"].get<std::string>();
  return t;
}

picojson::value save(node_type t)
{
  picojson::object o;
  o["guid"] = picojson::value(t.guid.guid);
  o["name"] = picojson::value(t.name);
  o["category"] = picojson::value(t.category);

  picojson::array inputs;
  for(auto &&p : t.inputs)
  {
    picojson::object input;
    input["name"] = picojson::value(p.name);
    input["type"] = picojson::value(p.type.guid);
    inputs.emplace_back(input);
  }
  o["inputs"] = picojson::value(inputs);

  picojson::array outputs;
  for(auto &&p : t.outputs)
  {
    picojson::object output;
    output["name"] = picojson::value(p.name);
    output["type"] = picojson::value(p.type.guid);
    outputs.emplace_back(output);
  }
  o["outputs"] = picojson::value(outputs);

  return picojson::value(o);
}

node_type load_node_type(picojson::value v)
{
  node_type t{};

  auto o = v.get<picojson::object>();
  t.guid.guid = o["guid"].get<int64_t>();
  t.name = o["name"].get<std::string>();
  t.category = o["category"].get<std::string>();

  for(auto &&value : o["inputs"].get<picojson::array>())
  {
    picojson::object obj = value.get<picojson::object>();

    input p{};
    p.name = obj["name"].get<std::string>();
    p.type.guid = obj["type"].get<int64_t>();
    t.inputs.emplace_back(p);
  }

  for(auto &&value : o["outputs"].get<picojson::array>())
  {
    picojson::object obj = value.get<picojson::object>();

    output p{};
    p.name = obj["name"].get<std::string>();
    p.type.guid = obj["type"].get<int64_t>();
    t.outputs.emplace_back(p);
  }

  return t;
}

picojson::value save(type_registry r)
{
  picojson::object o;

  picojson::array node_types;
  for(auto &&t : r.node_types)
  {
    node_types.emplace_back(save(t));
  }
  o["node_types"] = picojson::value(node_types);

  picojson::array data_types;
  for(auto &&t : r.data_types)
  {
    data_types.emplace_back(save(t));
  }
  o["data_types"] = picojson::value(data_types);

  return picojson::value(o);
}

type_registry load_type_registry(picojson::value v)
{
  type_registry r{};

  auto o = v.get<picojson::object>();

  for(auto &&t : o["node_types"].get<picojson::array>())
  {
    r.node_types.emplace_back(load_node_type(t));
  }

  for(auto &&t : o["data_types"].get<picojson::array>())
  {
    r.data_types.emplace_back(load_data_type(t));
  }

  return r;
}

picojson::value save(node n)
{
  picojson::object o;

  o["uid"] = picojson::value(n.uid.id);
  o["type"] = picojson::value(n.type.guid);

  return picojson::value(o);
}

node load_node(picojson::value v)
{
  node n{};

  auto o = v.get<picojson::object>();

  n.uid.id = o["uid"].get<int64_t>();
  n.type.guid = o["type"].get<int64_t>();

  return n;
}

picojson::value save(connection c)
{
  picojson::object o;

  o["uid"] = picojson::value(c.uid.id);
  o["source"] = picojson::value(c.source.id);
  o["signal"] = picojson::value(c.signal);
  o["destination"] = picojson::value(c.destination.id);
  o["slot"] = picojson::value(c.slot);

  return picojson::value(o);
}

connection load_connection(picojson::value v)
{
  connection c{};

  auto o = v.get<picojson::object>();

  c.uid.id = o["uid"].get<int64_t>();
  c.source.id = o["source"].get<int64_t>();
  c.signal = o["signal"].get<std::string>();
  c.destination.id = o["destination"].get<int64_t>();
  c.slot = o["slot"].get<std::string>();

  return c;
}

picojson::value save(graph g)
{
  picojson::object o;

  picojson::array nodes;
  for(auto &&t : g.nodes)
  {
    nodes.emplace_back(save(t));
  }
  o["nodes"] = picojson::value(nodes);

  picojson::array connections;
  for(auto &&t : g.connections)
  {
    connections.emplace_back(save(t));
  }
  o["connections"] = picojson::value(connections);

  return picojson::value(o);
}

graph load_graph(picojson::value v)
{
  graph g{};

  auto o = v.get<picojson::object>();

  for(auto &&t : o["nodes"].get<picojson::array>())
  {
    g.nodes.emplace_back(load_node(t));
  }

  for(auto &&t : o["connections"].get<picojson::array>())
  {
    g.connections.emplace_back(load_connection(t));
  }

  return g;
}

} // namespace skadi
