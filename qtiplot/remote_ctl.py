"""Gives outside processes access to the Python interpreter.

Chris Nicolai 2010

A dedicated thread listens on a TCP port (by default, on the loopback interface '127.0.0.1' only).
Client and server alternately send messages, which consist of a verb, e.g. 'EVAL', and depending
on the verb, sdata (string data) such as '2**5', or bytes (binary array data).

Usage (server):

    >>> server = RemoteServer(host='127.0.0.1', port=55384, seek_ports=10, sockid_path=None)

Listens on the first available port in 55384..(55384+10).  Writes the actual port number
to a text file at sockid_path, if provided.  Enqueues requests for processing, but as most
applications require everything to run on the same thread, it requires you to call

    >>> server.process_one()

at frequent intervals, such as on a timer.  To stop the server:

    >>> server.stop()


Usage (client):

    >>> client = RemoteProxy(host='127.0.0.1', port=55384, sockid_path=None)

Connects to the server.  Uses the port number in sockid_path, if it exists.

    >>> import numpy
    >>> arr = numpy.array([1,2,3,4,5,6,7,8,9,10]).reshape((2,5))
    >>> print arr
    [[ 1  2  3  4  5]
     [ 6  7  8  9 10]]
    >>> client.set_array(arr, 'arr')
    >>> client.reval("arr[1,2]")
    '8'
    >>> client.rexec(\"""import numpy
    arr2 = numpy.array(arr, copy=True)
    arr2 *= 2
    arr2 -= 1\""")
    >>> local_arr2 = client.get_array('arr2')
    >>> print local_arr2
    [[ 1  3  5  7  9]
     [11 13 15 17 19]]
    >>> client.stop()


Protocol:

Strictly alternating messages, starting with the client.  Certain messages are always multi-line, and terminated with a period on its own line.  This transcript has all the possible interactions (>: client; <: server)

    >: VERSION<tab>1.0
    <: VERSION<tab>1.0
    >: EXEC
    >: import numpy
    >: table = newTable()
    >: .
    <: DONE
    >: EXEC
    >: i fnot (table is None):
    >:     table.setWindowLabel("tutu")
    <: ERROR
    <: Traceback (most recent call last):
    <: [...]
    <:   File "<string>", line 1:
    <:     i fnot (table is None):
    <:          ^
    <: Syntax error: invalid syntax
    <: .
    >: EVAL<tab>t
    <: VAL
    <: '<qti.Table object at 0x04cc2687>'
    <: .
    >: EVAL<tab>nonsense
    <: ERROR
    <: Traceback (most recent call last):
    <: [...more traceback...]
    <: .
    >: ARRAY<tab>3<tab>4<tab>float32<tab>remote_label
    >: [3*4*sizeof(float32) bytes of array data]
    >: .
    <: SET
    >: EVAL<tab>remote_label[1,2]
    <: VAL
    <: 34.6
    <: .
    >: GET ARRAY<tab>remote_label
    <: ARRAY<tab>3<tab>4<tab>float32<tab>remote_label
    <: [3*4*sizeof(float32) bytes of array data]
    <: .
"""

import __main__
import cStringIO
import numpy
import Queue
import re
import socket
import select
import struct
import sys
from threading import *
import time
import traceback

DEFAULT_HOST = '127.0.0.1'
DEFAULT_PORT = 55384

class Stop(Exception):
    """
    Raised and caught internally to break out of a read when Socket.stop() is called
    """
    def __init__(self):
        Exception.__init__(self, 'stop requested')

class Socket(object):
    """
Socket(host, port, async=True)

Common base class for sockets that communcate by structured blocks.
e.g. L{RemoteConnect}.
A Socket can manage its own reading-thread and enqueue messages for user-thread processing.

To specialize, override readMsg, sendMsg, msgNonNull, and msgIsNull.

    @ivar sock: a socket.socket
    """
