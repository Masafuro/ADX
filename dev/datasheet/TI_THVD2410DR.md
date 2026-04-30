具有 IEC ESD 保护功能的 THVD24x0 ±70V 故障保护 3.3V 至 5V RS-485 收发器

THVD2410, THVD2450
ZHCSJZ4B – JULY 2019 – REVISED OCTOBER 2021

3 说明

THVD2410  和  THVD2450  是  ±70V  故障保护、半双
工、RS-422/RS-485 收发器，由 3V 至 5.5V 的单电源
供电。在所有运行模式下均可保护总线接口引脚不受过
压条件破坏，可确保在恶劣的工业环境中实现稳定可靠
的通信。

这些器件具有集成式 IEC ESD 保护，无需外部系统级
保护组件。在更长的电缆敷设长度和/或存在大接地环
路电压的情况下，扩展 ±25V 输入共模范围可保证数据
通信稳定可靠。增强型 250mV 接收器迟滞可确保实现
高噪声抑制。此外，当输入同时开路或短路时，接收器
失效防护功能可保证处于逻辑高电平。

THVD24x0 器件采用小型 VSSOP 和 VSON 封装，适
用于空间受限型应用。这些器件在自然通风环境下的额
定温度范围为 –40°C 至 125°C。

器件型号

THVD2410
THVD2450

器件信息
封装(1)

VSON (8)

VSSOP (8)

SOIC (8)

封装尺寸（标称值）

3.00mm × 3.00mm

3.00mm × 3.00mm

4.90mm × 3.91mm

(1) 如需了解所有可用封装，请参阅数据表末尾的可订购产品附

录。

THVD2410 和 THVD2450 简化版原理图

1 特性

• 符合或超过

TIA/EIA-485A 和 TIA/EIA-422B 标准的要求

• 提供功能安全

– 可帮助进行功能安全系统设计的文档

• 3V 至 5.5V 电源电压
• 差分输出超过 2.1V，在 5V 电源下

与 PROFIBUS 兼容

• 总线 I/O 保护

– ±70V 直流总线故障
– ±16kV HBM ESD
– ±12kV IEC 61000-4-2 接触放电
– ±12kV IEC 61000-4-2 空气间隙放电
– ±4kV IEC 61000-4-4 快速瞬变脉冲

• 提供两种速度等级的

半双工器件
– THVD2410：500kbps
– THVD2450：50Mbps

• 工作环境

温度范围：-40°C 至 125°C

• 扩展级运行

共模范围：±25 V
• 增强型接收器迟滞，
可获得抗噪能力

• 低功耗

– 低待机电源电流：< 1µA
– 运行期间的电流：<5.6 mA

• 适用于热插拔功能的无干扰上电/断电
• 开路、短路和空闲总线失效防护
• 热关断
• 1/8 单位负载（多达 256 个总线节点）
• 小型 VSON 和 VSSOP 封装（可节省布板空间）或

SOIC 封装（可实现快插兼容性）

2 应用

• 电机驱动器
• 工厂自动化和控制
• HVAC 系统
• 楼宇自动化
• 电网基础设施
• 电表
• 过程分析
• 视频监控

本文档旨在为方便起见，提供有关 TI 产品中文版本的信息，以确认产品的概要。有关适用的官方英文版本的最新信息，请访问
www.ti.com，其内容始终优先。TI 不保证翻译的准确性和有效性。在实际设计之前，请务必参考最新版本的英文版本。

English Data Sheet: SLLSF20

AB76RD14REDE23THVD2410, THVD2450
ZHCSJZ4B – JULY 2019 – REVISED OCTOBER 2021

www.ti.com.cn

内容

1 特性................................................................................... 1
2 应用................................................................................... 1
3 说明................................................................................... 1
4 修订历史记录..................................................................... 2
5 引脚配置和功能................................................................. 3
6 规格................................................................................... 4
6.1 绝对最大额定值...........................................................4
6.2 ESD 等级.................................................................... 4
6.3 ESD 等级 [IEC]........................................................... 4
6.4 建议运行条件.............................................................. 5
6.5 热性能信息.................................................................. 5
6.6 功率损耗......................................................................5
6.7 电气特性......................................................................6
6.8 开关特性：THVD2410................................................ 7
6.9 开关特性：THVD2450................................................ 7
6.10 典型特性....................................................................8
7 参数测量信息................................................................... 10
8 详细说明.......................................................................... 12

8.1 概述...........................................................................12
8.2 功能方框图................................................................ 12
8.3 特性说明....................................................................12
8.4 器件功能模式............................................................ 13
9 应用和实现.......................................................................15
9.1 应用信息 ...................................................................15
9.2 典型应用....................................................................15
10 电源相关建议................................................................. 20
11 布局................................................................................21
11.1 布局指南..................................................................21
11.2 布局示例..................................................................21
12 器件和文档支持............................................................. 22
12.1 器件支持..................................................................22
12.2 接收文档更新通知................................................... 22
12.3 支持资源..................................................................22
12.4 商标.........................................................................22
12.5 Electrostatic Discharge Caution..............................22
12.6 术语表..................................................................... 22

4 修订历史记录

Changes from Revision A (October 2019) to Revision B (October 2021)
Page
• 添加了特性 “提供功能安全型”........................................................................................................................ 1

Changes from Revision * (July 2019) to Revision A (October 2019)
Page
• 删除了应用：地震测试设备.................................................................................................................................1
• 删除了器件信息 表中的 THVD2410 产品预发布说明..........................................................................................1

2

Submit Document Feedback

Copyright © 2022 Texas Instruments Incorporated

Product Folder Links: THVD2410 THVD2450

www.ti.com.cn

5 引脚配置和功能

THVD2410, THVD2450
ZHCSJZ4B – JULY 2019 – REVISED OCTOBER 2021

图 5-1. D (SOIC) 和 DGK (VSSOP)，8 引脚封装，顶视图

图 5-2. DRB (VSON)，8 引脚封装，顶视图

表 5-1. 引脚功能

引脚

DGK

DRB

I/O

说明

6

7

4

3

5

1

8

2

6

7

4

3

5

1

8

2

总线输入/输出 总线 I/O 端口、A（与 B 互补）

总线输入/输出 总线 I/O 端口、B（与 A 互补）

数字输入

驱动器数据输入

数字输入

驱动器使能，高电平有效（内部 2MΩ 下拉）

接地

器件接地

数字输出

接收数据输出

电源

3.3V 至 5V 电源

数字输入

接收器使能，低电平有效（内部 2MΩ 上拉电阻）

名称

A

B

D

DE

GND

R

VCC

RE

D

6

7

4

3

5

1

8

2

散热焊盘

—

—

—

—

无电气连接。应连接到 GND 平面从而获得最佳热性能

Copyright © 2022 Texas Instruments Incorporated

Submit Document Feedback

3

Product Folder Links: THVD2410 THVD2450

1R8 VCC 2RE7 B3DE6 A4D5 GNDNot to scale1R8 VCC 2RE7 B3DE6 A4D5 GNDNot to scaleThermalPadTHVD2410, THVD2450
ZHCSJZ4B – JULY 2019 – REVISED OCTOBER 2021

6 规格

6.1 绝对最大额定值
在自然通风条件下的工作温度范围内测得（除非另有说明）(1)

www.ti.com.cn

电源电压

总线电压

输入电压

接收器输出电流

存储温度

VCC

以 GND 为基准的任何总线引脚（A 或 B）的差模或共
模范围

任何逻辑引脚（D、DE 或 RE）上的电压范围

IO

Tstg

最小值

