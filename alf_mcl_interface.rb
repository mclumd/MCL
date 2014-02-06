require "net/telnet.rb"
require "socket"

localhost = Net::Telnet::new("Host" => "localhost",
                               "Timeout" => 100,
                               "Prompt" => /\n/,
                               "Port" => 5150)


# sets up server to communicate with Alfred
serv = TCPServer.new("#{Socket.gethostname}", 5155)
file = File.new("/home/emily/Desktop/ALFRED/beth_alfred/etc/mclhost.#{ENV['USER']}", "w")
file.syswrite("port 5155\n")
file.syswrite("host #{Socket.gethostname}\n")
file.syswrite("process #{Process.pid}\n")
file.close

responses = []
initialized = false
mclPauseLength = 100.0
mcl_violations = 1

puts "Checking for connection from Alfred"
# accepts connection from Alfred
sock = serv.accept

while 1
	
command = []
buf = []
response = ""
	puts "Before receive"
	# receives command from Alfred
	buf = sock.recv(4096)
	command = buf

	while buf.length > 4095
		buf = sock.recv(4096)
		command = command + buf
	end
	puts "commands: #{command}"	
	commands = command.split

	commands.each { |com|
	puts "com: #{com}"
	
	if com.start_with? "monitor"
		#Get the pause length to add to response to alfred
		pauseLength = "0"#com[/\=(.*?)}/,1]

		localhost.cmd(com) { |c| response = c 
		if response =~ /ok\(\[response\(.*code=([^,]*),.*\]\)/
      				alfmessage = "term(af(#{$1}(#{pauseLength})))."
			sock.puts(alfmessage)
			puts "Alfred Message: #{alfmessage} \n"
		end
		responses << response
		puts "Response: #{response} \n"
		}

	else	
		
		if com.start_with? "failed"
			referent = responses.last.partition("ref=")[2].slice(0,10)
			com = "suggestionFailed(pause_time," + referent + ")"
			localhost.cmd(com) { |c| response = c }
		elsif com.start_with? "ignored"
			referent = responses.last.partition("ref=")[2].slice(0,10)
			com = "suggestionIgnored(pause_time," + referent + ")"
			localhost.cmd(com) { |c| response = c }
		elsif com.start_with? "used"
			referent = responses.last.partition("ref=")[2].slice(0,10)
			com = "suggestionImplemented(pause_time," + referent + ")"
			localhost.cmd(com) { |c| response = c }
		elsif com.start_with? "initialize"
			if initialized == false
				#Initialize Pause_Time
				com = %q[initialize(pause_time)]
				localhost.cmd(com) { |c| response = c}
				com = %q[configure(pause_time,alftest,alftest)]
				localhost.cmd(com) { |c| response = c}
				com = %q[declareObservableSelf(pause_time,pauselen,1)]
				localhost.cmd(com) { |c| response = c}
				com = %q[declareObservableSelf(pause_time,viol,1)]
				localhost.cmd(com) { |c| response = c}
				com = %q[setObsPropSelf(pause_time,pauselen,prop_sclass,sc_actor)]
				localhost.cmd(com) { |c| response = c}
				com = %q[setObsPropSelf(pause_time,viol,prop_sclass,sc_counter)]
				localhost.cmd(com) { |c| response = c}
				com = %q[declareEG(pause_time,0)]
				localhost.cmd(com) { |c| response = c}
				com = %q[declareSelfExp(pause_time,0,ec_stayunder,pauselen,]+ mclPauseLength.to_s + ')'
				localhost.cmd(com) { |c| response = c}
				com = %q[declareSelfExp(pause_time,0,ec_stayunder,viol,]+ mcl_violations.to_s + ')'
				localhost.cmd(com) { |c| response = c}

				initialized = true
			end
		elsif com.start_with? "complete"
			str = com
			newpausestring = str[/{(.+)}/,1]
			newpause = newpausestring.to_f
			com = 'EGabort(pause_time,0)'
			localhost.cmd(com) { |c| response = c}

			#Reseting Expectation with new pause time			
			com = %q[declareObservableSelf(pause_time,pauselen,1)]
			localhost.cmd(com) { |c| response = c}
			com = %q[setObsPropSelf(pause_time,pauselen,prop_sclass,sc_resource)]
			localhost.cmd(com) { |c| response = c}
			com = %q[declareEG(pause_time,0)]
			localhost.cmd(com) { |c| response = c}
			com = %q[declareSelfExp(pause_time,0,ec_stayunder,pauselen,]+ newpause.to_s + ')'
			localhost.cmd(com) { |c| response = c}
		else
			localhost.cmd(com) { |c| response = c }
		end
	end
	}
end

sock.close
localhost.close
