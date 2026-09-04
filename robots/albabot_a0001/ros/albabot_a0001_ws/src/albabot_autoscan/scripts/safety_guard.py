#!/usr/bin/env python
# -*- coding: utf-8 -*-
"""
safety_guard.py  -  albabot_autoscan 안전 계층 (/cmd_vel 유일 발행자)

역할:
  - explorer 가 요청한 [v, w] (/scan_cmd) 를 받되,
  - /camera/scan(depth->laserscan) 과 /robot_info(초음파) 로 전방 장애물을 판단해
    * 위험거리 이내      -> 강제 정지 (요청 무시)
    * 감속거리 이내      -> 속도 선형 감속
    * explorer 명령 끊김 -> 정지
  - 항상 control_rate 로 /cmd_vel 을 발행해 펌웨어 하트비트 워치독을 유지.

펌웨어 규약: /cmd_vel = Float32MultiArray[left_motor, right_motor], Remote 모드에서 적용.
좌우 변환은 driving_m1 GetVelocity 와 동일 부호:
    left  = -(v - d*w)
    right = -(v + d*w)
"""
import math
import rospy
from std_msgs.msg import Float32MultiArray
from sensor_msgs.msg import LaserScan
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

        self.use_us        = bool(g("use_ultrasonic", True))
        self.us_stop_cm    = float(g("ultrasonic_stop_cm", 30.0))
        self.us_valid_max  = float(g("ultrasonic_valid_max_cm", 250.0))

        self.d       = float(g("wheel_base", 0.328))
        self.max_v   = float(g("max_v", 0.6))
        self.max_w   = float(g("max_w", 0.8))
        self.rate_hz = float(g("control_rate", 20.0))
        self.cmd_timeout = float(g("cmd_timeout", 0.5))

        self.req_v = 0.0
        self.req_w = 0.0
        self.last_cmd_time = rospy.Time(0)
        self.front_dist = float("inf")   # 카메라 전방 최소거리
        self.us_min_cm  = float("inf")   # 초음파 최소거리(cm)
        self.last_scan_time = rospy.Time(0)

        self.pub = rospy.Publisher(self.cmd_out_topic, Float32MultiArray, queue_size=1)
        rospy.Subscriber(self.cmd_in_topic, Float32MultiArray, self.cmd_cb, queue_size=1)
        rospy.Subscriber(self.scan_topic, LaserScan, self.scan_cb, queue_size=1)
        if self.use_us:
            rospy.Subscriber(self.robot_info_topic, RobotInfo, self.info_cb, queue_size=1)

        rospy.loginfo("[safety_guard] stop=%.2fm slow=%.2fm front=%.0fdeg us=%s",
                      self.stop_distance, self.slow_distance,
                      math.degrees(self.front_angle), self.use_us)

    # --- 콜백 ---
    def cmd_cb(self, msg):
        if len(msg.data) >= 2:
            self.req_v = max(-self.max_v, min(self.max_v, msg.data[0]))
            self.req_w = max(-self.max_w, min(self.max_w, msg.data[1]))
            self.last_cmd_time = rospy.Time.now()

    def scan_cb(self, scan):
        # 전방 ±front_angle 창의 최소 유효거리
        best = float("inf")
        ang = scan.angle_min
        for r in scan.ranges:
            if -self.front_angle <= ang <= self.front_angle:
                if scan.range_min < r < scan.range_max and not math.isinf(r) and not math.isnan(r):
                    if r < best:
                        best = r
            ang += scan.angle_increment
        self.front_dist = best
        self.last_scan_time = rospy.Time.now()

    def info_cb(self, info):
        vals = []
        for s in info.sonic[:4]:            # 전방 4개만 사용
            cm = float(s)
            if 0.0 < cm <= self.us_valid_max:
                vals.append(cm)
        self.us_min_cm = min(vals) if vals else float("inf")

    # --- 안전 판정 ---
    def danger(self):
        if self.front_dist <= self.stop_distance:
            return True, "camera %.2fm" % self.front_dist
        if self.use_us and self.us_min_cm <= self.us_stop_cm:
            return True, "ultrasonic %.0fcm" % self.us_min_cm
        return False, ""

    def scale_factor(self):
        # slow_distance~stop_distance 사이 선형 감속
        if self.front_dist >= self.slow_distance:
            return 1.0
        if self.front_dist <= self.stop_distance:
            return 0.0
        span = self.slow_distance - self.stop_distance
        return max(0.0, (self.front_dist - self.stop_distance) / span) if span > 0 else 0.0

    def publish(self, v, w):
        left  = -(v - self.d * w)
        right = -(v + self.d * w)
        m = Float32MultiArray()
        m.data = [float(left), float(right)]
        self.pub.publish(m)

    def spin(self):
        rate = rospy.Rate(self.rate_hz)
        while not rospy.is_shutdown():
            now = rospy.Time.now()
            v, w = self.req_v, self.req_w

            # 1) explorer 명령 타임아웃 -> 정지
            if (now - self.last_cmd_time).to_sec() > self.cmd_timeout:
                v, w = 0.0, 0.0

            # 2) 스캔 신호가 오래 끊기면(카메라 문제) 보수적으로 정지
            if (now - self.last_scan_time).to_sec() > 1.0:
                v, w = 0.0, 0.0

            # 3) 장애물 위험 -> 강제 정지 (요청 무시)
            danger, why = self.danger()
            if danger:
                v, w = 0.0, 0.0
                rospy.logwarn_throttle(1.0, "[safety_guard] STOP (%s)", why)
            else:
                # 4) 감속 구간 -> 전진성분만 감속(회전은 유지해 회피 가능)
                k = self.scale_factor()
                v = v * k

            self.publish(v, w)
            rate.sleep()


if __name__ == "__main__":
    rospy.init_node("safety_guard")
    try:
        SafetyGuard().spin()
    except rospy.ROSInterruptException:
        pass
