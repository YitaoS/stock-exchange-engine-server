# Testing Server Scalability

This Python script tests server concurrency using multiprocessing. It sends data to the server and measures the time it takes to send the data.

## Usage

use `python3 myxml.py` to simulate serial performance.

use `python3 test.py` to simulate parallel performance.

Import the necessary modules: time, multiprocessing, myxml.XMLTestGenerator, and myxml.test.

Create a function that sends data to the server. Customize it as needed.
Set the maximum number of processes and create a Pool object.
Start the timer and use the map method of the Pool object to send data to the server.

Close the Pool object and wait for all the processes to finish.

Calculate the elapsed time and print it.

That's it! Remember to be cautious with the number of processes used to avoid overloading the server.