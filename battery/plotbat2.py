#!/usr/bin/env python

import datetime
import sys
import csv

import matplotlib.pyplot as plt
import matplotlib.dates as mdates

LOGFILE = "logbat5.csv"

def readData ():
	x = []
	vcc = []
	vbat = []

	with open (LOGFILE, "r") as fp:
		reader = csv.DictReader (fp)
		for row in reader:
			x.append (row['no'])
			vcc.append (row['vcc'])
			vbat.append (row['vbat'])

	return x, vcc, vbat

plt.title ("Battery Drainage")
plt.grid (True)
plt.setp (plt.xticks ()[1], rotation = 30, ha = 'right')

x, vcc, vbat = readData ()
#~ start = x[0]
#~ xdelta = [(i - start) for i in x]
#~ xnew = [datetime.datetime.fromtimestamp (td.total_seconds () - 3600) for td in xdelta]
#~ plt.plot (x, vcc, "b", label = "Vcc (incorrect)")
plt.plot (x, vbat, "r", label = "Vbat")
plt.legend (loc = 'best', fancybox = True, framealpha = 0.5)
#~ plt.xticks (range (len (x)), xdelta, rotation = 45, size='small')
#~ plt.axis ("auto")
plt.show ()
