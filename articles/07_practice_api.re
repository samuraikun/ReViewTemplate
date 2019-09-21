= API

== GraphQL APIの追加

バックエンド部分となる APIを追加します。Amplifyを使うと、数コマンドだけで、DynamoDBやElasticSearchなどの各サービスに連携したAPIをすぐに作ることができます。

//cmd{
amplify add api
//}

最初は、REST APIにするかGraphQLどちらかを選択します。GraphQLを選択します。

//cmd{
? Please select from one of the below mentioned services (Use arrow keys)
> GraphQL
  REST
//}

API名を決めます。デフォルト名にするのでそのままEnterで構いません。

//cmd{
? Provide API name: (appsyncamplifysample)
//}

APIの認証方式を選択します。Amazon Cognito User Poolを選択します。

//cmd{
? Choose the default authorization type for the API
  API key
> Amazon Cognito User Pool
  IAM
  OpenID Connect
//}

追加の設定は特に必要ないので、そのままEnter

//cmd{
? Do you want to configure advanced settings for the GraphQL API
> No, I am done.
  Yes, I want to make some additional changes.
//}

既存のGraphQLのスキーマファイルが、あればそれを使用するかどうか聞かれます。今はないので、そのままEnterかあるいは「N」とタイプして下さい。

//cmd{
? Do you have an annotated GraphQL schema? (y/N)
//}

Amplify側で、GraphQLのスキーマファイルを自動生成するか聞かれます。これは、イエスです。

//cmd{
? Do you want a guided schema creation? (Y/n)
//}

どういう構成のスキーマにするか聞かれます。後で、スキーマは変更するので、ここではどれを選んでも構いません。

//cmd{
? What best describes your project: (Use arrow keys)
> Single object with fields(e.g., “Todo” with ID, name, description)
  One-to-many relationship(e.g., “Blogs” with “Posts” and “Comments”)
  Objects with fine-grained access control
  (e.g., a project management app with owner-based authorizati
on)
//}

今すぐスキーマを変更するかどうか聞かれます。ここは、イエスです。

//cmd{
Do you want to edit the schema now? (Y/n)
//}

イエスを選択すると、起動しているエディターにスキーマ定義ファイルが開きます。
自動生成されたスキーマを次のように、今回のアプリケーションの仕様に合ったスキーマに変更します。

==== スキーマを定義する

各スキーマの意味は、以下のようになります。

 * Storehouse = 在庫を管理する倉庫
 * Product = 在庫となる商品そのもの
 * S3Object = S3に格納する画像などのコンテンツ
 * User = ユーザー

//listnum[set-schema][ドメインのスキーマを定義]{
type Storehouse @model @searchable @auth(rules: [{ allow: owner }]) {
  id: ID!
  name: String!
  products: [Product]
    @connection(name: "StorehouseProducts", sortField: "createdAt")
  tags: [String]
  owner: String!
  createdAt: String
}

type Product @model @auth(rules: [{ allow: owner }]) {
  id: ID!
  description: String!
  storehouse: Storehouse @connection(name: "StorehouseProducts")
  file: S3Object!
  price: Float!
  shipped: Boolean!
  owner: String
  createdAt: String
}

type S3Object {
  bucket: String!
  region: String!
  key: String!
}

type User
  @model(
    queries: { get: "getUser" }
    mutations: { create: "registerUser", update: "updateUser" }
    subscriptions: null
  ) {
  id: ID!
  username: String!
  email: String!
  registered: Boolean
}
//}

==== @modelについて

DBに保存しておきたいリソースについては、@modelを定義する
@modelだけ定義すると、自動で、各リソースに対するCRUD処理を自動生成する、取得はqueries、更新は、mutations
@modelは、queries, mutations, subscriptionsを任意で自前で定義することも可能

==== @connectionについて

各スキーマのリレーションを定義するための型。
AppSyncでは、@connectionという型を使って、リレーションを表現することができます。

1対多のリレーションは、@connection(name: "リレーション先のドメイン名を示す名前")と記述し、1対1のリレーションは、単に@connectionだけとなります。

==== @authについて

あるリソースについてアクセス権をもたせたい場合は、@authという型を指定します。
例えば、Productは、作った本人のみがアクセスできるようにするという場合に型定義します。

==== @searchable

ElasticSearchと連携して検索可能にするための型も用意されています。
それが、@searchableで、基本的にこの型を定義するだけで、AmplifyがよしなにElasticSearchとAppSyncとの連携を行ってくれます。

ただ、ElasticSearchを使うことになるので、料金は月2000円以上はかかってしまうのが、少しつらいところです。

スキーマ定義が完了したら、amplify pushして、リソース構成を更新します。

//image[amplify_push_api1][amplify push]{
//}

APIを追加する場合は、いくつか質問に答えます。

各スキーマに対応したQueries,Mutations,Subscriptionsのファイルを作成されるか聞かれるので、これを許可します。
言語も選択できるので、javascriptを選択します。

//cmd{
? Do you want to generate code for your newly created GraphQL API Yes
? Choose the code generation language target
> javascript
  typescript

? Enter the file name pattern of graphql queries,
mutations and subscriptions (src/graphql/**/*.js)

? Do you want to generate/update all possible GraphQL operations
- queries, mutations and subscriptions(Y/n)
//}

スキーマのリレーション先のネスト数を指定します。デフォルトは、2ですが3にしておきます。

//cmd{
Enter maximum statement depth [increase from default if your schema is deeply nested] (2)
//}

Enterを押すと、CloudFormationによるリソース構成が始まり、しばらく時間がかかります。
最後にこういったログが出れば、完了です。

//cmd{
...中略

Generated GraphQL operations successfully and saved at src/graphql
All resources are updated in the cloud

GraphQL endpoint: https://xxx.appsync-api.ap-northeast-1/amazonaws.com/graphql
//}

これで、APIの作成は以上となります。
スキーマを定義して、Amplify CLIのコマンドを実行するだけで、各モデルのCRUD処理のAPIを作成することができました。

とても楽にAPIができましたね！素晴らしい！！
