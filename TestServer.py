#!/usr/local/bin/python -utt
# ======================================================
#                   M C L S e r v e r . p y
# ======================================================

__author__  = 'Dean Earl Wright III <dean[three][at]UMBC[dot]edu>'
__date__    = '04 June 2010'
__version__ = '$Revision: 0 $'

"""MCLServer.py

Connect to (or fake a) TCP MCL Server.
"""

# ======================================================
#                                                imports
# ======================================================
import os
import string
import sys
import time
import unittest
import telnetlib
from optparse import OptionParser
import signal

#import MCLResponse

# ======================================================
#                                              constants
# ======================================================
MCL_HOST = 'localhost'
MCL_PORT = 5150
MCL_FAKE = 0
MCL_DEBUG = 0
MCL_LOG = None
MCL_TIMEOUT_MIN = 60
MCL_TIMEOUT = 5 * MCL_TIMEOUT_MIN

SERVER_NAME = "MCLserver"
SERVER_OPT  = "--mcl.port"
SERVER_WAIT = 5
KILL_WAIT   = 1
KILL_RETRY  = 20

# ======================================================
#                                                globals
# ======================================================
SERVERS = {}

# ======================================================
#                                              MCLServer
# ======================================================
class MCLServer(object):

    def __init__(self, host=MCL_HOST,
                       port=MCL_PORT,
                       debug=MCL_DEBUG,
                       log=MCL_LOG):

        # 1. Save the TCP host and port
        self.host = host
        self.port = port
        self.debug = debug
        self.log = log

    def send_ok(self, command, timeout=MCL_TIMEOUT, error=True):
        print ("MCLServer:send_ok(self,%s,timeout=%s,error=%s)"
               % (command, timeout, error))

    def set_logFile(self, log):
        self.send_ok('setLog(%s)' % log)

    def simple(self):

        # 1. Set the name of the MCL connection
        self.send_ok('initialize(simple)')

        # 2. Set the name of the MCL ontology to use
        self.send_ok('ontology(simple, basic)')

        # 3. Set the name of the MCL domain
        self.send_ok('configure(simple, simple_domain)')

        # 4. Create location, time, and fuel sensors
        self.send_ok('declareObservableSelf(simple, location, 0)')
        self.send_ok('declareObservableSelf(simple, elasped_time, 0)')
        self.send_ok('declareObservableSelf(simple, fuel, 100)')

        # 5. Set the three sensors' characteristics
        self.send_ok('setObsPropSelf(simple, location, PROP_SCLASS, SC_SPATIAL)')
        self.send_ok('setObsLegalRangeSelf(simple, location, 0, 100)')
        self.send_ok('setObsPropSelf(simple, elasped_time, PROP_SCLASS, SC_TEMPORAL)')
        self.send_ok('setObsPropSelf(simple, fuel, PROP_SCLASS, SC_RESOURCE)')
        self.send_ok('setObsLegalRangeSelf(simple, fuel, 0, 40)')

        # 6. Set some global properties - sensors can fail
        self.send_ok('setPropertyDefault(simple, PCI_SENSORS_CAN_FAIL,PC_YES)')

        # 7. Create a pair of Expectation Groups
        self.send_ok('declareEG(simple,1)')
        self.send_ok('declareEG(simple,2,1,0)')

        # 8. Create expectation for the plan and action
        self.send_ok('declareSelfExp(simple,1,location,ec_take_value,3)')
        self.send_ok('declareSelfExp(simple,2,location,ec_take_value,1)')

        # 9. Do Monitor
        self.send_ok('monitor(simple,{location=0, elasped_time=0, fuel=40})')
        self.send_ok('monitor(simple,{location=0, elasped_time=1, fuel=50})')
        self.send_ok('monitor(simple,{location=0, elasped_time=2, fuel=40})')
        self.send_ok('EGcomplete(simple, 2, {location=0, elasped_time=3, fuel=35})')
        self.send_ok('monitor(simple,{location=0, elasped_time=4, fuel=35})')

        # 10. Exit
        self.send_ok('terminate(simple)')

        # 999. All is well
        return True

    def good_sensors(self):

        # 1. Set the name of the MCL connection
        self.send_ok('initialize(simple)')

        # 2. Set the name of the MCL ontology to use
        self.send_ok('ontology(simple, basic)')

        # 3. Set the name of the MCL domain
        self.send_ok('configure(simple, simple_domain)')

        # 4. Create location, time, and fuel sensors
        self.send_ok('declareObservableSelf(simple, location, 0)')
        self.send_ok('declareObservableSelf(simple, elasped_time, 0)')
        self.send_ok('declareObservableSelf(simple, fuel, 100)')

        # 5. Set the three sensors' characteristics
        self.send_ok('setObsPropSelf(simple, location, PROP_SCLASS, SC_SPATIAL)')
        self.send_ok('setObsLegalRangeSelf(simple, location, 0, 100)')
        self.send_ok('setObsPropSelf(simple, elasped_time, PROP_SCLASS, SC_TEMPORAL)')
        self.send_ok('setObsPropSelf(simple, fuel, PROP_SCLASS, SC_RESOURCE)')
        self.send_ok('setObsLegalRangeSelf(simple, fuel, 0, 40)')

        # 6. Set some global properties - sensors can't fail
        self.send_ok('setPropertyDefault(simple, PCI_SENSORS_CAN_FAIL,PC_NO)')

        # 7. Create a pair of Expectation Groups
        self.send_ok('declareEG(simple,1)')
        self.send_ok('declareEG(simple,2,1,0)')

        # 8. Create expectation for the plan and action
        self.send_ok('declareSelfExp(simple,1,location,ec_take_value,3)')
        self.send_ok('declareSelfExp(simple,2,location,ec_take_value,1)')

        # 9. Do Monitor
        self.send_ok('monitor(simple,{location=0, elasped_time=0, fuel=40})')
        self.send_ok('monitor(simple,{location=0, elasped_time=1, fuel=50})')
        self.send_ok('monitor(simple,{location=0, elasped_time=2, fuel=40})')
        self.send_ok('EGcomplete(simple,2,{location=0, elasped_time=3, fuel=35})')
        self.send_ok('monitor(simple,{location=0, elasped_time=4, fuel=35})')

        # 999. All is well
        return True

    def no_sensor_diag(self):

        # 1. Set the name of the MCL connection
        self.send_ok('initialize(simple)')

        # 2. Set the name of the MCL ontology to use
        self.send_ok('ontology(simple, basic)')

        # 3. Set the name of the MCL domain
        self.send_ok('configure(simple, simple_domain)')

        # 4. Create location, time, and fuel sensors
        self.send_ok('declareObservableSelf(simple, location, 0)')
        self.send_ok('declareObservableSelf(simple, elasped_time, 0)')
        self.send_ok('declareObservableSelf(simple, fuel, 100)')

        # 5. Set the three sensors' characteristics
        self.send_ok('setObsPropSelf(simple, location, PROP_SCLASS, SC_SPATIAL)')
        self.send_ok('setObsLegalRangeSelf(simple, location, 0, 100)')
        self.send_ok('setObsPropSelf(simple, elasped_time, PROP_SCLASS, SC_TEMPORAL)')
        self.send_ok('setObsPropSelf(simple, fuel, PROP_SCLASS, SC_RESOURCE)')
        self.send_ok('setObsLegalRangeSelf(simple, fuel, 0, 40)')

        # 6. Set some global properties - sensors can't fail
        self.send_ok('setPropertyDefault(simple, PCI_SENSORS_CAN_FAIL, PC_NO)')
        self.send_ok('setPropertyDefault(simple, CRC_SENSOR_DIAG, PC_NO)')

        # 7. Create a pair of Expectation Groups
        self.send_ok('declareEG(simple,1)')
        self.send_ok('declareEG(simple,2,1,0)')

        # 8. Create expectation for the plan and action
        self.send_ok('declareSelfExp(simple,1,location,ec_take_value,3)')
        self.send_ok('declareSelfExp(simple,2,location,ec_take_value,1)')

        # 9. Do Monitor
        self.send_ok('monitor(simple,{location=0, elasped_time=0, fuel=40})')
        self.send_ok('monitor(simple,{location=0, elasped_time=1, fuel=50})')
        self.send_ok('monitor(simple,{location=0, elasped_time=2, fuel=40})')
        self.send_ok('EGcomplete(simple,2,{location=0, elasped_time=3, fuel=35})')
        self.send_ok('monitor(simple,{location=0, elasped_time=4, fuel=35})')

        # 999. All is well
        return True

