# LunarLander Dashboard application
Lunar lander dashboard for the assignment

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
$ java -jar LanderDash.jar
```

## Protocol Description
The Lander listens for UDP messages on 

+ 127.0.0.1
+ port 65250


1. Messages are formatted as lines of text ending with newlines.
2. Lines are Key-value pairs with a colon separating key and value

### Message format
The two key fields for the message are 
```
fuel:
```
and 
```
altitude:
```
With values formatted as floating point numbers.

