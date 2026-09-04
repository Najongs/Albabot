# albabot_autoscan

라이다 없이 **RealSense D435**로 **저속 자율 공간스캔**. RGB-D SLAM(RTAB-Map) 매핑 +
카메라/초음파 기반 안전정지 + 반응형 탐색.

## 왜 라이다 없이 되나
D435의 depth를 `pointcloud_to_laserscan`으로 가상 2D 스캔(`/camera/scan`)으로 바꾸고,
RTAB-Map은 RGB-D+IMU로 매핑한다. 전방 사각(20cm 이내)은 초음파 4개로 보완.

## 구조 (안전 우선)
```
explorer  ──/scan_cmd[v,w]──>  safety_guard  ──/cmd_vel[L,R]──>  serial_node ──> 펌웨어 ──> 모터
   ▲                              ▲   ▲
/camera/scan                /camera/scan  /robot_info(초음파)
```
- **safety_guard 가 `/cmd_vel` 유일 발행자.** 장애물 임계거리 이내 / explorer 명령 끊김 /
  스캔 끊김 → 무조건 정지. explorer 는 절대 직접 모터를 못 건드린다.
- 펌웨어 `Obstacle_cb` 정지 로직은 비활성이라 안전은 전적으로 ROS(safety_guard)가 담당.
- 펌웨어 **하트비트 워치독**: `/cmd_vel` 이 끊기면 모터 자동정지(노드 크래시 대비 데드맨).

## 노드
| 노드 | 입력 | 출력 | 역할 |
|------|------|------|------|
| `safety_guard.py` | `/camera/scan`, `/robot_info`, `/camera/imu`, `/scan_cmd` | `/cmd_vel` | 다중센서 안전판정·정지·감속·하트비트 |
| `explorer.py` | `/camera/scan` | `/scan_cmd` | 저속 순항/회피 + 주기적 회전스캔 |
| `set_remote_mode.py` | — | `/AlbabotMessage` | 부팅 시 Remote 모드 + 미션취소 |

### safety_guard 다중 안전 레이어 (겹침, 하나라도 걸리면 정지)
| 레이어 | 센서 | 감지 |
|--------|------|------|
| 거리 | 카메라 `/camera/scan` | 전방 위험거리 정지 / 감속거리 감속 |
| 근접 | 초음파 `sonic[8]` | 진행방향(전진=전방4/후진=후방4) 근접 정지 |
| **접촉/끼임** | 엔코더 `left/right_enc` | 전진 명령 중 엔코더 정지 → 스톨 → 정지+후진 회피 |
| **충격/전복** | IMU `/camera/imu` | 가속도 스파이크(충돌) / 중력벡터 기울기(경사·전복) |
| 전원 | `batVoltage` | 저전압 정지 (단위 미확정, 기본 off) |
| 신호 | scan/cmd 끊김 | 입력 끊기면 정지 (+ 펌웨어 하트비트 데드맨) |

> 별도 범퍼/E-stop 배선 없이 **엔코더 스톨 + IMU 충격**으로 접촉감지. `hall[2]`는 용도
> 불명(자석선/전류 추정), `gio`는 펌웨어에서 미배선(0x0000)이라 현재 미사용.
> 각 레이어는 `config/autoscan.yaml` 에서 on/off·임계값 조정. 배터리/스톨 임계값은 실측 후 보정 필요.

파라미터: `config/autoscan.yaml` (거리 임계값·속도 상한 전부 보수적).

## 실행

### Phase 0 — 매핑만 (움직임 없음, 먼저 이걸로 검증)
```bash
roslaunch albabot_autoscan mapping.launch rviz:=true
# 로봇을 손으로 밀거나 조이스틱으로 천천히 이동 → /camera/scan, 맵 생성 확인
```

### 전체 자율스캔
```bash
# 안전 기본: 노드는 다 뜨되 로봇은 정지 상태
roslaunch albabot_autoscan autoscan.launch

# 준비되면(★ 첫 주행은 바퀴 거치=들고 상태에서) 탐색 활성화
roslaunch albabot_autoscan autoscan.launch explore:=true
```

## 안전 수칙
1. **첫 주행은 반드시 바퀴를 들고(거치)** `explore:=true` 로 좌우 바퀴 방향·정지 동작 확인.
2. 넓은 곳에서 최저속부터. `config/autoscan.yaml` 의 `forward_v`, `max_v` 를 작게.
3. 비상시 `Ctrl-C` → `/cmd_vel` 끊김 → 펌웨어 하트비트로 자동정지.
4. `stop_distance`(기본 0.6m) 안에 사람/물체 두고 실제 정지되는지 먼저 확인.

## 튜닝 포인트
- 너무 자주 멈추면 `stop_distance`↓ 또는 `front_angle_deg`↓
- 초음파 단위가 cm가 아니면 `ultrasonic_stop_cm`/`use_ultrasonic` 조정
- 카메라 마운트 위치가 다르면 `mapping.launch` 의 `base_to_camera` static TF 수정