最大值

单位

-0.5

–70

–0.3

–24

–65

7

70

5.7

24

170

V

V

V

mA

°C

(1) 超出绝对最大额定值 的运行可能会对器件造成永久损坏。绝对最大额定值并不表示器件在这些条件下或在建议运行条件 以外的任何其他
条件下能够正常运行。如果在建议运行条件 之外但在绝对最大额定值 范围内短暂运行，器件可能不会受到损坏，但可能无法完全正常工
作。以这种方式运行器件可能会影响器件的可靠性、功能和性能，并缩短器件寿命。

6.2 ESD 等级

V(ESD)

静电放电

人体放电模型 (HBM)，符合 ANSI/ESDA/JEDEC
JS-001 标准(1)

总线端子和 GND

除总线端子和 GND 外的所有
引脚

充电器件模型 (CDM)，符合 ANSI/ESDA/JEDEC JS-002 标准(2)

(1)
(2)

JEDEC 文档 JEP155 指出：500V HBM 可实现在标准 ESD 控制流程下安全生产。
JEDEC 文件 JEP157 指出：250V CDM 可实现在标准 ESD 控制流程下安全生产。

6.3 ESD 等级 [IEC]

V(ESD)

静电放电

接触放电，符合 IEC 61000-4-2 标准

总线端子和 GND

空气间隙放电，符合 IEC 61000-4-2 标准

总线端子和 GND

V(EFT)

电快速瞬变

符合 IEC 61000-4-4 标准

总线终端

值

单位

±16,000

±8,000

±1,500

V

V

V

值

单位

±12,000

±12,000

±4,000

V

V

4

Submit Document Feedback

Copyright © 2022 Texas Instruments Incorporated

Product Folder Links: THVD2410 THVD2450

www.ti.com.cn

6.4 建议运行条件

THVD2410, THVD2450
ZHCSJZ4B – JULY 2019 – REVISED OCTOBER 2021

在自然通风条件下的工作温度范围内测得（除非另有说明）

最小值

标称值

最大值

单位

VCC

VI

VIH

VIL

VID

IO

IOR

RL

1/tUI

TA

TJ

电源电压

任何总线端子上的输入电压（独立或共模）(1)

高电平输入电压（驱动器、驱动器使能和接收器使能输入）

低电平输入电压（驱动器、驱动器使能和接收器使能输入）

差分输入电压

驱动器输出电流

接收器输出电流

差分负载电阻

信令速率

运行环境温度

结温

THVD2410

THVD2450

(1) 本数据表采用将最小正值（最大负值）指定为最小值的代数约定。

3

–25

2

–25

-60

–8

54

-40

-40

6.5 热性能信息

热指标(1)

RθJA

RθJC(top)

RθJB

ψJT

ψJB

结至环境热阻

结至外壳（顶部）热阻

结至电路板热阻

结至顶部特征参数

结至电路板特征参数

RθJC(bot)

结至外壳（底部）热阻

THVD2410
THVD2450

D
(SOIC)

8 引脚

115.9

53.1

60.1

10.1

59.2

不适用

THVD2410
THVD2450

DGK
(VSSOP)

8 引脚

164.0

49.5

85.5

5.1

83.7

不适用

(1) 有关传统和新热指标的更多信息，请参阅半导体和 IC 封装热指标应用报告。

6.6 功率损耗

参数

测试条件

PD

驱动器和接收器已启用，
VCC = 5.5V，TA = 125℃，
随机数据 (PRBS7) 在信号传输速率下传输

未端接
RL = 300Ω、CL = 50pF（驱动器）

RS-422 负载
RL = 100Ω、CL = 50pF（驱动器）

RS-485 负载
RL = 54Ω、CL = 50pF（驱动器）

THVD2410

500kbps

THVD2450

50Mbps

THVD2410

500kbps

THVD2450

50Mbps

THVD2410

500kbps

THVD2450

50Mbps

5.5

25

0.8

25

60

8

500

50

125

150

60

THVD2410
THVD2450

DRB
(VSON)

8 引脚

47.6

49.4

20.3

0.9

20.2

5.6

值

130

340

170

340

240

370

V

V

V

V

V

mA

mA

Ω

kbps

Mbps

°C

°C

单位

°C/W

°C/W

°C/W

°C/W

°C/W

°C/W

单位

mW

mW

mW

Copyright © 2022 Texas Instruments Incorporated

Submit Document Feedback

5

Product Folder Links: THVD2410 THVD2450

短路输出电流

DE = VCC、–70V ≤（VA 或 VB）≤ 70V

-250

250

mA

THVD2410, THVD2450
ZHCSJZ4B – JULY 2019 – REVISED OCTOBER 2021

6.7 电气特性

www.ti.com.cn

在自然通风条件下的工作温度范围内测得（除非另有说明）。所有典型值均在 25℃ 和电源电压 VCC = 5V 时测得。

测试条件

最小值 典型值 最大值 单位

参数

驱动器

|VOD|

驱动器差分输出电压幅度

RL = 60Ω、–25V ≤ Vtest ≤ 25V（请参阅图 7-1）

RL = 60Ω、–25V ≤ Vtest ≤ 25V、4.5V ≤ VCC ≤ 5.5V（请参阅图 7-1）

RL = 100Ω（请参阅图 7-2）

RL = 54Ω（请参阅图 7-2）

Δ|VOD| 差分输出电压的变化

RL = 54Ω 或 100Ω（请参阅图 7-2）

共模输出电压

RL = 54Ω 或 100Ω（请参阅图 7-2）

稳态共模输出电压的变化

RL = 54Ω 或 100Ω（请参阅图 7-2）

VOC

ΔVOC
(SS)

IOS

接收器

II

总线输入电流

DE = 0V、VCC = 0V 或 5.5V

DE = 0V、VCC = 0V
或 5.5V

VTH+

VTH-

VHYS

正向输入阈值电压(1)

负向输入阈值电压(1)

输入滞后

VTH_FSH 输入失效防护阈值

±25V 的共模范围以外

输入差分电容

在 A 和 B 之间测得、f = 1MHz

输出高电压

输出低电压

IOH = –8mA

IOL = 8 mA

输出高阻抗电流

VO = 0V 或 VCC、RE = VCC

输入电流 (DE)

3V ≤ VCC ≤ 5.5V、0V ≤ VIN ≤ VCC

输入电流（D、RE）

3V ≤ VCC ≤ 5.5V、0V ≤ VIN ≤ VCC

CA,B

VOH

VOL

IOZ

逻辑

IIN

IIN

过热保护

TSHDN

热关断阈值

温度上升

热关断迟滞

THYS

电源

ICC

电源电流（静态）

驱动器和接收器已启用

驱动器被启用，接收器被禁用

驱动器被禁用，接收器被启用

驱动器和接收器被禁用

RE = 0V、DE =
VCC，空载

RE = VCC、DE =
VCC，空载

RE = 0V、DE = 0V，
空载

RE = VCC、DE =
0V、D = 开路，空载

(1) 在任何特定情况下，确保 VTH+ 至少比 VTH– 高 VHYS。

VI = 12 V

VI = 25 V

VI = –7 V

VI = -25 V

125

250

200

-40

40

75

150

–100

–40

-250 –150

40

125

–200 –125

-40

250

50

VCC –
0.4

VCC –
0.2

3.3

3.3

4

3.3

VCC/2

1.5

2.1

2

1.5

-50

1

-50

V

V

V

V

mV

V

mV

50

3

50

μA

mV

mV

mV

mV

pF

V

V

µA

µA

µA

°C

°C

