#include <stdlib.h>
#include "gpiolib_addr.h"
#include "gpiolib_reg.h"
#include <stdint.h>
#include <stdio.h>		//for the printf() function
#include <fcntl.h>
#include <linux/watchdog.h> 	//needed for the watchdog specific constants
#include <unistd.h> 		//needed for sleep
#include <sys/ioctl.h> 		//needed for the ioctl function
#include <stdlib.h> 		//for atoi
#include <time.h> 		//for time_t and the time() function
#include <sys/time.h>           //for gettimeofday()

//Below is a macro that had been defined to output appropriate logging messages
//You can think of it as being similar to a function
//file        - will be the file pointer to the log file
//time        - will be the current time at which the message is being printed
//programName - will be the name of the program, in this case it will be Lab4Sample
//str         - will be a string that contains the message that will be printed to the file.
#define PRINT_MSG(file, time, programName, str) \
	do{ \
			fprintf(file, "%s : %s : %s", time, programName, str); \
			fflush(file); \
	}while(0)

//In the macro, we use the fflush() function to make sure that the messages get printed
//to the file. If we did not use fflush() then it is possible some messages would not
//get printed if the program ended unexpectedly.

//Most of the following functions are the same as the ones from the Lab 3
//sample code.

//This function will initialize the GPIO pins and handle any error checking
//for the initialization
GPIO_Handle initializeGPIO()
{
	//This is the same initialization that was done in Lab 2
	GPIO_Handle gpio;
	gpio = gpiolib_init_gpio();
	if(gpio == NULL)
	{
		perror("Could not initialize GPIO");
	}
	return gpio;
}


//This is a function used to read from the config file. It is not implemented very
//well, so when you create your own you should try to create a more effective version
void readConfig(FILE* configFile, int* timeout, char* logFileName, char* statFileName)
{
	//Loop counter
	int i = 0;
	
	//A char array to act as a buffer for the file
	char buffer[255];

	//The value of the timeout variable is set to zero at the start
	*timeout = 0;

	//The value of the numBlinks variable is set to zero at the start
	//*numBlinks = 0;

	//This is a variable used to track which input we are currently looking
	//for (timeout, logFileName or numBlinks)
	int input = 0;

	//This will 
	//fgets(buffer, 255, configFile);
	//This will check that the file can still be read from and if it can,
	//then the loop will check to see if the line may have any useful 
	//information.
	while(fgets(buffer, 255, configFile) != NULL)
	{
		i = 0;
		//If the starting character of the string is a '#', 
		//then we can ignore that line
		if(buffer[i] != '#')
		{
			while(buffer[i] != 0)
			{
				//This if will check the value of timeout
				if(buffer[i] == '=' && input == 0)
				{
					//The loop runs while the character is not null
					while(buffer[i] != 0)
					{
						//If the character is a number from 0 to 9
						if(buffer[i] >= '0' && buffer[i] <= '9')
						{
							//Move the previous digits up one position and add the
							//new digit
							*timeout = (*timeout *10) + (buffer[i] - '0');
						}
						i++;
					}
					input++;
				}
				else if(buffer[i] == '=' && input == 1) //This will find the name of the log file
				{
					int j = 0;
					//Loop runs while the character is not a newline or null
					while(buffer[i] != 0  && buffer[i] != '\n')
					{
						//If the characters after the equal sign are not spaces or
						//equal signs, then it will add that character to the string
						if(buffer[i] != ' ' && buffer[i] != '=')
						{
							logFileName[j] = buffer[i];
							j++;
						}
						i++;
					}
					//Add a null terminator at the end
					logFileName[j] = 0;
					input++;
				}
				else if(buffer[i] == '=' && input == 2) //This will find the name of the stat file
				{
					int j = 0;
					//Loop runs while the character is not a newline or null
					while(buffer[i] != 0  && buffer[i] != '\n')
					{
						//If the characters after the equal sign are not spaces or
						//equal signs, then it will add that character to the string
						if(buffer[i] != ' ' && buffer[i] != '=')
						{
							statFileName[j] = buffer[i];
							j++;
						}
						i++;
					}
					//Add a null terminator at the end
					statFileName[j] = 0;
					input++;
				}
				else
				{
					i++;
				}
			}
		}
	}
}

