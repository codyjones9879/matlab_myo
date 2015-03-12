/*
*	engdemo.cpp
*
*	A simple program to illustrate how to call MATLAB
*	Engine functions from a C++ program.
*
* Copyright 1984-2011 The MathWorks, Inc.
* All rights reserved
*/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "engine.h"
//to get the fill() command into existance. fill() => Assigns val to all the elements in the array
#include <array>
//all your std::input/output streams library
#include <iostream>
//sstream => Header providing string stream classes:
#include <sstream>
//This header defines a set of standard exceptions that both the library and programs can use to report common errors.
#include <stdexcept>
//
#include <fstream>
#include <myo/myo.hpp>


#define  BUFSIZE 256

class DataCollector : public myo::DeviceListener {

public:DataCollector() : emgSamples() {
		

}
	   /*
	   here is where we put all of our functions belonging to our DataCollector object
	   */
	   //onUnpair called when Myo disconnected from Myo Connect by the user.
	   //void = no type or return associated with it.
	   //
	   void onUnpair(myo::Myo* myo, uint64_t timestamp) {

		   std::cout << "We have lost our MYO";
		   emgSamples.fill(0);

	   }
	   void onEmgData(myo::Myo* myo, uint64_t timestamp, const int8_t* emg)
	   {
		   emgStringSamples = "[";
		   for (int i = 0; i < 8; i++) {
			   emgSamples[i] = emg[i];
			   //emgStringSamples = emgStringSamples + std::to_string(emg[i]) + "]";
		       emgStringSamples = emgStringSamples + std::to_string(emg[i]) + ",";
				   
		   }
		   emgStringSamples = emgStringSamples + "]";
					
		
	   }
	   void print()
	   {
		   // Clear the current line
		   std::cout << '\n';

		   // Print out the EMG data.
		   //Store in global object? good idea?
		   for (size_t i = 0; i < emgSamples.size(); i++) {
			   std::ostringstream oss;
			   oss << static_cast<int>(emgSamples[i]);
			   std::string emgString = oss.str();

			   //std::cout << emgString << ';';
			   //std::cout << emgStringSamples;


		   }
		   //std::cout << emgStringSamples;
		   std::cout << std::flush;
	   }




	   //this is actually a variable....rediculously complicated.
	   std::array<int8_t, 8> emgSamples;
	   std::string emgStringSamples;


};



int main(int argc, char** argv)

