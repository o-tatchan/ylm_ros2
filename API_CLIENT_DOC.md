# YLM APIクライアント仕様

# YLM API Driver (ROS2 ノード)

本ノードは、後述するYLM API Clientを使用しています。
現在の機能は
-   ノード起動時に/start_scan APIを送る
-   ノード終了時に/stop_scan APIを送る
のみですが、YLM API Clientの機能を使うことで容易にカスタマイズできます。

# YLM API Client (ライブラリ)

本機能は、YLMのREST API仕様に基づいて作成されており、各APIの呼び出しを容易に実行できるように設計されています。
本パッケージ(ylm_ros2)上で`api_client` ライブラリとして定義されています。以下の機能により、YLMの状態、パラメータ、起動/停止などの動作を、REST API経由でリアルタイムに取得・実行・処理することができます。

## クラス構造

`LumotiveAPIClient` クラスは、以下の構成で設計されています。

### `private`
*   **`sensor_ip_`**: YLMの IP アドレスの設定値です。

### `public` 部分

#### 1. YLMの各API呼び出し関数
以下の 関数を定義しています。全て、YLM設定用通信仕様書に従っています。戻り値は、レスポンスのコンテンツ部分を文字列(std::string)で返します。

APIを追加する場合は、api_client.cppとapi_client.hに以下と同様の関数を追加すれば良いです。

*   `std::string get_system_version()`
*   `std::string get_sensor_id()`
*   `std::string get_state()`
*   `std::string post_start_scan()`
*   `std::string post_stop_scan()`
*   `std::string get_scan_parameters_options()`
*   `std::string get_scan_parameters()`
*   `std::string post_scan_parameters(const std::string& parameters)`
*   `std::string get_persistent_settings()`
*   `std::string post_persistent_settings(const std::string& parameters)`
*   `std::string post_restart()`
*   `std::string post_disable()`
*   `std::string get_logs()`***コンテンツタイプ要検証***
*   `std::string get_messages()`
*   `std::string get_time_sync_status()`



#### 2. クライアントライブラリラッパー (`client_library_wrapper`)
*   C++のREST APIクライアントライブラリはいくつもあるため、変更が容易にできるように用意しました。
*   使用するライブラリを変更する際は、この関数を以下の仕様通りに変更するだけで済みます。（現状はlibcurlを使用）
    *   引数
        *   `method`: lumotive_api_client::GET or lumotive_api_client::POST
        *   `endpoint`: APIエンドポイント　例） "/start_scan"
        *   `send_data`: POSTする文字列データ. method = POSTのときに有効
        *   `result`: レスポンスのコンテンツ部分を文字列で入れます
    *   戻り値
        *   正常に通信できたら`true`を返します
        *   何らかの以上がある場合`false`を返します


#### 4. レスポンス処理 (`curl_callback`)
*   libcurlを使う場合に、レスポンスを文字列で取得するために必要なコールバック関数です。