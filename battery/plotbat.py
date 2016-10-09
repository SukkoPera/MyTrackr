#!/usr/bin/env python

import re
import datetime
import sys

import matplotlib.pyplot as plt
import matplotlib.dates as mdates

LOGFILE = "/tmp/logbat.txt"

REGEX = re.compile ("\[([^]]+)\] ([0-9]+), Vcc = ([0-9]+\.[0-9]+) V, Vbat = ([0-9]+\.[0-9]+) V")

def readData ():
	x = []
	vcc = []
	vbat = []

	with open (LOGFILE, "r") as fp:
		line = fp.readline ()
		while line:
			line = line.strip ()
			m = REGEX.match (line)
			if m:
				dt = datetime.datetime.strptime (m.group (1), "%Y-%m-%d %H:%M:%S.%f")
				#~ x.append (m.group (2))
				x.append (dt)
				vcc.append (m.group (3))
				vbat.append (m.group (4))
			else:
				print "Line ignored: %s" % li
			line = fp.readline ()

	return x, vcc, vbat

LOGFILE2 = "/tmp/logbat2.txt"

REGEX2 = re.compile ("\[([^]]+)\] ([0-9]+),([0-9]+\.[0-9]+),([0-9]+\.[0-9]+)")

def readData2 ():
	x = []
	vcc = []
	vbat = []

	with open (LOGFILE2, "r") as fp:
		line = fp.readline ()
		while line:
			line = line.strip ()
			m = REGEX2.match (line)
			if m:
				if float (m.group (3)) <= 5 and float (m.group (4)) <= 5:
					dt = datetime.datetime.strptime (m.group (1), "%Y-%m-%d %H:%M:%S.%f")
					#~ x.append (m.group (2))
					x.append (dt)
					vcc.append (m.group (3))
					vbat.append (m.group (4))
					#~ print m.groups ()
				else:
					print "Line ignored: %s" % line
			else:
				#~ print "Line ignored: %s" % line
				pass
			line = fp.readline ()

	return x, vcc, vbat


plt.title ("Battery Drainage")
plt.grid (True)
plt.setp (plt.xticks ()[1], rotation = 30, ha = 'right')

# Enable this for one-shot graph
if False:
	x, vcc, vbat = readData ()
	x2, vcc2, vbat2 = readData2 ()
	x.extend (x2)
	vcc.extend (vcc2)
	vbat.extend (vbat2)
	#~ start = x[0]
	#~ xdelta = [(i - start) for i in x]
	#~ xnew = [datetime.datetime.fromtimestamp (td.total_seconds () - 3600) for td in xdelta]
	#~ plt.plot (x, vcc, "b", label = "Vcc (incorrect)")
	plt.plot (x, vbat, "r", label = "Vbat")
	plt.legend (loc = 'best', fancybox = True, framealpha = 0.5)
	#~ plt.xticks (range (len (x)), xdelta, rotation = 45, size='small')
	#~ plt.axis ("auto")
	plt.show ()
	sys.exit (0)

x, vcc, vbat = readData ()
x2, vcc2, vbat2 = readData2 ()
x.extend (x2)
vcc.extend (vcc2)
vbat.extend (vbat2)
lvcc = plt.plot (x, vcc, "b", label = "Vcc (incorrect)")[0]
lvbat = plt.plot (x, vbat, "r", label = "Vbat")[0]
plt.ion ()		# Enable interactive plotting
while True:
	x, vcc, vbat = readData ()
	x2, vcc2, vbat2 = readData2 ()
	x.extend (x2)
	vcc.extend (vcc2)
	vbat.extend (vbat2)
	#~ start = x[0]
	#~ xdelta = [(i - start) for i in x]
	#~ xnew = [datetime.datetime.fromtimestamp (td.total_seconds () - 3600) for td in xdelta]
	#~ plt.plot (x, vcc, "b", label = "Vcc (incorrect)")
	#~ plt.plot (x, vbat, "r", label = "Vbat")
	lvcc.set_xdata (x)
	lvcc.set_ydata (vcc)
	lvbat.set_xdata (x)
	lvbat.set_ydata (vbat)
	plt.legend (loc = 'best', fancybox = True, framealpha = 0.5)
	#~ plt.xticks (range (len (x)), xdelta, rotation = 45, size='small')
	#~ plt.axis ("auto")
	plt.draw ()
	plt.pause (8)		# Seconds