class Socket(object):
    def __init__(self, sock, async=True):
        self.sock = sock
        self.sock.settimeout(1.0)
        self.stopFlag = False
        self.stopped = False
        self.async = async
        if async:
            self.rq = Queue.Queue()
            self.thread = Thread(target=self.readerThread)
            self.thread.start()
    def __del__(self):
        try:
            if async:
                self.stopFlag = True
                self.thread.join()
            else:
                self.sock.close()
        except:
            pass
    def recvMsg(self, block=True, timeout=None): # async
        """
::
  if async:
      return the next message from the queue, possibly blocking until timeout.
      if the timeout is reached, raise Queue.empty
  if not async:
      return readMsg()
"""
        if self.async:
            return self.rq.get(block, timeout) # raises Queue.Empty if timeout or not block
        else: # ignoring block, timeout
            return self.readMsg()
    def readerThread(self):
        msg = self.msgNonNull()
        while (not self.msgIsNull(msg)) and (not self.stopFlag):
            msg = self.readMsg()
            if not self.msgIsNull(msg):
                self.rq.put( msg )
        self.sock.close()
        self.stopped = True
    def stop(self):
        """If async, stops the reader thread."""
        self.stopFlag = True
    def done(self):
        """Returns True if the socket is closed and rq is empty."""
        return self.stopped and not self.rq.qsize()
    def readBlock(self, count):
        """
Returns a block of count bytes as a string,
or raises Stop,
or any relevant socket.error from select or recv.
"""
        stopRequested = lambda: self.async and self.stopFlag
        block = ''
        while len(block) < count:
            if stopRequested():
                raise Stop()
            try:
                readers, writers, errs = select.select((self.sock,), (), (self.sock,), 1.0)
                if errs:
                    raise socket.error()
                if readers:
                    sub = self.sock.recv(count-len(block))
                    if len(sub) == 0:
                        raise socket.error()
                    block += sub
            except socket.timeout:
                pass
        return block
    def readLine(self, terminator='\n'):
        """
Returns the next line as a string, including the trailing terminator,
or raises Stop,
or any relevant socket.error from select or recv.
"""
        stopRequested = lambda: self.async and self.stopFlag
        block = cStringIO.StringIO()
        while True:
            if stopRequested():
                raise Stop()
            try:
                readers, writers, errs = select.select((self.sock,), (), (self.sock,), 1.0)
                if errs:
                    raise socket.error()
                if readers:
                    sub = self.sock.recv(1)
                    if len(sub) == 0:
                        raise socket.error()
                    block.write(sub)
                    if sub == terminator:
                        break
            except socket.timeout:
                pass
        return block.getvalue()
    def sendBlock(self, block):
        """Sends block as a string of bytes; returns True on success, False if the socket closed."""
        blocklen = len(block)
        sent = 0
        while sent < blocklen:
            try:
                sent1 = self.sock.send(block[sent:])
                if sent1 == 0: # hangup
                    self.stop()
                    return False
                sent += sent1
            except socket.error:
                self.stop()
                return False
            except socket.timeout:
                pass
        return True
    def msgNonNull(self):
        """Override this method to return a (simple) non-null message (for use as place-holder)."""
        raise Exception('unimplemented')
    def msgIsNull(self, msg):
        """Override this method to return whether a message is null."""
        raise Exception('unimplemented')
    def sendMsg(self, msg):
        """Override this method to send a message out self.sock."""
        raise Exception('unimplemented')
    def readMsg(self):
        """
Override this method to read a structured message using readBlock and return it.
Returns a null message to indicate end-of-stream (handle all exceptions).
"""
        raise Exception('unimplemented')

def connect_as_client(host, port):
    """Returns a new client L{socket.socket} connected to host, port."""
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sock.connect((host, port))
    return sock

