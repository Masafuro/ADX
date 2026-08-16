具有 4µA IQ 的 LMR36506 3V 至 65V、0.6A 超小型同步降压转换器

LMR36506
ZHCSKN9B – DECEMBER 2019 – REVISED DECEMBER 2022

1 特性

3 说明

LMR36506 是业界超小型 65V、0.6A 同步直流/直流降
压转换器，采用 2mm x 2mm HotRod 封装。这款易于
使用的转换器可处理高达 70V 的输入电压瞬变，提供
出色的 EMI 性能，并支持固定电压 3.3V 和其他可调输
出电压。这种瞬态容差降低了防止输入过压所需的设计
工作量，并满足 IEC 61000-4-5 的浪涌抗扰度要求。

LMR36506  采用具有内部补偿的峰值电流模式控制架
构，用于维持稳定运行和超小的输出电容。LMR36506
在 RT 引脚与地之间选用合适的电阻器后，可通过外部
编程在  200kHz  至  2.2MHz  范围内实现理想的开关频
率。借助精密 EN/UVLO 功能，可对器件启动和关断进
行精确控制。附带内置干扰滤波器和延迟释放功能的电
源正常状态标志可提供系统状态的真实指示，免去了使
用外部监控器的麻烦。LMR36506  紧凑的解决方案尺
寸和丰富的功能集简化了各种工业应用的实施。

器件型号

LMR36506

封装信息
封装(1)

RPE（VQFN-HR，
9）

封装尺寸（标称值）

2.00mm × 2.00mm

(1) 如需了解所有可用封装，请参阅数据表末尾的可订购产品附

录。

• 专用于条件严苛的工业应用：

– 结温范围：–40°C 至 +150°C
– 高达 70V 的输入瞬态保护
– 宽输入电压范围：3.0V（下降阈值）至 65V
– 低 EMI 和超低的开关节点振铃
– 提供可调的固定输出电压选项

• 适用于可扩展的工业电源：

– 与 LMR36503（65V、300mA）引脚兼容
– 可调开关频率：200kHz 至 2.2MHz（采用 RT

引脚型号时）

• 减小了解决方案尺寸并降低了成本：

– 功率密度超高，具有内部补偿并减少了外部组件

数量

– 具有可湿性侧面的超小型 2mm × 2mm HotRod

™ 封装

• 在整个负载范围内具有高效率和低功率耗散：

– 400kHz 时峰值效率为 93%（输入电压 12V 和

输出电压 3.3V 固定）

– 400kHz 时峰值效率为 90%（输入电压 24V 和

输出电压 3.3V 固定）
• 空载时工作静态电流很低

– 在 VIN 为 24V 和 VOUT 为 3.3V（固定输出选

项）时为 4µA

• 使用 TPSM365R6 模块缩短产品上市时间

2 应用

• 工厂自动化：现场发送器和过程传感器
• 楼宇自动化：HVAC 和防火安全
• 电器：园艺和电动工具

简化原理图

效率与输出电流间的关系 VOUT = 5V（固定值），
1MHz

本文档旨在为方便起见，提供有关 TI 产品中文版本的信息，以确认产品的概要。有关适用的官方英文版本的最新信息，请访问
www.ti.com，其内容始终优先。TI 不保证翻译的准确性和有效性。在实际设计之前，请务必参考最新版本的英文版本。

English Data Sheet: SNVSBB6

VIN VINEN/UVLOBOOTSWFBGNDVOUT CBOOTLINDCINCOUTRFBTRFBBVCC   PGOODCVCCRTLoad Current (A)010203040506070809010010P100P1m10m100mLMR3VIN = 12VVIN = 24VVIN = 36VVIN = 48VVIN = 54VLMR36506
ZHCSKN9B – DECEMBER 2019 – REVISED DECEMBER 2022

www.ti.com.cn

Table of Contents

1 特性................................................................................... 1
2 应用................................................................................... 1
3 说明................................................................................... 1
4 Revision History.............................................................. 2
5 Device Comparison Table...............................................3
6 Pin Configuration and Functions...................................4
7 Specifications.................................................................. 5
7.1 Absolute Maximum Ratings........................................ 5
7.2 ESD (Commercial) Ratings.........................................5
7.3 Recommended Operating Conditions.........................5
7.4 Thermal Information....................................................6
7.5 Electrical Characteristics.............................................6
7.6 Timing Characteristics.................................................8
7.7 Switching Characteristics............................................8
7.8 System Characteristics............................................... 8
7.9 Typical Characteristics.............................................. 10
8 Detailed Description...................................................... 11
8.1 Overview................................................................... 11

8.2 Functional Block Diagram......................................... 12
8.3 Feature Description...................................................13
8.4 Device Functional Modes..........................................22
9 Application and Implementation.................................. 28
9.1 Application Information............................................. 28
9.2 Typical Application.................................................... 29
9.3 Best Design Practices...............................................38
9.4 Power Supply Recommendations.............................38
9.5 Layout....................................................................... 38
10 Device and Documentation Support..........................41
10.1 Documentation Support.......................................... 41
10.2 接收文档更新通知................................................... 41
10.3 支持资源..................................................................41
10.4 Trademarks............................................................. 41
10.5 Electrostatic Discharge Caution..............................41
10.6 术语表..................................................................... 41

11 Mechanical, Packaging, and Orderable

Information.................................................................... 42

4 Revision History
注：以前版本的页码可能与当前版本的页码不同

Changes from Revision A (December 2020) to Revision B (December 2022)
Page
• 向特性 部分添加了 TPSM365R6 链接................................................................................................................ 1
• Changed ESD ratings descriptions.....................................................................................................................5
• Removed color from 图 8-2 and 图 9-19 ..........................................................................................................13

Page
Changes from Revision * (December 2019) to Revision A (December 2020)
• 将器件状态从“预告信息”更改为“量产数据”................................................................................................ 1
• 更新了整个文档中的表格、图和交叉参考的编号格式......................................................................................... 1

2

Submit Document Feedback

Copyright © 2022 Texas Instruments Incorporated

Product Folder Links: LMR36506

www.ti.com.cn

5 Device Comparison Table

LMR36506
ZHCSKN9B – DECEMBER 2019 – REVISED DECEMBER 2022

ORDERABLE PART
NUMBER

OUTPUT VOLTAGE

EXTERNAL SYNC

FSW

SPREAD SPECTRUM

LMR36506RFRPER

Adjustable

LMR36506R3RPER

3.3-V Fixed

LMR36506RF3RPER

3.3-V Fixed

LMR36506RRPER

Adjustable

LMR36506R5RPER

5-V Fixed

No
(Default FPWM at light
load)

Adjustable
with RT resistor

No
(Default PFM at light load)

Adjustable
with RT resistor

No
(Default FPWM at light
load)

No
(Default PFM at light load)

No
(Default PFM at light load)

Adjustable
with RT resistor

Adjustable
with RT resistor

Adjustable
with RT resistor

No

No

No

No

No

Copyright © 2022 Texas Instruments Incorporated

Submit Document Feedback

3

Product Folder Links: LMR36506

LMR36506
ZHCSKN9B – DECEMBER 2019 – REVISED DECEMBER 2022

6 Pin Configuration and Functions

www.ti.com.cn

图 6-1. RPE Package 9-Pin (2 mm × 2 mm) VQFN-HR Top View

PIN

NO.

NAME

I/O

表 6-1. Pin Functions

DESCRIPTION

1

2

3

4

5

6

7

8

9

RT

PGOOD

EN/UVLO

VIN

SW

BOOT

VCC

VOUT/BIAS or FB

GND

A

A

A

P

P

P

P

A

G

When part is trimmed as the RT variant, the switching frequency can be adjusted from 200 kHz to
2.2 MHz. Do not float this pin.

Open-drain power-good flag output. Connect to suitable voltage supply through a current limiting
resistor. High = power OK, low = power bad. this pin goes low when EN = low. This pin can be open
or grounded when not used.

Enable input to regulator. High = ON, low = OFF. Can be connected directly to VIN. Do not float this
pin.

Input supply to regulator. Connect a high-quality bypass capacitor or capacitors directly to this pin
and GND.

Regulator switch node. Connect to power inductor.

Bootstrap supply voltage for internal high-side driver. Connect a high-quality 100-nF capacitor from
this pin to the SW pin.

Internal LDO output. Used as supply to internal control circuits. Do not connect to external loads.
Can be used as logic supply for power-good flag. Connect a high-quality 1-µF capacitor from this
pin to GND.

Fixed output options are available with the VOUT/BIAS pin variant. Connect to output voltage node
for fixed VOUT. Check Device Comparison Table for more details.
The FB pin variant can help adjust the output voltage. Connect to tap point of feedback voltage
divider. Do not float this pin.

Power ground terminal. Connect to system ground. Connect to CIN with short, wide traces.

A = Analog, P = Power, G = Ground

4

Submit Document Feedback

Copyright © 2022 Texas Instruments Incorporated

Product Folder Links: LMR36506

SWPGOODRTVINGNDBOOTEN/UVLOVCC412358769 VOUT/BIASSWPGOODRTVINGNDBOOTEN/UVLOVCC412358769FBwww.ti.com.cn

LMR36506
ZHCSKN9B – DECEMBER 2019 – REVISED DECEMBER 2022

7 Specifications
7.1 Absolute Maximum Ratings
Over the recommended operating junction temperature range(1)

PARAMETER

VIN to GND

EN to GND

SW to GND

PGOOD to GND

Voltage

VOUT/BIAS to GND (Fixed output)

FB to GND - (Adjustable output)

BOOT to SW

VCC to GND

RT to GND (RT variant)

MODE/SYNC to GND (MODE/SYNC variant)

Junction temperature

Storage temperature

TJ

Tstg

(1)

MIN

–0.3

–0.3

–0.3

0

–0.3

–0.3

–0.3

–0.3

–0.3

–0.3

–40

–65

MAX

UNIT

70

70

70.3

20

16

16

5.5

5.5

5.5

5.5

150

150

V

V

V

V

V

V

V

V

V

V

°C

°C

Stresses beyond those listed under Absolute Maximum Ratings may cause permanent damage to the device. These are stress ratings
only, which do not imply functional operation of the device at these or any other conditions beyond those indicated
under Recommended Operating Conditions. Exposure to absolute-maximum-rated conditions for extended periods may affect device
reliability.

7.2 ESD (Commercial) Ratings

V(ESD)

Electrostatic discharge

Human-body model (HBM), per ANSI/ESDA/
JEDEC JS-001(1)

Charged-device model (CDM), per ANSI/ESDA/
JEDEC JS-002(2)

VALUE

UNIT

±2000

±750

V

V

(1)
(2)

JEDEC document JEP155 states that 500-V HBM allows safe manufacturing with a standard ESD control process
JEDEC document JEP157 states that 250-V CDM allows safe manufacturing with a standard ESD control process.

7.3 Recommended Operating Conditions
Over the recommended operating junction temperature range of –40 °C to 150 °C (unless otherwise noted)(1) (2)
MAX

TYP

MIN

Input
voltage

Output
current

Frequency
setting

Input voltage range after startup

Load current range(3)

Selectable frequency range with RT (RT variant only)

Set frequency value with RT connected to GND (RT variant only)

Set frequency value with RT connected to VCC (RT variant only)

3.6

0

0.2

65

0.6

2.2

2.2

1

UNIT

V

A

MHz

MHz

MHz

(1) Recommended operating conditions indicate conditions for which the device is intended to be functional, but do not ensure specific

performance limits. For ensured specifications, see Electrical Characteristics table.

(2) High junction temperatures degrade operating lifetimes. Operating lifetime is de-rated for junction temperatures greater than 125℃
(3) Maximum continuous DC current may be derated when operating with high switching frequency or high ambient temperature. See

Application section for details.

Copyright © 2022 Texas Instruments Incorporated

Submit Document Feedback

5

Product Folder Links: LMR36506

LMR36506
ZHCSKN9B – DECEMBER 2019 – REVISED DECEMBER 2022

www.ti.com.cn

7.4 Thermal Information
The value of RθJA given in this table is only valid for comparison with other packages and cannot be used for design
purposes. These values were calculated in accordance with JESD 51-7, and simulated on a 4-layer JEDEC board. They do
not represent the performance obtained in an actual application. For example, with a 4-layer PCB, a RθJA= 58℃/W can be
achieved

THERMAL METRIC(1)

RθJA

RθJC(top)

Junction-to-ambient thermal resistance

Junction-to-case (top) thermal resistance

Junction-to-board thermal resistance

Junction-to-top characterization parameter

Junction-to-board characterization parameter

RθJB

ΨJT

ΨJB

(1)

LMR36506

VQFN (RPE)

9 Pins

84.4

47.5

26.1

0.9

25.9

UNIT

°C/W

°C/W

°C/W

°C/W

°C/W

For more information about traditional and new thermal metrics, see the Semiconductor and IC Package Thermal Metrics application
report.  The value of RΘJA given in this table is only valid for comparison with other packages and can not be used for design
purposes. This value was calculated in accordance with JESD 51-7, and simulated on a 4-layer JEDEC board. It does not represent
the performance obtained in an actual application. For design information see the  Maximum Ambient Temperature section.

7.5 Electrical Characteristics

Limits apply over the recommended operating junction temperature (TJ) range of –40°C to +150°C, unless otherwise stated.
Minimum and maximum limits are specified through test, design or statistical correlation. Typical values represent the most
likely parametric norm at TJ = 25°C, and are provided for reference purposes only. Unless otherwise stated, the following
conditions apply: VIN = 24 V.(1)
PARAMETER

