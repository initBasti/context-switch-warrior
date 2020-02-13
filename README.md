# Context-Switch-Warrior
Add on for **taskwarrior** to *switch* the task **context** depending on the *daytime* and the *defined schedule*.

### Features:
* create and modify a cronjob for the program
* define zones with assigned taskwarrior contexts
* delay zone switches and cancel
* notifications on errors
* toggle if tasks are automatically canceled
* exclude time zones from the schedule(holiday, weekend)

### Todo:
* notification for upcoming events
* zone and exclusion definition from the CLI
* multiple schedules

---

#### Written in C, with the following requirments:
* taskwarrior
* getopt.h
* notify-send
* crontab

---

### How to Build
* make
* sudo make install

### Testing
make test
    
*generates results in the build/results folder*

---

#### Found Bugs or want to contribute?
please contact me @ sebastian.fricke.linux@gmx.de

---
