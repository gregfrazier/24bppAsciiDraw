# 24bppAsciiDraw
Draw 24bpp BMP files in ASCII (Windows GDI)

Compile with cl linking user32.lib and gdi32.lib.

Something like this:

cl asciidraw.cpp "C:\Program Files\Microsoft SDKs\Windows\v6.0A\Lib\user32.lib" "C:\Program Files\Microsoft SDKs\Windows\v6.0A\Lib\Gdi32.lib"

Then drag and drop a BMP file into the .exe or supply at command line. Displays the BMP in full-color ascii.

You can change the precision (resolution) of the output by changing BLOCK_WIDTH and BLOCK_HEIGHT. Make sure it's a multiple of 2 and it doesn't need to be matching.

Simple breakdown of how it works:
- breaks the image up into pieces of BLOCK_WIDTH x BLOCK_HEIGHT
- calculates a luminance value of the block to figure out which ASCII character to use
- gets an average of the block color and then uses that color to draw the character to the screen.
