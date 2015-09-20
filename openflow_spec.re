= OpenFlow の仕組み

//lead{
OpenFlow の概念が分かったところでもう少し細かい仕様に進みましょう。ここでは実用重視で OpenFlow 仕様のポイントとよく使う用語を押さえます。
//}

//indepimage[torema][][width=10cm]

== OpenFlow の標準仕様

OpenFlow標準仕様が主に定義するのは次の 2 つです。

 1. コントローラとスイッチの間の通信プロトコル (OpenFlow チャンネル)
 2. 設定されたフローエントリに対するスイッチの振る舞い

//noindent
本章ではこの 2 つの中でも特によく使う機能のみを重点的に説明します。

執筆時の OpenFlow 仕様最新バージョンであるバージョン 1.3 をベースに説明します。なお、1.3 と OpenFlow 登場初期に普及したバージョンである 1.0 とは考え方やモデルに大きな違いはありません。そのため 1.3 を理解すれば、古いバージョンである 1.0 も理解しやすいでしょう。

== スイッチとコントローラ間のやりとり

OpenFlow スイッチとコントローラは OpenFlow 仕様で規定されたメッセージをやりとりしながら動作します。ここでは具体的にどのようなメッセージを送受信するか順を追って説明します。@<chap>{whats_openflow}で見たカスタマーサービスセンターでのやりとりを思い出しながら読んでください。

=== スイッチとコントローラ間の接続を確立する

最初にコントローラとスイッチは OpenFlow チャンネルの接続を確立します。OpenFlow には OpenFlow チャンネルは、@<img>{secure_channel} のようにスイッチからコントローラへと接続するのが基本です。しかし、仕様ではコントローラ側からスイッチへの接続しても良いことになっています。
//image[secure_channel][スイッチがコントローラに接続し OpenFlow チャンネルを確立][width=12cm]

OpenFlow チャンネルは普通の TCP 接続です。コントローラとスイッチの両方が対応している場合には、性能は落ちますがよりセキュアな TLS (Transport Layer Security) を使うこともできます。

=== バージョンのネゴシエーション

次に使用する OpenFlow プロトコルのバージョンを確認するステップ、いわゆるバージョンネゴシエーションが始まります。OpenFlow チャンネルを確立すると、スイッチとコントローラはお互いに自分のしゃべれるバージョン番号を乗せた Hello メッセージを出し合います (@<img>{version_negotiation})。

//image[version_negotiation][Hello メッセージを出し合うことで相手の OpenFlow プロトコルバージョンを確認][width=12cm]

相手と同じバージョンを話せるようであればネゴシエーションに成功で、本格的におしゃべりを始められるようになります。

=== スイッチのスペックの確認

#@warn{あとで確認する}

次にコントローラは接続したスイッチがどんなスイッチかを確認します。ネゴシエーション直後はまだバージョンしか確認できていないので、コントローラはスイッチに Features Request というメッセージを送って次の各情報をリクエストします。

 * スイッチのユニーク ID (Datapath ID)
 * 物理ポートの一覧情報
 * サポートする機能の一覧

//image[features_request_reply][Features Request メッセージでスイッチのスペックを確認][width=12cm]

スイッチは Features Reply メッセージでこの情報を返信します。

=== コントローラへの受信パケットの通知

#@warn{1.0 との仕様の違いを説明する}

スイッチは、受信したパケットおよび関連情報をコントローラへ通知することが出来ます。このとき用いるメッセージを Packet In メッセージと呼びます。

//image[packet_in][受信パケットとその情報が Packet In メッセージとしてコントローラに上がる][width=12cm]

=== パケットの転送

#@warn{PacketOut と Flow Mod の説明を分離}

Packet Out メッセージは、コントローラからスイッチに出力させるパケットを送るために用いるメッセージです。例えば Packet In によりコントローラへ送られたパケットを正しい宛先に流すために、Packet Out メッセージを使います。これをやらないと Packet In によりコントローラへ送られたパケットはコントローラに残ったままで、宛先には届かないためです(@<img>{flowmod_packetout})。

