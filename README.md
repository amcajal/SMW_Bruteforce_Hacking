# Super Mario World Brute Force Hacking

Hacking Super Mario World (SNES) color palette using Brute Force approach

---

> When in doubt, use brute force
>       [- Ken Thompson](https://es.wikipedia.org/wiki/Ken_Thompson)


The goal of this article is to show a funny application of the "when in doubt,
use brute force" motto, as well as to demonstrate to which level automation
tools can be used.

---

## Index
[Introduction](#introduction)

[Hack overview](#hack_overview)

[Required tools](#required_tools)

[Main script](#main_script)

[Swapping Nibbles](#swapping_nibbles)

[Taking screenshots](#taking_screenshots)

[Brute force process](#bruteforce_process)

[Result](#result)

---

## Introduction

This is a screenshot of SNES game Super Mario World. Mario shows it’s universally
famous suit: red and blue.

![](https://github.com/amcajal/SMW_Bruteforce_Hacking/blob/master/images/smw_screenshot.png)

Goal is to change Mario's suit colors -to yellow and purple for example, 
to make it like Wario.

This could be done using [Lunar Magic](https://fusoya.eludevisibility.org/lm/index.html), 
a software tool to hack Super Mario World ROMs, among others. And it works pretty well.

But it could be done also modifying directly the bytes of the ROM itself. 
The assumption is that everything about the game -physics, logic, sprites,
color palettes- is contained in the ROM. So the bytes specifying the color
of Mario's suit are also contained in the ROM. **This is, indeed, true.**

Which ones? The bytes implementing the color palette could be found using
a resource like [Super Mario World Central](https://www.smwcentral.net/), 
where there are several posts explaining the memory map of SMW ROMS. 
Other resources could be checked also.

But let’s imagine SMWC does not exist, nor other resource. Only assumption is
that: every inch of the game must be contained in the ROM, including the color
palette. So if the correct byte is changed, Mario's suit colors will change.
I.e: supposing Mario's suit colors are encoded as RGB values, there will
be a place in the ROM where the sequence 0xFF 0x00 0x00 will appear. And changing
the byte 0xFF to 0x33, for example, will change the color of Mario.

![](https://github.com/amcajal/SMW_Bruteforce_Hacking/blob/master/images/smw_rom_hexcode.png)

Those bytes are initially unknown (their memory addresses), so **all of them need
to be tested** -this is the brute force approach.

## Hack_overview

Process is straightforward:
- Change first byte of the ROM
- Open ROM with and emulator, until Mario appears on screen.
- Its color changed? 
    - If so, one of the bytes of the color palette has been found.
    - If not, reverse change, and advance to next byte.

Doing this manually would work, but it would take a lot of time: considering the ROM
has 524288 bytes or so (depends on the version)
and a human (or at least, the author) can do this process 
(change byte, check on emulator, and revert change)
in 1 minute, it will take 524288 bytes * 1 min/byte = near 1 years 
to test all bytes, in the best of cases.

So this process is automated.

## Required_tools

The automation has been performed in different GNU/Linux OS, and requires the installation
of the following tools:
- [Imagemagick](https://imagemagick.org/index.php): to take screenshots, cut them, compare with others...
- [xdotool](https://www.semicomplete.com/projects/xdotool/): to automate mouse clicks and keyboard inputs, among other things
- [SNES9x](https://www.snes9x.com/): to run the game ROM
- [gcc](https://gcc.gnu.org/): to compile the required C code
- [bash](https://www.gnu.org/software/bash/): to run the scripts that performs the "hack" itself

And optionally:
- [Make](http://www.gnu.org/software/make/): to automate C code compilation
- [wxHexEditor](http://www.wxhexeditor.org/): to mess with the ROM bytes manually

And of course, the ROM of Super Mario World (SNES). **WARNING: all this operations
are performed using the USA version of the ROM.** Link to ROM is not provided
to avoid legal problems.

## Main_script

The "hack_smw_rom.bsh" script is the "main" script or "entry point" 
of the Super Mario World ROM
hacking process. It automates the "human procedure" to discover
which bytes contains the color palette of Mario.
Basically, it iterates over all the bytes, changing them according to the
"nibble_swapper" rules (later explained); then takes a specific screenshot of the game
where Mario appears, and checks if Mario's suit changed (all of this
performed by "take_screenshot.bsh" script); finally, these results are
stored in a log for further research.

The content of the script is as follows:

```bash
#!/bin/bash

ROM=./smw.sfc
NIBBLE_SWAPPER=./nibble_swapper.exe
SCREENSHOT_SCRIPT=./take_screenshot.bsh
RESULTS_FILE=./smw_hacking_results_$(date +%s).txt

echo "SMWRH: Testing all Super Mario World ROM bytes"

# Get number number of ROM bytes.
number_of_bytes=$(ls -l $ROM | awk '{print $5}')
echo "Number of bytes to test: $number_of_bytes"

# Iterate over all ROM bytes. Index starts at 0, because first call
# to fseek shall use a zero offset (that is, first byte on the ROM).
# See "nibble_swapper.c" source code for more details.
for ((index=0;index<$number_of_bytes - 1;index++))
do
	# Yes, this "echo" is incorrect, but its only purpose
	# is to provide feedback to the user.
	echo ">>> SMWRH: Testing byte $index out of $number_of_bytes"

	# Change the byte value
	$NIBBLE_SWAPPER $ROM $index

	# Take screenshot and check the result
	bash $SCREENSHOT_SCRIPT >> $RESULTS_FILE

	# Revert byte value change
	$NIBBLE_SWAPPER $ROM $index
done

echo "SMWRH: Testing all Super Mario World ROM bytes --- COMPLETE"
```

The script itself is very straightforward. Key parts here are the call to
the "nibble_swapper.exe" and the call to "take_screenshot.bsh" script.

## Swapping_Nibbles

**nibble_swapper.exe**, as it name implies, swaps the High nibble and the Low nibble 
of a byte, following certain rules:

    Rule 1) If both nibbles are equal, each nibble is increased by 0x08, applying
            and "overflow" operation if the result is higher than 0xFF. 
    Rule 2) If nibbles are different, the high nibble becomes the lower, 
            and vice versa.

Examples (all in hexadecimal, because it is easy to see):
- Byte is 0xE3; after swap, it becomes 0x3E. Swap again, it becomes 0xE3 (initial value).
- Byte is 0x00; after "swap", it becomes 0x88. "Swap" again, and it becomes 0x00.
    - This happens because high nibble overflows to 0x"1"0, same as low nibble. But we ignore the
    overflow. 
    Think in this way:
    Possible hexadecimal values: 0 1 2 3 4 5 6 7 8 9 A B C D E F
    If the value is D, for example, and we add to it 0x03, we dont want a 2-digit
    hexadecimal, we still want a 1-digit hexadecimal, so we consider that the
    list of values are a "closed loop", so D becomes E, E becomes F, and F goes
    back to 0.)
- Byte is 0x24; after "swap", it becomes 0xAC. "Swap" again, it becomes 0x24.

These rules pursue two things:
- One, is to allow the recovery of the initial byte value with the same operation.
    No need to save the initial value in a file, or whatever. The resulting
    code is simpler and easier to understand.
- Two, is to ensure a "different enough" value, resulting in a distinguishable
change in Mario's suit colors, and thus, a higher chance of detecting such
change by automatic means (image comparison). I.e: back to the assumption
made earlier, if colors are encoded as RGB values, and one of the Mario's
colors is 0xEE, changing byte to 0xEF (increasing its value by one) may not
be enough for tools to detect such small change in the color.

![](https://github.com/amcajal/SMW_Bruteforce_Hacking/blob/master/images/smw_color_test.png)

The source code implementing such rules are as follows:
```C
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

unsigned char swap_nibbles(unsigned char byte_to_swap)
{
    unsigned char new_value = 0x00;
    unsigned char high_nibble = 0x00;
    unsigned char low_nibble = 0x00;

    high_nibble = (byte_to_swap & 0xF0) >> 0x04;
    low_nibble = (byte_to_swap & 0x0F);

    if (high_nibble == low_nibble)
    {
        low_nibble = (low_nibble + 0x08) & 0x0F;
        high_nibble = low_nibble;        
    }

    new_value = ((new_value + low_nibble) << 4) + high_nibble;
    return new_value;
}


int main(int argc, char** argv)
{
    if (argc != 3)
    {
        printf("Usage: nibble_swaper.exe <file> <offset>\n");
        return 1;
    }

    FILE* file_worker = NULL;
    file_worker = fopen(argv[1], "r+");
    if (file_worker == NULL)
    {
        printf("Cannot open file %s\n", argv[1]);
        fclose(file_worker);
        return 1;
    }
   
    unsigned long offset = 0;
    offset = atol(argv[2]);

    rewind(file_worker);
    fseek(file_worker, offset, SEEK_SET);

    unsigned char byte_to_change = 0;
    fread(&byte_to_change, sizeof(unsigned char), 1, file_worker);
    unsigned char new_byte = swap_nibbles(byte_to_change);
    ungetc(byte_to_change, file_worker);    
    fwrite(&new_byte, sizeof(unsigned char), 1, file_worker);

    return 0;
}
```

The ".exe" can be generated running make, or either calling gcc manually as
follows:
```
$> gcc -Wall -g -O0 -o nibble_swapper.exe nibble_swapper.c
```

## Taking_screenshots

Script **take_screenshot.bsh**, on the other hand, performs all the "human operations" required
to check if the byte that has just been changed by "nibble_swapper.exe" is one of those containing
the color palette. Here is where the majority of the required tools (ImageMagick, xdotool, snes9x)
are used.

First, the content of the script, and then a more detailed explanation:

```bash
#!/bin/bash

ROM=./smw.sfc
SCSNAME=mario.png # SCS stands for ScreenShot. SS was not appropriate
CUTNAME=mario_cut.png
BASECUT=mario_base.png
BYTE_INDEX=$1 # This, provided by other script, is just for results printing

#Launch emulator and fork
snes9x $ROM & >> /dev/null

#Get ID of the emulator window
sleep 1
window_id=$(xdotool search --name MARIOWORLD)

#Wait required seconds to send Enter keys
sleep 6

#Send Enter keys to the emulator to start game
# We send 5 times an Enter key, to ensure
# the start of the gameplay
for i in {1..5}; do
	xdotool key --window $window_id Return
	sleep 0.5
done

#Wait required seconds for Mario to appear in the image
sleep 4

#Take screenshot
import -window $window_id $SCSNAME

#Close emulator
for in in {1..3}; do
	xdotool key --window $window_id Escape
	sleep 1
done

#Cut image, and get only the required part
convert $SCSNAME -crop 40x50+250+340 $CUTNAME

#Compare with the base cut
#Here is where mathematics starts: Using the Average Error
# (ae) metric, we want to be informed when an ae bigger than
# 0 but less than 300 is detected. 
dif=$(compare -metric ae $CUTNAME $BASECUT result.png 2>&1 | awk '{print $1}')
if [[ $dif>0 && $dif<=300 ]]
then
	# This provides the core information: the byte that causes such AE"
	echo "$BYTE_INDEX:$dif"
fi
```

As explained in the beginning, a human, in order to decide if a byte contains a color
of the color palette, will launch the game, then wait until Mario appears on screen,
and then will look carefully to Mario's suit to detect any change in its original colors.

This is exactly what the script does, but in a fully automatic way using the
mentioned tools:

- First, snes9x is launched.
- A human will now look only to the emulator window. To reproduce this with the automatic tools,
	the ID of the emulator window must be obtained. And this is performed with the
	xdotool command "search". 
	```
	$> window_id=$(xdotool search --name MARIOWORLD)
	```
	Option "name" is used, because is very easy to find the
	emulator window: the one displaying the string "MARIOWORLD" on it.
    
	![](https://github.com/amcajal/SMW_Bruteforce_Hacking/blob/master/images/smw_snes9x_emu.png)

	The returned numeric ID allows to perform further operations ONLY in such window.

- Now, a human would look at Mario's suit, and check for color differences. In automatic
	mode, this is more complex. A human would detect the differences at any moment
	where Mario is on screen. To perform this automatically, image comparison is
	performed, and to compare images, it must be ensured that the very same picture
	is always taken (otherwise, false positives will appear). Luckily, there is
	a perfect moment in Super Mario World to be used as "image checkpoint", and it
	is when the "Welcome! This is Dinosaur Land" message appears.
    ![](https://github.com/amcajal/SMW_Bruteforce_Hacking/blob/master/images/swm_mario_welcome_screen.png)
	At this exact moment of the game, Mario is visible, and everything else is static.
	Reaching that point of the game is trivial also, so its the perfect place to take
	the screenshot.

- Next operations are straightforward: simulate the "Return" key press with xdotool to
	reach the previous mentioned game state, take screenshot of the emulator window
	(using the numeric ID, obtained with xdotool also) with the command "import" from
	ImageMagick, and close the emulator.

- What it rests, is to compare the images. The whole screenshot could be compared, but such
	would be nonsense: the only thing to be compared shall be Mario. Thus, the screenshot is
	"cut", obtaining only the pixels where Mario appears. This is done using the command
	"convert" from ImageMagick tool, with option "crop". 
	```
	convert $SCSNAME -crop 40x50+250+340 $CUTNAME
	```
	The input parameter is the
	image coordinates and the size of the new image selection. Both values (coordinates
	and required cut size) can be obtained by opening the screenshot with GIMP, placing
	the mouse on the image in the desired place (coordinates will appear on GIMP),
	cutting the selection, and then checking the size of the selection (also on GIMP).
	In other words: coordinates and size can be obtained by performing the cut operation
	manually with GIMP, and then transforming the data to ImageMagick operations.
	What this does is turning the "welcome message" screenshot into this: ![](https://github.com/amcajal/SMW_Bruteforce_Hacking/blob/master/images/smw_mario_base.png)
	This is the "mario_base.png" image. This is the image that will be
	compared to, so it must be generated after the "hack_smw_rom.bsh" script is executed.
	IMPORTANT: screenshot and "crop"s should be PNG format. Lossy compression formats,
    even when they can be used, are harder to process.

- And finally, to compare the images, command "compare" from ImageMagick tool is used,
	using "metric" option, and "ae" as input parameter.
	```
	dif=$(compare -metric ae $CUTNAME $BASECUT result.png 2>&1 | awk '{print $1}')
	```
	Long story short (although not
	very accurate), "Average Error" (ae) metric returns a value telling how many pixels
	have changed from one pic to another. If value is 0, images are identical. Otherwise,
	something has changed.
    Command also generates a new image with the differences found between the
    two input images. For example:
    Comparing this ![](https://github.com/amcajal/SMW_Bruteforce_Hacking/blob/master/images/smw_mario_base.png)
    with this ![](https://github.com/amcajal/SMW_Bruteforce_Hacking/blob/master/images/smw_mario_mod_cut.png)
    (note the change in Mario's cap) give
    this ![](https://github.com/amcajal/SMW_Bruteforce_Hacking/blob/master/images/smw_diff_result.png) as a result.
	
- The goal is to discover which bytes contains the Mario's color palette, ignoring
    the rest. that’s the reason only "average error" values between 0 and 300
    are reported. Otherwise, when the bytes containing the background palette
    (or other palettes) are changed, they will be reported also, producing
    noise.
    ```
    if [[ $dif>0 && $dif<=300 ]]
    then
        # This provides the core information: the byte that causes such AE"
        echo "$BYTE_INDEX:$dif"
    fi
    ```
    Once again, the value 0 and 300 is chosen following manual experiments:
    taking the "base_cut" image, Mario's cap is colored by hand in GIMP, then
    compared with the base image. This gives a result of 176 ae. Considering the
    cap is not the biggest element in Mario, a kind of "extrapolation" is made
    to choose 300 as the biggest possible ae.
    
A last note about the script: number of keystrokes, delays duration 
and screenshot coordinates may vary 
from one system to another (i.e: depending at which speed 
the emulator is set, the screen resolution options, etc)
The ones in the script are those that work in the author's computer,
taking into account that emulator is configured to run at the same speed
than the original system (the physical SNES).

## Bruteforce_process

With everything set and done, "hack_smw_rom.bsh" can be launched. 
```
$> bash hack_smw_rom.bsh
```
A single instance
of the script will do the work. However, several instances of the script can be launched: in different virtual
machines, distributed across different computers, etc. The following line
from "hack_smw_rom.bsh" is the only one that would need to be changed:

```
for ((index=0;index<$number_of_bytes - 1;index++))
```

Modifying the index start and index end, two computers for example could test
at the same time half of the bytes. Another possible scenario is with 32 [Raspberry Pi](https://www.raspberrypi.org/)
 or 32 VM;
in both cases, if one instance is launched in each Rpi or VM, time will
be reduced to 11 days, considering each byte still taking 1 minute. Increasing
operations speed to take only 30 seconds per byte, all bytes would be tested
in almost 6 days.

## Result

Anyways, with several instances or just a single standalone execution, the
"./smw_hacking_results_$(date +%s).txt" log (the results file) will be
populated with something similar to this (the output is the one
generated by **take_screenshot.bsh**, and its <byte_index>:<ae>)

```
13519:179
13521:10
13523:179
13524:43
13526:153
```

**Eureka! Bytes containing the color palette have been found.**

A clear example is the byte 13519, corresponding to memory address 0x34CF, which
establishes Mario's cap color. In fact, this result can be validated by opening
the ROM (in its original state) with an hex editor, then browsing the 0x34CF byte,
changing it manually to the desired value, and opening the modified ROM with
snes9x.

![](https://github.com/amcajal/SMW_Bruteforce_Hacking/blob/master/images/swm_mario_yellowcap.png) As it can be seen, now Mario's cap is partially yellow.

With this results, its easy to determine at least the start of the bytes
containing the color palette. A couple more of tests, and it can be established
that the target bytes **starts at address 0x34CC**:

![](https://github.com/amcajal/SMW_Bruteforce_Hacking/blob/master/images/smw_palette_bytes.png)

Knowing the bytes containing the color palette, they can be modified at wise.
It does not take long until Mario's is dressed as Wario:

![](https://github.com/amcajal/SMW_Bruteforce_Hacking/blob/master/images/smw_mario_as_wario.png)

**Mission accomplished.**
