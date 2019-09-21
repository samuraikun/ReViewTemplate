= AppSyncとは

本章では、GraphQLを使用したサービスであるAppSyncについて学んでいきます。

通常、GraphQLを使用する際は、クライアントとバックエンドともにGraphQLを使えるようにするための実装が必要となります。
必要なことは以下になります。

 * スキーマ定義
 * Queries、Mutations、Subscriptionsをそれぞれ記述
 * GraphQLとデータベースのつなぎこみをするリゾルバーの用意

AppSyncを使わず、GraphQLを使う場合は、これら３つを自前で実装するかあるいは GraphQL実装の有名ライブラリであるApollo@<fn>{apollo}を使うかになります。
//footnote[apollo][https://www.apollographql.com/]

AppSyncを使うと、スキーマ定義は自前で用意しますが、他の２つはAppSync側が自動で用意してくれます。
Queries、Mutations、Subscriptionsは、AppSyncが、用意したスキーマ定義を元に自動生成してくれます。

また、データベースとのつなぎこみの役割を担うリゾルバーもAppSyncが自動生成します。これこそが、AppSyncを使う最大のメリットでもあります。
それは、AWSの各サービスとの連携が容易であることです。具体的には、スキーマ定義するだけで、DynamoDBやAurora Serverless、ElasticSearch、S3、Cognitoなどの連携が可能です。

== AppSyncのアーキテクチャ

AppSyncは、クライアントと各AWSサービスとの間の連携を担うサービスで、図にすると次のようなイメージになります。

//image[aws_appsync][AppSyncの仕組み]{
//}

== スキーマ

AppSyncは、GraphQLを使用するため、型指定によるスキーマを定義して、リソース構成を決めていきます。
定義したスキーマは、AppSyncの管理画面からも確認することが可能です。

//image[appsync_console_schema][AppSync管理画面からのスキーマ確認]{
//}

== クエリー

定義したスキーマを元にAppSyncで、GraphQL環境を構築すると、GraphQLのクエリーによって、各種データを取得することができます。
AppSyncの管理画面から、クエリーを実行することが可能で、クエリー毎に必要な引数も確認することができます。

//image[appsync_console_query][Appsync管理画面からクエリー実行]{
//}

== リゾルバー

AppSyncと各種AWSサービスとの連携を実際に行っている部分です。
リゾルバー自体は、JAVAベースのVTLというテンプレート言語で構成され、リゾルバーに変更を加える際は、その言語を使う必要があります。

この書籍では、このリゾルバーを直接変更することはありませんが、細かな独自制御などを行いたい場合は、リゾルバーを編集する必要があります。

//list[resolver_dynamodb][VTLで記述されたリゾルバーの例：DynamoDBへのUpdate]{
{
    "version" : "2017-02-28",
    "operation" : "UpdateItem",
    "key": {
        "foo" : ... typed value,
        "bar" : ... typed value
    },
    "update" : {
        "expression" : "someExpression"
        "expressionNames" : {
           "#foo" : "foo"
       },
       "expressionValues" : {
           ":bar" : ... typed value
       }
    },
    "condition" : {
        ...
    }
}
//}

AppSyncには、他にも便利な機能がありますが、最低限使うのに必要なのは、一旦ここまでで大丈夫でしょう。
より詳細を知りたい場合は、公式ドキュメント@<fn>{aws_appsync_official_doc}をご参照下さい。
//footnote[aws_appsync_official_doc][https://docs.aws.amazon.com/ja_jp/appsync/latest/devguide/welcome.html]
