AIOMIC(1)                   General Commands Manual                  AIOMIC(1)

NAME
     aiomic - graphical microphone tester

SYNOPSIS
     aiomic <device>

DESCRIPTION
     aiomic is a curses(3) frontend to test NetBSD audio(4) recording devices.

     aiomic continually captures input from the device and measures the Root
     Mean Square of the collected samples.

     aiomic uses the existing configuration of the device and displays the RMS
     as a percentage of the maximum value based on the device's precision.

     The following options are available:

     <device>
             The recording audio device. Write access to the device is
             required.

NAVIGATION
     aiomic uses the following keys for navigation:

     R       View the RMS of the recorded audio.

     I       View configuration details of the device.

     Q       Exit the application.

EXAMPLES
           aiomic /dev/audio2

SEE ALSO
     audio(4)

AUTHORS
     aiomic was written by Zac Brown <gummybuns@protonmail.com>.

NetBSD 10.0                     April 18, 2025                     NetBSD 10.0
