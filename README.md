# Sprinkler

## 概要
デスクトップの空き領域に画像をばら撒くソフトです
![](https://github.com/degarashi/sprinkler/blob/images/sprinkler_sc0.jpg)

## 既知の問題点
* 画像ディレクトリを指定する際、再帰的にファイルを検索するがこれを止めるオプションは無い
* 画像サイズをキャッシュしていないのでアプリケーション起動時に毎回ファイル走査が入る。(画像数が1000を越えてくると重い)
* 画像を表示する時に一旦フルサイズで読み込み、それをリサイズしている関係で容量の大きいファイルが多数あると画像配置の計算後、表示されるまで10秒以上秒かかる
* 対応している画像形式はpng, jpg, bmp, gifのみ
* 画像が操作ウィンドウで隠れてしまっても自動で位置を補正してくれない
* (Windows環境にて)仮想スクリーンの追加・削除を最初の一度しか検知できない

## 更新履歴
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