class TCPServer(object):
    """Listens for connections on its own thread; calls session_f(each new L{socket.socket}).
    
    @ivar host:
    @ivar port:
"""
    def __init__(self, host, port, session_f, seek_ports=0):
        """
    @param host:
    @param port: base TCP port number
    @param session_f: function(L{socket.socket}) called on each new connection
    @param seek_ports: attempts to bind ports (port, port+1, port+2, ..., port+seek_ports) until a port is free
"""
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.session_f = session_f
        self.host = host
        self.port = port
        ports2check = seek_ports + 1
        while True:
            try:
                self.sock.bind((host, self.port))
                break
            except socket.error:
                if ports2check <= 1:
                    raise # exit on socket.error when ports2check are exhausted
                self.port += 1
                ports2check -= 1
        self.sock.listen(1)
        self.sock.settimeout(1.0)
        self.stopFlag = False
        self.thread = Thread(target=self.listenThread)
        self.thread.start()
    def __del__(self):
        try:
            self.stopFlag = True
            self.thread.join()
        except:
            pass
    def stop(self):
        """Requests the listening thread to stop and close the socket; does not .thread.join()."""
        self.stopFlag = True
    def listenThread(self):
        while not self.stopFlag:
            try:
                client, addr = self.sock.accept()
                self.session_f(client, addr)
            except socket.timeout:
                pass
            except:
                traceback.print_exc()
        self.sock.close()




class RemoteMsg(object):
    """Represents one message with associated data.

@ivar verb: one of ['VERSION', 'EVAL', 'VAL', 'EXEC', 'DONE', 'ERROR', 'ARRAY', 'SET', 'GET ARRAY']
@ivar sdata: payload for VERSION, EVAL, VAL, EXEC, ERROR, GET_ARRAY; metadata for ARRAY
@ivar bytes: payload for ARRAY
"""
    def __init__(self, verb=None, sdata=None, bytes=None):
        self.verb = verb
        self.sdata = sdata
        self.bytes = bytes

def RemoteArray(arr, label):
    """Returns a L{RemoteMsg} containing arr and its label.

    verb = 'ARRAY'
    sdata = "%(rows)i\t%(cols)i\t%(dtype)s\t%(label)s"
    bytes = arr.tostring()

    @arr: a numpy.array, or something legal for numpy.array(arr)
    @label: remote python identifier for the array
"""
    try:
        arr.dtype
    except AttributeError:
        arr = numpy.array(arr)
    if len(arr.shape) == 1:
        return RemoteMsg('ARRAY', '%i\t%i\t%s\t%s' % (arr.shape[0], 1, str(arr.dtype), label), arr.tostring())
    elif len(arr.shape) == 2:
        return RemoteMsg('ARRAY', '%i\t%i\t%s\t%s' % (arr.shape[0], arr.shape[1], str(arr.dtype), label), arr.tostring())
    else:
        raise Exception('Unsupported array dimension: %s (need 1D or 2D)' % arr.shape)

def RemoteToArray(msg):
    """Returns (label, arr) contained in a L{RemoteArray} message."""
    dims = re.match(r"([01-9]+)\t([01-9]+)\t([^\t]+)\t?(.*)", msg.sdata)
    if dims:
        rows, cols, dtype, label = [dims.group(i) for i in (1,2,3,4)]
        rows, cols = int(rows), int(cols)
        return label, numpy.frombuffer(msg.bytes, dtype, rows*cols).reshape((rows, cols))
    else:
        raise Exception('bad array format: %s' % msg.sdata)


