#!/usr/bin/env python
# -*- coding: utf-8 -*-
"""
explorer.py  -  albabot_autoscan 저속 반응형 탐색 + 주기적 회전스캔

동작(상태기계):
  CRUISE   : 저속 전진. 전방이 front_clear_distance 이내로 막히면 TURN 으로.
  TURN     : 더 열린 쪽(좌/우 최소거리 비교)으로 제자리 회전, 전방이 뚫리면 CRUISE.
  SPIN_SCAN: spin_scan_period 마다 진입. 제자리 저속 회전으로 주변을 스윕
             (RTAB-Map 이 사방을 담도록). spin_scan_duration 후 CRUISE.

출력: /scan_cmd = Float32MultiArray[v, w]  (safety_guard 가 받아 최종 판단/정지)
      * explorer 는 절대 직접 /cmd_vel 을 쓰지 않는다. 안전은 safety_guard 전담.
"""
import math
import rospy
from std_msgs.msg import Float32MultiArray
from sensor_msgs.msg import LaserScan


class Explorer(object):
    def __init__(self):
        g = lambda k, d: rospy.get_param("~" + k, d)
        self.scan_topic = g("scan_topic", "/camera/scan")
        self.cmd_topic  = g("cmd_out_topic", "/scan_cmd")
        self.rate_hz    = float(g("rate", 10.0))

        self.forward_v  = float(g("forward_v", 0.35))
        self.turn_w     = float(g("turn_w", 0.6))
        self.front_clear = float(g("front_clear_distance", 1.2))
        self.front_angle = math.radians(float(g("front_angle_deg", 30.0)))

        self.spin_period   = float(g("spin_scan_period", 25.0))
        self.spin_duration = float(g("spin_scan_duration", 12.0))
        self.spin_w        = float(g("spin_scan_w", 0.5))

        self.front = float("inf")
        self.left = float("inf")
        self.right = float("inf")
        self.have_scan = False

        self.state = "CRUISE"
        self.turn_dir = 1.0            # +좌 / -우
        self.state_since = rospy.Time.now()
        self.last_spin = rospy.Time.now()

        self.pub = rospy.Publisher(self.cmd_topic, Float32MultiArray, queue_size=1)
        rospy.Subscriber(self.scan_topic, LaserScan, self.scan_cb, queue_size=1)
        rospy.loginfo("[explorer] forward_v=%.2f turn_w=%.2f front_clear=%.2fm",
                      self.forward_v, self.turn_w, self.front_clear)

    def scan_cb(self, scan):
        front = float("inf"); left = float("inf"); right = float("inf")
        ang = scan.angle_min
        for r in scan.ranges:
            if scan.range_min < r < scan.range_max and not math.isinf(r) and not math.isnan(r):
                if -self.front_angle <= ang <= self.front_angle:
                    front = min(front, r)
                if ang > 0:
                    left = min(left, r)
                else:
                    right = min(right, r)
            ang += scan.angle_increment
        self.front, self.left, self.right = front, left, right
        self.have_scan = True

    def set_state(self, s):
        if s != self.state:
            rospy.loginfo("[explorer] %s -> %s (front=%.2f L=%.2f R=%.2f)",
                          self.state, s, self.front, self.left, self.right)
            self.state = s
            self.state_since = rospy.Time.now()

    def publish(self, v, w):
        m = Float32MultiArray(); m.data = [float(v), float(w)]
        self.pub.publish(m)

    def spin(self):
        rate = rospy.Rate(self.rate_hz)
        while not rospy.is_shutdown():
            now = rospy.Time.now()
            v, w = 0.0, 0.0

            if not self.have_scan:
                self.publish(0.0, 0.0)      # 스캔 없으면 대기(안전)
                rate.sleep(); continue

            # 주기적 회전스캔 진입
            if self.state != "SPIN_SCAN" and (now - self.last_spin).to_sec() >= self.spin_period:
                self.set_state("SPIN_SCAN")

            if self.state == "SPIN_SCAN":
                v, w = 0.0, self.spin_w      # 제자리 저속 회전
                if (now - self.state_since).to_sec() >= self.spin_duration:
                    self.last_spin = now
                    self.set_state("CRUISE")

            elif self.state == "CRUISE":
                if self.front < self.front_clear:
                    # 더 열린 쪽으로 회전 방향 결정
                    self.turn_dir = 1.0 if self.left >= self.right else -1.0
                    self.set_state("TURN")
                else:
                    v, w = self.forward_v, 0.0

            elif self.state == "TURN":
                w = self.turn_w * self.turn_dir
                # 전방이 충분히 뚫리면 순항 복귀
                if self.front >= self.front_clear * 1.15:
                    self.set_state("CRUISE")

            self.publish(v, w)
            rate.sleep()


if __name__ == "__main__":
    rospy.init_node("explorer")
    try:
        Explorer().spin()
    except rospy.ROSInterruptException:
        pass
