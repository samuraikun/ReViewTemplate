= はじめに

本書をお読みいただきありがとうございます。「Amplify & AppSync GraphQLによるサーバーレス開発入門」では、AWSのサービスである Amplify と AppSync を中心にフルサーバーレスな構成の
Webアプリケーションを0から実装する方法を学べる内容となっております。

ただその前になぜ筆者が、Amplify と AppSync に興味を持ったのかお伝えします。
筆者は、興味・得意とする技術がフロントエンド分野の技術です。
そのためインフラやサーバーサイドの開発リソースをなるべく最小化して、UI・UXの開発・改善に注力したいと常日頃考えています。

一方で、アプリケーションそのもののパフォーマンスやスケーラビリティを犠牲にしたくないのもまた事実です。
昨今では、そのような問題に対して Google が提供するmBaasと呼ばれるような Firebase が人気です。

Firebase は、認証やデータベース、ストレージひいてはホスティングなど現代のWebアプリケーションにおいてほぼ必須な各機能群を
サービスとして提供し、それを組み合わせることで容易にWebアプリケーションを開発することができます。
筆者もその便利さに惹かれ、実際にFirebaseを用いてアプリケーションを開発し、@<kw>{DX, Developer Experience}のよさを体感しました。

ただ、筆者は普段業務では、FirebaseやGCPよりもAWSを触ることのほうが多いため、AWSのサービスを使って、FirebaseのようなDXを得られるものはないかと
探し求めていました。

そこで、注目したのが、AmplifyとAppSyncです。

Firebaseでは、クライアントとデータベースであるFirestoreが直接連携するのに対して、AppSyncは、GraphQLでクライアントとデータベースの間を受け持つサービスです。
また、リソースに対するアクセス制御は、Cognito@<fn>{cognito}と組み合わせることができるため、柔軟な認証設定を実現させることができます。
//footnote[cognito][https://aws.amazon.com/jp/cognito/]

次にAmplifyですが、AppSyncとCognitoの連携をCLIベースで簡単に設定することできるフレームワークです。
他にも、AppSyncは、

 * データベースとして DynamoDB@<fn>{dynamodb}と連携
 * ストレージとして S3@<fn>{s3}との連携
 * ホスティングとして CloudFront@<fn>{cloudfront}との連携
 * 検索機能として ElasticSearch@<fn>{elasticsearch}との連携

など様々なAWSのサービスを組みわせることが容易なため、Firebaseと同じ様な開発体験を得ることができます。その上で、GraphQLというスキーマベースなインターフェースを
介して、クライアントとバックエンドのやり取りが可能なため、可読性とメンテナンス性も優れています。

それでは、GraphQL、AppSync、Amplifyについてもう少し掘り下げてみましょう。

//footnote[dynamodb][https://aws.amazon.com/jp/dynamodb/]
//footnote[s3][https://aws.amazon.com/jp/s3/]
//footnote[cloudfront][https://aws.amazon.com/jp/cloudfront/]
//footnote[elasticsearch][https://aws.amazon.com/jp/elasticsearch-service/]
