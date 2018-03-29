Ömer Mesud TOKER
21302479
CS 342 - 1
Project 3

First of all, I adapted the monitor based solution given in our textbook. initialization, test, pickup and
putdown functions are those functions that adapted form textbook with pthread library functions. I
have also a trace function to determine what a philosopher does. It was also used for testing and
debugging. With this function, if a philosopher eats <count> times, thread will be exited. Moreover, I
have life function which is given to threads. It shows the life of philosophers, thinking and eating.
Randomizing eating and thinking times is done in this function. I have also a standartDeviation
function to find the standard deviation of hungry state durations for each philosopher. Lastly, in main
function, I started with some if statements to check the inputs. For example, if number of
philosophers is given as 28, one of those ifs gives an error message and then terminate the code.
Then, threads are created and finally, total durations of hungry state, average durations of hungry
states and standard deviation of hungry states for all philosophers are written as milliseconds
at the end of main function. Moreover, I print the average duration of hungry state of all
philosophers with its standard deviation.

To run this code please write similar to the following:

./phil 7 500 1000 50 100 uniform 10

