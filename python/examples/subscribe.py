from rovecomm import RoveComm
import struct
import time

# The Scenario: Two sensors
thermometer_ip_address = '192.168.1.50'
thermometer_data_id    = 1234

gps_ip_address         = '192.168.1.51'
gps_data_id            = 5678

# A program that reads from these
# sensors over RoveComm and prints out
# statistics about them every five seconds

temperatures = []
location_history = []


def process_thermometer_data(raw_data):
    # The thermometer sends data as a float
    temperature = struct.unpack("f", raw_data)
    temperatures.append(temperature)


def process_gps_data(raw_data):
    # The GPS sends data as two doubles
    lat, lon = struct.unpack("dd", raw_data)
    location_history.append((lat, lon))

rovecomm_node = RoveComm()

# Ask the devices to automatically report telemetry to me
# It's up to the device to choose when to send data
rovecomm_node.subscribe(thermometer_ip_address)
rovecomm_node.subscribe(gps_ip_address)

# Tell rovecomm where to put the data when it comes in
rovecomm_node.callbacks[thermometer_data_id] = process_thermometer_data
rovecomm_node.callbacks[gps_data_id] = process_gps_data

while True:
    # Report statistics on what's come in
    print "Min and max temperature seen: (%d, %d)" % (min(temperatures), max(temperatures))
    print "Most recent location: %s " % location_history[-1]

    # Give up if it's too hot
    if max(temperatures) > 95.0:
        rovecomm_node.unsubscribe(thermometer_ip_address) # I don't want to hear you anymore
        temperatures = [65, 70, 67]                       # Everything is sunny and breezy in my world

    time.sleep(5)