#ifndef ERRORS_H
#define ERRORS_H


struct KeyNotFoundError: public std::runtime_error
{
    KeyNotFoundError(std::string const& key)
        : std::runtime_error("Key " + key + " was not found.")
    {}
};

#endif