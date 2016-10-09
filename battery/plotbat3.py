#!/usr/bin/env python

import datetime
import sys
import csv

import matplotlib.pyplot as plt
import matplotlib.dates as mdates

LOGFILE = "battery.csv"

def readData ():
	dt = []
	vcc = []
	vbat = []

	with open (LOGFILE, "r") as fp:
		reader = csv.DictReader (fp)
		for row in reader:
			dtstr = "%s %s" % (row['date'], row['time'])
			dtdt = datetime.datetime.strptime (dtstr, "%Y/%m/%d %H:%M:%S")
			if len (dt) > 0 and dtdt < dt[len (dt) - 1]:
				print "Ignoring record: %s" % dtstr
			else:
				dt.append (dtdt)
				vcc.append (float (row['vcc']))
				vbat.append (float (row['vbat']))

	return dt, vcc, vbat

plt.title ("Battery Drainage")
plt.grid (True)
plt.setp (plt.xticks ()[1], rotation = 30, ha = 'right')

dt, vcc, vbat = readData ()
print "Average Vcc: %.2f" % (sum (vcc) / len (vcc))
print "Max/Min Vcc: %.2f/%.2f" % (max (vcc), min (vcc))
print


def getVbatAtTime (dt, vbat, t):
	"""Returns battery voltage at time t"""
	for d, v in zip (dt, vbat):
		if t <= d:
			return v
	return None

# Let's look for a voltage drop of at least 0.2v in 20 min. This means that the
# battery charge is almost over
cutoff = None
for d, v in zip (dt, vbat):
	tdelta = d + datetime.timedelta (minutes = 20)
	vt = getVbatAtTime (dt, vbat, tdelta)
	if vt is not None:
		#~ print d, v
		#~ print tdelta, vt
		vdiff = abs (v - vt)
		#~ print vdiff
		#~ print "--"
		if vdiff >= 0.2 and cutoff is None:
			cutoff = d, v
print "Cutoff should have been at %s at %.2f v" % (cutoff[0], cutoff[1])


first = dt[0]
last = dt[len (dt) - 1]
totsec = (last - first).total_seconds ()
dur = totsec / (60 * 60)
print "Battery duration: %.2f hours" % dur

totsec2 = (cutoff[0] - first).total_seconds ()
dur2 = totsec2 / (60 * 60)
print "Battery duration with recommended cutoff: %.2f hours" % dur2

totsec3 = (last - cutoff[0]).total_seconds ()
dur3 = totsec3 / 60
print "Duration missed with cutoff: %d minutes" % dur3

# Time to get a 20% discharge
N = 5
t10 = totsec2 / N
print "Battery discharges 20%% in %d minutes" % (t10 / 60)

thresholds = []
x = first
for j in xrange (0, N + 1):
	vt = getVbatAtTime (dt, vbat, x)
	print "Battery had %d%% charge at %s with V = %.2f" % (100 - j * (100 / N), x, vt)
	thresholds.append (vt)
	x += datetime.timedelta (seconds = t10)
#~ print "const byte BATTERY_POINTS[] = {%s};" % ", ".join (str (x) for x in thresholds[1:])	# No need for 1st element
print "const byte BATTERY_POINTS[] = {%s};" % ", ".join (str (int (x * 100 - 150)) for x in thresholds[1:])	# No need for 1st element

# Plot graph
plt.plot (dt, vcc, "b", label = "Vcc")
plt.plot (dt, vbat, "r", label = "Vbat")
plt.ylabel ('Voltage (V)')
plt.legend (loc = 'best', fancybox = True, framealpha = 0.5)
plt.show ()
