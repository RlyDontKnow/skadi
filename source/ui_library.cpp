#include "ui_library.h"

#include <algorithm>
#include <cassert>
#include <optional>
#include <memory>

#include "QtCore/QMimeData"

namespace skadi
{

struct ui_library_model::node_item
{
  node_type_id guid;
  std::string name;

  bool operator<(node_item const &rhs) const
  {
    return name < rhs.name;
  }
};

struct ui_library_model::category_item
{
  std::string name;

  bool operator<(category_item const &rhs) const
  {
    return name < rhs.name;
  }
};

struct ui_library_model::index_data
{
  int row;
  int row_count;
  int parent_row;
};

ui_library_model::ui_library_model(type_registry const &registry)
{
  std::vector<std::pair<category_item, node_item>> data;
  for(auto &&node_type : registry.node_types)
  {
    data.emplace_back(category_item{node_type.category}, node_item{node_type.guid, node_type.name});
  }

  std::sort(begin(data), end(data));

  std::optional<std::string> category;
  int category_index{};
  int node_index{};
  int category_node_index = -1;
  for(auto &&[category_item, node_item] : data)
  {
    if(category_item.name != category)
    {
      if(category_node_index >= 0)
      {
        index_map[category_node_index].row_count = node_index;
      }
      category_node_index = static_cast<int>(model.size());
      node_index = {};

      category = category_item.name;
      index_reverse_map.emplace_back(static_cast<int>(model.size()));
      index_map.push_back({category_index++, 0, -1});
      model.emplace_back(std::move(category_item));
    }

    model.emplace_back(std::move(node_item));
    index_map.push_back({node_index++, 0, category_node_index});
  }

  if(category_node_index >= 0)
  {
    index_map[category_node_index].row_count = node_index;
  }
}

ui_library_model::~ui_library_model() = default;

QVariant ui_library_model::data(QModelIndex const &index, int role) const
{
  if(!is_valid_index(index))
  {
    return{};
  }

  switch(role)
  {
  case Qt::DisplayRole:
    return visit([](auto &&v) { return v.name; }, model[index.internalId()]).data();

  default:
    return{};
  }
}

Qt::ItemFlags ui_library_model::flags(QModelIndex const &index) const
{
  if(is_valid_index(index))
  {
    if(model[index.internalId()].index() == 0) // category
    {
      return Qt::ItemIsEnabled;
    }
    else // node
    {
      return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDragEnabled;
    }
  }
  else
  {
    return{};
  }
}

QModelIndex ui_library_model::index(int row, int, QModelIndex const &parent) const
{
  if(row < 0)
  {
    // nothing to be done
  }
  else if(!is_valid_index(parent))
  {
    if(row < index_reverse_map.size())
    {
      return createIndex(row, 0, index_reverse_map[row]);
    }
  }
  else if(auto parent_row = parent.internalId(); row < index_map[parent_row].row_count)
  {
    assert(row == index_map[parent_row + 1 + row].row);
    return createIndex(row, 0, parent_row + row + 1);
  }
  return{};
}

QModelIndex ui_library_model::parent(QModelIndex const &index) const
{
  if(!is_valid_index(index))
  {
    return{};
  }

  auto const row = index_map[index.internalId()].parent_row;
  if(row >= 0)
  {
    return createIndex(index_map[row].row, 0, row);
  }
  else
  {
    return{};
  }
}

int ui_library_model::rowCount(QModelIndex const &index) const
{
  if(!is_valid_index(index))
  {
    return static_cast<int>(index_reverse_map.size());
  }
  else
  {
    return index_map[index.internalId()].row_count;
  }
}

int ui_library_model::columnCount(QModelIndex const &) const
{
  return 1;
}

Qt::DropActions skadi::ui_library_model::supportedDragActions() const
{
  return Qt::CopyAction;
}

Qt::DropActions skadi::ui_library_model::supportedDropActions() const
{
  return{};
}

QStringList skadi::ui_library_model::mimeTypes() const
{
  QStringList result;
  result.push_back("application/x-skadinodetype");
  result.push_back("text/plain");
  return result;
}

QMimeData *skadi::ui_library_model::mimeData(QModelIndexList const &indices) const
{
  if(indices.size() != 1)
  {
    return nullptr;
  }
  auto &&variant = model[indices[0].internalId()];
  if(variant.index() != 1) // not a node_item
  {
    return nullptr;
  }
  auto &&item = std::get<1>(variant);

  auto data = std::make_unique<QMimeData>();
  data->setText(QString::fromStdString(item.name));
  QByteArray guid(reinterpret_cast<char const *>(&item.guid), sizeof(item.guid));
  data->setData("application/x-skadinodetype", guid);
  return data.release();
}

bool ui_library_model::is_valid_index(QModelIndex const &index) const
{
  auto const row = index.internalId();
  return (index.model() == this)
    && (row >= 0) && (row < model.size());
}

} // namespace skadi
