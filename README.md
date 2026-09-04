# Albabot

로비텍(Rovitek) **Albabot** 로봇 계열의 ROS 코드 아카이브. 2020년 미니 PC(`swpark-dev-1`, 192.168.0.23)에서 개발된 여러 모델의 워크스페이스를 소스만 정리해 모은 단일 저장소.

- **플랫폼**: ROS Melodic (ROS1) / Ubuntu 18.04
- **운용**: 조이스틱 기반 2-모드 주행 (AGV 유도선 추종 / Remote 수동) — `docs/Albabot_사용자설명서.docx`
- 빌드 산출물(`build/ devel/ logs/ .catkin_tools/`)과 기존 개별 `.git` 이력은 제외하고 소스만 담음.

## 저장소 구조

```
robots/                        모델별 워크스페이스 (소스만)
  albabot_t0001/               T0001 — 텔레프레즌스(열화상+RealSense, 웹 UI). 미니 PC의 활성 워크스페이스
    ros02/src/                   tb2_base_pkg, qtRobot, qtgui_user, ad1_driving, albabot_msgs ...
    ros/src/                     이전(ros) 버전
    ui/  ui_backup(제외)         node.js 기반 텔레프레즌스 UI
    fw/                          펌웨어(tp1_mainCon, alarm_led / Arduino)
    script/                      startup.bash(udev 룰), telebot_autostart.bash
  albabot_a0001/               A0001 — AGV/자율주행 (AD1 베이스, 기어 30:1, 네비게이션/RTAB-Map/AMCL)
  albabot_s0001/, albabot_s0002/  S000x — 서비스 AGV 변종
  rovitek_albabot_ws/          AD1 통합 — 순찰(patrol)/매핑/teleop/사운드/소켓
scripts/                       홈에 있던 실행 스크립트 (autostart, navigation start 등)
docs/                          사용자설명서(.docx), maps/(pgm+yaml)
ops/                           미니 PC 역터널 세팅 (저장소 운영용, 로봇과 무관)
```

## 공통 패키지 (여러 모델이 재사용)

| 패키지 | 역할 |
|--------|------|
| `ad1_driving` (`driving_m1`) | 모터 제어 노드. 기어비/PID/속도/휠간거리 파라미터로 모델별 튜닝 |
| `ad1_sensor` | 오도메트리, 라이다 장애물 감지 |
| `ad1_socket` (`ad1_ui_socket`) | UI 통신 소켓 |
| `albabot_msgs` | 커스텀 ROS 메시지 정의 |
| `albabot_base_pkg` / `tb*_base_pkg` | 모델별 베이스 브링업(센서/TF/시리얼) |

## 실행 방법 (모델별 최상위 브링업)

```bash
# 0) 하드웨어 udev 심볼릭 링크 (최초 1회, root)
#    ttyUSB-Albabot(모터), ttyUSB-Head, video_thermal 생성
sudo robots/albabot_t0001/script/startup.bash

# 환경: source /opt/ros/melodic/setup.bash + 해당 워크스페이스 devel/setup.bash
#       export AD1_MAP_NAME=<맵.yaml>  (네비게이션 시)

# T0001 텔레프레즌스 (미니 PC 부팅 시 autostart 대상)
roslaunch tb2_base_pkg tele_presence_manager.launch

# A0001 조이스틱 주행 (AGV / Remote 모드)
roslaunch albabot_base_pkg albabot_a0001.launch

# AD1 순찰 (rplidar + 사운드 + 패트롤)
roslaunch albabot_ad1_base patrol_control.launch

# 네비게이션 (map_server + AMCL + rviz)
roslaunch ad1_navigation ad1_navigation.launch
```

## 조이스틱 운용 (사용자설명서 요약)

1. 로봇 전원 ON → 조이스틱 전원 → **약 3분 부팅**
2. **AGV mode**: 바닥 자석 유도선 노드 위에 정렬 → Mode 스위치로 전환 → 단위동작(Next/left/right/turn/cancel)
3. **Remote mode**: 조이스틱 스틱으로 전후진·좌우, Up/Down 버튼으로 속도 조절
4. 종료: Albabot 전원 OFF (조이스틱 전원 자동 종료)

## 하드웨어 접점

- 모터 제어기: `rosserial_python serial_node.py` → `/dev/ttyUSB-Albabot` @ **921600**
- 라이다: `rplidar_ros` → `/dev/rplidar` @ 115200
- 조이스틱: `joy_node` → `/dev/input/js0`
- 카메라(T0001): RealSense(head/front) + USB 열화상(`/dev/video_thermal`, 160×120)

## 참고

- 각 워크스페이스는 미니 PC에서 개별 git 저장소였음(이력은 이 아카이브에 미포함).
- 모델 간 모터 파라미터(기어비 30:1↔50:1, PID, 휠간거리)가 launch `<arg>`로 갈리므로, 실물 매칭 시 해당 값 확인 필요.
- 미니 PC의 원래 `.bashrc`는 `ROS_MASTER_URI=http://172.16.200.158:11311`, `AD1_MAP_NAME=/home/swpark/mymap.yaml` 로 설정돼 있었음.
