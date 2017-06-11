#pragma once

#include "graph.h"

#include <variant>

#include "QtGui/QStandardItem"
#include "QtGui/QStandardItemModel"

namespace skadi
{

class ui_library_model
  : public QAbstractItemModel
{
  Q_OBJECT

public:
  explicit ui_library_model(type_registry const &);
  ~ui_library_model();

  QVariant data(QModelIndex const &index, int role) const override;
  Qt::ItemFlags flags(QModelIndex const &index) const override;
  QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
  QModelIndex parent(QModelIndex const &index) const override;
  int rowCount(QModelIndex const &parent = QModelIndex()) const override;
  int columnCount(QModelIndex const &parent = QModelIndex()) const override;

private:
  Qt::DropActions supportedDragActions() const override;
  Qt::DropActions supportedDropActions() const override;
  QStringList mimeTypes() const override;
  QMimeData *mimeData(QModelIndexList const &) const override;

  bool is_valid_index(QModelIndex const &) const;
  struct node_item;
  struct category_item;
  struct index_data;
  using item = std::variant<category_item, node_item>;

  std::vector<index_data> index_map;
  std::vector<int> index_reverse_map;
  std::vector<item> model;
};

} // namespace skadi
