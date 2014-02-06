require "net/telnet.rb"

localhost = Net::Telnet::new("Host" => "localhost",
                               "Timeout" => 100,
                               "Prompt" => /\n/,
                               "Port" => 5150)

localhost.cmd("initialize(name)") { |c| print c }

puts "ontology name?"
oName = gets.chomp
puts "config path?"
cPath = gets.chomp

localhost.cmd("configure(name," + oName + "," + cPath + ")") { |c| print c }

class ExpectationGroup
	
	def initialize(key)
		@key = key
		@expectations = []
		@time = 0
	end
	
	def set_parent key
		@parent = key
	end
	
	def set_referent key
		@referent = key
	end
	
	def add_expectation exp
		@expectations << exp
	end
	
	def set_time time
		@time = time
	end
	
	def declare? time
		if @time == nil or @time == time
			return true
		end
		return false
	end
	
	def to_commands
		commands = []
		s = "declareEG(name," + @key.to_s
		if @referent != nil and @parent != nil
			s += "," + @parent.to_s + "," + @referent.to_s
		end
		s += ")"
		commands << s
		@expectations.each do|exp|
			commands << exp.to_command(@key)
		end
		return commands
	end
	
	def to_file
		s = @key.to_s + ";" + @time.to_s + ";"
		@expectations.each do|exp|
			s += exp.to_file + ","
		end
		s = s.chop
		if @referent != nil and @parent != nil
			s += ";" + @parent.to_s + ";" + @referent.to_s
		end
		return s
	end
	
	def from_file line
		vals = line.split(";")
		@key = vals[0].to_i
		@time = vals[1].to_i
		if vals[2] != nil
			exps = vals[2].split(",")
			exps.each do|exp|
				e = Expectation.new(0, "0")
				e.from_file exp
				@expectations << e
			end
		end
		if vals[3] != nil
			@parent = vals[3].to_i
		end
		if vals[4] != nil
			@referent = vals[4].to_i
		end
	end
	
	def to_s
		s = "group " + @key.to_s
		@expectations.each do|exp|
			s += exp.to_s + "\n"
		end
		return s
	end
end



class Expectation
	
	
	
	def initialize(type, obj)
		@expectationArr = ["stayunder", "stayover", "maintainvalue", "withinnormal",
		"realtime", "ticktime", "go_up", "go_down", "net_zero", "any_change",
		"net_range", "take_value", "dont_care", "be_legal"]
		@type = type
		@obj = obj
	end
	
	def set_value val
		@value = val
	end
	
	def set_sensor sensor
		@sensor = sensor
	end
	
	def to_s
		s = "Expectation: "
		if @sensor != nil
			s += @sensor + " "
		end
		s += @expectationArr[@type]
		if @value != nil
			s += " " + @value.to_s
		end
		return s
	end
	
	def to_command group
		if @obj == "self"
			s = "declareSelfExp(name," + group.to_s
			
			s += ",ec_" + @expectationArr[@type]
			if @sensor != nil
				s += "," + @sensor
			end
			if @value != nil
				s += "," + @value.to_s
			end
			s += ")"
			return s
		end
	end
	
	def to_file
		s = @type.to_s + "-" + @obj
		if @sensor != nil
			s += "-" + @sensor
		end
		if @value != nil
			s += "-" + @value.to_s
		end
		return s
	end
	
	#this will mess up if there is a value but no sensor.
	def from_file line
		vals = line.split("-")
		@type = vals[0].to_i
		@obj = vals[1]
		if vals[2] != nil
			@sensor = vals[2]
		end
		if vals[3] != nil
			@value = vals[3].to_f
		end
	end
				
end

class Sensor
	
	attr_accessor :name, :value
	
	def initialize(name, type, value)
		@name = name
		@type = type
		#value is a string. Convert to int for operations.
		@value = value
	end
	
	def set_range(min, max)
		@min = min
		@max = max
	end
	
	def to_s
		s = @name + ":  type = " + @type + ";  value = " + @value
		if @min != nil
			s += ";  min val = " + @min.to_s
		end
		if @max != nil
			s += ";  max val = " + @max.to_s
		end
		return s
	end
	
	def to_commands
		commands = []
		s = "declareObservableSelf(name," + @name + "," + @value + ")"
		commands << s
		s = "setObsPropSelf(name," + @name + "," + "prop_sclass" + "," + 
			"sc_" + @type.downcase + ")" 
		commands << s
		return commands
	end
	
	def copy_from_file line
		parts = line.split(";")
		@name = parts[0]
		@type = parts[1]
		@value = parts[2]
		index = 3
		while parts.length > index
			part = parts[index]
			if part.start_with? "min="
				@min = part.delete("min=").to_i
			elsif part.start_with? "max="
				@max = part.delete("max=").to_i
			end
			index += 1
		end
	end
	
	def to_file
		s = @name + ";" + @type + ";" + @value
		if @min != nil
			s += ";min=" + @min.to_s
		end
		if @max != nil
			s += ";max=" + @max.to_s
		end
		return s
	end
