#pragma once

#include "graph.h"
#include "picojson.h"

namespace skadi
{

picojson::value save(data_type);
data_type load_data_type(picojson::value);

picojson::value save(node_type);
node_type load_node_type(picojson::value);

picojson::value save(type_registry);
type_registry load_type_registry(picojson::value);

picojson::value save(node);
node load_node(picojson::value);

picojson::value save(connection);
connection load_connection(picojson::value);

picojson::value save(graph);
graph load_graph(picojson::value);

} // namespace skadi
