# MCP9600-Arduino-Reflow
An Arduino example of using an MCP9600 with an OLED display to control a reflow oven

Very simple and is really just here as a reference example. It completely relies on the thermal characteristics of my particular oven to give a decent profile, so if you do try to use this for reflow soldering you *must* test and adjust to make it work with your oven!

For the display, I use one of these: https://www.amazon.ca/gp/product/B01N78FUH7

The address for the MCP9600 sensor is set on line 26 of Reflow.ino (not convenient, I know, but I wrote this as an experiment really, but it worked just fine so I stopped changing it)
