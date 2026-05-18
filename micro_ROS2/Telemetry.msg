# vcs_msgs/msg/Telemetry.msg
# Sent by VCS → Jetson at 10 Hz; replaces raw PKT_TYPE_TX frames.
#
# Field mapping to original tx_buf:
#   rpm          : mechanical_rpm          (was tx_buf[4:5], int16)
#   steer_pct    : pot_filtered_mv scaled  (was tx_buf[6:7], uint16 0-10000)
#   brake_pct    : brake_at_limit*100      (was tx_buf[8],   uint8)
#   aux          : reserved                (was tx_buf[9],   always 0)
#   reverse      : reverse_switch*100      (was tx_buf[10],  uint8)

int16  rpm
uint16 steer_pct
uint8  brake_pct
uint8  aux
uint8  reverse
