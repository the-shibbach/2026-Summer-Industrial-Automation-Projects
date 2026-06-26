I'm ngl all the tutorials online are out of date so this took me like over 10 hours to figure out. So. 

https://www.youtube.com/watch?v=0IJQvJP59oA&t=137s
this guy gets you most of the way there. but it is wrong. 

So, you need to download
- OpenPLC Editor
- OpenPLC Runtime
- Docker Desktop
- FactoryIO

Install it the usual way.

For Docker Desktop.
Run docker start openplc-runtime in Windows Powershell.

Open up OpenPLC Runtime (v4).

In OpenPLC Editor have 2 variables.
- Sensor, Local, BOOL, %IX0.0
- Conveyor, Local, BOOL, %QX0.0
code in structured text: Conveyor := Sensor;

Device -> Configuration DEvice: OpenPLC Runtime v4.
IP Address is your own IP address (I'm ofc not gonna put mine here!)

Remote Device: 
TCP/IP, IP Address is host.docker.internal, Port 1502, SlaveID 1.
IO tag mapping. Sensor (Digital input discrete input) -> read discrete inputs (FC2)
Conveyor (Digital output single coil) -> write single coil (FC5).

FactoryIO:
Choose simple A to B scene. Go to driver -> configuration.
Host: Choose your IP.
Port: 1502
Slave ID: 1
Network adapter is your own. 