# ======================================================
#                                           MCLServerTCP
# ======================================================
class MCLServerTCP(MCLServer):

    def __init__(self, host=MCL_HOST,
                       port=MCL_PORT,
                       debug=MCL_DEBUG,
                       log=MCL_LOG):

        # 1. Initialize our parent
        MCLServer.__init__(self, host=host,
                                 port=port,
                                 debug=debug,
                                 log=log)

        # 2. Open telnet port
        self.telnet = telnetlib.Telnet(host, port)

        # 3. Set debugging level
        if debug and self.telnet:
            self.telnet.set_debuglevel(debug)

        # 4. Last command and result
        self.command = None
        self.response = None

        # 5. If logging to file, start it up
        if log:
            self.set_logFile(log)

    def send_ok(self, command, timeout=MCL_TIMEOUT, error=True):
        if not self.telnet:
            return False
        self.command = command
        self.telnet.write(command)
        self.response = self.telnet.read_until('\n', timeout).strip()
        if not self.response.startswith('ok'):
            if error:
                raise Exception('%s --> %s' %
                                (self.command, self.response))
            return False
        return True

    def monitor(self, command, timeout=MCL_TIMEOUT, error=True):
        self.send_ok(command, timeout=timeout, error=error)
        return MCLResponse.MCLResponse.decode_monitor_response(self.response[3:-1])

    def set_debuglevel(self, level):
        if self.telnet:
            self.telnet.set_debuglevel(level)

    def close(self):
        if self.telnet:
            try:
                self.set_logFile('stdout')
            except Exception:
                pass
            self.telnet.close()
        self.telnet = None