class RemoteConnect(Socket):
    """Reads and writes L{RemoteMsg}s."""
    def __init__(self, sock, async=True):
        Socket.__init__(self, sock, async)
    def msgNonNull(self):
        return RemoteMsg('placeholder')
    def msgIsNull(self, msg):
        return msg.verb is None
    def sendMsg(self, msg):
        """Sends a L{RemoteMsg}.  Does not catch any potential socket.error s."""
        if msg.verb in ('EXEC', 'VAL', 'ERROR', 'SET', 'DONE'):
            self.sendBlock('%s\n' % msg.verb)
        else:
            self.sendBlock('%s\t%s\n' % (msg.verb, msg.sdata))
        if msg.verb in ('EXEC', 'VAL', 'ERROR'):
            self.sendBlock('%s\n.\n' % msg.sdata)
        elif msg.bytes:
            self.sendBlock('%s\n.\n' % msg.bytes)
    def readMsg(self):
        """Don't call this directly; use L{Socket.recvMsg}."""
        msg = RemoteMsg()
        while not msg.verb:
            try:
                line = self.readLine()
                fields = re.match(r"([^\t]+)\t?(.*)", line)
                if not fields: continue # big problem?
                verb = fields.group(1).strip()
                sdata = fields.group(2).replace('\r', '')
                if verb in ('EXEC', 'VAL', 'ERROR'):
                    script = cStringIO.StringIO()
                    while True:
                        line = self.readLine().replace('\r', '')
                        if line.strip() == '.':
                            break
                        script.write(line)
                    return RemoteMsg(verb, script.getvalue()[:-1]) # skip final newline
                elif verb == 'ARRAY':
                    dims = re.match(r"([01-9]+)\t([01-9]+)\t([^\t]+)\t?(.*)", sdata)
                    if dims:
                        rows, cols, dtype, label = [dims.group(i) for i in (1,2,3,4)]
                        rows, cols = int(rows), int(cols)
                        bytes = self.readBlock(rows*cols*numpy.dtype(dtype).itemsize)
                        msg = RemoteMsg(verb, sdata, bytes)
                    while self.readLine().strip() != '.':
                        pass
                    return msg
                else:
                    return RemoteMsg(verb, sdata)
            except socket.error:
                return RemoteMsg() # null message closes socket
            except KeyboardInterrupt:
                raise
            except:
                return RemoteMsg('ERROR', traceback.format_exc())

class RemoteSession(RemoteConnect):
    """Handles the server side of one connection, with its own locals dict.
Enqueues L{RemoteMsg}s for processing on its own thread;
you call session.process_one() frequently on the appropriate thread.
"""
    def __init__(self, sock, async=True):
        RemoteConnect.__init__(self, sock, async)
        self.locals = {}
    def process_one(self):
        """Processes and answers at most one message; returns False if there are none."""
        try:
            msg = self.recvMsg(block=False)
        except Queue.Empty:
            return False
        try:
            if msg.verb == 'EXEC':
                exec(msg.sdata, __main__.__dict__, self.locals)
                self.sendMsg(RemoteMsg('DONE'))
            elif msg.verb == 'EVAL':
                self.sendMsg(RemoteMsg('VAL', str(eval(msg.sdata, __main__.__dict__, self.locals))))
            elif msg.verb == 'ARRAY':
                lbl, arr = RemoteToArray(msg)
                self.locals[lbl] = arr
                self.sendMsg(RemoteMsg('SET'))
            elif msg.verb == 'GET ARRAY':
                self.sendMsg(RemoteArray(self.locals[msg.sdata], msg.sdata))
            elif msg.verb == 'ERROR': # in recv/parse
                self.sendMsg(msg)
            elif msg.verb == 'VERSION':
                # TODO: modify behavior for older versions (oldest is 1.0)
                self.sendMsg(RemoteMsg('VERSION', '1.0'))
            else:
                self.sendMsg(RemoteMsg('ERROR', 'unknown verb: %s'%msg.verb))
        except socket.error:
            self.stop()
        except KeyboardInterrupt:
            raise
        except:
            self.sendMsg(RemoteMsg('ERROR', traceback.format_exc()))
        return True
            

