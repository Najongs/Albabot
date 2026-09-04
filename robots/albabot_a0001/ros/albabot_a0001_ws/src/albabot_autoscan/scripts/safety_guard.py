#!/usr/bin/env python
# -*- coding: utf-8 -*-
"""
safety_guard.py  -  albabot_autoscan 안전 계층 (/cmd_vel 유일 발행자)

여러 센서를 겹쳐 안전을 판단하고, 항상 control_rate 로 /cmd_vel 을 발행해
펌웨어 하트비트 워치독을 유지한다. explorer 는 /scan_cmd 로 "요청"만 하고,
실제 모터 명령은 이 노드만 낸다.

안전 레이어 (겹침, 하나라도 걸리면 정지):
  1) 카메라 /camera/scan  : 전방 거리 -> 위험거리 정지 / 감속거리 감속
  2) 초음파 robot_info.sonic : 전진=전방4 / 후진=후방4 근접 정지
  3) 엔코더 스톨          : 전진 명령 중인데 엔코더 정지 -> 접촉/끼임 -> 정지+후진
  4) IMU /camera/imu      : 충격(가속도 스파이크) / 기울기(경사·전복) -> 정지
  5) 배터리 batVoltage    : 저전압 -> 정지 (기본 비활성)
  6) explorer/스캔 신호 끊김 -> 정지

펌웨어 규약: /cmd_vel = Float32MultiArray[left, right], Remote 모드에서 적용.
좌우 변환(driving_m1 GetVelocity 동일 부호): left=-(v-d*w), right=-(v+d*w)
"""
import math
from collections import deque
import rospy
from std_msgs.msg import Float32MultiArray
from sensor_msgs.msg import LaserScan, Imu
from albabot_msgs.msg import RobotInfo


