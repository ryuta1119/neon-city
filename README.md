# NEON CITY Ver.1.0

C言語・raylibを用いた2Dアクションゲーム。

## この公開用ソースについて

`NEON_CITY_v1.0_visual_audio`は既存ゲームへの更新用ファイルです。
不足する基礎モジュール・マップ・タイトルBGMは既存NEON_CITYから補い、更新対象7ファイルがVer.1.0と一致することを確認しました。
元フォルダを変更せず、公開用コピーにのみブラウザ対応を追加しています。
既存の個人セーブ、ローカルビルド、編集環境の設定は収録していません。

## 現在の機能

- CENTRAL HUB / TRAINING ARENA
- 自動生成ミッション（殲滅・生存・タイムアタック）
- 射撃・近接攻撃・ダッシュ
- Credits、ショップ、装備の購入・強化
- HP、装備、ミッション情報、簡易マップを含むHUD
- タイトルBGM「Bass Over」
- セーブ・コンティニュー

エリア別BGMは構想です。music-sketches/のMIDIとガイドは制作参考資料であり、完成音源やゲーム内実装とは区別しています。ポートフォリオの完成音源「Neon After Hours」とも別の資料です。

## GitHubで公開

1. GitHubで公開リポジトリ `neon-city` を作成します。
2. このフォルダの中身をリポジトリ直下に配置します。隠しフォルダ `.github/workflows/pages.yml` も必ず含めます。
3. リポジトリの Settings → Pages → Source を **GitHub Actions** に変更します。
4. mainブランチへpushするか、Actionsで「Build and publish NEON CITY」を手動実行します。
5. ビルドとDeployの両方が成功すると、Pagesに公開URLが表示されます。

自動ビルドはEmscripten 4.0.10とraylib 6.0を使用します。
ワークフローは公式のEmscripten SDKとraylibをダウンロードします。

## 検証状況

- Mac版：この公開用コピーをローカルのraylib 6.0でビルド成功。
- ブラウザ版：対応コード・ビルド設定を用意。Emscriptenがローカルになく、ネットワーク制限により導入できないため、WebAssemblyのビルド・ブラウザでのプレイ・公開は未確認。
- 公開後はメニュー、音量、マウス座標、戦闘、ショップ、セーブ後の再読み込みを確認する必要があります。

## ローカルビルド

Mac（raylibとCMakeが必要）:

```sh
cmake -S . -B build -DCMAKE_PREFIX_PATH="$(brew --prefix)"
cmake --build build
./build/neon_city
```

ブラウザ版（Emscripten SDKを有効化後）:

```sh
emcmake cmake -S . -B build-web -DCMAKE_BUILD_TYPE=Release
cmake --build build-web --parallel 2
python3 -m http.server 8000 --directory build-web
```

`http://localhost:8000`をブラウザで開き、読み込み後に「プレイ開始」を押します。HTMLを直接ダブルクリックしても起動できません。

## 操作

WASD：移動、左クリック：射撃、右クリック：近接攻撃、Shift：ダッシュ。
E：端末、↑↓・Enter：選択・決定、Esc：戻る、P：拠点でセーブ、R：依頼再生成・再挑戦。
PCのキーボードとマウス向けです。

ブラウザ版はlocalStorageへセーブを保存する設計です。同じブラウザ・同じサイトでのみ継続でき、ストレージが無効・消去された場合は復元できません。ブラウザの終了操作はタブ自体を閉じません。
