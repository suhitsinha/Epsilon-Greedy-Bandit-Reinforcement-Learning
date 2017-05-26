Algorithm Used:
***************

1. Play each machine once in round robin machanism, i.e. algorithm will run as round robin for the first arm_number iteration.
2. Calculate the following for each arm
	a. Average reward of every arm j.
	b. number of times each of the machines has been played nj.
	c. Total number of plays performed so far n.
	d. Variance of each arm after n plays variance(j).
3. choose the arm that maximizes the following:
	mean(j) + sqrt(ln(n)/nj * min(.25, variance(j))).
4. .25 is the upper bound on the variance of each arm.
5. Get the rewards.
6. Go to step 2 until number of plays reached as stated by the user.


How to run:
***********
It can be run in the previous set up itself by setting the parameters in startexperiment.sh Also the client and server can be run separately by putting appropriate parameters in the sripts startserver.sh and startclient.sh

Parameters for startserver.sh:
numArms
horizon
port
banditFile
randomSeed
OUTPUTFILE

Parameters for startclient.sh:
numArms
horizon
hostname
port
randomSeed

###############
# References: #
###############

1. Finite-time Analysis of the Multiarmed Bandit Problem
	PETER AUER et. al.
