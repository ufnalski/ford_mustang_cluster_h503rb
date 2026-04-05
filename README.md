# Ford Mustang VI (S550) 2015+ instrument panel cluster hacking[^1] (STM32H503RB)
 An STM32 HAL example of communicating with the IPC. Tested on three different versions of the cluster: GR3T, FR33, and JR3T.

[^1]: Cambridge Dictionary: the activity of riding on a horse in the countryside for pleasure :slightly_smiling_face:

# Motivation
Engaging classrooms/labs. Hands-on training on the CAN bus communication. You can buy a decent IPC starting from 10 EUR or even less. You will see such examples in my other submissions later on (e.g. Peugeot 207, Citroen C5 or Ford Focus Mk3). Here I deal with Ford Mustang VI IPCs. I've bought mines from [PinyaStore](https://allegro.pl/uzytkownik/PinyaStore) and [GoTradeMCh](https://allegro.pl/uzytkownik/GoTradeMCh) for under 30 EUR mark each. Why IPCs in CAN bus training? We need at least two nodes to play with the bus (except the loopback-mode scenario). A node built from scratch using a Nucleo board, a CAN transceiver, a stepper motor with a driver (to have some movement, which is always more fun), and a breadboard with a power supply would cost you around 30 EUR. For the same money you can play with a piece of art such as GR3T featuring analog gauges with a controllable RGB backlight, a TFT color display, plenty of LED indicators, and a speaker. It's gonna be fun to turn all that stuff alive. Using such training aids is definitely fun for me and I hope that this fun can be contagious. And you can learn a lot interacting with production devices, e.g. that selecting CAN segments does matter.

![Ford Mustang (S550) 2016 instrument panel cluster](/Assets/Images/ford_mustang_home_lab.JPG)

# How to
Probably the most convenient way would be to get your hands on a working car and sniff the bus. If this is not the case for you, as was not the case for me with Ford Mustang, still we have some options.

First of all, choose an IPC that has been hacked by other hobbyists. And by hacking I don't mean here any illegal activity. We are not going to gain access to any protected data stored in the cluster to change it, such as the odometer. I mean completely legal reverse engineering done by hobbyists who enjoy connecting production IPCs to e.g. computer games/simulators via [SimHub](https://www.simhubdash.com/).

Second, use a brute force method to find some more frames and there are always more of them. Pick devices that use 11-bit identifiers. The 29-bit IDs are impractical to be hacked using the brute force method. Let me demonstrate why. To spot a movement or a LED blink by human eye a 100 ms visual event is sufficient (at least my experiments demonstrate that). Don't confuse it with a reaction time which is an order of magnitude slower. You will miss around 10 frames between the frame that switched on the diode and the one that you actually stopped at by pressing the button. There are 2048 different 11-bit identifiers. We need then less than 4 minutes to send a selected pattern with all the possible IDs. The common choices for the pattern are 0x55 and 0xAA. Note that the message has to have a specific length to be accepted by the node (hardware filters check the ID, but on the software side we often verify also the length before we act on the message content). CAN 2.0B[^2] limits the length to 8 bytes. It is not uncommon for IPCs to use not only 8-byte long messages, but also shorter ones, e.g. 7-byte, 6-byte or even 5-byte long ones. 4 minutes times 2 patterns times 2 lengths (I would suggest going initially for 8-byte and 7-byte long messages) gives 16 minutes. Bearable. Doable when accompanied with sipping your favorite beverage :coffee: This is not to encourage you to repeat this experiment multiple times. It wouldn't be productive in most cases. Just find a single undocumented frame and play with it to feel satisfaction from the discovery. And then move to more advanced stuff, e.g. CAN-TP[^3], OBD2 simulators or OBD2 scanners[^4] - more on that in future submissions. Oh, and for the 29-bit IDs this would take close to 2 years.

