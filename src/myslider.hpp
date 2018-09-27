#pragma once
#include <QWidget>
#include <memory>
#include <QSlider>

namespace Ui {
	class MySlider;
}

class QAbstractItemModel;
namespace dg {
	class MySlider :
		public QWidget
	{
		Q_OBJECT

		private:
			std::shared_ptr<Ui::MySlider>	_ui;
			QAbstractItemModel*	_model;
			int					_index;
			QString				_suffix;
		public:
			explicit MySlider(QWidget* parent=nullptr);
			void setName(const QString& name);
			QSlider* refSlider() const noexcept;
			void setModel(QAbstractItemModel* model, int index);
			void setSuffix(const QString& sfx);
		protected:
			virtual QString _makeValueString(int value) const = 0;
			virtual int _fromVariant(const QVariant& v) = 0;
			virtual QVariant _toModel(int value) = 0;
		private slots:
			void valueChanged(int n);
			void dataChanged(const QModelIndex& lt, const QModelIndex& br, const QVector<int>& role);
	};
}
