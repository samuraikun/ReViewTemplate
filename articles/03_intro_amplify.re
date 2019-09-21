= Amplifyとは

Amplifyは、コマンドベースで、AWSの各サービスを組み合わせた構成を簡単に構築できます。
Amplifyは、専用のCLIツールを提供しているのでそれを使用して、環境構築するのがベターです。

== Amplify CLIの概要

AmplifyのCLIツールであるamplify-cli@<fn>{amplify_cli}は、基本操作のコマンドと各サービスを対象にするコマンドが存在します。
//footnote[amplify_cli][https://github.com/aws-amplify/amplify-cli]

基本的なコマンドの組み合わせとしては、以下のようになります。

//cmd{
amplify 基本操作コマンド サービス名

// 例
amplify init // Amplifyによる初期設定
amplify add api // AmplifyによるAPIの追加
//}

amplify-cliのコマンドによって構築したAWSのサービスは、CloudFormation@<fn>{cloudformation}で、定義されます。
そのため、Terraform@<fn>{terraform}のような@<kw>{IaC, Infrastructure as Code}@<fn>{lac}を用途にしたツールに似ています。
//footnote[cloudformation][https://aws.amazon.com/jp/cloudformation/]
//footnote[terraform][https://www.terraform.io/]
//footnote[lac][https://ja.wikipedia.org/wiki/Infrastructure_as_Code]

== Amplify/AppSyncによるTODOアプリ開発

では、実際にAmplifyとAppSync・DynomoDBを組み合わせて、シンプルなReact製のTODOアプリを作ってみましょう。
任意のディレクトリに移動して、以下のコマンドを実行して下さい。

=== Reactのセットアップ

//cmd{
mkdir sample-todo
cd sample-todo

npm i -g @aws-amplify/cli

npx create-react-app .
//}

=== Amplifyのセットアップ

次に、amplify-cliによるAmplifyのセットアップを行います。作業中のターミナルに続けて、以下のコマンドを実行して、ターミナル上でいくつかの質問に答えます。

//cmd{
amplify init

? Enter a name for the project sample-todo-amplify
? Enter a name for the environment dev
? Choose your default editor: Visual Studio Code
? Choose the type of app that you're building javascript
Please tell us about your project
? What javascript framework are you using react
? Source Directory Path:  src
? Distribution Directory Path: build
? Build Command:  yarn build
? Start Command: yarn start
Using default provider  awscloudformation

? Do you want to use an AWS profile? Yes
? Please choose the profile you want to use 任意のIAMユーザー
//}

ここまで進むと、実際にAmplifyによる環境構築が始まります。

=== GraphQL APIの追加

次に、TODOモデルのためのGraphQL APIを追加します。次のコマンドを実行します。

//cmd{
amplify add api
//}

まずはじめに、RESTかGraphQLかどちらかのタイプのAPIにするか問われるので、GraphQLを選択します。

//cmd{
? Please select from one of the below mentioned services
> GraphQL
  REST
//}

次に、APIの名前を任意で決め、APIのアクセス方法について、問われます。
これは、「Amazon Cognito User Pool」を選択して下さい。

//cmd{
? Provide API name: sampletodographqlapi
? Choose the default authorization type for the API
  API key
> Amazon Cognito User Pool
  IAM
  OpenID Connect
//}

Cognitoの設定について問われますが、そのままデフォルトの設定で、構いません。

//cmd{
Do you want to use the default authentication and security configuration?
> Default configuration
  Default configuration with Social Provider (Federation)
  Manual configuration
  I want to learn more.
//}

サインインの方法については、「Username」にします。

//cmd{
How do you want users to be able to sign in?
> Username
  Email
  Phone Number
  Email and Phone Number
  I want to learn more.
//}

追加設定について問われますが、特に必要ないので、「No, Iam done 」で次に進みます。

//cmd{
Do you want to configure advanced settings?
> No, I am done.
  Yes, I want to make some additional changes.

? Do you want to configure advanced settings for the GraphQL API
> No, I am done.
  Yes, I want to make some additional changes.
//}

すでにGraphQLのスキーマファイルが、存在するか聞かれますが、今はないので、Noでそのまま次に進めます。

