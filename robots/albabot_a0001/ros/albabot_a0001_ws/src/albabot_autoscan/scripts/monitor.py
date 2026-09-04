#!/usr/bin/env python
# -*- coding: utf-8 -*-
"""
monitor.py  -  autoscan 동작 관찰용 로거

주요 신호를 일정 주기로 CSV 한 줄씩 저장(사람이 읽고 임계값 보정하기 좋게).
rosbag(원본) 과 별개로, 요약 지표만 뽑는다.

컬럼: t, front_m, us_front_cm, us_rear_cm, encL, encR, dEncL, dEncR,
      batt, imu_ax, imu_ay, imu_az, imu_mag, cmdL, cmdR, req_v, req_w
저장 위치: <log_dir>/monitor_<시각>.csv  (기본 ~/autoscan_logs)
"""
import os
import time
import math
import rospy
from std_msgs.msg import Float32MultiArray
from sensor_msgs.msg import LaserScan, Imu
from albabot_msgs.msg import RobotInfo


class Monitor(object):
    def __init__(self):
        self.log_dir = os.path.expanduser(rospy.get_param("~log_dir", "~/autoscan_logs"))
        self.rate_hz = float(rospy.get_param("~rate", 5.0))
        self.front_angle = math.radians(float(rospy.get_param("~front_angle_deg", 40.0)))
        if not os.path.isdir(self.log_dir):
            os.makedirs(self.log_dir)
        stamp = time.strftime("%Y%m%d_%H%M%S")
        self.path = os.path.join(self.log_dir, "monitor_%s.csv" % stamp)
        self.f = open(self.path, "w")
        self.f.write("t,front_m,us_front_cm,us_rear_cm,encL,encR,dEncL,dEncR,"
                     "batt,imu_ax,imu_ay,imu_az,imu_mag,cmdL,cmdR,req_v,req_w\n")
        self.f.flush()

        # 상태
        self.front = float("inf")
        self.sonic = []
        self.encL = 0; self.encR = 0
        self.prev_encL = None; self.prev_encR = None
        self.batt = 0
        self.ax = self.ay = self.az = 0.0
        self.cmdL = 0.0; self.cmdR = 0.0
        self.req_v = 0.0; self.req_w = 0.0

        rospy.Subscriber("/camera/scan", LaserScan, self.scan_cb, queue_size=1)
        rospy.Subscriber("/robot_info", RobotInfo, self.info_cb, queue_size=1)
        rospy.Subscriber("/camera/imu", Imu, self.imu_cb, queue_size=1)
        rospy.Subscriber("/cmd_vel", Float32MultiArray, self.cmd_cb, queue_size=1)
        rospy.Subscriber("/scan_cmd", Float32MultiArray, self.req_cb, queue_size=1)
        rospy.loginfo("[monitor] 로그 저장: %s", self.path)

    def scan_cb(self, s):
        best = float("inf"); ang = s.angle_min
        for r in s.ranges:
            if -self.front_angle <= ang <= self.front_angle:
                if s.range_min < r < s.range_max and not math.isinf(r) and not math.isnan(r):
                    best = min(best, r)
            ang += s.angle_increment
        self.front = best

    def info_cb(self, m):
        self.encL = int(m.left_enc); self.encR = int(m.right_enc)
        self.sonic = list(m.sonic); self.batt = int(m.batVoltage)

    def imu_cb(self, m):
        self.ax = m.linear_acceleration.x
        self.ay = m.linear_acceleration.y
        self.az = m.linear_acceleration.z

    def cmd_cb(self, m):
        if len(m.data) >= 2:
            self.cmdL, self.cmdR = m.data[0], m.data[1]

    def req_cb(self, m):
        if len(m.data) >= 2:
            self.req_v, self.req_w = m.data[0], m.data[1]

    def _us(self, idxs):
        vals = [float(self.sonic[i]) for i in idxs if i < len(self.sonic) and self.sonic[i] > 0]
        return min(vals) if vals else -1

    def spin(self):
        rate = rospy.Rate(self.rate_hz)
        t0 = time.time()
        while not rospy.is_shutdown():
            dL = 0 if self.prev_encL is None else self.encL - self.prev_encL
            dR = 0 if self.prev_encR is None else self.encR - self.prev_encR
            self.prev_encL = self.encL; self.prev_encR = self.encR
            mag = math.sqrt(self.ax**2 + self.ay**2 + self.az**2)
            front = -1 if math.isinf(self.front) else round(self.front, 3)
            row = [round(time.time() - t0, 2), front,
                   round(self._us([0, 1, 2, 3]), 1), round(self._us([4, 5, 6, 7]), 1),
                   self.encL, self.encR, dL, dR, self.batt,
                   round(self.ax, 3), round(self.ay, 3), round(self.az, 3), round(mag, 3),
                   round(self.cmdL, 3), round(self.cmdR, 3),
                   round(self.req_v, 3), round(self.req_w, 3)]
            self.f.write(",".join(str(x) for x in row) + "\n")
            self.f.flush()
            rate.sleep()
        self.f.close()


if __name__ == "__main__":
    rospy.init_node("monitor")
    try:
        Monitor().spin()
    except rospy.ROSInterruptException:
        pass
