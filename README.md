# Dis-Play-Thermal-printing-display
### Recipte printer notes to star your day!
A small hardware and firmware project to print daily weather report to your life!

## What is this?

How can we get away from our screens and return to basics. Especially in early mornings we should not have to stare at our phones first thing to see how the day will turn out. This printer will instead print out the weather for you (and maybe wake you up with its little whirr) 

The future goals:

- Design a nice case
- Expand to print transit times and maybe evensend messages



## Repository content
| Path | Contents |
|------|----------|
|[cad](cad.md)|	Future CAD|
|[designer/](designer/)| The recipte designer HTML|
|[firmware/](firmwear/)| Arduino file for on device weather fetch|
|[refrences/](refrences/)| Refrences|
|[BOM.md](BOM.md)|	Bill of materials Halflife|
[Thermal printer v1.csv](Thermalprinterv1.csv )|	Full bill of materials|
|[JOURNAL.md](JOURNAL.md)|	Devlog, synced from Half Life|
|[LICENSE](LICENSE)|	MIT licence|
|[README.md](README.md)| You are here|


## BOM 
|Reference Designators|Quantity|Description                                               |FIELD4|Value |
|---------------------|--------|----------------------------------------------------------|------|------|
|C1, C3               |2       |Capacitor - Generic                                       |      |100uF |
|C2                   |1       |Capacitor - Generic                                       |      |1000uF|
|D1                   |1       |Intelligent control LED integrated light source           |      |WS2812|
|J2                   |1       |USB Type C 16Pin Connector                                |      |      |
|JP1                  |1       |Pin Header                                                |      |      |
|S1                   |1       |Tactile Switch Contact                                    |      |      |
|U1                   |1       |Seeed Studio XIAO Series Through-hole Footprint and Symbol|      |      |
|P1                   |1       |Thermal printer                                           |      |      |

## Help

Contact author 

## Author

Lucas Klemmer  
klinksolutions22@gmail.com

## License
MIT LIcense

If anyone ends up reproducing this project  or making it even better I would be happy to see your improvements and add them to the official release.