//cmd{
? Do you have an annotated GraphQL schema? (y/N)
//}

サンプル用のスキーマファイルを自動生成するか聞かれますが、こちらは、Noにして下さい。
続けてくる質問は、「Todo」と入力して下さい。

//cmd{
? Do you want a guided schema creation? (Y/n) n
? Provide a custom type name Todo
//}

これで、API追加の設定は完了です。
「schema.graphql」というファイルが作成されており、以下のようなスキーマが定義されています。

//source[amplify/backend/api/sampletodographqlapi/schema.graphql]{
type Todo @model {
	id: ID!
	title: String!
	content: String!
	price: Int
	rating: Float
}
//}

このファイルを次のように変更します。

//source[amplify/backend/api/sampletodographqlapi/schema.graphql]{
type Todo @model @auth(rules: [{ allow: owner }]) {
  id: ID!
  note: String!
}
//}

最後に、AWS側に今回のセットアップ内容をデプロイします。
コマンドとしては、以下のようなコマンドになります。

デプロイ前に今回の変更内容が出力されます。内容に問題がなければそのままデプロイを開始します。

//cmd{
$ amplify push

Current Environment: dev

| Category | Resource name        | Operation | Provider plugin   |
| -------- | -------------------- | --------- | ----------------- |
| Auth     | sampletodoamplify    | Create    | awscloudformation |
| Api      | sampletodographqlapi | Create    | awscloudformation |
? Are you sure you want to continue? (Y/n)
//}

新しいAPIの追加の場合は、GraphQLのクエリーとミューテーションの実装ファイルを作成するか問われるので、Yesを入力してファイルを作成します。

//cmd{
? Do you want to generate code for your newly created GraphQL API (Y/n)
//}

言語とファイル作成先のパスについて問われます。JavaScriptを選択して、パスについてもそのままEnterで構いません。

//cmd{
? Choose the code generation language target (Use arrow keys)
> javascript
  typescript
  flow
? Enter the file name pattern of graphql
queries, mutations and subscriptions (src/graphql/**/*.js)
//}

残りの質問も全てEnterで進めましょう。

//cmd{
? Do you want to generate/update all possible
GraphQL operations - queries, mutations and subscriptions Yes
? Enter maximum statement depth [increase from default
if your schema is deeply nested] 2
//}

ここまで進むと、以下のようにCloudFormationによる構築が始まります。

//cmd{
UPDATE_IN_PROGRESS sample-todo-amplify-dev-20190921014351
AWS::CloudFormation::Stack

...

Generated GraphQL operations successfully and saved at src/graphql
All resources are updated in the cloud

GraphQL endpoint: https://xxxx/graphql
//}

== React側の実装

Amplifyは、JavaScriptのSDKライブラリとReact用のコンポーネントライブラリを提供しているので、まずそれらを追加します。

//cmd{
yarn add aws-amplify aws-amplify-react
//}

次に、create-react-appによって作成されたいくつかのファイルを次のように変更します。


=== ReactとAmplifyの連携

Reactコンポーネントのレンダリング前に、アプリケーションとAmplifyとの連携を行います。

 * ① Amplify SDK本体をインポート
 * ② AWSの各種設定値が定義された設定ファイルをインポート
 * ③ Amplifyに設定を適用させる

//source[src/index.js]{
import React from 'react';
import ReactDOM from 'react-dom';
import './index.css';
import App from './App';
import * as serviceWorker from './serviceWorker';
import Amplify from 'aws-amplify'; // ①
import awsconfig from './aws-exports'; // ②
Amplify.configure(awsconfig); // ③

ReactDOM.render(<App />, document.getElementById('root'));
serviceWorker.unregister();
//}

=== TODOを追加する

GraphQLを使って、TODOを追加できるUIを実装していきましょう。
Reactによる状態管理は、基本的にReact Hooks@<fn>{react_hooks}を使っています。
//footnote[react_hooks][https://ja.reactjs.org/docs/hooks-intro.html]

