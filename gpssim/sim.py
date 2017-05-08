#!/usr/bin/env python

import datetime
import time
import random
import sys

from geopy.point import Point
from geopy.units import arcminutes
from geopy.distance import Distance, vincenty

class GpsSimulator (object):
	# Initial position and movement parameters
	START = (45, 7)
	INIT_SPD = 3		# m/s (3 ~= 10 km/h)
	INIT_TRK = 135
	DELTA_SPD = 2
	DELTA_TRK = 5

	# These are currentlt held constant
	INIT_HDOP = 0.5
	INIT_NSAT = 7

	# Interval between updates
	#~ SIM_STEP = 300

	def __init__ (self):
		random.seed ()

		self.pos = Point (self.START)
		self.spd = self.INIT_SPD
		self.trk = self.INIT_TRK
		self.hdop = self.INIT_HDOP
		self.nsat = self.INIT_NSAT	# 0-12

		self._vinz = vincenty ()

		# We use UTC throughout, as that's what NMEA wants
		self.starttime = datetime.datetime.utcnow ()
		self.now = self.starttime
		self.step = 0

	@staticmethod
	def _splitSentence (sentence):
		"""Extracts a sentence and its checksum and returns them"""
		if len (sentence) > 4 and sentence[0] == '$' and sentence[-3] == '*':
			line = sentence[1:-4]
			#~ checksum = int (sentence [-2:], 16)
			checksum = sentence [-2:]
			ret = line, checksum
		else:
			ret = None, None
		return ret

	@staticmethod
	def _calc_checksum (line):
		"""Calculates checksum of a sentence. $...*?? will be stripped off, if present"""
		if line[0] == '$' and line[-3] == '*':
			sentence = GpsSimulator._splitSentence (line)[0]
		else:
			sentence = line

		if sentence is not None:
			sum = 0
			for c in line[1:]:
				if c == '*':
					break
				else:
					sum ^= ord (c)
		else:
			print "Invalid line"
			sum = None

		return sum

	@staticmethod
	def _toDegMinDec (deg):
		pass

	@staticmethod
	def _msToKnots (ms):
		return ms * 1.94384449

	def getSentences (self):
		sentences = []
		#~ print pos.latitude, pos.longitude, spd, trk

		lat = (int (abs (self.pos.latitude)) * 100 + arcminutes (degrees = abs (self.pos.latitude) - int (abs (self.pos.latitude))), "N" if self.pos[0] >= 0 else "S")
		lon = (int (abs (self.pos.longitude)) * 100 + arcminutes (degrees = abs (self.pos.longitude) - int (abs (self.pos.longitude))), "E" if self.pos[1] >= 0 else "W")
		alt = 42

		# http://www.gpsinformation.org/dale/nmea.htm
		# Message Structure:
		# $GPGGA,hhmmss.ss,Latitude,N,Longitude,E,FS,NoSV,HDOP,msl,m,Altref,m,DiffAge,DiffStation*cs<CR><LF>
		# Example:
		# $GPGGA,092725.00,4717.11399,N,00833.91590,E,1,8,1.01,499.6,M,48.0,M,,0*5B
		sentence = "$GPGGA,%s,%011.6f,%c,%012.6f,%c,1,%d,%.2f,%f,M,0,M,,,,*??" % (self.now.strftime ("%H%M%S.00"), lat[0], lat[1], lon[0], lon[1], self.nsat, self.hdop, alt)
		ck = self._calc_checksum (sentence)
		sentence = sentence[:-2] + "%02X" % ck
		sentences.append (sentence)

		# GPRMC - Recommended Minimum
		# $GPRMC,123519,A,4807.038,N,01131.000,E,022.4,084.4,230394,003.1,W*6A
		sentence = "$GPRMC,%s,A,%011.6f,%c,%012.6f,%c,%.2f,%.2f,%s,,,A*??" % (self.now.strftime ("%H%M%S.00"), lat[0], lat[1], lon[0], lon[1], self._msToKnots (self.spd), self.trk, self.now.strftime ("%d%m%y"))
		ck = self._calc_checksum (sentence)
		sentence = sentence[:-2] + "%02X" % ck
		sentences.append (sentence)

		# GPGLL - Geographic position, Latitude and Longitude
		# $GPGLL,4505.32567,N,00740.01892,E,215929.00,A*6D
		# SEND LAST!
		sentence = "$GPGLL,%011.6f,%c,%012.6f,%c,%s,A*??" % (lat[0], lat[1], lon[0], lon[1], self.now.strftime ("%H%M%S.00"))
		ck = self._calc_checksum (sentence)
		sentence = sentence[:-2] + "%02X" % ck
		sentences.append (sentence)

		return sentences

	def update (self, elapsedSeconds = None):
		self.step += 1

		if elapsedSeconds is None:
			delta = datetime.datetime.utcnow () - self.now
			self.now = datetime.datetime.utcnow ()
			elapsedSeconds = delta.total_seconds ()
			#~ print "%d second(s) since last update" % elapsedSeconds
		else:
			self.now += datetime.timedelta (seconds = elapsedSeconds)

		# Calc new pos
		self.pos = self._vinz.destination (self.pos, self.trk, distance = Distance (kilometers = self.spd * elapsedSeconds / 1000.0))

		# Update parameters
		dspd = random.uniform (-self.DELTA_SPD, self.DELTA_SPD)
		dtrk = random.uniform (-self.DELTA_TRK, self.DELTA_TRK)
		self.spd = max (0, self.spd + dspd)
		assert self.spd >= 0, "Negative speed"
		self.trk += dtrk
		#~ print dspd, dtrk, "->", self.spd, self.trk

if __name__ == '__main__':
	gps = GpsSimulator ()
	for i in xrange (0, 100):
		for sentence in gps.getSentences ():
			print sentence
		gps.update (300)