0.2

0.4

1

5

-1

-5

150

170

10

3.5

2.5

1.8

0.1

5.6

mA

4.4

mA

2.4

mA

1

µA

6

Submit Document Feedback

Copyright © 2022 Texas Instruments Incorporated

Product Folder Links: THVD2410 THVD2450

www.ti.com.cn

THVD2410, THVD2450
ZHCSJZ4B – JULY 2019 – REVISED OCTOBER 2021

6.8 开关特性：THVD2410
500kbps 器件 (THVD2410)，在建议运行条件下测得。所有典型值均在 25℃ 和电源电压 VCC = 5V 时测得。
典型值

测试条件

最小值

参数

驱动器

tr，tf

差分输出上升/下降时间

tPHL，tPLH 传播延迟

RL = 54 Ω，CL = 50 pF

请参阅 图 7-3

tSK(P)

脉冲延迟，|tPHL – tPLH|

tPHZ，tPLZ 禁用时间

tPZH，tPZL 启用时间

tSHDN

接收器

tr，tf

到关断的时间

输出上升/下降时间

RE = 0V

RE = VCC

RE = VCC

请参阅图 7-4 和图 7-5

tPHL，tPLH 传播延迟

CL = 15 pF

请参阅图 7-6

tSK(P)

脉冲延迟，|tPHL – tPLH|

tPHZ，tPLZ 禁用时间

tPZH(1)、
tPZL(1)、
tPZH(2)、
tPZL(2)

tD(OFS)

tD(FSO)

tSHDN

启用时间

延迟进入失效防护操作

延迟退出失效防护操作

到关断的时间

DE = VCC

DE = 0V

CL = 15 pF

DE = 0V

请参阅图 7-7

请参阅 图 7-8

请参阅 图 7-9

请参阅 图 7-8

280

275

45

175

1.5

13

50

30

90

2

10

45

240

50

7

35

50

6.9 开关特性：THVD2450
50Mbps 器件 (THVD2450)，在建议运行条件下测得。所有典型值均在 25℃ 和电源电压 VCC = 5V 时测得。
典型值

测试条件

最小值

参数

驱动器

tr，tf

差分输出上升/下降时间

tPHL，tPLH 传播延迟

RL = 54 Ω，CL = 50 pF

请参阅图 7-3

tSK(P)

脉冲延迟，|tPHL – tPLH|

tPHZ，tPLZ 禁用时间

tPZH，tPZL 启用时间

tSHDN

接收器

tr，tf

到关断的时间

输出上升/下降时间

RE = 0V

RE = VCC

RE = VCC

请参阅图 7-4 和图 7-5

tPHL，tPLH 传播延迟

CL = 15 pF

请参阅 图 7-6

tSK(P)

脉冲延迟，|tPHL – tPLH|

tPHZ，tPLZ 禁用时间

tPZH(1)、
tPZL(1)、
tPZH(2)、
tPZL(2)

tD(OFS)

tD(FSO)

tSHDN

启用时间

延迟进入失效防护操作

延迟退出失效防护操作

到关断的时间

DE = VCC

DE = 0V

CL = 15 pF

DE = 0V

请参阅 图 7-7

请参阅 图 7-8

请参阅 图 7-9

请参阅 图 7-8

5

10

11

8

1.5

2

40

7

50

2

10

35

5

50

7

25

50

最大值

单位

600

350

10

95

270

4

500

20

80

7

40

120

ns

ns

ns

ns

ns

µs

ns

ns

ns

ns

ns

ns

4

μs

18

60

500

μs

ns

ns

最大值

单位

7

16

3.5

30

25

4

500

6

55

4

15

70

ns

ns

ns

ns

ns

μs

ns

ns

ns

ns

ns

ns

4

μs

18

50

500

μs

ns

ns

Copyright © 2022 Texas Instruments Incorporated

Submit Document Feedback

7

Product Folder Links: THVD2410 THVD2450

THVD2410, THVD2450
ZHCSJZ4B – JULY 2019 – REVISED OCTOBER 2021

6.10 典型特性

www.ti.com.cn

DE = VCC

D = 0V

TA=25°C

DE = VCC

D = 0V

TA=25°C

图 6-1. 驱动器输出电压与驱动器输出电流之间的关系

图 6-2. 驱动器差分输出电压与驱动器输出电流之间的关系

TA=25°C

DE = D = VCC

RL = 54Ω

图 6-4. THVD2410 驱动器上升时间或下降时间与温度之间的关系

图 6-3. 驱动器输出电流与电源电压间的关系

图 6-5. THVD2410 驱动器传播延迟与温度之间的关系

TA=25°C

RL = 54Ω

图 6-6. THVD2410 电源电流与信号速率之间的关系

8

Submit Document Feedback

Copyright © 2022 Texas Instruments Incorporated

Product Folder Links: THVD2410 THVD2450

Driver Output Current (mA)Driver Output Voltage (V)010203040506070809010000.511.522.533.544.55D001VOL (VCC = 5 V)VOH (VCC = 5 V)VOL (VCC = 3.3 V)VOH (VCC = 3.3 V)Driver Output Current (mA)Driver Differential Output Voltage (V)0102030405060708090100-0.500.511.522.533.544.55D002VOD (VCC = 5 V)VOD (VCC = 3.3 V)Supply Voltage (V)Driver Output Current (mA)00.511.522.533.544.555.5-50510152025303540455055606570D003Temperature (0C)Driver Rise and Fall Time (ns)-60-40-20020406080100120140235240245250255260265270275280285290295D007Rise time (VCC = 5 V)Fall time (VCC = 5 V)Rise time (VCC = 3.3 V)Fall time (VCC = 3.3 V)Temperature (0C)Driver Propagation Delay (ns)-60-40-20020406080100120140268270272274276278280282284286D008tPLH (VCC = 5 V)tPHL (VCC = 5 V)tPLH (VCC = 3.3 V)tPHL (VCC = 3.3 V)Signaling Rate (kbps)Supply Current (mA)050100150200250300350400450500404550556065707580859095100105110115120D009VCC = 5 VVCC = 3.3 Vwww.ti.com.cn

6.10 典型特性 (continued)

THVD2410, THVD2450
ZHCSJZ4B – JULY 2019 – REVISED OCTOBER 2021

图 6-7. THVD2450 驱动器上升时间或下降时间与温度之间的关系

图 6-8. THVD2450 驱动器传播延迟与温度之间的关系

TA=25°C

RL = 54Ω

图 6-9. THVD2450 电源电流与信号速率之间的关系

Copyright © 2022 Texas Instruments Incorporated

Submit Document Feedback

9

Product Folder Links: THVD2410 THVD2450

Temperature (0C)Driver Rise and Fall Time (ns)-40-200204060801001201404.94.9555.055.15.155.25.255.35.355.45.455.55.555.6D004Rise time (VCC = 5 V)Fall time (VCC = 5 V)Rise time (VCC = 3.3 V)Fall time (VCC = 3.3 V)Temperature (0C)Driver Propagation Delay (ns)-40-200204060801001201407.588.599.51010.51111.51212.51313.514D005tPLH (VCC = 5 V)tPHL (VCC = 5 V)tPLH (VCC = 3.3 V)tPHL (VCC = 3.3 V)Signaling Rate (Mbps)Supply Current (mA)05101520253035404550404550556065707580859095100105110115120D006VCC = 5 VVCC = 3.3 VTHVD2410, THVD2450
ZHCSJZ4B – JULY 2019 – REVISED OCTOBER 2021

7 参数测量信息

www.ti.com.cn

