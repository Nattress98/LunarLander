# LunarLander
Lunar lander model for the assignment

## Instructions

To build the java application with `make`
```shell-session
$ make
```

To run the application, with `make`
```shell-session
$ make run
```
or with java directly
```shell-session
$ java -jar LunarLander.jar
```

## Protocol Description
The Lander listens for UDP messages on 

+ 127.0.0.1
+ port 65200


1. Messages are formatted as lines of text ending with newlines.
2. Lines are Key-value pairs with a colon separating key and value

### command message
The command message starts with a line 

        command:!

It is followed by one or more action lines, either 

        main-engine: 34

Where the number is the throttle percentage to set  
or  

        rcs-roll: 2.3

Where the number is the roll-rate in degrees-per-second, +ve is anti-clockwise

#### examples
```
command:
main-engine: 50
```
sets the throttle to 50%

```
command:
rcs-roll:-4
```
sets the roll-rate to -4°/s, which is anti-clockwise.

```
command:
main-engine:20
rcs-roll:1
```
sets both the throttle and roll-rate.

#### reply
the message send back in response is

```
command:=
```

### State message
The state request message queries the state of the physics model.

```
state:?
```

#### reply
The reply to the state message is
```state:=
x:45.6
y:10.3
O:5
x':0.1
y':-1
O':+0.01
```
Values are the _x_ and _y_ coordinates, the orientation _O_, and the rates of
change of these values.

### Condition
The condition request queries the server for the status of the lander.
```
condition:?
```

#### reply
The reply to the message is
```
condition:=
fuel:98%
altitude: 20.7
contact:flying|down|crashed
```
The __contact__ value is one of the three words indicating the state.

### Request Terrain 
This message requests the terrain below the lander from
its mapping radar.  This is a limited view of the terrain to a handful of
points either side of the lander.
```
terrain:?
```

#### Reply from server
```
terrain:=
points:10
data-x: 10.0,15.0,20.0,...
data-y: 12.4,12.7,13,14.0,...
```
