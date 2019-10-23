#ifndef CHIRPPER_H
#define CHIRPPER_H

class Logger {
    public:
        Logger();
        File logFile;
    private:
        const int CS_PIN = 4;
};

#endif