//This function will get the current time using the gettimeofday function
void getTime(char* buffer)
{
	//Create a timeval struct named tv
  	struct timeval tv;

	//Create a time_t variable named curtime
  	time_t curtime;


	//Get the current time and store it in the tv struct
  	gettimeofday(&tv, NULL); 

	//Set curtime to be equal to the number of seconds in tv
  	curtime=tv.tv_sec;

	//This will set buffer to be equal to a string that in
	//equivalent to the current date, in a month, day, year and
	//the current time in 24 hour notation.
  	strftime(buffer,30,"%m-%d-%Y  %T.",localtime(&curtime));

} 


//HARDWARE DEPENDENT CODE BELOW
#ifndef MARMOSET_TESTING

/* You may want to create helper functions for the Hardware Dependent functions*/

//This function should initialize the GPIO pins


//This function should accept the diode number (1 or 2) and output
//a 0 if the laser beam is not reaching the diode, a 1 if the laser
//beam is reaching the diode or -1 if an error occurs.
#define LASER1_PIN_NUM 4 
#define LASER2_PIN_NUM 8 
int laserDiodeStatus(GPIO_Handle gpio, int diodeNumber) {
	if(gpio == NULL) {
		return -1;
	}

	if(diodeNumber == 1) {
		uint32_t level_reg = gpiolib_read_reg(gpio, GPLEV(0));

		if(level_reg & (1 << LASER1_PIN_NUM)) {
			return 1;
		} else {
			return 0;
		}
	} else if(diodeNumber == 2) {
		uint32_t level_reg = gpiolib_read_reg(gpio, GPLEV(0));

		if(level_reg & (1 << LASER2_PIN_NUM)) {
			return 1;
		} else {
			return 0;
		}
	} else {
		return -1;
	}
}  

#endif


//END OF HARDWARE DEPENDENT CODE

//This function will output the number of times each laser was broken
//and it will output how many objects have moved into and out of the room.

//laser1Count will be how many times laser 1 is broken (the left laser).
//laser2Count will be how many times laser 2 is broken (the right laser).
//numberIn will be the number  of objects that moved into the room.
//numberOut will be the number of objects that moved out of the room.
void outputMessage(int laser1Count, int laser2Count, int numberIn, int numberOut) {
	printf("Laser 1 was broken %d times \n", laser1Count);
	printf("Laser 2 was broken %d times \n", laser2Count);
	printf("%d objects entered the room \n", numberIn);
	printf("%d objects exitted the room \n", numberOut);
}

//This function accepts an errorCode. You can define what the corresponding error code
//will be for each type of error that may occur.
void errorMessage(int errorCode) {
	fprintf(stderr, "An error occured; the error code was %d \n", errorCode);
}


#ifndef MARMOSET_TESTING

