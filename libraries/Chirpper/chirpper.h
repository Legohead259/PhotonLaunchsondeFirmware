#ifndef CHIRPPER_H
#define CHIRPPER_H

class Chirpper {
public:
    Chirpper(int pin);
    void chirp(int pulse=25);
    void readyChirp(int intv=125, int iter=3);

private:
    int pin = 0;
};

#endif