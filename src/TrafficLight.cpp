#include <iostream>
#include <random>
#include <condition_variable>
#include "TrafficLight.h"

/* Implementation of class "MessageQueue" */


template <typename T>
T MessageQueue<T>::receive()
{ 
    std::unique_lock<std::mutex> uLock(_mutex);
    _cond.wait(uLock, [this] {return !_queue.empty(); }); // pass unique lock to condition variable

    T msg = std::move(_queue.back());
    _queue.pop_back();
    return msg;

}

template <typename T>
void MessageQueue<T>::send(T &&msg)
{
    // See the use of emplace_back
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    std::lock_guard<std::mutex> lck(_mutex);
    _queue.clear();
    _queue.emplace_back(std::move(msg));
    _cond.notify_one();
}


/* Implementation of class "TrafficLight" */


TrafficLight::TrafficLight()
{
    _currentPhase = TrafficLightPhase::red;
}

void TrafficLight::waitForGreen()
{
    while(true) {
        // sleep at every iteration to reduce CPU usage
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        _currentPhase = _msgQue.receive();
        // break the loop if we get a green
        if(_currentPhase==TrafficLightPhase::green)
            break;
    }
    //std::unique_lock<std::mutex> lck(_mtx);
    //std::cout << "    Traffic Lights have changed to green" << std::endl;
    //lck.unlock();
    
}

TrafficLightPhase TrafficLight::getCurrentPhase()
{
    return _currentPhase;
}

void TrafficLight::simulate()
{
    threads.emplace_back(std::thread(&TrafficLight::cycleThroughPhases, this));
}

// virtual function which is executed in a thread
void TrafficLight::cycleThroughPhases()
{
    // This function implements an infinite loop that measures the time between two loop cycles 
    // and toggles the current phase of the traffic light between red and green and sends an update method 
    // to the message queue using move semantics. The cycle duration should be a random value between 4 and 6 seconds. 
    // Also, the while-loop should use std::this_thread::sleep_for to wait 1ms between two cycles. 
    std::chrono::time_point<std::chrono::system_clock> lastUpdate;
    /* Init random generator between 4 and 6 seconds */
    std::random_device rd;
    std::mt19937 eng(rd());
    std::uniform_int_distribution<> distr(4000, 6000);
    /* initialize variables */
    int cycleDuration = distr(eng);
    long timeSinceLastUpdate = 0;
    // init stop watch
    lastUpdate = std::chrono::system_clock::now();
    while(true) {
        // sleep at every iteration to reduce CPU usage
        std::this_thread::sleep_for(std::chrono::milliseconds(1));

        // compute time difference to stop watch
        timeSinceLastUpdate = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - lastUpdate).count();
        if (timeSinceLastUpdate >= cycleDuration ) 
        {
            // toggle the current phase 
            if(_currentPhase==TrafficLightPhase::red )
                _currentPhase  = TrafficLightPhase::green;
            else 
                _currentPhase  = TrafficLightPhase::red;
            
            _msgQue.send(std::move(_currentPhase));

            // pick another cycle_duration between 4 to 6 seconds
            cycleDuration = distr(eng);
            // It is very important to do this update here within if-statement
            lastUpdate = std::chrono::system_clock::now();
        }

    }
}

