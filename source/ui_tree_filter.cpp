#include "ui_tree_filter.h"

namespace skadi
{

ui_tree_filter::ui_tree_filter(QObject *parent)
  : QSortFilterProxyModel(parent)
{
}

bool ui_tree_filter::filterAcceptsRow(int row, QModelIndex const &parent) const
{
  auto model = sourceModel();
  auto index = model->index(row, 0, parent);
  bool result{};
  if(model->hasChildren(index))
  {
    for(int i{}; i < model->rowCount(index); ++i)
    {
      result = result || filterAcceptsRow(i, index);
    }
  }
  else
  {
    result = QSortFilterProxyModel::filterAcceptsRow(row, parent);
  }
  return result;
}

} // namespace skadi
