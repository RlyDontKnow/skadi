#include "graph_io.h"
#include "picojson.h"
#include "ui_library.h"
#include "ui_scene.h"
#include "ui_tree_filter.h"
#include "ui_view.h"

#include <fstream>
#include <iterator>

#include "QtWidgets/QApplication"
#include "QtWidgets/QDockWidget"
#include "QtWidgets/QLineEdit"
#include "QtWidgets/QMainWindow"
#include "QtWidgets/QTreeView"
#include "QtWidgets/QVboxLayout"

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

void setup_ui(ui_view *scene_view, ui_library_model *library_model)
{
  auto window = new QMainWindow;
  window->setObjectName("Skadi");
  window->resize(1280, 960);
  auto widget = new QWidget(window);
  auto widget_layout = new QVBoxLayout(widget);
  widget_layout->addWidget(scene_view);
  window->setCentralWidget(widget);

  auto dock = new QDockWidget(window);
  window->addDockWidget(static_cast<Qt::DockWidgetArea>(1), dock);
  dock->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
  auto dock_widget = new QWidget();
  dock->setWidget(dock_widget);
  auto dock_layout = new QVBoxLayout(dock_widget);

  auto library_filter = new QLineEdit(dock_widget);
  dock_layout->addWidget(library_filter);
  auto library_view = new QTreeView(dock_widget);
  dock_layout->addWidget(library_view);
  library_view->setHeaderHidden(true);
  library_view->setDragEnabled(true);
  auto filtered_library_model = new ui_tree_filter(library_view);
  filtered_library_model->setSourceModel(library_model);
  library_view->setModel(filtered_library_model);
  library_view->expandAll();
  QObject::connect(library_filter, &QLineEdit::textChanged, filtered_library_model, &ui_tree_filter::setFilterWildcard);

  window->show();
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

  ui_library_model library_model(type_registry);
  
  try
  {
    scene.set_content(load_graph(config["graph"]));
    scene.set_layout(load_graph_layout(config["layout"]));
    view.centerOn(scene.itemsBoundingRect().center());
  }
  catch(std::runtime_error &)
  {
    // nothing to be done - just start fresh if it failed
  }
  
  setup_ui(&view, &library_model);

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
