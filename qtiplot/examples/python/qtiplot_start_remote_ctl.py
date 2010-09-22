"""To make QtiPlot scriptable by outside apps, move this script into the startup script folder,
and make sure the default scripting language is Python."""

import qtiplot_remote_ctl

globals.remote_ctl_server = qtiplot_remote_ctl.ServerClass()
globals.remote_ctl_server.start()
displayInfo("Remote control server started on port %d" % globals.remote_ctl_server.remote.port)