图 7-1. 具有共模负载的驱动器差分输出电压的测量

图 7-2. 具有 RS-485 负载的驱动器差分和共模输出的测量

图 7-3. 驱动差分输出上升和下降次数以及传播延迟的测量

图 7-4. 具有高电平有效输出和下拉负载的驱动器启用和禁用次数的测量

图 7-5. 具有低电平有效输出和上拉负载的驱动器启用和禁用次数的测量

10

Submit Document Feedback

Copyright © 2022 Texas Instruments Incorporated

Product Folder Links: THVD2410 THVD2450

0V or VccABDEVccVtest      VODD375 Ÿ RL375 ŸABVOCVOC(PP)ûVOC(SS)VAVB0V or VccABDRL/2CLVOCVODRL/2VODVItPLHtPHLtrtf90%50%10%50%Vcc± 2 V0 V InputGeneratorABVccVODDRL=  54 Ÿ VI50 ŸDECL= 50 pF~~~~2 VVOVItPZHtPHZ50%50%Vcc§(cid:3)0V0VVOH90%InputGeneratorABVODRL= 110 ŸVI50ŸDECL= 50 pFS1Copyright © 2017, Texas Instruments IncorporatedVOVItPZLtPLZ50%50%Vcc§(cid:3)Vcc0VVOL10%InputGeneratorABVODRL= 110 ŸVI50ŸDECL= 50 pFS1VccCopyright © 2017, Texas Instruments Incorporatedwww.ti.com.cn

THVD2410, THVD2450
ZHCSJZ4B – JULY 2019 – REVISED OCTOBER 2021

图 7-6. 接收器输出上升和下降次数以及传播延迟的测量

图 7-7. 驱动器已启用情况下接收器启用/禁用次数的测量

图 7-8. 驱动器被禁用情况下接收器启用次数的测量

图 7-9. 失效防护延迟测量

Copyright © 2022 Texas Instruments Incorporated

Submit Document Feedback

11

Product Folder Links: THVD2410 THVD2450

VODVItPLHtPHLtrtf90%50%10%50%3 VVOHVOL0 VInputGeneratorVOVI50 ŸREABR1.5V0VCL=15 pF VOVItPHZ50%50%Vcc0V0VVOH90%50%10%VOLVCCtPLZtPZL(1)VOtPZH(1)D at Vcc S1 to GNDInputGeneratorVOVI50 ŸABRCL=15 pF RE0V or VccDEVccDVcc1 kŸS1§(cid:3)D at 0V S1 to Vcc  VOVI50%Vcc§(cid:3)0V0VVOH50%VOLVCCtPZL(2)VOtPZH(2)A at 1.5VB at 0VS1 to GNDA at 0VB at 1.5VS1 to VCC50%InputGeneratorVOVI50ŸABRCL= 15 pFRE0V or 1.5 VVcc1 kŸS11.5 V or 0 V50%90%10%tPLZtPHZCopyright © 2017, Texas Instruments IncorporatedVOABRCL= 15 pFREVA = 0 V or -750 mVVB = 0 V or +750 mV0 VVA - VB-1.5 V0 VVO0 VVCCVCC / 2tD(OFS)tD(FSO)Copyright © 2017, Texas Instruments IncorporatedTHVD2410, THVD2450
ZHCSJZ4B – JULY 2019 – REVISED OCTOBER 2021

www.ti.com.cn

8 详细说明
8.1 概述

THVD2410 和 THVD2450 是具有故障保护功能的半双工 RS-485 收发器，提供两种速度等级，分别适用于速率高
达 500kbps 和 50Mbps 的数据传输。这些器件均具有高电平有效的驱动器使能引脚和低电平有效的接收器使能引
脚。禁用驱动器和接收器后可获得低于 1µA 的关断电流。

8.2 功能方框图

图 8-1. THVD2410 和 THVD2450 方框图

8.3 特性说明
8.3.1 ±70V 故障保护

与标准 RS-485 器件相比，THVD24x0 收发器加强了总线故障保护。在恶劣工业环境中运行的收发器经常承受超
过 TIA/EIA-485A 标准定义 –7V 至 +12V 的电压瞬变。为了防止在这种情况下受到损坏，具有较低绝对最大额定
值的通用 RS-485 器件需要成本高昂的外部保护元件。为了简化系统设计并降低整体系统成本，THVD24x0 器件
无需任何外部元件即可在高达 ±70V 的电压下受到保护。

8.3.2 集成 IEC ESD 和 EFT 保护

内部 ESD 保护电路可根据高达 ±12kV 的 IEC 61000-4-2 标准，保护收发器免受静电放电 (ESD) 的影响，并根据
高达 ±4kV 的 IEC 61000-4-4 标准保护收发器免受电气快速瞬变 (EFT) 的影响。THVD24x0 ESD 结构有助于限制
电压偏移并快速从中恢复，因为它们允许系统级的 EFT 标准 A（有瞬态噪声时不会丢失数据）。

8.3.3 驱动器过压和过流保护

THVD24x0 驱动器可防止 –70V 至 +70V 范围内的任何直流电源短路。器件内部将短路电流限制为 ±250mA，以
符合 TIA/EIA-485A 标准。此外，如果输出故障电压超过 |±25V|，折返式限流电路可进一步将驱动器短路电流降至
±5mA 以下。

所有器件都具有热关断保护功能，如果结温超过 TSHDN，则会因过多功率损耗而禁用驱动器和接收器。

8.3.4 增强型接收器抗噪性能

THVD24x0  的差分接收器具有完全对称的阈值，即使在输入振幅很小的情况下也能保持信号的占空比。此外，
250mV（典型值）迟滞可确保出色的抗噪性能。

12

Submit Document Feedback

Copyright © 2022 Texas Instruments Incorporated

Product Folder Links: THVD2410 THVD2450

RDDEREBAGNDVCCwww.ti.com.cn

8.3.5 接收器失效防护状态运行

接收器因以下原因而失效防护，导致无效总线状态：
• 出现总线开路，例如连接器断开
• 出现总线短路，例如电缆损坏，将双绞线短路在一起
• 当总线上的驱动器没有进行有源驱动时，出现总线空闲

THVD2410, THVD2450
ZHCSJZ4B – JULY 2019 – REVISED OCTOBER 2021

其中任一种情况下，如果输入振幅停留时间超过 tD(OFS) 至少 |VTH_FSH|，接收器将输出失效防护逻辑高电平状态。

8.3.6 低功耗关断模式

如果驱动 DE 低电平和 RE 高电平的时间多于 500ns，则器件进入关断模式。如果 DE 变为高电平或 RE 变为低电
平，计数器复位。如果使能引脚处于禁用状态的时间少于 50ns，则器件不会进入关断模式。此功能可防止器件由
于 DE 和 RE 之间的偏差而意外进入关断模式。

8.4 器件功能模式

当驱动器使能引脚 DE 为逻辑高电平时，差分输出 A 和 B 跟随数据输入 D 的逻辑状态。D 处的逻辑高电平会导致
A 变为高电平，B 变为低电平。在这种情况下，定义为 VOD = VA – VB 的差分输出电压为正。当 D 为低电平时，
输出状态反转：B 变为高电平，A 为低电平，VOD 是负数。

当 DE 为低电平时，两个输出都变为高阻态。在这种情况下，与 D 处的逻辑状态无关。DE 引脚有一个内部下拉
电阻接地。因此，当处于开路状态时，驱动器默认禁用（高阻抗）。D 引脚有一个内部上拉电阻接 VCC，因此，
当启用驱动器且处于开路状态时，输出 A 变为高电平，B 变为低电平。

