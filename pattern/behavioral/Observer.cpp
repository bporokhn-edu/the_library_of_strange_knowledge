/*
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
When:
    Use the Observer pattern when changes to the state of one object may require changing other objects,
    and the actual set of objects is unknown beforehand or changes dynamically.

    Use the pattern when some objects in your app must observe others, but only for a limited time or in specific cases.


* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
Pros:
    Open/Closed Principle.
    You can introduce new subscriber classes without having to change the publisher’s code
    (and vice versa if there’s a publisher interface).

    You can establish relations between objects at runtime.

Cons:
    Subscribers are notified in random order.

* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*/

#include <iostream>
#include <memory>
#include <ios>
#include <algorithm>
#include <functional>
#include <variant>
#include <exception>
#include <stdexcept>
#include <vector>
#include <numeric>

class IObserver
{
public:
    virtual ~IObserver() = default;
    virtual void update(float temperature, float humidity, float pressure) = 0;
};

class WeatherData
{
public:
    void addObserver(std::shared_ptr<IObserver> observer)
    {
        observers_.push_back(observer);
    }

    void removeObserver(std::shared_ptr<IObserver> observer)
    {
        observers_.erase(std::remove(observers_.begin(), observers_.end(), observer), observers_.end());
    }

    void notifyObservers()
    {
        for (auto observer : observers_)
        {
            observer->update(temperature_, humidity_, pressure_);
        }
    }

    void UpdateMeasurements(float temperature, float humidity, float pressure)
    {
        temperature_ = temperature;
        humidity_ = humidity;
        pressure_ = pressure;
        measurementsChanged();
    }

private:
    void measurementsChanged()
    {
        notifyObservers();
    }

    std::vector<std::shared_ptr<IObserver>> observers_{};
    float temperature_{};
    float humidity_{};
    float pressure_{};
};

class CurrentConditionsDisplay : public IObserver
{
public:
    void update(float temperature, float humidity, float pressure) override
    {
        std::cout << "Current conditions: " << temperature << "°C, " << humidity << "% humidity, " << pressure << " hPa\n";
    }
};

class StatisticsDisplay : public IObserver
{
public:
    void update(float temperature, [[maybe_unused]]float humidity, [[maybe_unused]]float pressure) override
    {
        temperatures_.push_back(temperature);
        float avgTemp = std::accumulate(temperatures_.begin(), temperatures_.end(), 0.0) / temperatures_.size();
        std::cout << "Average temperature: " << avgTemp << "°C\n";
    }

private:
    std::vector<float> temperatures_{};
};

int main()
{
    std::cout << std::endl
              << "Observer 1" << std::endl
              << std::endl;

    WeatherData weatherData;

    std::shared_ptr<IObserver> currentDisplay = std::make_shared<CurrentConditionsDisplay>();
    std::shared_ptr<IObserver> statisticsDisplay = std::make_shared<StatisticsDisplay>();

    weatherData.addObserver(currentDisplay);
    weatherData.addObserver(statisticsDisplay);

    weatherData.UpdateMeasurements(25.0, 65.0, 1013.0);
    weatherData.UpdateMeasurements(27.0, 70.0, 1012.0);
    weatherData.UpdateMeasurements(23.0, 60.0, 1011.0);

    std::cout
        << std::endl
        << "Observer End" << std::endl
        << std::endl;

    return 0;
}