=== フローテーブルの更新

Flow Mod メッセージは、コントローラがパケットを転送するためのフローエントリをスイッチに送るためのメッセージです。Flow Mod メッセージを受け取ったスイッチは、メッセージ中のフローエントリを、自身のフローテーブルに書き込みます。

//image[flowmod_packetout][Flow Mod によってフローテーブルを更新][width=12cm]

=== フローエントリ削除の通知

フローエントリが消されるとき、消されたフローエントリの情報とそのフローエントリにしたがって処理されたパケットの統計情報がコントローラに通知されます。これを Flow Removed メッセージと呼びます。このメッセージはネットワークのトラフィック量の集計に使えます。

//image[flow_removed][フローエントリが寿命で削除されると、転送されたパケットの統計情報が Flow Removed としてコントローラに上がる][width=12cm]

== フローエントリの中身

@<chap>{whats_openflow}で見たようにフローエントリは次の 3 要素から成ります。

 * マッチングルール
 * 優先度
 * カウンタ (統計情報)
 * インストラクション
 * タイムアウト (寿命)
 * クッキー 

以下ではそれぞれの中身を少し細かく見ていきます。なお、これらを最初からすべて頭に入れる必要はありません。以降の章を読んでいてわからなくなったらレファレンスとして活用してください。

=== 優先度

フローエントリには、優先度 (0 〜 65535) が設定できます。受信パケットが、フローテーブル中に複数のフローエントリにマッチする場合、この優先度の値が高いフローエントリが優先されます。

=== 統計情報

OpenFlow 1.3 ではフローエントリごとに次の統計情報を取得できます。

 * 受信パケット数
 * 受信バイト数
 * フローエントリが作られてからの経過時間 (秒)
 * フローエントリが作られてからの経過時間 (ナノ秒)

=== フローエントリの寿命

フローエントリには「寿命」を設定できます。寿命の指定には次の 2 種類があります。

 * アイドルタイムアウト: 参照されない時間がこの寿命に逹すると、そのフローエントリを消す。パケットが到着し、フローエントリが参照された時点で 0 秒にリセットされる。
 * ハードタイムアウト: 参照の有無を問わず、フローエントリが書き込まれてからの時間がこの寿命に逹すると、そのフローエントリを消す。

どちらのタイムアウトも 0 にして打ち込むと、そのフローエントリは明示的に消さない限りフローテーブルに残ります。

=== クッキー

フローエントリには、クッキーを設定できます。クッキーに設定された値は、スイッチにおけるパケット処理には全く影響を与えません。そのため、コントローラは、フローエントリを管理する等の目的のために、クッキーのフィールドに自由に使えます。

== マッチングルール

マッチングルールとは、OpenFlow スイッチがパケットを受け取ったときにアクションを起こすかどうかを決める条件です。たとえば「パケットの宛先が http サーバーだったら」とか「パケットの宛先がブロードキャストアドレスだったら」などという条件に適合したパケットにだけ、スイッチがアクションを起こすというわけです。

OpenFlow 1.3 では、40 種類の条件が使えます。主な条件を @<table>{matching_rules} に示します。これらの条件はイーサネットや TCP/UDP でよく使われる値です。

====[column] 取間先生曰く: マッチングルールの別名

フローエントリの 3 要素のひとつ、マッチングルールには実は "OpenFlow 12 タプル"、"マッチフィールド" という別の呼び方もあって、よく混乱します。そこでこの本では" マッチングルール" で統一することにしました。パケットが来たときにルールに従ってマッチする、という役割をすなおに表現していて、いちばんわかりやすい名前だからです。

OpenFlow 1.3 での正式な呼び名は、"マッチフィールド" です。しかし、マッチフィールドでは、わかりづらいし、変に難しそうですよね。

====[/column]

