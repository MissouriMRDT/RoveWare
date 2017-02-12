from rovecomm import RoveComm
from random import random

rovecomm_node = RoveComm()

# ---------------------------------
# Send some really simple telemetry
# ---------------------------------

data_id = 155
telemetry = random()

# This sends the contents of telemetry out
# to all subscribers
rovecomm_node.send(data_id, telemetry)


# -------------------------------
# Send more complicated telemetry
# -------------------------------

import struct

data_id = 160
complex_telemetry = (random(), int(50*random()), "Hi")

# Pack telemetry into a float, unsigned byte, and c-string
packed_telemetry = struct.pack(">fBs")
rovecomm_node.send(data_id, packed_telemetry)