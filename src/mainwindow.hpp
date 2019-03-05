#pragma once
#include "widget/gr_mainwindow.hpp"
#include "idtype.hpp"
#include <memory>

namespace Ui {
	class MainWindow;
}

namespace dg {
	class DBTag;
	class DBImage;
	class DatabaseSignal;
	namespace place {
		struct Result;
		using ResultV = QVector<Result>;
	}
	namespace widget {
		class ImageLabel;
	}
}
class QSystemTrayIcon;
namespace dg { namespace widget {
	class MainWindow :
		public widget::GeomRestore_MW
	{
		Q_OBJECT
		private:
			using base_t = widget::GeomRestore_MW;
			using LabelV = std::vector<widget::ImageLabel*>;
			//! ステートインタフェース
			struct State {
				virtual ~State() {}
				virtual void onEnter(MainWindow& self) {}
				virtual void onExit(MainWindow& self) {}
				virtual void onSprinkle(MainWindow& self) = 0;
				virtual void onStop(MainWindow& self) = 0;
				virtual void onReposition(MainWindow& self) = 0;
				virtual void onSprinkleResult(MainWindow& self, const dg::place::ResultV& r) = 0;
				virtual void onSprinkleAbort(MainWindow&) = 0;
			};
			using State_U = std::unique_ptr<State>;

			struct IdleState : State {
				void onSprinkle(MainWindow& self) override;
				void onStop(MainWindow& self) override;
				void onReposition(MainWindow& self) override;
				void onSprinkleResult(MainWindow&, const dg::place::ResultV&) override;
				void onSprinkleAbort(MainWindow&) override;
			};
			//! 配置計算中(Geneスレッドの計算完了を待っている)
			struct ProcState : State {
				// Geneスレッドに送るタグ番号リスト
				TagIdV		_tag;
				ImageIdV	_img;
				// true: Sprinklerのsprinkle()を呼ぶ
				// false: SprinklerのsprinkleImageSet()を呼ぶ
				bool		_procTag;
				// 新しい画像セットを示すフラグ(true時は前の画像セットを置き換え)
				bool		_newSet;

				ProcState(TagIdV&& tag, bool newSet);
				ProcState(const ImageIdV& img, bool newSet);
				void _setEnable(MainWindow& self, bool b);
				void onEnter(MainWindow&) override;
				void onExit(MainWindow&) override;
				void onSprinkle(MainWindow& self) override;
				void onStop(MainWindow& self) override;
				void onReposition(MainWindow& self) override;
				void onSprinkleResult(MainWindow&, const dg::place::ResultV&) override;
				void onSprinkleAbort(MainWindow&) override;
			};
			//! 配置計算をキャンセル後、Geneスレッドの確認通知を待っている
			struct AbortState : State {
				void onSprinkle(MainWindow& self) override;
				void onStop(MainWindow& self) override;
				void onReposition(MainWindow& self) override;
				void onSprinkleResult(MainWindow&, const dg::place::ResultV&) override;
				void onSprinkleAbort(MainWindow&) override;
			};

		private:
			std::shared_ptr<Ui::MainWindow>	_ui;
			State_U							_state;
			DBTag							*_dbTag;
			DBImage							*_dbImg;
			QMenu							*_ctrlMenu;
			QSystemTrayIcon					*_tray;
			// Label: 配置した画像
			LabelV							_label;
			// 前回配置した画像リスト
			// 画像ソースを変更するとリセット
			ImageIdV						_prevImg;

			void _setState(State_U state);
			//! [OpenDir, OpenTag, OpenRect, OpenWatch]ウィンドウを非表示にする
			void _hideSubWindow();
			//! 配置した画像をクリア
			void _clearLabels();
			// タグリストを引数にして画像の残数カウンタ更新
			void _refresh_counter();
			// 現在選択しているタグに対して画像の残量カウンタを更新
			void _refresh_counter_tag(const TagIdV& tag);

		public:
			MainWindow(
				DBImage *img,
				DBTag *tag,
				DatabaseSignal *sig,
				QWidget *parent=nullptr
			);
		protected:
			void closeEvent(QCloseEvent* e) override;
			void changeEvent(QEvent* e) override;
		public slots:
			void stayOnTop(bool b);
			//! 画像配置の進捗
			void sprinkleProgress(int percent);
			//! 画像配置の開始
			void sprinkle();
			void stop();
			//! 前回の画像配置セットをもう一度配置
			void reposition();
			void resetViewFlagSelecting();
			void resetViewFlagAll();
		signals:
			// 対象タグの変更、または表示された数の変更
			void remainingImageCounterChanged(const TagIdV& tag, size_t total, size_t shown);
			// 配置された画像のどれか1つがクリックされた
			void labelClicked();
	};
}}
