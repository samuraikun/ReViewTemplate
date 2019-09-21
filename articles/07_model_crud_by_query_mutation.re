= GraphQL/Queries/MutationsによるモデルのCRUD処理

この章では、スキーマとして定義した各モデルをGraphQLを通じて、CRUD処理を行う内容について学んでいきます。

AmplifyによるGraphQL APIを追加したので、CRUD処理にあたるQueriesとMutationsのファイルが、
それぞれ@<b>{src/graphql/queries.js}、@<b>{src/graphql/mutations.js}というファイル名で、存在しています。

Amplifyを使うと、GraphQLのクエリーやミューテーションは、基本的には、自前で定義する必要がないのが楽になります。
もちろん、画面によって必要となるデータは、違うのでベースとして、これらのファイルをインポートして、コンポーネントで使いつつ、
特別なケースでは、コンポーネント内で、クエリーやミューテーションを定義して、そちらを使うという方法でも構いません。

それでは、アプリケーションのドメインとして、倉庫にあたるStorehouseモデルのCRUD処理を実装していきましょう。

※ 先に実装を見たい場合は、この章のブランチ@<fn>{crud-storehouse-branch}をご参考下さい。
//footnote[crud-storehouse-branch][https://github.com/samuraikun/appsync-amplify-sample-app-for-techbookfest7/tree/model-crud-by-mutations]

== StorehouseモデルのCreate処理

まずは、Storehouseモデルの新規作成画面を用意します。
完成後の画面は、次のようなUIです。

//image[create_new_storehouse][新規作成画面]{
//}

この実装で注目する点は、23~25行目のGraphQL APIへのリクエスト部分です。

@<b>{aws-amplify}が提供する@<b>{API}と@<b>{graphQLOperation}が、GraphQLによるCRUD処理を行うためのクライアント用APIです。
次のようなイメージで、実装します。

//source[example aws-amplify]{
API.graphQLOperation(query名 or mutation名, パラメーター)
//}

//listnum[NewStorehouse.js][src/components/NewStorehouse.js]{
import React, { useState } from 'react';
import { API, graphqlOperation } from 'aws-amplify';
import { createStorehouse } from '../graphql/mutations';
import { 
  Form,
  Button,
  Dialog,
  Input,
  Notification
} from 'element-react';

function NewStorehouse() {
  const [
    addStorehouseDialog,
    setAddStorehouseDialog
  ] = useState(false);
  const [name, setName] = useState('');

  const handleAddStorehouse = async () => {
    try {
      setAddStorehouseDialog(false)
      const input = { name }
      const result = await API.graphql(
        graphqlOperation(createStorehouse, { input })
      );

      setName('');
      console.log(`Created market: id ${result.data.createStorehouse.id}`)
    } catch(err) {
      Notification.error({
        title: 'Error',
        message: `
          ${err.message || 'Error adding storehouse'}
        `
      });
    }
  }

  return (
    <>
      <div className='storehouse-header'>
        <h1 className='storehouse-title'>
          Create Your StorehousePlace
          <Button
            type='text'
            icon='edit'
            className='storehouse-title-button'
            onClick={() => setAddStorehouseDialog(true)}
          />
        </h1>
      </div>
      <Dialog
        title='Create New Storehouse'
        visible={addStorehouseDialog}
        onCancel={() => setAddStorehouseDialog(false)}
        size='large'
        customClass='dialog'
      >
        <Dialog.Body>
          <Form labelPosition='top'>
            <Form.Item label='Add Storehouse Name'>
              <Input
                placeholder='Storehouse Name'
                trim={true}
                onChange={name => setName(name)}
                value={name}
              />
            </Form.Item>
          </Form>
        </Dialog.Body>
        <Dialog.Footer>
          <Button onClick={() => setAddStorehouseDialog(false)}>
            Cancel
          </Button>
          <Button
            type='primary'
            disabled={!name}
            onClick={handleAddStorehouse}
          >
            Add
          </Button>
        </Dialog.Footer>
      </Dialog>
    </>
  )
}
//}

次にこのコンポーネントをホーム画面に表示するための変更を追加します。

//source[src/pages/Homepage.js]{
import React from 'react';
import NewMarket from '../components/NewStorehouse';

export default function HomePage() {
  return (
    <>
      <NewMarket />
    </>
  )
}
//}

これで、倉庫モデルの新規作成画面ができました。試しに、＋ボタンをクリックすると、ダイアログが表示されます。
しかし、この時点では、Storehouseモデルを作成するためのパラメーター内容に不足があります。

必要なパラメーターを確認したい場合は、AWSコンソール画面から、AppSyncのページに移動します。

次の画面を参考に、createStorehouseに必要な引数を確認します。

//image[appsync_console1][AppSyncのコンソール画面]{
//}

これをみると、createStorehouseの引数の型は、@<b>{CreateStorehouseInput}という型であることがわかります。

//image[appsync_console2][引数の確認]{
//}


必要な引数は、作成するStorehouseモデルの名前と誰が作ったかを表すユーザー名が必須となります。

== React Context APIとuseContextを使ったデータの再利用

Storehouseモデルの新規作成には、ユーザー情報が必要であることがわかりました。
では、新規作成画面であるNewStorehouseコンポーネント内でもユーザー情報を取得する実装を追加する必要があるでしょうか？

答えはもちろん否です！！

