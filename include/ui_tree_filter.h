#pragma once

#include "QtCore/QSortFilterProxyModel"

namespace skadi
{

class ui_tree_filter
  : public QSortFilterProxyModel
{
  Q_OBJECT

public:
  ui_tree_filter(QObject *parent);

private:
  bool filterAcceptsRow(int, QModelIndex const &) const override;
};

} // namespace skadi
