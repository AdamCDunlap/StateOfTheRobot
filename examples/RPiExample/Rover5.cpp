#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>

#include <Rover5.h>
#include <StateOfTheRobot.h>
using namespace std::literals::chrono_literals;

std::vector<Rover5*> Rover5::instances;

Rover5::Rover5(uint8_t i2caddress)
    : interfaceAddress(i2caddress)
{
}

void Rover5::begin() {

    if ((i2cfile = open("/dev/i2c-1", O_RDWR)) < 0) {
        if ((i2cfile = open("/dev/i2c-0", O_RDWR)) < 0) {
            fprintf(stderr, "Failed to open i2c file\n");
            exit(1);
        }
    }

    if (ioctl(i2cfile, I2C_SLAVE, interfaceAddress) < 0) {
      fprintf(stderr, "I2C: Failed to acquire bus access/talk to slave %d\n", interfaceAddress);
      exit(1);
    }

    for (size_t i=0; i<spdLogLen; i++) {
        while(!updateEncoders());
    }

    pos.x = 0;
    pos.y = 0;
    pos.angle = 0;

    instances.push_back(this);
}
void Rover5::end() {
    sendI2C();
    close(i2cfile);
}

// Each speed is an int from -255 to 255
void Rover5::run(int16_t frontLeft, int16_t frontRight, int16_t backLeft, int16_t backRight) {
    powers[FL] = frontLeft;
    powers[FR] = frontRight;
    powers[BL] = backLeft;
    powers[BR] = backRight;
}
void Rover5::run(int16_t powers[4]) {
    memcpy(Rover5::powers, powers, sizeof(Rover5::powers));
}
void Rover5::move(int16_t x, int16_t y, int16_t z) {
    powers[FL] = +y + x + z;
    powers[FR] = +y - x - z;
    powers[BL] = +y - x + z;
    powers[BR] = +y + x - z;

    normalize4(powers, 255);

    // map {-255,255} to {-255,-25}, {0}, and {25,255}
    for (uint8_t i=0; i<4; i++) {
        if (powers[i] < 0) {
            // Cast to unsigned for the multiplication and division,
            //  but then cast back before the subtraction
            powers[i] = (int16_t)(((int16_t)(powers[i] + 255) *
                               (uint16_t)(255-minPower)) / 255) - 255;
        } else if (powers[i] > 0) {
            powers[i] = (int16_t)(((uint16_t)powers[i] *
                               (uint16_t)(255-minPower)) / 255) + minPower;
        }
        // If it's 0, keep it 0
    }

}

void Rover5::runOne(int16_t power, mtrNum_t num) {
    powers[num] = power;
}

void sendI2Cs() {
    Rover5::sendI2Cs();
}

// Runs motors using values stored in powers array
global_func_handle i2csender = global_func(GlobalFuncStart::enabled, sendI2Cs);

void Rover5::sendI2Cs() {
    static size_t idx = 0;
    if (!instances.empty()) {
        every(10ms) {
            Rover5* bot = instances[idx];
            ++idx;
            if (idx >= instances.size()) {
                idx = 0;
            }
            bot->sendI2C();
        }
    }
}

void Rover5::sendI2C() {
    powers[BR] *= -1;
    powers[BL] *= -1;

    printf("Sending %d %d %d %d\n", powers[0], powers[1], powers[2], powers[3]);

    int written;
    if ((written = write(i2cfile, powers, 8)) != 8) {
        printf("Failed to write to i2c. Wrote %d bytes\n", written);
    }
    powers[BR] *= -1;
    powers[BL] *= -1;
}

void Rover5::getPowers(int16_t outpowers[4]) {
    memcpy(outpowers, Rover5::powers, sizeof(Rover5::powers));
}

// Populates the ticks array with the current number of encoder ticks for
//  each motor
void Rover5::getTicks(long outticks[4]) {
    memcpy(outticks, Rover5::ticks, sizeof(Rover5::ticks));
}

void Rover5::getTickSpeeds(int16_t outspeeds[4]) {
    memcpy(outspeeds, Rover5::speeds, sizeof(Rover5::speeds));
}

void Rover5::getSpeeds(int16_t outspeeds[4]) {
    for (uint8_t i=0; i<4; i++) {
        outspeeds[i] = speeds[i] * ticksToMills;
    }
}

void Rover5::getDists(long dists[4]) {
    for (uint8_t i=0; i<4; i++) {
        dists[i] = ticks[i] * ticksToMills;
    }
}

bool Rover5::updateEncoders() {

    //{
    //    //unsigned long endtime;
    //    //unsigned long starttime;
    //    //starttime = micros();
    //    Wire.requestFrom(interfaceAddress, (uint8_t)16);
    //    //endtime = micros();
    //    //Serial.print(F("requestFrom time: "));
    //    //Serial.print(endtime - starttime);
    //    //Serial.print(' ');

    //}

    //if (Wire.available() < 16) {
    //    //Serial.println(F("Bytes not avilable"));
    //    return false;
    //}

    //uint8_t* ticksbreakdown = (uint8_t*)ticks;
    //for (uint8_t i=0; i<16; i++) {
    //    ticksbreakdown[i] = Wire.read();
    //}
    //ticks[FR] *= -1;
    //ticks[FL] *= -1;

    //updateSpeeds(ticks);

    //updatePosition();
    return true;
}

