# -*- coding: utf-8 -*-
"""
Created on Thu Aug 28 19:08:36 2014

@author: Denny Verbeeck
"""
import numpy as np
import matplotlib.pyplot as plt

x = np.arange(1, 10)
y = np.exp(-x)
y = [i + np.random.normal() * 0.01 for i in y]

plt.plot(x, y)
plt.grid()

ymean = np.mean(y)
ystddev = np.std(y)
plt.hold(True)

plt.plot([0,10],[ymean,ymean],'r')
plt.plot([0,10],[ymean - ystddev, ymean - ystddev],'--r')
plt.plot([0,10],[ymean + ystddev, ymean + ystddev],'--r')

Y = np.array(y)

for i in np.arange(1, 9):
  Yl = Y[0:i]
  Yr = Y[i:9]
  print Yl
  print Yr