ユーザー名などのユーザー情報は、コンポーネントツリーの最上層であるApp.js内で、useEffectを利用して、Appコンポーネント内のStoreに保持しています。
そのため、props経由で、NewStorehouseコンポーネントにユーザー情報を渡すでもよいですが、ユーザー情報のような多くの画面で必要そうなデータは、props経由で渡すべきではありません。@<fn>{props_hell}
//footnote[props_hell][propsによるバケツリレー: https://qiita.com/trapple/items/e7d64f62c1286d76f328]

このようなケースでは、Reduxのような状態管理ライブラリを使って、状態管理の責務を別に切り出すのがよくあるパターンです。

しかし、React Context APIとReact Hooksが誕生した今、React開発において、Reduxを無理に使う必要はありません！

それでは、React Context APIとReact Hooksを使用したデータの再利用について実装していきましょう。
まずは、Appコンポーネント配下にあるコンポーネントでも、ユーザー情報を取得できるようにするための準備を行います。

Reduxをご存知であれば、Connectコンポーネントを使用して、@<kw>{HOC, High Order Component}で、Storeとコンポーネントの接続を行うイメージです。

//listnum[create_context][React Context APIの定義]{
// ...import分

export const UserContext = React.createContext();

function App() {
  const [user, setUser] = useState(null)

  // ...省略

  return (
    <UserContext.Provider value={user}>
      ...省略
    </UserContext.Provider>    
  )
}
//}

これで、Appコンポーネント配下のコンポーネントで、userというStateを参照できるようになりました。
次にSateを参照する先のコンポーネントの実装についてです。

こちらは、React Hooksの一機能であるuseContextを使用します。
useContextに渡すのは、Appコンポーネント内で、定義したUserContextそのものです。

そして、UserContext経由で取得したユーザーデータをxx行目のhandleAddStorehouse関数に引数で渡すように変更します。

//listnum[use_context][useContextによるユーザー情報の参照]{
// ...import文

function NewStorehouse() {
  // ...省略
  const user = useContext(UserContext);

  const handleAddStorehouse = async user => {
    try {
      setAddStorehouseDialog(false)
      const owner = user.username;
      const input = { name, owner }
      const result = await API.graphql(graphqlOperation(createStorehouse, { input }));

      console.info(`Created storehouse: id ${result.data.createStorehouse.id}`);
      setName('');
    } catch(err) {
      Notification.error({
        title: 'Error',
        message: `${err.message || 'Error adding storehouse'}`
      });
    }
  }

  return (
    <>
      <div className='storehouse-header'>
        <h1 className='storehouse-title'>
          倉庫の作成
          <Button
            type='text'
            icon='plus'
            className='storehouse-title-button'
            onClick={() => setAddStorehouseDialog(true)}
          />
        </h1>
      </div>
      <Dialog
        title='倉庫の作成'
        visible={addStorehouseDialog}
        onCancel={() => setAddStorehouseDialog(false)}
        size='large'
        customClass='dialog'
      >
        <Dialog.Body>
          <Form labelPosition='top'>
            <Form.Item label='Add Storehouse Name'>
              <Input
                placeholder='Storehouse Name'
                trim={true}
                onChange={name => setName(name)}
                value={name}
              />
            </Form.Item>
          </Form>
        </Dialog.Body>
        <Dialog.Footer>
          <Button onClick={() => setAddStorehouseDialog(false)}>
            Cancel
          </Button>
          <Button
            type='primary'
            disabled={!name}
            onClick={() => handleAddStorehouse(user)}
          >
            Add
          </Button>
        </Dialog.Footer>
      </Dialog>
    </>
  )
}
//}

これで、新規作成ができるようになったので、画面から作成してみましょう。

//image[create_dialog_storehouse][新規作成画面]{
//}

作成されると、Chrome Developer toolsのコンソール欄に作成されたStorehouseモデルのIDが表示されます。

//image[console_log_create_storehouse][作成完了後のログ]{
//}

また、DynamoDBの管理画面からでも作成したデータが確認できます。

//image[dynamo_db_console][新規作成したデータをDynamoDBの管理画面から確認]{
//}


== StorehouseモデルのRead処理：Subscriptionによる一覧表示

現在は、Storehouseモデルの追加ができるだけで、追加されたデータを画面から、見ることができないという悲しい状況です。
そこでこの節では、追加したStorehouseモデルを一覧表示するためのコンポーネントを実装します。

変更を加えるのは、@<b>{StorehouseList.js}です。

//source[src/components/StorehouseList.js]{
import React from "react";
// import { Loading, Card, Icon, Tag } from "element-react";

const StorehouseList = () => {
  return <div>StorehouseList</div>;
};

export default StorehouseList;
//}

追加したデータを一覧表示するために必要なことは、次の3点になります。

 1. Storehouseモデルの一覧を取得するGraphQL Queryを使用する
 2. 取得したデータを表示する
 3. 新規作成したStorehouseモデルをリアルタイムに一覧に加えて表示する

では、1番目の処理から実装していきましょう。まずは、import文です。

//source[src/components/StorehouseList.js]{
import React from'react';
import { graphqlOperation } from 'aws-amplify';
import { Connect } from 'aws-amplify-react';
import { listStorehouses } from '../graphql/queries';
import { Loading, Card, Icon, Tag } from "element-react";
import { Link } from 'react-router-dom';
import Error from './Error';
//}

