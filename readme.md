mfsplash - splash image laptop meters for battery, volume, and brightness

Images are dark greyscale, rounded corners, mild gradient, and a crazy pattern
that may or may not explain why its called M F Splash.......


I claim copyright on this as my own work, but feel free to hack at it/redistribute
it as I'm putting it under the MIT License.

INSTALLATION COMES WITH ABSOLUTELY NO WARRANTY

To Install:
make
sudo make install

installs to /var/lib/mfsplash and /usr/local/sbin -- currently not configurable sorry


Then you can use it like so

	mfsplash /var/lib/mfsplash/icon/blah.png 50 - splash at 50% with this icon

	battery - shows current battery level and charging/not (require acpi)

	brightness [up|down] - change screen brightness (with sys backlight files)

	volume [up|down|int] - change master volume (requires amixer)

