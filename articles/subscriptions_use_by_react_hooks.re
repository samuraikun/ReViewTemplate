= React Hooks useContext・useReducerによるサブスクリプション機能

7章で、ProductモデルのCRUD処理が可能となりました。
この章では、Productモデルの各CRUD処理をトリガーにReact Hooksを使って、任意の処理を行う方法について見ていきます。

具体的にはProductモデルが、新規作成、更新、削除されたタイミングで、Productモデルの一覧表示されている内容を最新化します。