GraphQLによるREAD処理を行うため、@<b>{aws-amplify}の@<b>{graphqlOperation}と取得クエリーの@<b>{listStorehouse}を使います。
また、Amplifyが提供するReactコンポーネントである@<b>{Connect}コンポーネントも使用します。

このコンポーネントは、GraphQLのクエリーとミューテーションを実行できるコンポーネントで、その実行結果を返り値として、Connectコンポーネント内で、使用することできます。

こちらは、文章より実装を見たほうがイメージがつくかもしれません。次のような実装イメージになります。

Connectコンポーネントのpropsとしてクエリー発行の関数を渡し、このコンポーネント内部で、クエリー結果を使用することができるというイメージです。
もちろん、クエリーだけでなく、ミューテーションやサブスクリプションもpropsとして、このコンポーネントに渡すことができます。

//source[example_use_connect.js]{
return (
  <Connect
    query={graphQLOperation(listSomething)}
  >
    {({ data, loading, errors }) => {
      return (
        <Something data={data} />
      )
    }}
  </Connect>
)
//}

それでは、StorehouseList.jsの実装の続きを行いましょう。

実装としては、クエリーの取得結果を使って、UIの作成と行っています。

//source[src/components/Storehouse.js]{
import React from'react';
import { graphqlOperation } from 'aws-amplify';
import { Connect } from 'aws-amplify-react';
import { listStorehouses } from '../graphql/queries';
import { Loading, Card, Icon, Tag } from "element-react";
import { Link } from 'react-router-dom';
import Error from './Error';

const StorehouseList = () => {
  return (
    <Connect
      query={graphqlOperation(listStorehouses)}
    >
      {({ data, loading, errors }) => {
        if (errors.length > 0) return <Error errors={errors} />
        if (loading || !data.listStorehouses) return <Loading fullscreen={true} />

        return (
          <>
            <h2 className='header'>
              <img
                src='https://icon.now.sh/store_mall_directory/527FFF'
                alt='Store Icon'
                className='large-icon'
              />
              倉庫一覧
            </h2>
            {data.listStorehouses.items.map(storehouse => (
              <div key={storehouse.id} className='my-2'>
                <Card
                  bodyStyle={{
                    padding: '0.7em',
                    display: 'flex',
                    alignItems: 'center',
                    justifyContent: 'space-between'
                  }}
                >
                  <div>
                    <span className='flex'>
                      <Link className='link' to={`/storehouses/${storehouse.id}`}>
                        {storehouse.name}
                      </Link>
                      <span style={{ color: 'var(--darkAmazonOrange)' }}>
                        {storehouse.products.length}
                      </span>
                      <img
                        src='https://icon.now.sh/store_mall_directory'
                        alt='Shopping Cart'
                      />
                    </span>
                    <div style={{ color: 'var(--lightSquidInk)' }}>
                      {storehouse.owner}
                    </div>
                    <div>
                      {storehouse.tags && storehouse.tags.map(tag => (
                        <Tag key={tag} type='danger' className='mx-1'>
                          {tag}
                        </Tag>
                      ))}
                    </div>
                  </div>
                </Card>
              </div>
            ))}
          </>
        )
      }}
    </Connect>
  )
};

export default StorehouseList;
//}

@<b>{src/pages/HomePage.js}にStorehouseコンポーネントを呼び出します。

//source[src/pages/HomePage.js]{
import React from 'react';
import NewStorehouse from '../components/NewStorehouse';
import StorehouseList from '../components/StorehouseList';

export default function HomePage() {
  return (
    <>
      <NewStorehouse />
      <StorehouseList />
    </>
  )
}
//}

ブラウザを再更新すると、作成した倉庫一覧が、以下のように表示されます。

//image[fetch_storehouse_list][倉庫一覧の表示]{
//}

倉庫一覧を表示することはできましたが、都度ブラウザを再更新するのは面倒です。
できればStorehouseモデルが新規作成されたタイミングで、リスト一覧に表示される方が良いでしょう。

その場合は、Storehouseモデルが新規作成されたタイミングで、リスト一覧に作成したデータが、リアルタイムに追加されるようにしたいです。
こういったユースケースでは、GraphQL Subscriptionがマッチしています。

==== サブスクリプションによるリアルタイム処理
GraphQL Subscription は、あるイベントをトリガーに任意の処理をリアルタイムに行うGraphQLの機能です。AmplifyのConnectコンポーネントは、
クエリーと同じようにサブスクリプションも、propsとして引数で渡せば、Connectコンポーネント配下にサブスクリプションの実行結果を利用できるようになります。

サブスクリプション機能を追加したStorehouseコンポーネントの最終的な実装は、次のようになります。

注目してほしい点は、以下の３点です。

 * 28行目：Connectコンポーネントにどのサブスクリプションを使うか登録する
 * 29行目：登録したサブスクリプションが、発生したときにどんな処理をさせたいかを関数としてpropsで渡す
 * 13行目：onNewStorehouse関数内で、クエリ結果に新規作成したStorehouseモデルを追加して、新たなクエリ結果として返す

//listnum[subscription_storehouse][src/components/StorehouseList.js]{
import React, { useContext } from'react';
import { graphqlOperation } from 'aws-amplify';
import { Connect } from 'aws-amplify-react';
import { listStorehouses } from '../graphql/queries';
import { onCreateStorehouse } from '../graphql/subscriptions';
import { Loading, Card, Tag } from "element-react";
import { Link } from 'react-router-dom';
import Error from './Error';
import { UserContext } from '../App';

