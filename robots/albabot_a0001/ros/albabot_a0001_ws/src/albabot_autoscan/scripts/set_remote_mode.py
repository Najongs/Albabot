#!/usr/bin/env python
# -*- coding: utf-8 -*-
"""
set_remote_mode.py  -  펌웨어를 Remote 모드로 전환 + 잔여 미션 취소 (일회성)

Remote 모드에서만 /cmd_vel(g_pMotorOutput) 이 모터에 반영된다.
CAN 규약(id 0x100):
  [0x00, 0x31, 0x00]  모드 전환 -> 0=Remote
  [0x00, 0xff]        현재 명령/미션 취소
serial_node 가 /AlbabotMessage 를 펌웨어로 브리지하므로, 연결 후 잠깐 대기 후 발행.
"""
import rospy
from albabot_msgs.msg import CanMsg


def send(pub, data):
    m = CanMsg()
    m.id = 0x100
    m.dlc = len(data)
    m.data = data
    pub.publish(m)


if __name__ == "__main__":
    rospy.init_node("set_remote_mode")
    pub = rospy.Publisher("/AlbabotMessage", CanMsg, queue_size=1)

    # 구독자(serial_node)가 붙을 때까지 대기
    t0 = rospy.Time.now()
    while pub.get_num_connections() < 1 and (rospy.Time.now() - t0).to_sec() < 10.0:
        rospy.sleep(0.2)
    rospy.sleep(1.0)

    send(pub, [0x00, 0x31, 0x00])   # Remote 모드
    rospy.sleep(0.5)
    send(pub, [0x00, 0xff])         # 잔여 미션 취소
    rospy.sleep(0.5)
    rospy.loginfo("[set_remote_mode] Remote 모드 설정 + 미션 취소 전송 완료")
