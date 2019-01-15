# Sprinkler

## 概要
デスクトップの空き領域に画像をばら撒くソフトです
![](https://github.com/degarashi/sprinkler/blob/images/sprinkler_v010_sc0.jpg)

## 既知の問題点
* 画像を表示する時に一旦フルサイズで読み込み、それをリサイズしている関係で容量の大きいファイルが多数あると画像配置の計算後、表示されるまで10秒以上秒かかる
* 対応している画像形式はpng, jpg, bmp, gifのみ
* 画像が操作ウィンドウで隠れてしまっても自動で位置を補正してくれない
* (Windows環境にて)仮想スクリーンの追加・削除を最初の一度しか検知できない

## 更新履歴
### v0.1.4 (2019/01/15)
* 画像のチェックサム計算を簡略化し、ディレクトリの走査時間を削減  
* ディレクトリの走査状況をウィンドウの下部に表示  
* ディレクトリ名由来のタグが関連付け解除出来てしまうのを修正  
### v0.1.3 (2019/01/08)
* SQLクエリをトランザクションで纏めて効率化  
* 画像配置のパラメータをサイズではなく平均画像数で指定  
* 親がルートのディレクトリを画像ソースに指定した場合の不具合を修正(linux)  
* 画像候補が一枚しか無かった時の内部ステート管理を修正  
### v0.1.2 (2019/01/05)
* SQLiteデータベースに追加のインデックスを定義  
* SQLiteデータベースファイルをCacheLocationからConfigLocationへ変更  
* SQLiteデータベースのバージョンとアプリケーション設定ファイルのバージョンを別個に管理  
* 中断ボタンが効かない不具合を修正  
* タグによる画像検索クエリを効率化  
* アプリケーション設定ファイルをNative(Windowsではレジストリ)からINIフォーマットへ変更  
* 画像ソースダイアログがメインウィンドウより常に上に表示されていたのを修正  
* ウィンドウ位置の保存、復元が正常に出来ていない問題を修正  

### v0.1.1 (2018/12/29)
* 選択中タグに関連する画像の表示済みフラグのクリア  
![v011_flagclear](https://github.com/degarashi/sprinkler/blob/images/sprinkler_v011_flagclear.png)  
* 任意のタグ付け & 削除  
![v011_menu](https://github.com/degarashi/sprinkler/blob/images/sprinkler_v011_menu0.jpg)  
![v011_taginput](https://github.com/degarashi/sprinkler/blob/images/sprinkler_v011_taginput.png)  

### v0.1.0 (2018/12/25)
* SQLiteモジュールを組み込み、画像管理を一括化
* 画像のサムネイルをキャッシュするようにした
* ディレクトリ構成から画像を簡易的にタグ付けし、それを指定して散布
![v010_source](https://github.com/degarashi/sprinkler/blob/images/sprinkler_v010_source.png)
* タグの検索結果のプレビュー
![v010_preview](https://github.com/degarashi/sprinkler/blob/images/sprinkler_v010_preview.png)
* その他、UIの変更
![v010_rectview](https://github.com/degarashi/sprinkler/blob/images/sprinkler_v010_rectview.png)

### v0.0.5 (2018/10/04)
* 画像セットが空になった、画像配置計算した結果一枚も画像が置けなかった等のエラー時、または画像配置の結果、何枚置けたかをメッセージで表示  
![v005_ntf](https://github.com/degarashi/sprinkler/blob/images/sprinkler_v005_ntf.png)
![v005_Nntf](https://github.com/degarashi/sprinkler/blob/images/sprinkler_v005_Nntf.png)  
* 画像をCtrl + 右クリックすることでSprinklerを操作できるようになりました  
![v005_ctrlmenu](https://github.com/degarashi/sprinkler/blob/images/sprinkler_v005_ctrlmenu.png)  
* キープした画像を緑色の枠で強調表示(Sprinklerがアクティブになった時のみ)
![v005_keep](https://github.com/degarashi/sprinkler/blob/images/sprinkler_v005_keep.png)  
* 配置された画像のどれか1つをクリックすると他の画像も全て前面に出すように変更

### v0.0.4 (2018/10/02)
* タスクトレイのアイコンにカーソルを合わせると、現在どのくらいの画像を表示済みかを表示します  
![v004_indicator](https://github.com/degarashi/sprinkler/blob/images/sprinkler_v004_indicator.png)  
* 「画像散布時にウィンドウを隠す」にチェックを入れた上で画像を散布するとウィンドウを隠した後、散布するようにしました  
![v004_hide](https://github.com/degarashi/sprinkler/blob/images/sprinkler_v004_hide.png)  
* キープした画像の一括削除ボタンを追加

### v0.0.3 (2018/09/30)
* タスクトレイにアイコンを表示し、そこから各種操作をできるようにしました。

### v0.0.2 (2018/09/28)
* UIの日本語表示に対応

### v0.0.1 (2018/09/27)
* ひとまず公開

## 使い方
1. DirList ボタンを押してDirectoryListウィンドウを表示させ、Addボタンで表示対象の画像が入ったディレクトリを指定します
![](https://github.com/degarashi/sprinkler/blob/images/sprinkler_sc2.jpg)

2. 右下に総画像数が表示されるのでSprinkleボタンを押すと数秒の計算時間を経て画像をばらまきます

3. (Optional) WatchListボタンを押すとWatchListウィンドウが開きます。ここでAddを押す(Linux版)か、Addと書いてある領域をドラッグするとウィンドウを登録することが出来、画像はその領域を除けて配置されます
![](https://github.com/degarashi/sprinkler/blob/images/sprinkler_sc1.jpg)

4. MenuからQuitを選んで終了します

### タスクトレイについて
メインウィンドウを最小化するとタスクバーから隠れ、タスクトレイから操作可能になります。
この時アイコンを左クリックすると簡易メニューが、  
![Tasktray-MenuL](https://github.com/degarashi/sprinkler/blob/images/sprinkler_v003_menu0.png)  
右クリックで詳細メニューが出ます  
![Tasktray-MenuR](https://github.com/degarashi/sprinkler/blob/images/sprinkler_v003_menu1.png)  
再びメインウィンドウを表示したい時は「ウィンドウを表示」を選んで下さい。
