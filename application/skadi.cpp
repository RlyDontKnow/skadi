#include "graph_io.h"
#include "picojson.h"
#include "ui_view.h"
#include "ui_scene.h"

#include <fstream>
#include <iterator>

#include "QtWidgets/QApplication"

using namespace skadi;

picojson::object load_config(std::string const &config_file)
{
  std::ifstream fs(config_file);
  picojson::value v;
  fs >> v;
  auto err = picojson::get_last_error();
  if(!err.empty())
  {
    throw std::runtime_error(err);
  }
  return v.get<picojson::object>();
}

void save(picojson::object const &config, std::string const &config_file)
{
  std::ofstream fs(config_file);
  picojson::value(config).serialize(std::ostreambuf_iterator<char>(fs), true);
  
  auto err = picojson::get_last_error();
  if(!err.empty())
  {
    throw std::runtime_error(err);
  }
}

int main(int argc, char *argv[])
try
{
  QApplication app{argc, argv};

  auto config_file = (argc > 1) ? argv[1] : "test.json";
  auto config = load_config(config_file);
  auto type_registry = load_type_registry(config["type_registry"]);

  ui_scene scene(type_registry);
  ui_view view(&scene);
  
  try
  {
    scene.set_content(load_graph(config["graph"]));
    scene.set_layout(load_graph_layout(config["layout"]));
  }
  catch(std::runtime_error &)
  {
    // nothing to be done - just start fresh if it failed
  }
  
  view.setWindowTitle("editor");
  view.resize(1280, 960);
  view.show();

  int result = app.exec();

  config["layout"] = save(scene.get_layout());
  config["graph"] = save(scene.get_content());
  save(config, config_file);

  return result;
}
catch(std::exception &e)
{
  std::cerr << "unhandled exception: " << e.what() << std::endl;
}
