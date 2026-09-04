# Albabot 동작 원리

Albabot이 조이스틱 입력부터 실제 바퀴 구동까지 어떻게 동작하는지, 코드 기준으로 정리한 문서.

> 핵심 요약: **ROS(미니 PC) = 입력 해석기 + 명령 생성기**, **펌웨어(STM32) = 실제 모터/CAN 제어기**.
> 미니 PC는 직접 모터를 돌리지 않고, 조이스틱 입력을 두 갈래로 변환해 시리얼로 펌웨어에 넘긴다.

## 전체 데이터 흐름

```
[조이스틱] ──/joy──> [driving_m1 (AGVTeleop)] ──/cmd_vel (모터 L/R)──┐
                            │                                        ├──rosserial──> [MainController 펌웨어]
                            └──/AlbabotMessage (CanMsg)──────────────┘   (STM32, ttyUSB-Albabot@921600)
                                                                              │
[라이다] ──/obstacle──────────────────────────────────────────────────────────┤ (장애물 안전정지)
                                                                              ▼
[MainController] ──CAN 버스──> [BLDC 모터 컨트롤러] → 바퀴
       └──/robot_info (엔코더·초음파·홀·배터리·AGV미션상태)──> ROS/UI
```

- 관련 노드: `ad1_driving/driving_m1` (핵심 주행), `joy_node` (조이스틱), `serial_node.py` (rosserial 브리지), `ad1_socket/ad1_ui_socket` (UI 통신)
- 펌웨어: `fw/*/Albabot_MainController.ino` (rosserial 클라이언트)

## 데이터 모델 (`albabot_msgs`)

| 타입 | 필드 | 용도 |
|------|------|------|
| `CanMsg` | `id`, `dlc`, `data[]` | CAN 버스 프레임 (명령 전달 단위) |
| `RobotInfo` | `left_enc`/`right_enc`, `sonic[]`, `hall[]`, `batVoltage`, `agvStatus`, `agvDirection`, `agvCurrentMission`, `agvMissionCount`, `gio` | 로봇 → ROS 상태 보고 |
| `Command.srv` | `CanMsg` → `CanMsg` + `result` | CAN 명령 요청/응답 서비스 |

## 두 가지 동작 모드 (`driving_m1` / `AGVTeleop`)

`driving_m1` 노드는 `/joy`를 구독하고 20Hz로 `AGVTeleop::Publish()`를 돌린다. 모드는 조이스틱 Mode 버튼으로 토글.

### ① Remote 모드 — 수동 주행
- `JoyCB`: 스틱 축을 읽음
  - `axes[2]`(스로틀) → 속도 스케일 (최대 3.3)
  - `axes[1]` → 전후진 V, `axes[3]` → 회전 W
- `GetVelocity`: 차동구동 역기구학으로 좌우 바퀴 속도 계산
  - `vel_L = -(V - d·W)`, `vel_R = -(V + d·W)` (d = 축간거리)
- `/cmd_vel`로 `[L, R]` 발행 → 펌웨어 `Velocity_cb`가 모터 반영
- **1초 타임아웃**(`m_u32TimeoutCnt`): 조이스틱 신호가 끊기면 속도 0으로 자동 정지 (안전장치)

### ② AGV 모드 — 자석 유도선 추종 (단위동작)
- `/cmd_vel`을 쓰지 않음. 버튼 이벤트를 **CAN 명령**(`/AlbabotMessage`)으로 펌웨어에 전달
- `CheckButton`: 버튼 **누름→뗌 엣지**를 검출해 단위동작 트리거 (연타/눌림유지 방지)
- 실제 경로 주행(유도선 따라가기)은 펌웨어/모터컨트롤러가 수행. ROS는 "다음 노드로", "좌회전" 같은 상위 명령만 던진다.

## CAN 명령 프로토콜 (id `0x100`, `data[1]` = 서브커맨드)

| `data[1]` | 의미 | 트리거 |
|-----------|------|--------|
| `0x31` | 모드 전환 (0=Remote, 1=AGV) | Mode 버튼 |
| `0x01` | Next (다음 노드로 직진) | Next 버튼 |
| `0x02` | 좌회전 | Left |
| `0x03` | 우회전 | Right |
| `0x04` | U턴 | Turn |
| `0xff` | 현재 명령 취소 | Cancel |
| `0x40` | 속도 설정 (float, 4바이트) | Speed Up/Down |
| `0x45` / `0x46` | 감속비 / 축간거리 | InitRobot |
| `0x48` / `0x49` / `0x4a` | PID Kp / Ki / Kd | InitRobot |
| `0x42` | 가속도 | InitRobot |

float 파라미터는 `FLOAT_DATA` union으로 4바이트로 쪼개 `data[2..5]`에 실어 보낸다.

### 부팅 초기화 (`InitRobot`)
부팅 시 속도 → PID(kp/ki/kd) → 감속비 → 축간거리 → 가속도 순으로 CAN 명령을 순차 전송해 펌웨어를 세팅한다.
**launch 파일의 `<arg>` 값(모델별 기어비 30:1↔50:1, PID, 축간거리 등)이 실제 로봇에 주입되는 경로가 바로 이것.**

## 조이스틱 버튼 매핑

| 기능 | joy_type=0 (유선) | joy_type=1 (무선) |
|------|------------------|------------------|
| Mode | button[4] | button[4] |
| Next | button[1] | button[3] |
| Turn (U턴) | button[0] | button[0] |
| Left | button[2] | button[2] |
| Right | button[3] | button[1] |
| Cancel | button[5] | button[5] |
| 속도 ↑/↓ | axes[5] | axes[7] |

## 펌웨어 인터페이스 (`Albabot_MainController.ino`, rosserial)

- **구독**: `AlbabotMessage`(CanMsg → `GetMessageCB`), `cmd_vel`(Float32MultiArray → `Velocity_cb`), `obstacle`(→ `Obstacle_cb`, 안전정지)
- **발행**: `robot_info`(RobotInfo) — 엔코더·초음파·홀·배터리·AGV 미션상태
- `GetMessageCB`가 CAN 명령을 해석해 `CANsend()`로 CAN 버스에 전달 → 모터 컨트롤러 구동

## 실행

```bash
# 하드웨어 udev 링크 (최초 1회, root): ttyUSB-Albabot 등
sudo robots/albabot_t0001/script/startup.bash

# A0001 조이스틱 주행 (AGV / Remote)
roslaunch albabot_base_pkg albabot_a0001.launch
#  = joy_node + driving_m1(파라미터) + ad1_ui_socket + serial_node(→ /dev/ttyUSB-Albabot @921600)
```

## 요약

- **ROS 계층**: 조이스틱 해석 → (Remote) 속도 명령 `/cmd_vel` 또는 (AGV) 단위동작 CAN 명령 `/AlbabotMessage` 생성
- **시리얼 브리지**: rosserial(`serial_node.py`)이 두 토픽을 STM32로 전달
- **펌웨어 계층**: CAN 버스로 BLDC 모터 구동, 센서값을 `/robot_info`로 역보고
- **안전장치**: 조이스틱 1초 타임아웃 정지, 라이다 `/obstacle` 정지

---
*작성 기준: `ad1_driving/{driving_m1.cpp, agvteleop.{h,cpp}}`, `albabot_msgs`, `fw/*/Albabot_MainController.ino` (albabot_a0001 기준). 다른 모델(s000x, rovitek AD1)도 동일 골격.*
