![Bruce のメインメニュー](./media/pictures/bruce_banner.jpg)

# Bruce 日本語化プロジェクト

このリポジトリは、ESP32向けファームウェア [Bruce](https://github.com/BruceDevices/firmware) を日本語で使いやすくするための派生プロジェクトです。画面メニューなどの日本語化を進めながら、本家 Bruce の機能と対応機器をベースに開発しています。

日本語表示は既定で有効です。翻訳がまだ用意されていない表示は、英語のまま表示されます。

## 主な機能

使える機能は機種や接続している周辺機器によって異なります。

- **Wi-Fi** — 接続、アクセスポイント、スキャン、ネットワーク検証機能
- **Bluetooth LE** — スキャン、対応機器向けの各種機能
- **サブGHz・無線** — CC1101などを使った受信・送信、信号の記録・再生、スペクトラム表示
- **ESP-SDR** — LilyGO T-Embed CC1101向けの受信機能とスペクトラム表示
- **RFID・NFC** — 対応リーダーを使った読み取り、書き込み、データ管理
- **赤外線・FM・NRF24** — 対応モジュールを使った受信・送信や信号の確認
- **GPS・ファイル・スクリプト** — GPS連携、SDカード管理、WebUI、JavaScriptなど
- **設定・周辺機能** — 時計、画面表示、キーボード、USB機能など

## 対応機器

M5Stack、LILYGO、Elecrow、CYD系など、複数のESP32機器に対応しています。主な対象にはM5Stack Cardputer、M5StickC、LILYGO T-Embed CC1101、T-Deck、T-Display-S3などがあります。

機種ごとのビルド環境は<code>platformio.ini</code>と<code>boards/</code>以下の設定ファイルで確認できます。すべての機能がすべての機種で使えるわけではありません。

## ビルドと書き込み

PlatformIO Coreを用意し、リポジトリのルートでビルドします。

    # platformio.iniで指定された既定環境をビルド
    pio run

    # LilyGO T-Embed CC1101向けにビルド
    pio run -e lilygo-t-embed-cc1101

書き込みは対象機器のシリアルポートを指定して実行します。

    pio run -e lilygo-t-embed-cc1101 -t upload --upload-port COMx

Windowsでは<code>COMx</code>を実際のポート名に、Linuxでは<code>/dev/ttyACM0</code>などのポート名に置き換えてください。ビルドすると、リポジトリのルートに<code>Bruce-&lt;環境名&gt;.bin</code>が生成されます。

## 日本語訳への参加

画面表示の翻訳表は[include/i18n.h](include/i18n.h)にあります。英語の表示文字列をキーにして日本語訳を登録します。新しい表示を日本語化するときは、元の英語表記をキーとして追加してください。翻訳表にキーがない場合は、元の文字列がそのまま表示されます。

画面幅に収まる短い表現を使い、機種の画面で文字切れがないか確認してください。修正案や翻訳の追加は、IssueまたはPull Requestで歓迎します。

## 参考リンク

- [本家 Bruce のソースコード](https://github.com/BruceDevices/firmware)
- [本家 Bruce のWiki](https://wiki.bruce.computer/)（英語）
- [このプロジェクトのリポジトリ](https://github.com/sake846/Bruce-jp)
- [サードパーティのライセンスとクレジット](THIRD_PARTY.md)

## 利用上の注意

無線機能やセキュリティ検証機能を使う場合は、自分が所有する機器や、明示的な許可を得た環境だけで使用してください。利用地域の法令と無線に関する規則を守ってください。

## ライセンス

本プロジェクトのライセンスはGNU Affero General Public License v3.0以降（AGPL-3.0-or-later）です。詳細は[LICENSE](LICENSE)を確認してください。組み込まれているソフトウェアの個別ライセンスと出典は[THIRD_PARTY.md](THIRD_PARTY.md)に記載しています。

## クレジット

本家Bruceの開発者とコントリビューター、およびESP32Marauder、Launcher、Evil-M5Project、M5Stick-Nemoなどの関連プロジェクトに感謝します。個別のコードやライブラリの出典は[THIRD_PARTY.md](THIRD_PARTY.md)を参照してください。