输入

D

H

L

X

X

断开

使能

DE

H

H

L

断开

H

表 8-1. 驱动器功能表

输出

A

H

L

Z

Z

H

B

L

H

Z

Z

L

功能

有源驱动总线高电平

有源驱动总线低电平

驱动器被禁用

默认情况下，驱动器被禁用

默认情况下，有源驱动总线高电平

Copyright © 2022 Texas Instruments Incorporated

Submit Document Feedback

13

Product Folder Links: THVD2410 THVD2450

THVD2410, THVD2450
ZHCSJZ4B – JULY 2019 – REVISED OCTOBER 2021

www.ti.com.cn

当接收器使能引脚 RE 为逻辑低电平时，接收器被启用。当通过 VID = VA – VB 公式计算的差分输入电压高于正
输入阈值 VTH+ 时，接收器输出 R 变为高电平。当 VID 低于负输入阈值 VTH- 时，接收器输出 R 变为低电平。如果
VID 介于 VTH+ 和 VTH- 之间，则输出是不确定的。

当 RE 为逻辑高电平或处于开路时，接收器输出为高阻抗，与 VID 的幅度和极性无关。当收发器与总线断开（开
路）、总线线路短路（短路）或未对总线进行有源驱动（空闲总线）时，接收器输入的内部偏置会导致输出失效
防护高电平。

差分输入

VID = VA – VB
VTH+ < VID

VTH- < VID < VTH+

VID < VTH-

X

X

开电路总线

短路总线

闲置（终止）总线

表 8-2. 接收器功能表

使能

RE

L

L

L

H

断开

L

L

L

输出

R

H

?

L

Z

Z

H

H

H

功能

接收有效总线高电平

待定总线状态

接收有效总线低电平

接收器被禁用

默认情况下，接收器被禁用

故障安全高电平输出

故障安全高电平输出

故障安全高电平输出

14

Submit Document Feedback

Copyright © 2022 Texas Instruments Incorporated

Product Folder Links: THVD2410 THVD2450

www.ti.com.cn

9 应用和实现

THVD2410, THVD2450
ZHCSJZ4B – JULY 2019 – REVISED OCTOBER 2021

以下应用部分中的信息不属于 TI 元件规范，TI 不担保其准确性和完整性。TI 的客户应负责确定各元件
是否适用于其应用。客户应验证并测试其设计实现，以确认系统功能。

备注

9.1 应用信息

THVD2410 和 THVD2450 是具有故障保护功能的半双工 RS-485 收发器，通常用于异步数据传输。对于这些器
件，驱动器和接收器使能引脚允许配置不同的工作模式。

9.2 典型应用

RS-485 总线包含多个并联到总线电缆的收发器。为了消除线路反射，每个电缆末端都用一个端接电阻 RT，其值
与电缆的特征阻抗 0 匹配。这种方法称为并行端接，通常允许在更长的电缆长度上实现更高的数据速率。

图 9-1. 具有半双工收发器的典型 RS-485 网络

9.2.1 设计要求

RS-485 是一种稳健的电气标准，适用于长距离网络，可用于具有不同要求（例如距离、数据速率和节点数量）的
各种应用。

9.2.1.1 数据速率和总线长度

数据速率与电缆长度成反比关系，即数据速率越高，电缆长度越短；反之，数据速率越低，电缆长度越长。虽然
大多数 RS-485 系统使用介于 10kbps 和 100kbps 之间的数据速率，但在 4000 英尺或更远距离范围内，某些应用
需要的数据速率高达 250kbps。通过允许高达 5% 或 10% 的小信号抖动，可以实现更长的距离。

图 9-2. 电缆长度与数据速率特性间的关系

Copyright © 2022 Texas Instruments Incorporated

Submit Document Feedback

15

Product Folder Links: THVD2410 THVD2450

RTRTRABRREDEDDRABRREDEDDRDRREDEDABRDRREDEDABCable Length (ft)DataRate(bps)100001000100101001k10k100k1M10M100MConservativeCharacteristics5%, 10%, and 20% JitterTHVD2410, THVD2450
ZHCSJZ4B – JULY 2019 – REVISED OCTOBER 2021

www.ti.com.cn

在互连足够短（或在信号频率下具有适当的低衰减）、不会降低数据质量的情况下，甚至可以实现更高的数据速
率（即 THVD2450 速率为 50Mbps）。

9.2.1.2 桩线长度

将节点连接到总线时，收发器输入和电缆干线之间的距离（称为桩线）应尽可能短。桩线是一段无端接的总线线
路，随着长度的增加，桩线会引入相位不同的反射。作为通用指南，桩线的电气长度或往返延迟应小于驱动器上
升时间的十分之一，即最大的物理桩线长度，如方程式 1 所示。

L(STUB) ≤ 0.1 × tr × v × c

其中

• tr 是驱动器上升时间的 10/90
• c 是光速 (3 × 108m/s)
• v 是电缆或布线的信号速度，以 c 的系数表示

9.2.1.3 总线负载

(1)

RS-485 标准规定，符合标准的驱动器必须能够驱动 32 个单元负载 (UL)，其中 1 个单元负载表示大约 12kΩ 的负
载阻抗。由于 THVD24x0 器件包含 1/8 UL 收发器，因此可将多达 256 个接收器连接到总线。

16

Submit Document Feedback

Copyright © 2022 Texas Instruments Incorporated

Product Folder Links: THVD2410 THVD2450

www.ti.com.cn

9.2.1.4 瞬态保护

THVD2410, THVD2450
ZHCSJZ4B – JULY 2019 – REVISED OCTOBER 2021

THVD24x0 收发器系列的总线引脚包括针对 ±30kV HBM 和 ±12kV IEC 61000-4-2 接触放电的片上 ESD 保护。国
际电工委员会 (IEC) ESD  测试远比 HBM ESD  测试严格得多。IEC  模型的充电电容 C(S)  高出 50%，放电电阻
R(D) 低出 78%，所产生的放电电流明显高于 HBM 模型。如 IEC 61000-4-2 标准中所述，接触放电是首选的瞬态
保护测试方法。

图 9-3. HBM 模型和 IEC ESD 模型的电流比较（括号中为 HBM 值）

IEC ESD 保护的片上实现可显著提高设备的稳健性。人体接触连接器和电缆时，会发生常见的放电事件。设计人
员可以选择针对持续时间较长的瞬变（通常称为浪涌瞬变）实施保护。

EFT 通常是由继电器触点回跳或电感负载中断引起的。浪涌瞬变通常由雷击（直接雷击或感应电压和电流的间接
雷击）或电力系统切换（包括负载变化和短路切换）引起。这些瞬变通常发生在工业环境中，例如工厂自动化和
电网系统。

图 9-4 将 EFT 和浪涌瞬态的脉冲功率与 IEC ESD 瞬态功率进行了比较。左图显示了 0.5kV 浪涌瞬态和 4kV EFT
瞬态的相对脉冲功率，相比之下，左下角的 10kV ESD 瞬态不是很明显。500V 浪涌瞬态代表工业和过程自动化中
工厂环境中可能发生的事件。

右图显示了 6kV 浪涌瞬变相对于相同 0.5kV 浪涌瞬变的脉冲功率。6kV 浪涌瞬变最有可能发生在发电和电网系统
中。

图 9-4. ESD、EFT 和浪涌瞬态的功耗比较