TEST CONDITIONS

UNIT

MAX

TYP

MIN

SUPPLY VOLTAGE (VIN PIN)

Minimum operating input voltage
(rising)

Minimum operating input voltage
(falling)

Rising threshold

Once operating; Falling threshold

2.45

3.4

3.0

3.5

V

V

VIN_R

VIN_F

IQ_13p5_Fixed

IQ_13p5_Adj

IQ_24p0_Fixed

IQ_24p0_Adj

IB_13p5

IB_24p0

ISD_13p5

ISD_24p0

Non-switching input current;
measured at VIN pin(2)

VIN = VEN = 13.5V ; VOUT/BIAS = 5.25V,
VRT = 0V; Fixed output

Non-switching input current;
measured at VIN pin(2)

VIN = VEN = 13.5V ; VFB = 1.05V, VRT =
0V; Adjustable output

Non-switching input current;
measured at VIN pin(2)

VIN = VEN = 24V ; VOUT/BIAS = 5.25V, VRT
= 0V; Fixed output

Non-switching input current;
measured at VIN pin(2)

VIN = VEN = 24V ; VFB = 1.05V, VRT = 0V;
Adjustable output

Current into VOUT/BIAS pin (not
switching)(2)

VIN = 13.5V, VOUT/BIAS = 5.25V, VRT = 0V;
Fixed output

Current into VOUT/BIAS pin (not
switching)(2)

VIN = 24V, VOUT/BIAS = 5.25V, VRT = 0V;
Fixed output

Shutdown quiescent current;
measured at VIN pin(2)

Shutdown quiescent current;
measured at VIN pin(2)

VEN = 0; VIN = 13.5V

VEN = 0; VIN = 24V

ENABLE (EN PIN)

VEN-WAKE

VEN-VOUT

VEN-HYST

ILKG-EN

INTERNAL LDO

Enable wake-up threshold

Precision enable high level for
VOUT

Enable threshold hysteresis below
VEN- VOUT

0.25

0.672

1.05

µA

17

1.2

18

17

18

22

µA

1.7

µA

22

µA

22

µA

22

µA

0.5

1.1

µA

1

1.6

µA

14

0.8

14

14

14

0.4

1.16

1.263

1.36

0.3

0.35

0.4

V

V

V

Enable input leakage current

VEN = 3.3 V

0.3

8

nA

6

Submit Document Feedback

Copyright © 2022 Texas Instruments Incorporated

Product Folder Links: LMR36506

www.ti.com.cn

7.5 Electrical Characteristics (continued)

LMR36506
ZHCSKN9B – DECEMBER 2019 – REVISED DECEMBER 2022

Limits apply over the recommended operating junction temperature (TJ) range of –40°C to +150°C, unless otherwise stated.
Minimum and maximum limits are specified through test, design or statistical correlation. Typical values represent the most
likely parametric norm at TJ = 25°C, and are provided for reference purposes only. Unless otherwise stated, the following
conditions apply: VIN = 24 V.(1)
PARAMETER

TEST CONDITIONS

UNIT

MAX

TYP

MIN

VCC

ICC

Bias regulator current limit

Internal VCC voltage

Adjustable or fixed  output;  Auto mode

3.125

3.15

VCC-UVLO

Internal VCC undervoltage lockout VCC rising under voltage threshold

VCC-UVLO-HYST

Internal VCC under voltage lock-
out hysteresis

Hysteresis below VCC-UVLO

CURRENT LIMITS

ISC-0p3

Short circuit high side current
Limit(3)

ILS-LIMIT-0p3

Low side current limit(3)

0.3A Version

0.3A Version

IPEAK-MIN-0p3

Minimum peak inductor current(3)

PFM Operation, 0.3A Version; Duty
Factor = 0

ISC-0p6

ILS-LIMIT-0p6

IPEAK-MIN-0p6

IZC

IL-NEG

POWER GOOD

Short circuit high side current
Limit(3)

Low side current limit(3)

Minimum Peak Inductor Current(3) Auto Mode, duty factor = 0

Zero cross current(3)

Auto mode

Sink current limit (negative)(3)

FPWM mode

PG-OV

PGOOD upper threshold - rising

PG-UV

PGOOD lower threshold - falling

PG-HYS

PGOOD hysteresis - rising/falling

% of FB (Adjustable output) or % of
VOUT/BIAS (Fixed output)

% of FB (Adjustable output) or % of
VOUT/BIAS (Fixed output)

% of FB (Adjustable output) or % of
VOUT/BIAS (Fixed output)

VPG-VALID

RPG-EN5p0

RPG-EN0

MOSFETS

RDS-ON-HS

RDS-ON-LS

Minimum input voltage for proper
PG function

RDS(ON) PGOOD output

RDS(ON) PGOOD output

VEN = 5.0V, 1mA pull-up current

VEN = 0 V, 1mA pull-up current

High-side MOSFET on-resistance

Load = 0.3 A

Low-side MOSFET on-resistance

Load = 0.3 A

VCBOOT-UVLO

Cboot - SW UVLO threshold(4)

VOLTAGE REFERENCE

VOUT_Fixed3p3

Initial VOUT voltage accuracy for
3.3 V

FPWM mode

VOUT_Fixed5p0

Initial VOUT voltage accuracy for 5
V

FPWM mode

VREF

IFB

Internal reference voltage

VIN = 3.6V to 65V, FPWM mode

FB input current

Adjustable output, FB = 1V

3.22

240

3.65

1.2

65

3.3

0.8

0.5

0.575

0.35

0.09

0.4

0.11

1

1.11

0.7

0.19

0.01

0.7

0.752

0.227

0.022

0.8

3

0.4

0.42

0.3

0.067

0.87

0.6

0.127

0

0.6

106

107

110

93

1.3

0.75

20

10

2.14

94

96.5

1.8

2.3

1

40

18

560

280

2.3

2

70

31

920

460

2.42

3.25

3.3

3.34

4.93

0.985

5

1

85

5.07

1.01

110

V

mA

V

V

A

A

A

A

A

A

A

A

%

%

%

V

Ω

Ω

mΩ

mΩ

V

V

V

V

nA

(1) MIN and MAX limits are 100% production tested at 25ºC. Limits over the operating temperature range verified through correlation

using Statistical Quality Control (SQC) methods. Limits are used to calculate Average Outgoing Quality Level (AOQL).
This is the current used by the device open loop. It does not represent the total input current of the system when in regulation.
The current limit values in this table are tested, open loop, in production. They may differ from those found in a closed loop application.

(2)
(3)
(4) When the voltage across the CBOOT capacitor falls below this voltage, the low side MOSFET is turn to recharge the boot capacitor

Copyright © 2022 Texas Instruments Incorporated

Submit Document Feedback

7

Product Folder Links: LMR36506

LMR36506
ZHCSKN9B – DECEMBER 2019 – REVISED DECEMBER 2022

www.ti.com.cn

7.6 Timing Characteristics
Limits apply over the recommended operating junction temperature (TJ) range of –40°C to +150°C, unless otherwise stated.
Minimum and maximum limits are specified through test, design or statistical correlation. Typical values represent the most
likely parametric norm at TJ = 25°C, and are provided for reference purposes only. Unless otherwise stated, the following
conditions apply: VIN = 24 V.(1)
PARAMETER

TEST CONDITIONS

UNIT

MAX

TYP

MIN

SOFT START

tSS

Time from first SW pulse to VFB at
90%, of VREF

VIN ≥ 3.6 V

POWER GOOD

tRESET_FILTER

Glitch filter time constant for PG
function

tPGOOD_ACT

Delay time to PG high signal

1.95

2.58

3.2

ms

15

1.7

25

40

µs

1.956

2.16

ms

(1) MIN and MAX limits are 100% production tested at 25°C. Limits over the operating temperature range are verified through correlation

usingStatistical Quality Control (SQC) methods. Limits are used to calculate Average Outgoing Quality Level (AOQL).

7.7 Switching Characteristics
Limits apply over the recommended operating junction temperature (TJ) range of –40°C to +150°C, unless otherwise stated.
Minimum and maximum limits are specified through test, design or statistical correlation. Typical values represent the most
likely parametric norm at TJ = 25°C, and are provided for reference purposes only. Unless otherwise stated, the following
conditions apply: VIN = 24 V.(1)
PARAMETER

TEST CONDITIONS

UNIT

MAX

TYP

MIN

PWM LIMITS (SW)

tON-MIN

tOFF-MIN

tON-MAX

Minimum switch on-time

VIN =24 V, IOUT = 0.6 A

Minimum switch off-time

Maximum switch on-time

HS timeout in dropout

OSCILLATOR (RT)

fOSC_2p2MHz

fOSC_1p0MHz

fADJ_400kHz

Internal oscillator frequency

Internal oscillator frequency

RT = GND

RT = VCC

RT = 39.2 kΩ

40

40

7.6

2.1

0.93

0.34

57

58

9

2.2

1

0.4

80

77

9.8

ns

ns

µs

2.3 MHz

1.05 MHz

0.46 MHz

(1) MIN and MAX limits are 100% production tested at 25°C. Limits over the operating temperature range are verified through correlation

usingStatistical Quality Control (SQC) methods. Limits are used to calculate Average Outgoing Quality Level (AOQL).

7.8 System Characteristics

The following specifications apply only to the typical applications circuit, with nominal component values. Specifications in the
typical (TYP) column apply to TJ = 25°C only. Specifications in the minimum (MIN) and maximum (MAX) columns apply to the
case of typical components over the temperature range of TJ = –40°C to 150°C. These specifications are not ensured by
production testing.

PARAMETER

TEST CONDITIONS

MIN

TYP

MAX

UNIT

STANDBY CURRENT AND DUTY RATIO

ISUPPLY

ISUPPLY

DMAX

Input supply current when in
regulation

VIN  = 13.5  V,  VOUT/BIAS  = 3.3 V,  IOUT =
0 A, PFM mode

Input supply current when in
regulation

VIN  = 24  V,  VOUT/BIAS  = 3.3 V,  IOUT = 0
A, PFM mode

Maximum switch duty cycle(1)

6.5

4

98%

OUTPUT VOLTAGE ACCURACY (VOUT/BIAS)

VOUT_3p3V_ACC

VOUT_3p3V_ACC

VOUT = 3.3 V, VIN = 3.6 V to 65 V,
IOUT = 0 to full load(2)
VOUT = 3.3 V, VIN = 3.6V to 65 V,
IOUT = 0 A to full load(2)

FPWM mode

Auto mode

THERMAL SHUTDOWN

–1.5

–1.5

1.5

2.5

µA

µA

%

%

8

Submit Document Feedback

Copyright © 2022 Texas Instruments Incorporated

Product Folder Links: LMR36506

www.ti.com.cn

7.8 System Characteristics (continued)

LMR36506
ZHCSKN9B – DECEMBER 2019 – REVISED DECEMBER 2022

The following specifications apply only to the typical applications circuit, with nominal component values. Specifications in the
typical (TYP) column apply to TJ = 25°C only. Specifications in the minimum (MIN) and maximum (MAX) columns apply to the
case of typical components over the temperature range of TJ = –40°C to 150°C. These specifications are not ensured by
production testing.

PARAMETER

TEST CONDITIONS

TSD-R

TSD-F

TSD-HYS

Thermal shutdown rising

Shutdown threshold

Thermal shutdown falling

Recovery threshold

Thermal shutdown hysteresis

MIN

158

150

8

TYP

MAX

UNIT

168

158

10

180

165

15

°C

°C

°C

(1)

In dropout the switching frequency drops to increase the effective duty cycle. The lowest frequency is clamped at approximately: fMIN =
1 / (tON-MAX + TOFF-MIN). DMAX = tON-MAX /(tON-MAX + tOFF-MIN).

(2) Deviation is with respect to VIN =13.5 V

Copyright © 2022 Texas Instruments Incorporated

Submit Document Feedback

9

Product Folder Links: LMR36506

LMR36506
ZHCSKN9B – DECEMBER 2019 – REVISED DECEMBER 2022

7.9 Typical Characteristics

Unless otherwise specified, the following conditions apply: TA = 25°C, VIN = 13.5 V.

www.ti.com.cn

VOUT = 3.3 V Fixed

FSW = 1 MHz (FPWM)

VOUT = 3.3 V Fixed

FSW = 1 MHz (Auto)

图 7-1. Efficiency 3.3-V Output, FPWM

图 7-2. Efficiency 3.3-V Output, Auto Mode

VOUT = 5 V Fixed

FSW-NOM = 400 kHz (Auto)

VOUT = 5 V Fixed

FSW-NOM = 1 MHz (Auto)

图 7-3. Efficiency 5-V Output, Auto Mode

图 7-4. Efficiency 5-V Output, Auto Mode

图 7-5. Typical Input Supply Current at No Load for
Fixed 3.3-V and 5-V Output

图 7-6. Typical Start-up and Shutdown at VOUT = 5 V

10

Submit Document Feedback

Copyright © 2022 Texas Instruments Incorporated

Product Folder Links: LMR36506

