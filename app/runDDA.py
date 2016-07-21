import subprocess
import time 
import csv

file_size = 104.9
trial_num = 5000
constant_factor = 0.6
output_file_dda = 'Home_Router_DDA'+str(constant_factor)+'.csv'
output_file_dda = 'Contant_Factor_Checker_DDA'+str(constant_factor)+'.csv'
output_file_udt = 'Home_Router_TCP'+str(constant_factor)+'.csv'

def runDDA(val):
	cmd_iit = './recvfile 10.192.58.30 9000 three.txt rcv_three.txt'
	cmd_linksys_DDA = './recvfile 10.100.1.102 9000 three.txt rcv_three.txt'
	cmd_linksys_UDT = './recvfile 10.100.1.102 12345 three.txt rcv_three.txt'
	cmd_tplink = './recvfile 192.168.1.103 9000 three.txt rcv_three.txt'
	cmd_tcp = 'java Client'

	# export LD_LIBRARY_PATH="../../../../VII_Sem/BTP/Original_UDT/udt-git/udt4/src"
	dda_export_path  = "export LD_LIBRARY_PATH='/home/veerendra/BTP_IIT_DELHI/UDA/src'"
	udt_export_path  = "export LD_LIBRARY_PATH='/home/veerendra/UDA/src'"
	dda_run = "/home/veerendra/BTP_IIT_DELHI/DDA/udt4/app/"+cmd_linksys_DDA;
	udt_run = "/home/veerendra/VII_Sem/BTP/Original_UDT/udt-git/udt4/app/"+cmd_linksys_UDT
	start = time.time()
	if(val % 2 == 0):
		subprocess.call(dda_export_path,shell=True)
		subprocess.call(dda_run,shell=True)
	else:
		subprocess.call(cmd_tcp,shell=True)
		# subprocess.call(udt_run,shell=True)
		# subprocess.call(udt_export_path,shell=True)
		# subprocess.call(udt_run,shell=True)
	# subprocess.call(cmd_iit,shell=True)
	# subprocess.call(cmd_tplink,shell=True)
	end = time.time()
	return end - start


sum_time_taken = 0
sum_throughput_bytes = 0

count = 0	
while count <= trial_num :
	print "Running trail #"+str(count)+" of "+str(trial_num)
	time_taken = runDDA(count)
	# subprocess.call('rm rcv_three.txt',shell=True)
	print time_taken
	throughput_bytes = file_size/time_taken
	sum_time_taken += time_taken
	sum_throughput_bytes += throughput_bytes
	row = [count, time_taken, throughput_bytes, throughput_bytes*8]
	print row
	if(count % 2 == 0):
		fd = open(output_file_dda,'a')
	else:
		fd = open(output_file_udt,'a')
	writer = csv.writer(fd)
	writer.writerow(row)
	fd.close()
	count += 1
	time.sleep(5)

avg_time_taken = sum_time_taken/trial_num
avg_throughput_bytes = sum_throughput_bytes/trial_num
avg_throughput_bits = avg_throughput_bytes*8
avg_title = ['trial_num', 'avg_time_taken', 'avg_throughput_bytes' , 'avg_throughput_bits'] 
row = [trial_num, avg_time_taken, avg_throughput_bytes, avg_throughput_bits]

# fd = open(output_file,'a')
# writer = csv.writer(fd)
# writer.writerow(avg_title)
# writer.writerow(row)
# fd.close()

