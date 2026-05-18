# vcs_msgs/msg/DriveCommand.msg
# Sent by the Jetson → VCS at any rate; replaces raw PKT_TYPE_RX frames.
#
# Field ranges are identical to the original UART packet:
#   mode   : 0=manual 1=auto (was rx_buf[4])
#   steer  : 0-10000  percentage (was rx_buf[5:6])
#   speed  : 0-10000  percentage (was rx_buf[7:8])
#   brake  : 0-100              (was rx_buf[9])
#   dir    : 0=fwd 1=rev        (was rx_buf[10])

uint8  mode
uint16 steer
uint16 speed
uint8  brake
uint8  dir