//table[matching_rules][マッチングルールで指定できる主な条件]{
名前				説明
--------------------------------------------------------------
In Port			スイッチの論理ポート番号
In Phy Port		スイッチの物理ポート番号
Ether Src		送信元 MAC アドレス
Ether Dst		宛先 MAC アドレス
Ether Type		イーサネットの種別
VLAN ID			VLAN ID
VLAN Priority		VLAN PCP の値 (CoS)
IP DSCP			DiffServ コードポイント
IP ECN			IP ECN ビット
IP Src			送信元 IP アドレス
IP Dst			宛先 IP アドレス
IP Proto		IP のプロトコル種別
TCP Src Port		TCP の送信元ポート番号
TCP Dst Port		TCP の宛先ポート番号
UDP Src Port		UDP の送信元ポート番号
UDP Dst Port		UDP の宛先ポート番号
ICMPv4 Type		ICMP 種別
ICMPv4 Code		ICMP コード
IPv6 Src		送信元 IPv6 アドレス
IPv6 Dst		宛先 IPv6 アドレス
IPv6 Flowlabel		IPv6 フローラベル
ICMPv6 Type		ICMPv6 種別
ICMPv6 Code		ICMPv6 コード
MPLS Label		MPLS ラベル
MPLS TC			MPLS トラフィッククラス
PBB ISID		PBB ISID
//}

OpenFlow の世界では、このマッチングルールで指定できる条件を自由に組み合わせて通信を制御します。たとえば、

 * スイッチの物理ポート 1 番から届く、宛先が TCP 80 番 (= HTTP) のパケットを書き換える
 * MAC アドレスが 02:27:e4:fd:a3:5d で宛先の IP アドレスが 192.168.0.0/24 は遮断する

//noindent
などといった具合です。

====[column] 取間先生曰く: OSI ネットワークモデルが壊れる？

あるネットワークの経験豊富な若者がこんな事を言っていました。

「OpenFlow のようにレイヤをまたがって自由に何でもできるようになると、OSI ネットワークモデル(よく「レイヤ 2」とか「レイヤ 3」とか呼ばれるアレのこと。正確には ISO によって制定された、異機種間のデータ通信を実現するためのネットワーク構造の設計方針)が壊れるんじゃないか？」

その心配は無用です。OSI ネットワークモデルは正確に言うと「OSI 参照モデル」と言って、通信プロトコルを分類して見通しを良くするために定義した "参照用" の階層モデルです。たとえば自分が xyz プロトコルというのを作ったけど人に説明したいというときに、どう説明するか考えてみましょう。「これはレイヤ 3 のプロトコルで、…」という風に階層を指して (参照して) 説明を始めれば相手に通りがよいでしょう。つまり、OSI ネットワークモデルはネットワーク屋同士で通じる「語彙」として使える、まことに便利なものなのです。

でも、これはあくまで「参照」であって「規約」ではないので、すべてのネットワークプロトコル、ネットワーク機器がこれに従わなければいけない、というものではありません。さっき言ったように「この ○○ は、仮に OSI で言うとレイヤ4 にあたる」のように使うのが正しいのです。

そして、OpenFlow はたまたまいくつものレイヤの情報が使える、ただそれだけのことです。

====[/column]

== インストラクション

アクションとは、スイッチに入ってきたパケットをどう料理するか、という@<em>{動詞}にあたる部分です。よく「OpenFlow でパケットを書き換えて曲げる」などと言いますが、こうした書き換えなどはすべてアクションで実現できます。それでは、OpenFlow 1.3 ではどんなアクションが定義されているか見てみましょう。

アクションは次の 6 種類があります。

 * Output: パケットを指定したポートから出す
 * Group:
 * Drop: パケットを捨てる
 * Set-Queue: ポートごとに指定されたスイッチのキューに入れる。QoS 用
 * Push-Tag/Pop-Tag: 
 * Set-Field: パケットの中身を書き換える
 * Change-TTL: 