const StorehouseList = () => {
  const user = useContext(UserContext);
  const onNewStorehouse = (prevQuery, newData) => {
    // shallow copy
    let updatedQuery = { ...prevQuery }
    const updatedStorehouseList = [
      newData.onCreateStorehouse,
      ...prevQuery.listStorehouses.items
    ];

    updatedQuery.listStorehouses.items = updatedStorehouseList;

    return updatedQuery;
  }
  return (
    <Connect
      query={graphqlOperation(listStorehouses)}
      subscription={graphqlOperation(onCreateStorehouse, { owner: user.username })}
      onSubscriptionMsg={onNewStorehouse}
    >
      {({ data, loading, errors }) => {
        if (errors.length > 0) return <Error errors={errors} />
        if (loading || !data.listStorehouses) return <Loading fullscreen={true} />

        return (
          <>
            <h2 className='header'>
              <img
                src='https://icon.now.sh/store_mall_directory/527FFF'
                alt='Store Icon'
                className='large-icon'
              />
              倉庫一覧
            </h2>
            {data.listStorehouses.items.map(storehouse => (
              <div key={storehouse.id} className='my-2'>
                <Card
                  bodyStyle={{
                    padding: '0.7em',
                    display: 'flex',
                    alignItems: 'center',
                    justifyContent: 'space-between'
                  }}
                >
                  <div>
                    <span className='flex'>
                      <Link className='link' to={`/storehouses/${storehouse.id}`}>
                        {storehouse.name}
                      </Link>
                      <span style={{ color: 'var(--darkAmazonOrange)' }}>
                        {storehouse.products.length}
                      </span>
                      <img
                        src='https://icon.now.sh/store_mall_directory'
                        alt='Shopping Cart'
                      />
                    </span>
                    <div style={{ color: 'var(--lightSquidInk)' }}>
                      {storehouse.owner}
                    </div>
                    <div>
                      {storehouse.tags && storehouse.tags.map(tag => (
                        <Tag key={tag} type='danger' className='mx-1'>
                          {tag}
                        </Tag>
                      ))}
                    </div>
                  </div>
                </Card>
              </div>
            ))}
          </>
        )
      }}
    </Connect>
  )
};

export default StorehouseList;
//}

実際に画面から、新しいStorehouseモデルを作成してみると、ブラウザを再更新しなくてもリストに新規作成したStorehouseモデルが、追加表示されます。
このように、GraphQLのサブスクリプション機能を利用すると、リアルタイム性の高いインタラクティブなUIを簡単に実現することができます。

== StorehouseモデルのRead処理：ElasticSearchによる検索

== 子モデル(Product)のCRUD処理

この節では、倉庫を表すStorehouseモデルに属するProduct(製品)モデルのCRUD処理について扱います。
例として、「家電製品」という倉庫に「iPhone11 pro」という製品があるというイメージです。

Productモデルは、どんな製品かを表すために画像を持つデータとして、スキーマ定義しています。

//source[amplify/backend/api/appsynamplifysample/schema.graphql]{
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
//}

画像データ自体は、S3に格納するので、新たにAmplifyで、S3のバケットを作成します。

==== AmplifyによるS3バケットの追加

次のようなコマンドで、バケットを追加します。

//cmd{
amplify add storage
//}

使用用途について聞かれますので、「Content」を選択して下さい。

//cmd{
? Please select from one of the below mentioned services
(Use arrow keys)
> Content (Images, audio, video, etc.)
  NoSQL Database
//}

プロジェクト名とバケット名について問われます。任意の名前で構いません。

//cmd{
? Please provide a friendly name for your resource
that will be used to label this category in the project:

? Please provide bucket name:
//}

バケットへのアクセスを認証済みユーザーのみかあるいはそれ以外も許可するか選びます。これは、「Auth users only」を選んで下さい。

//cmd{
? Who should have access:
> Auth users only
  Auth and guest users
//}

バケットに対して、読み込み・書き込みを許可するか聞かれるので、矢印キーとスペースを押して全て選択状態にして下さい。

//cmd{
? Who should have access: Auth users only
? What kind of access do you want for Authenticated users?
 ◉ create/update
 ◉ read
>◉ delete
//}

S3のイベントをフックにして動作するLambdaを用意するか聞かれます。今回は、Lambdaは使用しないため、こちらは、Noです。

//cmd{
? Do you want to add a Lambda Trigger for your S3 Bucket? (y/N)
//}

最後に、リソース構成を更新します。

//cmd{
amplify push
//}

=== Storehouseモデルの詳細画面

Storehouseモデルの詳細画面は、StorehousePage.jsになります。

//source[src/pages/StorehousePage.js]{
import React from 'react'


function StorehousePage(props) {
  return (
    <div>StorehousePage {props.storehouseId}</div>
  )
}

export default StorehousePage;
//}


詳細画面の完成イメージは、次のようになります。

まずはStorehouseの詳細画面に、Storehouseモデルの概要を表示されるところまでを実装します。

==== import文

//source[src/pages/StorehousePage.js]{
import React, { useState, useEffect } from 'react';
import { API, graphqlOperation } from 'aws-amplify';
import { getStorehouse } from '../graphql/queries';
import { Loading, Tabs, Icon } from 'element-react';
import { Link } from 'react-router-dom';
import { format } from 'date-fns';
import NewProduct from '../components/NewProduct';
import Product from '../components/Product';
//}