end

egs = []
egIndex = 0
sensors = {}
time = 0
exec = []
responses = []

def get_all_vals sensors
	s = "{"
	sensors.each do|sensor|
		s += sensor.name + "=" + sensor.value + ","
	end
	s = s.chop + "}"
	return s
end

def read_file(file, sensors, egs, exec)
	mode = "sensor"
	egIndex = 0
	
	file.each do|line|
		
		if line == ""
			next
		end
		
		if line.start_with? "--exp--"
			mode = "exp"
			next
		end
		
		if line.start_with? "--exec--"
			mode = "exec"
			next
		end
		
		if mode == "exec"
			exec << line
			next
		end
			
		if mode == "exp"
			if line.split(";").length < 2
				next
			end
			eg = ExpectationGroup.new(0)
			eg.from_file line
			egs << eg
			egIndex += 1
			next
		end
		
		if mode == "sensor"
			if line.split(";").length < 3
				next
			end
			sensors[line.split(";")[0]] = Sensor.new("fill", "fill", "fill")
			sensors[line.split(";")[0]].copy_from_file line
			next
		end
	end
	return egIndex
end

while 1

	puts "sensor, quit, save, load, expectations, monitor"

	command = gets.chomp
	
	if command.start_with? "q"
		break
	end
	
	if command.start_with? "m"
		while 1
		
			puts "monitor, sensor change, add sensor, new exp group, exp change, back, respond"
			command = gets.chomp
			
			if command.start_with? "m"
				com = "monitor(name," + get_all_vals(sensors.values) + ")"
				response = nil
				localhost.cmd(com) { |c| response = c }
				puts response
				if response.length > 20
					responses << response
				end
				time += 1
				#put in egs activating if right time
				exec << "m"
				next
			end
			
			if command.start_with? "r"
				referent = responses.last.partition("ref=")[2].slice(0,10)
				puts "failed, ignored, or used?"
				
				response = gets.chomp
				if response.start_with? "f"
					com = "suggestionFailed(name," + referent + ")"
					localhost.cmd(com) { |c| response = c }
					exec << "responsefail" + ";" + referent
					next
				end
				if response.start_with? "i"
					com = "suggestionIgnored(name," + referent + ")"
					localhost.cmd(com) { |c| response = c }
					exec << "responseignore" + ";" + referent
					next
				end
				if response.start_with? "u"
					com = "suggestionImplemented(name," + referent + ")"
					localhost.cmd(com) { |c| response = c }
					exec << "responseimplemented" + ";" + referent
					next
				end
			end
			
			if command.start_with? "s"
				sensors.values.each do|sensor|
					puts sensor.to_s
				end
				puts "change value of which sensor?"
				name = gets.chomp
				if !(sensors.has_key? name)
					puts "not a sensor name"
					next
				end
				puts "change to what value?"
				value = gets.chomp
				sensors[name].value = value
				exec << "sensorval;" + name + ";" + value
				next
			end
			
			if command.start_with? "b"
				break
			end
		end
	end
			
	
	if command.start_with? "e"
		while 1
			#need to implement abort and complete
			puts "list, group(new), complete group, abort group, expectation (add), back"
			
			command = gets.chomp
			
			if command.start_with? "l"
				egs.each do|eg|
					puts eg.to_s
				end
				next
			end
			
			if command.start_with? "e"

				puts "type? 0:stayunder, 1:stayover, 2:maintainvalue, 3:withinnormal,4:realtime, 5:ticktime, 6:go_up, 7:go_down, 8:net_zero, 9:any_change,10:net_range, 11:take_value, 12:dont_care, 13:be_legal"
				type = gets.chomp.to_i
				if type < 0 or type > 13
					puts "invalid type"
					next
				end
				#only self expectations currently supported
				exp = Expectation.new(type, "self")
				
				#value, sensor, group
				puts "which group should this expectation be assigned to?"
				grp = gets.chomp.to_i
				if egIndex <= grp
					puts "invalid key"
					next
				end
				egs[grp].add_expectation exp
				
				puts "what is the name of the associated sensor, if any?"
				sensor = gets.chomp
				if sensor != ""
					exp.set_sensor sensor
				end
				puts "what is the associated value, if any?"
				value = gets.chomp
				if value != ""
					exp.set_value value.to_f
				end
				cmd = egs[grp].to_commands.last
				localhost.cmd(cmd) {|c| print c}
				
				next
			end
			
			if command.start_with? "g"
				group = ExpectationGroup.new(egIndex)
				
				puts "parent? Leave blank for none."
				parent = gets.chomp
				if parent != ""
					group.set_parent parent
					puts "referent?"
					ref = gets.chomp
					group.set_referent ref
				end
				
				puts "initialize when? leave blank for immediately."
				t = gets.chomp.to_i
				group.set_time t
				
				if group.declare? time
					group.to_commands.each do|command|
						localhost.cmd(command) { |c| print c }
					end
				end
				
				egs << group
				egIndex += 1
				next
			end
			
			if command.start_with? "b"
				break
			end
			
		end
		next
	end
	
	if command.start_with? "sa"
		puts "filename?"
		name = gets.chomp
		f = File.new(name, "w")
		sensors.values.each do|sensor|
			f.write(sensor.to_file + "\n")
		end
		f.write("--exp--\n")
		egs.each do|eg|
			f.write(eg.to_file + "\n")
		end
		f.write("--exec--\n")
		exec.each do|line|
			f.write(line + "\n")
		end
		f.close
		next
	end
	
	if command.start_with? "l"
		puts "filename?"
		name = gets.chomp
		f = File.new(name, "r")
		egIndex = read_file(f, sensors, egs, exec)
		
		sensors.values.each do|sensor|
			sensor.to_commands.each do|command|
				localhost.cmd(command) { |c| print c }
			end
		end
		
		egs.each do|eg|
			eg.to_commands.each do|command|
				localhost.cmd(command) { |c| print c }
			end
		end
		
		puts "execute now?"
		if gets.chomp.start_with? "y"
			index = 0
			while index < exec.length
				gets
				if exec[index].start_with? "m"
					com = "monitor(name," + get_all_vals(sensors.values) + ")"
					localhost.cmd(com) { |c| print c }
					time += 1
				elsif exec[index].start_with? "sensorval"
					vals = exec[index].split(";")
					sensors[vals[1]].value = vals[2]
					puts vals[1] + " set to " + vals[2]
				elsif exec[index].start_with? "responsefail"
					referent = exec[index].split(";")[1]
					com = "suggestionFailed(name," + referent + ")"
					localhost.cmd(com) { |c| response = c }
				elsif exec[index].start_with? "responseignore"
					referent = exec[index].split(";")[1]
					com = "suggestionIgnored(name," + referent + ")"
					localhost.cmd(com) { |c| response = c }
				elsif exec[index].start_with? "responseimplemented"
					referent = exec[index].split(";")[1]
					com = "suggestionImplemented(name," + referent + ")"
					localhost.cmd(com) { |c| response = c }
				end
				index +=1
			end
		end	
		
		next
	end
	
	if command.start_with? "se"
		
		while 1
		
		puts "editing sensors. Commands: 'add', 'set range', 'list', 'back', values(add)"
		command = gets.chomp
		
		if command.start_with? "b"
			break
		end
		
		#at some point, add to sensor class/save/load/blah
		if command.start_with? "v"
			puts "sensor name?"
			name = gets.chomp
			if sensors[name] == nil
				puts "sensor DNE"
			end
			
			while 1
				puts "type back to escape, anything else to add a value"
				value = gets.chomp
				if value == "back"
					break
				end
				localhost.cmd("addObsLegalValSelf(name," + name + "," + value + ")")
			
			end
			next
		end
		
		if command.start_with? "a"
			prop = "PROP_SCLASS"
			puts "sensor name?"
			name = gets.chomp
			puts "type? (spatial, state, control, temporal, resource, reward, ambient, objectprop, message, counter, unspec"
			type = gets.chomp	
			typeName = "SC_" + type.upcase
			puts "default value?"
			default = gets.chomp
		
			sensors[name] = Sensor.new(name, type, default)
		
			sensors[name].to_commands.each do|command|
				localhost.cmd(command) { |c| print c }
			end
			next
		end
		
		if command.start_with? "s"
			puts "which sensor?"
			name = gets.chomp
			puts "min value? (integer)"
			min = gets.chomp.to_i
			puts "max value? (integer)"
			max = gets.chomp.to_i
			
			if sensors[name] == nil
				puts "no sensor name " + name + " exists."
				next
			end
			sensors[name].set_range(min, max)
			
			localhost.cmd("setObsLegalRangeSelf(name," + name + "," + min.to_s + "," + max.to_s + ")")
			next
		end
		
		if command.start_with? "l"
			sensors.each do|sensor|
				puts sensor.to_s
			end
			next
		end
		
		end
	end
end
	
 
 
 
 
 
 localhost.close
