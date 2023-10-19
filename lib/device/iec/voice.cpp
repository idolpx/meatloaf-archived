#ifdef BUILD_IEC

#include "voice.h"

#include <string>

#include "utils.h"

using namespace std;

#define EOL 0x9B

void iecVoice::sam_parameters()
{
    string s = string((char *)lineBuffer); // change to lineBuffer
    vector<string> tokens = util_tokenize(s, ' ');

    for (vector<string>::iterator it = tokens.begin(); it != tokens.end(); ++it)
    {
        string t = *it;

        switch (t[0])
        {
        case 0x07: // ^G SING
            sing = true;
            break;
        case 0x09: // ^I PITCH
            pitch = *(++it);
            break;
        case 0x0D: // ^M Mouth
            mouth = *(++it);
            break;
        case 0x10: // ^P Phonetic
            phonetic = true;
            break;
        case 0x12: // ^R RESET
            sing = false;
            pitch.clear();
            mouth.clear();
            phonetic = false;
            speed.clear();
            throat.clear();
            break;
        case 0x13: // ^S Speed
            speed = *(++it);
            break;
        case 0x14: // ^T Throat
            throat = *(++it);
            break;
        default:
            if (it != tokens.begin())
                strcat((char *)samBuffer, " ");
            strcat((char *)samBuffer, t.c_str());
            break;
        }
    }
}

void iecVoice::sam_init()
{
    int n = 0;
    char *a[16];
    // int i = 0;

    memset(samBuffer, 0, sizeof(samBuffer));
    memset(a, 0, sizeof(a));

    // Construct parameter buffer.
    a[n++] = (char *)("sam");
    a[n++] = (char *)("-debug");

    sam_parameters();

    if (sing == true)
        a[n++] = (char *)("-sing");

    if (!pitch.empty())
    {
        a[n++] = (char *)("-pitch");
        a[n++] = (char *)(pitch.c_str());
    }

    if (!mouth.empty())
    {
        a[n++] = (char *)("-mouth");
        a[n++] = (char *)(mouth.c_str());
    }

    if (phonetic == true)
        a[n++] = (char *)("-phonetic");

    if (!pitch.empty())
    {
        a[n++] = (char *)("-pitch");
        a[n++] = (char *)(pitch.c_str());
    }

    if (!speed.empty())
    {
        a[n++] = (char *)("-speed");
        a[n++] = (char *)(speed.c_str());
    }

    if (!throat.empty())
    {
        a[n++] = (char *)("-throat");
        a[n++] = (char *)(throat.c_str());
    }

    a[n++] = (char *)samBuffer;
    sam(n, a);
};

void iecVoice::write()
{
    // act like a printer for POC
    uint8_t n = 40;
    uint8_t ck;

    memset(sioBuffer, 0, n); // clear buffer

    // append sioBuffer onto lineBuffer until EOL is reached
    // move this logic to append \0 into write
    uint8_t i = 0;
    while (i < 40)
    {
        if (sioBuffer[i] != EOL && buffer_idx < 121)
        {
            lineBuffer[buffer_idx] = sioBuffer[i];
            buffer_idx++;
        }
        else
        {
            lineBuffer[buffer_idx] = '\0';
            buffer_idx = 0;
            sam_init();
            // clear lineBuffer
            memset(lineBuffer, 0, sizeof(lineBuffer));
            break;
        }
        i++;
    }
}

// Status
void iecVoice::status()
{
    // act like a printer for POC
    uint8_t status[4];

    status[0] = 0;
    status[1] = lastAux1;
    status[2] = 15; // set timeout > 10 seconds (SAM audio buffer)
    status[3] = 0;

    //bus_to_computer(status, sizeof(status), false);
}

device_state_t iecVoice::process()
{
    Debug_printv( "payload[%d]", payload.c_str());
    
    // act like a printer for POC
    switch (cmdFrame.comnd)
    {
    case 'P': // 0x50
    case 'W': // 0x57
        write();
        lastAux1 = cmdFrame.aux1;
        break;
    case 'S': // 0x53
        status();
        break;
    default:
        break;
    }

    return device_state;
}

#endif /* BUILD_IEC */