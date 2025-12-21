#ifndef __GAME_EXCEPTION_H__
#define __GAME_EXCEPTION_H__

#include <exception>
#include <string>

namespace core {

class GameException : public std::exception {
public:
    explicit GameException(const std::string& message) : _message(message) {}

    virtual const char* what() const noexcept override {
        return _message.c_str();
    }

private:
    std::string _message;
};

} // namespace core

#endif // __GAME_EXCEPTION_H__