# ======================================================
#                                          MCLServerFake
# ======================================================
class MCLServerFake(MCLServer):

    def __init__(self, host=MCL_HOST,
                       port=MCL_FAKE,
                       debug=MCL_DEBUG,
                       log=MCL_LOG):

        # 1. Initialize our parent
        MCLServer.__init__(self, host=host,
                                 port=port,
                                 debug=debug,
                                 log=log)

        # 2. Last command and result
        self.command = None
        self.response = None

    def send_ok(self, command, timeout=MCL_TIMEOUT, error=True):
        self.command = command
        self.response = 'ok()'
        if timeout: pass
        if error: pass

    def monitor(self, command, timeout=MCL_TIMEOUT, error=True):
        self.command = command
        self.response = 'ok([])'
        if timeout: pass
        if error: pass
        return []

    def set_debuglevel(self, level):
        self.debug = level

    def close(self):
        pass

# ======================================================
#                                       start_mcl_server
# ======================================================
def start_mcl_server(name=SERVER_NAME,
                     host=MCL_HOST, port=MCL_PORT,
                     debug=MCL_DEBUG, verbose=False,
                     verify=False):

    # 1. Fake servers are always started
    if port == MCL_FAKE:
        return True

    # 2. Stop it if we think it may be running
    if port in SERVERS:
        kill_off_server(SERVERS[port])
        del SERVERS[port]

    # 3. Start a real server and record the pid id
    pid = os.spawnlp(os.P_NOWAIT,
                     SERVER_NAME,
                     SERVER_NAME,
                     SERVER_OPT,
                     str(port))
    SERVERS[port] = pid
    time.sleep(SERVER_WAIT)

    # 4. Verify that it is started (if requested)
    if verify:
        if verbose:
            print ("%s: connecting to MCL at %s port %s"
                  % (name, host, port))
        try:
            mcl = MCLServerTCP(host=host, port=port, debug=debug)

        # 5. Return False if it is not
        except Exception, e:
            print ("%s: unable to connect to MCL at %s port %s"
                  % (name, host, port))
            print e
            kill_off_server(pid)
            return False

        # 6. Return True if it is
        mcl.close()

    # 7. Return success
    return True

# ======================================================
#                                        stop_mcl_server
# ======================================================
def stop_mcl_server(name=SERVER_NAME,
                    host=MCL_HOST, port=MCL_PORT,
                    debug=MCL_DEBUG, verbose=False):

    # 1. Fake servers need no stopping
    if port == MCL_FAKE:
        return True

    # 2. Connect to the server
    if verbose:
        print ("%s: connecting to MCL at %s port %s"
              % (name, host, port))
    try:
        mcl = MCLServerTCP(host=host, port=port, debug=debug)

    # 3. Return False if we can't connect
    except Exception, e:
        print ("%s: unable to connect to MCL at %s port %s"
              % (name, host, port))
        print e
        if port in SERVERS:
            kill_off_server(SERVERS[port])
            del SERVERS[port]
        return False

    # 4. Send a terminate request
    mcl.send_ok('initialize(%s)' % name)
    mcl.send_ok('terminateWithExtremePrejudice(%s)' % name)
    mcl.close()
    time.sleep(KILL_WAIT)

    # 5. We ask nicely only once
    if port in SERVERS:
        kill_off_server(SERVERS[port])
        del SERVERS[port]

    # 6. Return success
    return True

