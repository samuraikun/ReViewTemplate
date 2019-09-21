= GraphQLとは

== TL;DR

 * REST APIのようなWeb APIのモデルの１つ
 * OSSのGraphQL API用の クエリ言語
 * 型定義を用いてAPIのリソースを定義できる
 * RESTが、CRUD形式のRPC@<fn>{rpc}としたら、GraphQLは SQLのようなQuery Languages(問い合わせ言語)
 * １つのエンドポイントだけで、クライアントとサーバー間のデータ取得、変更、購読をすることができる

//footnote[rpc][http://e-words.jp/w/RPC.html]

GraphQLの設定と処理形態は、以下のようにそれぞれ名付けられます。

 * スキーマ定義
 * 読み込み = Queries
 * 変更 = Mutations
 * 購読 = Subscriptions

== GraphQLが注目される背景

GraphQLがなぜ注目されたかについては、まずREST APIの存在が欠かせません。

REST APIは、URLでリソースを表現し、データ操作をHTTPメソッドによって行うWeb標準に則ったシンプルな設計で、それが普及に至った大きな理由です。
しかし、URLでリソースを表現するため、アプリケーションが大規模になるのに比例して、URLのエンドポイントの数も増大します。

アプリケーションの規模が、大きくなるほどに増大するAPIのエンドポイントの把握とメンテナンスが難しくなります。
具体的には、各APIで必要なリクエストパラメーターやレスポンス内容を把握するのが辛くなります。
そのため、SwaggerなどのAPI仕様をまとめたドキュメントを作成するという方法があります。

しかし、APIドキュメントを作成した場合、ドキュメントの内容とAPIの本実装に乖離がないようメンテナンスする必要があり、その管理にコストがかかります。
またクライアント側の開発目線でもいくつか問題があります。

例えば、様々なデータを表示・操作する必要があるページの場合、いくつものAPIリクエストが必要になり、実装が複雑になりやすいです。
また、クライアント層というのは、バックエンド層やデータ層と比較して、変更が多いため、REST APIだと、場合によってはAPIそのものの変更が必要になり、フロントエンドとバックエンドの分業が難しいです。
もちろんこれらの問題に対しては、そもそものAPIのリソース設計がよくないからというケースもあると思いますが、変更に強いREST APIの設計というのは、得てして難しいものです。

理由としては、RESTでは、URLでリソースを表現するということが、シンプルである一方、URL単位でしかリソースを表現できないという制約にもなるからです。
この制約があるために、変更に強いAPI設計をしようとすると、どうしても抽象度が高いURLになりがちで、そのURLを見ただけでは何がしたいAPIなのかわからないというケースが多いです。

他にも、UIによっては、レスポンス内容に不要なデータも含まれたり、リアクティブなUIを表現するために、イベントドリブンな設計にしたとしても、WebSocketを使わない限り、結局は、サーバーとの接続にHTTPのリクエスト・レスポンスが必要になります。

== GraphQLのメリット

RESTで起こる問題として、前節では以下のように述べました。

 1. アプリケーションの規模増大にして、比例して、URL単位のリソース管理もコストが増える
 2. APIドキュメントによる解決方法は、ドキュメントの管理コストという別の問題を招く
 3. UIによっては、レスポンス内容に不要なデータが含まれるケースがよくあるため、通信オーバーヘッドを招きやすい
 4. イベントドリブンと相性が悪い

GraphQLによって、問題がなぜ解決できるのか１番から順に見ていきましょう。

==== 実装とドキュメントが一緒

まず１番目と２番目の問題についてですが、GraphQLは、エンドポイントが単一で、リソースは型指定によるスキーマ定義を行うため、設定用のスキーマファイルそのものがドキュメントであり、実装のため、
スキーマファイルさえ、メンテナンスすれば１番目と２番目の問題は解決します。

==== クライアント側で、リクエスト内容を柔軟に決めることができる

3番目の問題についてはGraphQL Queriesによって、クエリによるリクエストを発行するので、必要なデータだけを含んだレスポンスを容易に取得することができます。

==== リアルタイム処理が容易

4番目は、GraphQL Subscriptionsが、内部でWebSocketを使用しているので、スキーマとして定義するだけでリアルタイム処理が実現できます。

== GraphQLのデメリット

次に、GraphQLのそれぞれの処理形態の特徴と例について見ていきましょう。

==== キャッシュ戦略が難しい

GraphQLは、単一のエンドポイントしか持たないため、従来のURLを利用したHTTPキャッシュが使えないです。

==== 監視・分析が難しい

こちらも単一エンドポイントだからこその理由ですが、URL単位で、パフォーマンスやユーザーのイベントログを収集・解析するようなツールによる分析が困難です。
具体的には、パフォーマンス監視で言えば、NewRelicなどのAPM(Application Performance Management)が、イベントログ収集・解析で言えば、Google Analytics が使えない。

==== 画像や動画のアップロード戦略が悩ましい

GraphQLのデメリットというよりは、GraphQLで使用するJSONのデメリットですが、HTTPのmultipart機能が使えないため、
画像や動画のようなバイナリデータを送信する際に、JSONのまま送信するには、Base64でエンコードする必要がありますが、その場合データ容量が増え、結果的に通信のオーバーロードを発生させてしまいます。

== スキーマ定義について

拡張子.graphqlのファイルを作成し、スキーマを定義していきます。
スカラー型やオブジェクト型、列挙型などが定義可能です。
また、Not NULL制約は、感嘆符の「！」で表現します。

REST APIではリソースをURLで表現し複数のエンドポイントを持ちますが、GraphQLでは、/graphql という１つのエンドポイントだけを持ち、リソースは型として表現します。
型として表現したリソースを用いて、Queries, Mutations, Subscription も同じように型定義します。

//listnum[set-schema][スキーマ定義]{
type Todo {
  id: ID!
  name: String
  description: String
  status: TodoStatus
}

enum TodoStatus {
  done
  pending
}

type Query {
  getTodos: [Todo]
}

type Mutation {
  addTodo(id: ID! description: String status: TodoStatus): Todo!
  deleteTodo(id: ID!): Todo!
}

type Subscription {
  addTodo: Todo
  @aws_subscribe(mutations: ["addTodo"]) ← AppSyncが提供する機能。GraphQL単体に含まれる機能ではない。
  deleteTodo: Todo
  @aws_subscribe(mutations: ["deleteTodo"])
}
//}

== Queriesについて

データの取得を行います。
レスポンス内容もクライアント側が決めることができます。

//listnum[queries][Queriesの例]{
query {
  getTodos {
    id
    name
  }
}

// レスポンス内容
{
  "data": {
    "getTodos": {
      "items": [
        {
          "id": "1",
          "name": "AppSyncを学ぶ"
        },
        {
          "id": "2",
          "name": "Amplifyを学ぶ"
        }
      ]
    }
  }
}
//}

== Mutationsについて

データの更新・削除を行います。

//listnum[mutations][Mutationsの例]{
mutation {
  addTodo(
    name
    description
    status
  ){
    id
  }
}

// レスポンス内容
{
  "data": {
    "addTodo": {
      "id": "hogehogefugafuga"
    }
  }
}
//}

== Subscriptionsについて

Mutationsの処理をトリガーに任意の処理を実行するPub/Sub型の機能
Pub/Sub機能を実現するための通信規格として、Websocket@<fn>{websocket}を使用します。
//footnote[websocket][https://ja.wikipedia.org/wiki/WebSocket]

//listnum[subscription][Subscriptionの例]{
subscription addTodoSub {
  addTodo {
    __typename
    name
    description
  }
}
//}