==== Storeの定義とuseEffectによるStorehouseモデルのデータ取得

//listnum[storehousepage][src/pages/StorehousePage.js]{
// ...import文

function StorehousePage(props) {
  const [storehouse, setStorehouse] = useState(null);
  const [isLoading, setIsLoading] = useState(true);
  const [isStorehouseOwner, setIsStorehouseOwner] = useState(false);

  useEffect(() => {
    async function handleGetStorehouse() {
      const input = { id: props.storehouseId };
      const result = await API.graphql(graphqlOperation(getStorehouse, input));
      const storehouseOwner = result.data.getStorehouse.owner
      console.log(result)

      setStorehouse(result.data.getStorehouse);
      setIsLoading(false);

      if (props.user) {
        setIsStorehouseOwner(props.user.username === storehouseOwner)
      }
    }

    handleGetStorehouse()
  }, [props]);
}
//}

==== render部分

//source[src/pages/StorehousePage.js]{
// ...import文

function StorehousePage() {
  // ...省略

  return isLoading ? (
    <Loading fullscreen={true} />
  ) : (
    <>
      {/* Back Button */}
      <Link className='link' to=''>
        倉庫一覧へ戻る
      </Link>

      {/* Storehouse MetaData */}
      <span className='items-center pt-2'>
        <h2 className='mb-mr'>{storehouse.name}</h2>- {storehouse.owner}
      </span>
      <div className='items-center pt-2'>
        <span style={{ color: 'var(--lightSquidInk)', paddingBottom: '1em' }}>
          <Icon name='date' className='icon' />
          {storehouse.createdAt}
        </span>
      </div>

      {/* New Product */}
      <Tabs type='border-card' value='1'>
        <Tabs.Pane
          label={
            <>
              <Icon name='plus' className='icon' />
              製品を追加
            </>
          }
          name='1'
        >
          <NewProduct storehouseId={storehouseId} />
        </Tabs.Pane>
        {/* Products List */}
        <Tabs.Pane
          label={
            <>
              <Icon name='menu' className='icon' />
              Products ({ storehouse.products.items.length })
            </>
          }
          name='2'
        >
          <div className='product-list'>
            {storehouse.products.items.map(product => (
              <Product key={product.id} product={product} />
            ))}
          </div>
        </Tabs.Pane>
      </Tabs>
    </>
  )
}
//}

ここまでで、一旦以下のような画面になります。
タブによって、製品の新規追加フォームとProductモデルの一覧が表示されるコンポーネントに切り替えることができる画面となります。

//image[storehouse_show_page_without_product][倉庫モデルの詳細画面]{
//}

=== Productモデルの新規追加

NewProduct.jsで、Productモデルの新規作成フォームを作ります。

まずは、import部分です。

//source[src/components/NewProduct.js]{
import React, { useState } from "react";
import { Storage, Auth, API, graphqlOperation } from 'aws-amplify';
import { PhotoPicker } from 'aws-amplify-react';
import { Form, Button, Input, Notification, Progress } from "element-react";
import aws_exports from '../aws-exports';
import { createProduct } from '../graphql/mutations';
//}

画像をS3バケットにアップロードするため、プレビュー画像を表示するための@<b>{PhotoPicker}コンポーネントをインポートしています。

次に、Storeの定義を追加します。

//source[NewProductのStore定義]{
// ...import文

function NewProduct({ storehouseId }) {
  // State
  const [description, setDescription] = useState('');
  const [price, setPrice] = useState(null);
  const [isUploading, setIsUploading] = useState(false);
  const [imagePreview, setImagePreview] = useState('');
  const [image, setImage] = useState('');
  const [percentUploaded, setPercentUploaded] = useState(0);
}
//}

次は、フォームの値を全て空にするクリア処理と新規作成用のサブミット処理を追加します。

サブミット処理のhandleAddProductでは、画像のアップロード自体は、AppSyncを経由せず、S3へダイレクトアップロードを行います。
ダイレクトアップロードが成功したら、返り値であるファイルのメタデータを最終的にGraphQL Mutationsのパラメータに含めて、リクエストを送信します。

//source[クリア処理とサブミットサブミット処理]{
// ...import文

function NewProduct({ storehouseId }) {
  // State
  const [description, setDescription] = useState('');
  const [price, setPrice] = useState(null);
  const [isUploading, setIsUploading] = useState(false);
  const [imagePreview, setImagePreview] = useState('');
  const [image, setImage] = useState('');
  const [percentUploaded, setPercentUploaded] = useState(0);

  const clearState = () => {
    setDescription('');
    setPrice('');
    setIsUploading(false);
    setImagePreview('');
    setImage('');
    setPercentUploaded(0);
  }

  const handleAddProduct = async () => {
    try {
      // ローディング中にする
      setIsUploading(true);

      const visibility = 'public';
      const { identityId } = await Auth.currentCredentials();

      // ファイルの保存先パスを指定
      const filename = `/${visibility}/${identityId}/${Date.now()}-${image.name}`;

      // アップロード処理
      const uploadedFile = await Storage.put(filename, image.file, {
        contentType: image.type,
        // アップロードの途中経過をパーセント表示するための、コールバック関数
        progressCallback: progress => {
          const percentUploaded = Math.round(progress.loaded / progress.total) * 100;
          setPercentUploaded(percentUploaded);
        }
      });
      const file = {
        key: uploadedFile.key,
        bucket: aws_exports.aws_user_files_s3_bucket,
        region: aws_exports.aws_user_files_s3_bucket_region
      }
      const input = {
        productStorehouseId: storehouseId,
        description,
        price,
        file
      }
      const result = await API.graphql(graphqlOperation(createProduct, { input }));
      console.log('Created product', result)

      Notification({
        title: '成功',
        message: '製品の登録が完了しました！',
        type: 'success'
      });
      clearState()
    } catch(err) {
      console.error('Error adding product', err)
    }
  }
}
//}

