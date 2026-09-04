#!/usr/bin/env python
# -*- coding: utf-8 -*-
"""
drive_test.py  -  바운드된 전진 넛지 테스트 (safety_guard 경유)

/scan_cmd 로 낮은 전진 v 를 duration 초 동안만 요청한 뒤 0 을 보내고 종료.
안전은 safety_guard 가 전담(장애물/스캔끊김/스톨 시 정지). 이 노드는 절대
/cmd_vel 을 직접 쓰지 않는다. 자기 종료 + 하트비트로 이중 안전.

파라미터:
  ~v         전진 속도(joy 스케일, 기본 0.2, 낮게)
  ~duration  주행 시간 초(기본 1.5)
  ~w         회전(기본 0.0, 순수 전진)
"""
import rospy
from std_msgs.msg import Float32MultiArray


if __name__ == "__main__":
    rospy.init_node("drive_test")
    v = float(rospy.get_param("~v", 0.2))
    w = float(rospy.get_param("~w", 0.0))
    dur = float(rospy.get_param("~duration", 1.5))
    pub = rospy.Publisher("/scan_cmd", Float32MultiArray, queue_size=1)

    # safety_guard 구독 대기
    t0 = rospy.Time.now()
    while pub.get_num_connections() < 1 and (rospy.Time.now() - t0).to_sec() < 5.0:
        rospy.sleep(0.1)

    rospy.loginfo("[drive_test] 전진 시작 v=%.2f w=%.2f dur=%.1fs", v, w, dur)
    rate = rospy.Rate(20)
    start = rospy.Time.now()
    while not rospy.is_shutdown() and (rospy.Time.now() - start).to_sec() < dur:
        m = Float32MultiArray(); m.data = [v, w]
        pub.publish(m)
        rate.sleep()

    # 정지 명령 반복 발행 후 종료
    for _ in range(20):
        m = Float32MultiArray(); m.data = [0.0, 0.0]
        pub.publish(m)
        rate.sleep()
    rospy.loginfo("[drive_test] 종료(정지)")
