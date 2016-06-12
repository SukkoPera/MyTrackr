#!/usr/bin/env python

import datetime

import serial

from sim import GpsSimulator

#~ LOGFILE = "/tmp/logbat2.txt"
LOGFILE = "loggps.nmea"
#~ LOGFILE = None
SERIAL = "/dev/ttyACM0"
SPEED = 9600
TIMEOUT = 1000	# ms

with serial.Serial (SERIAL, SPEED, timeout = TIMEOUT / 1000.0) as ser:
	gps = GpsSimulator ()

	while True:
		for sentence in gps.getSentences ():
			ser.write (sentence + "\r\n")
			now = datetime.datetime.now ()
			print "[%s] <-- %s" % (now, sentence)
		gps.update ()

		line = ser.readline ()
		if len (line) > 0:
			line = line.strip ()
			now = datetime.datetime.now ()
			outline = "[%s] --> %s" % (now, line)
			print outline
			if LOGFILE is not None:
				with open (LOGFILE, "a") as fp:
					fp.write (outline + "\n")

