
import numpy as np


def stepfunc ( a, b, th, x ):
	if ( x < th ):
	  return a
	else:
	  return b


def config( a21, a22 ):

	# fixed parameter values
	a1=20.0
	a2=20.0
	a3=20.0
	a4=20.0
	k1=4.0
	k2=4.0
	k3=4.0
	k4=4.0
	k21=5.0
	k14=5.0
	k22=10.0
	a14=20.0

	string = "<atlas>\n"

	# phase space bounds
	lbps = np.array ( [ 0.0, 0.0, 0.0, 0.0] )
	ubps = np.array ( [ 30.0, 200.0, 30.0, 30.0 ] )

	dimension = lbps . shape[0]

	string += "<dimension>"+str(dimension)+"</dimension>\n"

	string += "<phasespace>\n"
	string += "<bounds>\n"
	string += "<lower>"+str(lbps[0])+" "+str(lbps[1])+" "+str(lbps[2])+" "+str(lbps[3])+"</lower>\n"
	string += "<upper>"+str(ubps[0])+" "+str(ubps[1])+" "+str(ubps[2])+" "+str(ubps[3])+"</upper>\n"
	string += "</bounds>\n"
	string += "</phasespace>\n"

	#gamma = np.array ( [ -1.0, -1.0, -1.0 ] );

	string += "<gamma>\n"
	string += "<lower> -1.5 -1.5 -1.5 -1.5 </lower>\n"
	string += "<upper> -0.5 -0.5 -0.5 -0.5 </upper>\n"
	string += "</gamma>\n"

	# for the random sigma error bar
	delta=0.0

	# thresholds in each direction 
	theta1 = np.array ( [ min(k3,k21), max(k3,k21) ] )
	theta2 = np.array ( [ k1 ] )
	theta3 = np.array ( [ k4 ] )
	theta4 = np.array ( [ k22 ] )

	# prepend / append phase space bounds
	theta1=np.hstack((lbps[0],theta1))
	theta1=np.hstack((theta1,ubps[0]))

	theta2=np.hstack((lbps[1],theta2))
	theta2=np.hstack((theta2,ubps[1]))

	theta3=np.hstack((lbps[2],theta3))
	theta3=np.hstack((theta3,ubps[2]))

	theta4=np.hstack((lbps[3],theta4))
	theta4=np.hstack((theta4,ubps[3]))

	# number of thresholds in each direction
	# phase space bounds included 
	n1 = theta1 . shape[0]
	n2 = theta2 . shape[0]
	n3 = theta3 . shape[0]
	n4 = theta4 . shape[0]

	string += "<listboxes>\n"

	for m in range(len(theta4)-1):
	  for k in range(len(theta3)-1):
	    for j in range(len(theta2)-1):
	      for i in range(len(theta1)-1):

	        # print in xml format
	        string += "<box>\n"
	        string += "<bounds>\n"
	        string += "<lower>"+str(theta1[i])+" "+str(theta2[j])+" "+str(theta3[k])+" "+str(theta4[m])+" "+"</lower>\n"
	        string += "<upper>"+str(theta1[i+1])+" "+str(theta2[j+1])+" "+str(theta3[k+1])+" "+str(theta4[m+1])+" "+"</upper>\n"
	        string += "</bounds>\n"
	        #---
	        
	        #---
	        sigmax = stepfunc(a1,0.0,k1,0.5*(theta2[j]+theta2[j+1]))  
	        sigmay = stepfunc(0.0,a21,k21,0.5*(theta1[i]+theta1[i+1])) + stepfunc(0.0,a22,k22,0.5*(theta4[m]+theta4[m+1]))
	        sigmaz = stepfunc(0.0,a3,k3,0.5*(theta1[i]+theta1[i+1]))
	        sigmat = stepfunc(0.0,a4,k4,0.5*(theta3[k]+theta3[k+1]))

	        string += "<sigma>\n"
	        string += "<lower>"+str(sigmax-delta/2.0)+" "+str(sigmay-delta/2.0)+" "+str(sigmaz-delta/2.0)+" "+str(sigmat-delta/2.0)+" "+"</lower>\n"
	        string += "<upper>"+str(sigmax+delta/2.0)+" "+str(sigmay+delta/2.0)+" "+str(sigmaz+delta/2.0)+" "+str(sigmat+delta/2.0)+" "+"</upper>\n"
	        string += "</sigma>\n"
	        string += "</box>\n"

	string += "</listboxes>\n"
	string += "</atlas>\n"

	return string



#config()
