#!/usr/bin/env python
# -*- coding: utf-8 -*-
"""
motor_char.py  -  모터 특성 파악 (데드밴드/방향/엔코더) - 카메라 없이

/cmd_vel 로 전진의도([-mag,-mag])를 낮은 값부터 계단식으로 올리며,
엔코더가 움직이기 시작하는 값(데드밴드)과 좌/우 엔코더 반응·방향을 찾는다.
움직임 감지 즉시 정지. 하드 타임아웃/heartbeat 로 이중 안전.

★ 이 테스트는 카메라 장애물 안전을 거치지 않는다(현재 RealSense 불안정).
  반드시 전방이 비어 있는 상태에서만 실행.

파라미터:
  ~start_mag(0.3) ~max_mag(1.5) ~step(0.15) ~step_sec(0.5)
  ~move_enc(50)  움직임 판정 엔코더 변화 임계
  ~timeout(7.0)
"""
import rospy
from std_msgs.msg import Float32MultiArray
from albabot_msgs.msg import RobotInfo


class MotorChar(object):
    def __init__(self):
        self.encL = None; self.encR = None
        self.startL = None; self.startR = None
        rospy.Subscriber("/robot_info", RobotInfo, self.info_cb, queue_size=1)
        self.pub = rospy.Publisher("/cmd_vel", Float32MultiArray, queue_size=1)

    def info_cb(self, m):
        self.encL = int(m.left_enc); self.encR = int(m.right_enc)
        if self.startL is None:
            self.startL = self.encL; self.startR = self.encR

    def cmd(self, val):
        m = Float32MultiArray(); m.data = [val, val]  # 좌우 동일 = 순수 전/후진 의도
        self.pub.publish(m)

    def run(self):
        start_mag = float(rospy.get_param("~start_mag", 0.3))
        max_mag   = float(rospy.get_param("~max_mag", 1.5))
        step      = float(rospy.get_param("~step", 0.15))
        step_sec  = float(rospy.get_param("~step_sec", 0.5))
        move_enc  = int(rospy.get_param("~move_enc", 50))
        timeout   = float(rospy.get_param("~timeout", 7.0))

        # 엔코더 초기값 확보 대기
        t0 = rospy.Time.now()
        while self.startL is None and (rospy.Time.now() - t0).to_sec() < 3.0:
            rospy.sleep(0.1)
        if self.startL is None:
            rospy.logerr("[motor_char] robot_info 수신 실패 - 중단"); return

        rospy.loginfo("[motor_char] 시작 encL=%d encR=%d", self.startL, self.startR)
        rate = rospy.Rate(20)
        mag = start_mag
        t_start = rospy.Time.now()
        moved = False
        while not rospy.is_shutdown() and mag <= max_mag + 1e-6:
            if (rospy.Time.now() - t_start).to_sec() > timeout:
                rospy.logwarn("[motor_char] 타임아웃"); break
            t_step = rospy.Time.now()
            while not rospy.is_shutdown() and (rospy.Time.now() - t_step).to_sec() < step_sec:
                self.cmd(-mag)   # 전진의도(부호 -). 실제 방향은 엔코더/육안으로 확인
                dL = abs(self.encL - self.startL); dR = abs(self.encR - self.startR)
                if dL > move_enc or dR > move_enc:
                    rospy.logwarn("[motor_char] 움직임! mag=%.2f dEncL=%d dEncR=%d -> 정지",
                                  mag, self.encL - self.startL, self.encR - self.startR)
                    moved = True
                    break
                rate.sleep()
            if moved:
                break
            rospy.loginfo("[motor_char] mag=%.2f 무반응 (encL=%d encR=%d)", mag, self.encL, self.encR)
            mag += step

        # 정지
        for _ in range(30):
            self.cmd(0.0); rate.sleep()
        if not moved:
            rospy.logwarn("[motor_char] max_mag 까지 무반응 - 데드밴드/모터/모드 점검 필요")
        rospy.loginfo("[motor_char] 종료 encL=%d encR=%d (dL=%d dR=%d)",
                      self.encL, self.encR, self.encL - self.startL, self.encR - self.startR)


if __name__ == "__main__":
    rospy.init_node("motor_char")
    try:
        MotorChar().run()
    except rospy.ROSInterruptException:
        pass
