= セットアップ

この章では、シンプルな在庫管理システムを開発していきます。

まず最初に、フロントエンド部分のReactの環境構築とバックエンド側の環境構築をAmplifyで行います。

== フロントエンドのセットアップ

Reactのバージョンは、v16.9.0を前提とします。
Node.jsのバージョンは、v10.16.0になります。

Reactの環境構築については、事前に用意した以下URLのブランチを使用します。

@<href>{https://github.com/samuraikun/appsync-amplify-sample-app-for-techbookfest7/tree/initial-setup}

ダウンロードの完了後、ターミナルに以下のコマンドが実行して、アプリケーションが立ち上がることを確認して下さい。

//cmd{
yarn install
yarn start
//}

「App」という文字列が表示される画面が立ち上がれば成功です。

//image[initial_setup][初期表示の画面]{
//}

== Amplify による各AWSリソースのセットアップ

AmplifyのCLIツールをインストールします。

//cmd{
npm i -g @aws-amplify/cli
//}

==== IAMユーザーの作成

Amplify用のIAMユーザーを作成します。

//cmd{
amplify configure
//}

このコマンドを実行して、しばらくするとブラウザが自動で起動し、AWSマネジメントコンソールのログイン画面が現れるのでサインインして下さい。
再度、コンソールへ戻ると、どのリージョンを使用するか選択できるので、@<kw>{東京リージョン, ap-northeast-1}を選択します。

//image[select_regions][リージョンの指定]{
//}

IAMユーザーの名前を決めます。ここでは、techbookfest7という名前で作成します。

再度、ブラウザ側のAWSのコンソール画面に遷移して、新規IAMユーザーの設定を行います。コンソール画面では、特に設定は変更せず、次へのボタンを押すだけで構いません。
最後の画面で、作成したIAMユーザーのアクセスキーとシークレットキーが発行されるので、こちらは外部にもれないよう自身で保管します。

コンソールに戻り、入手したアクセスキーとシークレットキーを入力します。

//image[input_iam_key][各キーの入力]{
//}

最後に、任意のプロフィール名を決めれば完了です。

//image[iam_profile_name][プロフィール名]{
//}

==== Amplifyによる初期環境構築

いよいよAmplifyによるAWSリソースの構築を行います。まずは以下のコマンドを実行しましょう。

//cmd{
amplify init
//}

いくつか質問に答える必要があります。
最初にプロジェクト名など、基本的な設定を行います。

プロジェクト名は、「appsyncamplifysample」、環境名は、「production」にします。
//cmd{
? Enter a name for the project appsyncamplifysample
? Enter a name for the environment production
//}

次に、使用するエディターも聞かれるので、こちらはお好みでよいですが、ここではVS Codeを選択します。

//cmd{
? Choose your default editor: Visual Studio Code

//}

使用するフロントエンドフレームワークは、Reactです。

//cmd{
? Choose the type of app that you're building javascript
Please tell us about your project
? What javascript framework are you using
  angular
  ember
  ionic
❯ react
  react-native
  vue
  none
//}

その他、ビルドコマンドなどを指定します。

//cmd{
? Source Directory Path:  src
? Distribution Directory Path: build
? Build Command:  yarn build
? Start Command: yarn start
//}

次に、CloudFormationによる各AWSリソースの構成をAmplifyで構築します。
Amplifyで使用するIAMユーザーは、先程作成した techbookfest7 を選択します。

//image[amplify_init2][Amplifyで使用するIAMユーザー]{
//}

IAMユーザーを選択し、Enterを押すとCloudFormationによる構成が開始されます。

//image[amplify_init3][CloudFormationのログ]{
//}

これで、Amplifyの初期設定は以上となります。