# ======================================================
#                                        kill_off_server
# ======================================================
def kill_off_server(process_id):

    # 1. Ask nicely
    try:
        os.kill(process_id, signal.SIGHUP)
    except OSError:
        return True

    # 2. Give it a chance
    time.sleep(KILL_WAIT)

    # 3. Really make it go away
    try:
        os.kill(process_id, signal.SIGKILL)
    except OSError:
        return True

    # 4. Give it a chance
    time.sleep(KILL_WAIT)

    # 5. Really, really make it go away
    for _ in range(KILL_RETRY):
        try:
            os.kill(process_id, signal.SIGKILL)
        except OSError:
            return True
        time.sleep(KILL_WAIT)

    # 6. We gave it our best shot
    return False

# ======================================================
#                                             experiment
# ======================================================
def experiment(name, host=MCL_HOST, port=MCL_PORT,
                     debug=MCL_DEBUG, verbose=False):

    # 1. Connect to the MCL server and set debuging level
    if verbose:
        print ("%s: connecting to MCL at %s port %s"
              % (name, host, port))
    try:
        if port == MCL_FAKE:
            mcl = MCLServerFake(host=host, port=port,
                                debug=debug)
        else:
            mcl = MCLServerTCP(host=host, port=port,
                               debug=debug)
    except Exception, e:
        print ("%s: unable to connect to MCL at %s port %s"
              % (name, host, port))
        print e
        return False

    # 2. Carry out the experiment
    result = False
    try:
        if callable(getattr(mcl,name)):
            result = getattr(mcl,name)()

    # 3. Close the connection
    finally:
        mcl.close()

    # 4. Return the experiment's result
    return result

# ======================================================
#                                        test_mcl_server
# ======================================================
def test_mcl_server(host=MCL_HOST, port=MCL_FAKE,
                    debug=MCL_DEBUG, verbose=False):
    experiment('simple', host=host, port=port,
                         debug=debug, verbose=verbose)
    experiment('good_sensors', host=host, port=port,
                               debug=debug, verbose=verbose)
    experiment('no_sensor_diag', host=host, port=port,
                                 debug=debug, verbose=verbose)

# ======================================================
#                                               unittest
# ======================================================
class Test_MCLServer(unittest.TestCase):

    def test_MCLServer_empty(self):
        s = MCLServer()
        self.assertNotEqual(s, None)

# ======================================================
#                                       parseCommandLine
# ======================================================
def parseCommandLine():

    # 1. Create the command line parser
    parser = OptionParser()
    parser.add_option("-u", "--unittest", default=False,
                      action="store_true", dest="unittest",
                      help="Execute unittests")
    parser.add_option("-v", "--verbose", default=False,
                      action="store_true", dest="verbose",
                      help="Print status messages to stdout")
    parser.add_option("-o", "--port", default=MCL_PORT,
                      action="store", dest="port",
                      help="MCL server port number")
    parser.add_option("-d", "--debug", default=MCL_DEBUG,
                      action="count", dest="debug",
                      help="Telnet debug level: default=%s" % MCL_DEBUG)

    # 2. Parse the command line
    options, args = parser.parse_args()
    options.explain = []

    # 3. Validate mcl port
    try:
        options.port = int(options.port)
    except ValueError:
        parser.error('Invalid MCL port number')

    # 4. Check the arguments (if any)
    if args:
        parser.error('No arguments allowed')

    # 5. Return the options
    return options

#=============================================
#in-between-parser
#=============================================

MCL_NAME = "test_name"

def send_command(server, command, params):
	params.insert(0, MCL_NAME)
	str = command + '(' + string.join(params, ',') + ')'
	
	if(server.send_ok(str)):
		print str + ' executed successfully'
	else:
		print 'error executing ' + str
	

def execute_commands(saveFile = None, autoConfig = True):

	try:
		 mcl = MCLServerTCP(host=MCL_HOST, port=MCL_PORT, debug=MCL_DEBUG)
	except Exception, e:
		print ("%s: unable to connect to MCL at %s port %s"
              % (MCL_NAME, MCL_HOST, MCL_PORT))
                return False
    
	try:
		print "oy"
		send_command(mcl, 'initialize', [])
		print "oy"

    # 3. Close the connection
	finally:
		mcl.close()
    
	return True


# ======================================================
#                                                   main
# ======================================================
def main():
    "Command line invocation"

    execute_commands()

# ======================================================
#                                  module initialization
# ======================================================
if __name__ == "__main__":
    main()

# ======================================================
# end             M C L S e r v e r . p y            end
# ======================================================
