#pragma once
#include <QObject>
#include <unordered_set>

namespace dg {
	class ColumnTarget :
		public QObject
	{
		Q_OBJECT
		private:
			using Target = std::unordered_set<int>;
		private:
			Target		_target;

		public:
			explicit ColumnTarget(QObject* parent=nullptr);
			void addTarget(const QString& tableName, const char *const *columnNames, size_t n);
			template <size_t N>
			void addTarget(const QString& tableName, const char* const (&columnNames)[N]) {
				addTarget(tableName, static_cast<const char *const *>(columnNames), N);
			}

			void addTarget(int column);
			void remTarget(int column);
			bool has(int column) const;
	};
}