class SafetyGuard(object):
    def __init__(self):
        g = lambda k, d: rospy.get_param("~" + k, d)
        self.scan_topic       = g("scan_topic", "/camera/scan")
        self.robot_info_topic = g("robot_info_topic", "/robot_info")
        self.cmd_in_topic     = g("cmd_in_topic", "/scan_cmd")
        self.cmd_out_topic    = g("cmd_out_topic", "/cmd_vel")

        self.stop_distance = float(g("stop_distance", 0.60))
        self.slow_distance = float(g("slow_distance", 1.00))
        self.front_angle   = math.radians(float(g("front_angle_deg", 40.0)))

        # 초음파
        self.use_us       = bool(g("use_ultrasonic", True))
        self.us_stop_cm   = float(g("ultrasonic_stop_cm", 30.0))
        self.us_valid_max = float(g("ultrasonic_valid_max_cm", 250.0))
        self.us_front_idx = list(g("ultrasonic_front_idx", [0, 1, 2, 3]))
        self.us_rear_idx  = list(g("ultrasonic_rear_idx", [4, 5, 6, 7]))

        # 스톨(접촉) 감지
        self.use_stall      = bool(g("use_stall_detect", True))
        self.stall_sec      = float(g("stall_check_sec", 0.7))
        self.stall_min_cmd  = float(g("stall_min_cmd", 0.15))
        self.stall_enc_min  = float(g("stall_enc_min_delta", 20))
        self.backoff_sec    = float(g("stall_backoff_sec", 0.8))
        self.backoff_v      = float(g("stall_backoff_v", 0.25))

        # IMU
        self.use_imu     = bool(g("use_imu", True))
        self.imu_topic   = g("imu_topic", "/camera/imu")
        self.impact_a    = float(g("impact_accel", 4.0))
        self.tilt_stop   = math.radians(float(g("tilt_stop_deg", 18.0)))
        self.imu_latch   = float(g("imu_latch_sec", 2.0))

        # 배터리
        self.use_batt   = bool(g("use_battery", False))
        self.batt_min   = float(g("battery_min", 3300))

        # 모터 변환/상한
        self.d       = float(g("wheel_base", 0.328))
        self.max_v   = float(g("max_v", 0.6))
        self.max_w   = float(g("max_w", 0.8))
        self.rate_hz = float(g("control_rate", 20.0))
        self.cmd_timeout = float(g("cmd_timeout", 0.5))

        # 상태
        self.req_v = 0.0; self.req_w = 0.0
        self.last_cmd_time = rospy.Time(0)
        self.front_dist = float("inf")
        self.last_scan_time = rospy.Time(0)
        self.us_front = float("inf"); self.us_rear = float("inf")
        self.enc_hist = deque()        # (t, enc_avg)
        self.cmd_moving_since = None    # 전진명령 시작시각
        self.backoff_until = rospy.Time(0)
        self.batt = None
        self.last_info_time = rospy.Time(0)
        # IMU
        self.g_ref = None               # 시작시 중력방향(정지 가정)
        self.g_cal = []                 # 캘리브레이션 표본
        self.tilt = 0.0
        self.impact_until = rospy.Time(0)
        self.last_imu_time = rospy.Time(0)

        self.pub = rospy.Publisher(self.cmd_out_topic, Float32MultiArray, queue_size=1)
        rospy.Subscriber(self.cmd_in_topic, Float32MultiArray, self.cmd_cb, queue_size=1)
        rospy.Subscriber(self.scan_topic, LaserScan, self.scan_cb, queue_size=1)
        rospy.Subscriber(self.robot_info_topic, RobotInfo, self.info_cb, queue_size=1)
        if self.use_imu:
            rospy.Subscriber(self.imu_topic, Imu, self.imu_cb, queue_size=10)

        rospy.loginfo("[safety_guard] layers: scan=on us=%s stall=%s imu=%s batt=%s",
                      self.use_us, self.use_stall, self.use_imu, self.use_batt)

    # --- 콜백 ---
    def cmd_cb(self, msg):
        if len(msg.data) >= 2:
            self.req_v = max(-self.max_v, min(self.max_v, msg.data[0]))
            self.req_w = max(-self.max_w, min(self.max_w, msg.data[1]))
            self.last_cmd_time = rospy.Time.now()

    def scan_cb(self, scan):
        best = float("inf"); ang = scan.angle_min
        for r in scan.ranges:
            if -self.front_angle <= ang <= self.front_angle:
                if scan.range_min < r < scan.range_max and not math.isinf(r) and not math.isnan(r):
                    if r < best:
                        best = r
            ang += scan.angle_increment
        self.front_dist = best
        self.last_scan_time = rospy.Time.now()

    def _us_min(self, idxs, sonic):
        vals = []
        for i in idxs:
            if 0 <= i < len(sonic):
                cm = float(sonic[i])
                if 0.0 < cm <= self.us_valid_max:
                    vals.append(cm)
        return min(vals) if vals else float("inf")

    def info_cb(self, info):
        now = rospy.Time.now()
        self.last_info_time = now
        if self.use_us:
            self.us_front = self._us_min(self.us_front_idx, info.sonic)
            self.us_rear  = self._us_min(self.us_rear_idx, info.sonic)
        # 엔코더 히스토리(평균)
        enc_avg = 0.5 * (float(info.left_enc) + float(info.right_enc))
        self.enc_hist.append((now, enc_avg))
        # stall_sec*2 넘는 과거 표본 제거
        while self.enc_hist and (now - self.enc_hist[0][0]).to_sec() > self.stall_sec * 2.0:
            self.enc_hist.popleft()
        self.batt = float(info.batVoltage)

    def imu_cb(self, msg):
        a = msg.linear_acceleration
        ax, ay, az = a.x, a.y, a.z
        mag = math.sqrt(ax * ax + ay * ay + az * az)
        now = rospy.Time.now()
        self.last_imu_time = now
        if mag < 1e-3:
            return
        # 시작 30표본으로 중력방향 캘리브레이션(정지 가정)
        if self.g_ref is None:
            self.g_cal.append((ax / mag, ay / mag, az / mag))
            if len(self.g_cal) >= 30:
                sx = sum(c[0] for c in self.g_cal); sy = sum(c[1] for c in self.g_cal); sz = sum(c[2] for c in self.g_cal)
                n = math.sqrt(sx * sx + sy * sy + sz * sz) or 1.0
                self.g_ref = (sx / n, sy / n, sz / n)
                rospy.loginfo("[safety_guard] IMU 중력방향 캘리브레이션 완료")
            return
        # 충격: 가속도 크기가 g 에서 크게 벗어남
        if abs(mag - 9.81) > self.impact_a:
            self.impact_until = now + rospy.Duration(self.imu_latch)
        # 기울기: 현재 중력방향과 초기방향 사이 각도
        gx, gy, gz = ax / mag, ay / mag, az / mag
        dot = max(-1.0, min(1.0, gx * self.g_ref[0] + gy * self.g_ref[1] + gz * self.g_ref[2]))
        self.tilt = math.acos(dot)

    # --- 안전 판정 ---
    def check_stop(self, v_cmd):
        """정지시켜야 하면 (True, 사유). v_cmd 는 현재 내보내려는 전진성분."""
        now = rospy.Time.now()
        # 카메라 전방
        if self.front_dist <= self.stop_distance:
            return True, "camera %.2fm" % self.front_dist
        # 초음파 (진행방향 기준)
        if self.use_us:
            if v_cmd >= 0 and self.us_front <= self.us_stop_cm:
                return True, "us_front %.0fcm" % self.us_front
            if v_cmd < 0 and self.us_rear <= self.us_stop_cm:
                return True, "us_rear %.0fcm" % self.us_rear
        # IMU 충격 래치 / 기울기
        if self.use_imu:
            if now < self.impact_until:
                return True, "impact"
            if self.tilt >= self.tilt_stop:
                return True, "tilt %.0fdeg" % math.degrees(self.tilt)
        # 배터리
        if self.use_batt and self.batt is not None and self.batt <= self.batt_min:
            return True, "battery %.0f" % self.batt
        # 신호 끊김
        if (now - self.last_scan_time).to_sec() > 1.0:
            return True, "no scan"
        return False, ""

    def is_stalled(self, v_out):
        """전진 명령 중인데 엔코더가 안 변하면 접촉/끼임."""
        if not self.use_stall or abs(v_out) < self.stall_min_cmd:
            self.cmd_moving_since = None
            return False
        now = rospy.Time.now()
        if self.cmd_moving_since is None:
            self.cmd_moving_since = now
            return False
        if (now - self.cmd_moving_since).to_sec() < self.stall_sec:
            return False
        # stall_sec 창의 엔코더 변화량
        cutoff = now - rospy.Duration(self.stall_sec)
        window = [e for (t, e) in self.enc_hist if t >= cutoff]
        if len(window) < 2:
            return False
        if (max(window) - min(window)) < self.stall_enc_min:
            return True
        return False

    def scale_factor(self):
        if self.front_dist >= self.slow_distance:
            return 1.0
        if self.front_dist <= self.stop_distance:
            return 0.0
        span = self.slow_distance - self.stop_distance
        return max(0.0, (self.front_dist - self.stop_distance) / span) if span > 0 else 0.0

    def publish(self, v, w):
        left  = -(v - self.d * w)
        right = -(v + self.d * w)
        m = Float32MultiArray(); m.data = [float(left), float(right)]
        self.pub.publish(m)

    def spin(self):
        rate = rospy.Rate(self.rate_hz)
        while not rospy.is_shutdown():
            now = rospy.Time.now()
            v, w = self.req_v, self.req_w

            # explorer 명령 타임아웃 -> 정지
            if (now - self.last_cmd_time).to_sec() > self.cmd_timeout:
                v, w = 0.0, 0.0

            # 진행 중 스톨(접촉) -> 잠깐 후진 회피 래치
            if now < self.backoff_until:
                self.publish(-self.backoff_v, 0.0)
                rate.sleep(); continue
            if self.is_stalled(v):
                rospy.logwarn("[safety_guard] STALL 감지 -> 후진 회피")
                self.backoff_until = now + rospy.Duration(self.backoff_sec)
                self.cmd_moving_since = None
                self.publish(-self.backoff_v, 0.0)
                rate.sleep(); continue

            # 위험 판정 -> 강제 정지
            stop, why = self.check_stop(v)
            if stop:
                v, w = 0.0, 0.0
                rospy.logwarn_throttle(1.0, "[safety_guard] STOP (%s)", why)
            else:
                v = v * self.scale_factor()

            self.publish(v, w)
            rate.sleep()


if __name__ == "__main__":
    rospy.init_node("safety_guard")
    try:
        SafetyGuard().spin()
    except rospy.ROSInterruptException:
        pass