アクションは動詞と同じく指定した順番に実行されます。「おにぎりを作って、食べて、片付ける」といったふうに。たとえば、パケットを書き換えて指定したポートから出したいときには、

//emlist{
[Modify-Field, Forward]
//}

というアクションのリストを指定します。ここで、アクションは指定された順番に実行されることに注意してください。アクションの順番を変えてしまうと、違う結果が起こります。たとえば「おにぎりを食べてから、おにぎりを作る」と最後におにぎりが残ってしまいます。同様に先ほどの例を逆にしてしまうと、まず先にパケットがフォワードされてしまいます。その後 Modify-Field が実行されても、書き換えられた後、そのパケットは破棄されるだけです。

//emlist{
# パケットを書き換える前にフォワードされてしまう。
[Forward, Modify-Field]
//}

同じ動詞を複数指定することもできます。

//emlist{
[Modify-Field A, Modify-Field B, Forward A, Forward B]
//}

この場合は、フィールド A と B を書き換えて、ポート A と B へフォワードする、と読めます。このように、複数のフィールドを書き換えたり、複数のポートにパケットを出したりする場合には、アクションを複数連ねて指定します@<fn>{num_actions}。

//footnote[num_actions][指定できるアクション数の上限は OpenFlow スイッチとコントローラの実装に依存します。普通に使う分にはまず問題は起こらないでしょう。]

Drop は特殊なアクションで、実際に Drop アクションというものが具体的に定義されているわけではありません。アクションのリストに Forward アクションをひとつも入れなかった場合、そのパケットはどこにもフォワードされずに捨てられます。これを便宜的に Drop アクションと呼んでいるわけです。

それでは、最もよく使われる Forward アクションと Modify-Field アクションで具体的に何が指定できるか見て行きましょう。

==== Forward アクション

Forward アクションでは指定したポートからパケットを出力します。出力先にはポート番号を指定しますが、特殊用途のために定義されている論理ポートを使うこともできます。

 * ポート番号: パケットを指定した番号のポートに出す。
 * IN_PORT: パケットを入ってきたポートに出す。
 * ALL: パケットを入ってきたポート以外のすべてのポートに出す。
 * FLOOD: パケットをスイッチが作るスパニングツリーに沿って出す。
 * CONTROLLER: パケットをコントローラに明示的に送り、Packet In を起こす。
 * NORMAL: パケットをスイッチの機能を使って転送する。
 * LOCAL: パケットをスイッチのローカルスタックに上げる。ローカルスタック上で動作するアプリケーションにパケットを渡したい場合に使う。あまり使われない。

この中でも FLOOD や NORMAL は OpenFlow スイッチ機能と既存のスイッチ機能を組み合わせて使うための論理ポートです。

==== Modify-Field アクション

Modify-Field アクションではパケットのさまざまな部分を書き換えできます。

 * 送信元 MAC アドレスの書き換え
 * 宛先 MAC アドレスの書き換え
 * 送信元 IP アドレスの書き換え
 * 宛先 IP アドレスの書き換え
 * ToS フィールドの書き換え
 * TCP/UDP 送信元ポートの書き換え
 * TCP/UDP 宛先ポートの書き換え
 * VLAN ヘッダの除去
 * VLAN ID の書き換え (VLAN ヘッダがなければ、新たに付与する)
 * VLAN プライオリティの書き換え (VLAN ヘッダがなければ、新たに付与する)

それぞれのアクションでできることと、代表的な使い道を順番に見ていきましょう。

===== MAC アドレスの書き換え

MAC アドレス書き換えの代表的な例がルータです。OpenFlow はルータの実装に必要な、送信元と宛先 MAC アドレスの書き換えアクションをサポートしています。

//image[rewrite_mac][ルータでの送信元と宛先 MAC アドレスの書き換え][width=12cm]

