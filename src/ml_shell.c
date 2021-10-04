
#include "ml_shell.h"

// void MShell::setup (void)
// {
//         // // Setup callbacks for SerialCommand commands
//     cli.addDefaultHandler(unrecognized);     // Handler for command that isn't matched  (says "What?")

//     cli.addCommand("on",LED_on);             // Turns LED on
//     cli.addCommand("off",LED_off);           // Turns LED off
//     cli.addCommand("hello",SayHello);        // Echos the string argument back
//     cli.addCommand("p",process_command);     // Converts two arguments to integers and echos them back

//     cli.addCommand("$", listDirectory);
//     cli.addCommand("iec", iecCommand);

//     cli.addCommand("cat", catFile);
//     cli.addCommand("help", showHelp);

// }

// void MShell::service ( void )
// {
//     cli.readSerial();
// }