在浪涌瞬变情形中，高能量内容的特点是脉冲持续时间长和脉冲功率衰减缓慢。转储到收发器内部保护单元的瞬
变电能被转换成热能，从而加热并破坏保护单元，进而损坏收发器。图 9-5 显示了单个 ESD、EFT、浪涌瞬变以
及合规性测试期间常用的 EFT 脉冲序列的瞬态能量差异很大。

Copyright © 2022 Texas Instruments Incorporated

Submit Document Feedback

17

Product Folder Links: THVD2410 THVD2450

R(C)R(D)C(S)High-VoltagePulseGeneratorDeviceUnderTestCurrent (A)4035302520151050Time (ns)05010015020025030010-kV IEC10-kV HBM330Ω(1.5 kΩ)150pF(100 pF)50 M(1 M)Pulse Power (kW)2220181614121086420Time (µs)05101520253035400.5-kV Surge10-kV ESD4-kV EFTPulse Power (MW)2.22.01.81.61.41.21.00.80.60.40.20Time (µs)05101520253035400.5-kV Surge6-kV Surge3.02.82.62.4THVD2410, THVD2450
ZHCSJZ4B – JULY 2019 – REVISED OCTOBER 2021

www.ti.com.cn

图 9-5. 瞬态能量的比较

18

Submit Document Feedback

Copyright © 2022 Texas Instruments Incorporated

Product Folder Links: THVD2410 THVD2450

1000.10.0110110-310-410-510-6Pulse Energy (J)0.51246810PeakPulseVoltage(kV)1000ESDEFTSurge15EFTPulseTrainwww.ti.com.cn

9.2.2 详细设计过程

THVD2410, THVD2450
ZHCSJZ4B – JULY 2019 – REVISED OCTOBER 2021

图 9-6  建议使用保护电路来抵御 1kV  浪涌 (IEC 61000-4-5)  瞬变。表 9-1  显示了相关的物料清单。SMAJ30CA
TVS 二极管的额定工作电压高达 30V。这可以确保在直接 RS-485 总线与 24V 直流工业电源轨短路时，保护二极
管不会导通。

图 9-6. 针对半双工器件的浪涌瞬变的瞬态保护

器件

收发器

TVS

功能

RS-485 收发器

双向 400W 瞬态抑制器

(1) 请参阅器件支持

表 9-1. 元件列表(1)
订货编号

THVD24x0

SMAJ30CA

制造商

TI

Littelfuse

Copyright © 2022 Texas Instruments Incorporated

Submit Document Feedback

19

Product Folder Links: THVD2410 THVD2450

3.3V ± 5 VRxDTxDDIRMCU/UART10k10k100nFDIR10kRDDE/REBAGNDVCCTHVD24x0TVSTVSTHVD2410, THVD2450
ZHCSJZ4B – JULY 2019 – REVISED OCTOBER 2021

9.2.3 应用曲线

www.ti.com.cn

THVD2410

VCC = 5V
500kbps 速率下的随机 (PRBS7) 数据

RL = 50Ω

THVD2410

VCC = 3.3V
500kbps 速率下的随机 (PRBS7) 数据

RL = 50Ω

图 9-7. VCC = 5V 时的 THVD2410 波形

图 9-8. VCC = 3.3V 时的 THVD2410 波形

THVD2450

VCC = 5V
50Mbps 速率下的随机 (PRBS7) 数据

RL = 50Ω

THVD2450

VCC = 3.3V
50Mbps 速率下的随机 (PRBS7) 数据

RL = 50Ω

图 9-9. VCC = 5V 时的 THVD2450 波形

图 9-10. VCC = 3.3V 时的 THVD2450 波形

10 电源相关建议

为确保在所有数据速率和电源电压下可靠运行，应使用 100nF 陶瓷电容对各个电源进行去耦，该电容的位置应尽
可能靠近电源引脚。这样有助于减少开关模式电源输出中出现的电源电压波纹，并且有助于补偿 PCB 电源层的电
阻和电感。

20

Submit Document Feedback

Copyright © 2022 Texas Instruments Incorporated

Product Folder Links: THVD2410 THVD2450

www.ti.com.cn

11 布局
11.1 布局指南

THVD2410, THVD2450
ZHCSJZ4B – JULY 2019 – REVISED OCTOBER 2021

稳健而可靠的总线节点设计通常需要使用外部瞬态保护器件，以抑制工业环境中可能出现的浪涌瞬变。这些瞬变
的频率带宽较宽（大概 3MHz 至 300MHz），因此在 PCB 设计过程中应该应用高频布局技术。
1. 将保护电路放置在靠近总线连接器的位置，以防止噪声瞬变在电路板上传播。
2. 使用 VCC 和接地平面来提供低电感。请注意，高频电流会选择阻抗最小的路径，而非电阻最小的路径。
3. 将保护元件设计成信号路径的方向。不得将瞬态电流从信号路径强行转移至保护器件。
4. 在尽可能靠近电路板上收发器、UART 和/或控制器 IC 的 VCC 引脚的位置应用 100nF 至 220nF 去耦电容器。
5. 当去耦电容器和保护器件连接 VCC 和地时，应至少使用两个过孔以更大限度减小实际过孔电感。
6. 使用 1kΩ 至 10kΩ 的上拉和下拉电阻用于使能线路，从而在瞬态事件期间限制这些线路中的噪声电流。
7. 如果 TVS 钳位电压高于收发器总线引脚的指定最大电压，则在 A 和 B 总线线路中插入防脉冲电阻器。这些电

阻器可限制进入收发器的剩余钳位电流并防止其锁存。

11.2 布局示例

图 11-1. 半双工布局示例

Copyright © 2022 Texas Instruments Incorporated

Submit Document Feedback

21

Product Folder Links: THVD2410 THVD2450

MCUTHVD24x0JMPRRRC564R56Via to groundVia to VCC1TVS51TVS5THVD2410, THVD2450
ZHCSJZ4B – JULY 2019 – REVISED OCTOBER 2021

12 器件和文档支持
12.1 器件支持
12.1.1 第三方产品免责声明

www.ti.com.cn

TI 发布的与第三方产品或服务有关的信息，不能构成与此类产品或服务或保修的适用性有关的认可，不能构成此
类产品或服务单独或与任何 TI 产品或服务一起的表示或认可。

12.2 接收文档更新通知

要接收文档更新通知，请导航至 ti.com 上的器件产品文件夹。点击订阅更新 进行注册，即可每周接收产品信息更
改摘要。有关更改的详细信息，请查看任何已修订文档中包含的修订历史记录。

12.3 支持资源

TI E2E™ 支持论坛是工程师的重要参考资料，可直接从专家获得快速、经过验证的解答和设计帮助。搜索现有解
答或提出自己的问题可获得所需的快速设计帮助。

链接的内容由各个贡献者“按原样”提供。这些内容并不构成 TI 技术规范，并且不一定反映 TI 的观点；请参阅
TI 的《使用条款》。

12.4 商标
TI E2E™ is a trademark of Texas Instruments.
所有商标均为其各自所有者的财产。
12.5 Electrostatic Discharge Caution

This integrated circuit can be damaged by ESD. Texas Instruments recommends that all integrated circuits be handled
with appropriate precautions. Failure to observe proper handling and installation procedures can cause damage.

ESD damage can range from subtle performance degradation to complete device failure. Precision integrated circuits may
be more susceptible to damage because very small parametric changes could cause the device not to meet its published
specifications.

12.6 术语表

TI 术语表

本术语表列出并解释了术语、首字母缩略词和定义。

机械、封装和可订购信息

下述页面包含机械、封装和订购信息。这些信息是指定器件可用的最新数据。数据如有变更，恕不另行通知，且
不会对此文档进行修订。有关此数据表的浏览器版本，请查阅左侧的导航栏。

