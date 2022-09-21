#pragma once

#include <iostream>
#include <sstream>

template <typename Iterator>
std::string join(Iterator begin, Iterator end, std::string separator = ',') {
    std::ostringstream oss;
    if (begin != end) {
        oss << *begin++;
        for (; begin != end; ++begin) oss << separator << *begin;
    }
    return oss.str();
}

template <typename Container>
std::string join(Container const& c, std::string separator = ',') {
    return join(std::begin(c), std::end(c), separator);
}