Load Current (A)Efficiency (%)01020304050607080901001m10m100m1LMR3VIN = 12VVIN = 24VVIN = 36VVIN = 48VVIN = 54VLoad Current (A)Efficiency (%)010203040506070809010010P100P1m10m100m1LMR3VIN = 12VVIN = 24VVIN = 36VVIN = 48VVIN = 54VLoad Current (A)Efficiency (%)010203040506070809010010P100P1m10m100m1LMR3VIN = 12VVIN = 24VVIN = 36VVIN = 48VVIN = 54VLoad Current (A)010203040506070809010010P100P1m10m100mLMR3VIN = 12VVIN = 24VVIN = 36VVIN = 48VVIN = 54VInput Voltage (V)Quiescent Current (PA)510152025303540455055606524681012141618Inpu3.3V5V50ms/DIVVIN (1V/DIV) VOUT (1V/DIV) IOUT (200mA/DIV)

www.ti.com.cn

8 Detailed Description
8.1 Overview

LMR36506
ZHCSKN9B – DECEMBER 2019 – REVISED DECEMBER 2022

The LMR36506 is a wide input, low-quiescent current, high-performance regulator that can operate over a wide
range of duty ratio and the switching frequencies, including sub-AM band at 400 kHz and above AM band at 2.2
MHz. During wide input transients, if the minimum ON-time or the minimum OFF-time cannot support the desired
duty ratio at the higher switching frequency settings, the switching frequency is reduced automatically, allowing
the  LMR36506  to  maintain  the  output  voltage  regulation.  With  an  internally-compensated  design  optimized  for
minimal output capacitors, the system design process with the LMR36506 is simplified significantly compared to
other buck regulators available in the market.

The  LMR36506  is  designed  to  minimize  external  component  cost  and  solution  size  while  operating  in  all
demanding industrial environments. The LMR36506 family includes variants that can be set-up to operate over a
wide  switching  frequency  range,  from  200  kHz  to  2.2  MHz,  with  the  correct  resistor  selection  from  RT  pin  to
ground.  To  further  reduce  system  cost,  the  PGOOD  output  feature  with  built-in  delayed  release  allows  the
elimination of the reset supervisor in many applications.

The  LMR36506  comes  in  an  ultra-small  2-mm  x  2-mm  QFN  package  with  wettable  flanks  allowing  for  quick
optical inspection along with specially designed corner anchor pins for reliable board level solder connections.

Copyright © 2022 Texas Instruments Incorporated

Submit Document Feedback

11

Product Folder Links: LMR36506

LMR36506
ZHCSKN9B – DECEMBER 2019 – REVISED DECEMBER 2022

8.2 Functional Block Diagram

www.ti.com.cn

12

Submit Document Feedback

Copyright © 2022 Texas Instruments Incorporated

Product Folder Links: LMR36506

PGOODPGOOD LOGICFBOSCILLATORENCONTROL LOGIC &DRIVERENABLEGNDSWVINLDO+±MAX. & MIN. LIMITSERROR AMPLFIER+±+HS CURRENT SENSESLOPECOMPENSATIONVREFVCC+±±+HS CURRENT LMITCLOCKFSW FOLDBACKMIN. LS CURRENT LIMIT+±THERMAL SHUTDOWNCLOCKCOMPSOFT-START &BANDGAP SYS ENABLEVCC UVLOVCC UVLOGNDVINFPWM or AUTORTVOUT/BIASLS CURRENT LMIT SYS ENABLETSDTSDTSDLS CURRENT SENSEVOUT UV/OVVOUT UV/OVADJ. OUTPUT VOLTAGE VARIANTS ONLYFIXED OUTPUT VOLTAGE VARIANTS ONLYFIXED OUTPUT VOLTAGE VARIANTS ONLYADJ. OUTPUT VOLTAGE VARIANTS ONLYFIXED OUTPUT VOLTAGE VARIANTS ONLYVOUT/BIASFBVOUT/BIAS SYS ENABLEBOOTwww.ti.com.cn

8.3 Feature Description
8.3.1 Enable, Start-up, and Shutdown

LMR36506
ZHCSKN9B – DECEMBER 2019 – REVISED DECEMBER 2022

Voltage  at  the  EN  pin  controls  the  start-up  or  remote  shutdown  of  the  LMR36506  family  of  devices.  The  part
stays  shut  down  as  long  as  the  EN  pin  voltage  is  less  than  VEN-WAKE  =  0.4  V.  During  the  shutdown,  the  input
current drawn by the device typically drops down to 0.5 µA (VIN = 13.5 V). With the voltage at the EN pin greater
than the VEN-WAKE, the device enters the device standby mode, the internal LDO powers up to generate VCC. As
the EN voltage increases further, approaching VEN-VOUT, the device finally starts to switch, entering the start-up
mode,  with  a  soft  start.  During  the  device  shutdown  process,  when  the  EN  input  voltage  measures  less  than
(VEN-VOUT  –  VEN-HYST),  the  regulator  stops  switching  and  re-enters  the  device  standby  mode.  Any  further
decrease in the EN pin voltage, below VEN-WAKE, the device is then firmly shut down. The high-voltage compliant
EN  input  pin  can  be  connected  directly  to  the  VIN  input  pin  if  remote  precision  control  is  not  needed.  The  EN
input pin must not be allowed to float. The various EN threshold parameters and their values are listed in 节 7.5.
图   8-2  shows  the  precision  enable  behavior.  图   8-3  shows  a  typical  remote  EN  start-up  waveform  in  an
application. After EN goes high, after a delay of about 1 ms, the output voltage begins to rise with a soft start and
reaches close to the final value in about 2.67 ms (tss). After a delay of about 2 ms (tPGOOD_ACT), the PGOOD flag
goes high. During start-up, the device is not allowed to enter FPWM mode until the soft-start time has elapsed.
This time is measured from the rising edge of EN. Check 节 9.2.2.8.1 for component selection.

图 8-1. VIN UVLO Using the EN Pin

Copyright © 2022 Texas Instruments Incorporated

Submit Document Feedback

13

Product Folder Links: LMR36506

RENTRENBENAGNDVINLMR36506
ZHCSKN9B – DECEMBER 2019 – REVISED DECEMBER 2022

www.ti.com.cn

图 8-2. Precision Enable Behavior

图 8-3. Enable Start-up VIN = 12 V, VOUT = 5 V, IOUT = 600 mA

8.3.2 Adjustable Switching Frequency (with RT)

The  select  variants  in  the  LMR36506  family  with  the  RT  pin  allow  the  power  designers  to  set  any  desired
operating  frequency  between  200  kHz  and  2.2  MHz  in  their  applications.  See  图  8-4  to  determine  the  resistor
value needed for the desired switching frequency. See 表 8-1 for selection on programming the RT pin.

RT INPUT

VCC

GND

RT to GND

表 8-1. RT Pin Setting

SWITCHING FREQUENCY

1 MHz

2.2 MHz

Adjustable according to 图 8-4

14

Submit Document Feedback

Copyright © 2022 Texas Instruments Incorporated

Product Folder Links: LMR36506

ENVCC0VEN-VOUT3.3VVOUTVOUT0VEN-WAKEVEN-HYSTEN (2V/DIV)1ms/DIVVIN (5V/DIV) VOUT (5V/DIV) PGOOD (5V/DIV)IOUT (1A/DIV)www.ti.com.cn

LMR36506
ZHCSKN9B – DECEMBER 2019 – REVISED DECEMBER 2022

RT INPUT

Float (Not Recommended)

SWITCHING FREQUENCY

No Switching

表 8-1. RT Pin Setting (continued)

方程式 1 can be used to calculate the value of RT for a desired frequency.

(1)

where
• RT is the frequency setting resistor value (kΩ).
• FSW is the switching frequency (kHz).

8.3.3 Power-Good Output Operation

图 8-4. RT Values vs Frequency

The  power-good  feature  using  the  PG  pin  of  the  LMR36506  can  be  used  to  reset  a  system  microprocessor
whenever  the  output  voltage  is  out  of  regulation.  This  open-drain  output  remains  low  under  device  fault
conditions, such as current limit and thermal shutdown, as well as during normal start-up. A glitch filter prevents
false  flag  operation  for  any  short  duration  excursions  in  the  output  voltage,  such  as  during  line  and  load
transients. Output voltage excursions lasting less than tRESET_FILTER do not trip the power-good flag. Power-good
operation can best be understood in reference to  图 8-5. 表 8-2 gives a more detailed breakdown the PGOOD
operation. Here, VPG-UV is defined as the PG-UV scaled version of the VOUT-Reg (target regulated output voltage)
and VPG-HYS as the PG-HYS scaled version of the VOUT-Reg, where both PG-UV and PG-HYS are listed in 节 7.5.
During the initial power up, a total delay of 5 ms (typ.) is encountered from the time the VEN-VOUT is triggered to
the  time  that  the  power-good  is  flagged  high.  This  delay  only  occurs  during  the  device  start-up  and  is  not
encountered during any other normal operation of the power-good function. When EN is pulled low, the power-
good  flag  output  is  also  forced  low.  With  EN  low,  power-good  remains  valid  as  long  as  the  input  voltage
(VPG-VALID is ≥ 1 V (typical).

The power-good output scheme consists of an open-drain n-channel MOSFET, which requires an external pullup
resistor connected to a suitable logic supply. The power-good output scheme can also be pulled up to either VCC
or VOUT through an appropriate resistor, as desired. If this function is not needed, the PGOOD pin can be open
or grounded. Limit the current into this pin to ≤ 4 mA.

Copyright © 2022 Texas Instruments Incorporated

Submit Document Feedback

15

Product Folder Links: LMR36506

RT =18286Fsw1.021Switching Frequency (kHz)RT resistor (k:)200400600800100012001400160018002000220001020304050607080RTvsLMR36506
ZHCSKN9B – DECEMBER 2019 – REVISED DECEMBER 2022

www.ti.com.cn

图 8-5. Power-Good Operation (OV Events Not Included)

表 8-2. Fault Conditions for PGOOD (Pull Low)

FAULT CONDITION INITIATED

VOUT < VPG-UV AND t > tRESET_FILTER

VOUT > VPG-OV AND t > tRESET_FILTER

TJ > TSD-R

EN < VEN-VOUT - VEN-HYST

VCC < VCC-UVLO - VCC-UVLO-HYST

FAULT CONDITION ENDS (AFTER WHICH tPGOOD_ACT MUST PASS
BEFORE PGOOD OUTPUT IS RELEASED)

Output voltage in regulation:
VPG-UV + VPG-HYS < VOUT < VPG-OV - VPG-HYS

Output voltage in regulation

TJ < TSD-F AND output voltage in regulation

EN > VEN-VOUT AND output voltage in regulation

VCC > VCC-UVLO AND output voltage in regulation

8.3.4 Internal LDO, VCC UVLO, and VOUT/BIAS Input

The LMR36506 uses the internal LDO output and the VCC pin for all internal power supply. The VCC pin draws
power either from the VIN (in adjustable output variants) or the VOUT/BIAS (in fixed-output variants). In the fixed
output variants, after the LMR36506 is active but has yet to regulate, the VCC rail continues to draw power from
the input voltage, VIN, until the VOUT/BIAS voltage reaches > 3.15 V (or when the device has reached steady-
state regulation post the soft start). The VCC rail typically measures 3.15 V in both adjustable and fixed output
variants. To prevent unsafe operation, VCC has an undervoltage lockout, which prevents switching if the internal
voltage is too low. See VVCC-UVLO and VVCC-UVLO-HYST in 节 7.5. During start-up, VCC momentarily exceeds the
normal  operating  voltage  until  VVCC-UVLO  is  exceeded,  then  drops  to  the  normal  operating  voltage.  Note  that
these  undervoltage  lockout  values,  when  combined  with  the  LDO  dropout,  drives  the  minimum  input  voltage
rising and falling thresholds.

8.3.5 Bootstrap Voltage and VCBOOT-UVLO (CBOOT Terminal)

The high-side switch driver circuit requires a bias voltage higher than VIN to ensure the HS switch is turned ON.
The  capacitor  connected  between  CBOOT  and  SW  works  as  a  charge  pump  to  boost  voltage  on  the  CBOOT
terminal to (SW+VCC). The boot diode is integrated on the LMR36506 die to minimize physical solution size. A
100-nF capacitor rated for 10 V or higher is recommended for CBOOT. The CBOOT rail has an UVLO setting.
This UVLO has a threshold of VCBOOT-UVLO and is typically set at 2.3 V. If the CBOOT capacitor is not charged
above this voltage with respect to the SW pin, then the part initiates a charging sequence, turning on the low-
side switch before attempting to turn on the high-side device.

8.3.6 Output Voltage Selection

In  the  LMR36506  family,  select  variants  with  an  adjustable  output  voltage  option  (see 节  5),  and  you  need  an
external resistor divider connection between the output voltage node, the device FB pin, and the system GND,

16

Submit Document Feedback

Copyright © 2022 Texas Instruments Incorporated

Product Folder Links: LMR36506

Input VoltageInput VoltageOutput VoltageVPG-UV (falling)VPG-HYSVIN_R (rising)VPG_VALIDGNDVOUTVIN_F (falling) PGOODPGOOD may not be valid if input is below VPG-VALIDStartup delaytRESET_FILTERPGOOD may not be valid if input is below VPG-VALIDSmall glitches do not reset tPGOOD_ACT timer Small glitches do not cause reset to signal a faulttPGOOD_ACTtRESET_FILTERtPGOOD_ACTtRESET_FILTERtRESET_FILTERwww.ti.com.cn

LMR36506
ZHCSKN9B – DECEMBER 2019 – REVISED DECEMBER 2022

as shown in 图 8-6. The variants with adjustable output voltage option in the LMR36506 family are designed with
a 1-V internal reference voltage.

When  using  the  fixed-output  variants  from  the  LMR36506  family,  simply  connect  the  FB  pin  is  identified  as
VOUT/BIAS pin for fixed-output variants in the rest of the data sheet) to the system output voltage node. See 节
5 for more details.

