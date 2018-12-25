#pragma once

namespace dg::sql {
	namespace clause {
		namespace detail {
			struct SepBase {
				// Separator(IsNull)
				static auto Separator(std::true_type) {
					return QStringLiteral("");
				}
			};
			struct ANDSep : SepBase {
				using SepBase::Separator;
				static auto Separator(std::false_type) {
					return QStringLiteral(" AND ");
				}
			};
			struct CommaSep : SepBase {
				using SepBase::Separator;
				static auto Separator(std::false_type) {
					return QStringLiteral(", ");
				}
			};
			struct WhereDirect : ANDSep {
				template <class Key>
				static QString keyval(const Key& key, const QVariant& val) {
					if(val.isNull())
						return QStringLiteral(" IS NULL");
					return "=" % val.toString();
				}
			};
			struct SetDirect : CommaSep {
				template <class Key>
				static QString keyval(const Key& key, const QVariant& val) {
					if(val.isNull())
						return QStringLiteral("=NULL");
					return "=" % val.toString();
				}
			};

			template <class Exp, class First, class Str>
			QString _KeyValue(Str&& str) {
				return QString(str);
			}
			template <class Exp, class First, class Str, class Key, class... Args>
			QString _KeyValue(Str&& str, const Key& key, const QVariant& val, const Args&... args) {
				return _KeyValue<Exp, std::false_type>(
							str % Exp::Separator(First{}) % Exp::keyval(key, val),
							args...
						);
			}
			template <class Exp, class... Args>
			QString KeyValue(const Args&... args) {
				return _KeyValue<Exp, std::true_type>("", args...);
			}

			template <class Exp, class First, class Str>
			QString _Keys(Str&& str) {
				return QString(str);
			}
			template <class Exp,  class First, class Str, class Key, class... Args>
			QString _Keys(Str&& str, const Key& key, const Args&... args) {
				return _Keys<Exp, std::false_type>(
					str % Exp::Separator(First{}) % key % "=?",
					args...
				);
			}
			template <class Exp, class... Args>
			QString Keys(const Args&... args) {
				return _Keys<Exp, std::true_type>("", args...);
			}
		}

		template <class... Args>
		QString WhereDirect(const Args&... args) {
			return "WHERE " % detail::KeyValue<detail::WhereDirect>(args...);
		}
		template <class... Args>
		QString Where(const Args&... args) {
			return "WHERE " % detail::Keys<detail::ANDSep>(args...);
		}
		template <class... Args>
		QString SetDirect(const Args&... args) {
			return "SET " % detail::KeyValue<detail::SetDirect>(args...);
		}
	}
	template <class... Args>
	bool HasEntry(const char* table, const Args&... args) {
		const auto q = Query(
			QStringLiteral("SELECT COUNT(*) FROM ") % table % " " % _FindEntryQuery(args...) % ";",
			args...
		);
		return q.next();
	}
	template <class CB>
	void DetectVariant(const QVariant::Type type, const CB& cb) {
		switch(type) {
			case QVariant::Int:
				return cb((int32_t*)nullptr);
			case QVariant::LongLong:
				return cb((int64_t*)nullptr);
			case QVariant::String:
				return cb((QString*)nullptr);
			case QVariant::Bool:
				return cb((bool*)nullptr);
			default:
				throw std::runtime_error("unknown type");
		}
	}
	inline void PrintTable(const char* table) {
		qDebug() << "[" << table << "]";
		qDebug() << "-------------------------------------";
		auto q = Query(QString("SELECT * FROM %1").arg(table));
		const QSqlRecord rec = q.record();
		const int nc = rec.count();
		{
			auto out = qDebug();
			for(int i=0 ; i<nc ; i++) {
				if(i > 0)
					out << "\t|";
				out << rec.field(i).name().toStdString().c_str();
			}
		}
		while(q.next()) {
			auto out = qDebug();
			for(int i=0 ; i<nc ; i++) {
				if(i > 0)
					out << "\t";
				const QVariant& val = q.value(i);
				if(val.isNull())
					out << "<null>";
				else {
					DetectVariant(rec.field(i).type(), [&val, &out](auto* typ){
						out << detail::ConvertQV<std::remove_pointer_t<decltype(typ)>>(val);
					});
				}
			}
		}
		qDebug() << "-------------------------------------";
	}
}
