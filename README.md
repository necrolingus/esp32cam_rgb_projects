# esp32cam_rgb_projects
Various esp32cam projects to convert captured images to RGB

### Main issues
When converting a captured image on the esp32cam itself to RGB, it is pretty much just shades of grey.

The resolution and quality also cannot be too high because I want to keep processing below 100ms, preferably around 40ms, so I used, for the most part QVGA and a JPEG quality of between 10 and 12.


### Solution with the most promise
esp32cam_fm2rgb888 is still grey, but at least the RGB values change drastically depending on what the camera sees, albeit still just shades of grey

It is also the slowest, at around 100ms when dividing the image into 9 blocks (3 horizonal and 3 vertical)


### Fastest solution
All the examples using JPEGDecoder. It runs about (if I remember correctly) about 30ms to 50ms when dividing the image into 9 blocks (3 horizontal and 3 vertical)

But, its RGB values also do not really change like esp32cam_fm2rgb888 does. It always stays in the region of around 150 for R, G, and B. So just grey again...


### My chosen solution
Seeing the most promising solution (esp32cam_fm2rgb888) takes 100ms, which is about double what I wanted, I just decided to send the image over the network to a flask server.

The server will do the processing in Pillow, and send back the RGB values for each of the 9 blocks I want. This has the added benefit of potentially adding saturation, hue, etc and fast speeds.

The flask code to spin up a quick server is also included.