(2)

图 8-6. Setting Output Voltage for Adjustable Output Variant

In adjustable output voltage variants, an addition feed-forward capacitor, CFF, in parallel with the RFBT, can be
used to optimize the phase margin and transient response. See 节 9.2.2.8 for more details. No additional resistor
divider or feed-forward capacitor, CFF, is needed in fixed-output variants.

8.3.7 Soft Start and Recovery from Dropout

When designing with the LMR36506, slow rise in output voltage due to recovery from dropout and soft start must
be considered as a two separate operating conditions, as shown in 图 8-7 and 图 8-8. Soft start is triggered by
any of the following conditions:

• Power is applied to the VIN pin of the device, releasing undervoltage lockout.
• EN is used to turn on the device.
• Recovery from shutdown due to overtemperature protection.

After soft start is triggered, the IC takes the following actions:

• The reference used by the IC to regulate output voltage is slowly ramped up. The net result is that output

voltage, if previously 0 V, takes tSS to reach 90% of the desired value.

• Operating mode is set to auto mode of operation, activating the diode emulation mode for the low-side

MOSFET. This allows start-up without pulling the output low. This is true even when there is a voltage already
present at the output during a pre-bias start-up.

Copyright © 2022 Texas Instruments Incorporated

Submit Document Feedback

17

Product Folder Links: LMR36506

RFBB =VOUT  Å(cid:3)1RFBTRFBTRFBBFBAGNDVOUTLMR36506
ZHCSKN9B – DECEMBER 2019 – REVISED DECEMBER 2022

www.ti.com.cn

图 8-7. Soft Start With and Without Pre-bias Voltage

8.3.7.1 Recovery from Dropout

Any time the output voltage falls more than a few percent, output voltage ramps up slowly. This condition, called
graceful recovery from dropout in this document, differs from soft start in two important ways:

• The reference voltage is set to approximately 1% above what is needed to achieve the existing output

voltage.

• If the device is set to FPWM, the device continues to operate in that mode during its recovery from dropout. If
output voltage were to suddenly be pulled up by an external supply, the LMR36506 can pull down on the
output. Note that all protections that are present during normal operation are in place, preventing any
catastrophic failure if output is shorted to a high voltage or ground.

图 8-8. Recovery from Dropout

图 8-9. Typical Output Recovery from Dropout from 8 V to 4 V

18

Submit Document Feedback

Copyright © 2022 Texas Instruments Incorporated

Product Folder Links: LMR36506

VENVOUT Set PointEN and Output VoltagesTimetVOUTV90% of VOUT Set Point0 VtSSTriggering eventIf selected, FPWM is enabled only after completion of tSStENVENVOUT Set PointEN and Output VoltagesTimetVOUTV90% of VOUT Set Point0 VtSSTriggering eventtENIf selected, FPWM is enabled only after completion of tSSLoad currentVOUT Set Point and max output currentOutput Voltage and CurrentSlope the same as during soft startTimetVOUTVVOUT (2V/DIV)500µs/DIVVIN (2V/DIV) Load Current (0.2A/DIV)8V 4V 5V www.ti.com.cn

LMR36506
ZHCSKN9B – DECEMBER 2019 – REVISED DECEMBER 2022

Whether output voltage falls due to high load or low input voltage, after the condition that causes output to fall
below its set point is removed, the output climbs at the same speed as during start-up. shows an example of this
behavior.

8.3.8 Current Limit and Short Circuit

The LMR36506 is protected from overcurrent conditions by cycle-by-cycle current limiting on both high-side and
low-side MOSFETs.

High-side MOSFET overcurrent protection is implemented by the typical peak-current mode control scheme. The
HS  switch  current  is  sensed  when  the  HS  is  turned  on  after  a  short  blanking  time.  The  HS  switch  current  is
compared  to  either  the  minimum  of  a  fixed  current  set  point  or  the  output  of  the  internal  error  amplifier  loop
minus the slope compensation every switching cycle. Because the output of the internal error amplifier loop has
a maximum value and slope compensation increases with duty cycle, HS current limit decreases with increased
duty factor if duty factor is typically above 35%.

When the LS switch is turned on, the current going through it is also sensed and monitored. Like the high-side
device, the low-side device has a turnoff commanded by the internal error amplifier loop. In the case of the low-
side  device,  turnoff  is  prevented  if  the  current  exceeds  this  value,  even  if  the  oscillator  normally  starts  a  new
switching cycle. Also like the high-side device, there is a limit on how high the turnoff current is allowed to be.
This is called the low-side current limit, ILS-LIMIT (or IL-LS in  图 8-10). If the LS current limit is exceeded, the LS
MOSFET stays on and the HS switch is not to be turned on. The LS switch is turned off after the LS current falls
below this limit and the HS switch is turned on again as long as at least one clock period has passed because
the last time the HS device has turned on.

图 8-10. Current Limit Waveforms

Because  the  current  waveform  assumes  values  between  ISC  (or  IL-HS  in  图  8-10)  and  ILS-LIMIT,  the  maximum
output current is very close to the average of these two values unless duty factor is very high. After operating in
current limit, hysteretic control is used and current does not increase as output voltage approaches zero.

If duty factor is very high, current ripple must be very low to prevent instability. Because current ripple is low, the
part is able to deliver full current. The current delivered is very close to ILS-LIMIT.

Copyright © 2022 Texas Instruments Incorporated

Submit Document Feedback

19

Product Folder Links: LMR36506

iLVSWIL-LSInductor Currenttt00SW VoltageVIN Typically, tSW > Clock settingtON < tON_MAXIL-HSIOUTLMR36506
ZHCSKN9B – DECEMBER 2019 – REVISED DECEMBER 2022

www.ti.com.cn

图 8-11. Output Voltage versus Output Current

Under most conditions, current is limited to the average of IL-HS and IL-LS, which is approximately 1.3 times the
maximum-rated current. If input voltage is low, current can be limited to approximately IL-LS. Also note that the
maximum output current does not exceed the average of IL-HS and IL-LS. After the overload is removed, the part
recovers as though in soft start.

图 8-12. Short-Circuit Waveform

图 8-13. Overload Output Recovery (100 mA to 750
mA)

8.3.9 Thermal Shutdown

Thermal  shutdown  limits  total  power  dissipation  by  turning  off  the  internal  switches  when  the  device  junction
temperature exceeds 168°C (typical). Thermal shutdown does not trigger below 158°C (minimum). After thermal
shutdown  occurs,  hysteresis  prevents  the  part  from  switching  until  the  junction  temperature  drops  to
approximately  158°C  (typical).  When  the  junction  temperature  falls  below  158°C  (typical),  the  LMR36506
attempts another soft start.

While the LMR36506 is shut down due to high junction temperature, power continues to be provided to VCC. To
prevent  overheating  due  to  a  short  circuit  applied  to  VCC,  the  LDO  that  provides  power  for  VCC  has  reduced
current  limit  while  the  part  is  disabled  due  to  high  junction  temperature.  The  LDO  only  provides  a  few
milliamperes during thermal shutdown.

8.3.10 Input Supply Current

The LMR36506 is designed to have very low input supply current when regulating light loads. This is achieved
by  powering  much  of  the  internal  circuitry  from  the  output.  The  VOUT/BIAS  pin  in  the  fixed-output  voltage
variants is the input to the LDO that powers the majority of the control circuits. By connecting the VOUT/BIAS
input pin to the output node of the regulator, a small amount of current is drawn from the output. This current is
reduced at the input by the ratio of VOUT / VIN.

20

Submit Document Feedback

Copyright © 2022 Texas Instruments Incorporated

Product Folder Links: LMR36506

VOUTIL-LSOutput Voltage0IL-HSIOUTOutput Current0VIN ~ VOUT SettingVIN > 2 Â(cid:3)9OUT SettingVOUT SettingIOUT ratedVOUT (5V/DIV)10ms/DIVInductor Current (0.5A/DIV)0V Short AppliedShort RemovedVOUT (2V/DIV)2ms/DIVLoad Current (0.2A/DIV)5V 100mA 750mA www.ti.com.cn

LMR36506
ZHCSKN9B – DECEMBER 2019 – REVISED DECEMBER 2022

(3)

where
• IQ_VIN is the total standby (switching) current consumed by the operating (switching) buck converter when

unloaded.

• IQ is the current drawn from the VIN terminal. Check IQ_13p5_Fixed or IQ_24p0_Fixed in 节 7.5 for IQ.
• IEN is current drawn by the EN terminal. Include this current if EN is connected to VIN. Check ILKG-EN in 节 7.5

for IEN.

• IBIAS is bias current drawn by the BIAS input. Check IB_13p5 or IB_24p0 in 节 7.5 for IBIAS.
• ηeff is the light-load efficiency of the buck converter with IQ_VIN removed from the input current of the buck

converter. ηeff = 0.8 is a conservative value that can be used under normal operating conditions. This can be
traced back as the ISUPPLY in 节 7.7.

Copyright © 2022 Texas Instruments Incorporated

Submit Document Feedback

21

Product Folder Links: LMR36506

IQ_VIN = IQ + IEN + IBIAS ¾eff x VINVOUTLMR36506
ZHCSKN9B – DECEMBER 2019 – REVISED DECEMBER 2022

8.4 Device Functional Modes
8.4.1 Shutdown Mode

www.ti.com.cn

The EN pin provides electrical ON and OFF control of the device. When the EN pin voltage is below 0.4 V, both
the converter and the internal LDO have no output voltage and the part is in shutdown mode. In shutdown mode,
the quiescent current drops to typically 0.5 µA.

8.4.2 Standby Mode

The internal LDO has a lower EN threshold than the output of the converter. When the EN pin voltage is above
1.1 V (maximum) and below the precision enable threshold for the output voltage, the internal LDO regulates the
VCC  voltage  at  3.3  V  typical.  The  precision  enable  circuitry  is  ON  after  VCC  is  above  its  UVLO.  The  internal
power  MOSFETs  of  the  SW  node  remain  off  unless  the  voltage  on  EN  pin  goes  above  its  precision  enable
threshold. The LMR36506 also employs UVLO protection. If the VCC voltage is below its UVLO level, the output
of the converter is turned off.

8.4.3 Active Mode

The LMR36506 is in active mode whenever the EN pin is above VEN-VOUT, VIN is high enough to satisfy VIN_R,
and no other fault conditions are present. The simplest way to enable the operation is to connect the EN pin to
VIN, which allows self start-up when the applied input voltage exceeds the minimum VIN_R.

In active mode, depending on the load current, input voltage, and output voltage, the LMR36506 is in one of five
modes:

• Continuous conduction mode (CCM) with fixed switching frequency when load current is above half of the

inductor current ripple

• Auto Mode - Light Load Operation: PFM when switching frequency is decreased at very light load
• FPWM Mode - Light Load Operation: Discontinuous conduction mode (DCM) when the load current is lower

than half of the inductor current ripple

• Minimum on-time: At high input voltage and low output voltages, the switching frequency is reduced to

maintain regulation.

• Dropout mode: When switching frequency is reduced to minimize voltage dropout.

8.4.3.1 CCM Mode

The following operating description of the LMR36506 refers to 节 8.2 and to the waveforms in 图 8-14. In CCM,
the LMR36506 supplies a regulated output voltage by turning on the internal high-side (HS) and low-side (LS)
switches  with  varying  duty  cycle  (D).  During  the  HS  switch  on-time,  the  SW  pin  voltage,  VSW,  swings  up  to
approximately VIN, and the inductor current, iL, increases with a linear slope. The HS switch is turned off by the
control logic. During the HS switch off-time, tOFF, the LS switch is turned on. Inductor current discharges through
the  LS  switch,  which  forces  the  VSW  to  swing  below  ground  by  the  voltage  drop  across  the  LS  switch.  The
converter loop adjusts the duty cycle to maintain a constant output voltage. D is defined by the on-time of the HS
switch over the switching period:

D = TON / TSW

(4)

In  an  ideal  buck  converter  where  losses  are  ignored,  D  is  proportional  to  the  output  voltage  and  inversely
proportional to the input voltage:

D = VOUT / VIN

(5)

22

Submit Document Feedback

Copyright © 2022 Texas Instruments Incorporated

Product Folder Links: LMR36506

www.ti.com.cn

LMR36506
ZHCSKN9B – DECEMBER 2019 – REVISED DECEMBER 2022

图 8-14. SW Voltage and Inductor Current Waveforms in Continuous Conduction Mode (CCM)

8.4.3.2 Auto Mode - Light Load Operation

The LMR36506 can have two behaviors while lightly loaded. One behavior, called auto mode operation, allows
for  seamless  transition  between  normal  current  mode  operation  while  heavily  loaded  and  highly  efficient  light
load  operation.  The  other  behavior,  called  FPWM  Mode,  maintains  full  frequency  even  when  unloaded.  Which
mode  the  LMR36506  operates  in  depends  on  which  variant  from  this  family  is  selected.  Note  that  all  parts
operate in FPWM mode when synchronizing frequency to an external signal.

The light load operation is employed in the LMR36506 only in the auto mode. The light load operation employs
two techniques to improve efficiency:

• Diode emulation, which allows DCM operation. See 图 8-15.
• Frequency reduction. See 图 8-16.

Note that while these two features operate together to improve light load efficiency, they operate independent of
each other.

