#!/usr/bin/python
import time, socket, sys, numpy as np, socket

# ImportError: No module named numpy
# 	http://stackoverflow.com/questions/7818811/import-error-no-module-named-numpy

def print_help_and_exit():
	print('Usage: replay_cap.py <xyz.pcap> [--start=<N>|--end=<N>]')
	sys.exit()


if len(sys.argv) < 2:
	print_help_and_exit()


file_name = sys.argv[1]
try:
	pcap_file = open(file_name, 'rb')
except:
	print_help_and_exit()

# get pcap info
packet_counter = 0

global_header = pcap_file.read(24)
while 1:
	try:
		packet_header = np.fromstring(pcap_file.read(16), 'uint32')
		if len(packet_header) < 1:
			if packet_counter == 0:
				print('[Error] packet format error: bad file or use 64-bit replayer')
				pcap_file.close()
				sys.exit()
			else:
				break

		timestamp = packet_header[0] + packet_header[1]/1e6
		
		if packet_counter == 0:
			start_time = timestamp

		duration = timestamp - start_time

		size = packet_header[2]
		if size != 1248:
			pcap_file.read(size)
			continue
		pcap_file.read(1248)

		packet_counter += 1
	except KeyboardInterrupt:
		break

pcap_file.close()
if duration < 0.:
	print('[Error] packet format error: bad file or use 64-bit replayer')
	sys.exit()
print('[Info] packet\'s #: ' + str(packet_counter))
print('[Info] duration: ' + str(int(duration/60)) + ':' + "{:.2f}".format(duration%60))

# replay pcap file
START = 1
END  = 1000000000
try:
	for arg in sys.argv[2:]:
		if arg.startswith('--start='):
			START = int(arg[8:])
		elif arg.startswith('--end='):
			END = int(arg[6:])
except:
	print_help_and_exit()

replay_socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
ADDR = ('127.0.0.1', 2368)

try:
	pcap_file = open(file_name, 'rb')
except:
	print_help_and_exit()

packet_index = 0
last_duration = 0.
global_header = pcap_file.read(24)
while 1:
	try:
		packet_header = np.fromstring(pcap_file.read(16), 'uint32')
		if len(packet_header) < 1:
			break

		timestamp = packet_header[0] + packet_header[1]/1e6
		if packet_index == 0:
			start_timestamp = timestamp

		duration = timestamp - start_timestamp

		size = packet_header[2]
		if size != 1248:
			pcap_file.read(size)
			continue

		packet = pcap_file.read(1248)
		data = packet[42:]

		packet_index += 1
		if packet_index < START:
			continue		
		if packet_index > END:
			break

		cur_time = time.time()
		if packet_index > 1:
			sleep_time = (timestamp-last_timestamp) - (cur_time-last_time)
			# sleep_time<0 ==> replay delayed ==> replay immediately
			time.sleep(max(0, sleep_time))
		else:
			print('[Replaying] started...')

		replay_socket.sendto(data, ADDR)

		last_timestamp = timestamp
		last_time = cur_time

		if (duration-last_duration) > 1.33:
			print('[Replaying] time replayed: ' + str(int(duration/60)) + ':' + "{:.2f}".format(duration%60))
			last_duration = duration

	except KeyboardInterrupt:
		break

pcap_file.close()
print('')
if packet_index < packet_counter:
	print('[Stopped] # of packet replayed: ' + str(packet_index))
	print('[Stopped] time replayed: ' + str(int(duration/60)) + ':' + "{:.2f}".format(duration%60))
else:
	print('[Finished] # of packet replayed: ' + str(packet_index))
	print('[Finished] time replayed: ' + str(int(duration/60)) + ':' + "{:.2f}".format(duration%60))