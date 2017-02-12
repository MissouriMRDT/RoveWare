from rovecomm import RoveComm
import struct

rovecomm_node = RoveComm()

def set_speed_handler(contents):
    # Contents are typically C style structs
    # ">HH" is a format code for two uint16_t
    # See python's documentation for struct.unpack
    # for more info about format codes
    # And working with C style structs from python
    speed_left, speed_right = struct.unpack(">HH", contents)
    print "Speed set to %d, %d" % (speed_left, speed_right)

def add_waypoint_handler(contents):
    latitude, longitude = struct.unpack(">dd")
    print "Added waypoint (%f, %f) " % (latitude, longitude)

# use RoveComm.callbacks to define what code should
# run when a message with a certain data ID is received.
# Here we assign data ID 138 to set_speed_handler
# and data ID 267 to add_waypoint_handler
#
# This demo just passively waits for the device to send us something
# If you want to actively request data, see
# subscribe.py

rovecomm_node.callbacks[138] = set_speed_handler
rovecomm_node.callbacks[267] = add_waypoint_handler

# Rovecomm works in the background
# To keep the program running so we can receive stuff, we'll just wait
# for the user to press enter.
#
# If any messages come in while we're waiting, they'll be printed to
# the screen.

raw_input("Press enter to stop the program")