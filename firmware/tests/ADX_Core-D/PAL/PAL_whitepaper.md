# PAL Networkの基本骨子

## 1. 概要

**PAL Network** は、低コストMCUによる分散制御システムを対象とした、Polling-based Publish/Subscribe Networkである。

PALは **Polling Access Link** を意味する。

PAL Networkでは、MasterがPollingによって通信機会を制御し、各Nodeが持つTopic Publisherに対して順番にPublicationの機会を与える。

Publisherが送信したPublicationは共有Bus上に流れ、該当するTopicをSubscribeしているSubscriberがこれを受信する。

基本モデルは、

**Polling → Publish → Subscribe**

である。

PAL Networkの特徴は、通常のPub/SubモデルにPollingによる通信スケジューリングを組み合わせることで、低コストなMCU Node群に予測可能な通信周期と分散協調性を与えることにある。


## 2. PAL Networkの基本構造

PAL Networkは、Master、Node、Topic、Publisher、Subscriber、Shared Busによって構成される。

```text
                         PAL Network

                           Master
                             |
                           Poll
                             |
             +---------------+---------------+
             |               |               |
             v               v               v
           Topic A         Topic B         Topic C
             |               |               |
             v               v               v
           Node A          Node B          Node C
             |               |               |
           Publish         Publish         Publish
             |               |               |
             +---------------+---------------+
                             |
                             v
========================== PAL Bus ==========================
             |               |               |
             v               v               v
           Node B          Node C          Node D
             |               |               |
            CS              CS              CS
MasterはPublicationそのものを中継するのではない。
Masterの役割は、Pollingによって「どのTopicに、いつ通信機会を与えるか」を決定することである。
PublicationはPublisherから直接PAL Busへ送信され、Subscriberは共有Busからそれを受信する。
3. Node
PAL NetworkにおけるNodeは、単一の通信Endpointではない。
一つのNodeは、複数のTopicをPublishし、複数のTopicをSubscribeすることができる。
Node A
 ├── Publish: Temperature
 ├── Publish: MotorSpeed
 ├── Publish: MotorCurrent
 ├── Subscribe: Position
 └── Subscribe: LimitState
したがって、
Node = 複数のPublisher / Subscriberを実装する物理デバイス
として扱う。
NodeそのものをPolling単位として扱うのではなく、Network上のTopicをPolling単位として扱う。
4. Topic
PAL Networkでは、Topicが論理的な通信単位となる。
例えば、
Temperature
MotorSpeed
MotorCurrent
Position
LimitState
などがTopicとなる。
Topicには、データ型、Payloadサイズ、Polling周期、優先度など、通信スケジュールを決定するための属性を関連付けることができる。
Topicは単なるデータ名称ではない。
PAL Networkでは、TopicがPolling Schedule上の通信単位となる。
5. Unique Publisher per Topic
PAL Networkにおける重要な原則の一つは、
Each Topic has exactly one Publisher.
である。
すなわち、
Unique Publisher per Topic
を基本制約とする。
例えば、
Topic            Publisher
--------------------------------
Temperature      Node A
MotorSpeed       Node A
MotorCurrent     Node A
Position         Node B
LimitState       Node C
となる。
一つのNodeが複数のTopicをPublishすることはできる。
しかし、一つのTopicについて複数のNodeが同時にPublisherとなることはできない。
この制約は、単なるAddressing上の都合ではない。
PAL Networkの決定論的なPollingを成立させるための基本的な通信制約である。
6. Publisher
Publisherは、特定のTopicについてPublicationを生成するNode側の通信Endpointである。
例えば、
Node A

Publisher
 ├── Temperature
 ├── MotorSpeed
 └── MotorCurrent
となる。
Publisherが送信できるタイミングは、自由に決めるものではない。
MasterによるPollingによって通信機会が与えられたときに、対応するTopicのPublicationを送信する。
7. Common Subscriber
Subscriber（CS）は、特定のTopicをSubscribeするNode側の通信Endpointである。
複数のNodeが同じTopicをSubscribeすることができる。
            Topic: Temperature
                     |
                     v
                  Publisher
                     |
                     | Publish
                     v
================== PAL Bus ==================
          |                |                |
          v                v                v
        CS-B             CS-C             CS-D
          |                |                |
        Node B           Node C           Node D
このため、PAL Networkは一対多のデータ配布を自然に実現できる。
SubscriberはPublisherとの直接的な通信路を持つ必要がない。
Shared Bus上のPublicationを受信することで、Topic-based Pub/Subが成立する。
8. Polling
PAL Networkの通信スケジューリングはMasterによるPollingによって行われる。
MasterはPolling Scheduleに従ってTopicを順番にPollingする。
Polling Cycle

Poll(Temperature)
        |
        v
   Publication
        |
        v
Poll(MotorSpeed)
        |
        v
   Publication
        |
        v
Poll(Position)
        |
        v
   Publication
        |
        v
Poll(Current)
        |
        v
   Publication
Polling対象はNodeそのものではなく、Topicに紐付いたPublisherである。
これにより、同じNodeが複数のTopicを持っている場合でも、それぞれを独立したPolling Slotとして扱うことができる。
9. One Polling Slot = One Bounded Publication
PAL Networkの決定論的挙動を維持するため、基本原則として、
One Polling Slot grants one bounded publication opportunity.
とする。
つまり、MasterがあるTopicをPollingした場合、そのTopicのPublisherは原則として一つのBounded Packetを送信する。
Poll(Temperature)
        |
        v
Temperature Publisher
        |
        | One bounded packet
        v
PAL Bus
        |
        v
Next Poll
Masterが、
「Node Aが持っているデータを全部送信せよ」
という命令を行う設計にはしない。
そのような設計では、Node Aが内部に持つTopic数やデータ量によってPolling Slotの長さが変化し、Network Cycleの予測可能性が失われる。
例えば、
Poll(Node A)
    |
    +── Temperature
    +── MotorSpeed
    +── Current
    +── Voltage
    +── Diagnostic
    +── ...
という設計では、Node Aの実装内容によってBus占有時間が変化してしまう。
PAL Networkでは、
Poll(Temperature)
    ↓
1 packet

Poll(MotorSpeed)
    ↓
1 packet

Poll(Current)
    ↓
1 packet

Poll(Voltage)
    ↓
1 packet
のように、Topic単位で通信機会を分割する。
10. Topicが時間軸を形成する
PAL Networkでは、TopicはData Model上の単位であると同時に、Scheduling Model上の単位でもある。
Topic
  |
  +── Publisher
  |
  +── Data Type
  |
  +── Payload Size
  |
  +── Polling Slot
  |
  +── Publication
そのため、Network Cycleは基本的にPolling対象Topic数と各TopicのBounded Communication Timeによって決定される。
概念的には、
Tcycle = Σ Tslot
で表現できる。
各Polling Slotの最大時間がBoundedであれば、Network CycleもBoundedになる。
したがって、Node内部の処理量がNetwork Cycleを直接決定することを避ける。
11. Node-to-Node Communication
PAL Networkでは、Node-to-Node CommunicationをP2P通信として扱わない。
例えばNode AからNode BへTemperatureを伝達する場合、
Node A
  |
  | Publisher: Temperature
  |
  | Poll
  v
Publication
  |
  v
================ PAL Bus ================
                          |
                          v
                     Node B
                       CS
となる。
実際の通信経路は、
Master → Poll → Publisher → PAL Bus → Subscriber
である。
MasterがNode AのデータをNode Bへ転送するわけではない。
Node AがNode Bへ直接送信するわけでもない。
PAL Bus上のPublicationとSubscriptionによって、結果としてNode-to-Nodeの情報交換が成立する。
12. PAL NetworkへのNode加入
新しいNodeがPAL Networkへ加入する場合、そのNodeは通常のPublicationを開始する前に、Masterから必要なPublisher権限を取得する。
概念的には、
New Node
    |
    | Join Request
    v
Master
    |
    | UPT Request
    v
UPT Allocation
    |
    | Accept
    v
Polling List
    |
    v
Normal Operation
という流れになる。
具体的なJoin Sequenceは別途定義する。
加入用の専用Topic / UPTを用意し、新規Nodeがその通信機会を利用してMasterへ加入要求を送信する方式も候補となる。
13. UPT Registration
PAL Networkでは、PublisherがNetwork上で有効になるためには、MasterによるRegistrationが必要となる。
Masterは新しいPublisherを受理すると、そのPublisherをPolling Scheduleへ追加する。
UPT Registration

Node A
  |
  | Request
  v
Master
  |
  | Accept
  v
UPT Registration
  |
  v
Polling List
Polling Listは、PAL Networkにおける通信機会の管理表となる。
概念的には、
Polling List

Topic            Publisher
--------------------------------
Temperature      Node A
MotorSpeed       Node A
Position         Node B
Current          Node C
LimitState       Node C
のような構造になる。
14. Unauthorized Publisher
PAL Networkでは、Masterによって登録されていないPublisherが勝手にPublicationを送信することを想定する。
この場合、通常のPolling Scheduleに存在しないPublisherによるTransmissionとなるため、正常な通信と区別することができる。
さらに、TopicごとにPublisherが一意であるため、登録されていないPublisherが既存Topicを使用した場合、その問題は特定Topicに関連付けて切り分けることができる。
例えば、
Topic: Temperature
Registered Publisher: Node A

Unauthorized:
Node D → Temperature
の場合、
Temperature
   |
   +── Registered Publisher: Node A
   |
   +── Unauthorized Publisher: Node D
   |
   +── Collision / Protocol Violation
として識別できる。
ただし、共有Bus上で実際に物理的Collisionが発生した場合、その瞬間のFrameだけでなく、他の通信にも影響する可能性がある。
したがって、PAL Networkにおける「問題の局所化」は、主としてLogical Fault Domainの局所化を意味する。
15. PAL Networkの決定論
PAL Networkが目指す決定論は、各Nodeの高性能化によって実現するものではない。
高精度クロック、高機能なCommunication Controller、高いCPU性能を各Nodeに要求する代わりに、Network側のPollingとSchedulingによって通信機会を管理する。
Conventional distributed control

Node
 ├── Precise Clock
 ├── Advanced Controller
 └── High CPU Performance
            |
            v
       Deterministic behavior
これに対してPAL Networkでは、
PAL Network
 ├── Polling
 ├── Scheduling
 ├── Topic registration
 └── Bounded publication
            |
            v
     Low-cost MCU Nodes
            |
            v
   Predictable communication
というアプローチを取る。
16. CANとの違い
CANでは、複数のNodeがBusへの送信機会を競合し、Arbitrationによって送信Nodeが決定される。
Node A ─┐
Node B ─┼── Arbitration ──> Bus
Node C ─┘
PAL Networkでは、送信機会そのものをPollingによって事前に決定する。
Master
  |
  +── Poll(Topic A) ──> Publisher A ──> Bus
  |
  +── Poll(Topic B) ──> Publisher B ──> Bus
  |
  +── Poll(Topic C) ──> Publisher C ──> Bus
CANがNode側のArbitration能力によって分散制御を成立させるのに対し、PAL NetworkはNetwork側のPollingとSchedulingによって通信を協調させる。
17. Modbusとの違い
Modbusは基本的にMasterによるRequestとSlaveによるResponseを中心とする。
Master
  |
  | Request
  v
Slave
  |
  | Response
  v
Master
PAL Networkでは、PollingされたPublisherのPublicationを共有Busへ流し、複数のSubscriberがそれを受信できる。
Master
  |
  | Poll
  v
Publisher
  |
  | Publication
  v
PAL Bus
  |
  +──> Subscriber
  +──> Subscriber
  +──> Subscriber
したがってPAL Networkは、単純なRequest / Response型プロトコルではなく、
Polling-controlled Publish/Subscribe
として位置付けられる。
18. PAL Networkの基本原則
PAL Networkは、以下の原則を基本とする。
18.1 Polling-controlled Access
Network AccessはNode自身の競争ではなく、MasterのPollingによって制御される。
18.2 Unique Publisher per Topic
一つのTopicには、Network上で一つのPublisherだけが存在する。
18.3 Topic-oriented Scheduling
Polling ScheduleはNode単位ではなくTopic単位で構成される。
18.4 Bounded Publication
一つのPolling Slotで送信できるPublication量はBoundedでなければならない。
18.5 Multiple Topics per Node
一つのNodeは複数のTopicをPublishできる。
18.6 Multiple Subscriptions per Node
一つのNodeは複数のTopicをSubscribeできる。
18.7 Common Subscription
一つのTopicを複数のNodeがSubscribeできる。
18.8 Shared Bus
PublisherとSubscriberは共有Busを介して通信する。
18.9 Network-mediated Coordination
通信の協調機能を個々のNodeだけに持たせるのではなく、NetworkのPollingとSchedulingによって実現する。
18.10 Network-authorized Publication
通常のPublicationは、MasterによってNetworkへ登録されたPublisherだけが行う。
19. PAL Networkの論理モデル
PAL Networkの基本モデルは、次のように整理できる。
                   PAL Network

                         Master
                           |
                     Polling Schedule
                           |
          +----------------+----------------+
          |                |                |
          v                v                v
       Topic A          Topic B          Topic C
          |                |                |
        UPT-A            UPT-B            UPT-C
          |                |                |
          v                v                v
       Node A            Node B            Node C
          |                |                |
          +----------------+----------------+
                           |
                    Publication Bus
                           |
          +----------------+----------------+
          |                |                |
          v                v                v
        CS-A             CS-B             CS-C
          |                |                |
       Node D            Node E            Node F
ここで、
Node は物理的なデバイス。
Topic は論理的なデータ単位。
UPT はTopicに対して一意に割り当てられたPublisher。
CS はTopicをSubscribeするSubscriber。
Master はPollingとSchedulingを担当する。
PAL Bus はPublicationを共有する通信媒体。
という役割分担になる。
20. PAL Networkの基本通信シーケンス
PAL Networkの通常通信は、概念的には次のようになる。
1. Master selects next Topic
             |
             v
2. Master polls Topic Publisher
             |
             v
3. Publisher receives polling opportunity
             |
             v
4. Publisher sends one bounded Publication
             |
             v
5. Publication appears on PAL Bus
             |
             +--------> CS
             +--------> CS
             +--------> CS
             |
             v
6. Master advances to next polling slot
このサイクルを繰り返すことで、PAL Network全体の通信周期を構成する。
21. PAL Networkの本質
PAL Networkの本質は、単なるRS-485通信プロトコルでも、LINの拡張でもない。
また、単純なMaster / Slave型Request / Responseでもない。
PAL Networkは、
Topic-oriented Publish/Subscribe
と
Master-controlled Polling
と
Bounded Communication Slots
を組み合わせた分散制御ネットワークである。
その中心的な関係は、
Topic
  |
  | exactly one publisher
  v
UPT
  |
  | scheduled polling
  v
One bounded publication
  |
  v
PAL Bus
  |
  +──> CS
  +──> CS
  +──> CS
となる。
この構造によって、低コストなMCU Node群に対して、Network側から通信の秩序、予測可能性、そして分散協調性を与える。
22. Current Definition
現時点でのPAL Networkの暫定的な定義を次のようにする。
PAL Network is a polling-controlled publish/subscribe network in which each topic has a unique publisher, and a Master schedules bounded publication opportunities for those topics over a shared bus.
日本語では、
PAL Networkは、各Topicに一意のPublisherを割り当て、MasterがそのTopicごとのBoundedなPublication機会を共有Bus上でPollingによってスケジュールする、分散制御向けPublish/Subscribeネットワークである。
と定義する。

この定義を基礎として、今後の仕様では、UPT/CSの識別方法、Topic Addressing、Polling Schedule、Frame Format、Data Type、Payload Size、Timing、Timeout、Error Handling、Node Join、UPT Registration、Synchronizationなどを具体化していく。