ルータは 2 つのネットワークの間で動作し、ネットワーク間で行き交うパケットの交通整理を行います。ホスト A が異なるネットワークに属するホスト B にパケットを送ると、ルータはそのパケットを受け取りその宛先 IP アドレスから転送先のネットワークを決定します。そして、パケットに記述された宛先 MAC アドレスを次に送るべきホストの MAC アドレスに、送信元を自分の MAC アドレスに書き換えてデータを転送します。

===== IP アドレスの書き換え

IP アドレス書き換えの代表的な例が NAT (Network Address Transition) です。OpenFlow は NAT の実装に必要な、送信元と宛先 IP アドレスの書き換えアクションをサポートしています。

//image[rewrite_ip_address][NAT での送信元と宛先 IP アドレスの書き換え][width=12cm]

インターネットと接続するルータでは、プライベート/グローバルネットワーク間での通信を通すために IP アドレスを次のように変換します。プライベートネットワーク内のクライアントからインターネット上のサーバーに通信をする場合、ゲートウェイはプライベートネットワークから届いたパケットの送信元 IP アドレスを自分のグローバルな IP アドレスに変換して送信します。逆にサーバーからの返信は逆の書き換えを行うことによりプライベートネットワーク内のクライアントに届けます。

===== ToS フィールドの書き換え

ToS フィールドは通信のサービス品質 (QoS) を制御する目的でパケットを受け取ったルータに対して処理の優先度を指定するために使われます。OpenFlow はこの ToS フィールドの書き換えアクションをサポートしています。

===== TCP/UDP ポート番号の書き換え

TCP/UDP ポート番号書き換えの代表的な例が IP マスカレードです。OpenFlow は IP マスカレードの実装に必要な、送信元と宛先の TCP/UDP ポート番号の書き換えアクションをサポートしています。

//image[rewrite_port][IP マスカレードでの送信元と宛先 TCP/UDP ポート番号の書き換え][width=12cm]

ブロードバンドルータなど 1 つのグローバルアドレスで複数のホストが同時に通信を行う環境では、NAT だけだと TCP/UDP のポート番号が重複する可能性があります。そこで、IP マスカレードではプライベートネットワーク側のポート番号をホストごとに適当に割り当て、通信のつどポート番号を変換することで解決します。

===== VLAN ヘッダの書き換え

既存のタグ付き VLAN で構築したネットワークと OpenFlow で構築したネットワークを接続するという特別な用途のために、VLAN ヘッダの書き換えアクションがあります。VLAN をひとことで説明すると、既存のスイッチで構成されるネットワーク (ブロードキャストが届く範囲のネットワーク) を複数のネットワークに分割して使用するための仕組みです。この分割したネットワーク自体を VLAN と呼ぶ場合もあります。どの VLAN に所属するかを区別するのが VLAN のタグ (VLAN ID) で、パケットに付与される VLAN ヘッダがこのタグ情報を含みます。Modify-Field アクションは VLAN ヘッダの操作に必要なアクションを 3 種類用意しています。

//image[strip_vlan][VLAN ヘッダを書き換えるアクションの使い道][width=12cm]

: VLAN ヘッダの除去
  VLAN を流れる VLAN ヘッダ付きパケットから VLAN ヘッダを除去し、普通のパケットに戻すアクションです。

: VLAN ID の書き換え
  VLAN パケットが属する VLAN の ID を書き換えます。たとえば VLAN ID を 3 に書き換えるといったアクションを指定できます。また、VLAN ヘッダがついていないパケットに 指定した VLAN ID を持つ VLAN ヘッダを付与することもできます。

: VLAN プライオリティの書き換え
  VLAN 上でのパケットを転送する優先度を変更します。このプライオリティはトラフィックの種類 (データ、音声、動画など) を区別する場合などに使います。指定できる値は 0 (最低) から 7 (最高) までです。

== まとめ

OpenFlow仕様の中でも特にポイントとなる部分を見てきました。ここまでの章で学んできた内容だけで、すでにOpenFlow専門家と言ってもよいほどの知識が身に付いたはずです。次の章ではOpenFlowコントローラを開発するための代表的なプログラミングフレームワークを紹介します。

