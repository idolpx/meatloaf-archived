#ifndef VOICE_H
#define VOICE_H

#include <string>

#include "bus.h"
#include "samlib.h"

class iecVoice : public virtualDevice
{
protected:
    // act like a printer for POC
    uint8_t lastAux1 = 0;
    uint8_t buffer_idx = 0;
    uint8_t sioBuffer[41];
    uint8_t lineBuffer[121];
    uint8_t samBuffer[121];
    void write();

    /**
     * @brief Process command fanned out from bus
     * @return new device state
     */
    device_state_t process() override;

    virtual void status();

private:
    bool sing = false;
    std::string pitch;
    std::string mouth;
    bool phonetic = false;
    std::string speed;
    std::string throat;

    void sam_init();
    void sam_parameters();

public:
};

#endif /* VOICE_H */