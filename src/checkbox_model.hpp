#pragma once
#include "column_target.hpp"
#include <QModelIndex>
#include <QDebug>

namespace dg {
	template <class Base>
	class CheckBoxModel :
		public Base
	{
		private:
			using base_t = Base;
			const ColumnTarget*	_column;

		protected:
			bool _isCheckBox(const QModelIndex& index, const int role) const {
				return role == Qt::CheckStateRole &&
						_column->has(index.column());
			}

		public:
			CheckBoxModel(const ColumnTarget* column, QObject* parent):
				base_t(parent),
				_column(column)
			{}
			Qt::ItemFlags flags(const QModelIndex &index) const {
				if(_column->has(index.column()))
					return base_t::flags(index) | Qt::ItemIsUserCheckable;
				return base_t::flags(index);
			}
			bool setData(const QModelIndex &index, const QVariant &value, const int role=Qt::EditRole) {
				// 該当のカラム番号 & CheckStateRoleの時だけ処理を変える
				if(_isCheckBox(index, role)) {
					// Checkbox Flagでセットする
					const int flag = value.toBool() ?
						Qt::Checked : Qt::Unchecked;
					return base_t::setData(index, flag, role);
				}
				// それ以外はベースクラスの挙動
				return base_t::setData(index, value, role);
			}
			QVariant data(const QModelIndex &idx, const int role=Qt::DisplayRole) const {
				if(_isCheckBox(idx, role)) {
					// Checkbox Flagで返す
					return base_t::data(idx, Qt::DisplayRole).toBool() ?
						Qt::Checked : Qt::Unchecked;
				} else if(role == Qt::ToolTipRole)
					return base_t::data(idx, Qt::DisplayRole).toString();

				// それ以外はデフォルトの挙動
				return base_t::data(idx, role);
			}
	};
}
