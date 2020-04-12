import matplotlib.pyplot as plt 
file1 = open("statsdump.txt","r");
x = [0,256,512,1024];
y = [ [],[],[],[],[],[],[] ];
for i in range(0,4):
	line = file1.readline();
	bw = line.split(':');
	for j in range(0,7):
		y[j].append(float(bw[j]))

# plotting Average throughput
plt.figure()
plt.plot(x, y[0], color='green', linestyle='dashed', linewidth = 1, 
         marker='o', markerfacecolor='blue', markersize=6) 
plt.xlabel('RTS Threshold')
plt.ylabel('TCP throughput in Mbps');
plt.title('Plot of TCP throughput vs RTS threshold')


# plotting rts_bandwith,cts_bandwidth and ack_bandwidth
plt.figure()
plt.plot(x, y[1], color='green', linestyle='dashed', linewidth = 1, 
         marker='o', markerfacecolor='blue', markersize=6,label="RTS")
plt.plot(x, y[2], color='red', linestyle='dashed', linewidth = 1, 
         marker='o', markerfacecolor='blue', markersize=6,label="CTS")
plt.plot(x, y[3], color='yellow', linestyle='dashed', linewidth = 1, 
         marker='o', markerfacecolor='blue', markersize=6,label="ACK")
plt.xlabel('RTS Threshold')
plt.ylabel('Bandwidth')
plt.title('Bandwidth spent in transmission of RTS,CTS & ACK vs RTS Threshold')
plt.legend()

# plotting Average bandwidth spent in transmitting 
# TCP segments and TCP ACKS
plt.figure()
plt.plot(x, y[4], color='green', linestyle='dashed', linewidth = 1, 
         marker='o', markerfacecolor='blue', markersize=6,label="TCP ACK")
plt.plot(x, y[5], color='red', linestyle='dashed', linewidth = 1, 
         marker='o', markerfacecolor='blue', markersize=6,label="TCP SEGMENT")
plt.xlabel('RTS Threshold')
plt.ylabel('Bandwidth')
plt.title('Bandwidth spent in transmission of TCP ACK and TCP segments')
plt.legend()

# plot number of collisions
plt.figure()
plt.plot(x, y[6], color='green', linestyle='dashed', linewidth = 1, 
         marker='o', markerfacecolor='blue', markersize=6) 
plt.xlabel('RTS Threshold')
plt.ylabel('Collisions');
plt.title('Collisions vs RTS threshold')
#Show all plots
plt.show() 