[^2]: CAN FD is already knocking on the vehicle door. The next one is CAN XL. Both of them support much longer payloads. The brute force method will be rendered by them impractical. Enjoy CAN 2.0B while you can :wink:
[^3]: RAND_ASH: ["I hacked my old Mercedes instrument cluster to display custom text"](https://www.youtube.com/watch?v=iKwVWPU5P0I)
[^4]: [OBD2 PID Overview](https://www.csselectronics.com/pages/obd2-pid-table-on-board-diagnostics-j1979)

# Track Apps<sup>TM</sup>
Some versions of that IPC have Track Apps<sup>TM</sup> installed, or maybe all of them but I was able to activate it only in some of them. The app visualizes e.g. acceleration/deceleration of the car. Hacking this app is my small addition to the overall set of frames published in the sources I'm aware of.

![Ford Mustang (S550) 2016 instrument panel cluster](/Assets/Images/ford_mustang_university_lab.jpg)

# Missing files?
Don't worry :slightly_smiling_face: Just hit Alt-K to generate /Drivers/CMCIS/ and /Drivers/STM32H5xx_HAL_Driver/ based on the .ioc file. After a couple of seconds your project will be ready for building.

# Tools
Some IPCs also sent their own frames. For example, when you navigate the menu on your IPC it may distribute your choices to other components of the system such as the center console. In our case the instrument cluster shares the information about your preferences regarding e.g. temperature and distance units. It is convenient to use a bus sniffer to catch that part of the communication. Here are some tools from my home lab that work for me:
* a (https://sigrok.org/wiki/Supported_hardware)[logic analyzer] compatible with [PulseView](https://sigrok.org/wiki/PulseView) (approx. 10 EUR),
* [Arduino SLCAN monitor](https://github.com/latonita/arduino-canbus-monitor) (MIT license) plus [SavvyCAN](https://www.savvycan.com/) (approx. 10 EUR),
* [CANable 2.0](https://makerbase3d.com/product/makerbase-canable-v2/) plus [Cangaroo](https://canable.io/getting-started.html#cangaroo) (under 20 EUR mark),

and the one I enjoy at the university:
* [PCAN-USB FD](https://www.peak-system.com/PCAN-USB-FD.365.0.html?&L=1) plus [PCAN-View](https://www.peak-system.com/PCAN-View.242.0.html?&L=1) or [SavvyCAN](https://www.savvycan.com/).

# Wiring diagram
![Ford Mustang (S550) 2015+ System Wiring Diagrams - Instrument cluster](/Assets/Images/ford_mustang_ipc_wiring_diagram.JPG)
Source: cardiagn.com

# If you are new to the STM32 CAN/FDCAN[^5] internal peripheral
* [CAN bus](https://en.wikipedia.org/wiki/CAN_bus) (Wikipedia)
* [CAN FD](https://en.wikipedia.org/wiki/CAN_FD) (Wikipedia)
* [CAN Protocol in STM32](https://controllerstech.com/can-protocol-in-stm32/) (ControllersTech)
* [STM32 CAN || Multiple Devices](https://www.youtube.com/watch?v=-lcrrRrKdFg) (ControllersTech)
* [STM32 FDCAN in Loopback Mode](https://controllerstech.com/stm32-fdcan-in-loopback-mode/) (ControllersTech)
* [FDCAN in Normal Mode](https://controllerstech.com/fdcan-normal-mode-stm32/) (ControllersTech)

[^5]: The FDCAN (Flexible Data-Rate Controller Area Network) peripheral is fully backward compatible with the CAN peripheral.

# Exemplary hardware for breadboarding
* [SN65HVD230 CAN Board](https://www.waveshare.com/wiki/SN65HVD230_CAN_Board) (Waveshare)

# Sources and inspirations
* [MattechPC](https://www.youtube.com/@MattechPC) (in Polish)
* [Ford Mustang (S550) CAN Bus Research & Scripts](https://github.com/EricTurner3/s550-canbus). This repo made me buy the cluster :sunglasses: Thank you, Eric Turner :exclamation:
* [parse_can_logs](https://github.com/v-ivanyshyn/parse_can_logs/blob/master/Ford%20CAN%20IDs%20Summary.md) (MIT license)
* OLED: [stm32-ssd1306](https://github.com/afiskon/stm32-ssd1306) (MIT license)

# Call for action
Build your own [home laboratory/workshop](http://ufnalski.edu.pl/control_engineering_for_hobbyists/2024_dzien_otwarty_we/Dzien_Otwarty_WE_2024_Control_Engineering_for_Hobbyists.pdf)! Get inspired by [ControllersTech](https://www.youtube.com/@ControllersTech), [DroneBot Workshop](https://www.youtube.com/@Dronebotworkshop), [Andreas Spiess](https://www.youtube.com/@AndreasSpiess), [GreatScott!](https://www.youtube.com/@greatscottlab), [ElectroBOOM](https://www.youtube.com/@ElectroBOOM), [Phil's Lab](https://www.youtube.com/@PhilsLab), [atomic14](https://www.youtube.com/@atomic14), [That Project](https://www.youtube.com/@ThatProject), [Paul McWhorter](https://www.youtube.com/@paulmcwhorter), and many other professional hobbyists sharing their awesome projects and tutorials! Shout-out/kudos to all of them!

> [!WARNING]
> Control engineering - do try this at home :exclamation:

190+ challenges to start from: [Control Engineering for Hobbyists at the Warsaw University of Technology](http://ufnalski.edu.pl/control_engineering_for_hobbyists/Control_Engineering_for_Hobbyists_list_of_challenges.pdf).

Stay tuned :exclamation:
