= 認証機能の開発

この章では、アプリケーション開発でまず必要となる認証について取り扱います。AWSでは、Cognitoという認証のためのサービスがあるため、
AmplifyとCognitoを組み合わせた認証機能の開発について学んでいきましょう。

前章で、すでにAmplifyによって、APIと認証の基盤は作成済みなので、フロントエンド側の認証画面と認証時の振る舞いについて実装していきます。

先に実装を見たい場合は、この章のブランチ@<fn>{auth-component-branch}をご参考下さい。
//footnote[auth-component-branch][https://github.com/samuraikun/appsync-amplify-sample-app-for-techbookfest7/tree/auth-component]

== Amplify と Reactとの連携

Amplifyで、認証基盤は作成したので、ここではフロントエンドの実装を行います。
実装内容は、ログイン画面と認証済みの場合は、ホーム画面を表示するという変更を加えます。

まずは、認証済みかどうかで、URLを切り替えるため、react-router-domを追加します。

また、Element-UIのReactライブラリを使用するので、react-hot-loader(v4.2.13)をインストールします。
これは、element-reactが、React v16.9.0ではそのまま動作しないため、その対応です。

//cmd{
yarn add react-router-dom

yarn add -D react-hot-loader
//}

Amplifyの初期設定を行った時点で、aws-exports.jsというファイルが、ルートディレクトリに作成されます。
aws-exports.jsにAmplifyの各種情報があるので、AmplifyのSDKを使って、ReactとAmplifyとの連携を行います。

//source[src/index.js]{
import React from 'react';
import ReactDOM from 'react-dom';
import Amplify from 'aws-amplify';
import aws_exports from './aws-exports';
import App from './App';
import * as serviceWorker from './serviceWorker';
import 'element-theme-default';

Amplify.configure(aws_exports);

ReactDOM.render(<App />, document.getElementById('root'));

serviceWorker.unregister();
//}

実装内容の順番としては、以下のような順番で行い、最終的に実装全体を見ていくという流れになります。

 1. ログイン情報の有無で、画面の表示を切り替える
 2. ルーティングの設定
 3. サインイン・サインアウト等の認証イベントをトリガーにするサブスクリプションの実装
 4. ナビゲーションバーのコンポーネント実装
 5. サインアウト

== ログイン画面のコンポーネント作成

Amplifyには、サインアップ画面を自前で実装せずとも、サインアップのためのコンポーネントを提供しています。

それでは、src/App.jsに変更を加えます。まずはインポート部分です。

==== インポート定義部分

//source[/src/App.js]{
import React, { useState, useEffect } from 'react';
import { Auth, Hub, Logger } from 'aws-amplify'
import { Authenticator, AmplifyTheme } from 'aws-amplify-react';
import { BrowserRouter as Router, Route } from 'react-router-dom';
import './App.css';
import HomePage from './pages/HomePage';
import ProfilePage from './pages/ProfilePage';
import StorehousePage from './pages/StorehousePage';
import Nabvar from './components/Navbar';
//}

==== ログイン済みのユーザー情報を取得する実装

React hooksによるユーザー情報取得のフックイベントを追加します。
12行目の@<b>{Auth.currentAuthenticatedUser()}で、Cognitoで登録したユーザー情報が取得できます。

//listnum[src/App-1.js][ログイン済みユーザー情報を取得する]{
// ...import文

function App() {
  const [user, setUser] = useState(null)

  useEffect(() => {
    const getUserData = async () => {
      const user = await Auth.currentAuthenticatedUser()
      user ? setUser({ user }) : setUser(null)
    }

    getUserData()
  }, [])

  return (
    <div>App</div>
  )
}
//}

==== ログイン済みかどうかで表示を切り替え

//listnum[src/App-2.js][ログイン済みかどうかの表示切り替え]{
// ...import文

function App() {
  // ...省略

  return !user ? (
    <Authenticator />
  ) : <div>App</div>
}

export default App;
//}

変更後に画面が、サインイン画面になっていれば、OKです。

//image[sign_in_ui][サインイン画面]{
//}

新規登録もしておきましょう。電話番号は、フォーマットが正しければ何でも構いません。ここでは「15555555555」と入力します。
作成ボタンを押した後に、登録したメールアドレス宛に、ログインに必要な確認コードが含まれたメールが送信されるので、使い捨て用のメールアドレスを使うのがおすすめです。
筆者は、Chrome拡張の@<b>{Temp Mail}@<fn>{temp_mail}をおすすめします。
//footnote[temp_mail][https://chrome.google.com/webstore/detail/temp-mail-disposable-temp/inojafojbhdpnehkhhfjalgjjobnhomj]

//image[signup][サインアップ画面]{
//}

//image[temporary_login][確認コードの入力]{
//}

ログインが完了したら、次のような画面が表示されるとOKです。

//image[logined][ログイン後の画面]{
//}

== 認証イベントを購読するSubscriptionの実装

認証機能では、サインアップやログイン、ログアウトなどいくつかのイベントが存在します。
各イベントが、発生したタイミングで何かしらの処理を実行させたいというケースはよくあるでしょう。

Amplify SDKでは、このようなユースケースに使える便利な機能を提供しています。

それが、@<b>{Hub}@<fn>{amplify_hub}という機能です。
//footnote[amplify_hub][https://aws-amplify.github.io/docs/js/hub]

これは、Amplify SDKの処理に関するイベントをトリガーにコンポーネント間と通信し、任意の処理を実行できる Pub/Sub型の機能です。

具体例として、ログイン後のユーザー情報の取得やログアウト後のコンポーネントの状態変化などを行ってみましょう。

//listnum[app.js_hub][Hubを活用した認証イベントのPub/Sub処理]{
// ...import文
import { Auth, Hub, Logger } from 'aws-amplify';
// ...省略

const logger = new Logger('My-Logger');

function App() {
  const [user, setUser] = useState(null);

  const getUserData = async () => {...}

  getUserData();

  Hub.listen('auth', data => {
    switch(data.payload.event) {
      case 'signIn':
        getUserData()
        break;
      case 'signUp':
        break;
      case 'signOut':
        setUser(null);
        break;
      case 'signIn_failure':
        break;
      case 'configured':
        break;
      default:
        return
    }
  });
}
//}

== ルーティングの追加

react-router-domによるURL切り替えの画面遷移を実装します。
追加するルーティングは、2つです。

 * /
 * /storehouses/:storehouseId

//listnum[add-router][ルーティングの追加]{
// ...import文

function App() {
  ...省略

  return !user ? <Authenticator /> : (
    <Router>
      <>
        {/* Routes */}
        <div className='app-container'>
          <Route exact path='/' component={HomePage} />
          <Route path='/storehouses/:storehouseId'
          // 書面の都合上、改行していますが、１行です。
          component={({ match }) =>
          <StorehousePage storehouseId={match.params.storehouseId}
          />}
        />
        </div>
      </>
    </Router>
  )
}

export default App
//}

ついでに、StorehousePageコンポーネントにpropsを追加します。

//source[src/pages/StorehousePage.js]{
import React from 'react'


function MarketPage(props) {
  return (
    <div>MarketPage {props.storehouseId}</div>
  )
}

export default MarketPage;
//}

== ヘッダーコンポーネントの実装

ユーザーがログインしたかどうかで、表示を変えるので、ユーザー情報をprops経由で渡します。

//source[src/App.js]{
function App() {
  // ...省略

  return !user ? <Authenticator /> : (
    <Router>
      <>
        {/* Navbar */}
        <Nabvar user={user} />
        {/* Routes */}
        <div className='app-container'>
          <Route exact path='/' component={HomePage} />
          <Route
            path='/storehouses/:storehouseId'
            component={({ match }) => <StorehousePage storehouseId={match.params.storehouseId} />}
          />
        </div>
      </>
    </Router>
  )
}
//}

ヘッダー用コンポーネント本体の実装はこちらになります。

特に状態を持たないピュアなコンポーネントになります。
ヘッダーにpropsで受け取ったユーザー名を表示します。

//source[src/components/NavBar.js]{
import React from "react";
import { Menu as Nav, Icon, Button } from 'element-react';
import { NavLink } from 'react-router-dom';

const Navbar = ({ user, handleSignout }) => (
  <Nav mode='horizontal' theme='dark' defaultActive='1'>
    <div className='nav-container'>
      <Nav.Item index='1'>
        <NavLink to='/' className='nav-link'>
          <span className='app-title'>
            <img
              src='https://icon.now.sh/home'
              alt='App Icon'
              className='app-icon'
            />
            Storehouse App
          </span>
        </NavLink>
      </Nav.Item>

      <div className='nav-items'>
        <Nav.Item index='2'>
          <span
            className='app-user'
          >
            Hello, {user.username}
          </span>
        </Nav.Item>
        <Nav.Item index='3'>
          <NavLink to='/profile' className='nav-link'>
            <Icon name='setting' />
            Profile
          </NavLink>
        </Nav.Item>
        <Nav.Item index='4'>
          <Button type='warning'>
            Sign Out
          </Button>
        </Nav.Item>
      </div>
    </div>
  </Nav>
)

export default Navbar;
//}

画面の方を更新すると、次のようにヘッダーが表示された画面になります。

//image[navbar][ヘッダーが表示された画面]{
//}

== サインアウト機能

ヘッダーにサインアウトボタンがあるので、クリックしたときにサインアウトが実行されるようにします。
サインアウトのイベントを親コンポーネントに送信するための実装を追加します。

//source[src/componentsNavbar.js]{
<Nav.Item index='4'>
  <Button
    type='warning'
    onClick={handleSignout}
  >
    Sign Out
  </Button>
</Nav.Item>
//}

App.jsに、子コンポーネントであるヘッダーから受け取ったhandleSignoutの関数を追加します。

//source[src/App.js]{
// ...import文

function App() {
  ...省略

  const handleSignout = async () => {
    try {
      await Auth.signOut();
    } catch(err) {
      console.error('Error signing out user', err);
    }
  }

  return !user ? <Authenticator /> : (
    <Router>
      <>
        {/* Navbar */}
        <Nabvar user={user} handleSignout={handleSignout} />
        {/* Routes */}
        <div className='app-container'>
          ...省略
        </div>
      </>
    </Router>
  )
}
//}

これで基本的な認証機能は、以上です。サインアウトのボタンをクリックすると、ログイン画面に戻れば完了です。
