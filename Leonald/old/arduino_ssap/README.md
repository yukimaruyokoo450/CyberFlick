<div id="top"></div>

## 目次

1. [プロジェクトについて](#プロジェクトについて)
2. [ディレクトリ構成](#ディレクトリ構成)
3. [開発環境構築](#開発環境構築)
4. [開発](#開発)
5. [その他](#その他)

<!-- READMEの作成方法のドキュメントのリンク -->
<br />
<div align="right">
    <a href="https://zenn.dev/n_haru2/articles/8443eea376ff53"><strong>READMEの作成方法 »</strong></a>
</div>
<!-- プロジェクトについて -->

## プロジェクトについて

### SSAPオーディション
リング型マウスの開発

<!-- プロジェクトの概要を記載 -->

  <p align="left">
    <!-- プロジェクト詳細にBacklogのWikiのリンク -->
    <a href="https://startdash.sony-startup-acceleration-program.com/app/projects/70113/themes"><strong>プロジェクト詳細 »</strong></a>
<p align="right">(<a href="#top">トップへ</a>)</p>

## ディレクトリ構成
まだまだちいちゃい  

.  
├── i2c  
│   └── kx132.io  
├── main.io  
└── README.md  

<p align="right">(<a href="#top">トップへ</a>)</p>

## 開発環境構築


<!-- 開発環境構築に必要な情報を記載 -->
### 1. git のダウンロード  
<a href="https://qiita.com/T-H9703EnAc/items/4fbe6593d42f9a844b1c"><strong>このへん »</strong></a>を参考にgitをダウンロードする

```
git -version
```
このあたりが出てきたら成功してるかな
  
   
### 2. gitのユーザー登録  
VScodeのterminal上で行う。  
適当なフォルダに移動して下のようにユーザー登録を行う。

```
git config --global user.name <ユーザーネーム>
git config --global user.email <メールアドレス>
```

### 3. リポジトリをクローンする
リモートリポジトリをローカル環境にクローン

```
git clone https://github.com/yudai-rowing/arduino_ssap
```
自分のローカル環境に落ちてきたら成功。

<p align="right">(<a href="#top">トップへ</a>)</p>

## 開発
### ソースコード開発の流れ
いずれきちんと書く(ほんまかいな)。ざっくりとは下の通り。
```
リモートでブランチを切る → ローカルにgit pull → ローカルにcommit  
→ リモートへpush → リモートでプルリクエスト → リモートにmerge → ローカルにpull
```

### でも別に
みんなでソース書く必要ないか。書かない人は適当に落としてきてArduino IDEに打ち込んで試す。

<p align="right">(<a href="#top">トップへ</a>)</p>

## その他
SSHの設定とかわかる人書いてほしい

<p align="right">(<a href="#top">トップへ</a>)</p>