class RemoteServer(TCPServer):
    """Listens on the first available port in port..(port+seek_ports).  Writes the actual port number
to a text file at sockid_path, if provided.  One L{RemoteSession} per client enqueues requests for processing,
but as most applications require everything to run on the same thread, you must call

    >>> server.process_one()

at frequent intervals, such as on a timer.  To stop the server:

    >>> server.stop()
"""
    def __init__(self, host='127.0.0.1', port=55384, seek_ports=10, sockid_path=None):
        """Raises socket.error if no ports available.

@ivar process_one:
"""
        self.conns = []
        TCPServer.__init__(self, host, port, lambda client, addr: self.conns.append(RemoteSession(client)), seek_ports)
        self.process_one = self.__process().next
        if sockid_path:
            try:
                open(sockid_path, 'w').write("%i\n"%self.port)
            except:
                traceback.print_exc()
        print 'Remote control started on port %i' % self.port
    def __process(self):
        """Generator function to iterate over active connections; yielding after one job, or a full circle."""
        since_yield = 0
        while True:
            if not self.conns:
                yield
            for i in reversed(xrange(len(self.conns))):
                if (since_yield == len(self.conns)) or self.conns[i].process_one():
                    yield
                    since_yield = 0
                else:
                    since_yield += 1
                if self.conns[i].done():
                    del self.conns[i]
    def stop(self):
        """Stops the listening thread and closes the socket."""
        TCPServer.stop(self)
        for conn in self.conns:
            conn.stop()
        print 'Remote control stopped.'



class RemoteError(Exception):
    """Contains the traceback from an exception on the remote side."""
    def __init__(self, traceback):
        Exception.__init__(self, 'Remote error: %s' % traceback)



class RemoteProxy(object):
    """Client for L{RemoteServer}.  (block, timeout) in methods are the same as in module Queue."""
    def __init__(self, host='127.0.0.1', port=55384, sockid_path=None):
        """Connects to the server.  Uses the port number in sockid_path, if it exists."""
        rport = port
        if sockid_path:
            try:
                rport = int( open(sockid_path, 'r').readline().strip() )
            except:
                traceback.print_exc()
                print 'No/bad sockid file (%s); falling back on port %i' % (sockid_path, rport)
        self.conn = RemoteConnect(connect_as_client(host, rport))
        self.conn.sendMsg(RemoteMsg('VERSION', '1.0'))
        self.__check_error(True, 5.0)
    def reval(self, expr, block=True, timeout=None):
        """Evaluates expr on the remote side, returns the result as a string.  Can raise RemoteError."""
        self.conn.sendMsg(RemoteMsg('EVAL', expr))
        result = self.__check_error(block, timeout)
        return result.sdata
    def rexec(self, buf, block=True, timeout=None):
        """Executes the Python script in buf on the remote side.  Can raise RemoteError."""
        self.conn.sendMsg(RemoteMsg('EXEC', buf))
        self.__check_error(block, timeout)
    def set_array(self, arr, label, block=True, timeout=None):
        """Sends a numpy.array to the remote side, and assigns it to the remote identifier label."""
        self.conn.sendMsg(RemoteArray(arr, label))
        self.__check_error(block, timeout)
    def get_array(self, label, block=True, timeout=None):
        """Loads a numpy.array from the remote side, with the remote name label.  Can raise RemoteError."""
        self.conn.sendMsg(RemoteMsg('GET ARRAY', label))
        result = self.__check_error(block, timeout)
        return RemoteToArray(result)[1]
    def __check_error(self, block=True, timeout=None):
        result = self.conn.recvMsg(block, timeout)
        if result.verb == 'ERROR':
            raise RemoteError(result.sdata)
        return result
    def stop(self):
        """Closes the connection to L{RemoteServer}."""
        self.conn.stop()



if __name__ == "__main__":
    import os
    import tempfile
    
    SERVER_TIMER_SEC = 0.2 # processing max 1 message every this many seconds
    SOCKID_NAME = 'remote_ctl.sockid' # file to hold socket number, in the temp dir

    # optional first argument: port number
    sockid_path = os.path.join(tempfile.gettempdir(), SOCKID_NAME)
    if len(sys.argv) > 1:
        server = RemoteServer(port=int(sys.argv[1]), seek_ports=0, sockid_path=sockid_path)
    else: # no port provided: try DEFAULT_PORT + 10
        server = RemoteServer(port=DEFAULT_PORT, seek_ports=10, sockid_path=sockid_path)
    try:
        while True:
            server.process_one()
            time.sleep(SERVER_TIMER_SEC)
    except KeyboardInterrupt:
        pass
    except:
        traceback.print_exc()

    server.stop()

