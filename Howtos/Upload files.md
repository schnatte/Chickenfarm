#Upload files
for maintenance reasons files can be uploaded to the PI via SSH. This facilitate updates and editing existing code.
After an upload of the C code a compilation is mandatory.

After a long time not touching and updating the PI it is recommended to update && upgrade the system. The following command can be used over ssh.
`sudo apt-get update && apt-get upgrade.`

`scp /Users/danielkettnaker/Projekte/ChickenFarm/MAX44009/test.c pi@192.168.0.119:/home/pi/`

`scp //Users/danielkettnaker/Projekte/ChickenFarm/www/chickenfarm/css/format.css pi@192.168.0.119:/var/www/html/chickenfarm/css/
`