void Rover5::updateSpeeds(long ticks[4]) {
    //unsigned long curTime = micros();
    unsigned long curTime = 0;

    unsigned long timesDiff = curTime - tickLogs.times[tickLogs.nextEntry];
    //Serial.print(F("tm: ")); Serial.print(timesDiff); Serial.print(' ');
    for (uint8_t i=0; i<4; i++) {
        // Difference in ticks from oldest entry to entry about to be put in
        //  over difference in the times over the same
        long ticksDiff =  ticks[i] - tickLogs.ticks[tickLogs.nextEntry][i];
        speeds[i] = (int16_t)(1000000.0 * (float)ticksDiff / (float)timesDiff);
        //Serial.print(F("ck")); Serial.print(i); Serial.print(F(": ")); Serial.print(ticksDiff); Serial.print(' ');
        //Serial.print(F("tm: ")); Serial.print(timesDiff); Serial.print('\t');
    }
    tickLogs.Put(ticks, curTime);
    //Serial.print('|');
}

void Rover5::updatePosition() {
    // Variables used for integrating/dervitizing
    //unsigned long curTime = micros();
    unsigned long curTime = 0;
    static unsigned long lastTime = curTime;
    unsigned int timeDiff = curTime - lastTime;
    lastTime = curTime;

    // "n" function to get the current rotational velocity

    // use K as defined in Ether's paper
    // "Kinematic Analysis of Four-Wheel Mecanum Vehicle"
    // http://www.chiefdelphi.com/media/papers/download/2722

    // 6.75 is wheel base in inches, 8.5 is track witdth
    // Factor of (1000/ticksToMills) on each one is to convert to ticks
    // dividing by 1000 makes it into milliradians
    // Units: ticks/milliradian
    // Ends up being 0.8090376273838012901584924638102813403418365325139869
    const double K = (((6.75*(1000/ticksToMills))/2 + (8.5*(1000/ticksToMills))/2) * 4)/1000;

    // no need to integrate the speeds when the distances are there
    //
    //long angVel = (+speeds[FL] -speeds[FR] +speeds[BL] -speeds[BR])/K;
    // Integrate angVel to get the angle
    // Devide by 1000000 to convert from microseconds to seconds
    //pos.angle += (angVel * (currentTime - lastTime)) / 1000000;

    pos.angle = ticksToMills * (double)(+ticks[FL] -ticks[FR] +ticks[BL] -ticks[BR])/K;


    // "r" function to get the field relative velocity and rotational velocity

    // the postfix r means it's robot relative
    // These are in ticks/second
    int xvelr = (+(long)speeds[FL] -(long)speeds[FR] -(long)speeds[BL] +(long)speeds[BR])/4;
    int yvelr = (+(long)speeds[FL] +(long)speeds[FR] +(long)speeds[BL] +(long)speeds[BR])/4;

    // Now rotate the vector
    //float sinA = sin(pos.angle/1000.0);
    float sinA = 0;
    //float cosA = cos(pos.angle/1000.0);
    float cosA = 0;
    float xvel = /*(int)*/(xvelr * cosA - yvelr * sinA);
    float yvel = /*(int)*/(xvelr * sinA + yvelr * cosA);

    // max val of xvel is 25000
    // ((2^32)-1)/25000 = 172,000, which would mean if timeDiff is more than
    // 172 milliseconds, it would overflow. So first divide timediff by 10
    // (not much is lost since micros() only has a precision of 4) so that
    // a full second and half can go by without overflow. More than that and
    // the user deserves to get wrong answers.
    //pos.x  += (((long)xvel * (long)(timeDiff/10))/100000);
    //pos.y  += (((long)yvel * (long)(timeDiff/10))/100000);
    pos.x  += (((float)xvel * (float)(timeDiff/10))/100000);
    pos.y  += (((float)yvel * (float)(timeDiff/10))/100000);

    //printf_P(PSTR("xvelr%5d yvelr%5d sinA %.3f xvel %.3f yvel %.3f pos.x % f pos.y % f pos.angle % f enc[FL] %ld enc[FR] %ld enc[BL] %ld enc[BR] %ld\r\n"),
    //                  xvelr,   yvelr,     sinA,     xvel,     yvel,    pos.x,    pos.y,    pos.angle,  ticks[FL],  ticks[FR],  ticks[BL],  ticks[BR]);
}

void Rover5::normalize4(int16_t nums[4], int16_t maximum) {
    int16_t highest = abs(nums[0]);
    for (uint8_t i=1; i<4; i++) {
        int16_t tmp = abs(nums[i]);
        if(tmp > highest) highest = tmp;
    }

    // If all are below the max, we don't need to do anything
    if (highest <= maximum) return;

    for (uint8_t i=0; i<4; i++) {
        nums[i] = (int16_t)(( (int32_t)nums[i] * (long)maximum)/(long)highest);
    }
}