この実装で注目すべき点は、以下の３点です。

 * ① GraphQLの操作を行うためのAmplifyライブリをインポート
 * ② Amplify用のReactコンポーネントを使って、@<kw>{HOC, High Order Component}@<fn>{hoc_todo}によってサインイン画面を表示
 * ③ GraphQLミューテーションによるTODOの追加

//footnote[hoc_todo][https://ja.reactjs.org/docs/higher-order-components.html]

//listnum[src/todo-App.js][TODOを追加できるUIの実装]{
import React, { useState } from 'react';
import { API, graphqlOperation } from 'aws-amplify'; // ①
import { withAuthenticator } from 'aws-amplify-react';
import { createTodo } from './graphql/mutations';

function App() {
  const [note, setNote] = useState('');
  const [notes, setNotes] = useState([]);

  const handleOnChangeNote = event => {
    setNote(event.target.value)
  }

  const handleAddNote = async event => {
    event.preventDefault();
    const input = { note };
    const result = 
      await API
        .graphql(graphqlOperation(createTodo, { input })) // ③
    console.log(result)
    const newNote = result.data.createTodo;
    const updatedNotes = [newNote, ...notes];
    setNotes(updatedNotes);
    setNote('');
  }

  return (
    <div className={`
      flex
      flex-column
      items-center
      justify-center
      pa3
      bg-washed-red
    `}>
      <h1 className="code f2-l">
        Amplify Notetaker
      </h1>
      {/* Note Form */}
      <form onSubmit={handleAddNote} className="mb3">
        <input
          type="text"
          className="pa2 f4"
          placeholder="Write your note"
          onChange={handleOnChangeNote}
          value={note}
        />
        <button
          className="pa2 f4" type="submit"
         >
           Add Todo
        </button>
      </form>

      {/* Note List */}
      <div>
        {notes.map(item => (
          <div key={item.id} className="flex items-center">
            <li className="list pa1 f3">
              {item.note}
            </li>
            <button className="bg-transparent bn f4">
              <span>&times;</span>
            </button>
          </div>
        ))}
      </div>
    </div>
  );
}

export default withAuthenticator(App, { // ②
  includeGreetings: true
});
//}

ここまで、できればローカルでアプリケーションを起動して、動作確認します。

//cmd{
yarn start
//}

ブラウザが起動して、localhost:3000にアクセスすると、最初にログイン画面が表示されるので、サインアップ画面に切り替えて、アカウントを新規作成しましょう。

//image[sign_up_todo_app][サインアップ画面]{
//}

入力したメールアドレスに対して、メールが送信されます。
メール内に含まれる確認コードを入力すると、ログイン画面に遷移するので、先程設定したユーザー名とパスワードを入力するとログインが完了します。

//image[verify_code_todo][確認コード入力画面]{
//}

ログイン完了後に、TODOが入力できるフォームが表示されます。試しにフォームに何か入力して、ボタンを押すと追加したタスクが以下のように表示されます。

//image[add_new_todo][ホーム画面]{
//}

=== TODOの一覧を表示する

追加したTODOを表示するための実装を追加します。

　* ① 追加する実装としては、API通信という副作用のある処理を行うので、React HooksのuseEffectという関数を使用します
　* ② GraphQL経由でのデータ取得は、listTodosというGraphQL Queryを使用します

//listnum[fetch_todos][TODOの一覧を取得して表示する]{
import React, { useState, useEffect } from 'react';
import { API, graphqlOperation } from 'aws-amplify';
import { withAuthenticator } from 'aws-amplify-react';
import { createTodo } from './graphql/mutations';

function App() {
  const [note, setNote] = useState('');
  const [notes, setNotes] = useState([]);

  useEffect(() => { // ①
    async function fetchData() {
      const result = await API
        .graphql(graphqlOperation(listNotes)); // ②
      setNotes(result.data.listNotes.items);
    }
    fetchData();
  }, []);

  // ...省略
}

export default withAuthenticator(App, {
  includeGreetings: true
});
//}

これで、TODO一覧が取得され表示されます。

//image[fetch_todos][TODOの一覧表示]{
//}

=== TODOの更新と削除

