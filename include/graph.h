#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace skadi
{

struct data_type_id
{
  int64_t guid;
};

struct data_type
{
  data_type_id guid;
  std::string name;
};

struct input
{
  data_type_id type;
  std::string name;
};

struct output
{
  data_type_id type;
  std::string name;
};

struct node_type_id
{
  int64_t guid;
};

struct node_type
{
  node_type_id guid;
  std::string name;
  std::string category;

  std::vector<input> inputs;
  std::vector<output> outputs;
};

struct type_registry
{
  std::vector<data_type> data_types;
  std::vector<node_type> node_types;
};

struct node_instance_id
{
  int64_t id;
};

struct node
{
  node_instance_id uid;
  node_type_id type;
};

struct connection_instance_id
{
  int64_t id;
};

struct connection
{
  connection_instance_id uid;

  node_instance_id source;
  std::string signal;

  node_instance_id destination;
  std::string slot;
};

struct graph
{
  std::vector<node> nodes;
  std::vector<connection> connections;
};

} // namespace skadi