22

Submit Document Feedback

Copyright © 2022 Texas Instruments Incorporated

Product Folder Links: THVD2410 THVD2450

www.ti.com

2-Nov-2022

PACKAGE OPTION ADDENDUM

PACKAGING INFORMATION

Orderable Device

Status

(1)

Package Type Package
Drawing

Pins Package

Eco Plan

Qty

(2)

Lead finish/
Ball material

(6)

MSL Peak Temp

Op Temp (°C)

Device Marking

Samples

(3)

(4/5)

THVD2410DGKR

ACTIVE

VSSOP

DGK

THVD2410DR

ACTIVE

SOIC

D

THVD2410DRBR

ACTIVE

SON

DRB

THVD2450DGKR

ACTIVE

VSSOP

DGK

THVD2450DR

ACTIVE

SOIC

D

THVD2450DRBR

ACTIVE

SON

DRB

8

8

8

8

8

8

2500 RoHS & Green

NIPDAUAG | SN

Level-1-260C-UNLIM

-40 to 125

2500 RoHS & Green

NIPDAU

Level-1-260C-UNLIM

-40 to 125

3000 RoHS & Green

NIPDAU

Level-1-260C-UNLIM

-40 to 125

2500 RoHS & Green

NIPDAUAG | SN

Level-1-260C-UNLIM

-40 to 125

2500 RoHS & Green

NIPDAU

Level-1-260C-UNLIM

-40 to 125

3000 RoHS & Green

NIPDAU

Level-1-260C-UNLIM

-40 to 125

2410

2410

2410

2450

2450

2450

Samples

Samples

Samples

Samples

Samples

Samples

(1) The marketing status values are defined as follows:
ACTIVE: Product device recommended for new designs.
LIFEBUY: TI has announced that the device will be discontinued, and a lifetime-buy period is in effect.
NRND: Not recommended for new designs. Device is in production to support existing customers, but TI does not recommend using this part in a new design.
PREVIEW: Device has been announced but is not in production. Samples may or may not be available.
OBSOLETE: TI has discontinued the production of the device.

(2) RoHS:  TI defines "RoHS" to mean semiconductor products that are compliant with the current EU RoHS requirements for all 10 RoHS substances, including the requirement that RoHS substance
do not exceed 0.1% by weight in homogeneous materials. Where designed to be soldered at high temperatures, "RoHS" products are suitable for use in specified lead-free processes. TI may
reference these types of products as "Pb-Free".
RoHS Exempt: TI defines "RoHS Exempt" to mean products that contain lead but are compliant with EU RoHS pursuant to a specific EU RoHS exemption.
Green: TI defines "Green" to mean the content of Chlorine (Cl) and Bromine (Br) based flame retardants meet JS709B low halogen requirements of <=1000ppm threshold. Antimony trioxide based
flame retardants must also meet the <=1000ppm threshold requirement.

(3) MSL, Peak Temp. - The Moisture Sensitivity Level rating according to the JEDEC industry standard classifications, and peak solder temperature.

(4) There may be additional marking, which relates to the logo, the lot trace code information, or the environmental category on the device.

(5) Multiple Device Markings will be inside parentheses. Only one Device Marking contained in parentheses and separated by a "~" will appear on a device. If a line is indented then it is a continuation
of the previous line and the two combined represent the entire Device Marking for that device.

(6) Lead finish/Ball material - Orderable Devices may have multiple material finish options. Finish options are separated by a vertical ruled line. Lead finish/Ball material values may wrap to two
lines if the finish value exceeds the maximum column width.

Addendum-Page 1

www.ti.com

2-Nov-2022

PACKAGE OPTION ADDENDUM

Important Information and Disclaimer:The information provided on this page represents TI's knowledge and belief as of the date that it is provided. TI bases its knowledge and belief on information
provided by third parties, and makes no representation or warranty as to the accuracy of such information. Efforts are underway to better integrate information from third parties. TI has taken and
continues to take reasonable steps to provide representative and accurate information but may not have conducted destructive testing or chemical analysis on incoming materials and chemicals.
TI and TI suppliers consider certain information to be proprietary, and thus CAS numbers and other limited information may not be available for release.

In no event shall TI's liability arising out of such information exceed the total purchase price of the TI part(s) at issue in this document sold by TI to Customer on an annual basis.

Addendum-Page 2

www.ti.com

4-Nov-2022

PACKAGE MATERIALS INFORMATION

TAPE AND REEL INFORMATION

REEL DIMENSIONS

TAPE DIMENSIONS

K0

 P1

B0 W

Reel
Diameter

Cavity

A0

A0
B0
K0
W
P1

Dimension designed to accommodate the component width
Dimension designed to accommodate the component length
Dimension designed to accommodate the component thickness
Overall width of the carrier tape
Pitch between successive cavity centers

Reel Width (W1)

QUADRANT ASSIGNMENTS FOR PIN 1 ORIENTATION IN TAPE

Sprocket Holes

Q1

Q2

Q1

Q2

Q3

Q4

Q3

Q4

User Direction of Feed

Pocket Quadrants

*All dimensions are nominal

Device

Package
Type

Package
Drawing

Pins

SPQ

Reel
Diameter
(mm)

Reel
Width
W1 (mm)

A0
(mm)

B0
(mm)

K0
(mm)

P1
(mm)

W
(mm)

Pin1
Quadrant

THVD2410DGKR

THVD2410DGKR

THVD2410DR

THVD2410DRBR

VSSOP

VSSOP

SOIC

SON

THVD2450DGKR

VSSOP

THVD2450DR

THVD2450DRBR

SOIC

SON

DGK

DGK

D

DRB

DGK

D

DRB

8

8

8

8

8

8

8

2500

2500

2500

3000

2500

2500

3000

330.0

330.0

330.0

330.0

330.0

330.0

330.0

12.4

12.4

12.4

12.4

12.4

12.4

12.4

5.3

5.3

6.4

3.3

5.3

6.4

3.3

3.4

3.4

5.2

3.3

3.4

5.2

3.3

1.4

1.4

2.1

1.1

1.4

2.1

1.1

8.0

8.0

8.0

8.0

8.0

8.0

8.0

12.0

12.0

12.0

12.0

12.0

12.0

12.0

Q1

Q1

Q1

Q2

Q1

Q1

Q2

Pack Materials-Page 1

PACKAGE MATERIALS INFORMATION

www.ti.com

4-Nov-2022

TAPE AND REEL BOX DIMENSIONS

Width (mm)

H

W

L

*All dimensions are nominal

Device

Package Type

Package Drawing Pins

THVD2410DGKR

THVD2410DGKR

THVD2410DR

THVD2410DRBR

VSSOP

VSSOP

SOIC

SON

THVD2450DGKR

VSSOP

THVD2450DR

THVD2450DRBR

SOIC

SON

DGK

DGK

D

DRB

DGK

D

DRB

8

8

8

8

8

8

8

SPQ

2500

2500

2500

3000

2500

2500

3000

Length (mm) Width (mm) Height (mm)

366.0

364.0

356.0

367.0

364.0

356.0

367.0

364.0

364.0

356.0

367.0

364.0

356.0

367.0

50.0

27.0

35.0

35.0

27.0

35.0

35.0

Pack Materials-Page 2

DRB0008B

SCALE  4.000

PACKAGE OUTLINE

VSON - 1 mm max height

PLASTIC SMALL OUTLINE - NO LEAD

A

3.1
2.9

PIN 1 INDEX AREA

B

3.1
2.9