最後にrender処理です。相変わらずrender処理部分は長いので、ざっと眺めるだけで構いません。

//source[src/components/NewProduct.js]{
return (
    <div className='flex-center'>
      <h2 className='header'>製品の追加</h2>
      <div>
        <Form className='storehouse-header'>
          <Form.Item label='製品の説明'>
            <Input
              type='text'
              icon='information'
              placeholder='説明文を入力'
              value={description}
              onChange={description => setDescription(description)}
            />
          </Form.Item>
          <Form.Item label='製品価格'>
            <Input
              type='number'
              icon='plus'
              placeholder='価格(円)'
              value={price}
              onChange={price => setPrice(price)}
            />
          </Form.Item>
          {imagePreview && (
            <img
              className='image-preview'
              src={imagePreview}
              alt='Produt Preview'
            />
          )}
          {percentUploaded > 0 && (
            <Progress
              type='circle'
              className='progress'
              percentage={percentUploaded}
            />
          )}
          <PhotoPicker
            title='製品画像'
            preview='hidden'
            onLoad={url => setImagePreview(url)}
            onPick={file => setImage(file)}
            theme={{
              formContainer: {
                margin: 0,
                padding: '0.8em'
              },
              sectionBody: {
                margin: 0
              },
              formSection: {
                flex: 'flex',
                flexDirection: 'column',
                alignItems: 'center',
                justifyContent: 'center'
              },
              sectionHeader: {
                padding: '0.2em',
                color: 'var(--varkAmazonOrange)'
              },
              photoPickerButton: {
                display: 'none'
              }
            }}
          />
          <Form.Item>
            <Button
              type='primary'
              disabled={!image || !description || !price || isUploading}
              onClick={handleAddProduct}
              loading={isUploading}
            >
              {isUploading ? 'Uploading...' : 'Add Product'}
            </Button>
          </Form.Item>
        </Form>
      </div>
    </div>
  )
//}

全体像としては、次の用になります。

//listnum[new_product.js][src/components/NewProduct.js]{
import React, { useState } from "react";
import { Storage, Auth, API, graphqlOperation } from 'aws-amplify';
import { PhotoPicker } from 'aws-amplify-react';
import { Form, Button, Input, Notification, Progress } from "element-react";
import aws_exports from '../aws-exports';
import { createProduct } from '../graphql/mutations';

function NewProduct({ storehouseId }) {
  // State
  const [description, setDescription] = useState('');
  const [price, setPrice] = useState(null);
  const [isUploading, setIsUploading] = useState(false);
  const [imagePreview, setImagePreview] = useState('');
  const [image, setImage] = useState('');
  const [percentUploaded, setPercentUploaded] = useState(0);

  const clearState = () => {
    setDescription('');
    setPrice('');
    setIsUploading(false);
    setImagePreview('');
    setImage('');
    setPercentUploaded(0);
  }

  const handleAddProduct = async () => {
    try {
      setIsUploading(true);

      const visibility = 'public';
      const { identityId } = await Auth.currentCredentials();
      const filename = `/${visibility}/${identityId}/${Date.now()}-${image.name}`;
      const uploadedFile = await Storage.put(filename, image.file, {
        contentType: image.type,
        progressCallback: progress => {
          const percentUploaded = 
            Math.round(progress.loaded / progress.total) * 100;
          setPercentUploaded(percentUploaded);
        }
      });
      const file = {
        key: uploadedFile.key,
        bucket: aws_exports.aws_user_files_s3_bucket,
        region: aws_exports.aws_user_files_s3_bucket_region
      }
      const input = {
        productStorehouseId: storehouseId,
        description,
        price,
        file
      }
      const result = await API.graphql(graphqlOperation(createProduct, { input }));
      console.log('Created product', result)

      Notification({
        title: '成功',
        message: '製品の登録が完了しました！',
        type: 'success'
      });
      clearState()
    } catch(err) {
      console.error('Error adding product', err)
    }
  }

  return (
    <div className='flex-center'>
      <h2 className='header'>製品の追加</h2>
      <div>
        <Form className='storehouse-header'>
          <Form.Item label='製品の説明'>
            <Input
              type='text'
              icon='information'
              placeholder='説明文を入力'
              value={description}
              onChange={description => setDescription(description)}
            />
          </Form.Item>
          <Form.Item label='製品価格'>
            <Input
              type='number'
              icon='plus'
              placeholder='価格(円)'
              value={price}
              onChange={price => setPrice(price)}
            />
          </Form.Item>
          {imagePreview && (
            <img
              className='image-preview'
              src={imagePreview}
              alt='Produt Preview'
            />
          )}
          {percentUploaded > 0 && (
            <Progress
              type='circle'
              className='progress'
              percentage={percentUploaded}
            />
          )}
          <PhotoPicker
            title='製品画像'
            preview='hidden'
            onLoad={url => setImagePreview(url)}
            onPick={file => setImage(file)}
            theme={{
              formContainer: {
                margin: 0,
                padding: '0.8em'
              },
              sectionBody: {
                margin: 0
              },
              formSection: {
                flex: 'flex',
                flexDirection: 'column',
                alignItems: 'center',
                justifyContent: 'center'
              },
              sectionHeader: {
                padding: '0.2em',
                color: 'var(--varkAmazonOrange)'
              },
              photoPickerButton: {
                display: 'none'
              }
            }}
          />
          <Form.Item>
            <Button
              type='primary'
              disabled={!image || !description || !price || isUploading}
              onClick={handleAddProduct}
              loading={isUploading}
            >
              {isUploading ? 'Uploading...' : 'Add Product'}
            </Button>
          </Form.Item>
        </Form>
      </div>
    </div>
  )
}