{
	Engine *ep;
	mxArray *T = NULL, *result = NULL;
	mxArray *H = NULL;
	char buffer[BUFSIZE + 1];

	//x-axis samples
	//double time[8] = { 0.0, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0 };
	

	/*
	* Call engOpen with a NULL string. This starts a MATLAB process
	* on the current host using the command "matlab".
	*/

	ep = engOpen("");
	//if (!(ep = engOpen(""))) {
		//fprintf(stderr, "\nCan't start MATLAB engine\n");
		//return EXIT_FAILURE;
	//}

	/*
	* This part is to initilize the MYO and make sure we connect to our MYO prior 
	* to opening Matlab.
	*/

	try {
		//create a hub to connect your myo to
		myo::Hub hub("com.example.Engine");
			
	//send out a message as to what we are doing
		std::cout << "Connecting to your MYO...One Moment" << std::endl;



		//Check that myo
		myo::Myo* myo = hub.waitForMyo(0);

		// If we didn't find a myo lets error out.
		if (!myo) {
			throw std::runtime_error("Dude, Plug it in.");
		}

		std::cout << "Connected to a Myo armband!" << std::endl << std::endl;
		//enable EMG streaming
		myo->setStreamEmg(myo::Myo::streamEmgEnabled);

		//Construct our deviceListener
		DataCollector collector;

		hub.addListener(&collector);

		//create a timestamp variable
		//time_t timer;

		T = mxCreateDoubleMatrix(1, 8, mxREAL);

		//
		//memcpy((void *)mxGetPr(T), (void *)time, sizeof(time));

		engPutVariable(ep, "T", T);


		int x = 0;
		while (x < 1000000) {


			hub.run(1000 / 20);
			collector.print();
		



			//need to save as a string that looks like this [A;B;C;D;E;F;G;H]
			std::array<int8_t, 8> batman = collector.emgSamples;
			std::string batString = collector.emgStringSamples;
			std::string matlabCommand = "T=[T;" + batString + "];";
			std::cout << matlabCommand;

			const char* emgCharData = matlabCommand.c_str();
			x++;
			//t = [t[105; 22; 57; 77; -5; 1; -23; 76; ]]



			//engPutVariable(ep, "batman", batman);
			//engEvalString(ep, "T=[T T]");
			engEvalString(ep, emgCharData);

			//const char* hello = "T";

			//engEvalString(ep, "D = .5.*(-9.8).*T.^2;");

			/*
			* Plot the result
			*/
			engEvalString(ep, "plot(T);");
			//engEvalString(ep, "title('Position vs. Time for a falling object');");
			//engEvalString(ep, "xlabel('Time (seconds)');");
			//engEvalString(ep, "ylabel('Position (meters)');");

			//engEvalString(ep, "T=T+1;");

			/*
			* use fgetc() to make sure that we pause long enough to be
			* able to see the plot
			*/
			//printf("Hit return to continue\n\n");
			//fgetc(stdin);
			/*
			* We're done for Part I! Free memory, close MATLAB figure.
			*/
			//printf("Done for Part I.\n");
			//mxDestroyArray(T);
			//engEvalString(ep, "close;");


		}
		std::cout << "\n";
	}
	catch(const std::exception& e) {
		std::cerr << "Misson Control, We have a problem: " << e.what() << std::endl;
		std::cerr << "Try again, make sure myo connect is running.";
		std::cin.ignore();
		return 1;
	}

	/*
	* PART I
	*
	* For the first half of this demonstration, we will send data
	* to MATLAB, analyze the data, and plot the result.
	*/

	/*
	* Create a variable for our data
	*/
	T = mxCreateDoubleMatrix(1, 11, mxREAL);
	//memcpy((void *)mxGetPr(T), (void *)time, sizeof(time));
	/*
	* Place the variable T into the MATLAB workspace
	*/
	//engPutVariable(ep, "T", T);

	/*
	* Evaluate a function of time, distance = (1/2)g.*t.^2
	* (g is the acceleration due to gravity)
	*/
	//engEvalString(ep, "D = .5.*(-9.8).*T.^2;");

	/*
	* Plot the result
	*/
	//engEvalString(ep, "plot(T,D);");
	//engEvalString(ep, "title('Position vs. Time for a falling object');");
	//engEvalString(ep, "xlabel('Time (seconds)');");
	//engEvalString(ep, "ylabel('Position (meters)');");

	/*
	* use fgetc() to make sure that we pause long enough to be
	* able to see the plot
	*/
	//printf("Hit return to continue\n\n");
	//fgetc(stdin);
	/*
	* We're done for Part I! Free memory, close MATLAB figure.
	*/
	//printf("Done for Part I.\n");
	//mxDestroyArray(T);
	//engEvalString(ep, "close;");


	/*
	* PART II
	*
	* For the second half of this demonstration, we will request
	* a MATLAB string, which should define a variable X.  MATLAB
	* will evaluate the string and create the variable.  We
	* will then recover the variable, and determine its type.
	*/

	/*
	* Use engOutputBuffer to capture MATLAB output, so we can
	* echo it back.  Ensure first that the buffer is always NULL
	* terminated.
	*/

	//buffer[BUFSIZE] = '\0';
	//engOutputBuffer(ep, buffer, BUFSIZE);
	//while (result == NULL) {
		//char str[BUFSIZE + 1];
		/*
		* Get a string input from the user
		*/
		//printf("Enter a MATLAB command to evaluate.  This command should\n");
		//printf("create a variable X.  This program will then determine\n");
		//printf("what kind of variable you created.\n");
		//printf("For example: X = 1:5\n");
		//printf(">> ");

		//fgets(str, BUFSIZE, stdin);

		/*
		* Evaluate input with engEvalString
		*/
		//engEvalString(ep, str);

		/*
		* Echo the output from the command.
		*/
		//printf("%s", buffer);

		/*
		* Get result of computation
		*/
		//printf("\nRetrieving X...\n");
		//if ((result = engGetVariable(ep, "X")) == NULL)
		//	printf("Oops! You didn't create a variable X.\n\n");
		//else {
		///	printf("X is class %s\t\n", mxGetClassName(result));
	//	}
	//}

	/*
	* We're done! Free memory, close MATLAB engine and exit.
	*/
	//printf("Done!\n");
	//mxDestroyArray(result);
	//engClose(ep);
	system("pause");
	return EXIT_SUCCESS;
}