1 MAX

0.05
0.00

C

SEATING PLANE

0.08 C

EXPOSED
THERMAL PAD

1.65 0.05

(0.2) TYP

2X
1.95

4

1

6X  0.65

PIN 1 ID
(OPTIONAL)

5

8

2.4 0.05

8X

0.5
0.3

8X

0.35
0.25
0.1
0.05

C A B

C

NOTES:

1. All linear dimensions are in millimeters. Any dimensions in parenthesis are for reference only. Dimensioning and tolerancing
    per ASME Y14.5M.
2. This drawing is subject to change without notice.
3. The package thermal pad must be soldered to the printed circuit board for thermal and mechanical performance.

4218876/A   12/2017

www.ti.com

DRB0008B

EXAMPLE BOARD LAYOUT

VSON - 1 mm max height

PLASTIC SMALL OUTLINE - NO LEAD

8X (0.6)

8X (0.3)

6X (0.65)

1

4

(R0.05) TYP

( 0.2) VIA
TYP

(1.65)

SYMM

8

5

(2.4)

(0.95)

(0.575)

(2.8)

LAND PATTERN EXAMPLE
SCALE:20X

0.07 MAX
ALL AROUND

0.07 MIN
ALL AROUND

SOLDER MASK
OPENING

METAL

METAL UNDER
SOLDER MASK

SOLDER MASK
OPENING

NON SOLDER MASK
DEFINED
(PREFERRED)

SOLDER MASK
DEFINED

SOLDER MASK DETAILS

NOTES: (continued)

4. This package is designed to be soldered to a thermal pad on the board. For more information, see Texas Instruments literature
    number SLUA271 (www.ti.com/lit/slua271).
5. Vias are optional depending on application, refer to device data sheet. If any vias are implemented, refer to their locations shown
    on this view. It is recommended that vias under paste be filled, plugged or tented.

4218876/A   12/2017

www.ti.com

DRB0008B

EXAMPLE STENCIL DESIGN

VSON - 1 mm max height

PLASTIC SMALL OUTLINE - NO LEAD

8X (0.6)

1

8X (0.3)

SYMM

6X (0.65)

4

(R0.05) TYP

SYMM

METAL
TYP

8

5

(0.63)

(1.06)

(1.47)

(2.8)

SOLDER PASTE EXAMPLE
BASED ON 0.125 mm THICK STENCIL

EXPOSED PAD
 81% PRINTED SOLDER COVERAGE BY AREA
SCALE:25X

NOTES: (continued)

6. Laser cutting apertures with trapezoidal walls and rounded corners may offer better paste release. IPC-7525 may have alternate
    design recommendations.

4218876/A   12/2017

www.ti.com

D0008A

A

.228-.244  TYP
[5.80-6.19]

PIN 1 ID AREA

.189-.197
[4.81-5.00]
NOTE 3

1

4

PACKAGE OUTLINE

SOIC - 1.75 mm max height

SMALL OUTLINE INTEGRATED CIRCUIT

C

SEATING PLANE

.004 [0.1] C

6X  .050

[1.27]

2X
.150
[3.81]

4X (0 -15 )

SCALE  2.800

8

5

B

.150-.157
[3.81-3.98]
NOTE 4

8X .012-.020
     [0.31-0.51]
.010 [0.25]

C A B

.069 MAX
[1.75]

4X (0 -15 )

 SEE DETAIL A

.005-.010  TYP
[0.13-0.25]

.010
[0.25]

0 - 8

.016-.050
[0.41-1.27]

.004-.010
[0.11-0.25]

DETAIL A
TYPICAL

(.041)
[1.04]

4214825/C   02/2019

NOTES:

1. Linear dimensions are in inches [millimeters]. Dimensions in parenthesis are for reference only. Controlling dimensions are in inches.
    Dimensioning and tolerancing per ASME Y14.5M.
2. This drawing is subject to change without notice.
3. This dimension does not include mold flash, protrusions, or gate burrs. Mold flash, protrusions, or gate burrs shall not
    exceed .006 [0.15] per side.
4. This dimension does not include interlead flash.
5. Reference JEDEC registration MS-012, variation AA.

www.ti.com

D0008A

EXAMPLE BOARD LAYOUT

SOIC - 1.75 mm max height

SMALL OUTLINE INTEGRATED CIRCUIT

8X (.061 )
[1.55]

8X (.024)
[0.6]

6X (.050 )
[1.27]

1

4

SYMM

SEE
DETAILS

8

5

SYMM

(R.002 ) TYP

[0.05]

(.213)
[5.4]

LAND PATTERN EXAMPLE
EXPOSED METAL SHOWN
SCALE:8X

METAL

SOLDER MASK
OPENING

SOLDER MASK
OPENING

METAL UNDER
SOLDER MASK

EXPOSED
METAL

.0028 MAX
[0.07]
ALL AROUND

NON SOLDER MASK
DEFINED

EXPOSED
METAL

.0028 MIN
[0.07]
ALL AROUND

SOLDER MASK
DEFINED

SOLDER MASK DETAILS

NOTES: (continued)

6. Publication IPC-7351 may have alternate designs.
7. Solder mask tolerances between and around signal pads can vary based on board fabrication site.

4214825/C   02/2019

www.ti.com

D0008A

EXAMPLE STENCIL DESIGN

SOIC - 1.75 mm max height

SMALL OUTLINE INTEGRATED CIRCUIT

8X (.061 )
[1.55]

SYMM

1

4

8X (.024)
[0.6]

6X (.050 )
[1.27]

8

5

SYMM

(R.002 ) TYP

[0.05]

(.213)
[5.4]

SOLDER PASTE EXAMPLE
BASED ON .005 INCH [0.125 MM] THICK STENCIL
SCALE:8X

NOTES: (continued)

8. Laser cutting apertures with trapezoidal walls and rounded corners may offer better paste release. IPC-7525 may have alternate
    design recommendations.
9. Board assembly site may have different recommendations for stencil design.

4214825/C   02/2019

www.ti.com

重要声明和免责声明
TI“按原样”提供技术和可靠性数据（包括数据表）、设计资源（包括参考设计）、应用或其他设计建议、网络工具、安全信息和其他资源，
不保证没有瑕疵且不做出任何明示或暗示的担保，包括但不限于对适销性、某特定用途方面的适用性或不侵犯任何第三方知识产权的暗示担
保。

这些资源可供使用 TI 产品进行设计的熟练开发人员使用。您将自行承担以下全部责任：(1) 针对您的应用选择合适的 TI 产品，(2) 设计、验
证并测试您的应用，(3) 确保您的应用满足相应标准以及任何其他功能安全、信息安全、监管或其他要求。

这些资源如有变更，恕不另行通知。TI 授权您仅可将这些资源用于研发本资源所述的 TI 产品的应用。严禁对这些资源进行其他复制或展示。
您无权使用任何其他 TI 知识产权或任何第三方知识产权。您应全额赔偿因在这些资源的使用中对 TI 及其代表造成的任何索赔、损害、成
本、损失和债务，TI 对此概不负责。

TI 提供的产品受 TI 的销售条款或 ti.com 上其他适用条款/TI 产品随附的其他适用条款的约束。TI 提供这些资源并不会扩展或以其他方式更改
TI 针对 TI 产品发布的适用的担保或担保免责声明。

TI 反对并拒绝您可能提出的任何其他或不同的条款。IMPORTANT NOTICE

邮寄地址：Texas Instruments, Post Office Box 655303, Dallas, Texas 75265
Copyright © 2022，德州仪器 (TI) 公司

