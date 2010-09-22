"""Runs a remote_ctl server inside QtiPlot.

Chris Nicolai 2010

Usage:

    >>> global remote_ctl_server
    >>> remote_ctl_server = ServerClass()
    >>> remote_ctl_server.start()
    >>> ...
    >>> remote_ctl_server.stop()

"""

import os
from PyQt4 import QtCore
import qti
import remote_ctl
import Queue
import sys
import tempfile

TIMER_MS = 80
MAX_PER_TIMER = 10

sockid_path = os.path.join(tempfile.gettempdir(), 'qtiplot.sockid')

class ServerClass(object):
    """Manages the L{remote_ctl.RemoteServer} and a Qt timer to service it."""
    def __init__(self):
        self.remote = None
    def start(self, timer_ms=TIMER_MS, max_per_timer=MAX_PER_TIMER):
        """Starts the server and timer.start

@param timer_ms: checks for requests every this many milliseconds
@param max_per_timer: processes up to this many, if avail, each time
"""
        self.max_per_timer = max_per_timer
        self.remote = remote_ctl.RemoteServer(sockid_path=sockid_path)
        self.timer = QtCore.QTimer()
        QtCore.QObject.connect(self.timer, QtCore.SIGNAL("timeout()"), self.do_work)
        self.timer.start(timer_ms)
    def stop(self):
        self.timer.stop()
        self.remote.stop()
        self.timer = self.remote = None
    def do_work(self):
        for i in xrange(self.max_per_timer):
            if not self.remote.process_one():
                break

