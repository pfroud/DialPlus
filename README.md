# Dial Plus


A [Pebble](https://www.pebble.com) watchface based on [Priyesh Patel's](http://priyesh.me/) superb watchface Dial ([source](https://github.com/ItsPriyesh/Dial), [appstore](https://apps.getpebble.com/en_US/application/56512a8ba69d971f08000038)).

<p align="center" style="text-align: center">
![Dial on a Pebble Steel](readme_images/pebble_steel.png)
</p>

Dial Plus shows the time simply. When you flick your wrist, battery information and the date appear.

<p align="center" style="text-align: center">
![Layers of Dial Plus watchface](readme_images/layers.png)
</p>

## Issues addressed

### Clock alignment

After 6:00, the needle doesn't correctly line up with the ticks on the dial:

<p align="center" style="text-align: center">
![Incorrect alignment after 6:00](readme_images/alignment.png)
</p>

The original dial has a background image ([`background.png`](https://github.com/ItsPriyesh/Dial/blob/master/resources/background.png)) which is positioned so the needle points to the correct time.

There are 10 minutes between ticks. Both the needle and the ticks are 2 pixels wide, so there should be 20 pixels between ticks. However, there are only 19 pixels:

<p align="center" style="text-align: center">
![Not enough space between ticks](readme_images/spacing.png)
</p>



Here's Priyesh's code to compute the offset of the background image, from [line 59](https://github.com/ItsPriyesh/Dial/blob/master/src/Dial.c#L57-L59):

```
static void draw_clock(struct tm *tick_time) {
    const int64_t mins_in_day = 24 * 60;
    const int64_t mins_since_midnight = tick_time->tm_hour * 60 + tick_time->tm_min;
    const int64_t background_x_offset = mins_since_midnight * BACKGROUND_WIDTH * 2 / mins_in_day;
    ...
```

All the terms of `background_x_offset` except `mins_since_midnight` are constant and represent the number of pixels per minute. The background goes from 0 to 12 so twice that width is one day.

Since the ticks are the needle are 2 pixels wide, there should be 2 pixels per minute. The background is `#define`d to [1366 pixels](https://github.com/ItsPriyesh/Dial/blob/master/src/Dial.c#L3), so the result is 1366 × 2 / 1440 = 1.8972.

Since those are integers (and the Pebble only has floating-point emulation), information is lost.


### Font licence

[`Medium.ttf`](https://github.com/ItsPriyesh/Dial/blob/master/resources/Medium.ttf) is [Helvetica Neue](https://www.linotype.com/1245395/neue-helvetica-family.html), which is not free. The file has its extremely long copyright metadata intact:

>Part of the digitally encoded machine readable outline data for producing the Typefaces provided is  copyrighted © 2003 - 2006 Linotype GmbH, www.linotype.com. All rights reserved. This software is  the property of Linotype GmbH, and may not be reproduced, modified, disclosed or transferred without the express written approval of Linotype GmbH. Copyright © 1988, 1990, 1993 Adobe Systems Incorporated. All Rights Reserved. Helvetica is a trademark of Heidelberger Druckmaschinen AG, exclusively licensed through Linotype GmbH, and may be registered in certain jurisdictions. This typeface is original artwork of Linotype Design Studio. The design may be protected in certain jurisdictions.

(That was surprisingly hard to get that...I used used the [`Get-FileMetaData`](https://gallery.technet.microsoft.com/scriptcenter/get-file-meta-data-function-f9e8d804) script by IamMred on the Microsoft Script Center Repository.)

I replaced Helvetica with [Raster Gothic](https://developer.pebble.com/guides/app-resources/system-fonts/#raster-gothic) from the Pebble SDK.

<p align="center" style="text-align: center">
![Comparison of Helvetica Neue and Raster Gothic](readme_images/font_compare.png)
</p>



## Features added

### Battery display

When you shake your wrist, Dial Plus shows your watch's battery status.

<p align="center" style="text-align: center">
![Battery display](readme_images/battery.png)
</p>

### Calendar events (incomplete)

I wanted to show upcoming calendar events on the dial, using one of these styles:

<p align="center" style="text-align: center">
![Possible event mark styles](readme_images/event_marks.png)
</p>



I know this is possible without a companion app because [My Calendar](https://apps.getpebble.com/en_US/application/5425871e2375286a35000124?dev_settings=1) by Stanfy and [Calendar Cards](http://apps.getpebble.com/en_US/application/55ad0a036749cdddc6000075?dev_settings=1) by Ester Sanchez both do it.

It turns out it's a real pain in the ass and I have decided not to do it. There are [at least](https://github.com/pebble/slate) [two](https://developer.pebble.com/blog/2016/06/24/introducing-clay/) frameworks for doing watchface configuration, both poorly documented and neither supported by Cloudpebble (the online SDK).

### Organization

[`Dial.c`](https://github.com/ItsPriyesh/Dial/blob/master/src/Dial.c) was a single file (149 lines), which is not too bad. Since I added a lot of functionality, I split it up into files for [`main`](https://github.com/pfroud/DialPlus/blob/master/src/main.c), [`drawing`](https://github.com/pfroud/DialPlus/blob/master/src/drawing.c), [`animation`](https://github.com/pfroud/DialPlus/blob/master/src/animation.c), and [`handlers`](https://github.com/pfroud/DialPlus/blob/master/src/handlers.c).



&nbsp;

*The top picture of the Pebble Time Steel in Silver from the [store](https://www.pebble.com/buy-pebble-time-steel-smartwatch).*