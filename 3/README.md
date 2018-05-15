# Homework 3
### Networked Hot Potato

Plays a game of hot potato using TCP sockets on Linux/Unix. 

A _ringmaster_ tosses a potato to a random _player_. Each player then sends the potato to another random player until the potato explodes. 

### Usage

Each system is either a _master_ or a _player_. Only one master should exist (read: no testing has occurred for multiple masters).

For the master: 

```bash
make all && chmod +x master && ./master <port-number> <number-of-players> <hops>
```

Where `hops` above is the amount of hops before the potato explodes. 

For the players: 

```bash
make all && chmod +x master && ./player <master-machine-name> <port-number>
```

Where `master-machine-name` is the hostname of the _master_-hosting system and `port-number` is the port number the master is listening on. 

### Files

* `doodles/` - random scratch pad-style tiny programs to test functionality.
* `socket/` - instructor-provided examples of socket programming. Much more simple than the overall example here. 
* `REFERENCES` - required citationi of where parts of the program was sourced from. 
* `list.{c,h}` - custom generic linked list implementation. 
* `player.{c,h}` - implementation of a _player_ including initialization, playing, and deconstruction 
* `potato.h` - describes the potato interface and what it means to potato
* `ringmaster.{c,h}` - implementation of a _master_ including initialization, starting a game, and deconstruction 
* `socklib.{c,h}` - custom wrappers to ease the use of sockets in linux