export default NewProduct;
//}

これで、次のようなフォーム画面が表示されます。

//image[new_product_form][Productモデルの新規作成フォーム]{
//}

=== Productモデルの一覧取得・更新・削除

Productモデルの一覧表示と更新・削除をProduct.jsで実装します。

まずは、import文です。S3に格納されたProductモデルの画像を表示するため、S3Imageというコンポーネントを利用します。

//source[src/components/Product.js]{
import React, { useState } from 'react';
import { API, graphqlOperation } from 'aws-amplify';
import { S3Image } from 'aws-amplify-react';
import {
  Notification,
  Popover,
  Button,
  Dialog,
  Card,
  Form,
  Input
} from 'element-react';
import { updateProduct, deleteProduct } from '../graphql/mutations';
//}

次に、ProductコンポーネントのStore定義です。ダイアログ表示で、Productモデルの更新と削除を行うため、ダイアログの表示切り替えフラグを用意しています。

//source[src/components/Product.js]{
// ...import文

function Product({ product }) {
  const [updateProductDialog, setUpdateProductDialog] = useState(false);
  const [deleteProductDialog, setDeleteProductDialog] = useState(false);
  const [description, setDescription] = useState('');
  const [price, setPrice] = useState(null);
}
//}

更新と削除の関数を実装します。src/graphql/mutations内に定義しているupdateProductとdeleteProductを使用します。

//source[src/components/Product.js]{
// ...import文

function Product({ product }) {
  const [updateProductDialog, setUpdateProductDialog] = useState(false);
  const [deleteProductDialog, setDeleteProductDialog] = useState(false);
  const [description, setDescription] = useState('');
  const [price, setPrice] = useState(null);

  const handleEditProduct = () => {
    setUpdateProductDialog(true);
    setDescription(product.description);
    setPrice(product.price);
  }

  const handleUpdateProduct = async productId => {
    try {
      setUpdateProductDialog(false);
      const input = {
        id: productId,
        description,
        price
      }

      const result = await API.graphql(graphqlOperation(updateProduct, { input }));
      console.log('Update product', result)

      Notification({
        title: '成功',
        message: '製品の更新が完了しました！',
        type: 'success',
        duration: 2000
      });
    } catch(err) {
      console.error(`Failed to updated product with id: ${productId}`, err);
    }
  }

  const handleDeleteProduct = async productId => {
    try {
      setDeleteProductDialog(false);
      const input = { id: productId };
      await API.graphql(graphqlOperation(deleteProduct, { input }));

      Notification({
        title: '成功',
        message: '製品の削除が完了しました！',
        type: 'success',
        duration: 2000
      });
    } catch(err) {
      console.error(`Failed to deleted product with id: ${productId}`, err);
    }
  }
}
//}

最後にrender処理です。S3Imageコンポーネントに、画像ファイルの格納先であるパス情報を渡すと画像が表示できます。

//source[src/components/Product.js]{
return (
  <div className='card-container'>
    <Card bodyStyle={{ padding: 0, minWidth: '200px' }}>
      <S3Image
        imgKey={product.file.key}
        theme={{
          photoImg: { maxWidth: '100%', maxHeight: '100%' }
        }}
      />
      <div className='card-body'>
        <h3 className='m-0'>{product.description}</h3>
        <div className='text-right'>
          <span className='mx-1'>
            ¥{product.price}
          </span>
        </div>
      </div>
    </Card>
    {/* Update / Delete Product Buttons */}
    <div className='text-center'>
      <>
        <Button
          type='warning'
          icon='edit'
          className='m-1'
          onClick={() => handleEditProduct()}
        />
        <Popover
          placement='top'
          width='160'
          trigger='click'
          visible={deleteProductDialog}
          content={
            <>
              <p>Do you want to delete this?</p>
              <div className='text-right'>
                <Button
                  size='mini'
                  type='text'
                  className='m-1'
                  onClick={() => setDeleteProductDialog(false)}
                >
                  キャンセル
                </Button>
                <Button
                  type='primary'
                  size='mini'
                  className='m-1'
                  onClick={() => handleDeleteProduct(product.id)}
                >
                  削除
                </Button>
              </div>
            </>
          }
        >
          <Button
            type='danger'
            icon='delete'
            onClick={() => setDeleteProductDialog(true)}
          />
        </Popover>
      </>
    </div>

    {/* Update Product Dialog */}
    <Dialog
      title='Update Product'
      size='large'
      customClass='dialog'
      visible={updateProductDialog}
      onCancel={() => setUpdateProductDialog(false)}
    >
      <Dialog.Body>
        <Form labelPosition='top'>
          <Form.Item label='Update Description'>
            <Input
              type='text'
              icon='information'
              placeholder='Description'
              trim={true}
              value={description}
              onChange={description => setDescription(description)}
            />
          </Form.Item>
          <Form.Item label='Update Price'>
            <Input
              type='number'
              icon='plus'
              placeholder='Price ($USD)'
              value={price}
              onChange={price => setPrice(price)}
            />
          </Form.Item>
        </Form>
      </Dialog.Body>
      <Dialog.Footer>
        <Button
          onClick={() => handleUpdateProduct(product.id)}
        >
          Update
        </Button>
      </Dialog.Footer>
    </Dialog>
  </div>
);
//}