int main(const int argc, const char* const argv[]) {

	//Create a string that contains the program name
	const char* argName = argv[0];

	//These variables will be used to count how long the name of the program is
	int i = 0;
	int namelength = 0;

	while(argName[i] != 0)
	{
		namelength++;
		i++;
	} 

	char programName[namelength];

	i = 0;

	//Copy the name of the program without the ./ at the start
	//of argv[0]
	while(argName[i + 2] != 0)
	{
		programName[i] = argName[i + 2];
		i++;
	} 	

	//Create a file pointer named configFile
	FILE* configFile;
	//Set configFile to point to the Lab4Sample.cfg file. It is
	//set to read the file.
	configFile = fopen("/home/pi/laser.cfg", "r");
	
	char Time[30];
	FILE* logFile;
	FILE* statFile;
	int timeout = 10;
	
	//Output a warning message if the file cannot be openned
	if(!configFile)
	{
		perror("The config file could not be opened, opening default");
		logFile = fopen("defaultlog.log", "a");
		statFile = fopen("defaultstat.txt", "a");
		timeout = 10;
	}
	
	else{
		//Declare the variables that will be passed to the readConfig function
		char logFileName[50];
		char statFileName[50];
		//int numBlinks;

		//Call the readConfig function to read from the config file
		readConfig(configFile, &timeout, logFileName, statFileName);

		//Close the configFile now that we have finished reading from it
		fclose(configFile);

		//Set it to point to the file from the config file and make it append to
		//the file when it writes to it.
		printf("Log Name: %s\n", logFileName);
		
		logFile = fopen(logFileName, "a");
		
		//Check that the file opens properly.
		if(!logFile)
		{
			logFile = fopen("defaultlog.log", "a");
			perror("The log file could not be opened");
			if (!logFile){
				perror("The default log file could not be opened, exiting");
				return -1;	
			}
		}
		printf("Stat Name: %s\n", statFileName);
		statFile = fopen(statFileName, "a");
		
		if(!statFile)
		{
			statFile = fopen("defaultstat.txt", "a");		
			perror("The stat file could not be opened");
			if (!statFile){
				getTime(Time);
				PRINT_MSG(logFile, Time, programName, "The deafult file could not be opened\n\n");
			//return -1;
			}
		}
	}
	
	
	//Create a char array that will be used to hold the time values
	getTime(Time);

	//Initialize the GPIO pins
	GPIO_Handle gpio = initializeGPIO();
	//Get the current time
	getTime(Time);
	//Log that the GPIO pins have been initialized
	PRINT_MSG(logFile, Time, programName, "The GPIO pins have been initialized\n\n");

	
	//This variable will be used to access the /dev/watchdog file, similar to how
	//the GPIO_Handle works
	int watchdog;

	//We use the open function here to open the /dev/watchdog file. If it does
	//not open, then we output an error message. We do not use fopen() because we
	//do not want to create a file if it doesn't exist
	if ((watchdog = open("/dev/watchdog", O_RDWR | O_NOCTTY)) < 0) {
		printf("Error: Couldn't open watchdog device! %d\n", watchdog);
		return -1;
	} 
	//Get the current time
	getTime(Time);
	//Log that the watchdog file has been opened
	PRINT_MSG(logFile, Time, programName, "The Watchdog file has been opened\n\n");

	//This line uses the ioctl function to set the time limit of the watchdog
	//timer to 15 seconds. The time limit can not be set higher that 15 seconds
	//so please make a note of that when creating your own programs.
	//If we try to set it to any value greater than 15, then it will reject that
	//value and continue to use the previously set time limit
	ioctl(watchdog, WDIOC_SETTIMEOUT, &timeout);
	
	//Get the current time
	getTime(Time);
	//Log that the Watchdog time limit has been set
	PRINT_MSG(logFile, Time, programName, "The Watchdog time limit has been set\n\n");

	//The value of timeout will be changed to whatever the current time limit of the
	//watchdog timer is
	ioctl(watchdog, WDIOC_GETTIMEOUT, &timeout);

	//This print statement will confirm to us if the time limit has been properly
	//changed. The \n will create a newline character similar to what endl does.
	printf("The watchdog timeout is %d seconds.\n\n", timeout);

	//A counter used to keep track of how many blinks have occurred.
	int counter = 0;

	
	enum State {START, PRINT, LEFT_BROKEN_IN, RIGHT_BROKEN_IN, BOTH_ON_IN, BOTH_BROKEN_IN, LEFT_BROKEN_OUT, RIGHT_BROKEN_OUT, BOTH_ON_OUT, BOTH_BROKEN_OUT};	
	enum State s = START;
	
	int numberIn = 0;
	int numberOut = 0;
	int laser1Count = 0;
	int laser2Count=0;
	

#define PRINT_STATMSG(file, time, programName, str, int) \
	do{ \
			fprintf(file, "%s : %s : %s : %d\n\n", time, programName, str, int); \
			fflush(file); \
	}while(0)
	
	
	//Check if laser 1 is pointed at diode
	if (!laserDiodeStatus(gpio, 1)){
		printf("Error code is -1\n");
		return -1;
	}
	
	//Check if laser 2 is pointed at diode
	else if (!laserDiodeStatus(gpio, 2)){
		printf("Error code is -1\n");
		//End program
		return -1;
	}
	
	while (1){
		ioctl(watchdog, WDIOC_KEEPALIVE, 0);
		
		switch(s) {
		case START: 
			
				//Check if left laser is broken first, therefore object going in
				if (!laserDiodeStatus(gpio, 2)){
					//Add 1 to left laser count since broken
					laser2Count += 1;
					getTime(Time);	
					PRINT_STATMSG(statFile, Time, programName, "Laser 2 broken, laser2Count: ", laser2Count);
					// Sleep program to change state
					usleep(500);
					//Change state
					getTime(Time);
					PRINT_MSG(logFile, Time, programName, "The state has changed\n\n");
					s = LEFT_BROKEN_IN;
				}
				
				//Check if right laser is broken first, therefore object going out
				if (!laserDiodeStatus(gpio, 1)){
					//Add 1 to right laser count since broken
					laser1Count += 1;
					getTime(Time);	
					PRINT_STATMSG(statFile, Time, programName, "Laser 1 broken, laser1Count: ", laser1Count);
					//Sleep program to change state
					usleep(500);
					//Change state
					getTime(Time);
					PRINT_MSG(logFile, Time, programName, "The state has changed\n\n");
					s = RIGHT_BROKEN_OUT;
				}
				
			
			
			//If nothing happens stays in the same state
			break;
			
			
		case LEFT_BROKEN_IN:	
			
			//Check if right laser is broken 
			if (!laserDiodeStatus(gpio, 1)){
				//Add 1 to right laser count if broken
				laser1Count += 1;
				getTime(Time);	
				PRINT_STATMSG(statFile, Time, programName, "Laser 1 broken, laser1Count: ", laser1Count);
				//Sleep program to reset state without adding counts
				usleep(500);
				//Change state
				getTime(Time);
				PRINT_MSG(logFile, Time, programName, "The state has changed\n\n");
				s = BOTH_BROKEN_IN;
			}
			
			//Check if object decides to go backwards halfway through
			else if (laserDiodeStatus(gpio, 2) && laserDiodeStatus(gpio, 1)){
				//Change state
				getTime(Time);
				PRINT_MSG(logFile, Time, programName, "The state has changed\n\n");
				s = BOTH_ON_IN;
			}

			
			//If nothing happens stays in the same state
			break;
			
		case BOTH_BROKEN_IN:
			
			//Check if left laser broken and right laser is connected
			if (!laserDiodeStatus(gpio, 2) && laserDiodeStatus(gpio, 1)){
				//Change state
				getTime(Time);
				PRINT_MSG(logFile, Time, programName, "The state has changed\n\n");
				s = LEFT_BROKEN_IN;
			}
			
			//Check if right laser broken and left laser is connected
			else if (!laserDiodeStatus(gpio,1) && laserDiodeStatus(gpio, 2)){
				//Change state
				getTime(Time);
				PRINT_MSG(logFile, Time, programName, "The state has changed\n\n");
				s = RIGHT_BROKEN_IN;
			}
			
			//If nothing happens stays in the same state
			break;
		
		case RIGHT_BROKEN_IN:

			//Check if laser 2 gets broken, object going backwards
			if (!laserDiodeStatus(gpio, 2)){
				//Adding count to laser 2
				laser2Count += 1;
				getTime(Time);	
				PRINT_STATMSG(statFile, Time, programName, "Laser 2 broken, laser2Count: ", laser2Count);
				//Sleep program to change state and stop counting
				usleep(500);
				//Change state
				getTime(Time);
				PRINT_MSG(logFile, Time, programName, "The state has changed\n\n");
				s = BOTH_BROKEN_IN;
			}
			
			//Check if laser 2 and laser 1 are connected
			else if (laserDiodeStatus(gpio, 2) && laserDiodeStatus(gpio, 1)){
				//Add 1 to entering count
				numberIn += 1;
				getTime(Time);		
				PRINT_STATMSG(statFile, Time, programName, "Object entered, numberIn: ", numberIn);
				//Sleep program to limit count and change state
				usleep(500);
				//Change state
				getTime(Time);
				PRINT_MSG(logFile, Time, programName, "The state has changed\n\n");
				s = BOTH_ON_IN;
			}
		
			//If nothing happens stays in the same state
			break;
			
		case BOTH_ON_IN:
		
			//Check if laser 2 connection is broken, object attempting to enter
			if (!laserDiodeStatus(gpio, 2)){
				//Add 1 count to laser 2 being broken
				laser2Count += 1;
				getTime(Time);	
				PRINT_STATMSG(statFile, Time, programName, "Laser 2 broken, laser2Count: ", laser2Count);
				//Sleep program to limit count
				usleep(500);
				//Change state
				getTime(Time);
				PRINT_MSG(logFile, Time, programName, "The state has changed\n\n");
				s = LEFT_BROKEN_IN;
			}
			
			//Check if laser 1 connection is broken, object attempting to exit
			else if (!laserDiodeStatus(gpio, 1)){
				//Add 1 count to laser 1 being broken
				laser1Count += 1;
				getTime(Time);	
				PRINT_STATMSG(statFile, Time, programName, "Laser 1 broken, laser1Count: ", laser1Count);
				//Sleep program to limit count
				usleep(500);
				//Change state
				getTime(Time);
				PRINT_MSG(logFile, Time, programName, "The state has changed\n\n");
				s = RIGHT_BROKEN_OUT;
			}
			
			//If nothing happens stays in the same state
			break;
		
		case RIGHT_BROKEN_OUT:
			//Check if laser 2 is broken as well
			if (!laserDiodeStatus(gpio, 2)){
				//Add 1 to laser 2 count if broken
				laser2Count += 1;
				getTime(Time);	
				PRINT_STATMSG(statFile, Time, programName, "Laser 2 broken, laser2Count: ", laser2Count);
				//Sleep program to limit count
				usleep(500);
				//Change state
				getTime(Time);
				PRINT_MSG(logFile, Time, programName, "The state has changed\n\n");
				s = BOTH_BROKEN_OUT;
			}
			
			//Check if object went backwards, not exiting room
			else if (laserDiodeStatus(gpio, 2) && laserDiodeStatus(gpio, 1)){
				//Change state
				getTime(Time);
				PRINT_MSG(logFile, Time, programName, "The state has changed\n\n");
				s = BOTH_ON_OUT;
			}
	
			//If nothing happens stays in the same state
			break;
		
		case BOTH_BROKEN_OUT:
			
				//Check if object went foward
				if (!laserDiodeStatus(gpio, 2) && laserDiodeStatus(gpio, 1)){
					//Change state
					getTime(Time);
					PRINT_MSG(logFile, Time, programName, "The state has changed\n\n");
					s = LEFT_BROKEN_OUT;
				}
				
				//Check if object went backwards
				else if (!laserDiodeStatus(gpio,1) && laserDiodeStatus(gpio, 2)){
					//Change state
					getTime(Time);
					PRINT_MSG(logFile, Time, programName, "The state has changed\n\n");
					s = RIGHT_BROKEN_OUT;
				}
			
			//If nothing happens stays in the same state
			break;
		
		
		case LEFT_BROKEN_OUT:
			
				if (!laserDiodeStatus(gpio, 1)){
					//Add 1 to laser 1 count
					laser1Count += 1;
					getTime(Time);	
					PRINT_STATMSG(statFile, Time, programName, "Laser 1 broken, laser1Count: ", laser1Count);
					//Sleep program to limit count
					usleep(500);
					//Change state
					getTime(Time);
					PRINT_MSG(logFile, Time, programName, "The state has changed\n\n");
					s = BOTH_BROKEN_OUT;
				}
				
				//Check if object goes foward, leaving the room fully
				else if (laserDiodeStatus(gpio, 2) && laserDiodeStatus(gpio, 1)){
					//Add 1 to exit count if fully exits
					numberOut += 1;
					getTime(Time);	
					PRINT_STATMSG(statFile, Time, programName, "Object exited, numberOut: ", numberOut);
					//Sleep program to limit count
					usleep(500);
					//Change state
					getTime(Time);
					PRINT_MSG(logFile, Time, programName, "The state has changed\n\n");
					s = BOTH_ON_OUT;
				}
			
			//If nothing happens stays in the same state
			break;
			
	
		case BOTH_ON_OUT:
			
				//Check if object attempts to enter room
				if (!laserDiodeStatus(gpio, 2)){
					//Add 1 to laser 2 count
					laser2Count += 1;
					getTime(Time);	
					PRINT_STATMSG(statFile, Time, programName, "Laser 2 broken, laser2Count: ", laser2Count);
					//Sleep program to limit count
					usleep(500);
					//Change state
					getTime(Time);
					PRINT_MSG(logFile, Time, programName, "The state has changed\n\n");
					s = LEFT_BROKEN_IN;
				}
				
				//Check if object attempts to exit room
				else if (!laserDiodeStatus(gpio, 1)){
					//Add 1 to laser 1 count
					laser1Count += 1;
					getTime(Time);	
					PRINT_STATMSG(statFile, Time, programName, "Laser 1 broken, laser1Count: ", laser1Count);
					//Sleep program to limit count
					usleep(500);
					//Change state
					getTime(Time);
					PRINT_MSG(logFile, Time, programName, "The state has changed\n\n");
					s = RIGHT_BROKEN_OUT;
				}
			
			//If nothing happens stays in the same state
			break;
			
		}
	
	}
	outputMessage(laser1Count, laser2Count,  numberIn, numberOut);

	//Writing a V to the watchdog file will disable to watchdog and prevent it from
	//resetting the system
	write(watchdog, "V", 1);
	getTime(Time);
	//Log that the Watchdog was disabled
	PRINT_MSG(logFile, Time, programName, "The Watchdog was disabled\n\n");

	//Close the watchdog file so that it is not accidentally tampered with
	close(watchdog);
	getTime(Time);
	//Log that the Watchdog was closed
	PRINT_MSG(logFile, Time, programName, "The Watchdog was closed\n\n");

	//Free the gpio pins
	gpiolib_free_gpio(gpio);
	getTime(Time);
	//Log that the GPIO pins were freed
	PRINT_MSG(logFile, Time, programName, "The GPIO pins have been freed\n\n");




}
#endif