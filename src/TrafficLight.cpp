#include <iostream>
#include <random>
#include <chrono>
#include <thread>
#include "TrafficLight.h"
static std::random_device randomDevice;
/* Implementation of class "MessageQueue" */


template <typename T>
T MessageQueue<T>::receive()
{
    // FP.5a : The method receive should use std::unique_lock<std::mutex> and _condition.wait() 
    // to wait for and receive new messages and pull them from the queue using move semantics. 
    // The received object should then be returned by the receive function. 
  	 std::unique_lock<std::mutex> uLock(_mutex);
 	 _condition.wait(uLock, [this] { return !_queue.empty(); });
  // wait for meesages
  	T messagePool = std::move(_queue.back());
   _queue.clear();
   return messagePool;
}

template <typename T>
void MessageQueue<T>::send(T &&msg)
{
    // FP.4a : The method send should use the mechanisms std::lock_guard<std::mutex> 
    // as well as _condition.notify_one() to add a new message to the queue and afterwards send a notification.
    std::lock_guard<std::mutex> gLock(_mutex);
    _queue.emplace_back(std::move(msg));
    _condition.notify_one();
}


/* Implementation of class "TrafficLight" */


TrafficLight::TrafficLight()
{
    _currentPhase = TrafficLightPhase::red;
   _type = ObjectType::objectTrafficLight;
  
}

void TrafficLight::waitForGreen()
{
    // FP.5b : add the implementation of the method waitForGreen, in which an infinite while-loop 
    // runs and repeatedly calls the receive function on the message queue. 
    // Once it receives TrafficLightPhase::green, the method returns.
   while (true) {
        TrafficLightPhase greenLight = _messageQueue.receive();
        if (greenLight == TrafficLightPhase::green)
            return;
    }
}

TrafficLightPhase TrafficLight::getCurrentPhase()
{
  return _currentPhase;
}

void TrafficLight::simulate()
{
    // FP.2b : Finally, the private method „cycleThroughPhases“ should be started in a thread when the public method „simulate“ is called. To do this, use the thread queue in the base class. 
  threads.emplace_back(std::thread(&TrafficLight::cycleThroughPhases, this));

}

// virtual function which is executed in a thread
void TrafficLight::cycleThroughPhases()
{
    // FP.2a : Implement the function with an infinite loop that measures the time between two loop cycles 
    // and toggles the current phase of the traffic light between red and green and sends an update method 
    // to the message queue using move semantics. The cycle duration should be a random value between 4 and 6 seconds. 
    // Also, the while-loop should use std::this_thread::sleep_for to wait 1ms between two cycles. 
    // Random number generator
//     std::random_device randomDevice;
    std::mt19937 mt(randomDevice());
    std::uniform_real_distribution<double> dist(4,6);
    float cycleTime = dist(mt);

    // init variables for measuring time
    auto start = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> measuredTime;

    while (true) {
        // sleep for 100 miliseconds is enough since we wait for 4-6s
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        measuredTime = std::chrono::high_resolution_clock::now() - start;

        if (measuredTime.count() > cycleTime) {
            // reset start time and get new cycle time
            start = std::chrono::high_resolution_clock::now();
            cycleTime = dist(mt);
            // toggle phase
            _currentPhase =
                (_currentPhase == TrafficLightPhase::green) ? red : green;

            // send update method to message queue using move semantics.
//             TrafficLightPhase p = _currentPhase;
            _messageQueue.send(std::move(_currentPhase));
        }
    }
}