最終的な全体実装は、こちらになります。

//listnum[Product.js][Product.jsの全体実装]{
import React, { useState } from 'react';
import { API, graphqlOperation } from 'aws-amplify';
import { S3Image } from 'aws-amplify-react';
import {
  Notification,
  Popover,
  Button,
  Dialog,
  Card,
  Form,
  Input
} from 'element-react';
import { updateProduct, deleteProduct } from '../graphql/mutations';

function Product({ product }) {
  const [updateProductDialog, setUpdateProductDialog] = useState(false);
  const [deleteProductDialog, setDeleteProductDialog] = useState(false);
  const [description, setDescription] = useState('');
  const [price, setPrice] = useState(null);

  const handleEditProduct = () => {
    setUpdateProductDialog(true);
    setDescription(product.description);
    setPrice(product.price);
  }

  const handleUpdateProduct = async productId => {
    try {
      setUpdateProductDialog(false);
      const input = {
        id: productId,
        description,
        price
      }

      const result = await API.graphql(graphqlOperation(updateProduct, { input }));
      console.log('Update product', result)

      Notification({
        title: '成功',
        message: '製品の更新が完了しました！',
        type: 'success',
        duration: 2000
      });
    } catch(err) {
      console.error(`Failed to updated product with id: ${productId}`, err);
    }
  }

  const handleDeleteProduct = async productId => {
    try {
      setDeleteProductDialog(false);
      const input = { id: productId };
      await API.graphql(graphqlOperation(deleteProduct, { input }));

      Notification({
        title: '成功',
        message: '製品の削除が完了しました！',
        type: 'success',
        duration: 2000
      });
    } catch(err) {
      console.error(`Failed to deleted product with id: ${productId}`, err);
    }
  }

  return (
    <div className='card-container'>
      <Card bodyStyle={{ padding: 0, minWidth: '200px' }}>
        <S3Image
          imgKey={product.file.key}
          theme={{
            photoImg: { maxWidth: '100%', maxHeight: '100%' }
          }}
        />
        <div className='card-body'>
          <h3 className='m-0'>{product.description}</h3>
          <div className='text-right'>
            <span className='mx-1'>
              ¥{product.price}
            </span>
          </div>
        </div>
      </Card>
      {/* Update / Delete Product Buttons */}
      <div className='text-center'>
        <>
          <Button
            type='warning'
            icon='edit'
            className='m-1'
            onClick={() => handleEditProduct()}
          />
          <Popover
            placement='top'
            width='160'
            trigger='click'
            visible={deleteProductDialog}
            content={
              <>
                <p>Do you want to delete this?</p>
                <div className='text-right'>
                  <Button
                    size='mini'
                    type='text'
                    className='m-1'
                    onClick={() => setDeleteProductDialog(false)}
                  >
                    キャンセル
                  </Button>
                  <Button
                    type='primary'
                    size='mini'
                    className='m-1'
                    onClick={() => handleDeleteProduct(product.id)}
                  >
                    削除
                  </Button>
                </div>
              </>
            }
          >
            <Button
              type='danger'
              icon='delete'
              onClick={() => setDeleteProductDialog(true)}
            />
          </Popover>
        </>
      </div>

      {/* Update Product Dialog */}
      <Dialog
        title='Update Product'
        size='large'
        customClass='dialog'
        visible={updateProductDialog}
        onCancel={() => setUpdateProductDialog(false)}
      >
        <Dialog.Body>
          <Form labelPosition='top'>
            <Form.Item label='Update Description'>
              <Input
                type='text'
                icon='information'
                placeholder='Description'
                trim={true}
                value={description}
                onChange={description => setDescription(description)}
              />
            </Form.Item>
            <Form.Item label='Update Price'>
              <Input
                type='number'
                icon='plus'
                placeholder='Price ($USD)'
                value={price}
                onChange={price => setPrice(price)}
              />
            </Form.Item>
          </Form>
        </Dialog.Body>
        <Dialog.Footer>
          <Button
            onClick={() => handleUpdateProduct(product.id)}
          >
            Update
          </Button>
        </Dialog.Footer>
      </Dialog>
    </div>
  );
}

export default Product;
//}


これで、Productモデルを新規作成して、ブラウザを再更新するとProductモデルの一覧が表示され、更新と削除が可能となります。

//image[product_index][Productモデルの一覧表示]{
//}

//image[product_edit][Productモデルの更新]{
//}

//image[product_delete][Productモデルの削除]{
//}
