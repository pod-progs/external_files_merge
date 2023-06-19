#pragma once
#include <string>
#include <iostream>

struct MarketData 
{
    std::string symbol;
    std::string timestamp;
    double price;
    int size;
    std::string exchange;
    std::string type;

    // Overload the string operator for MarketData
    friend std::ostream& operator<<(std::ostream& os, const MarketData& entry) 
    {
        os << entry.symbol << ", ";
        os << entry.timestamp << ", ";
        os << entry.price << ", ";
        os << entry.size << ", ";
        os << entry.exchange << ", ";
        os << entry.type;
        return os;
    }
};

// Comparator function to compare market data entries based on timestamp and symbol
inline bool compare_market_data(const std::pair<MarketData, size_t>& md1, const std::pair<MarketData, size_t>& md2) 
{
    if (md1.first.timestamp == md2.first.timestamp) {
        return md1.first.symbol > md2.first.symbol;
    }
    return md1.first.timestamp > md2.first.timestamp;
}