続いて、TODOの更新と削除をGraphQL Mutationsによって行う実装を追加します。

 * ① 更新と削除のミューテーションをインポートします
 * ② 更新と削除に必要なパラメータとしてIDを保持するStoreを用意する
 * ③ 既に存在するTODOの場合は、CreateからUpdateに切り替え
 * ④ TODOの更新
 * ⑤ TODOの削除

//source[src/App.js]{
import React, { useState, useEffect } from 'react';
import { API, graphqlOperation } from 'aws-amplify';
import { withAuthenticator } from 'aws-amplify-react';
import {
  createTodo,
  updateTodo, // ①
  deleteTodo // ①
} from './graphql/mutations';
import { listTodos } from './graphql/queries';

function App() {
  const [id, setId] = useState('');
  const [note, setNote] = useState('');
  const [todos, setTodos] = useState([]);

  useEffect(() => {
    async function fetchData() {
      const result = await API
        .graphql(graphqlOperation(listTodos));
      setTodos(result.data.listTodos.items);
    }
    fetchData();
  }, []);

  const handleOnChangeNote = event => {
    setNote(event.target.value)
  }

  const handleAddTodo = async event => {
    event.preventDefault();

    if (hasExistingTodo()) { // ③
      handleUpdateTodo()
    } else {
      const input = { note };
      const result = await API
        .graphql(graphqlOperation(createTodo, { input }))
      const newTodo = result.data.createTodo;
      const updatedTodos = [newTodo, ...todos];
      setTodos(updatedTodos);
      setNote('');
    }
  }

  // ④
  const handleUpdateTodo = async () => {
    const input = { id, note };
    const result = await API
      .graphql(graphqlOperation(updateTodo, { input }));
    const updatedTodo = result.data.updateTodo;
    const index = todos
      .findIndex(todo => todo.id === updatedTodo.id);
    const updatedTodos = [
      ...todos.slice(0, index),
      updatedTodo,
      ...todos.slice(index + 1)
    ];
    setTodos(updatedTodos)
  }

  // ⑤
  const handleDeleteTodo = async todoId => {
    const input = { id: todoId };
    const result = await API
      .graphql(graphqlOperation(deleteTodo, { input }));
    const deletedTodoId = result.data.deleteTodo.id;
    const updatedTodos = todos
      .filter(todo => todo.id !== deletedTodoId);
    setTodos(updatedTodos);
  }

  const handleSetNote = ({ id, note }) => {
    setId(id);
    setNote(note);
  }

  const hasExistingTodo = () => {
    if (id) {
      const isTodo = todos.findIndex(todo => todo.id === id) > -1;
      return isTodo;
    }

    return false;
  }

  return (
    <div className={`
      flex
      flex-column
      items-center
      justify-center
      pa3
      bg-washed-red`
    }>
      <h1 className="code f2-l">
        Amplify Todos
      </h1>
      {/* Todo Form */}
      <form onSubmit={handleAddTodo} className="mb3">
        <input
          type="text"
          className="pa2 f4"
          placeholder="Write your todo"
          onChange={handleOnChangeNote}
          value={note}
        />
        <button className="pa2 f4" type="submit">
          { id ? "Update Todo" : "Add Todo" }
        </button>
      </form>

      {/* Todo List */}
      <div>
        {todos.map(item => (
          <div key={item.id} className="flex items-center">
            <li
              onClick={() => handleSetNote(item)}
              className="list pa1 f3"
            >
              {item.note}
            </li>
            <button
              onClick={() => handleDeleteTodo(item.id)}
              className="bg-transparent bn f4"
            >
              <span>&times;</span>
            </button>
          </div>
        ))}
      </div>
    </div>
  );
}

export default withAuthenticator(App, {
  includeGreetings: true
});
//}

これで、TODOの作成・更新・削除ができるアプリケーションが作れました。

このように、Amplify・AppSyncを使うと、API通信部分は、GraphQLでやり取りを行い、データベース(DynomoDB)へ保存するための連携も、AppSync側で行ってくれるため、基本的にアプリケーション開発者は、
フロントエンドのみの開発に集中することができます。

それでは、次のページからの実践編では、もう少し本格的なアプリケーションを題材に、この章では、扱わなかったGraphQL SubscriptionやReact Hooksをさらに活用したアプリケーション開発について学んでいきましょう。
