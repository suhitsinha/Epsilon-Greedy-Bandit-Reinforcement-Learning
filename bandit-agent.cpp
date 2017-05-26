#include <iostream>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <math.h>
#include<cstdlib>
#define MAXHOSTNAME 256

using namespace std;

void options(){

  cout << "Usage:\n";
  cout << "bandit-agent\n"; 
  cout << "\t[--numArms numArms]\n";
  cout << "\t[--randomSeed randomSeed]\n";
  cout << "\t[--horizon horizon]\n";
  cout << "\t[--hostname hostname]\n";
  cout << "\t[--port port]\n";
}


/*
  Read command line arguments, and set the ones that are passed (the others remain default.)
*/
bool setRunParameters(int argc, char *argv[], int &numArms, int &randomSeed, unsigned long int &horizon, string &hostname, int &port){

  int ctr = 1;
  while(ctr < argc){

    //cout << string(argv[ctr]) << "\n";

    if(string(argv[ctr]) == "--help"){
      return false;//This should print options and exit.
    }
    else if(string(argv[ctr]) == "--numArms"){
      if(ctr == (argc - 1)){
	return false;
      }
      numArms = atoi(string(argv[ctr + 1]).c_str());
      ctr++;
    }
    else if(string(argv[ctr]) == "--randomSeed"){
      if(ctr == (argc - 1)){
	return false;
      }
      randomSeed = atoi(string(argv[ctr + 1]).c_str());
      ctr++;
    }
    else if(string(argv[ctr]) == "--horizon"){
      if(ctr == (argc - 1)){
	return false;
      }
      horizon = atoi(string(argv[ctr + 1]).c_str());
      ctr++;
    }
    else if(string(argv[ctr]) == "--hostname"){
      if(ctr == (argc - 1)){
	return false;
      }
      hostname = string(argv[ctr + 1]);
      ctr++;
    }
    else if(string(argv[ctr]) == "--port"){
      if(ctr == (argc - 1)){
	return false;
      }
      port = atoi(string(argv[ctr + 1]).c_str());
      ctr++;
    }
    else{
      return false;
    }

    ctr++;
  }

  return true;
}



int main(int argc, char *argv[]){
  
  // Run Parameter defaults.
  int numArms = 5;
  int randomSeed = time(0);
  unsigned long int horizon = 1000;
  string hostname = "localhost";

  int port = 5000;

  //Set from command line, if any.
  if(!(setRunParameters(argc, argv, numArms, randomSeed, horizon, hostname, port))){
    //Error parsing command line.
    options();
    return 1;
  }

  struct sockaddr_in remoteSocketInfo;
  struct hostent *hPtr;
  int socketHandle;

  bzero(&remoteSocketInfo, sizeof(sockaddr_in));
  
  if((hPtr = gethostbyname((char*)(hostname.c_str()))) == NULL){
    cerr << "System DNS name resolution not configured properly." << "\n";
    cerr << "Error number: " << ECONNREFUSED << "\n";
    exit(EXIT_FAILURE);
  }

  if((socketHandle = socket(AF_INET, SOCK_STREAM, 0)) < 0){
    close(socketHandle);
    exit(EXIT_FAILURE);
  }

  memcpy((char *)&remoteSocketInfo.sin_addr, hPtr->h_addr, hPtr->h_length);
  remoteSocketInfo.sin_family = AF_INET;
  remoteSocketInfo.sin_port = htons((u_short)port);

  if(connect(socketHandle, (struct sockaddr *)&remoteSocketInfo, sizeof(sockaddr_in)) < 0){
    close(socketHandle);
    exit(EXIT_FAILURE);
  }


  char sendBuf[256];
  char recvBuf[256];
  int play=0;

/**************************	UCB tuned Choices	****************************/
	//Define the variables

  double *arm_reward_mean = new double[numArms]();
  double *arm_reward_mean_sq = new double[numArms]();
  double *variance = new double[numArms]();
  double *no_of_pulls = new double[numArms]();
  double *arm_maximize = new double[numArms]();
  //Initializing the arm_max value to negetive infinity
  double arm_max = -999999;
  int arm_max_index = 0;
  double min_t;
  int armToPull = 0;
  sprintf(sendBuf, "%d", armToPull);
  cout << "Sending action " << armToPull << ".\n";

  while(send(socketHandle, sendBuf, strlen(sendBuf)+1, MSG_NOSIGNAL) >= 0){

    float reward = 0;

    recv(socketHandle, recvBuf, 256, 0);
    sscanf(recvBuf, "%f", &reward);
    cout << "Received reward " << reward << ".\n";

	//calculating the mean reward for the last played arm upto the current term
	arm_reward_mean[armToPull] = (reward + (arm_reward_mean[armToPull] * no_of_pulls[armToPull]))/(no_of_pulls[armToPull]+1);

	//calculating the mean of reward square for the last played arm	
	arm_reward_mean_sq[armToPull] = ((reward*reward) + (arm_reward_mean_sq[armToPull] * no_of_pulls[armToPull]))/(no_of_pulls[armToPull]+1);
	//Increasing the number of pull of arm j by 1
	no_of_pulls[armToPull] = no_of_pulls[armToPull]+1;
	// Calculating the new variance of the pulled arm
	variance[armToPull] = (arm_reward_mean_sq[armToPull]) - (arm_reward_mean[armToPull]*arm_reward_mean[armToPull]);
	
    	// For first arm_numbers put the schedule as round robin
	if (play < numArms)
	{
		//Choosing the next arm in round robin schedule
		armToPull = (armToPull + 1) % numArms;
	}
	else
	{
	// After each arm got one iteration, start the optimal scheduling 
		arm_max=-999999;
		for(int i=0;i<numArms;i++)
		{
			// calculate the min between 1/4 and variance of each arm
			if (variance[i] + sqrt(2*(log(play))/no_of_pulls[i]) > .25)
			{
				//if variance value is more than .25 then take value as .25
				min_t = .25;
			}
			else
			{
				//else if variance value is less than .25 then take value as variance
				min_t = variance[i] + sqrt(2*log(play)/no_of_pulls[i]);
			}

			// calculate the maximize value for each arm
			arm_maximize[i] = arm_reward_mean[i] + sqrt((log(play)/no_of_pulls[i])*min_t);
			//find the arm with maximum term
			if(arm_max< arm_maximize[i])
			{
				arm_max = arm_maximize[i];
				arm_max_index = i;

			}			
		}
		// pick the arm with maximized output
		armToPull = arm_max_index;
	}

	//Increase the number of play by one
	play++;


	

/**********************************************************************/
    sprintf(sendBuf, "%d", armToPull);
    cout << "Sending action " << armToPull << ".\n";
  }
  
  close(socketHandle);
  cout << "Terminating.\n";

}
          