8.4.3.2.1 Diode Emulation

Diode  emulation  prevents  reverse  current  through  the  inductor  which  requires  a  lower  frequency  needed  to
regulate given a fixed peak inductor current. Diode emulation also limits ripple current as frequency is reduced.
With  a  fixed  peak  current,  as  output  current  is  reduced  to  zero,  frequency  must  be  reduced  to  near  zero  to
maintain regulation.

Copyright © 2022 Texas Instruments Incorporated

Submit Document Feedback

23

Product Folder Links: LMR36506

iLVSWILPKIOUTIrippleInductor Current- IOUTÂ5DSLSD = tt00SW VoltagetONtSWVINtSWtONtOFFVOUTVIN§(cid:3)LMR36506
ZHCSKN9B – DECEMBER 2019 – REVISED DECEMBER 2022

www.ti.com.cn

In auto mode, the low-side device is turned off after SW node current is near zero. As a result, after output current is less than half of

what inductor ripple is in CCM, the part operates in DCM which is equivalent to the statement that diode emulation is active.

图 8-15. PFM Operation

The LMR36506 has a minimum peak inductor current setting (ILPK (see IPEAK-MIN in 节 7.5) while in auto mode.
After current is reduced to a low value with fixed input voltage, on-time is constant. Regulation is then achieved
by adjusting frequency. This mode of operation is called PFM mode regulation.

8.4.3.2.2 Frequency Reduction

The  LMR36506  reduces  frequency  whenever  output  voltage  is  high.  This  function  is  enabled  whenever  the
internal error amplifier compensation output, COMP, an internal signal, is low and there is an offset between the
regulation set point of FB and the voltage applied to FB. The net effect is that there is larger output impedance
while lightly loaded in auto mode than in normal operation. Output voltage must be approximately 1% high when
the part is completely unloaded.

In auto mode, after output current drops below approximately 1/10th the rated current of the part, output resistance increases so that

output voltage is 1% high while the buck is completely unloaded.

图 8-16. Steady State Output Voltage versus Output Current in Auto Mode

In PFM operation, a small DC positive offset is required on the output voltage to activate the PFM detector. The
lower the frequency in PFM, the more DC offset is needed on VOUT. If the DC offset on VOUT is not acceptable, a
dummy load at VOUT or FPWM Mode can be used to reduce or eliminate this offset.

24

Submit Document Feedback

Copyright © 2022 Texas Instruments Incorporated

Product Folder Links: LMR36506

iLVSWILPKIOUTInductor CurrentD = tt00SW VoltagetONtSWVINtSWtONtOFFtHIGHZVOUTVIN< 1% Above Set pointVOUT Set PointOutput VoltageCurrent LimitVOUTOutput CurrentIOUT0www.ti.com.cn

8.4.3.3 FPWM Mode - Light Load Operation

LMR36506
ZHCSKN9B – DECEMBER 2019 – REVISED DECEMBER 2022

In FPWM Mode, frequency is maintained while lightly loaded. To maintain frequency, a limited reverse current is
allowed to flow through the inductor. Reverse current is limited by reverse current limit circuitry, see 节 7.5 for
reverse current limit values.

In FPWM mode, Continuous Conduction (CCM) is possible even if IOUT is less than half of Iripple.

图 8-17. FPWM Mode Operation

For  all  devices,  in  FPWM  mode,  frequency  reduction  is  still  available  if  output  voltage  is  high  enough  to
command minimum on-time even while lightly loaded, allowing good behavior during faults which involve output
being pulled up.

8.4.3.4 Minimum On-time (High Input Voltage) Operation

The LMR36506 continues to regulate output voltage even if the input-to-output voltage ratio requires an on-time
less than the minimum on-time of the chip with a given clock setting. This is accomplished using valley current
control.  At  all  times,  the  compensation  circuit  dictates  both  a  maximum  peak  inductor  current  and  a  maximum
valley  inductor  current.  If  for  any  reason,  valley  current  is  exceeded,  the  clock  cycle  is  extended  until  valley
current falls below that determined by the compensation circuit. If the converter is not operating in current limit,
the  maximum  valley  current  is  set  above  the  peak  inductor  current,  preventing  valley  control  from  being  used
unless there is a failure to regulate using peak current only. If the input-to-output voltage ratio is too high, such
that the inductor current peak value exceeds the peak command dictated by compensation, the high-side device
cannot  be  turned  off  quickly  enough  to  regulate  output  voltage.  As  a  result,  the  compensation  circuit  reduces
both peak and valley current. After a low enough current is selected by the compensation circuit, valley current
matches that being commanded by the compensation circuit. Under these conditions, the low-side device is kept
on  and  the  next  clock  cycle  is  prevented  from  starting  until  inductor  current  drops  below  the  desired  valley
current. Because on-time is fixed at its minimum value, this type of operation resembles that of a device using a
Constant On-Time (COT) control scheme; see 图 8-18.

Copyright © 2022 Texas Instruments Incorporated

Submit Document Feedback

25

Product Folder Links: LMR36506

iLVSWILPKIOUTIrippleInductor CurrentD = tt00SW VoltagetONtSWVINtSWtONtOFFVOUTVIN§(cid:3)(cid:3)LMR36506
ZHCSKN9B – DECEMBER 2019 – REVISED DECEMBER 2022

www.ti.com.cn

In valley control mode, minimum inductor current is regulated, not peak inductor current.

图 8-18. Valley Current Mode Operation

8.4.3.5 Dropout

Dropout operation is defined as any input-to-output voltage ratio that requires frequency to drop to achieve the
required  duty  cycle.  At  a  given  clock  frequency,  duty  cycle  is  limited  by  minimum  off-time.  After  this  limit  is
reached  as  shown  in  图  8-20,  if  clock  frequency  was  to  be  maintained,  the  output  voltage  falls.  Instead  of
allowing  the  output  voltage  to  drop,  the  LMR36506  extends  the  high  side  switch  on-time  past  the  end  of  the
clock  cycle  until  the  needed  peak  inductor  current  is  achieved.  The  clock  is  allowed  to  start  a  new  cycle  after
peak  inductor  current  is  achieved  or  after  a  pre-determined  maximum  on-time,  tON-MAX,  of  approximately  9  µs
passes. As a result, after the needed duty cycle cannot be achieved at the selected clock frequency due to the
existence of a minimum off-time, frequency drops to maintain regulation. As shown in 图 8-19 if input voltage is
low enough so that output voltage cannot be regulated even with an on-time of tON-MAX, output voltage drops to
slightly below the input voltage by VDROP. For additional information on recovery from dropout, refer back to 图
8-8.

Output voltage and frequency versus input voltage: If there is little difference between input voltage and output voltage setting, the IC

reduces frequency to maintain regulation. If input voltage is too low to provide the desired output voltage at approximately 110 kHz,

input voltage tracks output voltage.

图 8-19. Frequency and Output Voltage in Dropout

26

Submit Document Feedback

Copyright © 2022 Texas Instruments Incorporated

Product Folder Links: LMR36506

iLVSWILVLYIOUTIrippleInductor Current- IOUTÂ5DSLSD = tt00SW VoltagetONtSWVINtSW > Clock settingtON = tON_MINtOFFVOUTVIN§(cid:3)VOUTOutput SettingOutput VoltageVIN0Input VoltageFSWSwitching FrequencyVIN0Input VoltageInput Voltage ~110kHzVDROPOutput VoltageFSW-NOMwww.ti.com.cn

LMR36506
ZHCSKN9B – DECEMBER 2019 – REVISED DECEMBER 2022

Switching waveforms while in dropout. Inductor current takes longer than a normal clock to reach the desired peak value. As a result,
frequency drops. This frequency drop is limited by tON-MAX.

图 8-20. Dropout Waveforms

Copyright © 2022 Texas Instruments Incorporated

Submit Document Feedback

27

Product Folder Links: LMR36506

iLVSWIOUTIrippleInductor Current- IOUTÂ5DSLSD = tt00SW VoltagetONtSWVINtSW > Clock settingtOFF = tOFF_MINtON < tON_MAXVOUTVIN§(cid:3)ILPKLMR36506
ZHCSKN9B – DECEMBER 2019 – REVISED DECEMBER 2022

9 Application and Implementation

备注

www.ti.com.cn

以下应用部分中的信息不属于 TI 器件规格的范围，TI 不担保其准确性和完整性。TI 的客 户应负责确定
器件是否适用于其应用。客户应验证并测试其设计，以确保系统功能。

9.1 Application Information

The LMR36506 step-down DC-to-DC converter is typically used to convert a higher DC voltage to a lower DC
voltage  with  a  maximum  output  current  of  0.6  A.  The  following  design  procedure  can  be  used  to  select
components for the LMR36506.

备注

All  of  the  capacitance  values  given  in  the  following  application  information  refer  to  effective  values
unless otherwise stated. The effective value is defined as the actual capacitance under DC bias and
temperature,  not  the  rated  or  nameplate  values.  Use  high-quality,  low-ESR,  ceramic  capacitors  with
an  X7R  or  better  dielectric  throughout.  All  high  value  ceramic  capacitors  have  a  large  voltage
coefficient  in  addition  to  normal  tolerances  and  temperature  effects.  Under  DC  bias  the  capacitance
drops  considerably.  Large  case  sizes  and  higher  voltage  ratings  are  better  in  this  regard.  To  help
mitigate  these  effects,  multiple  capacitors  can  be  used  in  parallel  to  bring  the  minimum  effective
capacitance up to the required value. This can also ease the RMS current requirements on a single
capacitor.  A  careful  study  of  bias  and  temperature  variation  of  any  capacitor  bank  must  be  made  to
ensure that the minimum value of effective capacitance is provided.

28

Submit Document Feedback

Copyright © 2022 Texas Instruments Incorporated

Product Folder Links: LMR36506

www.ti.com.cn

9.2 Typical Application

LMR36506
ZHCSKN9B – DECEMBER 2019 – REVISED DECEMBER 2022

图  9-1  shows  a  typical  application  circuit  for  the  LMR36506.  This  device  is  designed  to  function  over  a  wide
range  of  external  components  and  system  parameters.  However,  the  internal  compensation  is  optimized  for  a
certain  range  of  external  inductance  and  output  capacitance.  As  a  quick-start  guide,  表  9-1  provides  typical
component values for a range of the most common output voltages.

图 9-1. Example Application Circuit

表 9-1. Typical External Component Values(1)

ƒSW
(kHz)

VOUT
(V)

L (µH)

NOMINAL COUT
(RATED
CAPACITANCE)

MINIMUM COUT
(RATED
CAPACITANCE)

RFBT (Ω)

RFBB (Ω)

CIN

CBOOT

CVCC

400

1000

400

1000

3.3

3.3

5

5

33

15

47

22

1 x 47 µF

2 x 22 µF

1 x 47 µF

2 x 22 µF

1 x 22 µF

1 x 22 µF

1 x 22 µF

1 x 22 µF

100 k

100 k

100 k

100 k

43.2 k

43.2 k

24.9 k

24.9 k

2.2 µF + 1 × 100 nF

100 nF

2.2 µF + 1 × 100 nF

100 nF

2.2 µF + 1 × 100 nF

100 nF

2.2 µF + 1 × 100 nF

100 nF

1 µF

1 µF

1 µF

1 µF

(1)

Inductor values are calculated based on typical VIN = 24 V.

9.2.1 Design Requirements

节 9.2.2 provides a detailed design procedure based on 表 9-2.

表 9-2. Detailed Design Parameters

DESIGN PARAMETER

Input voltage

Output voltage

Maximum output current

Switching frequency

EXAMPLE VALUE

24 V (6 V to 65 V)

5 V

0 A to 0.6 A

1000 kHz

9.2.2 Detailed Design Procedure

The following design procedure applies to 图 9-1 and 表 9-1.

9.2.2.1 Choosing the Switching Frequency

The  choice  of  switching  frequency  is  a  compromise  between  conversion  efficiency  and  overall  solution  size.
Lower  switching  frequency  implies  reduced  switching  losses  and  usually  results  in  higher  system  efficiency.
However, higher switching frequency allows the use of smaller inductors and output capacitors, hence, a more
compact design. For this example, 1000 kHz is used.

Copyright © 2022 Texas Instruments Incorporated

Submit Document Feedback

29

Product Folder Links: LMR36506

VINENVCCSWBOOTFBVINVOUTLCBOOTCOUTCINCVCC2.2 µF0.1 µF1 µFRTLMR36506CHF100 nFRFBTRFBB100 NŸCFFGNDPGLMR36506
ZHCSKN9B – DECEMBER 2019 – REVISED DECEMBER 2022

9.2.2.2 Setting the Output Voltage

www.ti.com.cn

For the fixed output voltage versions, pin 8 (VOUT/BIAS) of the device must be connected directly to the output
voltage node. This output sensing point is normally located near the top of the output capacitor. If the sensing
point is located further away from the output capacitors (that is, remote sensing), then a small 100-nF capacitor
can be needed at the sensing point.

9.2.2.2.1 FB for Adjustable Output

In an adjustable output voltage version, pin 8 of the device is FB. The output voltage of LMR36506 is externally
adjustable using an external resistor divider network. The divider network is comprised of RFBT and RFBB, and
closes  the  loop  between  the  output  voltage  and  the  converter.  The  converter  regulates  the  output  voltage  by
holding the voltage on the FB pin equal to the internal reference voltage, VREF. The resistance of the divider is a
compromise between excessive noise pickup and excessive loading of the output. Smaller values of resistance
reduce  noise  sensitivity  but  also  reduce  the  light-load  efficiency.  The  recommended  value  for  RFBT  is  100  kΩ
with a maximum value of 1 MΩ. After RFBT is selected, 方程式 6 is used to select RFBB. VREF is nominally 1 V.
See 节 7.5.

For this 5-V example, RFBT = 100 kΩ and RFBB = 24.9 kΩ is chosen.

9.2.2.3 Inductor Selection

The parameters for selecting the inductor are the inductance and saturation current. The inductance is based on
the  desired  peak-to-peak  ripple  current  and  is  normally  chosen  to  be  in  the  range  of  20%  to  40%  of  the
maximum  output  current.  Experience  shows  that  the  best  value  for  inductor  ripple  current  is  30%  of  the
maximum load current. Note that when selecting the ripple current for applications with much smaller maximum
load than the maximum available from the device, use the maximum device current.  方程式 7 can be used to
determine the value of inductance. The constant K is the percentage of inductor current ripple. For this example,
choose K = 0.3 and find an inductance of L = 22 µH. Select the next standard value of L = 22 µH.

(6)

(7)

Ideally, the saturation current rating of the inductor is at least as large as the high-side switch current limit, ISC
(see 节 7.5). This ensures that the inductor does not saturate, even during a short circuit on the output. When
the inductor core material saturates, the inductance falls to a very low value, causing the inductor current to rise
very rapidly. Although the valley current limit, ILIMIT, is designed to reduce the risk of current runaway, a saturated
inductor can cause the current to rise to high values very rapidly. This can lead to component damage. Do not
allow the inductor to saturate. Inductors with a ferrite core material have very hard saturation characteristics, but
usually have lower core losses than powdered iron cores. Powered iron cores exhibit a soft saturation, allowing
some relaxation in the current rating of the inductor. However, they have more core losses at frequencies above
about  1  MHz.  In  any  case,  the  inductor  saturation  current  must  not  be  less  than  the  maximum  peak  inductor
current at full load.

To avoid subharmonic oscillation, the inductance value must not be less than that given in 方程式 8:

The  maximum  inductance  is  limited  by  the  minimum  current  ripple  for  the  current  mode  control  to  perform
correctly. As a rule-of-thumb, the minimum inductor ripple current must be no less than about 10% of the device
maximum rated current under nominal conditions.

30

Submit Document Feedback

Copyright © 2022 Texas Instruments Incorporated

Product Folder Links: LMR36506

(8)

»¼º«¬ª(cid:16) 1VVRRREFOUTFBTFBB(cid:11)(cid:12)INOUTmaxOUTSWOUTINVVIKfVVL˜˜˜(cid:16) LMIN •(cid:3)(cid:3)1.5 x  VOUTfSWwww.ti.com.cn

9.2.2.4 Output Capacitor Selection

LMR36506
ZHCSKN9B – DECEMBER 2019 – REVISED DECEMBER 2022

The  current  mode  control  scheme  of  the  LMR36506  devices  allows  operation  over  a  wide  range  of  output
capacitance. The output capacitor bank is usually limited by the load transient requirements and stability rather
than the output voltage ripple. Please refer to 节 9.2 for typical output capacitor value for 3.3-V and 5-V output
voltages.  Based  on  this  table,  for  a  5-V  output  design,  you  can  choose  the  recommended  ceramic  output
capacitor for this example. For other designs with other output voltages, WEBENCH can be used as a starting
point for selecting the value of output capacitor.

In practice, the output capacitor has the most influence on the transient response and loop-phase margin. Load
transient testing and bode plots are the best way to validate any given design and must always be completed
before  the  application  goes  into  production.  In  addition  to  the  required  output  capacitance,  a  small  ceramic
placed on the output can help reduce high-frequency noise. Small-case size ceramic capacitors in the range of 1
nF to 100 nF can be very helpful in reducing spikes on the output caused by inductor and board parasitics.

Limit the maximum value of total output capacitance to about 10 times the design value, or 1000 µF, whichever
is smaller. Large values of output capacitance can adversely affect the start-up behavior of the regulator as well
as the loop stability. If values larger than noted here must be used, then a careful study of start-up at full load
and loop stability must be performed.

9.2.2.5 Input Capacitor Selection

The ceramic input capacitors provide a low impedance source to the regulator in addition to supplying the ripple
current and isolating switching noise from other circuits. A minimum ceramic capacitance of 4.7 µF is required on
the  input  of  the  LMR36506.  This  must  be  rated  for  at  least  the  maximum  input  voltage  that  the  application
requires, preferably twice the maximum input voltage. This capacitance can be increased to help reduce input
voltage ripple and maintain the input voltage during load transients. In addition, a small case size 100-nF to 220-
nF  ceramic  capacitor  must  be  used  at  the  input,  as  close  a  possible  to  the  regulator.  This  provides  a  high
frequency bypass for the control circuits internal to the device. For this example, a 4.7-µF, 100-V, X7R (or better)
ceramic capacitor is chosen. The 100 nF must also be rated at 100 V with an X7R dielectric.

Using an electrolytic capacitor on the input in parallel with the ceramics is desirable. This is especially true if long
leads or traces are used to connect the input supply to the regulator. The moderate ESR of this capacitor can
help damp any ringing on the input supply caused by the long power leads. The use of this additional capacitor
also helps with voltage dips caused by input supplies with unusually high impedance.

Most of the input switching current passes through the ceramic input capacitor or capacitors. The approximate
RMS  value  of  this  current  can  be  calculated  from  方程式  9  and  must  be  checked  against  the  manufacturers'
maximum ratings.

(9)

9.2.2.6 CBOOT

The LMR36506 requires a bootstrap capacitor connected between the BOOT pin and the SW pin. This capacitor
stores energy that is used to supply the gate drivers for the power MOSFETs. A high-quality ceramic capacitor of
100 nF and at least 16 V is required.

9.2.2.7 VCC

The  VCC  pin  is  the  output  of  the  internal  LDO  used  to  supply  the  control  circuits  of  the  regulator.  This  output
requires  a  1-µF,  16-V  ceramic  capacitor  connected  from  VCC  to  GND  for  proper  operation.  In  general,  this
output must not be loaded with any external circuitry. However, this output can be used to supply the pullup for
the power-good function (see 节 8.3.3). A value in the range of 10 kΩ to 100 kΩ is a good choice in this case.
The nominal output voltage on VCC is 3.2 V; see 节 7.5 for limits.

9.2.2.8 CFF Selection

In  some  cases,  a  feedforward  capacitor  can  be  used  across  RFBT  to  improve  the  load  transient  response  or
improve the loop-phase margin. This is especially true when values of RFBT > 100 kΩ are used. Large values of

Copyright © 2022 Texas Instruments Incorporated

Submit Document Feedback

31

Product Folder Links: LMR36506

2IIOUTRMS#LMR36506
ZHCSKN9B – DECEMBER 2019 – REVISED DECEMBER 2022

www.ti.com.cn

RFBT, in combination with the parasitic capacitance at the FB pin, can create a small signal pole that interferes
with the loop stability. A CFF can help mitigate this effect. Use 方程式 10 to estimate the value of CFF. The value
found with 方程式 10 is a starting point; use lower values to determine if any advantage is gained by the use of a
CFF  capacitor.  The  Optimizing  Transient  Response  of  Internally  Compensated  DC-DC  Converters  with
Feedforward Capacitor Application Report is helpful when experimenting with a feedforward capacitor.

9.2.2.8.1 External UVLO

In  some  cases,  an  input  UVLO  level  different  than  that  provided  internal  to  the  device  is  needed.  This  can  be
accomplished by using the circuit shown in 图 9-2. The input voltage at which the device turns on is designated
as VON while the turnoff voltage is VOFF. First, a value for RENB is chosen in the range of 10 kΩ to 100 kΩ, then
方程式 11 is used to calculate RENT and VOFF.

(10)

图 9-2. Setup for External UVLO Application

(11)

where
• VON is the VIN turnon voltage.
• VOFF is the VIN turnoff voltage.

9.2.2.9 Maximum Ambient Temperature

As with any power conversion device, the LMR36506 dissipates internal power while operating. The effect of this
power  dissipation  is  to  raise  the  internal  temperature  of  the  converter  above  ambient.  The  internal  die
temperature (TJ) is a function of the ambient temperature, the power loss, and the effective thermal resistance,
RθJA,  of  the  device  and  PCB  combination.  The  maximum  junction  temperature  for  the  LMR36506  must  be
limited  to  150°C.  This  establishes  a  limit  on  the  maximum  device  power  dissipation  and,  therefore,  the  load
current. 方程式 12 shows the relationships between the important parameters. Larger ambient temperatures (TA)
and  larger  values  of  RθJA  reduce  the  maximum  available  output  current.  The  converter  efficiency  can  be
estimated by using the curves provided in this data sheet. If the desired operating conditions cannot be found in
one of the curves, interpolation can be used to estimate the efficiency. Alternatively, the EVM can be adjusted to
match  the  desired  application  requirements  and  the  efficiency  can  be  measured  directly.  The  correct  value  of
RθJA is more difficult to estimate. As stated in the Semiconductor and IC Package Thermal Metrics Application
Report,  the  values  given  in  节  7.4  are  not  valid  for  design  purposes  and  must  not  be  used  to  estimate  the
thermal performance of the application. The values reported in that table were measured under a specific set of
conditions that are rarely obtained in an actual application.

32

Submit Document Feedback

Copyright © 2022 Texas Instruments Incorporated

Product Folder Links: LMR36506

OUTREFFBTOUTOUTFFVVR120CVC˜˜˜(cid:31)ENRENTRENBVIN¸¸¹·¨¨©§(cid:16)˜ ˜¸¸¹·¨¨©§(cid:16) (cid:16)(cid:16)ENHYSENONOFFENBHENONENTVV1VVR1VVRwww.ti.com.cn

LMR36506
ZHCSKN9B – DECEMBER 2019 – REVISED DECEMBER 2022

where
• η is the efficiency.

The effective RθJA is a critical parameter and depends on many factors such as the following:

(12)

• Power dissipation
• Air temperature/flow
• PCB area
• Copper heat-sink area
• Number of thermal vias under the package
• Adjacent component placement

A typical example of RθJA versus copper board area can be found in 图 9-3. The copper area given in the graph
is  for  each  layer.  For  a  4-layer  PCB  design,  the  top  and  bottom  layers  are  2-oz.  copper  each,  while  the  inner
layers are 1 oz. For a 2-layer PCB design, the top and bottom layers are 2-oz. copper each. Note that the data
given  in  these  graphs  are  for  illustration  purposes  only,  and  the  actual  performance  in  any  given  application
depends on all of the factors mentioned above.

Using the value of RθJA from 图 9-3 for a given PCB copper area and ΨJT from 节 7.4, one can approximate the
junction temperature of the IC for a given operating condition using 方程式 13

TJ ≈ TA + RθJA x IC Power Loss

(13)

where
• TJ is the IC junction temperature (°C).
• TA is the ambient temperature (°C).
• RθJA is the thermal resistance (°C/W)
• IC Power Loss is the power loss for the IC (W).

The IC Power loss mentioned above is the overall power loss minus the loss that comes from the inductor DC
Resistance. The overall power loss can be approximated from the efficiency curves in the Application Curves or
by using WEBENCH for a specific operating condition and temperature.

Copyright © 2022 Texas Instruments Incorporated

Submit Document Feedback

33

Product Folder Links: LMR36506

(cid:11)(cid:12)(cid:11)(cid:12)OUTJAAJMAXOUTV11RTTI˜K(cid:16)K˜(cid:16) TLMR36506
ZHCSKN9B – DECEMBER 2019 – REVISED DECEMBER 2022

www.ti.com.cn

图 9-3. RθJA versus PCB Copper Area for the VQFN (RPE) Package

Use  the  following  resources  as  guides  to  optimal  thermal  PCB  design  and  estimating  RθJA  for  a  given
application environment:

• Thermal Design by Insight not Hindsight Application Report
• A Guide to Board Layout for Best Thermal Resistance for Exposed Pad Packages Application Report
• Semiconductor and IC Package Thermal Metrics Application Report
• Thermal Design Made Simple with LM43603 and LM43602 Application Report
• PowerPAD™ Thermally Enhanced Package Application Report
• PowerPAD™ Made Easy Application Report
• Using New Thermal Metrics Application Report
• PCB Thermal Calculator

34

Submit Document Feedback

Copyright © 2022 Texas Instruments Incorporated

Product Folder Links: LMR36506

PCB Copper Area (mm2)RTJA (qC/W)0100020003000400050006000406080100120140160180200220Rthe2 Layer, 0.5W4 Layer, 0.5Wwww.ti.com.cn

9.2.3 Application Curves

LMR36506
ZHCSKN9B – DECEMBER 2019 – REVISED DECEMBER 2022

Unless otherwise specified the following conditions apply: VIN = 24V, TA = 25°C.

LMR36506R3

VOUT = 3.3 V Fixed

1 MHz (AUTO)

图 9-4. Efficiency

LMR36506R

VOUT = 3.3 V Fixed

1 MHz (AUTO)

图 9-5. Line and Load Regulation

LMR36506R

VOUT = 5 V

400 kHz (AUTO)

LMR36506R

VOUT = 5 V

400 kHz (AUTO)

图 9-6. Efficiency

图 9-7. Line and Load Regulation

LMR36506R5

VOUT = 5 V Fixed

1 MHz (AUTO)

LMR36506R5

VOUT =5 V

1 MHz (AUTO)

图 9-8. Efficiency

图 9-9. Line and Load Regulation

Copyright © 2022 Texas Instruments Incorporated

Submit Document Feedback

35

Product Folder Links: LMR36506

Load Current (A)Efficiency (%)010203040506070809010010P100P1m10m100m1LMR3VIN = 12VVIN = 24VVIN = 36VVIN = 48VVIN = 54VLoadSetPoint(A)Output Voltage (V)00.10.20.30.40.50.63.33.3053.313.3153.323.3253.333.3353.34LoadVIN = 12VVIN = 24VVIN = 36VVIN = 48VVIN = 54VLoad Current (A)Efficiency (%)010203040506070809010010P100P1m10m100m1LMR3VIN = 12VVIN = 24VVIN = 36VVIN = 48VVIN = 54VLoad Current (A)Output Voltage (V)00.10.20.30.40.50.65.025.035.045.055.065.075.085.095.1LoadVIN = 12VVIN = 24VVIN = 36VVIN = 48VVIN = 54VLoad Current (A)010203040506070809010010P100P1m10m100mLMR3VIN = 12VVIN = 24VVIN = 36VVIN = 48VVIN = 54VLoad Current (A)00.10.20.30.40.50.655.015.025.035.045.055.065.075.08LoadVIN = 12VVIN = 24VVIN = 36VVIN = 48VVIN = 54VLMR36506
ZHCSKN9B – DECEMBER 2019 – REVISED DECEMBER 2022

www.ti.com.cn

LMR36506R3

VOUT = 3.3 V Fixed

1 MHz (AUTO)

LMR36506R

VOUT = 5 V

400 kHz (AUTO)

图 9-10. Dropout

图 9-11. Dropout

LMR36506R3

VOUT = 3.3 V Fixed

1 MHz

LMR36506R3

VOUT = 3.3 V Fixed

1 MHz

0 A to 0.6 A,1 A/µs

图 9-12. Load Transient

0.3 A to 0.6 A,1

A/µs

图 9-13. Load Transient

LMR36506R

VOUT = 5 V Fixed

400 kHz

LMR36506R

VOUT = 5 V Fixed

400 kHz

0 A to 0.6 A,1 A/µs

图 9-14. Load Transient

0.3 A to 0.6 A,1
A/µs

图 9-15. Load Transient

36

Submit Document Feedback

Copyright © 2022 Texas Instruments Incorporated

Product Folder Links: LMR36506

Input Voltage (V)Output Voltage (V)3.003.253.503.754.004.254.504.755.002.402.552.702.853.003.153.303.45DropIOUT = 0AIOUT = 0.3AIOUT = 0.6AInput Voltage (V)Output Voltage (V)3.03.54.04.55.05.56.06.57.02.53.03.54.04.55.05.5DropIOUT = 0AIOUT = 0.3AIOUT = 0.6ALoad Current (0.5A/DIV)200µs/DIVVOUT (200mV/DIV) 3.3V Load Current (0.5A/DIV)200µs/DIVVOUT (100mV/DIV) 3.3V Load Current (0.5A/DIV)200µs/DIVVOUT (200mV/DIV) 5V Load Current (0.5A/DIV)200µs/DIVVOUT (100mV/DIV) 5V www.ti.com.cn

LMR36506
ZHCSKN9B – DECEMBER 2019 – REVISED DECEMBER 2022

LMR36506R3

VOUT = 3.3V Fixed

No Load

LMR36506R

VOUT = 5V

图 9-16. Output Ripple

Load = 0.6A

400kHz

图 9-17. Output Ripple

图 9-18. Schematic for Typical Application Curves

表 9-3. BOM for Typical Application Curves

U1

ƒSW

VOUT

L

LMR36506R3RPER

1000 kHz

3.3 V (Fixed)

LMR36506RRPER

400 kHz

5 V

LMR36506R5RPER

1000 kHz

5 V (Fixed)

15 µH, 260
mΩ

47 µH, 68.4
mΩ

22 µH, 99.65
mΩ

NOMINAL COUT
(RATED CAPACITANCE)

2 × 22 µF

RT pin

RFBT

RFBB

Short to
VCC

0 Ω

Open

2× 22 µF

39.2 kΩ 100 kΩ

24.9 kΩ

2 x 22 µF

Short to
VCC

0 Ω

Open

Copyright © 2022 Texas Instruments Incorporated

Submit Document Feedback

37

Product Folder Links: LMR36506

Inductor Current (200mA/DIV)5ms/DIVVOUT (20mV/DIV) Inductor Current (500mA/DIV)2µs/DIVVOUT (10mV/DIV) VINENVCCSWBOOTFBVINVOUTLCBOOTCOUTCINCVCC2.2 µF0.1 µF1 µFRTLMR36506CHF100 nFRFBTRFBBGNDPGRTLMR36506
ZHCSKN9B – DECEMBER 2019 – REVISED DECEMBER 2022

9.3 Best Design Practices

www.ti.com.cn

• Do not exceed the Absolute Maximum Ratings.
• Do not exceed the Recommended Operating Conditions.
• Do not exceed the ESD Ratings.
• Do not allow the EN input to float.
• Do not allow the output voltage to exceed the input voltage, nor go below ground.
• Follow all the guidelines and suggestions found in this data sheet before committing the design to production.
TI application engineers are ready to help critique your design and PCB layout to help make your project a
success.

9.4 Power Supply Recommendations

The  characteristics  of  the  input  supply  must  be  compatible  with 节  7  found  in  this  data  sheet.  In  addition,  the
input supply must be capable of delivering the required input current to the loaded regulator. The average input
current can be estimated with 方程式 14.

(14)

where

• η is the efficiency

If  the  regulator  is  connected  to  the  input  supply  through  long  wires  or  PCB  traces,  special  care  is  required  to
achieve  good  performance.  The  parasitic  inductance  and  resistance  of  the  input  cables  can  have  an  adverse
effect on the operation of the regulator. The parasitic inductance, in combination with the low-ESR, ceramic input
capacitors,  can  form  an  underdamped  resonant  circuit,  resulting  in  overvoltage  transients  at  the  input  to  the
regulator.  The  parasitic  resistance  can  cause  the  voltage  at  the  VIN  pin  to  dip  whenever  a  load  transient  is
applied to the output. If the application is operating close to the minimum input voltage, this dip can cause the
regulator to momentarily shut down and reset. The best way to solve these kind of issues is to limit the distance
from the input supply to the regulator or plan to use an aluminum or tantalum input capacitor in parallel with the
ceramics.  The  moderate  ESR  of  these  types  of  capacitors  help  dampen  the  input  resonant  circuit  and  reduce
any overshoots. A value in the range of 20 µF to 100 µF is usually sufficient to provide input damping and help to
hold the input voltage steady during large load transients.

Sometimes,  for  other  system  considerations,  an  input  filter  is  used  in  front  of  the  regulator.  This  can  lead  to
instability, as well as some of the effects mentioned above, unless it is designed carefully. The AN-2162 Simple
Success  With  Conducted  EMI  From  DC/DC  Converters  User's  Guide  provides  helpful  suggestions  when
designing an input filter for any switching regulator.

In some cases, a transient voltage suppressor (TVS) is used on the input of regulators. One class of this device
has  a  snap-back  characteristic  (thyristor  type).  The  use  of  a  device  with  this  type  of  characteristic  is  not
recommended. When the TVS fires, the clamping voltage falls to a very low value. If this voltage is less than the
output  voltage  of  the  regulator,  the  output  capacitors  discharge  through  the  device  back  to  the  input.  This
uncontrolled current flow can damage the device.

9.5 Layout
9.5.1 Layout Guidelines

The PCB layout of any DC/DC converter is critical to the optimal performance of the design. Poor PCB layout
can disrupt the operation of an otherwise good schematic design. Even if the converter regulates correctly, bad
PCB  layout  can  mean  the  difference  between  a  robust  design  and  one  that  cannot  be  mass  produced.
Furthermore, to a great extent, the EMI performance of the regulator is dependent on the PCB layout. In a buck
converter,  the  most  critical  PCB  feature  is  the  loop  formed  by  the  input  capacitor  or  capacitors  and  power
ground, as shown in  图 9-19. This loop carries large transient currents that can cause large transient voltages
when reacting with the trace inductance. These unwanted transient voltages disrupt the proper operation of the
converter. Because of this, the traces in this loop must be wide and short, and the loop area as small as possible

38

Submit Document Feedback

Copyright © 2022 Texas Instruments Incorporated

Product Folder Links: LMR36506

K˜˜ INOUTOUTINVIVIwww.ti.com.cn

LMR36506
ZHCSKN9B – DECEMBER 2019 – REVISED DECEMBER 2022

to  reduce  the  parasitic  inductance.  图  9-20  shows  a  recommended  layout  for  the  critical  components  of  the
LMR36506.

1. Place the input capacitors as close as possible to the VIN and GND terminals.
2. Place bypass capacitor for VCC close to the VCC pin. This capacitor must be placed close to the device and

routed with short, wide traces to the VCC and GND pins.

3. Use wide traces for the CBOOT capacitor. Place CBOOT close to the device with short/wide traces to the
BOOT and SW pins. Route the SW pin to the N/C pin and used to connect the BOOT capacitor to SW.
4. Place the feedback divider as close as possible to the FB pin of the device. Place RFBB, RFBT, and CFF, if

used, physically close to the device. The connections to FB and GND must be short and close to those pins
on the device. The connection to VOUT can be somewhat longer. However, the latter trace must not be
routed near any noise source (such as the SW node) that can capacitively couple into the feedback path of
the regulator.

5. Use at least one ground plane in one of the middle layers. This plane acts as a noise shield and as a heat

dissipation path.

6. Provide wide paths for VIN, VOUT, and GND. Making these paths as wide and direct as possible reduces

any voltage drops on the input or output paths of the converter and maximizes efficiency.

7. Provide enough PCB area for proper heat-sinking. As stated in 节 9.2.2.9, enough copper area must be used
to ensure a low RθJA, commensurate with the maximum load current and ambient temperature. The top and
bottom PCB layers must be made with two ounce copper and no less than one ounce. If the PCB design
uses multiple copper layers (recommended), these thermal vias can also be connected to the inner layer
heat-spreading ground planes.

8. Keep switch area small. Keep the copper area connecting the SW pin to the inductor as short and wide as
possible. At the same time, the total area of this node must be minimized to help reduce radiated EMI.

See the following PCB layout resources for additional important guidelines:

• Layout Guidelines for Switching Power Supplies Application Report
• Simple Switcher PCB Layout Guidelines Application Report
• Construction Your Power Supply- Layout Considerations Seminar
• Low Radiated EMI Layout Made Simple with LM4360x and LM4600x Application Report

图 9-19. Current Loops with Fast Edges

9.5.1.1 Ground and Thermal Considerations

As  previously  mentioned,  TI  recommends  using  one  of  the  middle  layers  as  a  solid  ground  plane.  A  ground
plane  provides  shielding  for  sensitive  circuits  and  traces  as  well  as  a  quiet  reference  potential  for  the  control
circuitry. Connect the GND pin to the ground planes using vias next to the bypass capacitors. The GND trace, as
well  as  the  VIN  and  SW  traces,  must  be  constrained  to  one  side  of  the  ground  planes.  The  other  side  of  the
ground plane contains much less noise; use for sensitive routes.

Copyright © 2022 Texas Instruments Incorporated

Submit Document Feedback

39

Product Folder Links: LMR36506

CINVINSWGNDLMR36506
ZHCSKN9B – DECEMBER 2019 – REVISED DECEMBER 2022

www.ti.com.cn

TI  recommends  providing  adequate  device  heat-sinking  by  having  enough  copper  near  the  GND  pin.  See  图
9-20  for  example  layout.  Use  as  much  copper  as  possible,  for  system  ground  plane,  on  the  top  and  bottom
layers for the best heat dissipation. Use a four-layer board with the copper thickness for the four layers, starting
from the top as: 2 oz / 1 oz / 1 oz / 2 oz. A four-layer board with enough copper thickness, and proper layout,
provides low current conduction impedance, proper shielding and lower thermal resistance.

9.5.2 Layout Example

图 9-20. Example Layout

40

Submit Document Feedback

Copyright © 2022 Texas Instruments Incorporated

Product Folder Links: LMR36506

CINCOUTL1CINGNDCVCCCBOOTRFBTCFFRFBBRENTRENBwww.ti.com.cn

10 Device and Documentation Support
10.1 Documentation Support
10.1.1 Related Documentation

LMR36506
ZHCSKN9B – DECEMBER 2019 – REVISED DECEMBER 2022

For related documentation see the following:
• Texas Instruments, Thermal Design by Insight not Hindsight Application Report
• Texas Instruments, A Guide to Board Layout for Best Thermal Resistance for Exposed Pad Packages

Application Report

• Texas Instruments, Semiconductor and IC Package Thermal Metrics Application Report
• Texas Instruments, Thermal Design Made Simple with LM43603 and LM43602 Application Report
• Texas Instruments, PowerPAD™ Thermally Enhanced Package Application Report
• Texas Instruments, PowerPAD™ Made Easy Application Report
• Texas Instruments, Using New Thermal Metrics Application Report
• Texas Instruments, Layout Guidelines for Switching Power Supplies Application Report
• Texas Instruments, Simple Switcher PCB Layout Guidelines Application Report
• Texas Instruments, Construction Your Power Supply- Layout Considerations Seminar
• Texas Instruments, Low Radiated EMI Layout Made Simple with LM4360x and LM4600x Application Report

10.2 接收文档更新通知

要接收文档更新通知，请导航至 ti.com 上的器件产品文件夹。点击订阅更新 进行注册，即可每周接收产品信息更
改摘要。有关更改的详细信息，请查看任何已修订文档中包含的修订历史记录。

10.3 支持资源

TI E2E™ 支持论坛是工程师的重要参考资料，可直接从专家获得快速、经过验证的解答和设计帮助。搜索现有解
答或提出自己的问题可获得所需的快速设计帮助。

链接的内容由各个贡献者“按原样”提供。这些内容并不构成 TI 技术规范，并且不一定反映 TI 的观点；请参阅
TI 的《使用条款》。

10.4 Trademarks
HotRod™, PowerPAD™, and TI E2E™ are trademarks of Texas Instruments.
is a trademark of TI.
所有商标均为其各自所有者的财产。
10.5 Electrostatic Discharge Caution

This integrated circuit can be damaged by ESD. Texas Instruments recommends that all integrated circuits be handled
with appropriate precautions. Failure to observe proper handling and installation procedures can cause damage.

ESD damage can range from subtle performance degradation to complete device failure. Precision integrated circuits may
be more susceptible to damage because very small parametric changes could cause the device not to meet its published
specifications.

10.6 术语表

TI 术语表

本术语表列出并解释了术语、首字母缩略词和定义。

Copyright © 2022 Texas Instruments Incorporated

Submit Document Feedback

41

Product Folder Links: LMR36506

LMR36506
ZHCSKN9B – DECEMBER 2019 – REVISED DECEMBER 2022

www.ti.com.cn

11 Mechanical, Packaging, and Orderable Information

The  following  pages  include  mechanical,  packaging,  and  orderable  information.  This  information  is  the  most
current data available for the designated devices. This data is subject to change without notice and revision of
this document. For browser-based versions of this data sheet, refer to the left-hand navigation.

42

Submit Document Feedback

Copyright © 2022 Texas Instruments Incorporated

Product Folder Links: LMR36506

www.ti.com

16-Dec-2022

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

MSL Peak Temp

Op Temp (°C)

Device Marking

Samples

(3)

(4/5)

LMR36506R3RPER

ACTIVE

VQFN-HR

RPE

LMR36506R5RPER

ACTIVE

VQFN-HR

RPE

LMR36506RF3RPER

ACTIVE

VQFN-HR

RPE

LMR36506RFRPER

ACTIVE

VQFN-HR

RPE

LMR36506RRPER

ACTIVE

VQFN-HR

RPE

9

9

9

9

9

3000 RoHS & Green

3000 RoHS & Green

3000 RoHS & Green

3000 RoHS & Green

3000 RoHS & Green

(6)

SN

SN

SN

SN

SN

Level-2-260C-1 YEAR

-40 to 150

MC02

Level-2-260C-1 YEAR

-40 to 150

MC07

Level-2-260C-1 YEAR

-40 to 150

MC03

Level-2-260C-1 YEAR

-40 to 150

MC01

Level-2-260C-1 YEAR

-40 to 150

MC04

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

16-Dec-2022

PACKAGE OPTION ADDENDUM

Important Information and Disclaimer:The information provided on this page represents TI's knowledge and belief as of the date that it is provided. TI bases its knowledge and belief on information
provided by third parties, and makes no representation or warranty as to the accuracy of such information. Efforts are underway to better integrate information from third parties. TI has taken and
continues to take reasonable steps to provide representative and accurate information but may not have conducted destructive testing or chemical analysis on incoming materials and chemicals.
TI and TI suppliers consider certain information to be proprietary, and thus CAS numbers and other limited information may not be available for release.

In no event shall TI's liability arising out of such information exceed the total purchase price of the TI part(s) at issue in this document sold by TI to Customer on an annual basis.

 OTHER QUALIFIED VERSIONS OF LMR36506 :
• Automotive : LMR36506-Q1

 NOTE: Qualified Version Definitions:

• Automotive - Q100 devices qualified for high-reliability automotive applications targeting zero defects

Addendum-Page 2

www.ti.com

16-Dec-2022

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

LMR36506R3RPER

LMR36506R5RPER

LMR36506RF3RPER

LMR36506RFRPER

LMR36506RRPER

VQFN-
HR

VQFN-
HR

VQFN-
HR

VQFN-
HR

VQFN-
HR

RPE

RPE

RPE

RPE

RPE

9

9

9

9

9

3000

180.0

8.4

2.2

2.2

1.2

4.0

8.0

3000

180.0

8.4

2.2

2.2

1.2

4.0

8.0

3000

180.0

8.4

2.2

2.2

1.2

4.0

8.0

3000

180.0

8.4

2.2

2.2

1.2

4.0

8.0

3000

180.0

8.4

2.2

2.2

1.2

4.0

8.0

Q2

Q2

Q2

Q2

Q2

Pack Materials-Page 1

PACKAGE MATERIALS INFORMATION

www.ti.com

16-Dec-2022

TAPE AND REEL BOX DIMENSIONS

Width (mm)

H

W

L

*All dimensions are nominal

Device

Package Type

Package Drawing Pins

LMR36506R3RPER

LMR36506R5RPER

LMR36506RF3RPER

LMR36506RFRPER

LMR36506RRPER

VQFN-HR

VQFN-HR

VQFN-HR

VQFN-HR

VQFN-HR

RPE

RPE

RPE

RPE

RPE

9

9

9

9

9

SPQ

3000

3000

3000

3000

3000

Length (mm) Width (mm) Height (mm)

195.0

195.0

195.0

195.0

195.0

200.0

200.0

200.0

200.0

200.0

45.0

45.0

45.0

45.0

45.0

Pack Materials-Page 2

RPE 9
2 x 2, 0.5 mm pitch

GENERIC PACKAGE VIEW

VQFN-HR - 1.0 mm max height

PLASTIC QUAD FLATPACK - NO LEAD

This image is a representation of the package family, actual package may vary.
Refer to the product data sheet for package details.

4227057/A

www.ti.com

PACKAGE OUTLINE

VQFN-HR - 1.0 mm max height

PLASTIC QUAD FLATPACK - NO LEAD

RPE0009B

SCALE  6.000

2.1
1.9

B

PIN 1 INDEX AREA

A

2.1
1.9

1.0
0.8

0.05
0.00

2X

0.55
0.45

2X  0.738

2X  0.25

0

.000 PKG

2X  0.25

2X  0.738

4

1

4X (0.15)

5

8

PIN 1 ID

2X

0.6
0.5

9

0.4
0.3

C

SEATING PLANE

0.08 C

4X (0.15)

4X

0.275
0.175
0.1
0.05

C A B

C

(0.1) TYP

1.1 0.05

4X

0.275
0.175
0.1
0.05

C A B

C

4X

0.45
0.35

NOTES:

1. All linear dimensions are in millimeters. Any dimensions in parenthesis are for reference only. Dimensioning and tolerancing
    per ASME Y14.5M.
2. This drawing is subject to change without notice.

4227033/A   08/2021

www.ti.com

RPE0009B

EXAMPLE BOARD LAYOUT

VQFN-HR - 1.0 mm max height

PLASTIC QUAD FLATPACK - NO LEAD

2X (0.75)

2X (0.4)

(0.35)

9

4X (0.225)

2X (

0.738

)

4X (0.6)

4X (0.25)

2X (

0.25

)

0

.000 PKG

2X (

0.25

)

(R0.05) TYP

2X (

0.738

)

1

4

SYMM

(1.8)

LAND PATTERN EXAMPLE
EXPOSED METAL SHOWN
SCALE: 30X

2X (0.575)

(1.3)

(

0.55

)

SEE SOLDER MASK
DETAILS

8

5

2X (0.575)

2X (0.7)

0.05 MAX
ALL AROUND

0.05 MIN
ALL AROUND

EXPOSED
METAL

METAL EDGE

EXPOSED
METAL

METAL UNDER
SOLDER MASK

SOLDER MASK
OPENING

SOLDER MASK
OPENING

NON SOLDER MASK
DEFINED
(PREFERRED)

SOLDER MASK
DEFINED

SOLDER MASK DETAILS

NOTES: (continued)

3. For more information, see Texas Instruments literature number SLUA271 (www.ti.com/lit/slua271).
4. Vias are optional depending on application, refer to device data sheet. If any vias are implemented, refer to their locations shown
    on this view. It is recommended that vias under paste be filled, plugged or tented.

4227033/A   08/2021

www.ti.com

RPE0009B

EXAMPLE STENCIL DESIGN

VQFN-HR - 1.0 mm max height

PLASTIC QUAD FLATPACK - NO LEAD

2X (0.75)

2X (0.55)

2X
(0.4)

2X (0.35)

9

4X (0.225)

2X (

0.738

)

4X (0.6)

4X (0.25)

2X (

0.25

)

0

.000 PKG

2X (

0.25

)

(R0.05) TYP

2X (

0.738

)

1

4

2X (0.5)

(

0.925

)

(

0.175

)

8

5

2X (0.575)

2X (0.7)

SYMM

(1.8)

SOLDER PASTE EXAMPLE
BASED ON 0.125 MM THICK STENCIL
SCALE: 30X

PADS 1 & 8:
90% PRINTED SOLDER COVERAGE BY AREA UNDER PACKAGE
PAD 9:
85% PRINTED SOLDER COVERAGE BY AREA UNDER PACKAGE

NOTES: (continued)

5. Laser cutting apertures with trapezoidal walls and rounded corners may offer better paste release. IPC-7525 may have alternate
   design recommendations.

DWG_NO:5/REV:5   MM_YYYY:5

www.ti.com

PACKAGE OUTLINE

VQFN-HR - 1.0 mm max height

PLASTIC QUAD FLATPACK - NO LEAD

RPE0009A

SCALE  6.000

2.1
1.9

B

PIN 1 INDEX AREA

A

2.1
1.9

0.1 MIN

(0.2) TYP

(0.05)

A-A 40.000

SECTION A-A
TYPICAL

C

SEATING PLANE

0.08 C

4X (0.15)

4X

0.275
0.175
0.1
0.05

C A B

C

1.1 0.05

5

A

8

4X

0.275
0.175
0.1
0.05

C A B

C

4X

0.45
0.35

1.0
0.8

0.05
0.00

2X

0.55
0.45

2X  0.738

2X  0.25

0

.000 PKG

2X  0.25

2X  0.738

4

1

PIN 1 ID
0.6
0.5

2X

0.4
0.3

4X (0.15)

A

9

NOTES:

1. All linear dimensions are in millimeters. Any dimensions in parenthesis are for reference only. Dimensioning and tolerancing
    per ASME Y14.5M.
2. This drawing is subject to change without notice.

4224447/B   01/2022

www.ti.com

RPE0009A

EXAMPLE BOARD LAYOUT

VQFN-HR - 1.0 mm max height

PLASTIC QUAD FLATPACK - NO LEAD

2X (0.75)

2X (0.4)

(0.35)

9

4X (0.225)

1

2X (

0.738

)

4X (0.6)

4X (0.25)

2X (

0.25

)

0

.000 PKG

2X (

0.25

)

(R0.05) TYP

2X (

0.738

)

4

2X (0.35)

SYMM

(1.8)

LAND PATTERN EXAMPLE
EXPOSED METAL SHOWN
SCALE: 30X

2X (0.575)

(1.3)

(

0.55

)

SEE SOLDER MASK
DETAILS

8

5

2X (0.575)

2X (0.7)

0.05 MAX
ALL AROUND

0.05 MIN
ALL AROUND

EXPOSED
METAL

METAL EDGE

EXPOSED
METAL

METAL UNDER
SOLDER MASK

SOLDER MASK
OPENING

SOLDER MASK
OPENING

NON SOLDER MASK
DEFINED
(PREFERRED)

SOLDER MASK
DEFINED

SOLDER MASK DETAILS

NOTES: (continued)

3. For more information, see Texas Instruments literature number SLUA271 (www.ti.com/lit/slua271).
4. Vias are optional depending on application, refer to device data sheet. If any vias are implemented, refer to their locations shown
    on this view. It is recommended that vias under paste be filled, plugged or tented.

4224447/B   01/2022

www.ti.com

RPE0009A

EXAMPLE STENCIL DESIGN

VQFN-HR - 1.0 mm max height

PLASTIC QUAD FLATPACK - NO LEAD

2X (0.75)

2X (0.55)

2X
(0.4)

2X (0.35)

9

4X (0.225)

2X (

0.738

)

4X (0.6)

4X (0.25)

2X (

0.25

)

0

.000 PKG

2X (

0.25

)

(R0.05) TYP

2X (

0.738

)

1

4

2X (0.35)

SYMM

(1.8)

2X (0.5)

(

0.925

)

(

0.175

)

8

5

2X (0.575)

2X (0.7)

SOLDER PASTE EXAMPLE
BASED ON 0.125 MM THICK STENCIL
SCALE: 30X

PADS 1 & 8:
90% PRINTED SOLDER COVERAGE BY AREA UNDER PACKAGE
PAD 9:
85% PRINTED SOLDER COVERAGE BY AREA UNDER PACKAGE

NOTES: (continued)

5. Laser cutting apertures with trapezoidal walls and rounded corners may offer better paste release. IPC-7525 may have alternate
   design recommendations.

DWG_NO:5/REV:5   MM_YYYY:5

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

